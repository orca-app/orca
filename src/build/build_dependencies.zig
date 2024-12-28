// This is a zig program run as part of the orca build process to build angle and dawn
// libraries from source.

const std = @import("std");

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
        cmake: []const u8,
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
        var cmake: ?[]const u8 = null;
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
            } else if (std.mem.eql(u8, arg, "--cmake")) {
                cmake = splitIter.next();
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
        } else if (cmake == null) {
            missing_arg = "cmake";
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
                .cmake = cmake.?,
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

fn exec(arena: std.mem.Allocator, argv: []const []const u8, cwd: []const u8, env: *const std.process.EnvMap) !void {
    var log_msg = std.ArrayList(u8).init(arena);
    var log_writer = log_msg.writer();
    try log_writer.print("running: ", .{});
    for (argv) |arg| {
        try log_writer.print("{s} ", .{arg});
    }
    try log_writer.print(" in dir {s}", .{cwd});
    std.log.info("{s}\n", .{log_msg.items});

    var process = std.process.Child.init(argv, arena);
    process.stdin_behavior = .Ignore;
    process.cwd = cwd;
    process.env_map = env;
    process.stdout_behavior = .Inherit;
    process.stderr_behavior = .Inherit;

    try process.spawn();

    const term = try process.wait();

    switch (term) {
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

    std.debug.print("\n", .{});
}

fn execShell(arena: std.mem.Allocator, argv: []const []const u8, cwd: []const u8, env: *const std.process.EnvMap) !void {
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
        // std.debug.print("\t{s}\n", .{src_entry.path});
        _ = switch (src_entry.kind) {
            .directory => try dest_dir.makePath(src_entry.path),
            .file => try src_dir.updateFile(src_entry.path, dest_dir, src_entry.path, .{}),
            else => {},
        };
    }
}

const CommitStamp = struct {
    commit: []const u8,

    fn write(opts: *const Options, path: []const u8) !void {
        std.log.info("writing commit file to {s}", .{path});

        const cwd = std.fs.cwd();

        try cwd.makePath(opts.paths.output_dir);

        var json_buffer = std.ArrayList(u8).init(opts.arena);
        var writer = std.json.writeStream(json_buffer.writer(), .{ .whitespace = .indent_4 });
        defer writer.deinit();

        try writer.beginObject();
        {
            try writer.objectField("commit");
            try writer.write(opts.commit_sha);
        }
        try writer.endObject();

        try cwd.writeFile(.{
            .sub_path = path,
            .data = json_buffer.items,
            .flags = .{},
        });
    }
};

const ANGLE_COMMIT_FILENAME = "angle.json";
const DAWN_COMMIT_FILENAME = "dawn.json";

fn ensureDepotTools(opts: *const Options) !std.process.EnvMap {
    var env: std.process.EnvMap = try std.process.getEnvMap(opts.arena);
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

    return env;
}

fn noopLog(comptime _: []const u8, _: anytype) void {}

const ShouldLogError = enum {
    LogError,
    NoError,
};

fn isLibUpToDate(opts: *const Options, comptime log_error: ShouldLogError) bool {
    const logfn = if (log_error == .LogError) &std.log.err else &noopLog;

    const commit_filename: []const u8 = switch (opts.lib) {
        .Angle => ANGLE_COMMIT_FILENAME,
        .Dawn => DAWN_COMMIT_FILENAME,
    };

    const commit_stamp_path = std.fs.path.join(opts.arena, &.{ opts.paths.output_dir, commit_filename }) catch @panic("OOM");
    const json_data: []const u8 = std.fs.cwd().readFileAlloc(opts.arena, commit_stamp_path, MAX_FILE_SIZE) catch |e| {
        logfn("Failed to read commit file from location '{s}': {}", .{ commit_stamp_path, e });
        return false;
    };

    const loaded_stamp = std.json.parseFromSliceLeaky(CommitStamp, opts.arena, json_data, .{}) catch |e| {
        logfn("Failed to parse file '{s}' json: {}. Raw json data:\n{s}\n", .{
            commit_stamp_path,
            e,
            json_data,
        });
        return false;
    };
    if (std.mem.eql(u8, loaded_stamp.commit, opts.commit_sha) == false) {
        logfn("{s} doesn't match the required angle commit. expected {s}, got {s}", .{
            commit_stamp_path,
            opts.commit_sha,
            loaded_stamp.commit,
        });
        return false;
    }

    return true;
}

fn checkAngle(opts: *const Options) !void {
    if (isLibUpToDate(opts, .LogError) == false) {
        return error.AngleOutOfDate;
    }
}

fn buildAngle(opts: *const Options) !void {
    if (isLibUpToDate(opts, .NoError)) {
        // std.log.info("angle is up to date - no rebuild needed.\n", .{});
        return;
    } else if (opts.check_only) {
        const msg =
            \\Angle files are not present or don't match required commit.
            \\Angle commit: {s}
            \\
            \\You can build the required files by running 'zig build angle'
            \\
            \\Alternatively you can trigger a CI run to build the binaries on github:
            \\  * For Windows, go to https://github.com/orca-app/orca/actions/workflows/build-angle-win.yaml
            \\  * For macOS, go to https://github.com/orca-app/orca/actions/workflows/build-angle-mac.yaml
            \\  * Click on \"Run workflow\" to tigger a new run, or download artifacts from a previous run
            \\  * Put the contents of the artifacts folder in './build/angle.out'
        ;
        std.log.err(msg, .{opts.commit_sha});
        return error.AngleOutOfDate;
    }

    const cwd = std.fs.cwd();
    try cwd.makePath(opts.paths.intermediate_dir);

    std.log.info("angle is out of date - rebuilding", .{});

    var env: std.process.EnvMap = try ensureDepotTools(opts);
    defer env.deinit();

    const src_path = try std.fs.path.join(opts.arena, &.{ opts.paths.intermediate_dir, opts.lib.toStr() });
    try copyFolder(opts.arena, src_path, opts.paths.src_dir);

    const bootstrap_path = try std.fs.path.join(opts.arena, &.{ src_path, "scripts/bootstrap.py" });
    try exec(opts.arena, &.{ opts.paths.python, bootstrap_path }, src_path, &env);

    try execShell(opts.arena, &.{ "gclient", "sync" }, src_path, &env);

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

    const optimize_output_path = try std.fs.path.join(opts.arena, &.{ src_path, "out", optimize_str });
    try cwd.makePath(optimize_output_path);

    try execShell(opts.arena, &.{ "gn", "gen", optimize_output_path, gn_args }, src_path, &env);

    try execShell(opts.arena, &.{ "autoninja", "-C", optimize_output_path, "libEGL", "libGLESv2" }, src_path, &env);

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
            const src_include_path = try join(a, &.{ src_path, folder });
            const dest_include_path = try join(a, &.{ output_dir, folder });
            try cwd.deleteTree(dest_include_path);
            try cwd.makePath(dest_include_path);
            _ = try copyFolder(a, dest_include_path, src_include_path);
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

    // write commit stamp file
    const commit_stamp_path = try std.fs.path.join(opts.arena, &.{ opts.paths.output_dir, ANGLE_COMMIT_FILENAME });
    try CommitStamp.write(opts, commit_stamp_path);

    std.log.info("angle build successful", .{});
}

fn buildDawn(opts: *const Options) !void {
    if (isLibUpToDate(opts, .NoError)) {
        // std.log.info("dawn is up to date - no rebuild needed.\n", .{});
        return;
    } else if (opts.check_only) {
        const msg =
            \\Dawn files are not present or don't match required commit.
            \\Dawn commit: {s}
            \\You can build the required files by running 'zig build dawn'
            \\
            \\Alternatively you can trigger a CI run to build the binaries on github:
            \\  * For Windows, go to https://github.com/orca-app/orca/actions/workflows/build-dawn-win.yaml
            \\  * For macOS, go to https://github.com/orca-app/orca/actions/workflows/build-dawn-mac.yaml
            \\  * Click on "Run workflow" to tigger a new run, or download artifacts from a previous run
            \\  * Put the contents of the artifacts folder in './build/dawn.out'
        ;
        std.log.err(msg, .{opts.commit_sha});

        return error.DawnOutOfDate;
    }

    std.log.info("dawn is out of date - rebuilding", .{});

    var env: std.process.EnvMap = try ensureDepotTools(opts);
    defer env.deinit();

    const cwd = std.fs.cwd();

    const src_path = try std.fs.path.join(opts.arena, &.{ opts.paths.intermediate_dir, opts.lib.toStr() });
    try copyFolder(opts.arena, src_path, opts.paths.src_dir);

    const src_dir = try cwd.openDir(src_path, .{});
    _ = try src_dir.updateFile("scripts/standalone.gclient", src_dir, ".gclient", .{});

    try execShell(opts.arena, &.{ "gclient", "sync" }, src_path, &env);

    {
        const cmake_patch =
            \\add_library(webgpu SHARED ${DAWN_PLACEHOLDER_FILE})
            \\common_compile_options(webgpu)
            \\target_link_libraries(webgpu PRIVATE dawn_native)
            \\target_link_libraries(webgpu PUBLIC dawn_headers)
            \\target_compile_definitions(webgpu PRIVATE WGPU_IMPLEMENTATION WGPU_SHARED_LIBRARY)
            \\target_sources(webgpu PRIVATE ${WEBGPU_DAWN_NATIVE_PROC_GEN})
            \\
        ;
        const cmake_list_path = try std.fs.path.join(opts.arena, &.{ src_path, "src/dawn/native/CMakeLists.txt" });
        const cmake_list_file = try cwd.createFile(cmake_list_path, .{
            .read = false,
            .truncate = false,
        });
        defer cmake_list_file.close();

        try cmake_list_file.seekFromEnd(0);
        try cmake_list_file.writeAll(cmake_patch);
    }

    const diff_file_path = try std.fs.path.join(opts.arena, &.{ src_path, "../../deps/dawn-d3d12-transparent.diff" });
    try exec(opts.arena, &.{ "git", "apply", "-v", diff_file_path }, src_path, &env); // TODO maybe use --unsafe-paths ?

    const optimize_str = if (opts.optimize == .Debug) "Debug" else "Release";
    const cmake_build_type = try std.fmt.allocPrint(opts.arena, "CMAKE_BUILD_TYPE={s}", .{optimize_str});

    var cmake_args = std.ArrayList([]const u8).init(opts.arena);
    // zig fmt: off
    try cmake_args.appendSlice(&.{
            opts.paths.cmake,
            "-S", "dawn",
            "-B", "dawn.build",
            "-D", cmake_build_type,
            "-D", "CMAKE_POLICY_DEFAULT_CMP0091=NEW",
            "-D", "BUILD_SHARED_LIBS=OFF",
            "-D", "BUILD_SAMPLES=ON",
            "-D", "DAWN_BUILD_SAMPLES=ON",
            "-D", "TINT_BUILD_SAMPLES=OFF",
            "-D", "TINT_BUILD_DOCS=OFF",
            "-D", "TINT_BUILD_TESTS=OFF",
    });
    // zig fmt: on

    // zig fmt: off
    if (builtin.os.tag == .windows) {
        try cmake_args.appendSlice(&.{
            "-D", "DAWN_ENABLE_D3D12=ON",
            "-D", "DAWN_ENABLE_D3D11=OFF",
            "-D", "DAWN_ENABLE_METAL=OFF",
            "-D", "DAWN_ENABLE_NULL=OFF",
            "-D", "DAWN_ENABLE_DESKTOP_GL=OFF",
            "-D", "DAWN_ENABLE_OPENGLES=OFF",
            "-D", "DAWN_ENABLE_VULKAN=OFF"
        });
    } else {
        try cmake_args.appendSlice(&.{
            "-D", "DAWN_ENABLE_METAL=ON",
            "-D", "DAWN_ENABLE_NULL=OFF",
            "-D", "DAWN_ENABLE_DESKTOP_GL=OFF",
            "-D", "DAWN_ENABLE_OPENGLES=OFF",
            "-D", "DAWN_ENABLE_VULKAN=OFF"
        });
    }
    // zig fmt: on

    try exec(opts.arena, cmake_args.items, opts.paths.intermediate_dir, &env);

    // TODO allow user customization of number of parallel jobs
    // zig fmt: off
    const cmake_build_args = &.{
        opts.paths.cmake,
        "--build", "dawn.build",
        "--config", optimize_str,
        "--target", "webgpu",
        "--parallel",
    };
    // zig fmt: on
    try exec(opts.arena, cmake_build_args, opts.paths.intermediate_dir, &env);

    const commit_stamp_path = try std.fs.path.join(opts.arena, &.{ opts.paths.output_dir, DAWN_COMMIT_FILENAME });
    try CommitStamp.write(opts, commit_stamp_path);
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
