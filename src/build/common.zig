// Contains some functionality used in multiple helper programs in the zig build graph.

const std = @import("std");
const builtin = @import("builtin");

pub const CopyFolderOpts = struct {
    include_extensions: []const []const u8 = &.{},
    ignore_patterns: []const []const u8 = &.{},
    required_filenames: []const []const u8 = &.{},
};

pub fn copyFolder(
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
                    const src_file_path: []const u8 = try std.fs.path.join(allocator, &.{ src, src_entry.path });
                    const dest_file_path: []const u8 = try std.fs.path.join(allocator, &.{ dest, src_entry.path });
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
