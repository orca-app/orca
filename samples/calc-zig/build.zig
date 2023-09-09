const std = @import("std");

pub fn build(b: *std.Build) void {
    const optimize = b.standardOptimizeOption(.{});

    // builds the wasm module out of the orca C sources and main.zig
    const lib = b.addSharedLibrary(.{
        .name = "module",
        .root_source_file = .{ .path = "src/main.zig" },
        .target = std.zig.CrossTarget{
            .cpu_arch = .wasm32,
            .os_tag = .freestanding,
        },
        .optimize = optimize,
    });
    lib.rdynamic = true;

    // const orca_sources = [_][]const u8{"../../src/orca.c"}; //, "../../src/libc-shim/src/*.c"
    const orca_sources = [_][]const u8{ "../../src/orca.c", "../../src/libc-shim/src/string.c" };
    const orca_compile_opts = [_][]const u8{
        "-D__ORCA__",
        "--no-standard-libraries",
        "-fno-builtin",
        "-I../../src",
        "-I../../src/libc-shim/include",
        "-I../../ext",
        "-Wl,--export-all",
        "-g",
    };

    // -isystem ..\..\src\libc-shim\include ^
    // -I..\..\ext -I ..\..\src

    lib.addIncludePath(.{ .path = "../../src" });
    lib.addIncludePath(.{ .path = "../../src/libc-shim/include" });
    lib.addIncludePath(.{ .path = "../../ext" });
    lib.addCSourceFiles(&orca_sources, &orca_compile_opts);

    const orca_module: *std.Build.Module = b.createModule(.{
        .source_file = .{ .path = "../../src/orca.zig" },
    });
    lib.addModule("orca", orca_module);

    // copies the wasm module into zig-out/lib
    b.installArtifact(lib);

    // Runs the orca build command
    const bundle_cmd_str = [_][]const u8{ "orca", "bundle", "--orca-dir", "../..", "--name", "Calc", "--icon", "icon.png", "--resource-dir", "data", "zig-out/lib/module.wasm" };
    var bundle_cmd = b.addSystemCommand(&bundle_cmd_str);
    bundle_cmd.step.dependOn(b.getInstallStep());

    const bundle_step = b.step("bundle", "Runs the orca toolchain to bundle the wasm module into an orca app.");
    bundle_step.dependOn(&bundle_cmd.step);
}
