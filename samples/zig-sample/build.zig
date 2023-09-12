const std = @import("std");

fn addSourceString(str: []const u8, strings: *std.ArrayList(u8), sources: *std.ArrayList([]const u8)) !void {
    var begin = strings.items.len;
    try strings.appendSlice(str);
    var realstring = strings.items[begin..];
    try sources.append(realstring);
}

pub fn build(b: *std.Build) !void {
    const optimize = b.standardOptimizeOption(.{});
    const wasm_target = std.zig.CrossTarget{
        .cpu_arch = .wasm32,
        .os_tag = .freestanding,
    };

    //NOTE(reuben) - Ideally we would build the orca wasm lib and link it all within build.zig, but there
    //               seems to be a bug where the -mbulk-memory flag is ignored and memory.copy and
    //               memory.fill instructions aren't generated, leading the memcpy and memset instructions
    //               to become infinitely recursive, crashing the program. So for now we build the orca
    //               wasm lib with clang and link it in here.

    // var orca_source_strings = try std.ArrayList(u8).initCapacity(b.allocator, 1024 * 4);
    // var orca_sources = try std.ArrayList([]const u8).initCapacity(b.allocator, 128);
    // defer orca_source_strings.deinit();
    // defer orca_sources.deinit();

    // {
    //     try addSourceString("../../src/orca.c", &orca_source_strings, &orca_sources);

    //     var libc_shim_dir = try std.fs.cwd().openIterableDir("../../src/libc-shim/src", .{});
    //     var walker = try libc_shim_dir.walk(b.allocator);
    //     defer walker.deinit();

    //     while (try walker.next()) |entry| {
    //         const extension = std.fs.path.extension(entry.path);
    //         if (std.mem.eql(u8, extension, ".c")) {
    //             var path_buffer: [std.fs.MAX_PATH_BYTES]u8 = undefined;
    //             var abs_path = try libc_shim_dir.dir.realpath(entry.path, &path_buffer);
    //             try addSourceString(abs_path, &orca_source_strings, &orca_sources);
    //             // std.debug.print("adding libc shim source: {s}\n", .{abs_path});
    //         }
    //     }
    // }

    // const orca_compile_opts = [_][]const u8{
    //     "-D__ORCA__",
    //     "--no-standard-libraries",
    //     "-fno-builtin",
    //     "-isystem ../../src/libc-shim/include",
    //     "-I../../src",
    //     "-I../../ext",
    //     "-O2",
    //     "-mbulk-memory",
    //     "-Wl,--no-entry",
    //     "-Wl,--export-all",
    //     "-g",
    // };

    // var orca_lib = b.addStaticLibrary(.{
    //     .name = "orca",
    //     .target = wasm_target,
    //     .optimize = optimize,
    // });
    // orca_lib.rdynamic = true;
    // orca_lib.addIncludePath(.{ .path = "../../src" });
    // orca_lib.addIncludePath(.{ .path = "../../src/libc-shim/include" });
    // orca_lib.addIncludePath(.{ .path = "../../ext" });
    // orca_lib.addCSourceFiles(orca_sources.items, &orca_compile_opts);
    // b.installArtifact(orca_lib);

    // builds the wasm module out of the orca C sources and main.zig
    const wasm_lib = b.addSharedLibrary(.{
        .name = "module",
        .root_source_file = .{ .path = "src/main.zig" },
        .target = wasm_target,
        .optimize = optimize,
    });
    wasm_lib.rdynamic = true;

    const orca_module: *std.Build.Module = b.createModule(.{
        .source_file = .{ .path = "../../src/orca.zig" },
    });
    wasm_lib.addIncludePath(.{ .path = "../../src" });
    wasm_lib.addIncludePath(.{ .path = "../../src/libc-shim/include" });
    wasm_lib.addIncludePath(.{ .path = "../../ext" });
    wasm_lib.addModule("orca", orca_module);
    // wasm_lib.linkLibrary(orca_lib);
    wasm_lib.addLibraryPath(.{ .path = "." });
    wasm_lib.linkSystemLibrary("orca");

    // copies the wasm module into zig-out/wasm_lib
    b.installArtifact(wasm_lib);

    // Runs the orca build command
    const bundle_cmd_str = [_][]const u8{ "orca", "bundle", "--orca-dir", "../..", "--name", "Sample", "--icon", "icon.png", "--resource-dir", "data", "zig-out/lib/module.wasm" };
    var bundle_cmd = b.addSystemCommand(&bundle_cmd_str);
    bundle_cmd.step.dependOn(b.getInstallStep());

    const bundle_step = b.step("bundle", "Runs the orca toolchain to bundle the wasm module into an orca app.");
    bundle_step.dependOn(&bundle_cmd.step);
}
