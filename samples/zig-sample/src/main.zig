const std = @import("std");
const oc = @import("orca");

const Vec2 = oc.Vec2;
const Mat2x3 = oc.Mat2x3;

var surface: oc.Surface = undefined;
var canvas: oc.Canvas = undefined;
var font: oc.Font = undefined;
var orca_image: oc.Image = undefined;

var counter: u32 = 0;
var last_seconds: f64 = 0;
var frame_size: Vec2 = .{ .x = 0, .y = 0 };

var rotation_demo: f32 = 0;

export fn oc_on_init() void {
    oc.windowSetTitle("zig sample");
    oc.windowSetSize(Vec2{ .x = 480, .y = 640 });

    oc.log.info("current platform: {}", .{oc.getHostPlatform()}, @src());

    surface = oc.Surface.canvas();
    canvas = oc.Canvas.create();

    oc.assert(oc.Canvas.nil().isNil() == true, "nil canvas should be nil", .{}, @src());
    oc.assert(canvas.isNil() == false, "created canvas should not be nil", .{}, @src());

    var ranges = oc.UnicodeRange.range(&[_]oc.UnicodeRange.Enum{
        .BasicLatin,
        .C1ControlsAndLatin1Supplement,
        .LatinExtendedA,
        .LatinExtendedB,
        .Specials,
    });
    font = oc.Font.createFromPath("/zig.ttf", &ranges);
    oc.assert(font.isNil() == false, "created font should not be nil", .{}, @src());

    orca_image = oc.Image.createFromPath(surface, oc.Str8.fromSlice("/orca_jumping.jpg"), false);
    oc.assert(orca_image.isNil() == false, "created image should not be nil", .{}, @src());
}

export fn oc_on_resize(width: u32, height: u32) void {
    frame_size = Vec2{ .x = @floatFromInt(width), .y = @floatFromInt(height) };
    oc.log.info("frame resize: {d:.2}, {d:.2}", .{ frame_size.x, frame_size.y }, @src());
}

export fn oc_on_mouse_down(button: oc.MouseButton) void {
    oc.log.info("mouse down! {}", .{button}, @src());
}

export fn oc_on_mouse_up(button: oc.MouseButton) void {
    oc.log.info("mouse up! {}", .{button}, @src());
}

export fn oc_on_mouse_wheel(dx: f32, dy: f32) void {
    oc.log.info("mouse wheel! dx: {d:.2}, dy: {d:.2}", .{ dx, dy }, @src());
}

export fn oc_on_key_down(scan: oc.ScanCode, key: oc.KeyCode) void {
    oc.log.info("key down: {} {}", .{ scan, key }, @src());
}

export fn oc_on_key_up(scan: oc.ScanCode, key: oc.KeyCode) void {
    oc.log.info("key up: {} {}", .{ scan, key }, @src());

    switch (key) {
        oc.KeyCode.Escape => oc.requestQuit(),
        oc.KeyCode.B => oc.abort("aborting", .{}, @src()),
        oc.KeyCode.A => oc.assert(false, "test assert failed", .{}, @src()),
        oc.KeyCode.W => oc.log.warn("logging a test warning", .{}, @src()),
        oc.KeyCode.E => oc.log.err("logging a test error", .{}, @src()),
        else => {},
    }
}

export fn oc_on_frame_refresh() void {
    counter += 1;

    const secs: f64 = oc.clockTime(oc.ClockKind.Date);

    if (last_seconds != @floor(secs)) {
        last_seconds = @floor(secs);
        oc.log.info("seconds since Jan 1, 1970: {d:.0}", .{secs}, @src());
    }

    _ = canvas.select();
    oc.setColorRgba(0.05, 0.05, 0.05, 1.0);
    oc.clear();

    oc.setColorRgba(1.0, 0.05, 0.05, 1.0);

    {
        const translation: Mat2x3 = .{ .m = [_]f32{ 1, 0, 50, 0, 1, 50 } };
        Mat2x3.push(translation);
        defer Mat2x3.pop();

        oc.assert(std.meta.eql(Mat2x3.top(), translation), "top of matrix stack should be what we pushed", .{}, @src());
        oc.setWidth(1);
        oc.rectangleFill(50, 0, 10, 10);
        oc.rectangleStroke(70, 0, 10, 10);
        oc.roundedRectangleFill(90, 0, 10, 10, 3);
        oc.roundedRectangleStroke(110, 0, 10, 10, 3);

        const green = oc.Color{ .Flat = .{ .r = 0.05, .g = 1, .b = 0.05, .a = 1 } };
        oc.setColor(green);
        oc.assert(std.meta.eql(oc.getColor().Flat, green.Flat), "color should be green", .{}, @src());

        oc.setTolerance(1);
        oc.setJoint(.Bevel);
        oc.ellipseFill(140, 5, 10, 5);
        oc.ellipseStroke(170, 5, 10, 5);
        oc.circleFill(195, 5, 5);
        oc.circleStroke(215, 5, 5);

        oc.arc(230, 5, 5, 0, std.math.pi);
    }

    {
        rotation_demo += 0.03;

        const rot = Mat2x3.rotate(rotation_demo);
        const trans = Mat2x3.translate(285, 55);
        Mat2x3.push(Mat2x3.mul_m(trans, rot));
        defer Mat2x3.pop();

        oc.rectangleFill(-5, -5, 10, 10);
    }

    {
        const str = oc.Str8.fromSlice("Hello from Zig!");
        const font_size = 18;
        const text_rect = font.textMetrics(font_size, str).ink;

        const center_x = frame_size.x / 2;
        const text_begin_x = center_x - text_rect.Flat.w / 2;

        Mat2x3.push(Mat2x3.translate(text_begin_x, 150));
        defer Mat2x3.pop();

        oc.setColorRgba(1.0, 0.05, 0.05, 1.0);

        oc.setFont(font);
        oc.setFontSize(font_size);
        oc.moveTo(0, 0);
        oc.textOutlines(str);
        oc.fill();
    }

    {
        const trans = Mat2x3.translate(0, 200);
        const scale = Mat2x3.scaleUniform(0.25);
        Mat2x3.push(Mat2x3.mul_m(trans, scale));
        defer Mat2x3.pop();

        const size = orca_image.size();

        orca_image.draw(oc.Rect.xywh(0, 0, size.x, size.y));

        var half_size = orca_image.size();
        half_size.x /= 2;
        orca_image.drawRegion(oc.Rect.xywh(0, 0, half_size.x, half_size.y), oc.Rect.xywh(size.x + 10, 0, half_size.x, half_size.y));
    }

    surface.select();
    canvas.render();
    surface.present();
}

export fn oc_on_terminate() void {
    oc.log.info("byebye {}", .{counter}, @src());
}
