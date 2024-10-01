const std = @import("std");
// const OptimizeMode = std.builtin.OptimizeMode;

const builtin = @import("builtin");

const MAX_FILE_SIZE = 1024 * 1024 * 128;

const Lib = enum {
    Angle,
    Dawn,
};

const Options = struct {
    arena: std.mem.Allocator,
    lib: Lib,
    commit_sha: []const u8,
    check_only: bool = false,
    optimize: std.builtin.OptimizeMode = .ReleaseFast,
    paths: struct {
        python: []const u8,
        depot_tools: []const u8,
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
        var depot_tools: ?[]const u8 = null;
        var src_dir: ?[]const u8 = null;
        var intermediate_dir: ?[]const u8 = null;
        var output_dir: ?[]const u8 = null;

        for (args) |raw_arg| {
            var splitIter = std.mem.splitScalar(u8, raw_arg, '=');
            const arg = splitIter.next();
            if (std.mem.eql(u8, arg, "--lib")) {
                if (splitIter.next()) |lib_str| {
                    if (std.mem.eql(lib_str, "angle")) {
                        lib = .Angle;
                    } else if (std.mem.eql(lib_str, "dawn")) {
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
            } else if (std.mem.eql(arg, "--debug")) {
                optimize = .Debug;
            } else if (std.mem.eql(arg, "--python")) {
                python = splitIter.next();
            } else if (std.mem.eql(arg, "--depot_tools")) {
                depot_tools = splitIter.next();
            } else if (std.mem.eql(arg, "--src")) {
                src_dir = splitIter.next();
            } else if (std.mem.eql(arg, "--intermediate")) {
                intermediate_dir = splitIter.next();
            } else if (std.mem.eql(arg, "--output")) {
                output_dir = splitIter.next();
            }

            // logic above should have consumed all tokens, if any are left it's an error
            if (splitIter.next() != null) {
                return error.InvalidArgument;
            }
        }

        if (lib == null or commit_sha == null or python == null or depot_tools == null or src_dir == null or intermediate_dir == null or output_dir == null) {
            return error.MissingRequiredArgument;
        }

        return .{
            .arena = arena,
            .lib = lib.?,
            .commit_sha = commit_sha.?,
            .check_only = check_only,
            .optimize = optimize,
            .paths = .{
                .python = python.?,
                .depot_tools = depot_tools.?,
                .src_dir = src_dir.?,
                .intermediate_dir = intermediate_dir.?,
                .output_dir = output_dir.?,
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
    std.debug.print("running: ", .{});
    for (argv) |arg| {
        std.debug.print("{s}", .{arg});
    }
    std.debug.print("\n", .{});

    const result = try std.process.Child.run(.{
        .allocator = arena,
        .argv = argv,
        .cwd = cwd,
        .env_map = env,
    });
    switch (result.term) {
        .Exited => |v| {
            if (v != 0) {
                std.log.err("process {s} exited with nonzero exit code {}", .{ argv[0], v });
                return error.NonZeroExitCode;
            }
        },
        else => {
            std.log.err("process {s} exited abnormally", .{argv[0]});
            return error.AbnormalExit;
        },
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

fn checksumAngle(opts: *const Options) ![]const u8 {
    return try Checksum.dir(opts.arena, opts.paths.output_dir, .{ .exclude_files = &.{
        ANGLE_CHECKSUM_FILENAME,
        ".DS_Store",
    } });
}

fn isAngleUpToDate(opts: *const Options) bool {
    const sum = checksumAngle(opts) catch |e| {
        std.log.err("Failed checksum dir '{s}': {}\n", .{ opts.paths.output_dir, e });
        return false;
    };

    const checksum_path = std.fs.path.join(opts.arena, &.{ opts.paths.output_dir, ANGLE_CHECKSUM_FILENAME }) catch @panic("OOM");
    const json_data: []const u8 = std.fs.cwd().readFileAlloc(opts.arena, checksum_path, MAX_FILE_SIZE) catch |e| {
        std.log.err("Failed to read checksum file from location '{s}': {}", .{ checksum_path, e });
        return false;
    };

    const loaded_checksum = std.json.parseFromSliceLeaky(CommitChecksum, opts.arena, json_data, .{}) catch |e| {
        std.log.err("Failed to parse file '{s}' json: {}. Raw json data:\n{s}\n", .{
            checksum_path,
            e,
            json_data,
        });
        return false;
    };
    if (std.mem.eql(u8, loaded_checksum.commit, opts.commit_sha) == false) {
        std.log.err("{s} doesn't match the required angle commit. expected {s}, got {s}", .{
            checksum_path,
            opts.commit_sha,
            loaded_checksum.commit,
        });
        return false;
    }

    if (std.mem.eql(u8, loaded_checksum.sum, sum) == false) {
        std.log.err("{s} doesn't match checksum. expected {s}, got {s}", .{
            checksum_path,
            loaded_checksum.commit,
            sum,
        });
        return false;
    }

    return true;
}

fn checkAngle(opts: *const Options) !void {
    if (isAngleUpToDate(opts) == false) {
        return error.AngleOutOfDate;
    }
}

fn buildAngle(opts: *const Options) !void {
    if (isAngleUpToDate(opts)) {
        std.debug.print("angle is up to date - no rebuild needed.\n", .{});
        return;
    } else if (opts.check_only) {
        return error.AngleOutOfDate;
    }

    var env: std.process.EnvMap = try std.process.getEnvMap(opts.arena);
    defer env.deinit();
    if (builtin.os.tag == .windows) {
        try env.put("DEPOT_TOOLS_WIN_TOOLCHAIN", "0");
    }
    // TODO add depot_tools to PATH

    const bootstrap_path = try std.fs.path.join(opts.arena, &.{ opts.paths.src_dir, "scripts/bootstrap.py" });
    try exec(opts.arena, &.{ opts.paths.python, bootstrap_path }, opts.paths.intermediate_dir, &env);

    try exec(opts.arena, &.{ "gclient", "sync" }, opts.paths.intermediate_dir, &env);

    // const angle_bootstrap = Step.Run.create(b, "run python");

    // if (target.result.os.tag == .windows) {
    //     angle_bootstrap.setEnvironmentVariable("DEPOT_TOOLS_WIN_TOOLCHAIN", "0");
    //     if (b.lazyDependency("python3-win64", .{})) |python3_dep| {
    //         _ = python3_dep;
    //         // angle_bootstrap.addFileArg(python3_dep.path("python.exe"));
    //     }
    // } else {
    //     return error.UnhandledOS; // TODO handle macOS
    //     // angle_bootstrap.addArgs(.{"python"});
    // }
    // angle_bootstrap.addArgs(&.{"python"});

    // angle_bootstrap.setCwd(angle_build_dir);
    // angle_bootstrap.addPathDir(depot_tools_dir);
    // angle_bootstrap.addFileArg(b.path("scripts/bootstrap.py"));
    // angle_bootstrap.step.dependOn(&stage_angle_repo.step);

    // const angle_gclient_sync = b.addSystemCommand(&.{ "gclient", "sync" });
    // angle_gclient_sync.setCwd(angle_build_dir);
    // angle_gclient_sync.addPathDir(depot_tools_dir);
    // if (target.result.os.tag == .windows) {
    //     angle_gclient_sync.setEnvironmentVariable("DEPOT_TOOLS_WIN_TOOLCHAIN", "0");
    // }
    // angle_gclient_sync.step.dependOn(&angle_bootstrap.step);

    const optimize_str = if (opts.optimize == .Debug) "Debug" else "Release";
    const is_debug_str = if (opts.optimize == .Debug) "is_debug=True" else "is+_debug=False";
    // const out_dir = try std.fmt.allocPrint("out/{s}", .{angle_optimize_str});
    // const out_dir = try std.fmt.allocPrint("out/{s}", .{angle_optimize_str});

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

    // const angle_gn = b.addSystemCommand(&.{ "gn", "gen", angle_out_dir, b.fmt("--args={s}", .{gn_args_str}) });
    // angle_gn.setCwd(angle_build_dir);
    // angle_gn.addPathDir(depot_tools_dir);
    // angle_gn.step.dependOn(&angle_gclient_sync.step);

    const gn_args: []const u8 = try std.fmt.allocPrint(opts.arena, "--args={s}", .{gn_all_args});

    const optimize_output_dir = try std.fs.path.join(opts.arena, &.{ opts.paths.output_dir, optimize_str });

    try exec(opts.arena, &.{ "gn", "gen", optimize_output_dir, gn_args }, opts.paths.intermediate_dir, &env);

    try exec(opts.arena, &.{ "autoninja", "-C", optimize_output_dir, "libEGL", "libGLESv2" }, opts.paths.intermediate_dir, &env);

    // write stamp file
    {
        const commit_checksum = CommitChecksum{
            .sum = try checksumAngle(opts),
            .commit = opts.commit_sha,
        };
        var json = std.ArrayList(u8).init(opts.arena);
        try std.json.stringify(commit_checksum, .{}, json.writer());
        const checksum_path = std.fs.path.join(opts.arena, &.{ opts.paths.output_dir, ANGLE_CHECKSUM_FILENAME }) catch @panic("OOM");
        try std.fs.cwd().writeFile(.{
            .sub_path = checksum_path,
            .data = json.items,
            .flags = .{},
        });
    }

    // stringify

    // const angle_autoninja = b.addSystemCommand(&.{ "autoninja", "-C", angle_out_dir, "libEGL", "libGLESv2" });
    // angle_autoninja.setCwd(angle_build_dir);
    // angle_autoninja.addPathDir(depot_tools_dir);
    // angle_autoninja.step.dependOn(&angle_gn.step);

    // const angle_sentinel = AngleDawnHelpers.WriteBuildSentinel.create(b, .Angle);
    // angle_sentinel.step.dependOn(&angle_autoninja.step);
    // angle_sentinel.step.dependOn(&angle_bootstrap.step);
}

fn buildDawn(opts: *const Options) !void {
    _ = opts;
}

pub fn main() !void {
    var arena = std.heap.ArenaAllocator.init(std.heap.page_allocator);
    defer arena.deinit();
    const allocator: std.mem.Allocator = arena.allocator();

    const args: []const [:0]u8 = try std.process.argsAlloc(allocator);
    defer std.process.argsFree(allocator, args);

    const opts = try Options.parse(args, allocator);
    switch (opts.lib) {
        .Angle => try buildAngle(opts),
        .Dawn => try buildDawn(opts),
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
        .commit_sha = "8a8c8fc280d74b34731e0e417b19bff7c967388a",
        .paths = .{
            .python = "C:/Users/Reuben/AppData/Local/zig/p/1220f762a97c1e1613f7259ae688806289485fc5145e9453e7b6611a3f8afa0c0749/python.exe",
            .depot_tools = "C:/Users/Reuben/AppData/Local/zig/p/122080075bb2fa27f94ac19d5fb2051f90b07027d305feb3e9073d501c7312704d09",
            .src_dir = "C:/Users/Reuben/AppData/Local/zig/p/1220df877ce2ab2f8775207778ed97f1df3123447150066d5704cbf4678e8871e982",
            .intermediate_dir = "E:/dev/handmade/orca/build.bak/",
            .output_dir = "E:/dev/handmade/orca/build.bak/angle.out",
        },
    };
    try buildAngle(&opts);
}
