const std = @import("std");

const Build = std.Build;
const LazyPath = Build.LazyPath;
const Step = Build.Step;
const Module = Build.Module;
const ModuleImport = Module.Import;
const CrossTarget = std.zig.CrossTarget;
const CompileStep = std.Build.Step.Compile;

const MACOS_VERSION_MIN = "13.0.0";

const CSources = struct {
    files: std.ArrayListUnmanaged([]const u8),
    b: *Build,

    fn init(b: *Build) CSources {
        return .{
            .files = .empty,
            .b = b,
        };
    }

    fn collect(sources: *CSources, path: []const u8) !void {
        const path_from_root = sources.b.pathFromRoot(path); // ensures if the user is in a subdir the path is correct
        const cwd: std.fs.Dir = sources.b.build_root.handle;
        const dir: std.fs.Dir = try cwd.openDir(path_from_root, .{ .iterate = true });
        var iter: std.fs.Dir.Iterator = dir.iterate();
        while (try iter.next()) |entry| {
            if (entry.kind == .file and std.mem.endsWith(u8, entry.name, ".c")) {
                const filepath = try std.fs.path.resolve(sources.b.allocator, &.{ path, entry.name });
                try sources.files.append(sources.b.allocator, filepath);
            }
        }
    }

    fn deinit(sources: *CSources) void {
        for (sources.files.items) |path| {
            sources.b.allocator.free(path);
        }
        sources.files.deinit(sources.b.allocator);
    }
};

fn pathExists(dir: std.fs.Dir, path: []const u8) bool {
    dir.access(path, .{}) catch {
        return false;
    };
    return true;
}

var HOME_PATH: []const u8 = "";
fn homePath(target: Build.ResolvedTarget, b: *Build) []const u8 {
    if (HOME_PATH.len == 0) {
        var envmap: std.process.EnvMap = std.process.getEnvMap(b.allocator) catch @panic("OOM");
        defer envmap.deinit();

        if (target.result.os.tag == .windows) {
            if (envmap.get("HOMEDRIVE")) |home_drive| {
                if (envmap.get("HOMEPATH")) |home_path| {
                    HOME_PATH = b.pathJoin(&.{ home_drive, home_path });
                }
            }
        } else if (target.result.os.tag.isDarwin()) {
            if (envmap.get("HOME")) |path| {
                HOME_PATH = b.allocator.dupe(u8, path) catch @panic("OOM");
            } else {
                @panic("Unable to find home directory - missing environment variable 'HOME'. Either set this envar or avoid using ~ in the path.");
            }
        } else {
            @panic("Unimplemented"); // unimplemented for the target OS
        }
    }
    return HOME_PATH;
}

fn stageAngleDawnArtifacts(b: *Build, target: Build.ResolvedTarget, update_step: *Step.UpdateSourceFiles, staging_path: []const u8, angle_lib_path: LazyPath, dawn_lib_path: LazyPath) !void {
    if (target.result.os.tag == .windows) {
        update_step.addCopyFileToSource(try angle_lib_path.join(b.allocator, "d3dcompiler_47.dll"), b.pathJoin(&.{ staging_path, "d3dcompiler_47.dll" }));
        update_step.addCopyFileToSource(try angle_lib_path.join(b.allocator, "libEGL.dll"), b.pathJoin(&.{ staging_path, "libEGL.dll" }));
        update_step.addCopyFileToSource(try angle_lib_path.join(b.allocator, "libEGL.dll.lib"), b.pathJoin(&.{ staging_path, "libEGL.dll.lib" }));
        update_step.addCopyFileToSource(try angle_lib_path.join(b.allocator, "libGLESv2.dll"), b.pathJoin(&.{ staging_path, "libGLESv2.dll" }));
        update_step.addCopyFileToSource(try angle_lib_path.join(b.allocator, "libGLESv2.dll.lib"), b.pathJoin(&.{ staging_path, "libGLESv2.dll.lib" }));
        update_step.addCopyFileToSource(try dawn_lib_path.join(b.allocator, "webgpu.dll"), b.pathJoin(&.{ staging_path, "webgpu.dll" }));
        update_step.addCopyFileToSource(try dawn_lib_path.join(b.allocator, "webgpu.lib"), b.pathJoin(&.{ staging_path, "webgpu.lib" }));
    } else if (target.result.os.tag.isDarwin()) {
        update_step.addCopyFileToSource(try angle_lib_path.join(b.allocator, "libEGL.dylib"), b.pathJoin(&.{ staging_path, "libEGL.dylib" }));
        update_step.addCopyFileToSource(try angle_lib_path.join(b.allocator, "libGLESv2.dylib"), b.pathJoin(&.{ staging_path, "libGLESv2.dylib" }));
        update_step.addCopyFileToSource(try dawn_lib_path.join(b.allocator, "libwebgpu.dylib"), b.pathJoin(&.{ staging_path, "libwebgpu.dylib" }));
    }
}

const GenerateWasmBindingsParams = struct {
    exe: *Build.Step.Compile,
    api: []const u8,
    spec_path: []const u8,
    host_bindings_path: []const u8,
    guest_bindings_path: ?[]const u8 = null,
    guest_include_path: ?[]const u8 = null,
    deps: []const *Build.Step = &.{},
};

fn generateWasmBindings(b: *Build, params: GenerateWasmBindingsParams) *Build.Step.UpdateSourceFiles {
    const copy_outputs_to_src: *Build.Step.UpdateSourceFiles = b.addUpdateSourceFiles();

    const run = b.addRunArtifact(params.exe);
    run.addArg(std.mem.join(b.allocator, "", &.{ "--api-name=", params.api }) catch @panic("OOM"));
    run.addPrefixedFileArg("--spec-path=", b.path(params.spec_path));
    const host_bindings_path = run.addPrefixedOutputFileArg("--bindings-path=", params.host_bindings_path);
    copy_outputs_to_src.addCopyFileToSource(host_bindings_path, params.host_bindings_path);
    if (params.guest_bindings_path) |path| {
        const guest_bindings_path = run.addPrefixedOutputFileArg("--guest-stubs-path=", path);
        copy_outputs_to_src.addCopyFileToSource(guest_bindings_path, path);
    }
    if (params.guest_include_path) |path| {
        run.addArg(std.mem.join(b.allocator, "", &.{ "--guest-include-path=", path }) catch @panic("OOM"));
    }

    for (params.deps) |dep| {
        run.step.dependOn(dep);
    }

    copy_outputs_to_src.step.dependOn(&run.step);

    return copy_outputs_to_src;
}

pub fn build(b: *Build) !void {
    const git_version_opt: ?[]const u8 = b.option([]const u8, "version", "Specify the specific git version you want to package") orelse null;

    const cwd = b.build_root.handle;

    const target: Build.ResolvedTarget = b.standardTargetOptions(.{});
    const optimize: std.builtin.OptimizeMode = b.standardOptimizeOption(.{});

    const compile_flag_min_macos_version: []const u8 = b.fmt("-mmacos-version-min={s}", .{MACOS_VERSION_MIN});

    /////////////////////////////////////////////////////////
    // zlib

    var z_sources = CSources.init(b);
    defer z_sources.deinit();
    try z_sources.collect("src/ext/zlib/");

    const z_lib = b.addLibrary(.{
        .linkage = .static,
        .name = "z",
        .root_module = b.createModule(.{
            .target = target,
            .optimize = optimize,
            .link_libc = true,
        }),
    });
    z_lib.addIncludePath(b.path("src/ext/zlib/"));
    z_lib.addCSourceFiles(.{
        .files = z_sources.files.items,
        .flags = &.{
            "-DHAVE_SYS_TYPES_H",
            "-DHAVE_STDINT_H",
            "-DHAVE_STDDEF_H",
            "-DZ_HAVE_UNISTD_H",
        },
    });

    /////////////////////////////////////////////////////////
    // curl - used in orca cli tool

    // Original zig build code MIT licensed from: https://github.com/jiacai2050/zig-curl/blob/main/libs/curl.zig
    const curl_mod = b.createModule(.{
        .target = target,
        .optimize = optimize,
        .link_libc = true,
    });

    var curl_sources = CSources.init(b);
    try curl_sources.collect("src/ext/curl/lib");
    try curl_sources.collect("src/ext/curl/lib/vauth");
    try curl_sources.collect("src/ext/curl/lib/vtls");
    try curl_sources.collect("src/ext/curl/lib/vssh");
    try curl_sources.collect("src/ext/curl/lib/vquic");

    for (curl_sources.files.items) |path| {
        curl_mod.addCSourceFile(.{
            .file = b.path(path),
            .flags = &.{ "-std=gnu89", "-fno-sanitize=undefined" },
        });
    }

    curl_mod.addIncludePath(b.path("src/ext/curl/lib"));
    curl_mod.addIncludePath(b.path("src/ext/curl/include"));
    curl_mod.addIncludePath(b.path("src/ext/zlib"));

    curl_mod.addCMacro("BUILDING_LIBCURL", "1");
    curl_mod.addCMacro("CURL_STATICLIB", "1");
    curl_mod.addCMacro("CURL_DISABLE_LDAP", "1");
    curl_mod.addCMacro("CURL_DISABLE_LDAPS", "1");
    // curl_mod.addCMacro("USE_MBEDTLS", "1");
    curl_mod.addCMacro("CURL_DISABLE_DICT", "1");
    curl_mod.addCMacro("CURL_DISABLE_FILE", "1");
    curl_mod.addCMacro("CURL_DISABLE_FTP", "1");
    curl_mod.addCMacro("CURL_DISABLE_GOPHER", "1");
    curl_mod.addCMacro("CURL_DISABLE_IMAP", "1");
    curl_mod.addCMacro("CURL_DISABLE_MQTT", "1");
    curl_mod.addCMacro("CURL_DISABLE_POP3", "1");
    curl_mod.addCMacro("CURL_DISABLE_RTSP", "1");
    curl_mod.addCMacro("CURL_DISABLE_SMB", "1");
    curl_mod.addCMacro("CURL_DISABLE_SMTP", "1");
    curl_mod.addCMacro("CURL_DISABLE_TELNET", "1");
    curl_mod.addCMacro("CURL_DISABLE_TFTP", "1");
    curl_mod.addCMacro("HAVE_LIBZ", "1");
    curl_mod.addCMacro("HAVE_ZLIB_H", "1");

    if (target.result.os.tag == .windows) {
        curl_mod.addCMacro("USE_WINDOWS_SSPI", "1");
        curl_mod.addCMacro("USE_SCHANNEL", "1");
        curl_mod.addCMacro("OS", "\"win32\"");
        curl_mod.addCMacro("ENABLE_IPV6", "1");
        curl_mod.addCMacro("HAVE_GETHOSTBYNAME_R", "1");

        curl_mod.linkSystemLibrary("ws2_32", .{});
        curl_mod.linkSystemLibrary("wldap32", .{});
        curl_mod.linkSystemLibrary("advapi32", .{});
        curl_mod.linkSystemLibrary("crypt32", .{});
        curl_mod.linkSystemLibrary("gdi32", .{});
        curl_mod.linkSystemLibrary("user32", .{});
        curl_mod.linkSystemLibrary("bcrypt", .{});
    } else {
        curl_mod.addCMacro("CURL_EXTERN_SYMBOL", "__attribute__ ((__visibility__ (\"default\"))");

        if (target.result.os.tag.isDarwin()) {
            curl_mod.addCMacro("OS", "\"mac\"");
        } else if (target.result.os.tag == .linux) {
            curl_mod.addCMacro("OS", "\"Linux\"");
            curl_mod.addCMacro("HAVE_LINUX_TCP_H", "1");
        }

        curl_mod.addCMacro("HAVE_ALARM", "1");
        curl_mod.addCMacro("HAVE_ALLOCA_H", "1");
        curl_mod.addCMacro("HAVE_ARPA_INET_H", "1");
        curl_mod.addCMacro("HAVE_ARPA_TFTP_H", "1");
        curl_mod.addCMacro("HAVE_ASSERT_H", "1");
        curl_mod.addCMacro("HAVE_BASENAME", "1");
        curl_mod.addCMacro("HAVE_BOOL_T", "1");
        curl_mod.addCMacro("HAVE_BUILTIN_AVAILABLE", "1");
        curl_mod.addCMacro("HAVE_CLOCK_GETTIME_MONOTONIC", "1");
        curl_mod.addCMacro("HAVE_DLFCN_H", "1");
        curl_mod.addCMacro("HAVE_ERRNO_H", "1");
        curl_mod.addCMacro("HAVE_FCNTL", "1");
        curl_mod.addCMacro("HAVE_FCNTL_H", "1");
        curl_mod.addCMacro("HAVE_FCNTL_O_NONBLOCK", "1");
        curl_mod.addCMacro("HAVE_FREEADDRINFO", "1");
        curl_mod.addCMacro("HAVE_FTRUNCATE", "1");
        curl_mod.addCMacro("HAVE_GETADDRINFO", "1");
        curl_mod.addCMacro("HAVE_GETEUID", "1");
        curl_mod.addCMacro("HAVE_GETPPID", "1");
        curl_mod.addCMacro("HAVE_GETHOSTBYNAME", "1");
        curl_mod.addCMacro("HAVE_GETHOSTBYNAME_R_6", "1");
        curl_mod.addCMacro("HAVE_GETHOSTNAME", "1");
        curl_mod.addCMacro("HAVE_GETPPID", "1");
        curl_mod.addCMacro("HAVE_GETPROTOBYNAME", "1");
        curl_mod.addCMacro("HAVE_GETPEERNAME", "1");
        curl_mod.addCMacro("HAVE_GETSOCKNAME", "1");
        curl_mod.addCMacro("HAVE_IF_NAMETOINDEX", "1");
        curl_mod.addCMacro("HAVE_GETPWUID", "1");
        curl_mod.addCMacro("HAVE_GETPWUID_R", "1");
        curl_mod.addCMacro("HAVE_GETRLIMIT", "1");
        curl_mod.addCMacro("HAVE_GETTIMEOFDAY", "1");
        curl_mod.addCMacro("HAVE_GMTIME_R", "1");
        curl_mod.addCMacro("HAVE_IFADDRS_H", "1");
        curl_mod.addCMacro("HAVE_INET_ADDR", "1");
        curl_mod.addCMacro("HAVE_INET_PTON", "1");
        curl_mod.addCMacro("HAVE_SA_FAMILY_T", "1");
        curl_mod.addCMacro("HAVE_INTTYPES_H", "1");
        curl_mod.addCMacro("HAVE_IOCTL", "1");
        curl_mod.addCMacro("HAVE_IOCTL_FIONBIO", "1");
        curl_mod.addCMacro("HAVE_IOCTL_SIOCGIFADDR", "1");
        curl_mod.addCMacro("HAVE_LDAP_URL_PARSE", "1");
        curl_mod.addCMacro("HAVE_LIBGEN_H", "1");
        curl_mod.addCMacro("HAVE_IDN2_H", "1");
        curl_mod.addCMacro("HAVE_LL", "1");
        curl_mod.addCMacro("HAVE_LOCALE_H", "1");
        curl_mod.addCMacro("HAVE_LOCALTIME_R", "1");
        curl_mod.addCMacro("HAVE_LONGLONG", "1");
        curl_mod.addCMacro("HAVE_MALLOC_H", "1");
        curl_mod.addCMacro("HAVE_MEMORY_H", "1");
        curl_mod.addCMacro("HAVE_NETDB_H", "1");
        curl_mod.addCMacro("HAVE_NETINET_IN_H", "1");
        curl_mod.addCMacro("HAVE_NETINET_TCP_H", "1");
        curl_mod.addCMacro("HAVE_NET_IF_H", "1");
        curl_mod.addCMacro("HAVE_PIPE", "1");
        curl_mod.addCMacro("HAVE_POLL", "1");
        curl_mod.addCMacro("HAVE_POLL_FINE", "1");
        curl_mod.addCMacro("HAVE_POLL_H", "1");
        curl_mod.addCMacro("HAVE_POSIX_STRERROR_R", "1");
        curl_mod.addCMacro("HAVE_PTHREAD_H", "1");
        curl_mod.addCMacro("HAVE_PWD_H", "1");
        curl_mod.addCMacro("HAVE_RECV", "1");
        curl_mod.addCMacro("HAVE_SELECT", "1");
        curl_mod.addCMacro("HAVE_SEND", "1");
        curl_mod.addCMacro("HAVE_FSETXATTR", "1");
        curl_mod.addCMacro("HAVE_FSETXATTR_5", "1");
        curl_mod.addCMacro("HAVE_SETJMP_H", "1");
        curl_mod.addCMacro("HAVE_SETLOCALE", "1");
        curl_mod.addCMacro("HAVE_SETRLIMIT", "1");
        curl_mod.addCMacro("HAVE_SETSOCKOPT", "1");
        curl_mod.addCMacro("HAVE_SIGACTION", "1");
        curl_mod.addCMacro("HAVE_SIGINTERRUPT", "1");
        curl_mod.addCMacro("HAVE_SIGNAL", "1");
        curl_mod.addCMacro("HAVE_SIGNAL_H", "1");
        curl_mod.addCMacro("HAVE_SIGSETJMP", "1");
        curl_mod.addCMacro("HAVE_SOCKADDR_IN6_SIN6_SCOPE_ID", "1");
        curl_mod.addCMacro("HAVE_SOCKET", "1");
        curl_mod.addCMacro("HAVE_STDBOOL_H", "1");
        curl_mod.addCMacro("HAVE_STDINT_H", "1");
        curl_mod.addCMacro("HAVE_STDIO_H", "1");
        curl_mod.addCMacro("HAVE_STDLIB_H", "1");
        curl_mod.addCMacro("HAVE_STRCASECMP", "1");
        curl_mod.addCMacro("HAVE_STRDUP", "1");
        curl_mod.addCMacro("HAVE_STRERROR_R", "1");
        curl_mod.addCMacro("HAVE_STRINGS_H", "1");
        curl_mod.addCMacro("HAVE_STRING_H", "1");
        curl_mod.addCMacro("HAVE_STRSTR", "1");
        curl_mod.addCMacro("HAVE_STRTOK_R", "1");
        curl_mod.addCMacro("HAVE_STRTOLL", "1");
        curl_mod.addCMacro("HAVE_STRUCT_SOCKADDR_STORAGE", "1");
        curl_mod.addCMacro("HAVE_STRUCT_TIMEVAL", "1");
        curl_mod.addCMacro("HAVE_SYS_IOCTL_H", "1");
        curl_mod.addCMacro("HAVE_SYS_PARAM_H", "1");
        curl_mod.addCMacro("HAVE_SYS_POLL_H", "1");
        curl_mod.addCMacro("HAVE_SYS_RESOURCE_H", "1");
        curl_mod.addCMacro("HAVE_SYS_SELECT_H", "1");
        curl_mod.addCMacro("HAVE_SYS_SOCKET_H", "1");
        curl_mod.addCMacro("HAVE_SYS_STAT_H", "1");
        curl_mod.addCMacro("HAVE_SYS_TIME_H", "1");
        curl_mod.addCMacro("HAVE_SYS_TYPES_H", "1");
        curl_mod.addCMacro("HAVE_SYS_UIO_H", "1");
        curl_mod.addCMacro("HAVE_SYS_UN_H", "1");
        curl_mod.addCMacro("HAVE_TERMIOS_H", "1");
        curl_mod.addCMacro("HAVE_TERMIO_H", "1");
        curl_mod.addCMacro("HAVE_TIME_H", "1");
        curl_mod.addCMacro("HAVE_UNAME", "1");
        curl_mod.addCMacro("HAVE_UNISTD_H", "1");
        curl_mod.addCMacro("HAVE_UTIME", "1");
        curl_mod.addCMacro("HAVE_UTIMES", "1");
        curl_mod.addCMacro("HAVE_UTIME_H", "1");
        curl_mod.addCMacro("HAVE_VARIADIC_MACROS_C99", "1");
        curl_mod.addCMacro("HAVE_VARIADIC_MACROS_GCC", "1");
        curl_mod.addCMacro("RANDOM_FILE", "\"/dev/urandom\"");
        curl_mod.addCMacro("RECV_TYPE_ARG1", "int");
        curl_mod.addCMacro("RECV_TYPE_ARG2", "void *");
        curl_mod.addCMacro("RECV_TYPE_ARG3", "size_t");
        curl_mod.addCMacro("RECV_TYPE_ARG4", "int");
        curl_mod.addCMacro("RECV_TYPE_RETV", "ssize_t");
        curl_mod.addCMacro("SEND_QUAL_ARG2", "const");
        curl_mod.addCMacro("SEND_TYPE_ARG1", "int");
        curl_mod.addCMacro("SEND_TYPE_ARG2", "void *");
        curl_mod.addCMacro("SEND_TYPE_ARG3", "size_t");
        curl_mod.addCMacro("SEND_TYPE_ARG4", "int");
        curl_mod.addCMacro("SEND_TYPE_RETV", "ssize_t");
        curl_mod.addCMacro("SIZEOF_INT", "4");
        curl_mod.addCMacro("SIZEOF_SHORT", "2");
        curl_mod.addCMacro("SIZEOF_LONG", "8");
        curl_mod.addCMacro("SIZEOF_OFF_T", "8");
        curl_mod.addCMacro("SIZEOF_CURL_OFF_T", "8");
        curl_mod.addCMacro("SIZEOF_SIZE_T", "8");
        curl_mod.addCMacro("SIZEOF_TIME_T", "8");
        curl_mod.addCMacro("STDC_HEADERS", "1");
        curl_mod.addCMacro("TIME_WITH_SYS_TIME", "1");
        curl_mod.addCMacro("USE_THREADS_POSIX", "1");
        curl_mod.addCMacro("USE_UNIX_SOCKETS", "1");
        curl_mod.addCMacro("_FILE_OFFSET_BITS", "64");
    }
    const curl_lib = b.addLibrary(.{
        .linkage = .static,
        .name = "curl",
        .root_module = curl_mod,
    });

    /////////////////////////////////////////////////////////
    // Orca CLI tool

    const git_version_tool: []const u8 = blk: {
        if (git_version_opt) |git_version| {
            break :blk try b.allocator.dupe(u8, git_version);
        } else {
            const git_version = b.run(&.{ "git", "rev-parse", "--short", "HEAD" });
            break :blk std.mem.trimRight(u8, git_version, "\n");
        }
    };

    var orca_tool_compile_flags: std.ArrayList([]const u8) = .init(b.allocator);
    defer orca_tool_compile_flags.deinit();
    try orca_tool_compile_flags.append("-DFLAG_IMPLEMENTATION");
    try orca_tool_compile_flags.append("-DOC_NO_APP_LAYER");
    try orca_tool_compile_flags.append("-DOC_BUILD_DLL");
    try orca_tool_compile_flags.append("-DCURL_STATICLIB");
    try orca_tool_compile_flags.append(b.fmt("-DORCA_TOOL_VERSION={s}", .{git_version_tool}));

    if (optimize == .Debug) {
        try orca_tool_compile_flags.append("-DOC_DEBUG");
        try orca_tool_compile_flags.append("-DOC_LOG_COMPILE_DEBUG");
    }

    const orca_tool_exe = b.addExecutable(.{
        .name = "orca_tool",
        .target = target,
        .optimize = optimize,
    });
    orca_tool_exe.addIncludePath(b.path("src/"));
    orca_tool_exe.addIncludePath(b.path("src/tool"));
    orca_tool_exe.addIncludePath(b.path("src/ext/stb"));
    orca_tool_exe.addIncludePath(b.path("src/ext/curl/include"));
    orca_tool_exe.addIncludePath(b.path("src/ext/zlib"));
    orca_tool_exe.addIncludePath(b.path("src/ext/microtar"));

    orca_tool_exe.addCSourceFiles(.{
        .files = &.{"src/tool/main.c"},
        .flags = orca_tool_compile_flags.items,
    });

    orca_tool_exe.linkLibrary(curl_lib);
    orca_tool_exe.linkLibrary(z_lib);
    if (target.result.os.tag == .windows) {
        orca_tool_exe.linkSystemLibrary("shlwapi");
        orca_tool_exe.linkSystemLibrary("shell32");
        orca_tool_exe.linkSystemLibrary("ole32");
        orca_tool_exe.linkSystemLibrary("kernel32");
    } else if (target.result.os.tag.isDarwin()) {
        orca_tool_exe.linkFramework("Cocoa");
        orca_tool_exe.linkFramework("SystemConfiguration");
        orca_tool_exe.linkFramework("CoreFoundation");
        orca_tool_exe.linkFramework("CoreServices");
        orca_tool_exe.linkFramework("SystemConfiguration");
        orca_tool_exe.linkFramework("Security");
    }

    orca_tool_exe.step.dependOn(&curl_lib.step);
    orca_tool_exe.step.dependOn(&z_lib.step);
    orca_tool_exe.linkLibC();

    const build_orca_tool: *Build.Step.InstallArtifact = b.addInstallArtifact(orca_tool_exe, .{});

    const run_orca_tool: *Build.Step.Run = b.addRunArtifact(orca_tool_exe);
    run_orca_tool.step.dependOn(&build_orca_tool.step);
    if (b.args) |args| {
        run_orca_tool.addArgs(args); // forwards args afer -- to orca cli, ex: zig build orca-tool -- update
    }

    const run_tool_step = b.step("orca-tool", "Run the Orca CLI tool");
    run_tool_step.dependOn(&run_orca_tool.step);

    /////////////////////////////////////////////////////////
    // angle build dependencies

    var angle_include_path: LazyPath = .{ .cwd_relative = "" };
    var angle_lib_path: LazyPath = .{ .cwd_relative = "" };
    if (b.option([]const u8, "angle-path", "Specify a local build of the Angle library.")) |user_path| {
        const user_include_path = b.pathJoin(&.{ user_path, "include" });
        const user_lib_path = b.pathJoin(&.{ user_path, "lib" });
        angle_include_path = LazyPath{ .cwd_relative = user_include_path };
        angle_lib_path = LazyPath{ .cwd_relative = user_lib_path };
    } else if (target.result.os.tag == .windows) {
        if (b.lazyDependency("angle-windows-x64", .{})) |angle_dep| {
            angle_include_path = angle_dep.path("include");
            angle_lib_path = angle_dep.path("lib");
        }
    } else if (target.result.os.tag.isDarwin()) {
        if (target.result.cpu.arch == .aarch64) {
            if (b.lazyDependency("angle-mac-arm64", .{})) |angle_dep| {
                angle_include_path = angle_dep.path("include");
                angle_lib_path = angle_dep.path("lib");
            }
        } else {
            if (b.lazyDependency("angle-mac-x64", .{})) |angle_dep| {
                angle_include_path = angle_dep.path("include");
                angle_lib_path = angle_dep.path("lib");
            }
        }
    } else {
        const fail = b.addFail("Angle not configured for this platform.");
        b.getInstallStep().dependOn(&fail.step);
    }

    /////////////////////////////////////////////////////////
    // dawn build dependencies

    var dawn_include_path: LazyPath = .{ .cwd_relative = "" };
    var dawn_lib_path: LazyPath = .{ .cwd_relative = "" };
    if (b.option([]const u8, "dawn-path", "Specify a local build of the Dawn library.")) |user_path| {
        const user_include_path = b.pathJoin(&.{ user_path, "include" });
        const user_lib_path = b.pathJoin(&.{ user_path, "lib" });
        dawn_include_path = LazyPath{ .cwd_relative = user_include_path };
        dawn_lib_path = LazyPath{ .cwd_relative = user_lib_path };
    } else if (target.result.os.tag == .windows) {
        if (b.lazyDependency("dawn-windows-x64", .{})) |dawn_dep| {
            dawn_include_path = dawn_dep.path("include");
            dawn_lib_path = dawn_dep.path("lib");
        }
    } else if (target.result.os.tag.isDarwin()) {
        if (target.result.cpu.arch == .aarch64) {
            if (b.lazyDependency("dawn-mac-arm64", .{})) |dawn_dep| {
                dawn_include_path = dawn_dep.path("include");
                dawn_lib_path = dawn_dep.path("lib");
            }
        } else {
            if (b.lazyDependency("dawn-mac-x64", .{})) |dawn_dep| {
                dawn_include_path = dawn_dep.path("include");
                dawn_lib_path = dawn_dep.path("lib");
            }
        }
    } else {
        const fail = b.addFail("Dawn not configured for this platform.");
        b.getInstallStep().dependOn(&fail.step);
    }

    /////////////////////////////////////////////////////////
    // binding generator

    const bindgen_exe: *Build.Step.Compile = b.addExecutable(.{
        .name = "bindgen",
        .root_module = b.createModule(.{
            .root_source_file = b.path("src/build/bindgen.zig"),
            .target = b.graph.host,
            .optimize = .Debug,
        }),
    });

    const bindgen_install = b.addInstallArtifact(bindgen_exe, .{});

    const bindgen_run: *Build.Step.Run = b.addRunArtifact(bindgen_exe);
    if (b.args) |args| {
        bindgen_run.addArgs(args);
    }
    bindgen_run.step.dependOn(&bindgen_install.step);

    const bindgen_step = b.step("run-bindgen", "Generate wasm bindings from a json spec file");
    bindgen_step.dependOn(&bindgen_run.step);

    /////////////////////////////////////////////////////////
    // Orca runtime and dependencies

    const stage_angle_dawn_src = b.addUpdateSourceFiles();
    stage_angle_dawn_src.addCopyFileToSource(try angle_include_path.join(b.allocator, "EGL/egl.h"), "src/ext/angle/include/EGL/egl.h");
    stage_angle_dawn_src.addCopyFileToSource(try angle_include_path.join(b.allocator, "EGL/eglext.h"), "src/ext/angle/include/EGL/eglext.h");
    stage_angle_dawn_src.addCopyFileToSource(try angle_include_path.join(b.allocator, "EGL/eglext_angle.h"), "src/ext/angle/include/EGL/eglext_angle.h");
    stage_angle_dawn_src.addCopyFileToSource(try angle_include_path.join(b.allocator, "EGL/eglplatform.h"), "src/ext/angle/include/EGL/eglplatform.h");
    stage_angle_dawn_src.addCopyFileToSource(try angle_include_path.join(b.allocator, "GLES/egl.h"), "src/ext/angle/include/GLES/egl.h");
    stage_angle_dawn_src.addCopyFileToSource(try angle_include_path.join(b.allocator, "GLES/gl.h"), "src/ext/angle/include/GLES/gl.h");
    stage_angle_dawn_src.addCopyFileToSource(try angle_include_path.join(b.allocator, "GLES/glext.h"), "src/ext/angle/include/GLES/glext.h");
    stage_angle_dawn_src.addCopyFileToSource(try angle_include_path.join(b.allocator, "GLES/glplatform.h"), "src/ext/angle/include/GLES/glplatform.h");
    stage_angle_dawn_src.addCopyFileToSource(try angle_include_path.join(b.allocator, "GLES2/gl2.h"), "src/ext/angle/include/GLES2/gl2.h");
    stage_angle_dawn_src.addCopyFileToSource(try angle_include_path.join(b.allocator, "GLES2/gl2ext.h"), "src/ext/angle/include/GLES2/gl2ext.h");
    stage_angle_dawn_src.addCopyFileToSource(try angle_include_path.join(b.allocator, "GLES2/gl2ext_angle.h"), "src/ext/angle/include/GLES2/gl2ext_angle.h");
    stage_angle_dawn_src.addCopyFileToSource(try angle_include_path.join(b.allocator, "GLES2/gl2platform.h"), "src/ext/angle/include/GLES2/gl2platform.h");
    stage_angle_dawn_src.addCopyFileToSource(try angle_include_path.join(b.allocator, "GLES3/gl3.h"), "src/ext/angle/include/GLES3/gl3.h");
    stage_angle_dawn_src.addCopyFileToSource(try angle_include_path.join(b.allocator, "GLES3/gl31.h"), "src/ext/angle/include/GLES3/gl31.h");
    stage_angle_dawn_src.addCopyFileToSource(try angle_include_path.join(b.allocator, "GLES3/gl32.h"), "src/ext/angle/include/GLES3/gl32.h");
    stage_angle_dawn_src.addCopyFileToSource(try angle_include_path.join(b.allocator, "GLES3/gl3platform.h"), "src/ext/angle/include/GLES3/gl3platform.h");
    stage_angle_dawn_src.addCopyFileToSource(try angle_include_path.join(b.allocator, "KHR/khrplatform.h"), "src/ext/angle/include/KHR/khrplatform.h");
    stage_angle_dawn_src.addCopyFileToSource(try dawn_include_path.join(b.allocator, "webgpu.h"), "src/ext/dawn/include/webgpu.h");

    // copy angle + dawn libs to output directory
    const stage_angle_dawn_artifacts = b.addUpdateSourceFiles();
    try stageAngleDawnArtifacts(b, target, stage_angle_dawn_artifacts, b.exe_dir, angle_lib_path, dawn_lib_path);

    // generate GLES API spec from OpenGL XML registry
    // TODO port this to C or Zig
    const python_exe_name = if (b.graph.host.result.os.tag == .windows) "python.exe" else "python3";
    const python_gen_gles_spec_run: *Build.Step.Run = b.addSystemCommand(&.{python_exe_name});
    python_gen_gles_spec_run.addArg("scripts/gles_gen.py");
    python_gen_gles_spec_run.addPrefixedFileArg("--spec=", b.path("src/ext/gl.xml"));
    const gles_api_header = python_gen_gles_spec_run.addPrefixedOutputFileArg("--header=", "orca_gl31.h");
    const gles_api_json = python_gen_gles_spec_run.addPrefixedOutputFileArg("--json=", "gles_api.json");
    const gles_api_log = python_gen_gles_spec_run.addPrefixedOutputFileArg("--log=", "gles_gen.log");

    const install_gles_gen_log = b.addInstallFile(gles_api_log, "log/gles_gen.log");

    const stage_gles_api_spec_artifacts = b.addUpdateSourceFiles();
    stage_gles_api_spec_artifacts.step.dependOn(&install_gles_gen_log.step);
    stage_gles_api_spec_artifacts.addCopyFileToSource(gles_api_header, "src/graphics/orca_gl31.h");
    stage_gles_api_spec_artifacts.addCopyFileToSource(gles_api_json, "src/wasmbind/gles_api.json");

    // generate wasm bindings

    const orca_runtime_bindgen_core: *Build.Step.UpdateSourceFiles = generateWasmBindings(b, .{
        .exe = bindgen_exe,
        .api = "core",
        .spec_path = "src/wasmbind/core_api.json",
        .host_bindings_path = "src/wasmbind/core_api_bind_gen.c",
        .guest_bindings_path = "src/wasmbind/core_api_stubs.c",
    });

    const orca_runtime_bindgen_surface: *Build.Step.UpdateSourceFiles = generateWasmBindings(b, .{
        .exe = bindgen_exe,
        .api = "surface",
        .spec_path = "src/wasmbind/surface_api.json",
        .host_bindings_path = "src/wasmbind/surface_api_bind_gen.c",
        .guest_bindings_path = "src/graphics/orca_surface_stubs.c",
        .guest_include_path = "graphics/graphics.h",
    });

    const orca_runtime_bindgen_clock: *Build.Step.UpdateSourceFiles = generateWasmBindings(b, .{
        .exe = bindgen_exe,
        .api = "clock",
        .spec_path = "src/wasmbind/clock_api.json",
        .host_bindings_path = "src/wasmbind/clock_api_bind_gen.c",
        .guest_include_path = "platform/platform_clock.h",
    });

    const orca_runtime_bindgen_io: *Build.Step.UpdateSourceFiles = generateWasmBindings(b, .{
        .exe = bindgen_exe,
        .api = "io",
        .spec_path = "src/wasmbind/io_api.json",
        .host_bindings_path = "src/wasmbind/io_api_bind_gen.c",
        .guest_bindings_path = "src/platform/orca_io_stubs.c",
        .guest_include_path = "platform/platform_io_dialog.h",
    });

    const orca_runtime_bindgen_gles: *Build.Step.UpdateSourceFiles = generateWasmBindings(b, .{
        .exe = bindgen_exe,
        .api = "gles",
        .spec_path = "src/wasmbind/gles_api.json",
        .host_bindings_path = "src/wasmbind/gles_api_bind_gen.c",
        .deps = &.{&stage_gles_api_spec_artifacts.step},
    });

    // wgpu shaders header

    const gen_header_exe: *Build.Step.Compile = b.addExecutable(.{
        .name = "gen_header",
        .root_module = b.createModule(.{
            .root_source_file = b.path("src/build/gen_header_from_files.zig"),
            .target = b.graph.host,
            .optimize = .Debug,
        }),
    });

    const wgpu_shaders: []const []const u8 = &.{
        "src/graphics/wgsl_shaders/common.wgsl",
        "src/graphics/wgsl_shaders/path_setup.wgsl",
        "src/graphics/wgsl_shaders/segment_setup.wgsl",
        "src/graphics/wgsl_shaders/backprop.wgsl",
        "src/graphics/wgsl_shaders/chunk.wgsl",
        "src/graphics/wgsl_shaders/merge.wgsl",
        "src/graphics/wgsl_shaders/balance_workgroups.wgsl",
        "src/graphics/wgsl_shaders/raster.wgsl",
        "src/graphics/wgsl_shaders/blit.wgsl",
        "src/graphics/wgsl_shaders/final_blit.wgsl",
    };

    const run_gen_wgpu_header: *Build.Step.Run = b.addRunArtifact(gen_header_exe);
    for (wgpu_shaders) |path| {
        run_gen_wgpu_header.addPrefixedFileArg("--file=", b.path(path));
    }
    run_gen_wgpu_header.addArg("--namespace=oc_wgsl_");
    run_gen_wgpu_header.addPrefixedDirectoryArg("--root=", b.path(""));
    const wgpu_header_path = run_gen_wgpu_header.addPrefixedOutputFileArg("--output=", "generated_headers/wgpu_renderer_shaders.h");

    const update_wgpu_header: *Build.Step.UpdateSourceFiles = b.addUpdateSourceFiles();
    update_wgpu_header.addCopyFileToSource(wgpu_header_path, "src/graphics/wgpu_renderer_shaders.h");

    // platform lib

    var orca_platform_compile_flags: std.ArrayList([]const u8) = .init(b.allocator);
    defer orca_platform_compile_flags.deinit();
    try orca_platform_compile_flags.append("-std=c11");
    try orca_platform_compile_flags.append("-DOC_BUILD_DLL");
    try orca_platform_compile_flags.append("-D_USE_MATH_DEFINES");
    if (optimize == .Debug) {
        try orca_platform_compile_flags.append("-DOC_DEBUG");
        try orca_platform_compile_flags.append("-DOC_LOG_COMPILE_DEBUG");
    }
    if (target.result.os.tag.isDarwin()) {
        try orca_platform_compile_flags.append(compile_flag_min_macos_version);
    }
    // if (target.result.os.tag == .windows) {
    //     try orca_platform_compile_flags.append("-Wl,--delayload=libEGL.dll");
    //     try orca_platform_compile_flags.append("-Wl,--delayload=libGLESv2.dll");
    //     try orca_platform_compile_flags.append("-Wl,--delayload=webgpu.dll");
    // }

    const orca_platform_lib = b.addLibrary(.{
        .linkage = .dynamic,
        .name = "orca_platform",
        .win32_manifest = b.path("src/app/win32_manifest.manifest"),
        .root_module = b.createModule(.{
            .target = target,
            .optimize = optimize,
        }),
    });

    orca_platform_lib.addIncludePath(b.path("src"));
    orca_platform_lib.addIncludePath(b.path("src/ext"));
    orca_platform_lib.addIncludePath(b.path("src/ext/angle/include"));
    orca_platform_lib.addIncludePath(b.path("src/ext/dawn/include"));
    orca_platform_lib.addLibraryPath(angle_include_path);

    var orca_platform_files: std.ArrayList([]const u8) = .init(b.allocator);
    try orca_platform_files.append("src/orca.c");

    orca_platform_lib.addLibraryPath(angle_lib_path);
    orca_platform_lib.addLibraryPath(dawn_lib_path);

    if (target.result.os.tag == .windows) {
        orca_platform_lib.linkSystemLibrary("user32");
        orca_platform_lib.linkSystemLibrary("opengl32");
        orca_platform_lib.linkSystemLibrary("gdi32");
        orca_platform_lib.linkSystemLibrary("dxgi");
        orca_platform_lib.linkSystemLibrary("dxguid");
        orca_platform_lib.linkSystemLibrary("d3d11");
        orca_platform_lib.linkSystemLibrary("dcomp");
        orca_platform_lib.linkSystemLibrary("shcore");
        orca_platform_lib.linkSystemLibrary("dwmapi");
        orca_platform_lib.linkSystemLibrary("comctl32");
        orca_platform_lib.linkSystemLibrary("ole32");
        orca_platform_lib.linkSystemLibrary("shell32");
        orca_platform_lib.linkSystemLibrary("shlwapi");

        // TODO delay load these the graphics libs
        // orca_platform_lib.linkSystemLibrary("delayimp");
        orca_platform_lib.linkSystemLibrary("libEGL.dll");
        orca_platform_lib.linkSystemLibrary("libGLESv2.dll");
        orca_platform_lib.linkSystemLibrary("webgpu");
    } else if (target.result.os.tag.isDarwin()) {
        orca_platform_lib.linkFramework("Carbon");
        orca_platform_lib.linkFramework("Cocoa");
        orca_platform_lib.linkFramework("Metal");
        orca_platform_lib.linkFramework("QuartzCore");
        orca_platform_lib.linkFramework("UniformTypeIdentifiers");

        orca_platform_lib.linkSystemLibrary2("EGL", .{ .weak = true });
        orca_platform_lib.linkSystemLibrary2("GLESv2", .{ .weak = true });
        orca_platform_lib.linkSystemLibrary2("webgpu", .{ .weak = true });

        try orca_platform_files.append("src/orca.m");
    }

    orca_platform_lib.addCSourceFiles(.{
        .files = orca_platform_files.items,
        .flags = orca_platform_compile_flags.items,
    });

    orca_platform_lib.step.dependOn(&stage_angle_dawn_artifacts.step);
    orca_platform_lib.step.dependOn(&stage_angle_dawn_src.step);

    orca_platform_lib.step.dependOn(&update_wgpu_header.step);

    orca_platform_lib.step.dependOn(&orca_runtime_bindgen_core.step);
    orca_platform_lib.step.dependOn(&orca_runtime_bindgen_surface.step);
    orca_platform_lib.step.dependOn(&orca_runtime_bindgen_clock.step);
    orca_platform_lib.step.dependOn(&orca_runtime_bindgen_io.step);
    orca_platform_lib.step.dependOn(&orca_runtime_bindgen_gles.step);

    const orca_platform_install_opts: Build.Step.InstallArtifact.Options = .{
        .dest_dir = .{ .override = .bin },
    };

    const orca_platform_install: *Build.Step.InstallArtifact = b.addInstallArtifact(orca_platform_lib, orca_platform_install_opts);

    // wasm3

    const wasm3_lib = b.addLibrary(.{
        .linkage = .static,
        .name = "wasm3",
        .root_module = b.createModule(.{
            .target = target,
            .optimize = optimize,
            .link_libc = true,
        }),
    });

    const wasm3_sources: []const []const u8 = &.{
        "src/ext/wasm3/source/m3_api_libc.c",
        "src/ext/wasm3/source/m3_api_meta_wasi.c",
        "src/ext/wasm3/source/m3_api_tracer.c",
        "src/ext/wasm3/source/m3_api_uvwasi.c",
        "src/ext/wasm3/source/m3_api_wasi.c",
        "src/ext/wasm3/source/m3_bind.c",
        "src/ext/wasm3/source/m3_code.c",
        "src/ext/wasm3/source/m3_compile.c",
        "src/ext/wasm3/source/m3_core.c",
        "src/ext/wasm3/source/m3_env.c",
        "src/ext/wasm3/source/m3_exec.c",
        "src/ext/wasm3/source/m3_function.c",
        "src/ext/wasm3/source/m3_info.c",
        "src/ext/wasm3/source/m3_module.c",
        "src/ext/wasm3/source/m3_parse.c",
        "src/ext/wasm3/source/extensions/m3_extensions.c",
    };

    var wasm3_compile_flags: std.ArrayList([]const u8) = .init(b.allocator);
    try wasm3_compile_flags.append("-fno-sanitize=undefined");
    if (target.result.os.tag.isDarwin()) {
        try wasm3_compile_flags.append("-foptimize-sibling-calls");
        try wasm3_compile_flags.append("-Wno-extern-initializer");
        try wasm3_compile_flags.append("-Dd_m3VerboseErrorMessages");
        try wasm3_compile_flags.append(compile_flag_min_macos_version);
    }

    wasm3_lib.addIncludePath(b.path("src/ext/wasm3/source"));
    wasm3_lib.addCSourceFiles(.{
        .files = wasm3_sources,
        .flags = wasm3_compile_flags.items,
    });

    // orca runtime exe

    var orca_runtime_compile_flags: std.ArrayList([]const u8) = .init(b.allocator);
    defer orca_runtime_compile_flags.deinit();
    try orca_runtime_compile_flags.append("-DOC_WASM_BACKEND_WASM3=1");
    try orca_runtime_compile_flags.append("-DOC_WASM_BACKEND_BYTEBOX=0");
    if (optimize == .Debug) {
        try orca_runtime_compile_flags.append("-DOC_DEBUG");
        try orca_runtime_compile_flags.append("-DOC_LOG_COMPILE_DEBUG");
    }

    const orca_runtime_exe = b.addExecutable(.{
        .name = "orca_runtime",
        .root_module = b.createModule(.{
            .target = target,
            .optimize = optimize,
        }),
    });

    orca_runtime_exe.addIncludePath(b.path("src"));
    orca_runtime_exe.addIncludePath(b.path("src/ext"));
    orca_runtime_exe.addIncludePath(angle_include_path);
    orca_runtime_exe.addIncludePath(b.path("src/ext/wasm3/source"));

    orca_runtime_exe.root_module.addRPathSpecial("@executable_path/");

    orca_runtime_exe.addCSourceFiles(.{
        .files = &.{"src/runtime.c"},
        .flags = orca_runtime_compile_flags.items,
    });

    orca_runtime_exe.linkLibrary(wasm3_lib);
    orca_runtime_exe.linkLibrary(orca_platform_lib);
    orca_runtime_exe.linkLibC();

    orca_runtime_exe.step.dependOn(&stage_angle_dawn_artifacts.step);

    orca_runtime_exe.step.dependOn(&orca_runtime_bindgen_core.step);
    orca_runtime_exe.step.dependOn(&orca_runtime_bindgen_surface.step);
    orca_runtime_exe.step.dependOn(&orca_runtime_bindgen_clock.step);
    orca_runtime_exe.step.dependOn(&orca_runtime_bindgen_io.step);
    orca_runtime_exe.step.dependOn(&orca_runtime_bindgen_gles.step);

    const orca_runtime_exe_install: *Build.Step.InstallArtifact = b.addInstallArtifact(orca_runtime_exe, .{});

    ///////////////////////////////////////////////////////
    // orca wasm libc

    var wasm_target_query: std.Target.Query = .{
        .cpu_arch = std.Target.Cpu.Arch.wasm32,
        .os_tag = std.Target.Os.Tag.freestanding,
    };
    wasm_target_query.cpu_features_add.addFeature(@intFromEnum(std.Target.wasm.Feature.bulk_memory));
    wasm_target_query.cpu_features_add.addFeature(@intFromEnum(std.Target.wasm.Feature.nontrapping_fptoint));

    const wasm_target: Build.ResolvedTarget = b.resolveTargetQuery(wasm_target_query);

    const wasm_optimize: std.builtin.OptimizeMode = .ReleaseSmall;

    // zig fmt: off
    const libc_flags: []const []const u8 = &.{
        // need to provide absolute paths to these since we're overriding the default zig lib dir
        b.fmt("-I{s}", .{b.pathFromRoot("src")}),
        "-isystem", b.pathFromRoot("src/orca-libc/src/include"),
        "-isystem", b.pathFromRoot("src/orca-libc/src/include/private"),
        b.fmt("-I{s}", .{b.pathFromRoot("src/orca-libc/src/arch")}),
        b.fmt("-I{s}", .{b.pathFromRoot("src/orca-libc/src/internal")}),

        // warnings
        "-Wall",
        "-Wextra",
        "-Werror",
        "-Wno-null-pointer-arithmetic",
        "-Wno-unused-parameter",
        "-Wno-sign-compare",
        "-Wno-unused-variable",
        "-Wno-unused-function",
        "-Wno-ignored-attributes",
        "-Wno-missing-braces",
        "-Wno-ignored-pragmas",
        "-Wno-unused-but-set-variable",
        "-Wno-unknown-warning-option",
        "-Wno-parentheses",
        "-Wno-shift-op-parentheses",
        "-Wno-bitwise-op-parentheses",
        "-Wno-logical-op-parentheses",
        "-Wno-string-plus-int",
        "-Wno-dangling-else",
        "-Wno-unknown-pragmas",

        // defines
        "-D__ORCA__",
        "-DBULK_MEMORY_THRESHOLD=32",

        // other flags
        "--std=c11",
    };
    // zig fmt: on

    // dummy crt1 object for sysroot folder

    const dummy_crt_obj = b.addObject(.{
        .name = "crt1",
        .root_module = b.createModule(.{
            .target = wasm_target,
            .optimize = wasm_optimize,
            .link_libc = false,
        }),
    });
    dummy_crt_obj.addCSourceFiles(.{
        .files = &.{"src/orca-libc/src/crt/crt1.c"},
        .flags = libc_flags,
    });

    const libc_install_opts: Build.Step.InstallArtifact.Options = .{
        .dest_dir = .{ .override = .{ .custom = "orca-libc/lib" } },
    };

    const dummy_crt_install: *Build.Step.InstallArtifact = b.addInstallArtifact(dummy_crt_obj, libc_install_opts);

    // wasm libc
    //
    // NOTE - libc must be built in a 2-stage pass by compiling all the c files into .objs individually and then linking them
    // all together at the end into a static lib. There are a couple reasons for this:
    // 1. The build runner invokes zig.exe with commandline args corresponding to its inputs, and the commandline gets too
    //    long if all the C files are added directly to the static lib. :(
    // 2. Generating a unity build file doesn't work because the .c files have been written assuming individual compilation
    //    and there are lots of constants with conflicting names in different files. For example, see "huge" in csinh.c
    //    and csinhf.c
    // 3. Only one .c file is allowed to correspond to an obj file. We can't combine multiple C files into one obj.

    const wasm_libc_source_paths: []const []const u8 = &.{
        "src/orca-libc/src/complex",
        "src/orca-libc/src/crt",
        "src/orca-libc/src/ctype",
        "src/orca-libc/src/errno",
        "src/orca-libc/src/exit",
        "src/orca-libc/src/fenv",
        "src/orca-libc/src/internal",
        "src/orca-libc/src/malloc",
        "src/orca-libc/src/math",
        "src/orca-libc/src/multibyte",
        "src/orca-libc/src/prng",
        "src/orca-libc/src/stdio",
        "src/orca-libc/src/stdlib",
        "src/orca-libc/src/string",
    };

    var wasm_libc_sources = CSources.init(b);
    defer wasm_libc_sources.deinit();

    var wasm_libc_objs: std.ArrayList(*Build.Step.Compile) = .init(b.allocator);
    try wasm_libc_objs.ensureUnusedCapacity(1024); // there are 496 .c files in the libc so this should be enough

    for (wasm_libc_source_paths) |path| {
        wasm_libc_sources.files.shrinkRetainingCapacity(0);
        try wasm_libc_sources.collect(path);

        const libc_group: []const u8 = std.fs.path.basename(path);

        for (wasm_libc_sources.files.items) |filepath| {
            const filename: []const u8 = std.fs.path.basename(filepath);
            const obj_name: []const u8 = try std.mem.join(b.allocator, "_", &.{ "libc", libc_group, filename });
            const obj = b.addObject(.{
                .name = obj_name,
                .root_module = b.createModule(.{
                    .target = wasm_target,
                    .optimize = wasm_optimize,
                    .single_threaded = true,
                    .link_libc = false,
                }),
                .zig_lib_dir = b.path("src/orca-libc"), // ensures c stdlib headers bundled with zig are ignored
            });
            obj.addCSourceFiles(.{
                .files = &.{filepath},
                .flags = libc_flags,
            });
            try wasm_libc_objs.append(obj);
        }
    }

    const wasm_libc_lib = b.addLibrary(.{
        .linkage = .static,
        .name = "c",
        .root_module = b.createModule(.{
            .target = wasm_target,
            .optimize = wasm_optimize,
            .link_libc = false,
            .single_threaded = true,
        }),
    });
    for (wasm_libc_objs.items) |obj| {
        wasm_libc_lib.addObject(obj);
    }

    wasm_libc_lib.installHeadersDirectory(b.path("src/orca-libc/include"), "orca-libc/include", .{});

    const libc_install: *Build.Step.InstallArtifact = b.addInstallArtifact(wasm_libc_lib, libc_install_opts);

    /////////////////////////////////////////////////////////
    // Orca wasm SDK

    const wasm_sdk_flags: []const []const u8 = &.{
        // "-Isrc",
        // "-Isrc/ext",
        // "-Isrc/orca-libc/include",
        b.fmt("-I{s}", .{b.pathFromRoot("src")}),
        b.fmt("-I{s}", .{b.pathFromRoot("src/ext")}),
        b.fmt("-I{s}", .{b.pathFromRoot("src/orca-libc/include")}),
        "--no-standard-libraries",
        "-D__ORCA__",
        // "-Wl,--no-entry",
        // "-Wl,--export-dynamic",
        // "-Wl,--relocatable"
    };

    const wasm_sdk_obj = b.addObject(.{
        .name = "orca_wasm",
        .root_module = b.createModule(.{
            .target = wasm_target,
            .optimize = wasm_optimize,
            .single_threaded = true,
            .link_libc = false,
        }),
        .zig_lib_dir = b.path("src/orca-libc"),
    });
    wasm_sdk_obj.addCSourceFile(.{
        .file = b.path("src/orca.c"),
        .flags = wasm_sdk_flags,
    });

    wasm_sdk_obj.step.dependOn(&stage_angle_dawn_artifacts.step);
    wasm_sdk_obj.step.dependOn(&stage_angle_dawn_src.step);

    wasm_sdk_obj.step.dependOn(&orca_runtime_bindgen_core.step);
    wasm_sdk_obj.step.dependOn(&orca_runtime_bindgen_surface.step);
    wasm_sdk_obj.step.dependOn(&orca_runtime_bindgen_clock.step);
    wasm_sdk_obj.step.dependOn(&orca_runtime_bindgen_io.step);
    wasm_sdk_obj.step.dependOn(&orca_runtime_bindgen_gles.step);

    const wasm_sdk_lib = b.addLibrary(.{
        .linkage = .static,
        .name = "orca_wasm",
        .root_module = b.createModule(.{
            .target = wasm_target,
            .optimize = wasm_optimize,
            .link_libc = false,
            .single_threaded = true,
        }),
    });
    wasm_sdk_lib.addObject(wasm_sdk_obj);

    const wask_sdk_install_opts: Build.Step.InstallArtifact.Options = .{
        .dest_dir = .{ .override = .bin },
    };

    const wasm_sdk_install: *Build.Step.InstallArtifact = b.addInstallArtifact(wasm_sdk_lib, wask_sdk_install_opts);

    ///////////////////////////////////////////////////////////////
    // zig build - default install step builds and installs a dev build of orca

    const build_orca = b.step("orca", "Build all orca binaries");
    build_orca.dependOn(&orca_platform_install.step);
    build_orca.dependOn(&orca_runtime_exe_install.step);
    build_orca.dependOn(&libc_install.step);
    build_orca.dependOn(&dummy_crt_install.step);
    build_orca.dependOn(&wasm_sdk_install.step);
    build_orca.dependOn(&build_orca_tool.step);

    const package_sdk_exe: *Build.Step.Compile = b.addExecutable(.{
        .name = "package_sdk",
        .root_module = b.createModule(.{
            .root_source_file = b.path("src/build/package_sdk.zig"),
            .target = b.graph.host,
            .optimize = .Debug,
        }),
    });

    const sdk_install_path_opt = b.option([]const u8, "sdk-path", "Specify absolute path for installing the Orca SDK.");

    const SdkHelpers = struct {
        fn addAbsolutePathArg(b_: *Build, target_: Build.ResolvedTarget, run: *Build.Step.Run, prefix: []const u8, path: []const u8) void {
            if (path.len == 0) {
                return;
            }

            var path_absolute: []const u8 = path;

            if (std.fs.path.isAbsolute(path_absolute) == false) {
                if (path_absolute[0] == '~') {
                    const home: []const u8 = homePath(target_, b_);
                    path_absolute = std.fs.path.join(b_.allocator, &.{ home, path_absolute[1..] }) catch @panic("OOM");
                } else {
                    path_absolute = b_.pathFromRoot(path);
                }
            }

            const sdk_path = std.mem.join(b_.allocator, "", &.{ prefix, path_absolute }) catch @panic("OOM");
            run.addArg(sdk_path);
        }
    };

    const orca_install: *Build.Step.Run = b.addRunArtifact(package_sdk_exe);
    orca_install.addArg("--dev-install");
    orca_install.addPrefixedDirectoryArg("--artifacts-path=", LazyPath{ .cwd_relative = b.install_path });
    orca_install.addPrefixedDirectoryArg("--resources-path=", b.path("resources"));
    orca_install.addPrefixedDirectoryArg("--src-path=", b.path("src"));

    if (sdk_install_path_opt) |sdk_install_path| {
        SdkHelpers.addAbsolutePathArg(b, target, orca_install, "--sdk-path=", sdk_install_path);
    }

    if (git_version_opt) |git_version| {
        orca_install.addArg(b.fmt("--version={s}", .{git_version}));
    }

    orca_install.step.dependOn(build_orca);

    const opt_sdk_version = b.option([]const u8, "sdk-version", "Override current git version for sdk packaging.");
    if (opt_sdk_version) |sdk_version| {
        const version = try std.mem.join(b.allocator, "", &.{ "--version=", sdk_version });
        orca_install.addArg(version);
    }

    b.getInstallStep().dependOn(&orca_install.step);

    /////////////////////////////////////////////////////////////////
    // zig build clean

    const clean_step: *Build.Step = b.step("clean", "Delete all build artifacts and start fresh.");

    const clean_paths = [_][]const u8{
        // folders
        "zig-out",
        "src/ext/angle",
        "src/ext/dawn",
        "scripts/files",
        "scripts/__pycache",

        // files
        "src/graphics/orca_surface_stubs.c",
        "src/platform/orca_io_stubs.c",
        "src/wasmbind/clock_api_bind_gen.c",
        "src/wasmbind/core_api_bind_gen.c",
        "src/wasmbind/core_api_stubs.c",
        "src/wasmbind/gles_api.json",
        "src/wasmbind/gles_api_bind_gen.c",
        "src/wasmbind/io_api_bind_gen.c",
        "src/wasmbind/surface_api_bind_gen.c",
    };
    for (clean_paths) |path| {
        const remove_dir = b.addRemoveDirTree(b.path(path));
        clean_step.dependOn(&remove_dir.step);
    }

    b.getUninstallStep().dependOn(clean_step);

    /////////////////////////////////////////////////////////////////
    // sketches

    const sketches = b.step("sketches", "Build all sketches into build/sketches");

    const sketches_install_opts: Build.Step.InstallArtifact.Options = .{
        .dest_dir = .{ .override = .{ .custom = "sketches" } },
    };

    const orca_platform_sketches_install: *Build.Step.InstallArtifact = b.addInstallArtifact(orca_platform_lib, sketches_install_opts);
    sketches.dependOn(&orca_platform_sketches_install.step);

    const stage_sketch_dependency_artifacts = b.addUpdateSourceFiles();
    {
        const sketches_install_path = b.pathJoin(&.{ b.install_path, "sketches" });

        const resources: []const []const u8 = &.{
            "resources/CMUSerif-Roman.ttf",
            "resources/Courier.ttf",
            "resources/gamma-1.0-or-2.2.png",
            "resources/gamma_dalai_lama_gray.png",
            "resources/gradient_srgb.png",
            "resources/OpenSansLatinSubset.ttf",
            "resources/OpenSans-Regular.ttf",
            "resources/OpenSans-Bold.ttf",
            "resources/Top512.png",
            "resources/triceratops.png",
        };

        for (resources) |resource| {
            const src = b.path(b.pathJoin(&.{ "sketches", resource }));
            const dest = b.pathJoin(&.{ sketches_install_path, resource });
            stage_sketch_dependency_artifacts.addCopyFileToSource(src, dest);
        }

        try stageAngleDawnArtifacts(b, target, stage_sketch_dependency_artifacts, sketches_install_path, angle_lib_path, dawn_lib_path);
        sketches.dependOn(&stage_sketch_dependency_artifacts.step);
    }

    const sketches_folders: []const []const u8 = &.{
        //"atlas", // bitrotted
        "canvas",
        "canvas_test",
        "canvas_triangle_stress",
        "check-bleeding",
        "colorspace",
        "image",
        "keyboard",
        "minimalD3D12",
        "minimalDawnWebGPU",
        // "multi_surface", // bitrotted
        "perf_text",
        // "render_thread", // bitrotted
        "simpleWindow",
        "smiley",
        //"smooth_resize", // bitrotted
        // "test-clear", // wasm bundle test - probably should be in samples?
        "tiger",
        //"triangleGL", // openGL API no longer supported
        "triangleGLES",
        "triangleMetal",
        // "triangleWGPU", // bitrotted
        // "ui", // bitrotted
    };

    for (sketches_folders) |sketch| {
        const sketch_source: []const u8 = b.pathJoin(&.{ "sketches", sketch, "main.c" });
        if (pathExists(cwd, sketch_source) == false) {
            continue;
        }

        if (std.mem.eql(u8, "triangleMetal", sketch) and target.result.os.tag.isDarwin() == false) {
            continue;
        }

        const flags: []const []const u8 = &.{
            "-Isrc",
            "-Isrc/ext",
            "-Isrc/ext/angle/include",
            "-Isrc/ext/dawn/include",
            "-Isrc/util",
            "-Isrc/platform",
        };

        const sketch_exe: *Build.Step.Compile = b.addExecutable(.{
            .name = sketch,
            .root_module = b.createModule(.{
                .target = target,
                .optimize = optimize,
                .link_libc = true,
            }),
        });
        sketch_exe.addCSourceFiles(.{
            .files = &.{sketch_source},
            .flags = flags,
        });
        sketch_exe.linkLibrary(orca_platform_lib);

        const install: *Build.Step.InstallArtifact = b.addInstallArtifact(sketch_exe, sketches_install_opts);
        sketches.dependOn(&install.step);
    }

    /////////////////////////////////////////////////////////////////
    // tests

    const tests = b.step("test", "Build and run all tests");

    const TestConfig = struct {
        name: []const u8,
        testfile: []const u8 = "main.c",
        run: bool = false,
        wasm: bool = false,
    };

    // several tests require UI interactions so we won't run them all automatically, but configure
    // only some of them to be run
    const test_configs: []const TestConfig = &.{
        .{
            .name = "bulkmem",
            .wasm = true,
        },
        .{
            .name = "file_dialog",
        },
        .{
            .name = "file_open_request",
        },
        .{
            .name = "files",
            .run = true,
        },
        .{
            .name = "perf",
            .testfile = "driver.c",
        },
        .{
            .name = "wasm_tests",
            .wasm = true,
        },
    };

    const tests_install_opts: Build.Step.InstallArtifact.Options = .{
        .dest_dir = .{ .override = .{ .custom = "tests" } },
    };

    const orca_platform_tests_install: *Build.Step.InstallArtifact = b.addInstallArtifact(orca_platform_lib, tests_install_opts);
    tests.dependOn(&orca_platform_tests_install.step);

    const stage_test_dependency_artifacts = b.addUpdateSourceFiles();
    {
        const tests_install_path = b.pathJoin(&.{ b.install_path, "tests" });
        try stageAngleDawnArtifacts(b, target, stage_sketch_dependency_artifacts, tests_install_path, angle_lib_path, dawn_lib_path);
        tests.dependOn(&stage_test_dependency_artifacts.step);
    }

    for (test_configs) |config| {
        // TODO add support for building wasm samples
        if (config.wasm) {
            continue;
        }

        const test_source: []const u8 = b.pathJoin(&.{ "tests", config.name, config.testfile });

        const test_exe: *Build.Step.Compile = b.addExecutable(.{
            .name = config.name,
            .root_module = b.createModule(.{
                .target = target,
                .optimize = optimize,
                .link_libc = true,
            }),
        });
        test_exe.addCSourceFiles(.{
            .files = &.{test_source},
            .flags = &.{b.fmt("-I{s}", .{b.pathFromRoot("src")})},
        });
        test_exe.linkLibrary(orca_platform_lib);

        if (target.result.os.tag == .windows) {
            test_exe.linkSystemLibrary("shlwapi");
        }

        const install: *Build.Step.InstallArtifact = b.addInstallArtifact(test_exe, tests_install_opts);
        tests.dependOn(&install.step);

        if (config.run) {
            if (config.wasm) {
                // TODO add support for running wasm tests
                const fail = b.addFail("Running is currently not supported for wasm tests.");
                tests.dependOn(&fail.step);
            } else {
                const test_dir_path = b.path(b.pathJoin(&.{ "tests", config.name }));

                const run_test = b.addRunArtifact(test_exe);
                run_test.addPrefixedFileArg("--test-dir=", test_dir_path); // allows tests to access their data files
                run_test.step.dependOn(&stage_test_dependency_artifacts.step);
                run_test.step.dependOn(&install.step); // causes test exe working dir to be build\tests\ instead of zig-cache

                tests.dependOn(&run_test.step);
            }
        }
    }
}
