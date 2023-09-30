const std = @import("std");
const oc = @import("orca");

const lerp = std.math.lerp;

const Vec2 = oc.Vec2;
const Mat2x3 = oc.Mat2x3;
const Str8 = oc.Str8;

var surface: oc.Surface = undefined;
var canvas: oc.Canvas = undefined;
var font: oc.Font = undefined;
var orca_image: oc.Image = undefined;
var gradient_image: oc.Image = undefined;

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

    const ranges = oc.UnicodeRange.range(&[_]oc.UnicodeRange.Enum{
        .BasicLatin,
        .C1ControlsAndLatin1Supplement,
        .LatinExtendedA,
        .LatinExtendedB,
        .Specials,
    });
    font = oc.Font.createFromPath("/zig.ttf", &ranges);
    oc.assert(oc.Font.nil().isNil() == true, "nil font should be nil", .{}, @src());
    oc.assert(font.isNil() == false, "created font should not be nil", .{}, @src());

    orca_image = oc.Image.createFromPath(surface, Str8.fromSlice("/orca_jumping.jpg"), false);
    oc.assert(oc.Image.nil().isNil() == true, "nil image should be nil", .{}, @src());
    oc.assert(orca_image.isNil() == false, "created image should not be nil", .{}, @src());

    // generate a gradient and upload it to an image
    {
        const width = 256;
        const height = 128;

        const tl = oc.Color{ .r = 70.0 / 255.0, .g = 13.0 / 255.0, .b = 108.0 / 255.0 };
        const bl = oc.Color{ .r = 251.0 / 255.0, .g = 167.0 / 255.0, .b = 87.0 / 255.0 };
        const tr = oc.Color{ .r = 48.0 / 255.0, .g = 164.0 / 255.0, .b = 219.0 / 255.0 };
        const br = oc.Color{ .r = 151.0 / 255.0, .g = 222.0 / 255.0, .b = 150.0 / 255.0 };

        var pixels: [width * height]u32 = undefined;
        for (0..height) |y| {
            for (0..width) |x| {
                const h: f32 = @floatFromInt(height - 1);
                const w: f32 = @floatFromInt(width - 1);
                const y_norm: f32 = @as(f32, @floatFromInt(y)) / h;
                const x_norm: f32 = @as(f32, @floatFromInt(x)) / w;

                const tl_weight = (1 - x_norm) * (1 - y_norm);
                const bl_weight = (1 - x_norm) * y_norm;
                const tr_weight = x_norm * (1 - y_norm);
                const br_weight = x_norm * y_norm;

                const r: f32 = tl_weight * tl.r + bl_weight * bl.r + tr_weight * tr.r + br_weight * br.r;
                const g: f32 = tl_weight * tl.g + bl_weight * bl.g + tr_weight * tr.g + br_weight * br.g;
                const b: f32 = tl_weight * tl.b + bl_weight * bl.b + tr_weight * tr.b + br_weight * br.b;
                const color = oc.Color{ .r = r, .g = g, .b = b, .a = 1.0 };
                pixels[y * width + x] = color.toRgba8();
            }
        }

        gradient_image = oc.Image.create(surface, width, height);
        gradient_image.uploadRegionRgba8(oc.Rect.xywh(0, 0, width, height), @ptrCast((&pixels).ptr));
    }
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

    const secs: f64 = oc.clock.time(.Date);

    if (last_seconds != @floor(secs)) {
        last_seconds = @floor(secs);
        oc.log.info("seconds since Jan 1, 1970: {d:.0}", .{secs}, @src());
    }

    _ = canvas.select();
    oc.Canvas.setColorRgba(0.05, 0.05, 0.05, 1.0);
    oc.Canvas.clear();

    oc.Canvas.setColorRgba(1.0, 0.05, 0.05, 1.0);

    {
        const translation: Mat2x3 = .{ .m = [_]f32{ 1, 0, 50, 0, 1, 50 } };
        Mat2x3.push(translation);
        defer Mat2x3.pop();

        oc.assert(std.meta.eql(Mat2x3.top(), translation), "top of matrix stack should be what we pushed", .{}, @src());
        oc.Canvas.setWidth(1);
        oc.Canvas.rectangleFill(50, 0, 10, 10);
        oc.Canvas.rectangleStroke(70, 0, 10, 10);
        oc.Canvas.roundedRectangleFill(90, 0, 10, 10, 3);
        oc.Canvas.roundedRectangleStroke(110, 0, 10, 10, 3);

        const green = oc.Color{ .r = 0.05, .g = 1, .b = 0.05, .a = 1 };
        oc.Canvas.setColor(green);
        oc.assert(std.meta.eql(oc.Canvas.getColor(), green), "color should be green", .{}, @src());

        oc.Canvas.setTolerance(1);
        oc.Canvas.setJoint(.Bevel);
        oc.Canvas.ellipseFill(140, 5, 10, 5);
        oc.Canvas.ellipseStroke(170, 5, 10, 5);
        oc.Canvas.circleFill(195, 5, 5);
        oc.Canvas.circleStroke(215, 5, 5);

        oc.Canvas.arc(230, 5, 5, 0, std.math.pi);
    }

    {
        rotation_demo += 0.03;

        const rot = Mat2x3.rotate(rotation_demo);
        const trans = Mat2x3.translate(285, 55);
        Mat2x3.push(Mat2x3.mul_m(trans, rot));
        defer Mat2x3.pop();

        oc.Canvas.rectangleFill(-5, -5, 10, 10);
    }

    {
        var scratch_scope = oc.Arena.scratchBegin();
        defer scratch_scope.end();

        var scratch: *oc.Arena = scratch_scope.arena;

        var str1: Str8 = Str8.collate(scratch, &[_][]const u8{ "Hello", "from", "Zig!" }, ">> ", " ", " <<") catch |e| fatal(e, @src());

        var str2_list = oc.Str8List.init();
        str2_list.push(scratch, Str8.fromSlice("All")) catch |e| fatal(e, @src());
        str2_list.pushf(scratch, "your", .{}) catch |e| fatal(e, @src());
        str2_list.pushSlice(scratch, "base!!") catch |e| fatal(e, @src());

        oc.assert(str2_list.containsSlice("All"), "str2_list should have the string we just pushed", .{}, @src());

        {
            var elt_first = str2_list.list.first;
            var elt_last = str2_list.list.last;
            oc.assert(elt_first != null, "list checks", .{}, @src());
            oc.assert(elt_last != null, "list checks", .{}, @src());
            oc.assert(elt_first != elt_last, "list checks", .{}, @src());
            oc.assert(elt_first.?.next != null, "list checks", .{}, @src());
            oc.assert(elt_first.?.prev == null, "list checks", .{}, @src());
            oc.assert(elt_last.?.next == null, "list checks", .{}, @src());
            oc.assert(elt_last.?.prev != null, "list checks", .{}, @src());
            oc.assert(elt_first.?.next != elt_last, "list checks", .{}, @src());
            oc.assert(elt_last.?.prev != elt_first, "list checks", .{}, @src());
        }

        var str2: Str8 = str2_list.collate(scratch, Str8.fromSlice("<< "), Str8.fromSlice("-"), Str8.fromSlice(" >>")) catch |e| fatal(e, @src());

        const font_size = 18;
        const text_metrics = font.textMetrics(font_size, str1);
        const text_rect = text_metrics.ink;

        const center_x = frame_size.x / 2;
        const text_begin_x = center_x - text_rect.w / 2;

        Mat2x3.push(Mat2x3.translate(text_begin_x, 100));
        defer Mat2x3.pop();

        oc.Canvas.setColorRgba(1.0, 0.05, 0.05, 1.0);
        oc.Canvas.setFont(font);
        oc.Canvas.setFontSize(font_size);
        oc.Canvas.moveTo(0, 0);
        oc.Canvas.textOutlines(str1);
        oc.Canvas.moveTo(0, 35);
        oc.Canvas.textOutlines(str2);
        oc.Canvas.fill();
    }

    {
        var scratch_scope = oc.Arena.scratchBegin();
        defer scratch_scope.end();

        var scratch: *oc.Arena = scratch_scope.arena;

        var separators = oc.Str8List.init();
        separators.pushSlice(scratch, " ") catch |e| fatal(e, @src());
        separators.pushSlice(scratch, "|") catch |e| fatal(e, @src());
        separators.pushSlice(scratch, "-") catch |e| fatal(e, @src());

        const big_string = Str8.fromSlice("This is |a one-word string that  |  has no      spaces in it");
        var strings: oc.Str8List = big_string.split(scratch, separators) catch |e| fatal(e, @src());
        var collated = strings.join(scratch) catch |e| fatal(e, @src());

        oc.Canvas.setFontSize(12);
        oc.Canvas.moveTo(0, 170);
        oc.Canvas.textOutlines(collated);
        oc.Canvas.fill();
    }

    {
        const orca_size = orca_image.size();

        {
            const trans = Mat2x3.translate(0, 200);
            const scale = Mat2x3.scaleUniform(0.25);
            Mat2x3.push(Mat2x3.mul_m(trans, scale));
            defer Mat2x3.pop();

            orca_image.draw(oc.Rect.xywh(0, 0, orca_size.x, orca_size.y));

            var half_size = orca_size;
            half_size.x /= 2;
            orca_image.drawRegion(oc.Rect.xywh(0, 0, half_size.x, half_size.y), oc.Rect.xywh(orca_size.x + 10, 0, half_size.x, half_size.y));
        }

        {
            const x_offset = orca_size.x * 0.25 + orca_size.x * 0.25 * 0.5 + 5;
            const gradient_size = gradient_image.size();

            const trans = Mat2x3.translate(x_offset, 200);
            const scale = Mat2x3.scaleUniform((orca_size.y * 0.25) / gradient_size.y);
            Mat2x3.push(Mat2x3.mul_m(trans, scale));
            defer Mat2x3.pop();

            gradient_image.draw(oc.Rect.xywh(0, 0, gradient_size.x, gradient_size.y));
        }
    }

    surface.select();
    canvas.render();
    surface.present();
}

export fn oc_on_terminate() void {
    oc.log.info("byebye {}", .{counter}, @src());
}

fn fatal(err: anyerror, source: std.builtin.SourceLocation) noreturn {
    oc.abort("Caught fatal {}", .{err}, source);
    unreachable;
}

fn oneMinusLerp(a: anytype, b: anytype, t: anytype) @TypeOf(a, b, t) {
    return 1.0 - lerp(a, b, t);
}
