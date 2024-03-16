const std = @import("std");
const complex = std.math.complex;
const Complex = complex.Complex(f32);

const Color = struct {
    R: u8,
    G: u8,
    B: u8,
};

const COLOR_BLACK = Color{ .R = 0, .G = 0, .B = 0 };
const COLOR_WHITE = Color{ .R = 255, .G = 255, .B = 255 };

const WIDTH = 256;
const HEIGHT = 256;

var pixels = [_]Color{.{ .R = 0, .G = 0, .B = 0 }} ** (WIDTH * HEIGHT);

fn mandelbrot(c: Complex, max_counter: i32) Color {
    var counter: u32 = 0;
    var z = Complex.init(0, 0);
    while (counter < max_counter) : (counter += 1) {
        z = z.mul(z).add(c);
        if (2.0 <= complex.abs(z)) {
            return COLOR_WHITE;
        }
    }

    return COLOR_BLACK;
}

export fn run(max_counter: i32) i32 {
    var y: u32 = 0;
    while (y < HEIGHT) : (y += 1) {
        var x: u32 = 0;
        while (x < WIDTH) : (x += 1) {
            const c = Complex.init(@as(f32, @floatFromInt(x)), @as(f32, @floatFromInt(y)));
            const color: Color = mandelbrot(c, max_counter);
            pixels[y * HEIGHT + x] = color;
        }
    }

    return 0;
}
