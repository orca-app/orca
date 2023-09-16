const std = @import("std");
const oc = @import("orca");

const Vec2 = oc.Vec2;

var surface: oc.Surface = undefined;
var canvas: oc.Canvas = undefined;

var counter: u32 = 0;
var lastSeconds: f64 = 0;

export fn oc_on_init() void {
    oc.windowSetTitle("zig calc");
    oc.windowSetSize(Vec2{ .x = 480, .y = 640 });

    oc.logInfo("current platform: {}", .{oc.getHostPlatform()}, @src());

    surface = oc.Surface.canvas();
    canvas = oc.Canvas.create();

    oc.assert(oc.Canvas.nil().isNil() == true, "nil canvas should be nil", .{}, @src());
    oc.assert(canvas.isNil() == false, "created canvas should not be nil", .{}, @src());
}

export fn oc_on_resize(width: u32, height: u32) void {
    oc.logInfo("frame resize: {}, {}", .{ width, height }, @src());
}

export fn oc_on_mouse_down(button: oc.MouseButton) void {
    oc.logInfo("mouse down! {}", .{button}, @src());
}

export fn oc_on_mouse_up(button: oc.MouseButton) void {
    oc.logInfo("mouse up! {}", .{button}, @src());
}

export fn oc_on_key_down(key: oc.KeyCode) void {
    oc.logInfo("key down: {}", .{key}, @src());
}

export fn oc_on_key_up(key: oc.KeyCode) void {
    oc.logInfo("key up: {}", .{key}, @src());

    switch (key) {
        oc.KeyCode.Escape => oc.requestQuit(),
        oc.KeyCode.B => oc.abort("aborting", .{}, @src()),
        oc.KeyCode.A => oc.assert(false, "test assert failed", .{}, @src()),
        else => {},
    }
}

export fn oc_on_frame_refresh() void {
    counter += 1;

    const secs: f64 = oc.clockTime(oc.ClockKind.Date);

    if (lastSeconds != @floor(secs)) {
        lastSeconds = @floor(secs);
        oc.logInfo("seconds since Jan 1, 1970: {d:.0}", .{secs}, @src());
    }

    _ = canvas.setCurrent();
    surface.select();
    oc.setColorRgba(0.05, 0.05, 0.05, 1.0);
    oc.clear();

    oc.setColorRgba(1.0, 0.05, 0.05, 1.0);

    {
        const translation: oc.Mat2x3 = .{ .m = [_]f32{ 1, 0, 50, 0, 1, 50 } };
        translation.push();
        defer oc.Mat2x3.pop();

        oc.assert(std.meta.eql(oc.matrixTop(), translation), "top of matrix stack should be what we pushed", .{}, @src());
        oc.setWidth(1);
        oc.rectangleFill(50, 50, 10, 10);
        oc.rectangleStroke(70, 50, 10, 10);
        oc.roundedRectangleFill(90, 50, 10, 10, 3);
        oc.roundedRectangleStroke(110, 50, 10, 10, 3);

        const green = oc.Color{ .Flat = .{ .r = 0.05, .g = 1, .b = 0.05, .a = 1 } };
        oc.setColor(green);
        oc.assert(std.meta.eql(oc.getColor().Flat, green.Flat), "color should be green", .{}, @src());

        oc.setTolerance(1);
        oc.setJoint(.Bevel);
        oc.ellipseFill(140, 55, 10, 5);
        oc.ellipseStroke(170, 55, 10, 5);
        oc.circleFill(195, 55, 5);
        oc.circleStroke(215, 55, 5);

        oc.arc(230, 55, 5, 0, std.math.pi);
    }

    surface.render(canvas);
    surface.present();
}

export fn oc_on_terminate() void {
    oc.logInfo("byebye {}", .{counter}, @src());
}
