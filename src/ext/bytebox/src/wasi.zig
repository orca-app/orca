const std = @import("std");
const builtin = @import("builtin");
const core = @import("core.zig");

const StringPool = @import("stringpool.zig");

const Val = core.Val;
const ValType = core.ValType;
const ModuleInstance = core.ModuleInstance;
const ModuleImportPackage = core.ModuleImportPackage;

const WasiContext = struct {
    const FdInfo = struct {
        fd: std.os.fd_t,
        path_absolute: []const u8,
        rights: WasiRights,
        is_preopen: bool,
        open_handles: u32 = 1,
        dir_entries: std.ArrayList(WasiDirEntry),
    };

    cwd: []const u8,
    argv: [][]const u8 = &[_][]u8{},
    env: [][]const u8 = &[_][]u8{},
    dirs: [][]const u8 = &[_][]u8{},

    // having a master table with a side table of wasi file descriptors lets us map multiple wasi fds into the same
    // master entry and avoid duplicating OS handles, which has proved buggy on win32
    fd_table: std.ArrayList(FdInfo),
    fd_table_freelist: std.ArrayList(u32),
    fd_wasi_table: std.AutoHashMap(u32, u32), // fd_wasi -> fd_table index
    fd_path_lookup: std.StringHashMap(u32), // path_absolute -> fd_table index

    strings: StringPool,
    next_fd_id: u32 = 3,
    allocator: std.mem.Allocator,

    fn init(opts: *const WasiOpts, allocator: std.mem.Allocator) !WasiContext {
        var context = WasiContext{
            .cwd = "",
            .fd_table = std.ArrayList(FdInfo).init(allocator),
            .fd_table_freelist = std.ArrayList(u32).init(allocator),
            .fd_wasi_table = std.AutoHashMap(u32, u32).init(allocator),
            .fd_path_lookup = std.StringHashMap(u32).init(allocator),
            .strings = StringPool.init(1024 * 1024 * 4, allocator), // 4MB for absolute paths
            .allocator = allocator,
        };

        {
            var cwd_buffer: [std.fs.MAX_PATH_BYTES]u8 = undefined;
            const cwd: []const u8 = try std.os.getcwd(&cwd_buffer);
            context.cwd = try context.strings.put(cwd);
        }

        if (opts.argv) |argv| {
            context.argv = try context.allocator.dupe([]const u8, argv);
            for (argv, 0..) |arg, i| {
                context.argv[i] = try context.strings.put(arg);
            }
        }

        if (opts.env) |env| {
            context.env = try context.allocator.dupe([]const u8, env);
            for (env, 0..) |e, i| {
                context.env[i] = try context.strings.put(e);
            }
        }

        if (opts.dirs) |dirs| {
            context.dirs = try context.allocator.dupe([]const u8, dirs);
            for (dirs, 0..) |dir, i| {
                context.dirs[i] = try context.resolveAndCache(null, dir);
            }
        }

        const path_stdin = try context.strings.put("stdin");
        const path_stdout = try context.strings.put("stdout");
        const path_stderr = try context.strings.put("stderr");

        var empty_dir_entries = std.ArrayList(WasiDirEntry).init(allocator);

        try context.fd_table.ensureTotalCapacity(3 + context.dirs.len);
        context.fd_table.appendAssumeCapacity(FdInfo{ .fd = std.io.getStdIn().handle, .path_absolute = path_stdin, .rights = .{}, .is_preopen = true, .dir_entries = empty_dir_entries });
        context.fd_table.appendAssumeCapacity(FdInfo{ .fd = std.io.getStdOut().handle, .path_absolute = path_stdout, .rights = .{}, .is_preopen = true, .dir_entries = empty_dir_entries });
        context.fd_table.appendAssumeCapacity(FdInfo{ .fd = std.io.getStdErr().handle, .path_absolute = path_stderr, .rights = .{}, .is_preopen = true, .dir_entries = empty_dir_entries });
        try context.fd_wasi_table.put(0, 0);
        try context.fd_wasi_table.put(1, 1);
        try context.fd_wasi_table.put(2, 2);

        for (context.dirs) |dir_path| {
            const openflags = WasiOpenFlags{
                .creat = false,
                .directory = true,
                .excl = false,
                .trunc = false,
            };
            const fdflags = WasiFdFlags{
                .append = false,
                .dsync = false,
                .nonblock = false,
                .rsync = false,
                .sync = false,
            };
            const rights = WasiRights{
                .fd_read = true,
                .fd_write = false, // we don't need to edit the directory itself
                .fd_seek = false, // directories don't have seek rights
            };
            const lookupflags = WasiLookupFlags{
                .symlink_follow = true,
            };
            var unused: Errno = undefined;
            const is_preopen = true;
            _ = context.fdOpen(null, dir_path, lookupflags, openflags, fdflags, rights, is_preopen, &unused);
        }

        return context;
    }

    fn deinit(self: *WasiContext) void {
        for (self.fd_table.items) |item| {
            item.dir_entries.deinit();
        }
        self.fd_table.deinit();
        self.fd_wasi_table.deinit();
        self.fd_path_lookup.deinit();
        self.strings.deinit();
    }

    fn resolveAndCache(self: *WasiContext, fd_info_dir: ?*FdInfo, path: []const u8) ![]const u8 {
        if (std.mem.indexOf(u8, path, &[_]u8{0})) |_| {
            return error.NullTerminatedPath;
        }

        // validate the scope of the path never leaves the preopen root
        {
            var depth: i32 = 0;
            var token_iter = std.mem.tokenize(u8, path, &[_]u8{ '/', '\\' });
            while (token_iter.next()) |item| {
                if (std.mem.eql(u8, item, "..")) {
                    depth -= 1;
                } else {
                    depth += 1;
                }
                if (depth < 0) {
                    return error.PathInvalidDepth;
                }
            }
        }

        var static_path_buffer: [std.fs.MAX_PATH_BYTES * 2]u8 = undefined;
        var fba = std.heap.FixedBufferAllocator.init(&static_path_buffer);
        const allocator = fba.allocator();

        const dir_path = if (fd_info_dir) |info| info.path_absolute else self.cwd;
        const paths = [_][]const u8{ dir_path, path };

        if (std.fs.path.resolve(allocator, &paths)) |resolved_path| {
            // preserve trailing slash
            var final_path = resolved_path;
            const last_char = path[path.len - 1];
            if (last_char == '/' or last_char == '\\') {
                final_path = try allocator.realloc(resolved_path, resolved_path.len + 1);
                final_path[final_path.len - 1] = std.fs.path.sep;
            }

            const cached_path: []const u8 = try self.strings.findOrPut(final_path);
            return cached_path;
        } else |err| {
            return err;
        }
    }

    fn fdLookup(self: *const WasiContext, fd_wasi: u32, errno: *Errno) ?*FdInfo {
        if (self.fd_wasi_table.get(fd_wasi)) |fd_table_index| {
            return &self.fd_table.items[fd_table_index];
        }

        errno.* = Errno.BADF;
        return null;
    }

    fn fdDirPath(self: *WasiContext, fd_wasi: u32, errno: *Errno) ?[]const u8 {
        if (Helpers.isStdioHandle(fd_wasi) == false) { // std handles are 0, 1, 2 so they're not valid paths
            if (self.fd_wasi_table.get(fd_wasi)) |fd_table_index| {
                const info: *FdInfo = &self.fd_table.items[fd_table_index];
                const path_relative = info.path_absolute[self.cwd.len + 1 ..]; // +1 to skip the last path separator
                return path_relative;
            }
        }

        errno.* = Errno.BADF;
        return null;
    }

    fn fdOpen(self: *WasiContext, fd_info_dir: ?*FdInfo, path: []const u8, lookupflags: WasiLookupFlags, openflags: WasiOpenFlags, fdflags: WasiFdFlags, rights: WasiRights, is_preopen: bool, errno: *Errno) ?u32 {
        if (self.resolveAndCache(fd_info_dir, path)) |resolved_path| {
            // Found an entry for this path, just reuse it while creating a new wasi fd
            if (self.fd_path_lookup.get(resolved_path)) |fd_table_index| {
                var fd_wasi: u32 = self.next_fd_id;
                self.next_fd_id += 1;
                self.fd_wasi_table.put(fd_wasi, fd_table_index) catch |err| {
                    errno.* = Errno.translateError(err);
                    return null;
                };
                self.fd_table.items[fd_table_index].open_handles += 1;
                return fd_wasi;
            }

            const open_func = if (builtin.os.tag == .windows) Helpers.openPathWindows else Helpers.openPathPosix;

            // if a path ends with a separator, posix treats it as a directory even if the flag isn't set, so make sure
            // we explicitly set the directory flag for similar behavior on windows
            var openflags2 = openflags;
            if (std.mem.endsWith(u8, resolved_path, std.fs.path.sep_str)) {
                openflags2.directory = true;
            }

            if (open_func(resolved_path, lookupflags, openflags2, fdflags, rights, errno)) |fd_os| {
                var fd_wasi: u32 = self.next_fd_id;
                self.next_fd_id += 1;

                var info: *FdInfo = undefined;
                var fd_table_index: u32 = undefined;

                if (self.fd_table_freelist.popOrNull()) |free_index| {
                    fd_table_index = free_index;
                    info = &self.fd_table.items[free_index];
                } else {
                    self.fd_table_freelist.ensureTotalCapacity(self.fd_table.items.len + 1) catch |err| {
                        errno.* = Errno.translateError(err);
                        return null;
                    };
                    fd_table_index = @intCast(self.fd_table.items.len);
                    info = self.fd_table.addOne() catch |err| {
                        errno.* = Errno.translateError(err);
                        return null;
                    };
                }

                info.fd = fd_os;
                info.path_absolute = resolved_path;
                info.rights = rights;
                info.is_preopen = is_preopen;
                info.open_handles = 1;
                info.dir_entries = std.ArrayList(WasiDirEntry).init(self.allocator);

                self.fd_wasi_table.put(fd_wasi, fd_table_index) catch |err| {
                    errno.* = Errno.translateError(err);
                    return null;
                };
                self.fd_path_lookup.put(resolved_path, fd_table_index) catch |err| {
                    errno.* = Errno.translateError(err);
                    return null;
                };

                return fd_wasi;
            }
        } else |err| {
            errno.* = Errno.translateError(err);
        }

        return null;
    }

    fn fdUpdate(self: *WasiContext, fd_wasi: u32, new_fd: std.os.fd_t) void {
        if (self.fd_wasi_table.get(fd_wasi)) |fd_table_index| {
            self.fd_table.items[fd_table_index].fd = new_fd;
        } else {
            unreachable; // fdUpdate should always be nested inside an fdLookup
        }
    }

    fn fdRenumber(self: *WasiContext, fd_wasi: u32, fd_wasi_new: u32, errno: *Errno) void {
        if (self.fd_wasi_table.get(fd_wasi)) |fd_table_index| {
            const fd_info: *const FdInfo = &self.fd_table.items[fd_table_index];

            if (fd_info.is_preopen) {
                errno.* = Errno.NOTSUP;
                return;
            }

            if (self.fd_wasi_table.get(fd_wasi_new)) |fd_other_table_index| {
                // need to replace the existing entry with the new one
                if (fd_other_table_index != fd_table_index) {
                    const fd_info_other: *const FdInfo = &self.fd_table.items[fd_table_index];
                    if (fd_info_other.is_preopen) {
                        errno.* = Errno.NOTSUP;
                        return;
                    }

                    var unused: Errno = undefined;
                    self.fdClose(fd_wasi_new, &unused);
                }
            }

            self.fd_wasi_table.put(fd_wasi_new, fd_table_index) catch |err| {
                errno.* = Errno.translateError(err);
                return;
            };

            _ = self.fd_wasi_table.remove(fd_wasi);
        } else {
            errno.* = Errno.BADF;
        }
    }

    fn fdClose(self: *WasiContext, fd_wasi: u32, errno: *Errno) void {
        if (self.fd_wasi_table.get(fd_wasi)) |fd_table_index| {
            var fd_info: *FdInfo = &self.fd_table.items[fd_table_index];

            _ = self.fd_wasi_table.remove(fd_wasi);
            _ = self.fd_path_lookup.remove(fd_info.path_absolute);

            fd_info.open_handles -= 1;
            if (fd_info.open_handles == 0) {
                std.os.close(fd_info.fd);
                self.fd_table_freelist.appendAssumeCapacity(fd_table_index); // capacity was allocated when the associated fd_table slot was allocated
            }
        } else {
            errno.* = Errno.BADF;
        }
    }

    // The main intention for this function is to close all wasi fd when a path is unlinked.
    fn fdCleanup(self: *WasiContext, path_absolute: []const u8) void {
        if (self.fd_path_lookup.get(path_absolute)) |fd_table_index| {
            var iter = self.fd_wasi_table.iterator();
            while (iter.next()) |kv| {
                if (kv.value_ptr.* == fd_table_index) {
                    self.fd_wasi_table.removeByPtr(kv.key_ptr);
                }
            }

            _ = self.fd_path_lookup.remove(path_absolute);

            var fd_info: *FdInfo = &self.fd_table.items[fd_table_index];
            std.os.close(fd_info.fd);
            fd_info.open_handles = 0;
            self.fd_table_freelist.appendAssumeCapacity(fd_table_index); // capacity was allocated when the associated fd_table slot was allocated
        }
    }

    fn hasPathAccess(self: *WasiContext, fd_info: *const FdInfo, relative_path: []const u8, errno: *Errno) bool {
        errno.* = Errno.PERM;

        if (self.dirs.len > 0) {
            const paths = [_][]const u8{ fd_info.path_absolute, relative_path };

            if (std.fs.path.resolve(self.allocator, &paths)) |resolved_path| {
                defer self.allocator.free(resolved_path);
                for (self.dirs) |allowdir| {
                    // can use startsWith to check because all the paths have been passed through resolve() already
                    if (std.mem.startsWith(u8, resolved_path, allowdir)) {
                        errno.* = Errno.SUCCESS;
                        return true;
                    }
                }
            } else |err| {
                errno.* = Errno.translateError(err);
            }
        }

        return false;
    }

    fn fromUserdata(userdata: ?*anyopaque) *WasiContext {
        std.debug.assert(userdata != null);
        return @as(*WasiContext, @alignCast(@ptrCast(userdata.?)));
    }
};

// Values taken from https://github.com/AssemblyScript/wasi-shim/blob/main/assembly/bindings/
const Errno = enum(u8) {
    SUCCESS = 0, // No error occurred. System call completed successfully.
    TOOBIG = 1, // Argument list too long.
    ACCES = 2, // Permission denied.
    ADDRINUSE = 3, // Address in use.
    ADDRNOTAVAIL = 4, // Address not available.
    AFNOSUPPORT = 5, // Address family not supported.
    AGAIN = 6, // Resource unavailable, or operation would block.
    ALREADY = 7, // Connection already in progress.
    BADF = 8, // Bad file descriptor.
    BADMSG = 9, // Bad message.
    BUSY = 10, // Device or resource busy.
    CANCELED = 11, // Operation canceled.
    CHILD = 12, // No child processes.
    CONNABORTED = 13, // Connection aborted.
    CONNREFUSED = 14, // Connection refused.
    CONNRESET = 15, // Connection reset.
    DEADLK = 16, // Resource deadlock would occur.
    DESTADDRREQ = 17, // Destination address required.
    DOM = 18, // Mathematics argument out of domain of function.
    DQUOT = 19, // Reserved.
    EXIST = 20, // File exists.
    FAULT = 21, // Bad address.
    FBIG = 22, // File too large.
    HOSTUNREACH = 23, // Host is unreachable.
    IDRM = 24, // Identifier removed.
    ILSEQ = 25, // Illegal byte sequence.
    INPROGRESS = 26, // Operation in progress.
    INTR = 27, // Interrupted function.
    INVAL = 28, // Invalid argument.
    IO = 29, // I/O error.
    ISCONN = 30, // Socket is connected.
    ISDIR = 31, // Is a directory.
    LOOP = 32, // Too many levels of symbolic links.
    MFILE = 33, // File descriptor value too large.
    MLINK = 34, // Too many links.
    MSGSIZE = 35, // Message too large.
    MULTIHOP = 36, // Reserved.
    NAMETOOLONG = 37, // Filename too long.
    NETDOWN = 38, // Network is down.
    NETRESET = 39, // Connection aborted by network.
    NETUNREACH = 40, // Network unreachable.
    NFILE = 41, // Too many files open in system.
    NOBUFS = 42, // No buffer space available.
    NODEV = 43, // No such device.
    NOENT = 44, // No such file or directory.
    NOEXEC = 45, // Executable file format error.
    NOLCK = 46, // No locks available.
    NOLINK = 47, // Reserved.
    NOMEM = 48, // Not enough space.
    NOMSG = 49, // No message of the desired type.
    NOPROTOOPT = 50, // Protocol not available.
    NOSPC = 51, // No space left on device.
    NOSYS = 52, // Function not supported.
    NOTCONN = 53, // The socket is not connected.
    NOTDIR = 54, // Not a directory or a symbolic link to a directory.
    NOTEMPTY = 55, // Directory not empty.
    NOTRECOVERABLE = 56, // State not recoverable.
    NOTSOCK = 57, // Not a socket.
    NOTSUP = 58, // Not supported, or operation not supported on socket.
    NOTTY = 59, // Inappropriate I/O control operation.
    NXIO = 60, // No such device or address.
    OVERFLOW = 61, // Value too large to be stored in data type.
    OWNERDEAD = 62, // Previous owner died.
    PERM = 63, // Operation not permitted.
    PIPE = 64, // Broken pipe.
    PROTO = 65, // Protocol error.
    PROTONOSUPPORT = 66, // Protocol not supported.
    PROTOTYPE = 67, // Protocol wrong type for socket.
    RANGE = 68, // Result too large.
    ROFS = 69, // Read-only file system.
    SPIPE = 70, // Invalid seek.
    SRCH = 71, // No such process.
    STALE = 72, // Reserved.
    TIMEDOUT = 73, // Connection timed out.
    TXTBSY = 74, // Text file busy.
    XDEV = 75, // Cross-device link.
    NOTCAPABLE = 76, // Extension: Capabilities insufficient.

    fn translateError(err: anyerror) Errno {
        return switch (err) {
            error.AccessDenied => .ACCES,
            error.DeviceBusy => .BUSY,
            error.DirNotEmpty => .NOTEMPTY,
            error.DiskQuota => .DQUOT,
            error.FileBusy => .TXTBSY,
            error.FileLocksNotSupported => .NOTSUP,
            error.FileNotFound => .NOENT,
            error.FileTooBig => .FBIG,
            error.FileSystem => .IO,
            error.InputOutput => .IO,
            error.IsDir => .ISDIR,
            error.LinkQuotaExceeded => .MLINK,
            error.NameTooLong => .NAMETOOLONG,
            error.NoDevice => .NODEV,
            error.NoSpaceLeft => .NOSPC,
            error.NotDir => .NOTDIR,
            error.OutOfMemory => .NOMEM,
            error.PathAlreadyExists => .EXIST,
            error.ProcessFdQuotaExceeded => .MFILE,
            error.ReadOnlyFileSystem => .ROFS,
            error.SymLinkLoop => .LOOP,
            error.SystemFdQuotaExceeded => .NFILE,
            error.SystemResources => .NOMEM,
            error.Unseekable => .SPIPE,
            error.WouldBlock => .AGAIN,
            error.InvalidUtf8 => .INVAL,
            error.BadPathName => .INVAL,
            error.NullTerminatedPath => .ILSEQ,
            error.PathInvalidDepth => .PERM,
            else => .INVAL,
        };
    }

    fn getLastWin32Error() Errno {
        const err = std.os.windows.kernel32.GetLastError();
        return switch (err) {
            else => .INVAL,
        };
    }
};

const WasiLookupFlags = packed struct {
    symlink_follow: bool,
};

const WasiOpenFlags = packed struct {
    creat: bool,
    directory: bool,
    excl: bool,
    trunc: bool,
};

const WasiRights = packed struct {
    fd_datasync: bool = true,
    fd_read: bool = true,
    fd_seek: bool = true,
    fd_fdstat_set_flags: bool = true,
    fd_sync: bool = true,
    fd_tell: bool = true,
    fd_write: bool = true,
    fd_advise: bool = true,
    fd_allocate: bool = true,
    path_create_directory: bool = true,
    path_create_file: bool = true,
    path_link_source: bool = true,
    path_link_target: bool = true,
    path_open: bool = true,
    fd_readdir: bool = true,
    path_readlink: bool = true,
    path_rename_source: bool = true,
    path_rename_target: bool = true,
    path_filestat_get: bool = true,
    path_filestat_set_size: bool = true,
    path_filestat_set_times: bool = true,
    fd_filestat_get: bool = true,
    fd_filestat_set_size: bool = true,
    fd_filestat_set_times: bool = true,
    path_symlink: bool = true,
    path_remove_directory: bool = true,
    path_unlink_file: bool = true,
    poll_fd_readwrite: bool = true,
    sock_shutdown: bool = true,
    sock_accept: bool = true,
};

const WasiFdFlags = packed struct {
    append: bool,
    dsync: bool,
    nonblock: bool,
    rsync: bool,
    sync: bool,
};

const WasiDirEntry = struct {
    inode: u64,
    filetype: std.os.wasi.filetype_t,
    filename: []u8,
};

const Whence = enum(u8) {
    Set,
    Cur,
    End,

    fn fromInt(int: i32) ?Whence {
        return switch (int) {
            0 => .Set,
            1 => .Cur,
            2 => .End,
            else => null,
        };
    }
};

// Since the windows API is so large, wrapping the win32 API is not in the scope of the stdlib, so it
// prefers to only declare windows functions it uses. In these cases we just declare the needed functions
// and types here.
const WindowsApi = struct {
    const windows = std.os.windows;

    const BOOL = windows.BOOL;
    const DWORD = windows.DWORD;
    const FILETIME = windows.FILETIME;
    const HANDLE = windows.HANDLE;
    const LARGE_INTEGER = windows.LARGE_INTEGER;
    const ULONG = windows.ULONG;
    const WCHAR = windows.WCHAR;
    const WINAPI = windows.WINAPI;
    const LPCWSTR = windows.LPCWSTR;

    const CLOCK = struct {
        const REALTIME = 0;
        const MONOTONIC = 1;
        const PROCESS_CPUTIME_ID = 2;
        const THREAD_CPUTIME_ID = 3;
    };

    const BY_HANDLE_FILE_INFORMATION = extern struct {
        dwFileAttributes: DWORD,
        ftCreationTime: FILETIME,
        ftLastAccessTime: FILETIME,
        ftLastWriteTime: FILETIME,
        dwVolumeSerialNumber: DWORD,
        nFileSizeHigh: DWORD,
        nFileSizeLow: DWORD,
        nNumberOfLinks: DWORD,
        nFileIndexHigh: DWORD,
        nFileIndexLow: DWORD,
    };

    const FILE_ID_FULL_DIR_INFORMATION = extern struct {
        NextEntryOffset: ULONG,
        FileIndex: ULONG,
        CreationTime: LARGE_INTEGER,
        LastAccessTime: LARGE_INTEGER,
        LastWriteTime: LARGE_INTEGER,
        ChangeTime: LARGE_INTEGER,
        EndOfFile: LARGE_INTEGER,
        AllocationSize: LARGE_INTEGER,
        FileAttributes: ULONG,
        FileNameLength: ULONG,
        EaSize: ULONG,
        FileId: LARGE_INTEGER,
        FileName: [1]WCHAR,
    };

    const SYMBOLIC_LINK_FLAG_FILE: DWORD = 0x0;
    const SYMBOLIC_LINK_FLAG_DIRECTORY: DWORD = 0x1;
    const SYMBOLIC_LINK_FLAG_ALLOW_UNPRIVILEGED_CREATE: DWORD = 0x2;

    extern "kernel32" fn GetSystemTimeAdjustment(timeAdjustment: *DWORD, timeIncrement: *DWORD, timeAdjustmentDisabled: *BOOL) callconv(WINAPI) BOOL;
    extern "kernel32" fn GetThreadTimes(in_hProcess: HANDLE, creationTime: *FILETIME, exitTime: *FILETIME, kernelTime: *FILETIME, userTime: *FILETIME) callconv(WINAPI) BOOL;
    extern "kernel32" fn GetFileInformationByHandle(file: HANDLE, fileInformation: *BY_HANDLE_FILE_INFORMATION) callconv(WINAPI) BOOL;
    extern "kernel32" fn CreateSymbolicLinkW(symlinkFileName: LPCWSTR, lpTargetFileName: LPCWSTR, flags: DWORD) callconv(WINAPI) BOOL;
    extern "kernel32" fn SetEndOfFile(file: HANDLE) callconv(WINAPI) BOOL;

    const GetCurrentProcess = std.os.windows.kernel32.GetCurrentProcess;
};

const FD_OS_INVALID = switch (builtin.os.tag) {
    .windows => std.os.windows.INVALID_HANDLE_VALUE,
    else => -1,
};

const Helpers = struct {
    fn signedCast(comptime T: type, value: anytype, errno: *Errno) T {
        if (value >= 0) {
            return @as(T, @intCast(value));
        }
        errno.* = Errno.INVAL;
        return 0;
    }

    fn resolvePath(fd_info: *const WasiContext.FdInfo, path_relative: []const u8, path_buffer: []u8, _: *Errno) ?[]const u8 {
        var fba = std.heap.FixedBufferAllocator.init(path_buffer[std.fs.MAX_PATH_BYTES..]);
        const allocator = fba.allocator();

        const paths = [_][]const u8{ fd_info.path_absolute, path_relative };
        const resolved_path = std.fs.path.resolve(allocator, &paths) catch unreachable;
        return resolved_path;
    }

    fn getMemorySlice(module: *ModuleInstance, offset: usize, length: usize, errno: *Errno) ?[]u8 {
        const mem: []u8 = module.memorySlice(offset, length);
        if (mem.len != length) {
            errno.* = Errno.FAULT;
            return null;
        }
        return mem;
    }

    fn writeIntToMemory(comptime T: type, value: T, offset: usize, module: *ModuleInstance, errno: *Errno) void {
        if (module.memoryWriteInt(T, value, offset) == false) {
            errno.* = Errno.FAULT;
        }
    }

    fn writeFilestatToMemory(stat: *const std.os.wasi.filestat_t, offset: u32, module: *ModuleInstance, errno: *Errno) void {
        const filetype = @intFromEnum(stat.filetype);
        Helpers.writeIntToMemory(u64, stat.dev, offset + 0, module, errno);
        Helpers.writeIntToMemory(u64, stat.ino, offset + 8, module, errno);
        Helpers.writeIntToMemory(u8, filetype, offset + 16, module, errno);
        Helpers.writeIntToMemory(u64, stat.nlink, offset + 24, module, errno);
        Helpers.writeIntToMemory(u64, stat.size, offset + 32, module, errno);
        Helpers.writeIntToMemory(u64, stat.atim, offset + 40, module, errno);
        Helpers.writeIntToMemory(u64, stat.mtim, offset + 48, module, errno);
        Helpers.writeIntToMemory(u64, stat.ctim, offset + 56, module, errno);
    }

    fn isStdioHandle(fd_wasi: u32) bool {
        return fd_wasi < 3; // std handles are 0, 1, 2 (stdin, stdout, stderr)
    }

    fn stringsSizesGet(module: *ModuleInstance, strings: [][]const u8, params: [*]const Val, returns: [*]Val) void {
        const strings_count: u32 = @as(u32, @intCast(strings.len));
        var strings_length: u32 = 0;
        for (strings) |string| {
            strings_length += @as(u32, @intCast(string.len)) + 1; // +1 for required null terminator of each string
        }

        var errno = Errno.SUCCESS;

        const dest_string_count = Helpers.signedCast(u32, params[0].I32, &errno);
        const dest_string_length = Helpers.signedCast(u32, params[1].I32, &errno);

        if (errno == .SUCCESS) {
            writeIntToMemory(u32, strings_count, dest_string_count, module, &errno);
            writeIntToMemory(u32, strings_length, dest_string_length, module, &errno);
        }

        returns[0] = Val{ .I32 = @intFromEnum(errno) };
    }

    fn stringsGet(module: *ModuleInstance, strings: [][]const u8, params: [*]const Val, returns: [*]Val) void {
        var errno = Errno.SUCCESS;

        const dest_string_ptrs_begin = Helpers.signedCast(u32, params[0].I32, &errno);
        const dest_string_mem_begin = Helpers.signedCast(u32, params[1].I32, &errno);

        if (errno == .SUCCESS) {
            var dest_string_ptrs: u32 = dest_string_ptrs_begin;
            var dest_string_strings: u32 = dest_string_mem_begin;

            for (strings) |string| {
                writeIntToMemory(u32, dest_string_strings, dest_string_ptrs, module, &errno);

                if (getMemorySlice(module, dest_string_strings, string.len + 1, &errno)) |mem| {
                    std.mem.copy(u8, mem[0..string.len], string);
                    mem[string.len] = 0; // null terminator

                    dest_string_ptrs += @sizeOf(u32);
                    dest_string_strings += @as(u32, @intCast(string.len + 1));
                }
            }
        }

        returns[0] = Val{ .I32 = @intFromEnum(errno) };
    }

    fn convertClockId(wasi_clockid: i32, errno: *Errno) i32 {
        return switch (wasi_clockid) {
            std.os.wasi.CLOCK.REALTIME => if (builtin.os.tag != .windows) std.os.system.CLOCK.REALTIME else WindowsApi.CLOCK.REALTIME,
            std.os.wasi.CLOCK.MONOTONIC => if (builtin.os.tag != .windows) std.os.system.CLOCK.MONOTONIC else WindowsApi.CLOCK.MONOTONIC,
            std.os.wasi.CLOCK.PROCESS_CPUTIME_ID => if (builtin.os.tag != .windows) std.os.system.CLOCK.PROCESS_CPUTIME_ID else WindowsApi.CLOCK.PROCESS_CPUTIME_ID,
            std.os.wasi.CLOCK.THREAD_CPUTIME_ID => if (builtin.os.tag != .windows) std.os.system.CLOCK.THREAD_CPUTIME_ID else WindowsApi.CLOCK.THREAD_CPUTIME_ID,
            else => {
                errno.* = Errno.INVAL;
                return 0;
            },
        };
    }

    fn posixTimespecToWasi(ts: std.os.system.timespec) std.os.wasi.timestamp_t {
        const ns_per_second = 1000000000;
        const sec_part = @as(u64, @intCast(ts.tv_sec));
        const nsec_part = @as(u64, @intCast(ts.tv_nsec));
        const timestamp_ns: u64 = (sec_part * ns_per_second) + nsec_part;
        return timestamp_ns;
    }

    fn filetimeToU64(ft: std.os.windows.FILETIME) u64 {
        const v: u64 = (@as(u64, @intCast(ft.dwHighDateTime)) << 32) | ft.dwLowDateTime;
        return v;
    }

    fn windowsFiletimeToWasi(ft: std.os.windows.FILETIME) std.os.wasi.timestamp_t {
        // Windows epoch starts on Jan 1, 1601. Unix epoch starts on Jan 1, 1970.
        const win_epoch_to_unix_epoch_100ns: u64 = 116444736000000000;
        const timestamp_windows_100ns: u64 = Helpers.filetimeToU64(ft);

        const timestamp_100ns: u64 = timestamp_windows_100ns - win_epoch_to_unix_epoch_100ns;
        const timestamp_ns: u64 = timestamp_100ns * 100;
        return timestamp_ns;
    }

    fn decodeLookupFlags(value: i32) WasiLookupFlags {
        return WasiLookupFlags{
            .symlink_follow = (value & 0x01) != 0,
        };
    }

    fn decodeOpenFlags(value: i32) WasiOpenFlags {
        return WasiOpenFlags{
            .creat = (value & 0x01) != 0,
            .directory = (value & 0x02) != 0,
            .excl = (value & 0x04) != 0,
            .trunc = (value & 0x08) != 0,
        };
    }

    fn decodeRights(value: i64) WasiRights {
        return WasiRights{
            .fd_datasync = (value & 0x0001) != 0,
            .fd_read = (value & 0x0002) != 0,
            .fd_seek = (value & 0x0004) != 0,
            .fd_fdstat_set_flags = (value & 0x0008) != 0,

            .fd_sync = (value & 0x0010) != 0,
            .fd_tell = (value & 0x0020) != 0,
            .fd_write = (value & 0x0040) != 0,
            .fd_advise = (value & 0x0080) != 0,

            .fd_allocate = (value & 0x0100) != 0,
            .path_create_directory = (value & 0x0200) != 0,
            .path_create_file = (value & 0x0400) != 0,
            .path_link_source = (value & 0x0800) != 0,

            .path_link_target = (value & 0x1000) != 0,
            .path_open = (value & 0x2000) != 0,
            .fd_readdir = (value & 0x4000) != 0,
            .path_readlink = (value & 0x8000) != 0,

            .path_rename_source = (value & 0x10000) != 0,
            .path_rename_target = (value & 0x20000) != 0,
            .path_filestat_get = (value & 0x40000) != 0,
            .path_filestat_set_size = (value & 0x80000) != 0,

            .path_filestat_set_times = (value & 0x100000) != 0,
            .fd_filestat_get = (value & 0x200000) != 0,
            .fd_filestat_set_size = (value & 0x400000) != 0,
            .fd_filestat_set_times = (value & 0x800000) != 0,

            .path_symlink = (value & 0x1000000) != 0,
            .path_remove_directory = (value & 0x2000000) != 0,
            .path_unlink_file = (value & 0x4000000) != 0,
            .poll_fd_readwrite = (value & 0x8000000) != 0,

            .sock_shutdown = (value & 0x10000000) != 0,
            .sock_accept = (value & 0x20000000) != 0,
        };
    }

    fn decodeFdFlags(value: i32) WasiFdFlags {
        return WasiFdFlags{
            .append = (value & 0x01) != 0,
            .dsync = (value & 0x02) != 0,
            .nonblock = (value & 0x04) != 0,
            .rsync = (value & 0x08) != 0,
            .sync = (value & 0x10) != 0,
        };
    }

    fn fdflagsToFlagsPosix(fdflags: WasiFdFlags) u32 {
        var flags: u32 = 0;

        if (fdflags.append) {
            flags |= std.os.O.APPEND;
        }
        if (fdflags.dsync) {
            flags |= std.os.O.DSYNC;
        }
        if (fdflags.nonblock) {
            flags |= std.os.O.NONBLOCK;
        }
        if (builtin.os.tag != .macos and fdflags.rsync) {
            flags |= std.os.O.RSYNC;
        }
        if (fdflags.sync) {
            flags |= std.os.O.SYNC;
        }

        return flags;
    }

    fn windowsFileAttributeToWasiFiletype(fileAttributes: WindowsApi.DWORD) std.os.wasi.filetype_t {
        if (fileAttributes & std.os.windows.FILE_ATTRIBUTE_DIRECTORY != 0) {
            return .DIRECTORY;
        } else if (fileAttributes & std.os.windows.FILE_ATTRIBUTE_REPARSE_POINT != 0) {
            return .SYMBOLIC_LINK;
        } else {
            return .REGULAR_FILE;
        }
    }

    fn posixModeToWasiFiletype(mode: std.os.mode_t) std.os.wasi.filetype_t {
        if (std.os.S.ISREG(mode)) {
            return .REGULAR_FILE;
        } else if (std.os.S.ISDIR(mode)) {
            return .DIRECTORY;
        } else if (std.os.S.ISCHR(mode)) {
            return .CHARACTER_DEVICE;
        } else if (std.os.S.ISBLK(mode)) {
            return .BLOCK_DEVICE;
        } else if (std.os.S.ISLNK(mode)) {
            return .SYMBOLIC_LINK;
            // } else if (std.os.S.ISSOCK(mode)) {
            //     stat_wasi.fs_filetype = std.os.wasi.filetype_t.SOCKET_STREAM; // not sure if this is SOCKET_STREAM or SOCKET_DGRAM
            // }
        } else {
            return .UNKNOWN;
        }
    }

    fn fdstatGetWindows(fd: std.os.fd_t, errno: *Errno) std.os.wasi.fdstat_t {
        if (builtin.os.tag != .windows) {
            @compileError("This function should only be called on the Windows OS.");
        }

        var stat_wasi = std.os.wasi.fdstat_t{
            .fs_filetype = std.os.wasi.filetype_t.REGULAR_FILE,
            .fs_flags = 0,
            .fs_rights_base = std.os.wasi.RIGHT.ALL,
            .fs_rights_inheriting = std.os.wasi.RIGHT.ALL,
        };

        var info: WindowsApi.BY_HANDLE_FILE_INFORMATION = undefined;
        if (WindowsApi.GetFileInformationByHandle(fd, &info) == std.os.windows.TRUE) {
            stat_wasi.fs_filetype = windowsFileAttributeToWasiFiletype(info.dwFileAttributes);

            if (stat_wasi.fs_filetype == .DIRECTORY) {
                stat_wasi.fs_rights_base &= ~std.os.wasi.RIGHT.FD_SEEK;
            }

            if (info.dwFileAttributes & std.os.windows.FILE_ATTRIBUTE_READONLY != 0) {
                stat_wasi.fs_rights_base &= ~std.os.wasi.RIGHT.FD_WRITE;
            }
        } else {
            errno.* = Errno.getLastWin32Error();
        }

        stat_wasi.fs_rights_inheriting = stat_wasi.fs_rights_base;

        return stat_wasi;
    }

    fn fdstatGetPosix(fd: std.os.fd_t, errno: *Errno) std.os.wasi.fdstat_t {
        if (builtin.os.tag == .windows) {
            @compileError("This function should only be called on an OS that supports posix APIs.");
        }

        var stat_wasi = std.os.wasi.fdstat_t{
            .fs_filetype = std.os.wasi.filetype_t.UNKNOWN,
            .fs_flags = 0,
            .fs_rights_base = std.os.wasi.RIGHT.ALL,
            .fs_rights_inheriting = std.os.wasi.RIGHT.ALL,
        };

        if (std.os.fcntl(fd, std.os.F.GETFL, 0)) |fd_flags| {
            if (std.os.fstat(fd)) |fd_stat| {

                // filetype
                stat_wasi.fs_filetype = posixModeToWasiFiletype(fd_stat.mode);

                // flags
                if (fd_flags & std.os.O.APPEND != 0) {
                    stat_wasi.fs_flags |= std.os.wasi.FDFLAG.APPEND;
                }
                if (fd_flags & std.os.O.DSYNC != 0) {
                    stat_wasi.fs_flags |= std.os.wasi.FDFLAG.DSYNC;
                }
                if (fd_flags & std.os.O.NONBLOCK != 0) {
                    stat_wasi.fs_flags |= std.os.wasi.FDFLAG.NONBLOCK;
                }
                if (builtin.os.tag != .macos and fd_flags & std.os.O.RSYNC != 0) {
                    stat_wasi.fs_flags |= std.os.wasi.FDFLAG.RSYNC;
                }
                if (fd_flags & std.os.O.SYNC != 0) {
                    stat_wasi.fs_flags |= std.os.wasi.FDFLAG.SYNC;
                }

                // rights
                if (fd_flags & std.os.O.RDWR != 0) {
                    // noop since all rights includes this by default
                } else if (fd_flags & std.os.O.RDONLY != 0) {
                    stat_wasi.fs_rights_base &= ~std.os.wasi.RIGHT.FD_WRITE;
                } else if (fd_flags & std.os.O.WRONLY != 0) {
                    stat_wasi.fs_rights_base &= ~std.os.wasi.RIGHT.FD_READ;
                }

                if (stat_wasi.fs_filetype == .DIRECTORY) {
                    stat_wasi.fs_rights_base &= ~std.os.wasi.RIGHT.FD_SEEK;
                }
            } else |err| {
                errno.* = Errno.translateError(err);
            }
        } else |err| {
            errno.* = Errno.translateError(err);
        }

        return stat_wasi;
    }

    fn fdstatSetFlagsWindows(fd_info: *const WasiContext.FdInfo, fdflags: WasiFdFlags, errno: *Errno) ?std.os.fd_t {
        const w = std.os.windows;

        const file_pos = w.SetFilePointerEx_CURRENT_get(fd_info.fd) catch |err| {
            errno.* = Errno.translateError(err);
            return null;
        };

        w.CloseHandle(fd_info.fd);

        const pathspace_w: w.PathSpace = w.sliceToPrefixedFileW(fd_info.path_absolute) catch |err| {
            errno.* = Errno.translateError(err);
            return null;
        };
        const path_w: [:0]const u16 = pathspace_w.span();

        var access_mask: w.ULONG = w.READ_CONTROL | w.FILE_WRITE_ATTRIBUTES | w.SYNCHRONIZE;
        const write_flags: w.ULONG = if (fdflags.append) w.FILE_APPEND_DATA else w.GENERIC_WRITE;
        if (fd_info.rights.fd_read and fd_info.rights.fd_write) {
            access_mask |= w.GENERIC_READ | write_flags;
        } else if (fd_info.rights.fd_write) {
            access_mask |= write_flags;
        } else {
            access_mask |= w.GENERIC_READ | write_flags;
        }

        const path_len_bytes = @as(u16, @intCast(path_w.len * 2));
        var unicode_str = w.UNICODE_STRING{
            .Length = path_len_bytes,
            .MaximumLength = path_len_bytes,
            .Buffer = @as([*]u16, @ptrFromInt(@intFromPtr(path_w.ptr))),
        };
        var attr = w.OBJECT_ATTRIBUTES{
            .Length = @sizeOf(w.OBJECT_ATTRIBUTES),
            .RootDirectory = null,
            .Attributes = w.OBJ_CASE_INSENSITIVE,
            .ObjectName = &unicode_str,
            .SecurityDescriptor = null,
            .SecurityQualityOfService = null,
        };

        var fd_new: w.HANDLE = undefined;
        var io: w.IO_STATUS_BLOCK = undefined;
        const rc = w.ntdll.NtCreateFile(
            &fd_new,
            access_mask,
            &attr,
            &io,
            null,
            w.FILE_ATTRIBUTE_NORMAL,
            w.FILE_SHARE_WRITE | w.FILE_SHARE_READ | w.FILE_SHARE_DELETE,
            w.FILE_OPEN,
            w.FILE_NON_DIRECTORY_FILE | w.FILE_SYNCHRONOUS_IO_NONALERT,
            null,
            0,
        );

        switch (rc) {
            .SUCCESS => {},
            .OBJECT_NAME_INVALID => unreachable,
            .INVALID_PARAMETER => unreachable,
            .OBJECT_PATH_SYNTAX_BAD => unreachable,
            .INVALID_HANDLE => unreachable,
            .OBJECT_NAME_NOT_FOUND => errno.* = Errno.NOENT,
            .OBJECT_PATH_NOT_FOUND => errno.* = Errno.NOENT,
            .NO_MEDIA_IN_DEVICE => errno.* = Errno.NODEV,
            .SHARING_VIOLATION => errno.* = Errno.ACCES,
            .ACCESS_DENIED => errno.* = Errno.ACCES,
            .USER_MAPPED_FILE => errno.* = Errno.ACCES,
            .PIPE_BUSY => errno.* = Errno.BUSY,
            .OBJECT_NAME_COLLISION => errno.* = Errno.EXIST,
            .FILE_IS_A_DIRECTORY => errno.* = Errno.ISDIR,
            .NOT_A_DIRECTORY => errno.* = Errno.NOTDIR,
            else => errno.* = Errno.INVAL,
        }

        if (errno.* != Errno.SUCCESS) {
            return null;
        }

        // at this point we need to return fd_new, but don't want to silently swallow errors
        w.SetFilePointerEx_BEGIN(fd_new, file_pos) catch |err| {
            errno.* = Errno.translateError(err);
        };

        return fd_new;
    }

    fn fdstatSetFlagsPosix(fd_info: *const WasiContext.FdInfo, fdflags: WasiFdFlags, errno: *Errno) ?std.os.fd_t {
        const flags: u32 = fdflagsToFlagsPosix(fdflags);

        if (std.os.fcntl(fd_info.fd, std.os.F.SETFL, flags)) |_| {} else |err| {
            errno.* = Errno.translateError(err);
        }

        // don't need to update the fd on posix platforms, so just return null
        return null;
    }

    fn fdFilestatSetTimesWindows(fd: std.os.fd_t, timestamp_wasi_access: u64, timestamp_wasi_modified: u64, fstflags: u32, errno: *Errno) void {
        var filetime_now: WindowsApi.FILETIME = undefined; // helps avoid 2 calls to GetSystemTimeAsFiletime
        var filetime_now_needs_set: bool = true;

        var access_time: std.os.windows.FILETIME = undefined;
        var access_time_was_set: bool = false;
        if (fstflags & std.os.wasi.FILESTAT_SET_ATIM != 0) {
            access_time = std.os.windows.nanoSecondsToFileTime(timestamp_wasi_access);
            access_time_was_set = true;
        }
        if (fstflags & std.os.wasi.FILESTAT_SET_ATIM_NOW != 0) {
            std.os.windows.kernel32.GetSystemTimeAsFileTime(&filetime_now);
            filetime_now_needs_set = false;
            access_time = filetime_now;
            access_time_was_set = true;
        }

        var modify_time: std.os.windows.FILETIME = undefined;
        var modify_time_was_set: bool = false;
        if (fstflags & std.os.wasi.FILESTAT_SET_MTIM != 0) {
            modify_time = std.os.windows.nanoSecondsToFileTime(timestamp_wasi_modified);
            modify_time_was_set = true;
        }
        if (fstflags & std.os.wasi.FILESTAT_SET_MTIM_NOW != 0) {
            if (filetime_now_needs_set) {
                std.os.windows.kernel32.GetSystemTimeAsFileTime(&filetime_now);
            }
            modify_time = filetime_now;
            modify_time_was_set = true;
        }

        const access_time_ptr: ?*std.os.windows.FILETIME = if (access_time_was_set) &access_time else null;
        const modify_time_ptr: ?*std.os.windows.FILETIME = if (modify_time_was_set) &modify_time else null;

        std.os.windows.SetFileTime(fd, null, access_time_ptr, modify_time_ptr) catch |err| {
            errno.* = Errno.translateError(err);
        };
    }

    fn fdFilestatSetTimesPosix(fd: std.os.fd_t, timestamp_wasi_access: u64, timestamp_wasi_modified: u64, fstflags: u32, errno: *Errno) void {
        const is_darwin = builtin.os.tag.isDarwin();
        const UTIME_NOW: i64 = if (is_darwin) @as(i32, -1) else (1 << 30) - 1;
        const UTIME_OMIT: i64 = if (is_darwin) @as(i32, -2) else (1 << 30) - 2;

        var times = [2]std.os.timespec{
            .{ // access time
                .tv_sec = 0,
                .tv_nsec = UTIME_OMIT,
            },
            .{ // modification time
                .tv_sec = 0,
                .tv_nsec = UTIME_OMIT,
            },
        };

        if (fstflags & std.os.wasi.FILESTAT_SET_ATIM != 0) {
            var ts: std.os.wasi.timespec = std.os.wasi.timespec.fromTimestamp(timestamp_wasi_access);
            times[0].tv_sec = ts.tv_sec;
            times[0].tv_nsec = ts.tv_nsec;
        }
        if (fstflags & std.os.wasi.FILESTAT_SET_ATIM_NOW != 0) {
            times[0].tv_nsec = UTIME_NOW;
        }
        if (fstflags & std.os.wasi.FILESTAT_SET_MTIM != 0) {
            var ts: std.os.wasi.timespec = std.os.wasi.timespec.fromTimestamp(timestamp_wasi_modified);
            times[1].tv_sec = ts.tv_sec;
            times[1].tv_nsec = ts.tv_nsec;
        }
        if (fstflags & std.os.wasi.FILESTAT_SET_MTIM_NOW != 0) {
            times[1].tv_nsec = UTIME_NOW;
        }

        std.os.futimens(fd, &times) catch |err| {
            errno.* = Errno.translateError(err);
        };
    }

    fn partsToU64(high: u64, low: u64) u64 {
        return (high << 32) | low;
    }

    fn filestatGetWindows(fd: std.os.fd_t, errno: *Errno) std.os.wasi.filestat_t {
        if (builtin.os.tag != .windows) {
            @compileError("This function should only be called on an OS that supports posix APIs.");
        }

        var stat_wasi: std.os.wasi.filestat_t = undefined;

        var info: WindowsApi.BY_HANDLE_FILE_INFORMATION = undefined;
        if (WindowsApi.GetFileInformationByHandle(fd, &info) == std.os.windows.TRUE) {
            stat_wasi.dev = 0;
            stat_wasi.ino = partsToU64(info.nFileIndexHigh, info.nFileIndexLow);
            stat_wasi.filetype = windowsFileAttributeToWasiFiletype(info.dwFileAttributes);
            stat_wasi.nlink = info.nNumberOfLinks;
            stat_wasi.size = partsToU64(info.nFileSizeHigh, info.nFileSizeLow);
            stat_wasi.atim = windowsFiletimeToWasi(info.ftLastAccessTime);
            stat_wasi.mtim = windowsFiletimeToWasi(info.ftLastWriteTime);
            stat_wasi.ctim = windowsFiletimeToWasi(info.ftCreationTime);
        } else {
            errno.* = Errno.getLastWin32Error();
        }

        return stat_wasi;
    }

    fn filestatGetPosix(fd: std.os.fd_t, errno: *Errno) std.os.wasi.filestat_t {
        if (builtin.os.tag == .windows) {
            @compileError("This function should only be called on an OS that supports posix APIs.");
        }

        var stat_wasi: std.os.wasi.filestat_t = undefined;

        if (std.os.fstat(fd)) |stat| {
            stat_wasi.dev = if (builtin.os.tag == .macos) @as(u32, @bitCast(stat.dev)) else stat.dev;
            stat_wasi.ino = stat.ino;
            stat_wasi.filetype = posixModeToWasiFiletype(stat.mode);
            stat_wasi.nlink = stat.nlink;
            stat_wasi.size = if (std.math.cast(u64, stat.size)) |s| s else 0;
            if (builtin.os.tag == .macos) {
                stat_wasi.atim = posixTimespecToWasi(stat.atimespec);
                stat_wasi.mtim = posixTimespecToWasi(stat.mtimespec);
                stat_wasi.ctim = posixTimespecToWasi(stat.ctimespec);
            } else {
                stat_wasi.atim = posixTimespecToWasi(stat.atim);
                stat_wasi.mtim = posixTimespecToWasi(stat.mtim);
                stat_wasi.ctim = posixTimespecToWasi(stat.ctim);
            }
        } else |err| {
            errno.* = Errno.translateError(err);
        }

        return stat_wasi;
    }

    // As of this 0.10.1, the zig stdlib has a bug in std.os.open() that doesn't respect the append flag properly.
    // To get this working, we'll just use NtCreateFile directly.
    fn openPathWindows(path: []const u8, lookupflags: WasiLookupFlags, openflags: WasiOpenFlags, fdflags: WasiFdFlags, rights: WasiRights, errno: *Errno) ?std.os.fd_t {
        if (builtin.os.tag != .windows) {
            @compileError("This function should only be called on an OS that supports windows APIs.");
        }

        const w = std.os.windows;

        const pathspace_w: w.PathSpace = w.sliceToPrefixedFileW(path) catch |err| {
            errno.* = Errno.translateError(err);
            return null;
        };
        const path_w: [:0]const u16 = pathspace_w.span();

        var access_mask: w.ULONG = w.READ_CONTROL | w.FILE_WRITE_ATTRIBUTES | w.SYNCHRONIZE;
        const write_flags: w.ULONG = if (fdflags.append) w.FILE_APPEND_DATA else w.GENERIC_WRITE;
        if (rights.fd_read and rights.fd_write) {
            access_mask |= w.GENERIC_READ | write_flags;
        } else if (rights.fd_write) {
            access_mask |= write_flags;
        } else {
            access_mask |= w.GENERIC_READ | write_flags;
        }

        const creation: w.ULONG = if (openflags.creat) w.FILE_CREATE else w.FILE_OPEN;

        const file_or_dir_flag: w.ULONG = if (openflags.directory) w.FILE_DIRECTORY_FILE else w.FILE_NON_DIRECTORY_FILE;
        const io_mode_flag: w.ULONG = w.FILE_SYNCHRONOUS_IO_NONALERT;
        const reparse_flags: w.ULONG = if (lookupflags.symlink_follow) 0 else w.FILE_OPEN_REPARSE_POINT;
        const flags: w.ULONG = file_or_dir_flag | io_mode_flag | reparse_flags;

        if (path_w.len > std.math.maxInt(u16)) {
            errno.* = Errno.NAMETOOLONG;
            return null;
        }

        const path_len_bytes = @as(u16, @intCast(path_w.len * 2));
        var unicode_str = w.UNICODE_STRING{
            .Length = path_len_bytes,
            .MaximumLength = path_len_bytes,
            .Buffer = @as([*]u16, @ptrFromInt(@intFromPtr(path_w.ptr))),
        };
        var attr = w.OBJECT_ATTRIBUTES{
            .Length = @sizeOf(w.OBJECT_ATTRIBUTES),
            .RootDirectory = null,
            .Attributes = w.OBJ_CASE_INSENSITIVE,
            .ObjectName = &unicode_str,
            .SecurityDescriptor = null,
            .SecurityQualityOfService = null,
        };

        var fd: w.HANDLE = undefined;
        var io: w.IO_STATUS_BLOCK = undefined;
        const rc = std.os.windows.ntdll.NtCreateFile(
            &fd,
            access_mask,
            &attr,
            &io,
            null,
            w.FILE_ATTRIBUTE_NORMAL,
            w.FILE_SHARE_WRITE | w.FILE_SHARE_READ | w.FILE_SHARE_DELETE,
            creation,
            flags,
            null,
            0,
        );

        // emulate the posix behavior on windows
        if (lookupflags.symlink_follow == false) {
            if (rc != .OBJECT_NAME_INVALID) {
                const attributes: w.DWORD = w.GetFileAttributesW(path_w) catch 0;
                if (windowsFileAttributeToWasiFiletype(attributes) == .SYMBOLIC_LINK) {
                    if (openflags.directory) {
                        errno.* = Errno.NOTDIR;
                    } else {
                        errno.* = Errno.LOOP;
                    }
                    if (rc == .SUCCESS) {
                        std.os.close(fd);
                    }
                    return null;
                }
            }
        }

        switch (rc) {
            .SUCCESS => return fd,
            .INVALID_PARAMETER => unreachable,
            .OBJECT_PATH_SYNTAX_BAD => unreachable,
            .INVALID_HANDLE => unreachable,
            .OBJECT_NAME_NOT_FOUND => errno.* = Errno.NOENT,
            .OBJECT_NAME_INVALID => errno.* = Errno.NOENT,
            .OBJECT_PATH_NOT_FOUND => errno.* = Errno.NOENT,
            .NO_MEDIA_IN_DEVICE => errno.* = Errno.NODEV,
            .SHARING_VIOLATION => errno.* = Errno.ACCES,
            .ACCESS_DENIED => errno.* = Errno.ACCES,
            .USER_MAPPED_FILE => errno.* = Errno.ACCES,
            .PIPE_BUSY => errno.* = Errno.BUSY,
            .OBJECT_NAME_COLLISION => errno.* = Errno.EXIST,
            .FILE_IS_A_DIRECTORY => errno.* = Errno.ISDIR,
            .NOT_A_DIRECTORY => errno.* = Errno.NOTDIR,
            else => {
                errno.* = Errno.INVAL;
            },
        }

        return null;
    }

    fn openPathPosix(path: []const u8, lookupflags: WasiLookupFlags, openflags: WasiOpenFlags, fdflags: WasiFdFlags, rights: WasiRights, errno: *Errno) ?std.os.fd_t {
        if (builtin.os.tag == .windows) {
            @compileError("This function should only be called on an OS that supports posix APIs.");
        }

        var flags: u32 = 0;
        if (openflags.creat) {
            flags |= std.os.O.CREAT;
        }
        if (openflags.directory) {
            flags |= std.os.O.DIRECTORY;
        }
        if (openflags.excl) {
            flags |= std.os.O.EXCL;
        }
        if (openflags.trunc) {
            flags |= std.os.O.TRUNC;
        }

        if (lookupflags.symlink_follow == false) {
            flags |= std.os.O.NOFOLLOW;
        }

        const fdflags_os = fdflagsToFlagsPosix(fdflags);
        flags |= fdflags_os;

        if (rights.fd_read and rights.fd_write) {
            if (openflags.directory) {
                flags |= std.os.O.RDONLY;
            } else {
                flags |= std.os.O.RDWR;
            }
        } else if (rights.fd_read) {
            flags |= std.os.O.RDONLY;
        } else if (rights.fd_write) {
            flags |= std.os.O.WRONLY;
        }

        const S = std.os.linux.S;
        const mode: std.os.mode_t = S.IRUSR | S.IWUSR | S.IRGRP | S.IWGRP | S.IROTH;
        if (std.os.open(path, flags, mode)) |fd| {
            return fd;
        } else |err| {
            errno.* = Errno.translateError(err);
            return null;
        }
    }

    fn enumerateDirEntries(fd_info: *WasiContext.FdInfo, start_cookie: u64, out_buffer: []u8, errno: *Errno) u32 {
        comptime std.debug.assert(std.os.wasi.DIRCOOKIE_START == 0);
        var restart_scan = (start_cookie == 0);

        if (restart_scan) {
            fd_info.dir_entries.clearRetainingCapacity();
        }

        const osFunc = switch (builtin.os.tag) {
            .windows => Helpers.enumerateDirEntriesWindows,
            .linux => Helpers.enumerateDirEntriesLinux,
            else => comptime blk: {
                if (builtin.os.tag.isDarwin()) {
                    break :blk Helpers.enumerateDirEntriesDarwin;
                }
                unreachable; // TODO add support for this platform
            },
        };

        var file_index = start_cookie;

        var fbs = std.io.fixedBufferStream(out_buffer);
        var writer = fbs.writer();

        while (fbs.pos < fbs.buffer.len and errno.* == .SUCCESS) {
            if (file_index < fd_info.dir_entries.items.len) {
                for (fd_info.dir_entries.items[file_index..]) |entry| {
                    const cookie = file_index + 1;
                    writer.writeIntLittle(u64, cookie) catch break;
                    writer.writeIntLittle(u64, entry.inode) catch break;
                    writer.writeIntLittle(u32, signedCast(u32, entry.filename.len, errno)) catch break;
                    writer.writeIntLittle(u32, @intFromEnum(entry.filetype)) catch break;
                    _ = writer.write(entry.filename) catch break;

                    file_index += 1;
                }
            }

            // load more entries for the next loop iteration
            if (fbs.pos < fbs.buffer.len and errno.* == .SUCCESS) {
                if (osFunc(fd_info, restart_scan, errno) == false) {
                    // no more files or error
                    break;
                }
            }
            restart_scan = false;
        }

        var bytes_written = signedCast(u32, fbs.pos, errno);
        return bytes_written;
    }

    fn enumerateDirEntriesWindows(fd_info: *WasiContext.FdInfo, restart_scan: bool, errno: *Errno) bool {
        comptime std.debug.assert(std.os.wasi.DIRCOOKIE_START == 0);

        const restart_scan_win32: std.os.windows.BOOLEAN = if (restart_scan) std.os.windows.TRUE else std.os.windows.FALSE;

        var file_info_buffer: [1024]u8 align(@alignOf(WindowsApi.FILE_ID_FULL_DIR_INFORMATION)) = undefined;
        var io: std.os.windows.IO_STATUS_BLOCK = undefined;
        var rc: std.os.windows.NTSTATUS = std.os.windows.ntdll.NtQueryDirectoryFile(
            fd_info.fd,
            null,
            null,
            null,
            &io,
            &file_info_buffer,
            file_info_buffer.len,
            .FileIdFullDirectoryInformation,
            std.os.windows.TRUE,
            null,
            restart_scan_win32,
        );
        switch (rc) {
            .SUCCESS => {},
            .NO_MORE_FILES => {
                return false;
            },
            .BUFFER_OVERFLOW => {
                unreachable;
            },
            .INVALID_INFO_CLASS => unreachable,
            .INVALID_PARAMETER => unreachable,
            else => {
                unreachable;
            },
        }

        if (rc == .SUCCESS) {
            const file_info = @as(*WindowsApi.FILE_ID_FULL_DIR_INFORMATION, @ptrCast(&file_info_buffer));

            const filename_utf16le = @as([*]u16, @ptrCast(&file_info.FileName))[0 .. file_info.FileNameLength / @sizeOf(u16)];

            var static_path_buffer: [std.fs.MAX_PATH_BYTES * 2]u8 = undefined;
            var fba = std.heap.FixedBufferAllocator.init(&static_path_buffer);
            var allocator = fba.allocator();
            const filename: []u8 = std.unicode.utf16leToUtf8Alloc(allocator, filename_utf16le) catch unreachable;

            var filetype: std.os.wasi.filetype_t = .REGULAR_FILE;
            if (file_info.FileAttributes & std.os.windows.FILE_ATTRIBUTE_DIRECTORY != 0) {
                filetype = .DIRECTORY;
            } else if (file_info.FileAttributes & std.os.windows.FILE_ATTRIBUTE_REPARSE_POINT != 0) {
                filetype = .SYMBOLIC_LINK;
            }

            var filename_duped = fd_info.dir_entries.allocator.dupe(u8, filename) catch |err| {
                errno.* = Errno.translateError(err);
                return false;
            };

            fd_info.dir_entries.append(WasiDirEntry{
                .inode = @as(u64, @bitCast(file_info.FileId)),
                .filetype = filetype,
                .filename = filename_duped,
            }) catch |err| {
                errno.* = Errno.translateError(err);
                return false;
            };
        }

        return true;
    }

    fn enumerateDirEntriesDarwin(fd_info: *WasiContext.FdInfo, restart_scan: bool, errno: *Errno) bool {
        if (restart_scan) {
            std.os.lseek_SET(fd_info.fd, 0) catch |err| {
                errno.* = Errno.translateError(err);
                return false;
            };
        }

        const dirent_t = std.c.dirent;

        var dirent_buffer: [1024]u8 align(@alignOf(dirent_t)) = undefined;
        var unused_seek: i64 = 0;
        const rc = std.os.system.__getdirentries64(fd_info.fd, &dirent_buffer, dirent_buffer.len, &unused_seek);
        errno.* = switch (std.c.getErrno(rc)) {
            .SUCCESS => .SUCCESS,
            .BADF => .BADF,
            .FAULT => .FAULT,
            .IO => .IO,
            .NOTDIR => .NOTDIR,
            else => .INVAL,
        };

        if (errno.* != .SUCCESS) {
            return false;
        }

        if (rc == 0) {
            return false;
        }

        var buffer_offset: usize = 0;
        while (buffer_offset < rc) {
            const dirent_entry = @as(*align(1) dirent_t, @ptrCast(dirent_buffer[buffer_offset..]));
            buffer_offset += dirent_entry.d_reclen;

            // TODO length should be (d_reclen - 2 - offsetof(dirent64, d_name))
            // const filename: []u8 = std.mem.sliceTo(@ptrCast([*:0]u8, &dirent_entry.d_name), 0);
            const filename: []u8 = @as([*]u8, @ptrCast(&dirent_entry.d_name))[0..dirent_entry.d_namlen];

            const filetype: std.os.wasi.filetype_t = switch (dirent_entry.d_type) {
                std.c.DT.UNKNOWN => .UNKNOWN,
                std.c.DT.FIFO => .UNKNOWN,
                std.c.DT.CHR => .CHARACTER_DEVICE,
                std.c.DT.DIR => .DIRECTORY,
                std.c.DT.BLK => .BLOCK_DEVICE,
                std.c.DT.REG => .REGULAR_FILE,
                std.c.DT.LNK => .SYMBOLIC_LINK,
                std.c.DT.SOCK => .SOCKET_DGRAM,
                std.c.DT.WHT => .UNKNOWN,
                else => .UNKNOWN,
            };

            var filename_duped = fd_info.dir_entries.allocator.dupe(u8, filename) catch |err| {
                errno.* = Errno.translateError(err);
                break;
            };

            fd_info.dir_entries.append(WasiDirEntry{
                .inode = dirent_entry.d_ino,
                .filetype = filetype,
                .filename = filename_duped,
            }) catch |err| {
                errno.* = Errno.translateError(err);
                break;
            };
        }

        return true;
    }

    fn enumerateDirEntriesLinux(fd_info: *WasiContext.FdInfo, restart_scan: bool, errno: *Errno) bool {
        if (restart_scan) {
            std.os.lseek_SET(fd_info.fd, 0) catch |err| {
                errno.* = Errno.translateError(err);
                return false;
            };
        }

        var dirent_buffer: [1024]u8 align(@alignOf(std.os.linux.dirent64)) = undefined;
        const rc = std.os.linux.getdents64(fd_info.fd, &dirent_buffer, dirent_buffer.len);
        errno.* = switch (std.os.linux.getErrno(rc)) {
            .SUCCESS => Errno.SUCCESS,
            .BADF => unreachable, // should never happen since this call is wrapped by fdLookup
            .FAULT => Errno.FAULT,
            .NOTDIR => Errno.NOTDIR,
            .NOENT => Errno.NOENT, // can happen if the fd_info.fd directory was deleted
            else => Errno.INVAL,
        };

        if (errno.* != .SUCCESS) {
            return false;
        }

        if (rc == 0) {
            return false;
        }

        var buffer_offset: usize = 0;
        while (buffer_offset < rc) {
            const dirent_entry = @as(*align(1) std.os.linux.dirent64, @ptrCast(dirent_buffer[buffer_offset..]));
            buffer_offset += dirent_entry.d_reclen;

            // TODO length should be (d_reclen - 2 - offsetof(dirent64, d_name))
            const filename: []u8 = std.mem.sliceTo(@as([*:0]u8, @ptrCast(&dirent_entry.d_name)), 0);

            const filetype: std.os.wasi.filetype_t = switch (dirent_entry.d_type) {
                std.os.linux.DT.BLK => .BLOCK_DEVICE,
                std.os.linux.DT.CHR => .CHARACTER_DEVICE,
                std.os.linux.DT.DIR => .DIRECTORY,
                std.os.linux.DT.FIFO => .UNKNOWN,
                std.os.linux.DT.LNK => .SYMBOLIC_LINK,
                std.os.linux.DT.REG => .REGULAR_FILE,
                std.os.linux.DT.SOCK => .SOCKET_DGRAM, // TODO handle SOCKET_DGRAM
                else => .UNKNOWN,
            };

            var filename_duped = fd_info.dir_entries.allocator.dupe(u8, filename) catch |err| {
                errno.* = Errno.translateError(err);
                break;
            };

            fd_info.dir_entries.append(WasiDirEntry{
                .inode = @as(u64, @bitCast(dirent_entry.d_ino)),
                .filetype = filetype,
                .filename = filename_duped,
            }) catch |err| {
                errno.* = Errno.translateError(err);
                break;
            };
        }

        return true;
    }

    fn initIovecs(comptime iov_type: type, stack_iov: []iov_type, errno: *Errno, module: *ModuleInstance, iovec_array_begin: u32, iovec_array_count: u32) ?[]iov_type {
        if (iovec_array_count < stack_iov.len) {
            const iov = stack_iov[0..iovec_array_count];
            const iovec_array_bytes_length = @sizeOf(u32) * 2 * iovec_array_count;
            if (getMemorySlice(module, iovec_array_begin, iovec_array_bytes_length, errno)) |iovec_mem| {
                var stream = std.io.fixedBufferStream(iovec_mem);
                var reader = stream.reader();

                for (iov) |*iovec| {
                    const iov_base: u32 = reader.readIntLittle(u32) catch {
                        errno.* = Errno.INVAL;
                        return null;
                    };

                    const iov_len: u32 = reader.readIntLittle(u32) catch {
                        errno.* = Errno.INVAL;
                        return null;
                    };

                    if (getMemorySlice(module, iov_base, iov_len, errno)) |mem| {
                        iovec.iov_base = mem.ptr;
                        iovec.iov_len = mem.len;
                    }
                }

                return iov;
            }
        } else {
            errno.* = Errno.TOOBIG;
        }

        return null;
    }
};

fn wasi_proc_exit(_: ?*anyopaque, _: *ModuleInstance, params: [*]const Val, _: [*]Val) void {
    const raw_exit_code = params[0].I32;

    if (raw_exit_code >= 0 and raw_exit_code < std.math.maxInt(u8)) {
        const exit_code = @as(u8, @intCast(raw_exit_code));
        std.os.exit(exit_code);
    } else {
        std.os.exit(1);
    }
}

fn wasi_args_sizes_get(userdata: ?*anyopaque, module: *ModuleInstance, params: [*]const Val, returns: [*]Val) void {
    var context = WasiContext.fromUserdata(userdata);
    Helpers.stringsSizesGet(module, context.argv, params, returns);
}

fn wasi_args_get(userdata: ?*anyopaque, module: *ModuleInstance, params: [*]const Val, returns: [*]Val) void {
    var context = WasiContext.fromUserdata(userdata);
    Helpers.stringsGet(module, context.argv, params, returns);
}

fn wasi_environ_sizes_get(userdata: ?*anyopaque, module: *ModuleInstance, params: [*]const Val, returns: [*]Val) void {
    var context = WasiContext.fromUserdata(userdata);
    Helpers.stringsSizesGet(module, context.env, params, returns);
}

fn wasi_environ_get(userdata: ?*anyopaque, module: *ModuleInstance, params: [*]const Val, returns: [*]Val) void {
    var context = WasiContext.fromUserdata(userdata);
    Helpers.stringsGet(module, context.env, params, returns);
}

fn wasi_clock_res_get(_: ?*anyopaque, module: *ModuleInstance, params: [*]const Val, returns: [*]Val) void {
    var errno = Errno.SUCCESS;

    const system_clockid: i32 = Helpers.convertClockId(params[0].I32, &errno);
    const timestamp_mem_begin = Helpers.signedCast(u32, params[1].I32, &errno);

    if (errno == .SUCCESS) {
        var freqency_ns: u64 = 0;
        if (builtin.os.tag == .windows) {
            // Follow the mingw pattern since clock_getres() isn't linked in libc for windows
            if (system_clockid == std.os.wasi.CLOCK.REALTIME or system_clockid == std.os.wasi.CLOCK.MONOTONIC) {
                const ns_per_second: u64 = 1000000000;
                const tick_frequency: u64 = std.os.windows.QueryPerformanceFrequency();
                freqency_ns = (ns_per_second + (tick_frequency >> 1)) / tick_frequency;
                if (freqency_ns < 1) {
                    freqency_ns = 1;
                }
            } else {
                var timeAdjustment: WindowsApi.DWORD = undefined;
                var timeIncrement: WindowsApi.DWORD = undefined;
                var timeAdjustmentDisabled: WindowsApi.BOOL = undefined;
                if (WindowsApi.GetSystemTimeAdjustment(&timeAdjustment, &timeIncrement, &timeAdjustmentDisabled) == std.os.windows.TRUE) {
                    freqency_ns = timeIncrement * 100;
                } else {
                    errno = Errno.INVAL;
                }
            }
        } else {
            var ts: std.os.system.timespec = undefined;
            if (std.os.clock_getres(system_clockid, &ts)) {
                freqency_ns = @as(u64, @intCast(ts.tv_nsec));
            } else |_| {
                errno = Errno.INVAL;
            }
        }

        Helpers.writeIntToMemory(u64, freqency_ns, timestamp_mem_begin, module, &errno);
    }

    returns[0] = Val{ .I32 = @intFromEnum(errno) };
}

fn wasi_clock_time_get(_: ?*anyopaque, module: *ModuleInstance, params: [*]const Val, returns: [*]Val) void {
    var errno = Errno.SUCCESS;

    const system_clockid: i32 = Helpers.convertClockId(params[0].I32, &errno);
    //const precision = params[1].I64; // unused
    const timestamp_mem_begin = Helpers.signedCast(u32, params[2].I32, &errno);

    if (errno == .SUCCESS) {
        const ns_per_second = 1000000000;
        var timestamp_ns: u64 = 0;

        if (builtin.os.tag == .windows) {
            switch (system_clockid) {
                std.os.wasi.CLOCK.REALTIME => {
                    var ft: WindowsApi.FILETIME = undefined;
                    std.os.windows.kernel32.GetSystemTimeAsFileTime(&ft);

                    timestamp_ns = Helpers.windowsFiletimeToWasi(ft);
                },
                std.os.wasi.CLOCK.MONOTONIC => {
                    const ticks: u64 = std.os.windows.QueryPerformanceCounter();
                    const ticks_per_second: u64 = std.os.windows.QueryPerformanceFrequency();

                    // break up into 2 calculations to avoid overflow
                    const timestamp_secs_part: u64 = ticks / ticks_per_second;
                    const timestamp_ns_part: u64 = ((ticks % ticks_per_second) * ns_per_second + (ticks_per_second >> 1)) / ticks_per_second;

                    timestamp_ns = timestamp_secs_part + timestamp_ns_part;
                },
                std.os.wasi.CLOCK.PROCESS_CPUTIME_ID => {
                    var createTime: WindowsApi.FILETIME = undefined;
                    var exitTime: WindowsApi.FILETIME = undefined;
                    var kernelTime: WindowsApi.FILETIME = undefined;
                    var userTime: WindowsApi.FILETIME = undefined;
                    if (std.os.windows.kernel32.GetProcessTimes(WindowsApi.GetCurrentProcess(), &createTime, &exitTime, &kernelTime, &userTime) == std.os.windows.TRUE) {
                        const timestamp_100ns: u64 = Helpers.filetimeToU64(kernelTime) + Helpers.filetimeToU64(userTime);
                        timestamp_ns = timestamp_100ns * 100;
                    } else {
                        errno = Errno.INVAL;
                    }
                },
                std.os.wasi.CLOCK.THREAD_CPUTIME_ID => {
                    var createTime: WindowsApi.FILETIME = undefined;
                    var exitTime: WindowsApi.FILETIME = undefined;
                    var kernelTime: WindowsApi.FILETIME = undefined;
                    var userTime: WindowsApi.FILETIME = undefined;
                    if (WindowsApi.GetThreadTimes(WindowsApi.GetCurrentProcess(), &createTime, &exitTime, &kernelTime, &userTime) == std.os.windows.TRUE) {
                        const timestamp_100ns: u64 = Helpers.filetimeToU64(kernelTime) + Helpers.filetimeToU64(userTime);
                        timestamp_ns = timestamp_100ns * 100;
                    } else {
                        errno = Errno.INVAL;
                    }
                },
                else => unreachable,
            }
        } else {
            var ts: std.os.system.timespec = undefined;
            if (std.os.clock_gettime(system_clockid, &ts)) {
                timestamp_ns = Helpers.posixTimespecToWasi(ts);
            } else |_| {
                errno = Errno.INVAL;
            }
        }

        Helpers.writeIntToMemory(u64, timestamp_ns, timestamp_mem_begin, module, &errno);
    }

    returns[0] = Val{ .I32 = @intFromEnum(errno) };
}

fn fd_wasi_datasync(userdata: ?*anyopaque, _: *ModuleInstance, params: [*]const Val, returns: [*]Val) void {
    const context = WasiContext.fromUserdata(userdata);
    const fd_wasi = @as(u32, @bitCast(params[0].I32));

    var errno = Errno.SUCCESS;

    if (context.fdLookup(fd_wasi, &errno)) |fd_info| {
        std.os.fdatasync(fd_info.fd) catch |err| {
            errno = Errno.translateError(err);
        };
    }

    returns[0] = Val{ .I32 = @intFromEnum(errno) };
}

fn fd_wasi_fdstat_get(userdata: ?*anyopaque, module: *ModuleInstance, params: [*]const Val, returns: [*]Val) void {
    var errno = Errno.SUCCESS;

    const context = WasiContext.fromUserdata(userdata);
    const fd_wasi = @as(u32, @bitCast(params[0].I32));
    const fdstat_mem_offset = Helpers.signedCast(u32, params[1].I32, &errno);

    if (errno == .SUCCESS) {
        if (context.fdLookup(fd_wasi, &errno)) |fd_info| {
            const fd_os: std.os.fd_t = fd_info.fd;
            const stat: std.os.wasi.fdstat_t = if (builtin.os.tag == .windows) Helpers.fdstatGetWindows(fd_os, &errno) else Helpers.fdstatGetPosix(fd_os, &errno);

            if (errno == .SUCCESS) {
                Helpers.writeIntToMemory(u8, @intFromEnum(stat.fs_filetype), fdstat_mem_offset + 0, module, &errno);
                Helpers.writeIntToMemory(u16, stat.fs_flags, fdstat_mem_offset + 2, module, &errno);
                Helpers.writeIntToMemory(u64, stat.fs_rights_base, fdstat_mem_offset + 8, module, &errno);
                Helpers.writeIntToMemory(u64, stat.fs_rights_inheriting, fdstat_mem_offset + 16, module, &errno);
            }
        }
    }

    returns[0] = Val{ .I32 = @intFromEnum(errno) };
}

fn fd_wasi_fdstat_set_flags(userdata: ?*anyopaque, _: *ModuleInstance, params: [*]const Val, returns: [*]Val) void {
    var errno = Errno.SUCCESS;

    const context = WasiContext.fromUserdata(userdata);
    const fd_wasi = @as(u32, @bitCast(params[0].I32));
    const fdflags: WasiFdFlags = Helpers.decodeFdFlags(params[1].I32);

    if (context.fdLookup(fd_wasi, &errno)) |fd_info| {
        const fdstat_set_flags_func = if (builtin.os.tag == .windows) Helpers.fdstatSetFlagsWindows else Helpers.fdstatSetFlagsPosix;
        if (fdstat_set_flags_func(fd_info, fdflags, &errno)) |new_fd| {
            context.fdUpdate(fd_wasi, new_fd);
        }
    }

    returns[0] = Val{ .I32 = @intFromEnum(errno) };
}

fn fd_wasi_prestat_get(userdata: ?*anyopaque, module: *ModuleInstance, params: [*]const Val, returns: [*]Val) void {
    var errno = Errno.SUCCESS;

    const context = WasiContext.fromUserdata(userdata);
    const fd_dir_wasi = @as(u32, @bitCast(params[0].I32));
    const prestat_mem_offset = Helpers.signedCast(u32, params[1].I32, &errno);

    if (errno == .SUCCESS) {
        if (context.fdDirPath(fd_dir_wasi, &errno)) |path_source| {
            const name_len: u32 = @as(u32, @intCast(path_source.len));

            Helpers.writeIntToMemory(u32, std.os.wasi.PREOPENTYPE_DIR, prestat_mem_offset + 0, module, &errno);
            Helpers.writeIntToMemory(u32, name_len, prestat_mem_offset + @sizeOf(u32), module, &errno);
        }
    }

    returns[0] = Val{ .I32 = @intFromEnum(errno) };
}

fn fd_wasi_prestat_dir_name(userdata: ?*anyopaque, module: *ModuleInstance, params: [*]const Val, returns: [*]Val) void {
    var errno = Errno.SUCCESS;

    const context = WasiContext.fromUserdata(userdata);
    const fd_dir_wasi = Helpers.signedCast(u32, params[0].I32, &errno);
    const path_mem_offset = Helpers.signedCast(u32, params[1].I32, &errno);
    const path_mem_length = Helpers.signedCast(u32, params[2].I32, &errno);

    if (errno == .SUCCESS) {
        if (context.fdDirPath(fd_dir_wasi, &errno)) |path_source| {
            if (Helpers.getMemorySlice(module, path_mem_offset, path_mem_length, &errno)) |path_dest| {
                if (path_source.len <= path_dest.len) {
                    std.mem.copy(u8, path_dest, path_source);

                    // add null terminator if there's room
                    if (path_dest.len > path_source.len) {
                        path_dest[path_source.len] = 0;
                    }
                } else {
                    errno = Errno.NAMETOOLONG;
                }
            }
        }
    }

    returns[0] = Val{ .I32 = @intFromEnum(errno) };
}

fn fd_wasi_read(userdata: ?*anyopaque, module: *ModuleInstance, params: [*]const Val, returns: [*]Val) void {
    var errno = Errno.SUCCESS;

    var context = WasiContext.fromUserdata(userdata);
    const fd_wasi = @as(u32, @bitCast(params[0].I32));
    const iovec_array_begin = Helpers.signedCast(u32, params[1].I32, &errno);
    const iovec_array_count = Helpers.signedCast(u32, params[2].I32, &errno);
    const bytes_read_out_offset = Helpers.signedCast(u32, params[3].I32, &errno);

    if (errno == .SUCCESS) {
        if (context.fdLookup(fd_wasi, &errno)) |fd_info| {
            var stack_iov = [_]std.os.iovec{undefined} ** 1024;
            if (Helpers.initIovecs(std.os.iovec, &stack_iov, &errno, module, iovec_array_begin, iovec_array_count)) |iov| {
                if (std.os.readv(fd_info.fd, iov)) |read_bytes| {
                    if (read_bytes <= std.math.maxInt(u32)) {
                        Helpers.writeIntToMemory(u32, @as(u32, @intCast(read_bytes)), bytes_read_out_offset, module, &errno);
                    } else {
                        errno = Errno.TOOBIG;
                    }
                } else |err| {
                    errno = Errno.translateError(err);
                }
            }
        }
    }

    returns[0] = Val{ .I32 = @intFromEnum(errno) };
}

fn fd_wasi_readdir(userdata: ?*anyopaque, module: *ModuleInstance, params: [*]const Val, returns: [*]Val) void {
    var errno = Errno.SUCCESS;

    var context = WasiContext.fromUserdata(userdata);
    const fd_wasi = @as(u32, @bitCast(params[0].I32));
    const dirent_mem_offset = Helpers.signedCast(u32, params[1].I32, &errno);
    const dirent_mem_length = Helpers.signedCast(u32, params[2].I32, &errno);
    const cookie = Helpers.signedCast(u64, params[3].I64, &errno);
    const bytes_written_out_offset = Helpers.signedCast(u32, params[4].I32, &errno);

    if (errno == .SUCCESS) {
        if (context.fdLookup(fd_wasi, &errno)) |fd_info| {
            if (Helpers.getMemorySlice(module, dirent_mem_offset, dirent_mem_length, &errno)) |dirent_buffer| {
                var bytes_written = Helpers.enumerateDirEntries(fd_info, cookie, dirent_buffer, &errno);
                Helpers.writeIntToMemory(u32, bytes_written, bytes_written_out_offset, module, &errno);
            }
        }
    }

    returns[0] = Val{ .I32 = @intFromEnum(errno) };
}

fn fd_wasi_renumber(userdata: ?*anyopaque, _: *ModuleInstance, params: [*]const Val, returns: [*]Val) void {
    var errno = Errno.SUCCESS;

    var context = WasiContext.fromUserdata(userdata);
    const fd_wasi = @as(u32, @bitCast(params[0].I32));
    const fd_to_wasi = @as(u32, @bitCast(params[1].I32));

    context.fdRenumber(fd_wasi, fd_to_wasi, &errno);

    returns[0] = Val{ .I32 = @intFromEnum(errno) };
}

fn fd_wasi_pread(userdata: ?*anyopaque, module: *ModuleInstance, params: [*]const Val, returns: [*]Val) void {
    var errno = Errno.SUCCESS;

    var context = WasiContext.fromUserdata(userdata);
    const fd_wasi = @as(u32, @bitCast(params[0].I32));
    const iovec_array_begin = Helpers.signedCast(u32, params[1].I32, &errno);
    const iovec_array_count = Helpers.signedCast(u32, params[2].I32, &errno);
    const read_offset = @as(u64, @bitCast(params[3].I64));
    const bytes_read_out_offset = Helpers.signedCast(u32, params[4].I32, &errno);

    if (errno == .SUCCESS) {
        if (context.fdLookup(fd_wasi, &errno)) |fd_info| {
            var stack_iov = [_]std.os.iovec{undefined} ** 1024;
            if (Helpers.initIovecs(std.os.iovec, &stack_iov, &errno, module, iovec_array_begin, iovec_array_count)) |iov| {
                if (std.os.preadv(fd_info.fd, iov, read_offset)) |read_bytes| {
                    if (read_bytes <= std.math.maxInt(u32)) {
                        Helpers.writeIntToMemory(u32, @as(u32, @intCast(read_bytes)), bytes_read_out_offset, module, &errno);
                    } else {
                        errno = Errno.TOOBIG;
                    }
                } else |err| {
                    errno = Errno.translateError(err);
                }
            }
        }
    }

    returns[0] = Val{ .I32 = @intFromEnum(errno) };
}

fn fd_wasi_advise(userdata: ?*anyopaque, _: *ModuleInstance, params: [*]const Val, returns: [*]Val) void {
    var errno = Errno.SUCCESS;

    var context = WasiContext.fromUserdata(userdata);

    const fd_wasi = @as(u32, @bitCast(params[0].I32));
    const offset: i64 = params[1].I64;
    const length: i64 = params[2].I64;
    const advice_wasi = @as(u32, @bitCast(params[3].I32));

    if (Helpers.isStdioHandle(fd_wasi) == false) {
        if (context.fdLookup(fd_wasi, &errno)) |fd_info| {
            // fadvise isn't available on windows or macos, but fadvise is just an optimization hint, so don't
            // return a bad error code
            if (builtin.os.tag == .linux) {
                const advice: usize = switch (advice_wasi) {
                    std.os.wasi.ADVICE_NORMAL => std.os.POSIX_FADV.NORMAL,
                    std.os.wasi.ADVICE_SEQUENTIAL => std.os.POSIX_FADV.SEQUENTIAL,
                    std.os.wasi.ADVICE_RANDOM => std.os.POSIX_FADV.RANDOM,
                    std.os.wasi.ADVICE_WILLNEED => std.os.POSIX_FADV.WILLNEED,
                    std.os.wasi.ADVICE_DONTNEED => std.os.POSIX_FADV.DONTNEED,
                    std.os.wasi.ADVICE_NOREUSE => std.os.POSIX_FADV.NOREUSE,
                    else => blk: {
                        errno = Errno.INVAL;
                        break :blk 0;
                    },
                };

                if (errno == .SUCCESS) {
                    const ret = @as(std.os.linux.E, @enumFromInt(std.os.system.fadvise(fd_info.fd, offset, length, advice)));
                    errno = switch (ret) {
                        .SUCCESS => Errno.SUCCESS,
                        .SPIPE => Errno.SPIPE,
                        .INVAL => Errno.INVAL,
                        .BADF => unreachable, // should never happen since we protect against this in fdLookup
                        else => unreachable,
                    };
                }
            }
        }
    } else {
        errno = Errno.BADF;
    }

    returns[0] = Val{ .I32 = @intFromEnum(errno) };
}

fn fd_wasi_allocate(userdata: ?*anyopaque, _: *ModuleInstance, params: [*]const Val, returns: [*]Val) void {
    var errno = Errno.SUCCESS;

    var context = WasiContext.fromUserdata(userdata);

    const fd_wasi = @as(u32, @bitCast(params[0].I32));
    const offset: i64 = params[1].I64;
    const length_relative: i64 = params[2].I64;

    if (context.fdLookup(fd_wasi, &errno)) |fd_info| {
        if (builtin.os.tag == .windows) {
            const stat: std.os.wasi.filestat_t = Helpers.filestatGetWindows(fd_info.fd, &errno);
            if (errno == .SUCCESS) {
                if (stat.size < offset + length_relative) {
                    const length_total: u64 = @as(u64, @intCast(offset + length_relative));
                    std.os.windows.SetFilePointerEx_BEGIN(fd_info.fd, length_total) catch |err| {
                        errno = Errno.translateError(err);
                    };

                    if (errno == .SUCCESS) {
                        if (WindowsApi.SetEndOfFile(fd_info.fd) != std.os.windows.TRUE) {
                            errno = Errno.INVAL;
                        }
                    }
                }
            }
        } else if (builtin.os.tag == .linux) {
            const mode = 0;
            const rc = std.os.linux.fallocate(fd_info.fd, mode, offset, length_relative);
            errno = switch (std.os.linux.getErrno(rc)) {
                .SUCCESS => Errno.SUCCESS,
                .BADF => unreachable, // should never happen since this call is wrapped by fdLookup
                .FBIG => Errno.FBIG,
                .INTR => Errno.INTR,
                .IO => Errno.IO,
                .NODEV => Errno.NODEV,
                .NOSPC => Errno.NOSPC,
                .NOSYS => Errno.NOSYS,
                .OPNOTSUPP => Errno.NOTSUP,
                .PERM => Errno.PERM,
                .SPIPE => Errno.SPIPE,
                .TXTBSY => Errno.TXTBSY,
                else => Errno.INVAL,
            };
        } else if (builtin.os.tag.isDarwin()) {
            var stat: std.c.Stat = undefined;
            if (std.c.fstat(fd_info.fd, &stat) != -1) {
                // fallocate() doesn't truncate the file if the total is less than the actual file length
                // so we need to emulate that behavior here
                const length_total = @as(u64, @intCast(@as(i128, offset) + length_relative));
                if (stat.size < length_total) {
                    std.os.ftruncate(fd_info.fd, length_total) catch |err| {
                        errno = Errno.translateError(err);
                    };
                }
            }
        } else {
            unreachable; // TODO implement support for this platform
        }
    }

    returns[0] = Val{ .I32 = @intFromEnum(errno) };
}

fn fd_wasi_close(userdata: ?*anyopaque, _: *ModuleInstance, params: [*]const Val, returns: [*]Val) void {
    var errno = Errno.SUCCESS;

    var context = WasiContext.fromUserdata(userdata);

    const fd_wasi = @as(u32, @bitCast(params[0].I32));

    if (errno == .SUCCESS) {
        context.fdClose(fd_wasi, &errno);
    }

    returns[0] = Val{ .I32 = @intFromEnum(errno) };
}

fn fd_wasi_filestat_get(userdata: ?*anyopaque, module: *ModuleInstance, params: [*]const Val, returns: [*]Val) void {
    var errno = Errno.SUCCESS;

    const context = WasiContext.fromUserdata(userdata);
    const fd_wasi = @as(u32, @bitCast(params[0].I32));
    const filestat_out_mem_offset = Helpers.signedCast(u32, params[1].I32, &errno);

    if (errno == .SUCCESS) {
        if (Helpers.isStdioHandle(fd_wasi)) {
            const zeroes = std.mem.zeroes(std.os.wasi.filestat_t);
            Helpers.writeFilestatToMemory(&zeroes, filestat_out_mem_offset, module, &errno);
        } else {
            if (context.fdLookup(fd_wasi, &errno)) |fd_info| {
                const stat: std.os.wasi.filestat_t = if (builtin.os.tag == .windows) Helpers.filestatGetWindows(fd_info.fd, &errno) else Helpers.filestatGetPosix(fd_info.fd, &errno);
                if (errno == .SUCCESS) {
                    Helpers.writeFilestatToMemory(&stat, filestat_out_mem_offset, module, &errno);
                }
            }
        }
    }

    returns[0] = Val{ .I32 = @intFromEnum(errno) };
}

fn fd_wasi_filestat_set_size(userdata: ?*anyopaque, _: *ModuleInstance, params: [*]const Val, returns: [*]Val) void {
    var errno = Errno.SUCCESS;

    const context = WasiContext.fromUserdata(userdata);
    const fd_wasi = @as(u32, @bitCast(params[0].I32));
    const size = Helpers.signedCast(u64, params[1].I64, &errno);

    if (errno == .SUCCESS) {
        if (Helpers.isStdioHandle(fd_wasi)) {
            errno = Errno.BADF;
        } else if (context.fdLookup(fd_wasi, &errno)) |fd_info| {
            std.os.ftruncate(fd_info.fd, size) catch |err| {
                errno = Errno.translateError(err);
            };
        }
    }

    returns[0] = Val{ .I32 = @intFromEnum(errno) };
}

fn fd_wasi_filestat_set_times(userdata: ?*anyopaque, _: *ModuleInstance, params: [*]const Val, returns: [*]Val) void {
    var errno = Errno.SUCCESS;

    const context = WasiContext.fromUserdata(userdata);
    const fd_wasi = @as(u32, @bitCast(params[0].I32));
    const timestamp_wasi_access = Helpers.signedCast(u64, params[1].I64, &errno);
    const timestamp_wasi_modified = Helpers.signedCast(u64, params[2].I64, &errno);
    const fstflags = @as(u32, @bitCast(params[3].I32));

    if (errno == .SUCCESS) {
        if (fstflags & std.os.wasi.FILESTAT_SET_ATIM != 0 and fstflags & std.os.wasi.FILESTAT_SET_ATIM_NOW != 0) {
            errno = Errno.INVAL;
        }

        if (fstflags & std.os.wasi.FILESTAT_SET_MTIM != 0 and fstflags & std.os.wasi.FILESTAT_SET_MTIM_NOW != 0) {
            errno = Errno.INVAL;
        }
    }

    if (errno == .SUCCESS) {
        if (Helpers.isStdioHandle(fd_wasi)) {
            errno = Errno.BADF;
        } else if (context.fdLookup(fd_wasi, &errno)) |fd_info| {
            const fd_filestat_set_times_func = if (builtin.os.tag == .windows) Helpers.fdFilestatSetTimesWindows else Helpers.fdFilestatSetTimesPosix;
            fd_filestat_set_times_func(fd_info.fd, timestamp_wasi_access, timestamp_wasi_modified, fstflags, &errno);
        }
    }

    returns[0] = Val{ .I32 = @intFromEnum(errno) };
}

fn fd_wasi_seek(userdata: ?*anyopaque, module: *ModuleInstance, params: [*]const Val, returns: [*]Val) void {
    var errno = Errno.SUCCESS;

    var context = WasiContext.fromUserdata(userdata);
    const fd_wasi = @as(u32, @bitCast(params[0].I32));
    const offset = params[1].I64;
    const whence_raw = params[2].I32;
    const filepos_out_offset = Helpers.signedCast(u32, params[3].I32, &errno);

    if (errno == .SUCCESS) {
        if (context.fdLookup(fd_wasi, &errno)) |fd_info| {
            if (fd_info.rights.fd_seek) {
                const fd_os: std.os.fd_t = fd_info.fd;
                if (Whence.fromInt(whence_raw)) |whence| {
                    switch (whence) {
                        .Set => {
                            if (offset >= 0) {
                                const offset_unsigned = @as(u64, @intCast(offset));
                                std.os.lseek_SET(fd_os, offset_unsigned) catch |err| {
                                    errno = Errno.translateError(err);
                                };
                            }
                        },
                        .Cur => {
                            std.os.lseek_CUR(fd_os, offset) catch |err| {
                                errno = Errno.translateError(err);
                            };
                        },
                        .End => {
                            std.os.lseek_END(fd_os, offset) catch |err| {
                                errno = Errno.translateError(err);
                            };
                        },
                    }

                    if (std.os.lseek_CUR_get(fd_os)) |filepos| {
                        Helpers.writeIntToMemory(u64, filepos, filepos_out_offset, module, &errno);
                    } else |err| {
                        errno = Errno.translateError(err);
                    }
                } else {
                    errno = Errno.INVAL;
                }
            } else {
                errno = Errno.ISDIR;
            }
        }
    }

    returns[0] = Val{ .I32 = @intFromEnum(errno) };
}

fn fd_wasi_tell(userdata: ?*anyopaque, module: *ModuleInstance, params: [*]const Val, returns: [*]Val) void {
    var errno = Errno.SUCCESS;

    const context = WasiContext.fromUserdata(userdata);

    const fd_wasi = @as(u32, @bitCast(params[0].I32));
    const filepos_out_offset = Helpers.signedCast(u32, params[1].I32, &errno);

    if (errno == .SUCCESS) {
        if (context.fdLookup(fd_wasi, &errno)) |fd_info| {
            if (std.os.lseek_CUR_get(fd_info.fd)) |filepos| {
                Helpers.writeIntToMemory(u64, filepos, filepos_out_offset, module, &errno);
            } else |err| {
                errno = Errno.translateError(err);
            }
        }
    }

    returns[0] = Val{ .I32 = @intFromEnum(errno) };
}

fn fd_wasi_write(userdata: ?*anyopaque, module: *ModuleInstance, params: [*]const Val, returns: [*]Val) void {
    var errno = Errno.SUCCESS;

    var context = WasiContext.fromUserdata(userdata);
    const fd_wasi = @as(u32, @bitCast(params[0].I32));
    const iovec_array_begin = Helpers.signedCast(u32, params[1].I32, &errno);
    const iovec_array_count = Helpers.signedCast(u32, params[2].I32, &errno);
    const bytes_written_out_offset = Helpers.signedCast(u32, params[3].I32, &errno);

    if (errno == .SUCCESS) {
        if (context.fdLookup(fd_wasi, &errno)) |fd_info| {
            var stack_iov = [_]std.os.iovec_const{undefined} ** 1024;
            if (Helpers.initIovecs(std.os.iovec_const, &stack_iov, &errno, module, iovec_array_begin, iovec_array_count)) |iov| {
                if (std.os.writev(fd_info.fd, iov)) |written_bytes| {
                    Helpers.writeIntToMemory(u32, @as(u32, @intCast(written_bytes)), bytes_written_out_offset, module, &errno);
                } else |err| {
                    errno = Errno.translateError(err);
                }
            }
        }
    }

    returns[0] = Val{ .I32 = @intFromEnum(errno) };
}

fn fd_wasi_pwrite(userdata: ?*anyopaque, module: *ModuleInstance, params: [*]const Val, returns: [*]Val) void {
    var errno = Errno.SUCCESS;

    var context = WasiContext.fromUserdata(userdata);
    const fd_wasi = @as(u32, @bitCast(params[0].I32));
    const iovec_array_begin = Helpers.signedCast(u32, params[1].I32, &errno);
    const iovec_array_count = Helpers.signedCast(u32, params[2].I32, &errno);
    const write_offset = Helpers.signedCast(u64, params[3].I64, &errno);
    const bytes_written_out_offset = Helpers.signedCast(u32, params[4].I32, &errno);

    if (errno == .SUCCESS) {
        if (context.fdLookup(fd_wasi, &errno)) |fd_info| {
            var stack_iov = [_]std.os.iovec_const{undefined} ** 1024;
            if (Helpers.initIovecs(std.os.iovec_const, &stack_iov, &errno, module, iovec_array_begin, iovec_array_count)) |iov| {
                if (std.os.pwritev(fd_info.fd, iov, write_offset)) |written_bytes| {
                    Helpers.writeIntToMemory(u32, @as(u32, @intCast(written_bytes)), bytes_written_out_offset, module, &errno);
                } else |err| {
                    errno = Errno.translateError(err);
                }
            }
        }
    }

    returns[0] = Val{ .I32 = @intFromEnum(errno) };
}

fn wasi_path_create_directory(userdata: ?*anyopaque, module: *ModuleInstance, params: [*]const Val, returns: [*]Val) void {
    var errno = Errno.SUCCESS;

    const context = WasiContext.fromUserdata(userdata);
    const fd_dir_wasi = @as(u32, @bitCast(params[0].I32));
    const path_mem_offset: u32 = Helpers.signedCast(u32, params[1].I32, &errno);
    const path_mem_length: u32 = Helpers.signedCast(u32, params[2].I32, &errno);

    if (errno == .SUCCESS) {
        if (context.fdLookup(fd_dir_wasi, &errno)) |fd_info| {
            if (Helpers.getMemorySlice(module, path_mem_offset, path_mem_length, &errno)) |path| {
                if (context.hasPathAccess(fd_info, path, &errno)) {
                    const S = std.os.linux.S;
                    const mode: std.os.mode_t = if (builtin.os.tag == .windows) undefined else S.IRWXU | S.IRWXG | S.IROTH;
                    std.os.mkdirat(fd_info.fd, path, mode) catch |err| {
                        errno = Errno.translateError(err);
                    };
                }
            }
        }
    }

    returns[0] = Val{ .I32 = @intFromEnum(errno) };
}

fn wasi_path_filestat_get(userdata: ?*anyopaque, module: *ModuleInstance, params: [*]const Val, returns: [*]Val) void {
    var errno = Errno.SUCCESS;

    const context = WasiContext.fromUserdata(userdata);
    const fd_dir_wasi = @as(u32, @bitCast(params[0].I32));
    const lookup_flags: WasiLookupFlags = Helpers.decodeLookupFlags(params[1].I32);
    const path_mem_offset: u32 = Helpers.signedCast(u32, params[2].I32, &errno);
    const path_mem_length: u32 = Helpers.signedCast(u32, params[3].I32, &errno);
    const filestat_out_mem_offset = Helpers.signedCast(u32, params[4].I32, &errno);

    if (errno == .SUCCESS) {
        if (context.fdLookup(fd_dir_wasi, &errno)) |fd_info| {
            if (Helpers.getMemorySlice(module, path_mem_offset, path_mem_length, &errno)) |path| {
                if (context.hasPathAccess(fd_info, path, &errno)) {
                    var flags: u32 = std.os.O.RDONLY;
                    if (lookup_flags.symlink_follow == false) {
                        flags |= std.os.O.NOFOLLOW;
                    }

                    const mode: std.os.mode_t = if (builtin.os.tag != .windows) 644 else undefined;

                    if (std.os.openat(fd_info.fd, path, flags, mode)) |fd_opened| {
                        defer std.os.close(fd_opened);

                        const stat: std.os.wasi.filestat_t = if (builtin.os.tag == .windows) Helpers.filestatGetWindows(fd_opened, &errno) else Helpers.filestatGetPosix(fd_opened, &errno);
                        if (errno == .SUCCESS) {
                            Helpers.writeFilestatToMemory(&stat, filestat_out_mem_offset, module, &errno);
                        }
                    } else |err| {
                        errno = Errno.translateError(err);
                    }
                }
            }
        }
    }

    returns[0] = Val{ .I32 = @intFromEnum(errno) };
}

fn wasi_path_open(userdata: ?*anyopaque, module: *ModuleInstance, params: [*]const Val, returns: [*]Val) void {
    var errno = Errno.SUCCESS;

    var context = WasiContext.fromUserdata(userdata);
    const fd_dir_wasi: u32 = Helpers.signedCast(u32, params[0].I32, &errno);
    const lookupflags: WasiLookupFlags = Helpers.decodeLookupFlags(params[1].I32);
    const path_mem_offset: u32 = Helpers.signedCast(u32, params[2].I32, &errno);
    const path_mem_length: u32 = Helpers.signedCast(u32, params[3].I32, &errno);
    const openflags: WasiOpenFlags = Helpers.decodeOpenFlags(params[4].I32);
    const rights_base: WasiRights = Helpers.decodeRights(params[5].I64);
    // const rights_inheriting: WasiRights = Helpers.decodeRights(params[6].I64);
    const fdflags: WasiFdFlags = Helpers.decodeFdFlags(params[7].I32);
    const fd_out_mem_offset = Helpers.signedCast(u32, params[8].I32, &errno);

    // use pathCreateDirectory if creating a directory
    if (openflags.creat and openflags.directory) {
        errno = Errno.INVAL;
    }

    if (errno == .SUCCESS) {
        if (Helpers.getMemorySlice(module, path_mem_offset, path_mem_length, &errno)) |path| {
            if (context.fdLookup(fd_dir_wasi, &errno)) |fd_info| {
                if (context.hasPathAccess(fd_info, path, &errno)) {
                    var rights_sanitized = rights_base;
                    rights_sanitized.fd_seek = !openflags.directory; // directories don't have seek rights

                    const is_preopen = false;
                    if (context.fdOpen(fd_info, path, lookupflags, openflags, fdflags, rights_sanitized, is_preopen, &errno)) |fd_opened_wasi| {
                        Helpers.writeIntToMemory(u32, fd_opened_wasi, fd_out_mem_offset, module, &errno);
                    }
                }
            }
        }
    }

    returns[0] = Val{ .I32 = @intFromEnum(errno) };
}

fn wasi_path_remove_directory(userdata: ?*anyopaque, module: *ModuleInstance, params: [*]const Val, returns: [*]Val) void {
    var errno = Errno.SUCCESS;

    var context = WasiContext.fromUserdata(userdata);

    const fd_dir_wasi = Helpers.signedCast(u32, params[0].I32, &errno);
    const path_mem_offset = Helpers.signedCast(u32, params[1].I32, &errno);
    const path_mem_length = Helpers.signedCast(u32, params[2].I32, &errno);

    if (errno == .SUCCESS) {
        if (Helpers.getMemorySlice(module, path_mem_offset, path_mem_length, &errno)) |path| {
            if (context.fdLookup(fd_dir_wasi, &errno)) |fd_info| {
                if (context.hasPathAccess(fd_info, path, &errno)) {
                    var static_path_buffer: [std.fs.MAX_PATH_BYTES * 2]u8 = undefined;
                    if (Helpers.resolvePath(fd_info, path, &static_path_buffer, &errno)) |resolved_path| {
                        std.os.unlinkat(FD_OS_INVALID, resolved_path, std.os.AT.REMOVEDIR) catch |err| {
                            errno = Errno.translateError(err);
                        };

                        if (errno == .SUCCESS) {
                            context.fdCleanup(resolved_path);
                        }
                    }
                }
            }
        }
    }

    returns[0] = Val{ .I32 = @intFromEnum(errno) };
}

fn wasi_path_symlink(userdata: ?*anyopaque, module: *ModuleInstance, params: [*]const Val, returns: [*]Val) void {
    var errno = Errno.SUCCESS;

    var context = WasiContext.fromUserdata(userdata);

    const link_contents_mem_offset = Helpers.signedCast(u32, params[0].I32, &errno);
    const link_contents_mem_length = Helpers.signedCast(u32, params[1].I32, &errno);
    const fd_dir_wasi = Helpers.signedCast(u32, params[2].I32, &errno);
    const link_path_mem_offset = Helpers.signedCast(u32, params[3].I32, &errno);
    const link_path_mem_length = Helpers.signedCast(u32, params[4].I32, &errno);

    if (Helpers.getMemorySlice(module, link_contents_mem_offset, link_contents_mem_length, &errno)) |link_contents| {
        if (Helpers.getMemorySlice(module, link_path_mem_offset, link_path_mem_length, &errno)) |link_path| {
            if (errno == .SUCCESS) {
                if (context.fdLookup(fd_dir_wasi, &errno)) |fd_info| {
                    if (context.hasPathAccess(fd_info, link_contents, &errno)) {
                        if (context.hasPathAccess(fd_info, link_path, &errno)) {
                            if (builtin.os.tag == .windows) {
                                var static_path_buffer: [std.fs.MAX_PATH_BYTES * 2]u8 = undefined;
                                if (Helpers.resolvePath(fd_info, link_path, &static_path_buffer, &errno)) |resolved_link_path| {
                                    const w = std.os.windows;

                                    const link_contents_w: w.PathSpace = w.sliceToPrefixedFileW(link_contents) catch |err| blk: {
                                        errno = Errno.translateError(err);
                                        break :blk undefined;
                                    };

                                    const resolved_link_path_w: w.PathSpace = w.sliceToPrefixedFileW(resolved_link_path) catch |err| blk: {
                                        errno = Errno.translateError(err);
                                        break :blk undefined;
                                    };

                                    if (errno == .SUCCESS) {
                                        const flags: w.DWORD = w.SYMBOLIC_LINK_FLAG_ALLOW_UNPRIVILEGED_CREATE;
                                        if (WindowsApi.CreateSymbolicLinkW(resolved_link_path_w.span(), link_contents_w.span(), flags) == w.FALSE) {
                                            errno = Errno.NOTSUP;
                                        }
                                    }
                                }
                            } else {
                                std.os.symlinkat(link_contents, fd_info.fd, link_path) catch |err| {
                                    errno = Errno.translateError(err);
                                };
                            }
                        }
                    }
                }
            }
        }
    }

    returns[0] = Val{ .I32 = @intFromEnum(errno) };
}

fn wasi_path_unlink_file(userdata: ?*anyopaque, module: *ModuleInstance, params: [*]const Val, returns: [*]Val) void {
    var errno = Errno.SUCCESS;

    var context = WasiContext.fromUserdata(userdata);

    const fd_dir_wasi = Helpers.signedCast(u32, params[0].I32, &errno);
    const path_mem_offset = Helpers.signedCast(u32, params[1].I32, &errno);
    const path_mem_length = Helpers.signedCast(u32, params[2].I32, &errno);

    if (errno == .SUCCESS) {
        if (Helpers.getMemorySlice(module, path_mem_offset, path_mem_length, &errno)) |path| {
            if (context.fdLookup(fd_dir_wasi, &errno)) |fd_info| {
                if (context.hasPathAccess(fd_info, path, &errno)) {
                    var static_path_buffer: [std.fs.MAX_PATH_BYTES * 2]u8 = undefined;
                    if (Helpers.resolvePath(fd_info, path, &static_path_buffer, &errno)) |resolved_path| {
                        std.os.unlinkat(FD_OS_INVALID, resolved_path, 0) catch |err| {
                            errno = Errno.translateError(err);
                        };

                        if (errno == .SUCCESS) {
                            context.fdCleanup(resolved_path);
                        }
                    }
                }
            }
        }
    }

    returns[0] = Val{ .I32 = @intFromEnum(errno) };
}

fn wasi_random_get(_: ?*anyopaque, module: *ModuleInstance, params: [*]const Val, returns: [*]Val) void {
    var errno = Errno.SUCCESS;

    const array_begin_offset: u32 = Helpers.signedCast(u32, params[0].I32, &errno);
    const array_length: u32 = Helpers.signedCast(u32, params[1].I32, &errno);

    if (errno == .SUCCESS) {
        if (array_length > 0) {
            if (Helpers.getMemorySlice(module, array_begin_offset, array_length, &errno)) |mem| {
                std.crypto.random.bytes(mem);
            }
        }
    }

    returns[0] = Val{ .I32 = @intFromEnum(errno) };
}

pub const WasiOpts = struct {
    argv: ?[][]const u8 = null,
    env: ?[][]const u8 = null,
    dirs: ?[][]const u8 = null,
};

pub fn initImports(opts: WasiOpts, allocator: std.mem.Allocator) !ModuleImportPackage {
    var context: *WasiContext = try allocator.create(WasiContext);
    errdefer allocator.destroy(context);
    context.* = try WasiContext.init(&opts, allocator);
    errdefer context.deinit();

    var imports: ModuleImportPackage = try ModuleImportPackage.init("wasi_snapshot_preview1", null, context, allocator);

    const void_returns = &[0]ValType{};

    try imports.addHostFunction("args_get", &[_]ValType{ .I32, .I32 }, &[_]ValType{.I32}, wasi_args_get, context);
    try imports.addHostFunction("args_sizes_get", &[_]ValType{ .I32, .I32 }, &[_]ValType{.I32}, wasi_args_sizes_get, context);
    try imports.addHostFunction("clock_res_get", &[_]ValType{ .I32, .I32 }, &[_]ValType{.I32}, wasi_clock_res_get, context);
    try imports.addHostFunction("clock_time_get", &[_]ValType{ .I32, .I64, .I32 }, &[_]ValType{.I32}, wasi_clock_time_get, context);
    try imports.addHostFunction("environ_get", &[_]ValType{ .I32, .I32 }, &[_]ValType{.I32}, wasi_environ_get, context);
    try imports.addHostFunction("environ_sizes_get", &[_]ValType{ .I32, .I32 }, &[_]ValType{.I32}, wasi_environ_sizes_get, context);
    try imports.addHostFunction("fd_advise", &[_]ValType{ .I32, .I64, .I64, .I32 }, &[_]ValType{.I32}, fd_wasi_advise, context);
    try imports.addHostFunction("fd_allocate", &[_]ValType{ .I32, .I64, .I64 }, &[_]ValType{.I32}, fd_wasi_allocate, context);
    try imports.addHostFunction("fd_close", &[_]ValType{.I32}, &[_]ValType{.I32}, fd_wasi_close, context);
    try imports.addHostFunction("fd_datasync", &[_]ValType{.I32}, &[_]ValType{.I32}, fd_wasi_datasync, context);
    try imports.addHostFunction("fd_fdstat_get", &[_]ValType{ .I32, .I32 }, &[_]ValType{.I32}, fd_wasi_fdstat_get, context);
    try imports.addHostFunction("fd_fdstat_set_flags", &[_]ValType{ .I32, .I32 }, &[_]ValType{.I32}, fd_wasi_fdstat_set_flags, context);
    try imports.addHostFunction("fd_filestat_get", &[_]ValType{ .I32, .I32 }, &[_]ValType{.I32}, fd_wasi_filestat_get, context);
    try imports.addHostFunction("fd_filestat_set_size", &[_]ValType{ .I32, .I64 }, &[_]ValType{.I32}, fd_wasi_filestat_set_size, context);
    try imports.addHostFunction("fd_filestat_set_times", &[_]ValType{ .I32, .I64, .I64, .I32 }, &[_]ValType{.I32}, fd_wasi_filestat_set_times, context);
    try imports.addHostFunction("fd_pread", &[_]ValType{ .I32, .I32, .I32, .I64, .I32 }, &[_]ValType{.I32}, fd_wasi_pread, context);
    try imports.addHostFunction("fd_prestat_dir_name", &[_]ValType{ .I32, .I32, .I32 }, &[_]ValType{.I32}, fd_wasi_prestat_dir_name, context);
    try imports.addHostFunction("fd_prestat_get", &[_]ValType{ .I32, .I32 }, &[_]ValType{.I32}, fd_wasi_prestat_get, context);
    try imports.addHostFunction("fd_pwrite", &[_]ValType{ .I32, .I32, .I32, .I64, .I32 }, &[_]ValType{.I32}, fd_wasi_pwrite, context);
    try imports.addHostFunction("fd_read", &[_]ValType{ .I32, .I32, .I32, .I32 }, &[_]ValType{.I32}, fd_wasi_read, context);
    try imports.addHostFunction("fd_readdir", &[_]ValType{ .I32, .I32, .I32, .I64, .I32 }, &[_]ValType{.I32}, fd_wasi_readdir, context);
    try imports.addHostFunction("fd_renumber", &[_]ValType{ .I32, .I32 }, &[_]ValType{.I32}, fd_wasi_renumber, context);
    try imports.addHostFunction("fd_seek", &[_]ValType{ .I32, .I64, .I32, .I32 }, &[_]ValType{.I32}, fd_wasi_seek, context);
    try imports.addHostFunction("fd_tell", &[_]ValType{ .I32, .I32 }, &[_]ValType{.I32}, fd_wasi_tell, context);
    try imports.addHostFunction("fd_write", &[_]ValType{ .I32, .I32, .I32, .I32 }, &[_]ValType{.I32}, fd_wasi_write, context);
    try imports.addHostFunction("path_create_directory", &[_]ValType{ .I32, .I32, .I32 }, &[_]ValType{.I32}, wasi_path_create_directory, context);
    try imports.addHostFunction("path_filestat_get", &[_]ValType{ .I32, .I32, .I32, .I32, .I32 }, &[_]ValType{.I32}, wasi_path_filestat_get, context);
    try imports.addHostFunction("path_open", &[_]ValType{ .I32, .I32, .I32, .I32, .I32, .I64, .I64, .I32, .I32 }, &[_]ValType{.I32}, wasi_path_open, context);
    try imports.addHostFunction("path_remove_directory", &[_]ValType{ .I32, .I32, .I32 }, &[_]ValType{.I32}, wasi_path_remove_directory, context);
    try imports.addHostFunction("path_symlink", &[_]ValType{ .I32, .I32, .I32, .I32, .I32 }, &[_]ValType{.I32}, wasi_path_symlink, context);
    try imports.addHostFunction("path_unlink_file", &[_]ValType{ .I32, .I32, .I32 }, &[_]ValType{.I32}, wasi_path_unlink_file, context);
    try imports.addHostFunction("proc_exit", &[_]ValType{.I32}, void_returns, wasi_proc_exit, context);
    try imports.addHostFunction("random_get", &[_]ValType{ .I32, .I32 }, &[_]ValType{.I32}, wasi_random_get, context);

    return imports;
}

pub fn deinitImports(imports: *ModuleImportPackage) void {
    var context = WasiContext.fromUserdata(imports.userdata);
    context.deinit();
    imports.allocator.destroy(context);

    imports.deinit();
}
