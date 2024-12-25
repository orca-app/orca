const std = @import("std");

// I'm currently working on porting the [Orca](https://github.com/orca-app/orca) build process to zig. Currently we use a python script that does multiple things:
// * Downloads 3rd party dependencies such as [depot_tools](https://chromium.googlesource.com/chromium/tools/depot_tools), [angle](https://chromium.googlesource.com/angle/angle), and [dawn](https://dawn.googlesource.com/dawn)
// * Builds angle and dawn using the depot_tools python scripts
// * Determines if build outputs are out of date by hashing build outputs and comparing the result to a stamp file. The benenfit of this is we have the option to build libs in CI and download the temporarily-stored build artifacts without having to build them locally.
// * Generates code according to .json spec files that are then included in the main orca build.
// I'm worried that removing custom build steps will make it

// make separate zig executables that:
// 1. build angle
// 2. build dawn
// 3. generate C code according to bindings

const Build = std.Build;
const Step = Build.Step;
const Module = Build.Module;
const ModuleImport = Module.Import;
const CrossTarget = std.zig.CrossTarget;
const CompileStep = std.Build.Step.Compile;

const CSources = struct {
    files: std.ArrayList([]const u8),
    b: *Build,

    fn init(b: *Build) CSources {
        return .{
            .files = std.ArrayList([]const u8).init(b.allocator),
            .b = b,
        };
    }

    fn collect(sources: *CSources, path: []const u8) !void {
        const cwd: std.fs.Dir = sources.b.build_root.handle;
        const dir: std.fs.Dir = try cwd.openDir(path, .{ .iterate = true });
        var iter: std.fs.Dir.Iterator = dir.iterate();
        while (try iter.next()) |entry| {
            if (entry.kind == .file and std.mem.endsWith(u8, entry.name, ".c")) {
                const filepath = try std.fs.path.join(sources.b.allocator, &.{ path, entry.name });
                try sources.files.append(filepath);
            }
        }
    }

    fn deinit(sources: CSources) void {
        for (sources.files.items) |path| {
            sources.b.allocator.free(path);
        }
        sources.files.deinit();
    }
};

fn makeDir(path: []const u8) !void {
    const cwd = std.fs.cwd();
    cwd.makeDir(path) catch |e| {
        if (e != error.PathAlreadyExists) {
            return e;
        }
    };
}

const LibShas = struct {
    angle: []const u8,
    dawn: []const u8,

    fn find(cwd: std.fs.Dir, allocator: std.mem.Allocator) !LibShas {
        const Helpers = struct {
            fn findFieldIndex(ast: *const std.zig.Ast, current_node: u32, search: []const u8) ?u32 {
                var buf: [2]std.zig.Ast.Node.Index = undefined;
                if (ast.fullStructInit(&buf, current_node)) |struct_ast| {
                    for (struct_ast.ast.fields) |i| {
                        const field_name = ast.tokenSlice(ast.firstToken(i) - 2);
                        if (std.mem.eql(u8, field_name, search)) {
                            return i;
                        }
                    }
                }
                return null;
            }

            // "https://chromium.googlesource.com/angle/angle.git/+archive/8a8c8fc280d74b34731e0e417b19bff7c967388a.tar.gz"
            fn extractCommitFromUrl(url: []const u8) []const u8 {
                const basename = std.fs.path.basename;
                const stem = std.fs.path.stem;
                return stem(stem(basename(url)));
            }
        };

        const contents: []const u8 = try cwd.readFileAlloc(allocator, "build.zig.zon", 1024 * 1024 * 1);
        const contents_z = try allocator.alloc(u8, contents.len + 1);
        @memcpy(contents_z[0..contents.len], contents);
        contents_z[contents.len] = 0;

        // NOTE: there will eventually be a ZON parser in the stdlib, but until then we'll just get by
        // with a hacky version that only looks for the libs we care about
        var ast = try std.zig.Ast.parse(allocator, contents_z[0..contents.len :0], .zon);

        var angle_sha: []const u8 = "";
        var dawn_sha: []const u8 = "";

        const root_index = ast.nodes.items(.data)[0].lhs;
        if (Helpers.findFieldIndex(&ast, root_index, "dependencies")) |deps_index| {
            if (Helpers.findFieldIndex(&ast, deps_index, "angle")) |angle_index| {
                if (Helpers.findFieldIndex(&ast, angle_index, "url")) |url_index| {
                    const url = ast.tokenSlice(ast.nodes.items(.main_token)[url_index]);
                    angle_sha = Helpers.extractCommitFromUrl(url);
                }
            }
            if (Helpers.findFieldIndex(&ast, deps_index, "dawn")) |angle_index| {
                if (Helpers.findFieldIndex(&ast, angle_index, "url")) |url_index| {
                    const url = ast.tokenSlice(ast.nodes.items(.main_token)[url_index]);
                    dawn_sha = Helpers.extractCommitFromUrl(url);
                }
            }
        }

        return LibShas{
            .angle = angle_sha,
            .dawn = dawn_sha,
        };
    }
};

const RunHelpers = struct {
    fn addPythonArg(run: *Build.Step.Run, target: Build.ResolvedTarget, b: *Build) void {
        if (target.result.os.tag == .windows) {
            if (b.lazyDependency("python3-win64", .{})) |python3_dep| {
                const python_path = python3_dep.path("python.exe");
                run.addPrefixedFileArg("--python=", python_path);
            }
        } else {
            run.addArg("--python=python");
        }
    }

    fn addCmakeArg(run: *Build.Step.Run, target: Build.ResolvedTarget, b: *Build) void {
        var lazy_cmake_dep: ?*Build.Dependency = undefined;
        var binary_name: []const u8 = undefined;

        if (target.result.os.tag == .windows) {
            lazy_cmake_dep = b.lazyDependency("cmake-win64", .{});
            binary_name = "cmake.exe";
        } else {
            lazy_cmake_dep = b.lazyDependency("cmake-linux64", .{});
            binary_name = "cmake";
        }

        if (lazy_cmake_dep) |cmake_dep| {
            const subpath = std.fs.path.join(b.allocator, &.{ "bin", binary_name }) catch @panic("OOM");
            const cmake_path = cmake_dep.path(subpath);
            run.addPrefixedFileArg("--cmake=", cmake_path);
        }
    }
};

const GenerateWasmBindingsParams = struct {
    exe: *Build.Step.Compile,
    api: []const u8,
    spec_path: []const u8,
    host_bindings_path: []const u8,
    guest_bindings_path: ?[]const u8 = null,
    guest_include_path: ?[]const u8 = null,
};

fn generateWasmBindings(b: *Build, params: GenerateWasmBindingsParams) *Build.Step.UpdateSourceFiles {
    var copy_outputs_to_src: *Build.Step.UpdateSourceFiles = b.addUpdateSourceFiles();

    var run = b.addRunArtifact(params.exe);
    run.addArg(std.mem.join(b.allocator, "", &.{ "--api-name=", params.api }) catch @panic("OOM"));
    run.addPrefixedFileArg("--spec-path=", b.path(params.spec_path));
    const host_bindings_path = run.addPrefixedOutputFileArg("--bindings-path=", params.host_bindings_path);
    copy_outputs_to_src.addCopyFileToSource(host_bindings_path, params.host_bindings_path);
    if (params.guest_bindings_path) |path| {
        const guest_bindings_path = run.addPrefixedOutputFileArg("--guest-stubs-path=", path);
        copy_outputs_to_src.addCopyFileToSource(guest_bindings_path, path);
    }
    if (params.guest_include_path) |path| {
        run.addPrefixedFileArg("--guest-include-path=", b.path(path));
    }

    copy_outputs_to_src.step.dependOn(&run.step);

    return copy_outputs_to_src;
}

pub fn build(b: *Build) !void {
    const git_version_opt: ?[]const u8 = b.option([]const u8, "version", "Specify the specific git version you want to package") orelse null;

    const cwd = b.build_root.handle;

    // set artifact output directory - a bit of a hack since we're directly overriding
    // the prefix, including whatever the user may have specified with -p, but in Orca's
    // case we always want everything going into build/
    {
        const default_install_dirs = Build.DirList{
            .exe_dir = "bin",
            // orca just simplifies everything by having both exes and libs go to the bin directory
            .lib_dir = "bin",
            // by default headers go into <prefix>/headers, but we want to be able to parent the libc
            // headers to orca-libc/, so by default the header lib is the root of the prefix
            .include_dir = "",
        };
        const output_dir = try cwd.realpathAlloc(b.allocator, "build");
        b.resolveInstallPrefix(output_dir, default_install_dirs);
    }

    const target = b.standardTargetOptions(.{});
    const optimize = b.standardOptimizeOption(.{});

    const shas = try LibShas.find(cwd, b.allocator);

    /////////////////////////////////////////////////////////
    // build_dependencies helper program

    const build_deps_exe: *Build.Step.Compile = b.addExecutable(.{
        .name = "build_dependencies",
        .root_source_file = b.path("src/build/build_dependencies.zig"),
        .target = target,
        .optimize = .Debug,
    });

    try makeDir("build");
    const deps_intermediate_path = try cwd.realpathAlloc(b.allocator, "build");

    /////////////////////////////////////////////////////////
    // angle build + check

    var angle_dep: *Build.Dependency = b.dependency("angle", .{});

    var run_angle_build: *Build.Step.Run = b.addRunArtifact(build_deps_exe);
    run_angle_build.addArg("--lib=angle");
    run_angle_build.addArg(b.fmt("--sha={s}", .{shas.angle}));
    run_angle_build.addArg(b.fmt("--intermediate={s}", .{deps_intermediate_path}));
    run_angle_build.addPrefixedFileArg("--src=", angle_dep.path(""));
    RunHelpers.addPythonArg(run_angle_build, target, b);
    RunHelpers.addCmakeArg(run_angle_build, target, b);

    const build_angle_step = b.step("angle", "Build Angle libs");
    build_angle_step.dependOn(&run_angle_build.step);

    var run_angle_uptodate: *Build.Step.Run = b.addRunArtifact(build_deps_exe);
    run_angle_uptodate.addArg("--check");
    run_angle_uptodate.addArg("--lib=angle");
    run_angle_uptodate.addArg(b.fmt("--sha={s}", .{shas.angle}));
    run_angle_uptodate.addArg(b.fmt("--intermediate={s}", .{deps_intermediate_path}));
    run_angle_uptodate.addPrefixedFileArg("--src=", angle_dep.path(""));
    RunHelpers.addPythonArg(run_angle_uptodate, target, b);
    RunHelpers.addCmakeArg(run_angle_uptodate, target, b);

    /////////////////////////////////////////////////////////
    // dawn build + check

    var dawn_dep: *Build.Dependency = b.dependency("dawn", .{});

    var run_dawn_build: *Build.Step.Run = b.addRunArtifact(build_deps_exe);
    run_dawn_build.addArg("--lib=dawn");
    run_dawn_build.addArg(b.fmt("--sha={s}", .{shas.dawn}));
    run_dawn_build.addArg(b.fmt("--intermediate={s}", .{deps_intermediate_path}));
    run_dawn_build.addPrefixedFileArg("--src=", dawn_dep.path(""));
    RunHelpers.addPythonArg(run_dawn_build, target, b);
    RunHelpers.addCmakeArg(run_dawn_build, target, b);

    const build_dawn_step = b.step("dawn", "Build Dawn libs");
    build_dawn_step.dependOn(&run_dawn_build.step);

    var run_dawn_uptodate: *Build.Step.Run = b.addRunArtifact(build_deps_exe);
    run_dawn_uptodate.addArg("--check");
    run_dawn_uptodate.addArg("--lib=dawn");
    run_dawn_uptodate.addArg(b.fmt("--sha={s}", .{shas.dawn}));
    run_dawn_uptodate.addArg(b.fmt("--intermediate={s}", .{deps_intermediate_path}));
    run_dawn_uptodate.addPrefixedFileArg("--src=", dawn_dep.path(""));
    RunHelpers.addPythonArg(run_dawn_uptodate, target, b);
    RunHelpers.addCmakeArg(run_dawn_uptodate, target, b);

    /////////////////////////////////////////////////////////
    // binding generator

    const bindgen_exe: *Build.Step.Compile = b.addExecutable(.{
        .name = "bindgen",
        .root_source_file = b.path("src/build/bindgen.zig"),
        .target = target,
        .optimize = .Debug,
    });

    const bindgen_install = b.addInstallArtifact(bindgen_exe, .{});

    const bindgen_run: *Build.Step.Run = b.addRunArtifact(bindgen_exe);
    if (b.args) |args| {
        bindgen_run.addArgs(args);
    }
    bindgen_run.step.dependOn(&bindgen_install.step);

    const bindgen_step = b.step("run-bindgen", "Generate wasm bindings from a json spec file");
    bindgen_step.dependOn(&bindgen_run.step);

    /////////////////////////////////////////////////////////
    // Orca runtime and dependencies

    // copy angle + dawn libs to output directory
    var install_angle = b.addInstallDirectory(.{
        .source_dir = b.path("build/angle.out/bin"),
        .install_dir = .bin,
        .install_subdir = "",
    });
    install_angle.step.dependOn(&run_angle_uptodate.step);

    var install_dawn = b.addInstallDirectory(.{
        .source_dir = b.path("build/dawn.out/bin"),
        .install_dir = .bin,
        .install_subdir = "",
    });
    install_dawn.step.dependOn(&run_dawn_uptodate.step);

    // generate wasm bindings

    const orca_runtime_bindgen_core = generateWasmBindings(b, .{
        .exe = bindgen_exe,
        .api = "core",
        .spec_path = "src/wasmbind/core_api.json",
        .host_bindings_path = "src/wasmbind/core_api_bind_gen.c",
        .guest_bindings_path = "src/wasmbind/core_api_stubs.c",
    });

    const orca_runtime_bindgen_surface = generateWasmBindings(b, .{
        .exe = bindgen_exe,
        .api = "surface",
        .spec_path = "src/wasmbind/surface_api.json",
        .host_bindings_path = "src/wasmbind/surface_api_bind_gen.c",
        .guest_bindings_path = "src/wasmbind/orca_surface_stubs.c",
        .guest_include_path = "src/graphics/graphics.h",
    });

    const orca_runtime_bindgen_clock = generateWasmBindings(b, .{
        .exe = bindgen_exe,
        .api = "clock",
        .spec_path = "src/wasmbind/clock_api.json",
        .host_bindings_path = "src/wasmbind/clock_api_bind_gen.c",
        .guest_include_path = "src/platform/platform_clock.h",
    });

    const orca_runtime_bindgen_io = generateWasmBindings(b, .{
        .exe = bindgen_exe,
        .api = "io",
        .spec_path = "src/wasmbind/io_api.json",
        .host_bindings_path = "src/wasmbind/io_api_bind_gen.c",
        .guest_bindings_path = "src/wasmbind/orca_io_stubs.c",
        .guest_include_path = "src/platform/platform_io_dialog.h",
    });

    // TODO port this to zig
    const python_gen_gles_spec = b.addSystemCommand(&.{
        "python.exe",
        "scripts/gles_gen.py",
        "--spec",
        "src/ext/gl.xml",
        "--json",
        "src/wasmbind/gles_api.json",
        "--header",
        "src/graphics/orca_gl31.h",
        "--log",
        "build/gles_gen.log",
    });

    const orca_runtime_bindgen_gles = generateWasmBindings(b, .{
        .exe = bindgen_exe,
        .api = "gles",
        .spec_path = "src/wasmbind/gles_api.json",
        .host_bindings_path = "src/wasmbind/gles_api_bind_gen.c",
    });
    orca_runtime_bindgen_gles.step.dependOn(&python_gen_gles_spec.step);

    // wgpu shaders header

    const gen_header_exe: *Build.Step.Compile = b.addExecutable(.{
        .name = "gen_header",
        .root_source_file = b.path("src/build/gen_header_from_files.zig"),
        .target = target,
        .optimize = .Debug,
    });

    const wgpu_shaders: []const []const u8 = &.{
        "src/graphics/wgsl_shaders/common.wgsl",
        "src/graphics/wgsl_shaders/path_setup.wgsl",
        "src/graphics/wgsl_shaders/segment_setup.wgsl",
        "src/graphics/wgsl_shaders/backprop.wgsl",
        "src/graphics/wgsl_shaders/chunk.wgsl",
        "src/graphics/wgsl_shaders/merge.wgsl",
        "src/graphics/wgsl_shaders/balance_workgroups.wgsl",
        "src/graphics/wgsl_shaders/raster.wgsl",
        "src/graphics/wgsl_shaders/blit.wgsl",
        "src/graphics/wgsl_shaders/final_blit.wgsl",
    };

    var run_gen_wgpu_header: *Build.Step.Run = b.addRunArtifact(gen_header_exe);
    for (wgpu_shaders) |path| {
        run_gen_wgpu_header.addPrefixedFileArg("--file=", b.path(path));
    }
    run_gen_wgpu_header.addArg("--namespace=oc_wgsl_");
    run_gen_wgpu_header.addPrefixedDirectoryArg("--root=", b.path(""));
    const wgpu_header_path = run_gen_wgpu_header.addPrefixedOutputFileArg("--output=", "generated_headers/wgpu_renderer_shaders.h");

    var update_wgpu_header: *Build.Step.UpdateSourceFiles = b.addUpdateSourceFiles();
    update_wgpu_header.addCopyFileToSource(wgpu_header_path, "src/graphics/wgpu_renderer_shaders.h");
    update_wgpu_header.step.dependOn(&run_gen_wgpu_header.step);

    // platform lib

    var orca_platform_compile_flags = std.ArrayList([]const u8).init(b.allocator);
    defer orca_platform_compile_flags.deinit();
    try orca_platform_compile_flags.append("-DOC_BUILD_DLL");
    if (optimize == .Debug) {
        try orca_platform_compile_flags.append("-DOC_DEBUG");
        try orca_platform_compile_flags.append("-DOC_LOG_COMPILE_DEBUG");
    }

    // TODO add manifest file
    // "/MANIFEST:EMBED", "/MANIFESTINPUT:src/app/win32_manifest.xml",

    var orca_platform_lib = b.addSharedLibrary(.{
        .name = "orca_platform",
        .target = target,
        .optimize = optimize,
    });

    orca_platform_lib.step.dependOn(&install_angle.step);
    orca_platform_lib.step.dependOn(&install_dawn.step);
    // orca_platform_lib.step.dependOn(&stage_angle_dawn_headers.step);
    orca_platform_lib.step.dependOn(&update_wgpu_header.step);

    orca_platform_lib.addIncludePath(b.path("src"));
    orca_platform_lib.addIncludePath(b.path("src/ext"));
    orca_platform_lib.addIncludePath(b.path("build/angle.out/include"));
    orca_platform_lib.addIncludePath(b.path("build/dawn.out/include"));

    orca_platform_lib.addCSourceFiles(.{
        .files = &.{"src/orca.c"},
        .flags = orca_platform_compile_flags.items,
    });

    orca_platform_lib.addLibraryPath(b.path("build/angle.out/bin"));
    orca_platform_lib.addLibraryPath(b.path("build/dawn.out/bin"));

    orca_platform_lib.linkSystemLibrary("user32");
    orca_platform_lib.linkSystemLibrary("opengl32");
    orca_platform_lib.linkSystemLibrary("gdi32");
    orca_platform_lib.linkSystemLibrary("dxgi");
    orca_platform_lib.linkSystemLibrary("dxguid");
    orca_platform_lib.linkSystemLibrary("d3d11");
    orca_platform_lib.linkSystemLibrary("dcomp");
    orca_platform_lib.linkSystemLibrary("shcore");
    // orca_platform_lib.linkSystemLibrary("delayimp");
    orca_platform_lib.linkSystemLibrary("dwmapi");
    orca_platform_lib.linkSystemLibrary("comctl32");
    orca_platform_lib.linkSystemLibrary("ole32");
    orca_platform_lib.linkSystemLibrary("shell32");
    orca_platform_lib.linkSystemLibrary("shlwapi");

    orca_platform_lib.linkSystemLibrary("libEGL.dll"); // todo DELAYLOAD?
    orca_platform_lib.linkSystemLibrary("libGLESv2.dll"); // todo DELAYLOAD?
    orca_platform_lib.linkSystemLibrary("webgpu");

    orca_platform_lib.step.dependOn(&orca_runtime_bindgen_core.step);
    orca_platform_lib.step.dependOn(&orca_runtime_bindgen_surface.step);
    orca_platform_lib.step.dependOn(&orca_runtime_bindgen_clock.step);
    orca_platform_lib.step.dependOn(&orca_runtime_bindgen_io.step);
    orca_platform_lib.step.dependOn(&orca_runtime_bindgen_gles.step);

    // wasm3

    var wasm3_sources = CSources.init(b);
    defer wasm3_sources.deinit();
    try wasm3_sources.collect("src/ext/wasm3/source");

    var wasm3_lib = b.addStaticLibrary(.{
        .name = "wasm3",
        .target = target,
        .optimize = optimize,
    });

    wasm3_lib.addIncludePath(b.path("src/ext/wasm3/source"));
    wasm3_lib.addCSourceFiles(.{
        .files = wasm3_sources.files.items,
        .flags = &.{},
    });
    wasm3_lib.linkLibC();

    // orca runtime exe

    var orca_runtime_compile_flags = std.ArrayList([]const u8).init(b.allocator);
    defer orca_runtime_compile_flags.deinit();
    try orca_runtime_compile_flags.append("-DOC_WASM_BACKEND_WASM3=1");
    try orca_runtime_compile_flags.append("-DOC_WASM_BACKEND_BYTEBOX=0");
    if (optimize == .Debug) {
        try orca_runtime_compile_flags.append("-DOC_DEBUG");
        try orca_runtime_compile_flags.append("-DOC_LOG_COMPILE_DEBUG");
    }

    const orca_runtime_exe = b.addExecutable(.{
        .name = "orca_runtime",
        .target = target,
        .optimize = optimize,
    });

    orca_runtime_exe.addIncludePath(b.path("src"));
    orca_runtime_exe.addIncludePath(b.path("src/ext"));
    orca_runtime_exe.addIncludePath(b.path("build/angle.out/include"));
    orca_runtime_exe.addIncludePath(b.path("src/ext/wasm3/source"));

    orca_runtime_exe.addCSourceFiles(.{
        .files = &.{"src/runtime.c"},
        .flags = orca_runtime_compile_flags.items,
    });

    orca_runtime_exe.linkLibrary(wasm3_lib);
    orca_runtime_exe.linkLibrary(orca_platform_lib);
    orca_runtime_exe.linkLibC();

    orca_runtime_exe.step.dependOn(&install_angle.step);
    orca_runtime_exe.step.dependOn(&install_dawn.step);

    orca_runtime_exe.step.dependOn(&orca_runtime_bindgen_core.step);
    orca_runtime_exe.step.dependOn(&orca_runtime_bindgen_surface.step);
    orca_runtime_exe.step.dependOn(&orca_runtime_bindgen_clock.step);
    orca_runtime_exe.step.dependOn(&orca_runtime_bindgen_io.step);
    orca_runtime_exe.step.dependOn(&orca_runtime_bindgen_gles.step);

    const install_runtime_exe: *Build.Step.InstallArtifact = b.addInstallArtifact(orca_runtime_exe, .{});
    // install_step.dependOn(&install_runtime_exe.step);

    const build_runtime_step = b.step("orca-runtime", "Build the Orca runtime from source.");
    build_runtime_step.dependOn(&install_runtime_exe.step);

    ///////////////////////////////////////////////////////
    // orca wasm libc

    var wasm_target_query: std.Target.Query = .{
        .cpu_arch = std.Target.Cpu.Arch.wasm32,
        .os_tag = std.Target.Os.Tag.freestanding,
    };
    wasm_target_query.cpu_features_add.addFeature(@intFromEnum(std.Target.wasm.Feature.bulk_memory));
    wasm_target_query.cpu_features_add.addFeature(@intFromEnum(std.Target.wasm.Feature.nontrapping_fptoint));

    const wasm_target: Build.ResolvedTarget = b.resolveTargetQuery(wasm_target_query);

    // zig fmt: off
    const libc_flags: []const []const u8 = &.{
        // includes
        "-Isrc",
        "-isystem", "src/orca-libc/include",
        "-isystem", "src/orca-libc/include/private",
        "-Isrc/orca-libc/src/arch",
        "-Isrc/orca-libc/src/internal",

        // warnings
        "-Wall", 
        "-Wextra", 
        "-Werror", 
        "-Wno-null-pointer-arithmetic", 
        "-Wno-unused-parameter", 
        "-Wno-sign-compare", 
        "-Wno-unused-variable", 
        "-Wno-unused-function", 
        "-Wno-ignored-attributes", 
        "-Wno-missing-braces", 
        "-Wno-ignored-pragmas", 
        "-Wno-unused-but-set-variable", 
        "-Wno-unknown-warning-option",
        "-Wno-parentheses", 
        "-Wno-shift-op-parentheses",
        "-Wno-bitwise-op-parentheses",
        "-Wno-logical-op-parentheses",
        "-Wno-string-plus-int",
        "-Wno-dangling-else",
        "-Wno-unknown-pragmas",

        // defines
        "-D__ORCA__",
        "-DBULK_MEMORY_THRESHOLD=32",

        // other flags
        "--std=c11",
    };
    // zig fmt: on

    // dummy crt1 object for sysroot folder

    var dummy_crt_obj = b.addObject(.{
        .name = "crt1",
        .target = wasm_target,
        .optimize = optimize,
        .link_libc = false,
    });
    dummy_crt_obj.addCSourceFiles(.{
        .files = &.{"src/orca-libc/src/crt/crt1.c"},
        .flags = libc_flags,
    });

    const libc_install_opts = Build.Step.InstallArtifact.Options{
        .dest_dir = .{ .override = .{ .custom = "orca-libc/lib" } },
    };

    const dummy_crt_install: *Build.Step.InstallArtifact = b.addInstallArtifact(dummy_crt_obj, libc_install_opts);
    // install_step.dependOn(&dummy_crt_install.step);

    // wasm libc with orca platform implementation

    const wasm_libc_source_paths: []const []const u8 = &.{
        "src/orca-libc/src/complex",
        "src/orca-libc/src/crt",
        "src/orca-libc/src/ctype",
        "src/orca-libc/src/errno",
        "src/orca-libc/src/exit",
        "src/orca-libc/src/fenv",
        "src/orca-libc/src/internal",
        "src/orca-libc/src/malloc",
        "src/orca-libc/src/math",
        "src/orca-libc/src/multibyte",
        "src/orca-libc/src/prng",
        "src/orca-libc/src/stdio",
        "src/orca-libc/src/stdlib",
        "src/orca-libc/src/string",
    };

    var wasm_libc_sources = CSources.init(b);
    defer wasm_libc_sources.deinit();

    var wasm_libc_objs = std.ArrayList(*Build.Step.Compile).init(b.allocator);
    for (wasm_libc_source_paths) |path| {
        const basename: []const u8 = std.fs.path.basename(path);
        const obj_name: []const u8 = try std.mem.join(b.allocator, "", &.{ "libc_", basename });
        var obj = b.addObject(.{
            .name = obj_name,
            .target = wasm_target,
            .optimize = optimize,
            .single_threaded = true,
            .link_libc = false,
            .zig_lib_dir = b.path("src/orca-libc"), // ensures c stdlib headers bundled with zig are ignored
        });
        wasm_libc_sources.files.shrinkRetainingCapacity(0);
        try wasm_libc_sources.collect(path);

        obj.addCSourceFiles(.{
            .files = wasm_libc_sources.files.items,
            .flags = libc_flags,
        });
        try wasm_libc_objs.append(obj);
    }

    var wasm_libc_lib = b.addStaticLibrary(.{
        .name = "c",
        .target = wasm_target,
        .optimize = optimize,
        .link_libc = false,
        .single_threaded = true,
    });
    for (wasm_libc_objs.items) |obj| {
        wasm_libc_lib.addObject(obj);
    }

    // wasm_libc_lib.rdynamic = true;
    // wasm_libc_lib.entry = .disabled;

    wasm_libc_lib.installHeadersDirectory(b.path("src/orca-libc/include"), "orca-libc/include", .{});

    const libc_install: *Build.Step.InstallArtifact = b.addInstallArtifact(wasm_libc_lib, libc_install_opts);
    // install_step.dependOn(&libc_install.step);

    const build_libc_step = b.step("orca-libc", "Build the Orca libC from source.");
    build_libc_step.dependOn(&libc_install.step);
    build_libc_step.dependOn(&dummy_crt_install.step);

    /////////////////////////////////////////////////////////
    // Orca wasm SDK

    const wasm_sdk_flags: []const []const u8 = &.{
        "-Isrc",
        "-Isrc/ext",
        "-Isrc/orca-libc/include",
        "--no-standard-libraries",
        "-D__ORCA__",
        // "-Wl,--no-entry",
        // "-Wl,--export-dynamic",
        // "-Wl,--relocatable"
    };

    var wasm_sdk_obj = b.addObject(.{
        .name = "orca_wasm",
        .target = wasm_target,
        .optimize = optimize,
        .link_libc = false,
        .single_threaded = true,
        .zig_lib_dir = b.path("src/orca-libc"),
    });
    wasm_sdk_obj.addCSourceFiles(.{
        .flags = wasm_sdk_flags,
        .files = &.{"src/orca.c"},
    });

    var wasm_sdk_lib = b.addExecutable(.{
        .name = "liborca",
        .target = wasm_target,
        .optimize = optimize,
        .link_libc = false,
        .single_threaded = true,
    });
    wasm_sdk_lib.addObject(wasm_sdk_obj);
    wasm_sdk_lib.rdynamic = true;
    wasm_sdk_lib.entry = .disabled;

    wasm_sdk_lib.step.dependOn(&orca_runtime_bindgen_core.step);
    wasm_sdk_lib.step.dependOn(&orca_runtime_bindgen_surface.step);
    wasm_sdk_lib.step.dependOn(&orca_runtime_bindgen_clock.step);
    wasm_sdk_lib.step.dependOn(&orca_runtime_bindgen_io.step);
    wasm_sdk_lib.step.dependOn(&orca_runtime_bindgen_gles.step);

    // wasm_sdk_lib.step.dependOn(&libc_install.step); // TODO probably needs to depend on the libc artifacts being installed to the build dir

    const wasm_sdk_install: *Build.Step.InstallArtifact = b.addInstallArtifact(wasm_sdk_lib, .{});
    // install_step.dependOn(&wasm_sdk_install.step);

    const build_wasm_sdk_step = b.step("orca-wasm-sdk", "Build the Orca wasm sdk from source.");
    build_wasm_sdk_step.dependOn(&wasm_sdk_install.step);

    /////////////////////////////////////////////////////////
    // Orca CLI tool and dependencies

    // curl

    var curl_compile_flags = std.ArrayList([]const u8).init(b.allocator);
    defer curl_compile_flags.deinit();
    try curl_compile_flags.append("-DCURL_STATICLIB");
    try curl_compile_flags.append("-DBUILDING_LIBCURL");
    if (optimize == .Debug) {
        try curl_compile_flags.append("-D_DEBUG");
    } else {
        try curl_compile_flags.append("-DNDEBUG");
    }

    var curl_sources = CSources.init(b);
    defer curl_sources.deinit();
    try curl_sources.collect("src/ext/curl/lib");
    try curl_sources.collect("src/ext/curl/lib/vauth");
    try curl_sources.collect("src/ext/curl/lib/vtls");
    try curl_sources.collect("src/ext/curl/lib/vssh");
    try curl_sources.collect("src/ext/curl/lib/vquic");

    var curl_lib = b.addStaticLibrary(.{
        .name = "curl",
        .target = target,
        .optimize = optimize,
    });
    curl_lib.addIncludePath(b.path("src/ext/curl/include"));
    curl_lib.addIncludePath(b.path("src/ext/curl/lib"));
    curl_lib.addCSourceFiles(.{
        .files = curl_sources.files.items,
        .flags = curl_compile_flags.items,
    });

    // TOOD figure out if we need to include the RC file

    curl_lib.linkSystemLibrary("ws2_32");
    curl_lib.linkSystemLibrary("wldap32");
    curl_lib.linkSystemLibrary("advapi32");
    curl_lib.linkSystemLibrary("crypt32");
    curl_lib.linkSystemLibrary("gdi32");
    curl_lib.linkSystemLibrary("user32");
    curl_lib.linkSystemLibrary("bcrypt");
    curl_lib.linkLibC();

    // zlib

    var z_sources = CSources.init(b);
    defer z_sources.deinit();
    try z_sources.collect("src/ext/zlib/");

    var z_lib = b.addStaticLibrary(.{
        .name = "z",
        .target = target,
        .optimize = optimize,
    });
    z_lib.addIncludePath(b.path("src/ext/zlib/"));
    z_lib.addCSourceFiles(.{
        .files = z_sources.files.items,
        .flags = &.{
            "-DHAVE_SYS_TYPES_H",
            "-DHAVE_STDINT_H",
            "-DHAVE_STDDEF_H",
            "-DZ_HAVE_UNISTD_H",
        },
    });
    z_lib.linkLibC();

    // orca cli tool

    const git_version: []const u8 = blk: {
        if (git_version_opt) |git_version| {
            break :blk try b.allocator.dupe(u8, git_version);
        } else {
            const git_version = b.run(&.{ "git", "rev-parse", "--short", "HEAD" });
            break :blk std.mem.trimRight(u8, git_version, "\n");
        }
    };

    var orca_tool_compile_flags = std.ArrayList([]const u8).init(b.allocator);
    defer orca_tool_compile_flags.deinit();
    try orca_tool_compile_flags.append("-DFLAG_IMPLEMENTATION");
    try orca_tool_compile_flags.append("-DOC_NO_APP_LAYER");
    try orca_tool_compile_flags.append("-DOC_BUILD_DLL");
    try orca_tool_compile_flags.append("-DCURL_STATICLIB");
    try orca_tool_compile_flags.append(b.fmt("-DORCA_TOOL_VERSION={s}", .{git_version}));

    if (optimize == .Debug) {
        try orca_tool_compile_flags.append("-DOC_DEBUG");
        try orca_tool_compile_flags.append("-DOC_LOG_COMPILE_DEBUG");
    }

    const orca_tool_exe = b.addExecutable(.{
        .name = "orca_tool",
        .target = target,
        .optimize = optimize,
    });
    orca_tool_exe.addIncludePath(b.path("src/"));
    orca_tool_exe.addIncludePath(b.path("src/tool"));
    orca_tool_exe.addIncludePath(b.path("src/ext/stb"));
    orca_tool_exe.addIncludePath(b.path("src/ext/curl/include"));
    orca_tool_exe.addIncludePath(b.path("src/ext/zlib"));
    orca_tool_exe.addIncludePath(b.path("src/ext/microtar"));

    orca_tool_exe.addCSourceFiles(.{
        .files = &.{"src/tool/main.c"},
        .flags = orca_tool_compile_flags.items,
    });

    orca_tool_exe.linkLibrary(curl_lib);
    orca_tool_exe.linkLibrary(z_lib);
    orca_tool_exe.linkSystemLibrary("shlwapi");
    orca_tool_exe.linkSystemLibrary("shell32");
    orca_tool_exe.linkSystemLibrary("ole32");
    orca_tool_exe.linkSystemLibrary("kernel32");

    orca_tool_exe.step.dependOn(&curl_lib.step);
    orca_tool_exe.step.dependOn(&z_lib.step);
    orca_tool_exe.linkLibC();

    const orca_tool_install: *Build.Step.InstallArtifact = b.addInstallArtifact(orca_tool_exe, .{});
    // install_step.dependOn(&orca_tool_install.step);

    const build_tool_step = b.step("orca-tool", "Build the Orca CLI tool from source.");
    build_tool_step.dependOn(&orca_tool_install.step);

    ///////////////////////////////////////////////////////////////
    // zig build orca

    const build_orca = b.step("orca", "Build all orca binaries");
    build_orca.dependOn(&install_runtime_exe.step);
    build_orca.dependOn(&dummy_crt_install.step);
    build_orca.dependOn(&libc_install.step);
    build_orca.dependOn(&wasm_sdk_install.step);
    build_orca.dependOn(&orca_tool_install.step);

    ///////////////////////////////////////////////////////////////
    // package-sdk and install-sdk commands

    const package_sdk_exe: *Build.Step.Compile = b.addExecutable(.{
        .name = "package_sdk",
        .root_source_file = b.path("src/build/package_sdk.zig"),
        .target = target,
        .optimize = .Debug,
    });

    var package_sdk_to_dir = b.addRunArtifact(package_sdk_exe);
    package_sdk_to_dir.addPrefixedFileArg("--artifacts-path=", b.path("build"));
    package_sdk_to_dir.addPrefixedFileArg("--resources-path=", b.path("resources"));
    package_sdk_to_dir.addPrefixedFileArg("--src-path=", b.path("src"));

    const opt_sdk_install_dir = b.option([]const u8, "sdk-dir", "Specify absolute path for package-sdk and install-sdk files.");
    if (opt_sdk_install_dir) |sdk_install_dir| {
        if (std.fs.path.isAbsolute(sdk_install_dir)) {
            const install_path = try std.mem.join(b.allocator, "", &.{ "--install-path=", sdk_install_dir });
            package_sdk_to_dir.addArg(install_path);
        } else {
            const fail_absolute_sdk_dir = b.addFail("sdk-dir must be an absolute path");
            package_sdk_to_dir.step.dependOn(&fail_absolute_sdk_dir.step);
        }
    } else {
        const fail_missing_sdk_dir = b.addFail("Specifying -Dsdk-dir=<path> is required for package-sdk.");
        package_sdk_to_dir.step.dependOn(&fail_missing_sdk_dir.step);
    }

    const opt_sdk_version = b.option([]const u8, "sdk-version", "Override current git version for sdk packaging.");
    if (opt_sdk_version) |sdk_version| {
        const version = try std.mem.join(b.allocator, "", &.{ "--version=", sdk_version });
        package_sdk_to_dir.addArg(version);
    }

    // const opt_sdk_git_version = b.option([]const u8, "git-version", "Specify the git version used for package-sdk and install-sdk.");

    // package command
    const package_sdk_step = b.step("package-sdk", "Packages the Orca SDK for a release.");
    package_sdk_step.dependOn(&package_sdk_to_dir.step);

    // if (opt_sdk_install_dir) |sdk_install_dir| {
    //     if (std.fs.path.isAbsolute(sdk_install_dir)) {
    //         const sdk_install_dir_lazypath = Build.LazyPath{ .cwd_relative = sdk_install_dir };
    //         const package_sdk_clean_dir = b.addRemoveDirTree(sdk_install_dir_lazypath);

    //         var files = std.ArrayList([]const u8).init(b.allocator);
    //         if (target.result.os.tag == .windows) {
    //             try files.append("orca.exe");
    //             try files.append("orca.dll");
    //             try files.append("orca_runtime.exe");
    //             try files.append("liborca_wasm.a");
    //             try files.append("libEGL.dll");
    //             try files.append("libGLESv2.dll");
    //             try files.append("webgpu.dll");
    //         } else {
    //             try files.append("orca");
    //             try files.append("orca_runtime");
    //             try files.append("liborca.dylib");
    //             try files.append("liborca_wasm.a");
    //             try files.append("libEGL.dylib");
    //             try files.append("libGLESv2.dylib");
    //             try files.append("libwebgpu.dylib");
    //         }

    //         var package_sdk_stage_files = b.addUpdateSourceFiles();
    //         const artifacts_dir = b.path("build/bin");
    //         for (files.items) |filename| {
    //             const source = artifacts_dir.path(b, filename);
    //             const dest = b.pathJoin(&.{ sdk_install_dir, filename });
    //             package_sdk_stage_files.addCopyFileToSource(source, dest);
    //         }

    //         package_sdk_stage_files.step.dependOn(&package_sdk_clean_dir.step);

    //         package_sdk_step.dependOn(&package_sdk_stage_files.step);
    //     } else {
    //         const fail_absolute_sdk_dir = b.addFail("sdk-dir must be an absolute path");
    //         package_sdk_step.dependOn(&fail_absolute_sdk_dir.step);
    //     }
    // } else {
    //     const fail_missing_sdk_dir = b.addFail("Specifying -Dsdk-dir=<path> is required for package-sdk.");
    //     package_sdk_step.dependOn(&fail_missing_sdk_dir.step);
    // }

    // install-sdk command

    // var orca_system_path: Build.LazyPath = undefined;
    // if (opt_install_dir) |install_dir| {
    //     orca_system_path = b.path(install_dir);
    // } else {
    //     var run_find_orca_install_path: *Build.Step.Run = b.addRunArtifact(orca_tool_exe);
    //     run_angle_build.addArg("install-path");
    //     run_find_orca_install_path.captureStdOut();
    // }

    // var current_commit: Build.LazyPath = undefined;
    // if (opt_git_version) |git_version| {
    //     current_commit = b.path(git_version);
    // } else {
    //     var run_find_orca_commit: *Build.Step.Run = b.addSystemCommand(&.{ "git", "rev-parse", "--short", "HEAD" });
    //     current_commit = run_find_orca_commit.captureStdOut();
    // }

    // const install_sdk_step = b.step("install-sdk", "Install a dev build of the Orca tools into the system Orca directory.")

    ///////////////////////////////////////////////////////////////
    // TODO bundle command ?

    // python_build_libc.step.dependOn(&orca_runtime_exe.step);
    // python_build_sdk.step.dependOn(&python_build_libc.step);
    // orca_tool_exe.step.dependOn(&python_build_sdk.step);
    // python_build_tool.step.dependOn(&python_build_sdk.step);

    // install_step.dependOn(&orca_tool_exe.step);

    // ensure_programs()

    // build_runtime_internal(args.release, args.wasm_backend) # this also builds the platform layer
    // build_libc_internal(args.release)
    // build_sdk_internal(args.release)
    // build_tool(args)

    // const python_install =

    // install_step.dependOn(&python_install.step);

    /////////////////////////////////////////////////////////////////
    // zig build clean

    const clean_step: *Build.Step = b.step("clean", "Delete all build artifacts and start fresh.");

    const clean_paths = [_][]const u8{
        // folders
        // ".zig-cache",
        // "build",
        "src/ext/angle",
        "src/ext/dawn",
        "scripts/files",
        "scripts/__pycache",

        // files
        "src/wasmbind/clock_api_bind_gen.c",
        "src/wasmbind/core_api_bind_gen.c",
        "src/wasmbind/core_api_stubs.c",
        "src/wasmbind/gles_api.json",
        "src/wasmbind/gles_api_bind_gen.c",
        "src/wasmbind/io_api_bind_gen.c",
        "src/wasmbind/orca_io_stubs.c",
        "src/wasmbind/orca_surface_stubs.c",
        "src/wasmbind/surface_api_bind_gen.c",
    };
    for (clean_paths) |path| {
        var remove_dir = b.addRemoveDirTree(b.path(path));
        clean_step.dependOn(&remove_dir.step);
    }

    b.getUninstallStep().dependOn(clean_step);

    /////////////////////////////////////////////////////////////////
    // tests

    // print("Removing build artifacts...")
    // yeetdir("build")
    // yeetdir("src/ext/angle")
    // yeetdir("src/ext/dawn")
    // yeetdir("scripts/files")
    // yeetdir("scripts/__pycache__")

    // build_all_step.dependOn(&python_build_all.step);

    // test_step.dependOn(&run_lib_unit_tests.step);
    // test_step.dependOn(&run_exe_unit_tests.step);

    // const run_lib_unit_tests = b.addRunArtifact(lib_unit_tests);

    // const exe = b.addExecutable(.{
    //     .name = "zig_init",
    //     .root_source_file = b.path("src/main.zig"),
    //     .target = target,
    //     .optimize = optimize,
    // });

    // // This declares intent for the executable to be installed into the
    // // standard location when the user invokes the "install" step (the default
    // // step when running `zig build`).
    // b.installArtifact(exe);

    // // This *creates* a Run step in the build graph, to be executed when another
    // // step is evaluated that depends on it. The next line below will establish
    // // such a dependency.
    // const run_cmd = b.addRunArtifact(exe);

    // // By making the run step depend on the install step, it will be run from the
    // // installation directory rather than directly from within the cache directory.
    // // This is not necessary, however, if the application depends on other installed
    // // files, this ensures they will be present and in the expected location.
    // run_cmd.step.dependOn(install_step);

    // // This allows the user to pass arguments to the application in the build
    // // command itself, like this: `zig build run -- arg1 arg2 etc`
    // if (b.args) |args| {
    //     run_cmd.addArgs(args);
    // }

    // This creates a build step. It will be visible in the `zig build --help` menu,
    // and can be selected like this: `zig build run`
    // This will evaluate the `run` step rather than the default, which is "install".
    // const run_step = b.step("run", "Run the app");
    // run_step.dependOn(&run_cmd.step);

    // // Creates a step for unit testing. This only builds the test executable
    // // but does not run it.
    // const lib_unit_tests = b.addTest(.{
    //     .root_source_file = b.path("src/root.zig"),
    //     .target = target,
    //     .optimize = optimize,
    // });

    // const run_lib_unit_tests = b.addRunArtifact(lib_unit_tests);

    // const exe_unit_tests = b.addTest(.{
    //     .root_source_file = b.path("src/main.zig"),
    //     .target = target,
    //     .optimize = optimize,
    // });

    // const run_exe_unit_tests = b.addRunArtifact(exe_unit_tests);

    // // Similar to creating the run step earlier, this exposes a `test` step to
    // // the `zig build --help` menu, providing a way for the user to request
    // // running the unit tests.
    // const test_step = b.step("test", "Run unit tests");
    // test_step.dependOn(&run_lib_unit_tests.step);
    // test_step.dependOn(&run_exe_unit_tests.step);
}
