const std = @import("std");

pub fn main() !void {
    var arena = std.heap.ArenaAllocator.init(std.heap.page_allocator);
    defer arena.deinit();
    const allocator: std.mem.Allocator = arena.allocator();

    const args: []const [:0]u8 = try std.process.argsAlloc(allocator);
    defer std.process.argsFree(allocator, args);

    const stdout_file = std.io.getStdOut().writer();
    var bw = std.io.bufferedWriter(stdout_file);

    const stdout = bw.writer();

    for (args, 0..) |arg, i| {
        if (i == 0) {
            continue;
        }
        try stdout.print("{s}", .{arg});
    }

    try bw.flush();
}
