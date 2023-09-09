const std = @import("std");
const oc = @import("orca");

export fn oc_on_init() void {
    oc.log_info("oc_on_init!!!!\n");
}

export fn oc_on_frame_refresh() void {
    oc.log_info("oc_on_frame_refresh!!!!\n");
}
