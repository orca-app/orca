const std = @import("std");
const oc = @import("orca");

var counter: u32 = 0;
var lastSeconds: f64 = 0;

export fn oc_on_init() void {
    oc.logInfo("platform: {}", .{oc.getHostPlatform()}, @src());

    oc.windowSetTitle("zig calc");
    oc.windowSetSize(oc.vec2{ .x = 480, .y = 640 });
}

export fn oc_on_resize(width: u32, height: u32) void {
    oc.logInfo("frame resize: {}, {}", .{ width, height }, @src());
}

export fn oc_on_mouse_down(button: c_int) void {
    oc.logInfo("mouse down! {}", .{button}, @src());
}

export fn oc_on_mouse_up(button: c_int) void {
    oc.logInfo("mouse up! {}", .{button}, @src());
}

export fn oc_on_key_down(key: c_int) void {
    oc.logInfo("key down: {}", .{key}, @src());
}

export fn oc_on_key_up(key: c_int) void {
    oc.logInfo("key up: {}", .{key}, @src());

    switch (key) {
        oc.KeyCodes.escape => oc.requestQuit(),
        oc.KeyCodes.b => oc.abort("aborting", .{}, @src()),
        oc.KeyCodes.a => oc.assert(false, "test assert failed", .{}, @src()),
        else => {},
    }
}

export fn oc_on_frame_refresh() void {
    counter += 1;

    const secs: f64 = oc.clockTime(oc.Clock.Date);

    if (lastSeconds != @floor(secs)) {
        lastSeconds = @floor(secs);
        oc.logInfo("seconds since Jan 1, 1970: {d:.0}", .{secs}, @src());
    }
}

export fn oc_on_terminate() void {
    oc.logInfo("byebye {}", .{counter}, @src());
}
