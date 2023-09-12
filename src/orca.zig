const std = @import("std");
const c = @cImport({
    @cDefine("__ORCA__", "");
    @cInclude("orca.h");
});

pub fn log_info(comptime fmt: []const u8, args: anytype, source: std.builtin.SourceLocation) void {
    var format_buf: [512:0]u8 = undefined;
    _ = std.fmt.bufPrintZ(&format_buf, fmt, args) catch 0; // just discard NoSpaceLeft error for now

    var line: c_int = @intCast(source.line);
    c.oc_log_ext(c.OC_LOG_LEVEL_INFO, source.fn_name.ptr, source.file.ptr, line, format_buf[0..].ptr);
}

pub const oc_request_quit = c.oc_request_quit;
