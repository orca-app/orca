const std = @import("std");
const oc = @import("root");

const lerp = std.math.lerp;

const Vec2 = oc.Vec2;
const Mat2x3 = oc.Mat2x3;
const Str8 = oc.Str8;

var allocator = std.heap.wasm_allocator;

var surface: oc.Surface = undefined;
var canvas: oc.Canvas = undefined;
var font: oc.Font = undefined;
var orca_image: oc.Image = undefined;
var gradient_image: oc.Image = undefined;

var counter: u32 = 0;
var last_seconds: f64 = 0;
var frame_size: Vec2 = .{ .x = 0, .y = 0 };

var rotation_demo: f32 = 0;

pub fn onInit() !void {
    oc.windowSetTitle("zig sample");
    oc.windowSetSize(Vec2{ .x = 480, .y = 640 });

    oc.log.info("current platform: {}", .{oc.getHostPlatform()}, @src());

    surface = oc.Surface.canvas();
    canvas = oc.Canvas.create();

    const surface_scaling = surface.contentsScaling();
    oc.log.info("surface scaling: {d:.2} {d:.2}", .{ surface_scaling.x, surface_scaling.y }, @src());

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

    orca_image = oc.Image.createFromPath(surface, "/orca_jumping.jpg", .NoFlip);
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

        var tmp_image = oc.Image.create(surface, width, height);
        tmp_image.uploadRegionRgba8(oc.Rect.xywh(0, 0, width, height), @ptrCast((&pixels).ptr));
        tmp_image.destroy();
        tmp_image = oc.Image.nil();
    }

    try testFileApis();
}

pub fn onResize(width: u32, height: u32) void {
    frame_size = Vec2{ .x = @floatFromInt(width), .y = @floatFromInt(height) };
    const surface_size = surface.getSize();
    oc.log.info("frame resize: {d:.2}, {d:.2}, surface size: {d:.2} {d:.2}", .{ frame_size.x, frame_size.y, surface_size.x, surface_size.y }, @src());
}

pub fn onMouseDown(button: oc.MouseButton) void {
    oc.log.info("mouse down! {}", .{button}, @src());
}

pub fn onMouseUp(button: oc.MouseButton) void {
    oc.log.info("mouse up! {}", .{button}, @src());
}

pub fn onMouseWheel(dx: f32, dy: f32) void {
    oc.log.info("mouse wheel! dx: {d:.2}, dy: {d:.2}", .{ dx, dy }, @src());
}

pub fn onKeyDown(scan: oc.ScanCode, key: oc.KeyCode) void {
    oc.log.info("key down: {} {}", .{ scan, key }, @src());
}

pub fn onKeyUp(scan: oc.ScanCode, key: oc.KeyCode) void {
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

pub fn onFrameRefresh() !void {
    counter += 1;

    const secs: f64 = oc.clock.time(.Date);

    if (last_seconds != @floor(secs)) {
        last_seconds = @floor(secs);
        oc.log.info("seconds since Jan 1, 1970: {d:.0}", .{secs}, @src());
    }

    _ = canvas.select();

    {
        const c1 = oc.Color{ .r = 0.05, .g = 0.05, .b = 0.05, .a = 1.0 };
        const c2 = oc.Color{ .r = 0.05, .g = 0.05, .b = 0.05, .a = 1.0 };
        oc.Canvas.setColorRgba(c1.r, c1.g, c1.b, c1.a);
        oc.assert(std.meta.eql(oc.Canvas.getColor(), c1), "color should be what we set", .{}, @src());
        oc.Canvas.setColor(c2);
        oc.assert(std.meta.eql(oc.Canvas.getColor(), c2), "color should be what we set", .{}, @src());
        oc.Canvas.clear();

        oc.Canvas.setTolerance(1);
        oc.assert(oc.Canvas.getTolerance() == 1, "tolerance should be 1", .{}, @src());
        oc.Canvas.setJoint(.Bevel);
        oc.assert(oc.Canvas.getJoint() == .Bevel, "joint should be what we set", .{}, @src());
        oc.Canvas.setCap(.Square);
        oc.assert(oc.Canvas.getCap() == .Square, "cap should be what we set", .{}, @src());
    }

    {
        const translation: Mat2x3 = .{ .m = [_]f32{ 1, 0, 50, 0, 1, 50 } };
        Mat2x3.push(translation);
        defer Mat2x3.pop();

        oc.assert(std.meta.eql(Mat2x3.top(), translation), "top of matrix stack should be what we pushed", .{}, @src());
        oc.Canvas.setWidth(1);
        oc.assert(oc.Canvas.getWidth() == 1, "width should be 1", .{}, @src());
        oc.Canvas.rectangleFill(50, 0, 10, 10);
        oc.Canvas.rectangleStroke(70, 0, 10, 10);
        oc.Canvas.roundedRectangleFill(90, 0, 10, 10, 3);
        oc.Canvas.roundedRectangleStroke(110, 0, 10, 10, 3);

        const green = oc.Color{ .r = 0.05, .g = 1, .b = 0.05, .a = 1 };
        oc.Canvas.setColor(green);
        oc.assert(std.meta.eql(oc.Canvas.getColor(), green), "color should be green", .{}, @src());

        oc.Canvas.ellipseFill(140, 5, 10, 5);
        oc.Canvas.ellipseStroke(170, 5, 10, 5);
        oc.Canvas.circleFill(195, 5, 5);
        oc.Canvas.circleStroke(215, 5, 5);

        oc.Canvas.arc(235, 5, 5, std.math.pi, 0);
        oc.Canvas.stroke();

        oc.Canvas.arc(260, 5, 5, std.math.pi, 0);
        oc.Canvas.fill();

        oc.Canvas.moveTo(0, 0);
        oc.assert(std.meta.eql(Vec2.zero(), oc.Canvas.getPosition()), "pos should be zero after moving there", .{}, @src());
    }

    {
        rotation_demo += 0.03;

        const rot = Mat2x3.rotate(rotation_demo);
        const trans = Mat2x3.translate(335, 55);
        Mat2x3.push(Mat2x3.mulM(trans, rot));
        defer Mat2x3.pop();

        oc.Canvas.rectangleFill(-5, -5, 10, 10);
    }

    {
        var scratch_scope = oc.Arena.scratchBegin();
        defer scratch_scope.end();

        var scratch: *oc.Arena = scratch_scope.arena;

        var str1: Str8 = Str8.collate(scratch, &[_][]const u8{ "Hello", "from", "Zig!" }, ">> ", " ", " <<");

        var str2_list = oc.Str8List.init();
        str2_list.push(scratch, Str8.fromSlice("All"));
        str2_list.pushf(scratch, "your", .{});
        str2_list.pushSlice(scratch, "base!!");

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

        var str2: Str8 = str2_list.collate(scratch, Str8.fromSlice("<< "), Str8.fromSlice("-"), Str8.fromSlice(" >>"));

        const font_size = 18;
        const text_metrics = font.textMetrics(font_size, str1.slice());
        const text_rect = text_metrics.ink;

        const center_x = frame_size.x / 2;
        const text_begin_x = center_x - text_rect.w / 2;

        Mat2x3.push(Mat2x3.translate(text_begin_x, 100));
        defer Mat2x3.pop();

        oc.Canvas.setColorRgba(1.0, 0.05, 0.05, 1.0);
        oc.Canvas.setFont(font);
        oc.Canvas.setFontSize(font_size);
        oc.Canvas.moveTo(0, 0);
        oc.Canvas.textOutlines(str1.slice());
        oc.Canvas.moveTo(0, 35);
        oc.Canvas.textOutlines(str2.slice());
        oc.Canvas.fill();
    }

    {
        var scratch_scope = oc.Arena.scratchBegin();
        defer scratch_scope.end();

        var scratch: *oc.Arena = scratch_scope.arena;

        var separators = oc.Str8List.init();
        separators.pushSlice(scratch, " ");
        separators.pushSlice(scratch, "|");
        separators.pushSlice(scratch, "-");

        var strings_array = std.ArrayList([]const u8).init(allocator);
        defer strings_array.deinit();
        try strings_array.append("This ");
        try strings_array.append("is");
        try strings_array.append(" |a");
        try strings_array.append("one-word string that ");
        try strings_array.append(" |  has");
        try strings_array.append(" no ");
        try strings_array.append("    spaces i");
        try strings_array.append("n it");

        var single_string = std.ArrayList(u8).init(allocator);
        for (strings_array.items) |str| {
            try single_string.appendSlice(str);
        }

        const big_string = Str8.fromSlice(single_string.items);
        var strings: oc.Str8List = big_string.split(scratch, separators);
        var collated = strings.join(scratch);

        oc.Canvas.setFontSize(12);
        oc.Canvas.moveTo(0, 170);
        oc.Canvas.textOutlines(collated.slice());
        oc.Canvas.fill();
    }

    {
        const orca_size = orca_image.size();

        {
            const trans = Mat2x3.translate(0, 200);
            const scale = Mat2x3.scaleUniform(0.25);
            Mat2x3.push(Mat2x3.mulM(trans, scale));
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
            Mat2x3.push(Mat2x3.mulM(trans, scale));
            defer Mat2x3.pop();

            gradient_image.draw(oc.Rect.xywh(0, 0, gradient_size.x, gradient_size.y));
        }
    }

    surface.select();
    canvas.render();
    surface.present();
}

pub fn onTerminate() void {
    font.destroy();
    canvas.destroy();

    oc.log.info("byebye {}", .{counter}, @src());
}

fn oneMinusLerp(a: anytype, b: anytype, t: anytype) @TypeOf(a, b, t) {
    return 1.0 - lerp(a, b, t);
}

fn testFileApis() !void {
    var cwd = try oc.File.open("/", oc.File.AccessFlags.readonly(), oc.File.OpenFlags.none());
    oc.assert(cwd.isNil() == false, "file should be valid", .{}, @src());
    defer cwd.close();

    var orca_jumping_file = try oc.File.open("/orca_jumping.jpg", oc.File.AccessFlags.readonly(), oc.File.OpenFlags.none());
    oc.assert(orca_jumping_file.isNil() == false, "file should be valid", .{}, @src());
    orca_jumping_file.close();

    orca_jumping_file = try oc.File.openAt(cwd, "orca_jumping.jpg", oc.File.AccessFlags.readonly(), oc.File.OpenFlags.none());
    oc.assert((try orca_jumping_file.getStatus()).type == .Regular, "status API works", .{}, @src());
    oc.assert(try orca_jumping_file.getSize() > 0, "size API works", .{}, @src());
    oc.assert(orca_jumping_file.isNil() == false, "file should be valid", .{}, @src());

    var tmp_image = oc.Image.createFromFile(surface, orca_jumping_file, .NoFlip);
    oc.assert(tmp_image.isNil() == false, "image loaded from file should not be nil", .{}, @src());
    tmp_image.destroy();
    orca_jumping_file.close();

    const temp_file_contents = "hello world!";
    const temp_file_path = "/temp_file.txt";
    {
        var tmp_file = try oc.File.open(temp_file_path, oc.File.AccessFlags.readwrite(), oc.File.OpenFlags{ .create = true });
        defer tmp_file.close();

        oc.assert(tmp_file.isNil() == false, "file should be valid", .{}, @src());
        oc.assert(try tmp_file.pos() == 0, "new file shouldn't have anything in it yet", .{}, @src());

        var writer = tmp_file.writer();
        const written = try writer.write(temp_file_contents);
        oc.assert(written == temp_file_contents.len, "should have written some bytes.", .{}, @src());
    }

    {
        var tmp_file = try oc.File.open(temp_file_path, oc.File.AccessFlags.readwrite(), oc.File.OpenFlags{ .create = true });
        defer tmp_file.close();

        _ = try tmp_file.seek(0, .Set);
        oc.assert(try tmp_file.pos() == 0, "should be back at the beginning of the file", .{}, @src());

        var buffer: [temp_file_contents.len]u8 = undefined;
        var reader = tmp_file.reader();
        _ = try reader.read(&buffer);
        oc.assert(std.mem.eql(u8, temp_file_contents, &buffer), "should have read what was in the original buffer", .{}, @src());
    }
}
