const std = @import("std");

const Build = std.Build;
const CrossTarget = std.zig.CrossTarget;
const Builder = std.build.Builder;
const CompileStep = std.build.CompileStep;
const InstallFileStep = std.build.InstallFileStep;

const ExeOpts = struct {
    exe_name: []const u8,
    root_src: []const u8,
    step_name: []const u8,
    description: []const u8,
    step_dependencies: ?[]*Build.Step = null,
    should_emit_asm: bool = false,
};

pub fn build(b: *Build) void {
    const should_emit_asm = b.option(bool, "asm", "Emit asm for the bytebox binaries") orelse false;

    const target = b.standardTargetOptions(.{});
    const optimize = b.standardOptimizeOption(.{});

    var bench_add_one_step: *CompileStep = buildWasmLib(b, "bench/samples/add-one.zig", optimize);
    var bench_fibonacci_step: *CompileStep = buildWasmLib(b, "bench/samples/fibonacci.zig", optimize);
    var bench_mandelbrot_step: *CompileStep = buildWasmLib(b, "bench/samples/mandelbrot.zig", optimize);

    const bytebox_module: *Build.Module = b.createModule(.{
        .source_file = Build.LazyPath.relative("src/core.zig"),
    });

    _ = buildExeWithRunStep(b, target, optimize, bytebox_module, .{
        .exe_name = "bytebox",
        .root_src = "run/main.zig",
        .step_name = "run",
        .description = "Run a wasm program",
        .should_emit_asm = should_emit_asm,
    });

    var bench_steps = [_]*Build.Step{
        &bench_add_one_step.step,
        &bench_fibonacci_step.step,
        &bench_mandelbrot_step.step,
    };
    _ = buildExeWithRunStep(b, target, optimize, bytebox_module, .{
        .exe_name = "benchmark",
        .root_src = "bench/main.zig",
        .step_name = "bench",
        .description = "Run the benchmark suite",
        .step_dependencies = &bench_steps,
    });

    const lib_bytebox = b.addStaticLibrary(.{
        .name = "bytebox",
        .root_source_file = .{ .path = "src/cffi.zig" },
        .target = target,
        .optimize = optimize,
    });
    lib_bytebox.installHeader("src/bytebox.h", "bytebox.h");
    b.installArtifact(lib_bytebox);

    // Unit tests
    const unit_tests = b.addTest(.{
        .root_source_file = .{ .path = "src/tests.zig" },
        .target = target,
        .optimize = optimize,
    });
    const run_unit_tests = b.addRunArtifact(unit_tests);
    const unit_test_step = b.step("test-unit", "Run unit tests");
    unit_test_step.dependOn(&run_unit_tests.step);

    // wasm tests
    var wasm_testsuite_step = buildExeWithRunStep(b, target, optimize, bytebox_module, .{
        .exe_name = "testsuite",
        .root_src = "test/main.zig",
        .step_name = "test-wasm",
        .description = "Run the wasm testsuite",
    });

    // wasi tests
    const wasi_testsuite = b.addSystemCommand(&.{"python3"});
    wasi_testsuite.addArg("test/wasi/run.py");
    const wasi_testsuite_step = b.step("test-wasi", "Run wasi testsuite");
    wasi_testsuite_step.dependOn(&wasi_testsuite.step);

    // All tests
    const all_tests_step = b.step("test", "Run unit, wasm, and wasi tests");
    all_tests_step.dependOn(unit_test_step);
    all_tests_step.dependOn(wasm_testsuite_step);
    all_tests_step.dependOn(wasi_testsuite_step);
}

fn buildExeWithRunStep(b: *Build, target: CrossTarget, optimize: std.builtin.Mode, bytebox_module: *Build.Module, opts: ExeOpts) *Build.Step {
    const exe = b.addExecutable(.{
        .name = opts.exe_name,
        .root_source_file = Build.LazyPath.relative(opts.root_src),
        .target = target,
        .optimize = optimize,
    });

    exe.addModule("bytebox", bytebox_module);

    // exe.addModule("bytebox", .{
    //     .source_file = Build.LazyPath.relative("src/core.zig"),
    // });

    // exe.emit_asm = if (opts.should_emit_asm) .emit else .default;
    b.installArtifact(exe);

    if (opts.step_dependencies) |steps| {
        for (steps) |step| {
            exe.step.dependOn(step);
        }
    }

    const run = b.addRunArtifact(exe);
    run.step.dependOn(b.getInstallStep());
    if (b.args) |args| {
        run.addArgs(args);
    }

    const step: *Build.Step = b.step(opts.step_name, opts.description);
    step.dependOn(&run.step);

    return step;
}

fn buildWasmLib(b: *Build, filepath: []const u8, optimize: std.builtin.Mode) *CompileStep {
    var filename: []const u8 = std.fs.path.basename(filepath);
    var filename_no_extension: []const u8 = filename[0 .. filename.len - 4];

    const lib = b.addSharedLibrary(.{
        .name = filename_no_extension,
        .root_source_file = Build.LazyPath.relative(filepath),
        .target = CrossTarget{
            .cpu_arch = .wasm32,
            .os_tag = .freestanding,
        },
        .optimize = optimize,
    });

    // const mode = b.standardOptimizeOption();
    // lib.setBuildMode(mode);
    b.installArtifact(lib);

    return lib;
}
