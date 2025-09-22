const std = @import("std");

const Options = struct {
    wast_dir: []const u8,
    output_dir: []const u8,
    wasm_tools: []const u8,

    fn parse(args: []const [:0]const u8) !Options {
        var wast_dir: ?[]const u8 = null;
        var output_dir: ?[]const u8 = null;
        var wasm_tools: ?[]const u8 = null;

        for (args, 0..) |raw_arg, i| {
            if (i == 0) {
                continue;
            }

            var splitIter = std.mem.splitScalar(u8, raw_arg, '=');
            const arg: []const u8 = splitIter.next().?;
            if (std.mem.eql(u8, arg, "--out")) {
                output_dir = splitIter.next();
            } else if (std.mem.eql(u8, arg, "--tests")) {
                wast_dir = splitIter.next();
            } else if (std.mem.eql(u8, arg, "--wasm-tools")) {
                wasm_tools = splitIter.next();
            }
        }

        var missing_arg: ?[]const u8 = null;
        if (wast_dir == null) {
            missing_arg = "out";
        } else if (output_dir == null) {
            missing_arg = "wast-directory";
        } else if (wasm_tools == null) {
            missing_arg = "wasm-tools";
        }

        if (missing_arg) |arg| {
            std.log.err("Missing required arg: {s}", .{arg});
            return error.MissingRequiredArgument;
        }

        return Options{
            .wast_dir = wast_dir.?,
            .output_dir = output_dir.?,
            .wasm_tools = wasm_tools.?,
        };
    }
};

pub fn main() !void {
    var arena = std.heap.ArenaAllocator.init(std.heap.page_allocator);
    defer arena.deinit();
    const allocator: std.mem.Allocator = arena.allocator();

    const args: []const [:0]u8 = try std.process.argsAlloc(allocator);
    defer std.process.argsFree(allocator, args);
    const opts = try Options.parse(args);

    var wast_files = std.ArrayList([]const u8).init(allocator);
    {
        var dir = try std.fs.cwd().openDir(opts.wast_dir, .{ .iterate = true });
        var walker = try dir.walk(allocator);
        defer walker.deinit();

        while (try walker.next()) |entry| {
            const ext = std.fs.path.extension(entry.basename);
            if (std.mem.eql(u8, ext, ".wast")) {
                // we have to clone the path as walker.next() or walker.deinit() will override/kill it
                const path = try allocator.dupe(u8, entry.path);
                try wast_files.append(path);
            }
        }
    }
    for (wast_files.items) |s| {
        const wastPath = try std.fs.path.join(allocator, &.{ opts.wast_dir, s });
        const basename = std.fs.path.stem(s);
        const outName = try std.mem.join(allocator, "", &.{ basename, ".json" });
        const outPath = try std.fs.path.join(allocator, &.{ opts.output_dir, outName });

        // std.debug.print("{s} -> {s}\n", .{ s, outPath });

        const result = try std.process.Child.run(.{
            .allocator = allocator,
            .argv = &.{ opts.wasm_tools, "json-from-wast", "-o", outPath, "--wasm-dir", opts.output_dir, wastPath },
            //.argv = &.{ "wast2json", "-o", outPath, wastPath },
        });
        std.debug.print("{s}{s}", .{ result.stdout, result.stderr });
    }
}
