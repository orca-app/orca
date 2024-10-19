const std = @import("std");
// const OptimizeMode = std.builtin.OptimizeMode;

const builtin = @import("builtin");

const MAX_FILE_SIZE = 1024 * 1024 * 128;

const Lib = enum {
    Angle,
    Dawn,

    fn toStr(lib: Lib) []const u8 {
        return switch (lib) {
            .Angle => "angle",
            .Dawn => "dawn",
        };
    }
};

const Options = struct {
    arena: std.mem.Allocator,
    lib: Lib,
    commit_sha: []const u8,
    check_only: bool,
    optimize: std.builtin.OptimizeMode,
    paths: struct {
        python: []const u8,
        src_dir: []const u8,
        intermediate_dir: []const u8,
        output_dir: []const u8,
    },

    fn parse(args: []const [:0]const u8, arena: std.mem.Allocator) !Options {
        var lib: ?Lib = null;
        var commit_sha: ?[]const u8 = null;
        var check_only: bool = false;
        var optimize: std.builtin.OptimizeMode = .ReleaseFast;

        var python: ?[]const u8 = null;
        var src_dir: ?[]const u8 = null;
        var intermediate_dir: ?[]const u8 = null;

        for (args, 0..) |raw_arg, i| {
            if (i == 0) {
                continue;
            }

            var splitIter = std.mem.splitScalar(u8, raw_arg, '=');
            const arg: []const u8 = splitIter.next().?;
            if (std.mem.eql(u8, arg, "--lib")) {
                if (splitIter.next()) |lib_str| {
                    if (std.mem.eql(u8, lib_str, "angle")) {
                        lib = .Angle;
                    } else if (std.mem.eql(u8, lib_str, "dawn")) {
                        lib = .Dawn;
                    } else {
                        return error.InvalidArgument;
                    }
                } else {
                    return error.InvalidArgument;
                }
            } else if (std.mem.eql(u8, arg, "--sha")) {
                commit_sha = splitIter.next();
            } else if (std.mem.eql(u8, arg, "--check")) {
                check_only = true;
            } else if (std.mem.eql(u8, arg, "--debug")) {
                optimize = .Debug;
            } else if (std.mem.eql(u8, arg, "--python")) {
                python = splitIter.next();
            } else if (std.mem.eql(u8, arg, "--src")) {
                src_dir = splitIter.next();
            } else if (std.mem.eql(u8, arg, "--intermediate")) {
                intermediate_dir = splitIter.next();
            }

            // logic above should have consumed all tokens, if any are left it's an error
            if (splitIter.next()) |last| {
                std.log.err("Unexpected part of arg: {s}", .{last});
                return error.InvalidArgument;
            }
        }

        var missing_arg: ?[]const u8 = null;
        if (lib == null) {
            missing_arg = "lib";
        } else if (commit_sha == null) {
            missing_arg = "sha";
        } else if (python == null) {
            missing_arg = "python";
        } else if (src_dir == null) {
            missing_arg = "src";
        } else if (intermediate_dir == null) {
            missing_arg = "intermediate";
        }

        if (missing_arg) |arg| {
            std.log.err("Missing required arg: {s}\n", .{arg});
            return error.MissingRequiredArgument;
        }

        var bad_absolute_path: ?[]const u8 = null;
        if (std.fs.path.isAbsolute(src_dir.?) == false) {
            bad_absolute_path = src_dir;
        } else if (std.fs.path.isAbsolute(intermediate_dir.?) == false) {
            bad_absolute_path = intermediate_dir;
        }

        if (bad_absolute_path) |path| {
            std.log.err("Path {s} must be absolute", .{path});
        }

        const output_folder: []const u8 = try std.fmt.allocPrint(arena, "{s}.out", .{lib.?.toStr()});
        const output_dir: []const u8 = try std.fs.path.join(arena, &.{ intermediate_dir.?, output_folder });

        return .{
            .arena = arena,
            .lib = lib.?,
            .commit_sha = commit_sha.?,
            .check_only = check_only,
            .optimize = optimize,
            .paths = .{
                .python = python.?,
                .src_dir = src_dir.?,
                .intermediate_dir = intermediate_dir.?,
                .output_dir = output_dir,
            },
        };
    }
};

const Sort = struct {
    fn lessThanString(_: void, lhs: []const u8, rhs: []const u8) bool {
        return std.mem.lessThan(u8, lhs, rhs);
    }
};

fn exec(arena: std.mem.Allocator, argv: []const []const u8, cwd: []const u8, env: *std.process.EnvMap) !void {
    var log_msg = std.ArrayList(u8).init(arena);
    var log_writer = log_msg.writer();
    try log_writer.print("running: ", .{});
    for (argv) |arg| {
        try log_writer.print("{s} ", .{arg});
    }
    std.log.info("{s}", .{log_msg.items});

    const result: std.process.Child.RunResult = try std.process.Child.run(.{
        .allocator = arena,
        .argv = argv,
        .cwd = cwd,
        .env_map = env,
    });

    if (result.stdout.len > 0) {
        std.log.info("\n{s}", .{result.stdout});
    }

    if (result.stderr.len > 0) {
        std.log.info("\n{s}", .{result.stderr});
        return error.StderrLogsEmitted;
    }

    switch (result.term) {
        .Exited => |v| {
            if (v != 0) {
                std.log.err("process {s} exited with nonzero exit code {}.", .{ argv[0], v });
                return error.NonZeroExitCode;
            }
        },
        else => {
            std.log.err("process {s} exited abnormally.", .{argv[0]});
            return error.AbnormalExit;
        },
    }
}

fn execShell(arena: std.mem.Allocator, argv: []const []const u8, cwd: []const u8, env: *std.process.EnvMap) !void {
    var final_args = std.ArrayList([]const u8).init(arena);
    if (builtin.os.tag == .windows) {
        try final_args.append("cmd.exe");
        try final_args.append("/c");
        try final_args.append(try std.mem.join(arena, "", &.{ argv[0], ".bat" }));
        try final_args.appendSlice(argv[1..]);
    } else {
        try final_args.appendSlice(argv);
    }
    try exec(arena, final_args.items, cwd, env);
}

fn pathExists(dir: std.fs.Dir, path: []const u8) std.fs.Dir.AccessError!bool {
    dir.access(path, .{}) catch |e| {
        if (e == std.fs.Dir.AccessError.FileNotFound) {
            return false;
        } else {
            return e;
        }
    };

    return true;
}

fn copyFolder(allocator: std.mem.Allocator, dest: []const u8, src: []const u8) !void {
    std.log.info("copying '{s}' to '{s}'", .{ src, dest });

    const cwd = std.fs.cwd();
    try cwd.makePath(dest);

    const src_dir: std.fs.Dir = try cwd.openDir(src, .{ .iterate = true });
    const dest_dir: std.fs.Dir = try cwd.openDir(dest, .{ .iterate = true });

    var src_walker = try src_dir.walk(allocator);
    while (try src_walker.next()) |src_entry| {
        _ = switch (src_entry.kind) {
            .directory => try dest_dir.makePath(src_entry.path),
            .file => try src_dir.updateFile(src_entry.path, dest_dir, src_entry.path, .{}),
            else => {},
        };
    }
}

// Algorithm ported from checksumdir package on pypy, which is MIT licensed.
const Checksum = struct {
    const Sha256 = std.crypto.hash.sha2.Sha256;
    const Sha1 = std.crypto.hash.Sha1;

    fn empty(allocator: std.mem.Allocator) ![]const u8 {
        const out: []u8 = try allocator.alloc(u8, Sha256.digest_length);
        @memset(out, 0);
        return out;
    }

    fn hexdigest(digest: []const u8, allocator: std.mem.Allocator) ![]const u8 {
        const out = try allocator.alloc(u8, digest.len * 2);
        return std.fmt.bufPrint(
            out,
            "{s}",
            .{std.fmt.fmtSliceHexLower(digest)},
        ) catch unreachable;
    }

    fn file(allocator: std.mem.Allocator, path: []const u8) ![]const u8 {
        const cwd: std.fs.Dir = std.fs.cwd();

        const file_contents: []const u8 = try cwd.readFileAlloc(allocator, path, MAX_FILE_SIZE);
        defer allocator.free(file_contents);

        var digest: [Sha256.digest_length]u8 = undefined;
        Sha256.hash(file_contents, &digest, .{});

        return try hexdigest(&digest, allocator);
    }

    fn dir(allocator: std.mem.Allocator, path: []const u8, opts: struct {
        exclude_files: []const []const u8 = &.{},
        exclude_dirs: []const []const u8 = &.{},
    }) ![]const u8 {
        const cwd: std.fs.Dir = std.fs.cwd();
        var root_dir: std.fs.Dir = cwd.openDir(path, .{ .iterate = true, .no_follow = true }) catch |e| {
            if (e == error.FileNotFound) {
                return empty(allocator);
            }
            return e;
        };
        defer root_dir.close();

        var dir_iter: std.fs.Dir.Walker = try root_dir.walk(allocator);
        defer dir_iter.deinit();

        var files_to_hash = std.ArrayList([]const u8).init(allocator);
        defer {
            for (files_to_hash.items) |p| allocator.free(p);
            files_to_hash.deinit();
        }

        while (try dir_iter.next()) |entry| {
            if (entry.kind == .file) {
                var exclude: bool = false;

                for (opts.exclude_files) |exclusion| {
                    exclude = exclude or std.mem.eql(u8, entry.basename, exclusion);
                }

                for (opts.exclude_dirs) |exclusion| {
                    exclude = exclude or std.mem.startsWith(u8, entry.path, exclusion);
                }

                if (!exclude) {
                    const file_path = allocator.dupe(u8, entry.path) catch @panic("OOM");
                    files_to_hash.append(file_path) catch @panic("OOM");
                }
            }
        }

        std.mem.sort([]const u8, files_to_hash.items, {}, Sort.lessThanString);

        var hashes = std.ArrayList([]const u8).init(allocator);
        defer {
            for (hashes.items) |h| allocator.free(h);
            hashes.deinit();
        }

        for (files_to_hash.items) |file_path| {
            const file_contents: []const u8 = try root_dir.readFileAlloc(allocator, file_path, MAX_FILE_SIZE);
            defer allocator.free(file_contents);

            const BLOCKSIZE = 64 * 1024;
            var blocks = std.mem.window(u8, file_contents, BLOCKSIZE, BLOCKSIZE);

            var hash = Sha1.init(.{});
            while (blocks.next()) |block| {
                hash.update(block);
            }

            const digest: []u8 = try allocator.alloc(u8, Sha1.digest_length);
            hash.final(digest[0..Sha1.digest_length]);
            try hashes.append(digest);
        }

        std.mem.sort([]const u8, hashes.items, {}, Sort.lessThanString);

        var hash = Sha1.init(.{});
        for (hashes.items) |h| {
            const hex = try hexdigest(h, allocator);
            hash.update(hex);
            allocator.free(hex);
        }

        var digest: [Sha1.digest_length]u8 = undefined;
        hash.final(&digest);
        return try hexdigest(&digest, allocator);
    }
};

const CommitChecksum = struct {
    commit: []const u8,
    sum: []const u8,
};

const ANGLE_CHECKSUM_FILENAME = "angle.json";
const DAWN_CHECKSUM_FILENAME = "dawn.json";

fn checksumLib(opts: *const Options) ![]const u8 {
    const checksum_file = if (opts.lib == .Angle) ANGLE_CHECKSUM_FILENAME else DAWN_CHECKSUM_FILENAME;

    return try Checksum.dir(opts.arena, opts.paths.output_dir, .{ .exclude_files = &.{
        checksum_file,
        ".DS_Store",
    } });
}

const ShouldLogError = enum {
    LogError,
    NoError,

    fn logfn(comptime self: ShouldLogError) *fn (comptime _: []const u8, _: anytype) void {
        return switch (self) {
            .LogError => &std.log.err,
            .NoError => &noopLog,
        };
    }

    fn noopLog(comptime _: []const u8, _: anytype) void {}
};

fn isAngleUpToDate(opts: *const Options, comptime log_error: ShouldLogError) bool {
    const logfn = log_error.logfn();

    const sum = checksumLib(opts) catch |e| {
        logfn("Failed checksum dir '{s}': {}\n", .{ opts.paths.output_dir, e });
        return false;
    };

    const checksum_path = std.fs.path.join(opts.arena, &.{ opts.paths.output_dir, ANGLE_CHECKSUM_FILENAME }) catch @panic("OOM");
    const json_data: []const u8 = std.fs.cwd().readFileAlloc(opts.arena, checksum_path, MAX_FILE_SIZE) catch |e| {
        logfn("Failed to read checksum file from location '{s}': {}", .{ checksum_path, e });
        return false;
    };

    const loaded_checksum = std.json.parseFromSliceLeaky(CommitChecksum, opts.arena, json_data, .{}) catch |e| {
        logfn("Failed to parse file '{s}' json: {}. Raw json data:\n{s}\n", .{
            checksum_path,
            e,
            json_data,
        });
        return false;
    };
    if (std.mem.eql(u8, loaded_checksum.commit, opts.commit_sha) == false) {
        logfn("{s} doesn't match the required angle commit. expected {s}, got {s}", .{
            checksum_path,
            opts.commit_sha,
            loaded_checksum.commit,
        });
        return false;
    }

    if (std.mem.eql(u8, loaded_checksum.sum, sum) == false) {
        logfn("{s} doesn't match checksum. expected {s}, got {s}", .{
            checksum_path,
            loaded_checksum.commit,
            sum,
        });
        return false;
    }

    return true;
}

fn isDawnUpToDate(opts: *const Options, comptime log_error: ShouldLogError) bool {
    const logfn = log_error.logfn();

    const checksum_path = std.fs.path.join(opts.arena, &.{ opts.paths.output_dir, DAWN_CHECKSUM_FILENAME }) catch @panic("OOM");
    const json_string: []const u8 = std.fs.cwd().readFileAlloc(opts.arena, checksum_path, MAX_FILE_SIZE) catch |e| {
        logfn("Failed to read checksum file from location '{s}': {}", .{ checksum_path, e });
        return false;
    };

    const json = std.json.parseFromSliceLeaky(std.json.Value, opts.arena, json_string, .{}) catch |e| {
        logfn("Failed to parse file '{s}' json: {}. Raw json data:\n{s}\n", .{
            checksum_path,
            e,
            json_string,
        });
        return false;
    };

    var dawn_required_files = std.ArrayList([]const u8).init(opts.arena);
    dawn_required_files.append("include/webgpu.h");
    if (builtin.os.tag == .windows) {
        try dawn_required_files.append("bin/webgpu.lib");
        try dawn_required_files.append("bin/webgpu.dll");
    } else {
        try dawn_required_files.append("bin/webgpu.dylib");
    }

    // var path_to_sum = std.StringHashMap(CommitChecksum).init(opts.arena);
    for (dawn_required_files.items) |path| {
        var commit: ?[]const u8 = null;
        var sum: ?[]const u8 = null;

        if (json.value.object.get(path)) |commit_sum_value| {
            if (std.meta.tag(commit_sum_value) != .object) {
                logfn("Unexpected json structure", .{});
                return false;
            }
            if (commit_sum_value.object.get("commit")) |commit_value| {
                commit = switch (commit_value) {
                    .string => |v| v,
                    else => null,
                };
            }
            if (commit_sum_value.object.get("sum")) |sum_value| {
                sum = switch (sum_value) {
                    .string => |v| v,
                    else => null,
                };
            }
        }

        if (commit == null or sum == null) {
            logfn("Failed to find data for {s}", .{path});
            return false;
        }

        if (std.mem.eql(u8, commit.?, opts.sha) == false) {
            logfn("Commit for {s} is out of date - expected {s} but got {s}", .{
                path, opts.sha, commit.?,
            });
        }

        const path_absolute = try std.fs.path.join(opts.arena, &.{ opts.output_dir, path });
        const expected_sum = try Checksum.file(opts.arena, path_absolute);
        if (std.mem.eql(u8, sum, expected_sum)) {
            logfn("Checksum for {s} is out of date - expected {s} but got {s}", .{
                path, expected_sum, sum.?,
            });
        }
    }

    return true;
}

fn checkAngle(opts: *const Options) !void {
    if (isAngleUpToDate(opts, .LogError) == false) {
        return error.AngleOutOfDate;
    }
}

fn buildAngle(opts: *const Options) !void {
    if (isAngleUpToDate(opts, .NoError)) {
        std.log.info("angle is up to date - no rebuild needed.\n", .{});
        return;
    } else if (opts.check_only) {
        return error.AngleOutOfDate;
    }

    const cwd = std.fs.cwd();
    try cwd.makePath(opts.paths.intermediate_dir);

    std.log.info("angle is out of date - rebuilding", .{});

    var env: std.process.EnvMap = try std.process.getEnvMap(opts.arena);
    defer env.deinit();
    if (builtin.os.tag == .windows) {
        try env.put("DEPOT_TOOLS_WIN_TOOLCHAIN", "0");
    }

    const depot_tools_path = try std.fs.path.join(opts.arena, &.{ opts.paths.intermediate_dir, "depot_tools" });
    if (try pathExists(std.fs.cwd(), depot_tools_path) == false) {
        std.log.info("cloning depot_tools to intermediate '{s}'...", .{opts.paths.intermediate_dir});
        try exec(opts.arena, &.{ "git", "clone", "https://chromium.googlesource.com/chromium/tools/depot_tools.git" }, opts.paths.intermediate_dir, &env);
    } else {
        std.log.info("depot_tools already exists, skipping clone", .{});
    }

    const key = "PATH";
    if (env.get(key)) |env_path| {
        const new_path = try std.fmt.allocPrint(opts.arena, "{s}" ++ [1]u8{std.fs.path.delimiter} ++ "{s}", .{ env_path, depot_tools_path });
        try env.put(key, new_path);
    } else {
        try env.put(key, depot_tools_path);
    }

    // {
    //     std.debug.print(">>> env:\n", .{});
    //     var iter = env.iterator();
    //     while (iter.next()) |entry| {
    //         std.debug.print("\t{s}: {s}\n", .{ entry.key_ptr.*, entry.value_ptr.* });
    //     }
    // }

    const src_dir = try std.fs.path.join(opts.arena, &.{ opts.paths.intermediate_dir, opts.lib.toStr() });
    try copyFolder(opts.arena, src_dir, opts.paths.src_dir);

    const bootstrap_path = try std.fs.path.join(opts.arena, &.{ src_dir, "scripts/bootstrap.py" });
    try exec(opts.arena, &.{ opts.paths.python, bootstrap_path }, src_dir, &env);

    try execShell(opts.arena, &.{ "gclient", "sync" }, src_dir, &env);

    const optimize_str = if (opts.optimize == .Debug) "Debug" else "Release";
    const is_debug_str = if (opts.optimize == .Debug) "is_debug=true" else "is_debug=false";

    var gn_args_list = std.ArrayList([]const u8).init(opts.arena);
    try gn_args_list.append("angle_build_all=false");
    try gn_args_list.append("angle_build_tests=false");
    try gn_args_list.append("is_component_build=false");
    try gn_args_list.append(is_debug_str);

    if (builtin.os.tag == .windows) {
        try gn_args_list.append("angle_enable_d3d9=false");
        try gn_args_list.append("angle_enable_gl=false");
        try gn_args_list.append("angle_enable_vulkan=false");
        try gn_args_list.append("angle_enable_null=false");
        try gn_args_list.append("angle_has_frame_capture=false");
    } else {
        //NOTE(martin): oddly enough, this is needed to avoid deprecation errors when _not_ using OpenGL,
        //              because angle uses some CGL APIs to detect GPUs.
        try gn_args_list.append("treat_warnings_as_errors=false");
        try gn_args_list.append("angle_enable_metal=true");
        try gn_args_list.append("angle_enable_gl=false");
        try gn_args_list.append("angle_enable_vulkan=false");
        try gn_args_list.append("angle_enable_null=false");
    }
    const gn_all_args = try std.mem.join(opts.arena, " ", gn_args_list.items);

    const gn_args: []const u8 = try std.fmt.allocPrint(opts.arena, "--args={s}", .{gn_all_args});

    const optimize_output_path = try std.fs.path.join(opts.arena, &.{ src_dir, "out", optimize_str });
    try cwd.makePath(optimize_output_path);

    try execShell(opts.arena, &.{ "gn", "gen", optimize_output_path, gn_args }, src_dir, &env);

    try execShell(opts.arena, &.{ "autoninja", "-C", optimize_output_path, "libEGL", "libGLESv2" }, src_dir, &env);

    // copy artifacts to output dir
    {
        const join = std.fs.path.join;
        const a = opts.arena;
        const output_dir = opts.paths.output_dir;

        const bin_path = try join(a, &.{ output_dir, "bin" });

        const inc_folders: []const []const u8 = &.{
            "include/KHR",
            "include/EGL",
            "include/GLES",
            "include/GLES2",
            "include/GLES3",
        };

        for (inc_folders) |folder| {
            const src_path = try join(a, &.{ src_dir, folder });
            const dest_path = try join(a, &.{ output_dir, folder });
            try cwd.deleteTree(dest_path);
            try cwd.makePath(dest_path);
            _ = try copyFolder(a, dest_path, src_path);
        }

        var libs = std.ArrayList([]const u8).init(a);
        if (builtin.os.tag == .windows) {
            try libs.append("libEGL.dll");
            try libs.append("libEGL.dll.lib");
            try libs.append("libGLESv2.dll");
            try libs.append("libGLESv2.dll.lib");
        } else {
            try libs.append("libEGL.dylib");
            try libs.append("libGLESv2.dylib");
        }

        var bin_src_dir: std.fs.Dir = try cwd.openDir(optimize_output_path, .{});

        try cwd.deleteTree(bin_path);
        try cwd.makePath(bin_path);

        const bin_dest_dir: std.fs.Dir = try cwd.openDir(bin_path, .{});
        for (libs.items) |filename| {
            _ = bin_src_dir.updateFile(filename, bin_dest_dir, filename, .{}) catch |e| {
                if (e == error.FileNotFound) {
                    const source_path = try std.fs.path.join(opts.arena, &.{ optimize_output_path, filename });
                    std.log.err("Failed to copy {s} - not found.", .{source_path});
                    return e;
                }
            };
        }

        if (builtin.os.tag == .windows) {
            const windows_sdk = std.zig.WindowsSdk.find(opts.arena) catch |e| {
                std.log.err("Failed to find Windows SDK. Do you have the Windows 10 SDK installed?", .{});
                return e;
            };

            var windows_sdk_path: []const u8 = "";
            if (windows_sdk.windows10sdk) |install| {
                windows_sdk_path = install.path;
            } else if (windows_sdk.windows81sdk) |install| {
                windows_sdk_path = install.path;
            } else {
                std.log.err("Failed to find Windows SDK. Do you have the Windows 10 SDK installed?", .{});
                return error.FailedToFindWindowsSdk;
            }

            const src_d3dcompiler_path = try std.fs.path.join(opts.arena, &.{
                windows_sdk_path,
                "Redist",
                "D3D",
                "x64",
            });
            var src_d3dcompiler_dir: std.fs.Dir = try cwd.openDir(src_d3dcompiler_path, .{});
            _ = try src_d3dcompiler_dir.updateFile("d3dcompiler_47.dll", bin_dest_dir, "d3dcompiler_47.dll", .{});
        }
    }

    // write stamp file
    {
        const commit_checksum = CommitChecksum{
            .sum = try checksumLib(opts),
            .commit = opts.commit_sha,
        };
        var json = std.ArrayList(u8).init(opts.arena);
        try std.json.stringify(commit_checksum, .{}, json.writer());
        try cwd.makePath(opts.paths.output_dir);
        const checksum_path = std.fs.path.join(opts.arena, &.{ opts.paths.output_dir, ANGLE_CHECKSUM_FILENAME }) catch @panic("OOM");
        try std.fs.cwd().writeFile(.{
            .sub_path = checksum_path,
            .data = json.items,
            .flags = .{},
        });
    }
}

fn buildDawn(opts: *const Options) !void {
    if (isDawnUpToDate(opts, .NoError)) {
        std.log.info("dawn is up to date - no rebuild needed.\n", .{});
        return;
    } else if (opts.check_only) {
        return error.DawnOutOfDate;
    }

    return error.Unimplemented;
}

pub fn main() !void {
    var arena = std.heap.ArenaAllocator.init(std.heap.page_allocator);
    defer arena.deinit();
    const allocator: std.mem.Allocator = arena.allocator();

    const args: []const [:0]u8 = try std.process.argsAlloc(allocator);
    defer std.process.argsFree(allocator, args);

    const opts = try Options.parse(args, allocator);
    switch (opts.lib) {
        .Angle => try buildAngle(&opts),
        .Dawn => try buildDawn(&opts),
    }
}

// test "check angle" {
//     var arena = std.heap.ArenaAllocator.init(std.testing.allocator);
//     defer arena.deinit();

//     var opts = Options{ .arena = arena.allocator(), .lib = .Angle, .commit_sha = "8a8c8fc280d74b34731e0e417b19bff7c967388a", .paths = .{
//         .python = "C:/Users/Reuben/AppData/Local/zig/p/1220f762a97c1e1613f7259ae688806289485fc5145e9453e7b6611a3f8afa0c0749/python.exe",
//         .depot_tools = "C:/Users/Reuben/AppData/Local/zig/p/122080075bb2fa27f94ac19d5fb2051f90b07027d305feb3e9073d501c7312704d09",
//         .src_dir = "C:/Users/Reuben/AppData/Local/zig/p/1220df877ce2ab2f8775207778ed97f1df3123447150066d5704cbf4678e8871e982",
//         .intermediate_dir = "E:/dev/handmade/orca/build.bak/",
//         .output_dir = "E:/dev/handmade/orca/build.bak/angle.out",
//     } };

//     try checkAngle(&opts);

//     opts.paths.output_dir = "E:/dev/handmade/orca/build.bak/angle.out/does_not_exist";
//     try std.testing.expectError(error.AngleOutOfDate, checkAngle(&opts));
// }

test "build angle" {
    var arena = std.heap.ArenaAllocator.init(std.testing.allocator);
    defer arena.deinit();

    var opts = Options{
        .arena = arena.allocator(),
        .lib = .Angle,
        .check_only = false,
        .optimize = .ReleaseFast,
        .commit_sha = "8a8c8fc280d74b34731e0e417b19bff7c967388a",
        .paths = .{
            .python = "C:/Users/Reuben/AppData/Local/zig/p/1220f762a97c1e1613f7259ae688806289485fc5145e9453e7b6611a3f8afa0c0749/python.exe",
            .src_dir = "C:/Users/Reuben/AppData/Local/zig/p/1220df877ce2ab2f8775207778ed97f1df3123447150066d5704cbf4678e8871e982",
            .intermediate_dir = "E:/dev/handmade/orca/build/",
        },
    };
    try buildAngle(&opts);
}
