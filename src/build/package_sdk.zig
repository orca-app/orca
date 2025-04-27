// This is a zig program run as part of the orca build process to copy all appropriate
// build artifacts to an output location. Optionally the user can package the SDK
// for installation to the system orca path.

const std = @import("std");
const builtin = @import("builtin");

const join = std.fs.path.join;
const assert = std.debug.assert;

fn trimWhitespace(str0: []const u8) []const u8 {
    const trim = std.mem.trim;
    const str1 = trim(u8, str0, "\n");
    const str2 = trim(u8, str1, "\r");
    const str3 = trim(u8, str2, " ");
    return str3;
}

fn findGitVersion(arena: std.mem.Allocator) ![]const u8 {
    const result: std.process.Child.RunResult = try std.process.Child.run(.{
        .allocator = arena,
        .argv = &.{ "git", "rev-parse", "--short", "HEAD" },
        .cwd_dir = std.fs.cwd(),
    });

    switch (result.term) {
        .Exited => |exit_code| {
            if (exit_code != 0) {
                std.log.err("git rev-parse failed with nonzero exit code {}", .{exit_code});
                return error.GitRevParseFailed;
            }
        },
        else => {
            std.log.err("git rev-parse failed. stderr: {s}", .{result.stderr});
            return error.GitRevParseFailed;
        },
    }

    const output = trimWhitespace(result.stdout);
    if (output.len == 0) {
        std.log.err("git rev-parse output was empty", .{});
        return error.GitRevParseUnexpectedOutput;
    }

    return output;
}

fn indexOfString(haystack: []const []const u8, needle: []const u8) ?usize {
    for (haystack, 0..) |item, i| {
        if (std.mem.eql(u8, item, needle)) {
            return i;
        }
    }
    return null;
}

const CopyFolderOpts = struct {
    include_extensions: []const []const u8 = &.{},
    ignore_patterns: []const []const u8 = &.{},
    required_filenames: []const []const u8 = &.{},
};

fn copyFolder(
    allocator: std.mem.Allocator,
    dest: []const u8,
    src: []const u8,
    opts: *const CopyFolderOpts,
) !void {
    std.log.info("copying '{s}' to '{s}'", .{ src, dest });

    const cwd = std.fs.cwd();
    try cwd.makePath(dest);

    const src_dir: std.fs.Dir = try cwd.openDir(src, .{ .iterate = true });
    const dest_dir: std.fs.Dir = try cwd.openDir(dest, .{ .iterate = true });

    var required_filenames = std.ArrayList([]const u8).init(allocator);
    try required_filenames.appendSlice(opts.required_filenames);

    var normalized_ignore_patterns = std.ArrayList([]u8).init(allocator);
    for (opts.ignore_patterns) |pattern| {
        try normalized_ignore_patterns.append(try std.fs.path.resolve(allocator, &.{pattern}));
    }

    var src_walker = try src_dir.walk(allocator);
    while (try src_walker.next()) |src_entry| {
        var included: bool = true;
        if (src_entry.kind != .directory) {
            included = opts.include_extensions.len == 0;
            const extension = std.fs.path.extension(src_entry.basename);
            for (opts.include_extensions) |included_extension| {
                if (std.mem.eql(u8, extension, included_extension)) {
                    included = true;
                    break;
                }
            }
        }

        var ignored: bool = false;
        for (normalized_ignore_patterns.items) |pattern| {
            if (std.mem.indexOf(u8, src_entry.path, pattern) != null) {
                ignored = true;
                break;
            }
        }

        var finish: bool = false;
        if (ignored == false and src_entry.kind == .file and required_filenames.items.len > 0) {
            ignored = true;
            for (required_filenames.items, 0..) |filename, i| {
                if (std.mem.eql(u8, filename, src_entry.path)) {
                    ignored = false;
                    _ = required_filenames.swapRemove(i);
                    finish = required_filenames.items.len == 0;
                    break;
                }
            }
        }

        if (included and !ignored) {
            if (src_entry.kind == .file) {
                if (opts.required_filenames.len > 0) {
                    const src_file_path: []const u8 = try join(allocator, &.{ src, src_entry.path });
                    const dest_file_path: []const u8 = try join(allocator, &.{ dest, src_entry.path });
                    std.log.info("copying '{s}' to '{s}'", .{ src_file_path, dest_file_path });
                }
                _ = try src_dir.updateFile(src_entry.path, dest_dir, src_entry.path, .{});
            }
        }

        if (finish) {
            break;
        }
    }

    if (required_filenames.items.len > 0) {
        const joined = try std.mem.join(allocator, ", ", required_filenames.items);
        std.log.err("Failed to find required files: {s}", .{joined});
        return error.FailedToCopyRequiredFiles;
    }
}

const ShouldLog = enum {
    False,
    True,
};

fn findSystemOrcaDir(allocator: std.mem.Allocator, should_log: ShouldLog) ![]const u8 {
    const result: std.process.Child.RunResult = std.process.Child.run(.{
        .allocator = allocator,
        .argv = &.{ "orca", "install-path" },
        .cwd_dir = std.fs.cwd(),
    }) catch |e| {
        if (should_log == .True) {
            std.log.err("Failed to run orca executable in PATH.", .{});
        }
        return e;
    };

    switch (result.term) {
        .Exited => |exit_code| {
            if (exit_code != 0) {
                if (should_log == .True) {
                    std.log.err("orca install-path failed with nonzero exit code {}", .{exit_code});
                }
                return error.OrcaInstallPathFailed;
            }
        },
        else => {
            if (should_log == .True) {
                std.log.err("orca install-path failed. stderr: {s}", .{result.stderr});
            }
            return error.OrcaInstallPathFailed;
        },
    }

    const output = trimWhitespace(result.stdout);
    if (output.len == 0) {
        if (should_log == .True) {
            std.log.err("orca install-path output was empty", .{});
        }
        return error.OrcaInstallPathUnexpectedOutput;
    }

    return output;
}

const Options = struct {
    arena: std.mem.Allocator,
    sdk_path: []const u8,
    sdk_deps_path_opt: ?[]const u8,
    artifacts_path: []const u8,
    resources_path: []const u8,
    src_path: []const u8,
    version: []const u8,
    is_dev_install: bool,

    fn parse(args: []const [:0]const u8, arena: std.mem.Allocator) !Options {
        var sdk_path: ?[]const u8 = null;
        var sdk_deps_path: ?[]const u8 = null;
        var artifacts_path: ?[]const u8 = null;
        var resources_path: ?[]const u8 = null;
        var src_path: ?[]const u8 = null;
        var version: ?[]const u8 = null;
        var is_dev_install: bool = false;

        for (args, 0..) |raw_arg, i| {
            if (i == 0) {
                continue;
            }

            var splitIter = std.mem.splitScalar(u8, raw_arg, '=');
            const arg: []const u8 = splitIter.next().?;
            if (std.mem.eql(u8, arg, "--sdk-path")) {
                sdk_path = splitIter.next();
            } else if (std.mem.eql(u8, arg, "--sdk-deps-path")) {
                sdk_deps_path = splitIter.next();
            } else if (std.mem.eql(u8, arg, "--artifacts-path")) {
                artifacts_path = splitIter.next();
            } else if (std.mem.eql(u8, arg, "--resources-path")) {
                resources_path = splitIter.next();
            } else if (std.mem.eql(u8, arg, "--src-path")) {
                src_path = splitIter.next();
            } else if (std.mem.eql(u8, arg, "--version")) {
                version = splitIter.next();
            } else if (std.mem.eql(u8, arg, "--dev-install")) {
                is_dev_install = true;
            }
        }

        var missing_arg: ?[]const u8 = null;
        if (sdk_path == null and is_dev_install == false) {
            missing_arg = "sdk-path";
        } else if (artifacts_path == null) {
            missing_arg = "artifacts-path";
        } else if (resources_path == null) {
            missing_arg = "resources-path";
        } else if (src_path == null) {
            missing_arg = "src-path";
        }

        if (missing_arg) |arg| {
            std.log.err("Missing required arg: {s}", .{arg});
            return error.MissingRequiredArgument;
        }

        if (version == null or std.mem.eql(u8, version.?, "")) {
            const git_version = try findGitVersion(arena);
            version = try std.mem.join(arena, "", &.{ "dev-", git_version });
            std.log.info("No version supplied, using version derived from git sha: {s}", .{version.?});
        }

        if (is_dev_install) {
            if (sdk_path == null) {
                const orca_dir: []const u8 = findSystemOrcaDir(arena, ShouldLog.False) catch |e| {
                    std.log.err(
                        \\When performing a dev install, you must either have a version of the orca CLI tool installed on your
                        \\PATH so orca knows where to install itself, or you must explicitly supply zig build with the arg:
                        \\    -Dsdk-path=<path>
                        \\
                    , .{});
                    return e;
                };
                sdk_path = try join(arena, &.{ orca_dir, version.? });
            } else {
                sdk_path = try join(arena, &.{ sdk_path.?, version.? });
            }
        }

        return Options{
            .arena = arena,
            .sdk_path = sdk_path.?,
            .sdk_deps_path_opt = sdk_deps_path,
            .artifacts_path = artifacts_path.?,
            .resources_path = resources_path.?,
            .src_path = src_path.?,
            .version = version.?,
            .is_dev_install = is_dev_install,
        };
    }
};

pub fn main() !void {
    var arena = std.heap.ArenaAllocator.init(std.heap.page_allocator);
    defer arena.deinit();
    const allocator: std.mem.Allocator = arena.allocator();

    const args: []const [:0]u8 = try std.process.argsAlloc(allocator);
    defer std.process.argsFree(allocator, args);

    const opts = try Options.parse(args, allocator);
    const cwd = std.fs.cwd();

    // package SDK
    {
        std.fs.deleteTreeAbsolute(opts.sdk_path) catch {};

        const src_bin_path = try join(opts.arena, &.{ opts.artifacts_path, "bin" });
        const dest_bin_path = try join(opts.arena, &.{ opts.sdk_path, "bin" });
        const bin_files_windows: []const []const u8 = &.{
            "orca_tool.exe",
            "orca_platform.dll",
            "orca_runtime.exe",
            "liborca_wasm.a",
            "libEGL.dll",
            "libGLESv2.dll",
            "webgpu.dll",
        };
        const bin_files_macos: []const []const u8 = &.{
            "orca_tool",
            "orca_runtime",
            "liborca_platform.dylib",
            "liborca_wasm.a",
            "libEGL.dylib",
            "libGLESv2.dylib",
            "libwebgpu.dylib",
        };
        const bin_files = if (builtin.os.tag == .windows) bin_files_windows else bin_files_macos;
        try copyFolder(opts.arena, dest_bin_path, src_bin_path, &.{ .required_filenames = bin_files });

        const src_paths: []const []const u8 = &.{
            try join(opts.arena, &.{ opts.artifacts_path, "orca-libc" }),
            opts.resources_path,
        };

        const dest_paths: []const []const u8 = &.{
            try join(opts.arena, &.{ opts.sdk_path, "orca-libc" }),
            try join(opts.arena, &.{ opts.sdk_path, "resources" }),
        };

        for (src_paths, dest_paths) |src, dest| {
            try copyFolder(opts.arena, dest, src, &.{});
        }

        const header_extensions: []const []const u8 = &.{
            ".h",
        };
        // For copying the src directory, manually copy the curl and wasm3 dirs since we only care
        // about the headers in a specific directory. Everything else is ok though
        {
            const dest_src_path = try join(opts.arena, &.{ opts.sdk_path, "src" });
            const ignore_patterns: []const []const u8 = &.{
                "tool/",
                "orca-libc/",
                "wasm/",
                "ext/curl/",
                "ext/wasm3/",
                "ext/zlib/build/", // copy all headers in zlib except zlib/build
            };
            try copyFolder(opts.arena, dest_src_path, opts.src_path, &.{ .include_extensions = header_extensions, .ignore_patterns = ignore_patterns });
        }

        {
            const curl_src_path = try std.fs.path.resolve(opts.arena, &.{ opts.src_path, "ext/curl/include" });
            const curl_dest_path = try std.fs.path.resolve(opts.arena, &.{ opts.sdk_path, "src/ext/curl/include" });

            try copyFolder(opts.arena, curl_dest_path, curl_src_path, &.{ .include_extensions = header_extensions });
        }

        {
            const wasm3_src_path = try std.fs.path.resolve(opts.arena, &.{ opts.src_path, "ext/wasm3/source" });
            const wasm3_dest_path = try std.fs.path.resolve(opts.arena, &.{ opts.sdk_path, "src/ext/wasm3/source" });
            try copyFolder(opts.arena, wasm3_dest_path, wasm3_src_path, &.{ .include_extensions = header_extensions });
        }

        // dev installs need to overwrite the orca tool in case there were any changes
        if (opts.is_dev_install) {
            const orca_dir_path: []const u8 = if (std.fs.path.dirname(opts.sdk_path)) |orca_dir| orca_dir else opts.sdk_path;

            const src_orca_exe_name: []const u8 = if (builtin.os.tag == .windows) "orca_tool.exe" else "orca_tool";
            const src_tool_path: []const u8 = try join(opts.arena, &.{ opts.artifacts_path, "bin", src_orca_exe_name });

            const dest_orca_exe_name: []const u8 = if (builtin.os.tag == .windows) "orca.exe" else "orca";
            const dest_tool_path: []const u8 = try join(opts.arena, &.{ orca_dir_path, dest_orca_exe_name });

            std.log.info("copying '{s}' to '{s}'", .{ src_tool_path, dest_tool_path });
            const orca_dir: std.fs.Dir = try cwd.openDir(orca_dir_path, .{ .iterate = true });
            _ = try cwd.updateFile(src_tool_path, orca_dir, dest_tool_path, .{});

            // copy pdb file as well since windows debuggers have a hard time finding the debug symbols otherwise
            if (builtin.os.tag == .windows) {
                const src_pdb_path: []const u8 = try join(opts.arena, &.{ opts.artifacts_path, "bin", "orca_tool.pdb" });
                const dest_pdb_path: []const u8 = try join(opts.arena, &.{ orca_dir_path, "orca.pdb" });
                std.log.info("copying '{s}' to '{s}'", .{ src_pdb_path, dest_pdb_path });
                _ = try cwd.updateFile(src_pdb_path, orca_dir, dest_pdb_path, .{});
            }

            try orca_dir.writeFile(.{
                .sub_path = "current_version",
                .data = opts.version,
            });
        }

        std.log.info("Packaged Orca SDK to {s}\n", .{opts.sdk_path});
    }

    if (opts.sdk_deps_path_opt) |sdk_deps_path| {
        std.fs.deleteTreeAbsolute(sdk_deps_path) catch {};

        const src_angle_json_path = try join(opts.arena, &.{ opts.artifacts_path, "angle.out", "angle.json" });
        const dest_angle_json_path = try join(opts.arena, &.{ sdk_deps_path, "angle.json" });
        assert(std.fs.path.isAbsolute(src_angle_json_path));
        assert(std.fs.path.isAbsolute(dest_angle_json_path));
        _ = try std.fs.Dir.updateFile(cwd, src_angle_json_path, cwd, dest_angle_json_path, .{});

        const src_dawn_json_path = try join(opts.arena, &.{ opts.artifacts_path, "dawn.out", "dawn.json" });
        const dest_dawn_json_path = try join(opts.arena, &.{ sdk_deps_path, "dawn.json" });
        assert(std.fs.path.isAbsolute(src_dawn_json_path));
        assert(std.fs.path.isAbsolute(dest_dawn_json_path));
        _ = try std.fs.Dir.updateFile(cwd, src_dawn_json_path, cwd, dest_dawn_json_path, .{});

        const src_bin_path = try join(opts.arena, &.{ opts.artifacts_path, "bin" });
        const dest_bin_path = try join(opts.arena, &.{ sdk_deps_path, "bin" });
        const bin_files_windows: []const []const u8 = &.{
            "libEGL.dll",
            "libEGL.dll.lib",
            "libGLESv2.dll",
            "libGLESv2.dll.lib",
            "webgpu.dll",
            "webgpu.lib",
            "d3dcompiler_47.dll",
        };
        const bin_files_macos: []const []const u8 = &.{
            "libEGL.dylib",
            "libGLESv2.dylib",
            "libwebgpu.dylib",
        };
        const bin_files = if (builtin.os.tag == .windows) bin_files_windows else bin_files_macos;
        try copyFolder(opts.arena, dest_bin_path, src_bin_path, &.{ .required_filenames = bin_files });

        const src_angle_include_path = try join(opts.arena, &.{ opts.artifacts_path, "angle.out", "include" });
        const dest_angle_include_path = try join(opts.arena, &.{ sdk_deps_path, "src", "ext", "angle", "include" });
        try cwd.makePath(dest_angle_include_path);
        try copyFolder(opts.arena, dest_angle_include_path, src_angle_include_path, &.{ .include_extensions = &.{".h"} });

        const src_dawn_include_path = try join(opts.arena, &.{ opts.artifacts_path, "dawn.out", "include" });
        const dest_dawn_include_path = try join(opts.arena, &.{ sdk_deps_path, "src", "ext", "dawn", "include" });
        try cwd.makePath(dest_dawn_include_path);
        try copyFolder(opts.arena, dest_dawn_include_path, src_dawn_include_path, &.{ .include_extensions = &.{".h"} });

        std.log.info("Packaged dev dependencies to {s}", .{sdk_deps_path});
    }
}
