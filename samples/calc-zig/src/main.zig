const std = @import("std");
const oc = @import("orca");

var counter: u32 = 0;

export fn oc_on_init() void {
    oc.log_info("counter: {}", .{counter}, @src());
}

export fn oc_on_frame_refresh() void {
    counter += 1;
    oc.log_info("counter: {}", .{counter}, @src());

    if (counter == 10) {
        oc.oc_request_quit();
    }
}

export fn oc_on_terminate() void {
    oc.log_info("byebye", .{}, @src());
}
