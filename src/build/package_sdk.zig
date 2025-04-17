// This is a zig program run as part of the orca build process to copy all appropriate
// build artifacts to an output location. Optionally the user can package the SDK
// for installation to the system orca path.

const std = @import("std");
const builtin = @import("builtin");

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

fn copyFolder(
    allocator: std.mem.Allocator,
    dest: []const u8,
    src: []const u8,
    include_extensions: []const []const u8,
    ignore_patterns: []const []const u8,
) !void {
    std.log.info("copying '{s}' to '{s}'", .{ src, dest });

    const cwd = std.fs.cwd();
    try cwd.makePath(dest);

    const src_dir: std.fs.Dir = try cwd.openDir(src, .{ .iterate = true });
    const dest_dir: std.fs.Dir = try cwd.openDir(dest, .{ .iterate = true });

    var normalized_ignore_patterns = std.ArrayList([]u8).init(allocator);
    for (ignore_patterns) |pattern| {
        try normalized_ignore_patterns.append(try std.fs.path.resolve(allocator, &.{pattern}));
    }

    var src_walker = try src_dir.walk(allocator);
    while (try src_walker.next()) |src_entry| {
        var included: bool = true;
        if (src_entry.kind != .directory) {
            included = include_extensions.len == 0;
            const extension = std.fs.path.extension(src_entry.basename);
            for (include_extensions) |included_extension| {
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

        if (included and !ignored) {
            _ = switch (src_entry.kind) {
                .file => try src_dir.updateFile(src_entry.path, dest_dir, src_entry.path, .{}),
                else => {},
            };
        }
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
    artifacts_path: []const u8,
    resources_path: []const u8,
    src_path: []const u8,
    version: []const u8,
    is_dev_install: bool,

    fn parse(args: []const [:0]const u8, arena: std.mem.Allocator) !Options {
        var sdk_path: ?[]const u8 = null;
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
                sdk_path = try std.fs.path.join(arena, &.{ orca_dir, version.? });
            } else {
                sdk_path = try std.fs.path.join(arena, &.{ sdk_path.?, version.? });
            }
        }

        return Options{
            .arena = arena,
            .sdk_path = sdk_path.?,
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

    std.fs.deleteTreeAbsolute(opts.sdk_path) catch {};

    const src_paths: []const []const u8 = &.{
        try std.fs.path.join(opts.arena, &.{ opts.artifacts_path, "bin" }),
        try std.fs.path.join(opts.arena, &.{ opts.artifacts_path, "orca-libc" }),
        opts.resources_path,
    };

    const dest_paths: []const []const u8 = &.{
        try std.fs.path.join(opts.arena, &.{ opts.sdk_path, "bin" }),
        try std.fs.path.join(opts.arena, &.{ opts.sdk_path, "orca-libc" }),
        try std.fs.path.join(opts.arena, &.{ opts.sdk_path, "resources" }),
    };

    for (src_paths, dest_paths) |src, dest| {
        try copyFolder(opts.arena, dest, src, &.{}, &.{});
    }

    const header_extensions: []const []const u8 = &.{
        ".h",
    };
    // For copying the src directory, manually copy the curl and wasm3 dirs since we only care
    // about the headers in a specific directory. Everything else is ok though
    {
        const dest_src_path = try std.fs.path.join(opts.arena, &.{ opts.sdk_path, "src" });
        const ignore_patterns: []const []const u8 = &.{
            "tool/",
            "orca-libc/",
            "wasm/",
            "ext/curl/",
            "ext/wasm3/",
            "ext/zlib/build/", // copy all headers in zlib except zlib/build
        };
        try copyFolder(opts.arena, dest_src_path, opts.src_path, header_extensions, ignore_patterns);
    }

    {
        const curl_src_path = try std.fs.path.resolve(opts.arena, &.{ opts.src_path, "ext/curl/include" });
        const curl_dest_path = try std.fs.path.resolve(opts.arena, &.{ opts.sdk_path, "src/ext/curl/include" });

        try copyFolder(opts.arena, curl_dest_path, curl_src_path, header_extensions, &.{});
    }

    {
        const wasm3_src_path = try std.fs.path.resolve(opts.arena, &.{ opts.src_path, "ext/wasm3/source" });
        const wasm3_dest_path = try std.fs.path.resolve(opts.arena, &.{ opts.sdk_path, "src/ext/wasm3/source" });
        try copyFolder(opts.arena, wasm3_dest_path, wasm3_src_path, header_extensions, &.{});
    }

    // dev installs need to overwrite the orca tool in case there were any changes
    if (opts.is_dev_install) {
        const orca_dir_path: []const u8 = if (std.fs.path.dirname(opts.sdk_path)) |orca_dir| orca_dir else opts.sdk_path;

        const src_orca_exe_name: []const u8 = if (builtin.os.tag == .windows) "orca_tool.exe" else "orca_tool";
        const src_tool_path: []const u8 = try std.fs.path.join(opts.arena, &.{ opts.artifacts_path, "bin", src_orca_exe_name });

        const dest_orca_exe_name: []const u8 = if (builtin.os.tag == .windows) "orca.exe" else "orca";
        const dest_tool_path: []const u8 = try std.fs.path.join(opts.arena, &.{ orca_dir_path, dest_orca_exe_name });

        std.log.info("copying '{s}' to '{s}'", .{ src_tool_path, dest_tool_path });
        const cwd = std.fs.cwd();
        const orca_dir: std.fs.Dir = try cwd.openDir(orca_dir_path, .{ .iterate = true });
        _ = try cwd.updateFile(src_tool_path, orca_dir, dest_tool_path, .{});

        // copy pdb file as well since windows debuggers have a hard time finding the debug symbols otherwise
        if (builtin.os.tag == .windows) {
            const src_pdb_path: []const u8 = try std.fs.path.join(opts.arena, &.{ opts.artifacts_path, "bin", "orca_tool.pdb" });
            const dest_pdb_path: []const u8 = try std.fs.path.join(opts.arena, &.{ orca_dir_path, "orca.pdb" });
            std.log.info("copying '{s}' to '{s}'", .{ src_pdb_path, dest_pdb_path });
            _ = try cwd.updateFile(src_pdb_path, orca_dir, dest_pdb_path, .{});
        }

        try orca_dir.writeFile(.{
            .sub_path = "current_version",
            .data = opts.version,
        });
    }

    std.log.info("Packaged Orca SDK to {s}", .{opts.sdk_path});
}
