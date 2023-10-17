const std = @import("std");
const builtin = @import("builtin");

fn addSourceString(str: []const u8, strings: *std.ArrayList(u8), sources: *std.ArrayList([]const u8)) !void {
    var begin = strings.items.len;
    try strings.appendSlice(str);
    var realstring = strings.items[begin..];
    try sources.append(realstring);
}

pub fn build(b: *std.Build) !void {
    const optimize = b.standardOptimizeOption(.{});
    var wasm_target = std.zig.CrossTarget{
        .cpu_arch = .wasm32,
        .os_tag = .freestanding,
    };
    wasm_target.cpu_features_add.addFeature(@intFromEnum(std.Target.wasm.Feature.bulk_memory));

    var orca_source_strings = try std.ArrayList(u8).initCapacity(b.allocator, 1024 * 4);
    var orca_sources = try std.ArrayList([]const u8).initCapacity(b.allocator, 128);
    defer orca_source_strings.deinit();
    defer orca_sources.deinit();

    {
        try addSourceString("../../src/orca.c", &orca_source_strings, &orca_sources);

        var libc_shim_dir = try std.fs.cwd().openIterableDir("../../src/libc-shim/src", .{});
        var walker = try libc_shim_dir.walk(b.allocator);
        defer walker.deinit();

        while (try walker.next()) |entry| {
            const extension = std.fs.path.extension(entry.path);
            if (std.mem.eql(u8, extension, ".c")) {
                var path_buffer: [std.fs.MAX_PATH_BYTES]u8 = undefined;
                var abs_path = try libc_shim_dir.dir.realpath(entry.path, &path_buffer);
                try addSourceString(abs_path, &orca_source_strings, &orca_sources);
            }
        }
    }

    const orca_compile_opts = [_][]const u8{
        "-D__ORCA__",
        "--no-standard-libraries",
        "-fno-builtin",
        "-g",
        "-O2",
        "-mexec-model=reactor",
        "-fno-sanitize=undefined",
        "-isystem ../../src/libc-shim/include",
        "-I../../src",
        "-I../../src/ext",
        "-Wl,--export-dynamic",
    };

    var orca_lib = b.addStaticLibrary(.{
        .name = "orca",
        .target = wasm_target,
        .optimize = optimize,
    });
    orca_lib.rdynamic = true;
    orca_lib.addIncludePath(.{ .path = "../../src" });
    orca_lib.addIncludePath(.{ .path = "../../src/libc-shim/include" });
    orca_lib.addIncludePath(.{ .path = "../../src/ext" });
    orca_lib.addCSourceFiles(orca_sources.items, &orca_compile_opts);

    // builds the wasm module out of the orca C sources and main.zig
    const app_module: *std.Build.Module = b.createModule(.{
        .source_file = .{ .path = "src/main.zig" },
    });
    const wasm_lib = b.addSharedLibrary(.{
        .name = "module",
        .root_source_file = .{ .path = "../../src/orca.zig" },
        .target = wasm_target,
        .optimize = optimize,
    });
    wasm_lib.rdynamic = true;
    wasm_lib.addIncludePath(.{ .path = "../../src" });
    wasm_lib.addIncludePath(.{ .path = "../../src/libc-shim/include" });
    wasm_lib.addIncludePath(.{ .path = "../../ext" });
    wasm_lib.addModule("app", app_module);
    wasm_lib.linkLibrary(orca_lib);

    // copies the wasm module into zig-out/wasm_lib
    b.installArtifact(wasm_lib);

    // Runs the orca build command
    const bundle_cmd_str = [_][]const u8{ "orca", "bundle", "--orca-dir", "../..", "--name", "UI", "--resource-dir", "data", "zig-out/lib/module.wasm" };
    var bundle_cmd = b.addSystemCommand(&bundle_cmd_str);
    bundle_cmd.step.dependOn(b.getInstallStep());

    const bundle_step = b.step("bundle", "Runs the orca toolchain to bundle the wasm module into an orca app.");
    bundle_step.dependOn(&bundle_cmd.step);

    // Runs the app
    const run_cmd_windows = [_][]const u8{"UI/bin/UI.exe"};
    const run_cmd_macos = [_][]const u8{ "open", "UI.app" };
    const run_cmd_str: []const []const u8 = switch (builtin.os.tag) {
        .windows => &run_cmd_windows,
        .macos => &run_cmd_macos,
        else => @compileError("unsupported platform"),
    };
    var run_cmd = b.addSystemCommand(run_cmd_str);
    run_cmd.step.dependOn(&bundle_cmd.step);

    const run_step = b.step("run", "Runs the bundled app using the Orca runtime.");
    run_step.dependOn(&run_cmd.step);
}
