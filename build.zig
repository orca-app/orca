const std = @import("std");

const Build = std.Build;
const LazyPath = Build.LazyPath;
const Step = Build.Step;
const Module = Build.Module;
const ModuleImport = Module.Import;
const CrossTarget = std.zig.CrossTarget;
const CompileStep = std.Build.Step.Compile;

const MACOS_VERSION_MIN = "13.0.0";

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
                const filepath = try std.fs.path.resolve(sources.b.allocator, &.{ path, entry.name });
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

fn pathExists(dir: std.fs.Dir, path: []const u8) bool {
    dir.access(path, .{}) catch {
        return false;
    };
    return true;
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
            // run.addArg("--python=python3");
            // run.addArg("--python=/usr/local/bin/python3");
            run.addArg("--python=/usr/bin/python3");
        }
    }

    fn addCmakeArg(run: *Build.Step.Run, target: Build.ResolvedTarget, b: *Build) void {
        if (target.result.os.tag == .windows) {
            if (b.lazyDependency("cmake-win64", .{})) |dep| {
                const subpath = std.fs.path.join(b.allocator, &.{ "bin", "cmake.exe" }) catch @panic("OOM");
                const cmake_path = dep.path(subpath);
                run.addPrefixedFileArg("--cmake=", cmake_path);
            }
        } else if (target.result.os.tag.isDarwin()) {
            if (b.lazyDependency("cmake-macos", .{})) |dep| {
                const subpath = std.fs.path.join(b.allocator, &.{ "CMake.app", "Contents", "bin", "cmake" }) catch @panic("OOM");
                const cmake_path = dep.path(subpath);
                run.addPrefixedFileArg("--cmake=", cmake_path);
            }
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
        run.addArg(std.mem.join(b.allocator, "", &.{ "--guest-include-path=", path }) catch @panic("OOM"));
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
        try makeDir("build");
        const output_dir = try cwd.realpathAlloc(b.allocator, "build");
        b.resolveInstallPrefix(output_dir, default_install_dirs);
    }

    const target = b.standardTargetOptions(.{});
    const optimize = b.standardOptimizeOption(.{});

    const compile_flag_min_macos_version = b.fmt("-mmacos-version-min={s}", .{MACOS_VERSION_MIN});

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
    var stage_angle_artifacts = b.addUpdateSourceFiles();
    stage_angle_artifacts.addCopyFileToSource(b.path("build/angle.out/include/EGL/egl.h"), "src/ext/angle/include/EGL/egl.h");
    stage_angle_artifacts.addCopyFileToSource(b.path("build/angle.out/include/EGL/eglext.h"), "src/ext/angle/include/EGL/eglext.h");
    stage_angle_artifacts.addCopyFileToSource(b.path("build/angle.out/include/EGL/eglext_angle.h"), "src/ext/angle/include/EGL/eglext_angle.h");
    stage_angle_artifacts.addCopyFileToSource(b.path("build/angle.out/include/EGL/eglplatform.h"), "src/ext/angle/include/EGL/eglplatform.h");
    stage_angle_artifacts.addCopyFileToSource(b.path("build/angle.out/include/GLES/egl.h"), "src/ext/angle/include/GLES/egl.h");
    stage_angle_artifacts.addCopyFileToSource(b.path("build/angle.out/include/GLES/gl.h"), "src/ext/angle/include/GLES/gl.h");
    stage_angle_artifacts.addCopyFileToSource(b.path("build/angle.out/include/GLES/glext.h"), "src/ext/angle/include/GLES/glext.h");
    stage_angle_artifacts.addCopyFileToSource(b.path("build/angle.out/include/GLES/glplatform.h"), "src/ext/angle/include/GLES/glplatform.h");
    stage_angle_artifacts.addCopyFileToSource(b.path("build/angle.out/include/GLES2/gl2.h"), "src/ext/angle/include/GLES2/gl2.h");
    stage_angle_artifacts.addCopyFileToSource(b.path("build/angle.out/include/GLES2/gl2ext.h"), "src/ext/angle/include/GLES2/gl2ext.h");
    stage_angle_artifacts.addCopyFileToSource(b.path("build/angle.out/include/GLES2/gl2ext_angle.h"), "src/ext/angle/include/GLES2/gl2ext_angle.h");
    stage_angle_artifacts.addCopyFileToSource(b.path("build/angle.out/include/GLES2/gl2platform.h"), "src/ext/angle/include/GLES2/gl2platform.h");
    stage_angle_artifacts.addCopyFileToSource(b.path("build/angle.out/include/GLES3/gl3.h"), "src/ext/angle/include/GLES3/gl3.h");
    stage_angle_artifacts.addCopyFileToSource(b.path("build/angle.out/include/GLES3/gl31.h"), "src/ext/angle/include/GLES3/gl31.h");
    stage_angle_artifacts.addCopyFileToSource(b.path("build/angle.out/include/GLES3/gl32.h"), "src/ext/angle/include/GLES3/gl32.h");
    stage_angle_artifacts.addCopyFileToSource(b.path("build/angle.out/include/GLES3/gl3platform.h"), "src/ext/angle/include/GLES3/gl3platform.h");
    stage_angle_artifacts.addCopyFileToSource(b.path("build/angle.out/include/KHR/khrplatform.h"), "src/ext/angle/include/KHR/khrplatform.h");
    if (target.result.os.tag == .windows) {
        stage_angle_artifacts.addCopyFileToSource(b.path("build/angle.out/bin/d3dcompiler_47.dll"), "build/bin/d3dcompiler_47.dll");
        stage_angle_artifacts.addCopyFileToSource(b.path("build/angle.out/bin/libEGL.dll"), "build/bin/libEGL.dll");
        stage_angle_artifacts.addCopyFileToSource(b.path("build/angle.out/bin/libEGL.dll.lib"), "build/bin/libEGL.dll.lib");
        stage_angle_artifacts.addCopyFileToSource(b.path("build/angle.out/bin/libGLESv2.dll"), "build/bin/libGLESv2.dll");
        stage_angle_artifacts.addCopyFileToSource(b.path("build/angle.out/bin/libGLESv2.dll.lib"), "build/bin/libGLESv2.dll.lib");
    } else {
        stage_angle_artifacts.addCopyFileToSource(b.path("build/angle.out/bin/libEGL.dylib"), "build/bin/libEGL.dylib");
        stage_angle_artifacts.addCopyFileToSource(b.path("build/angle.out/bin/libGLESv2.dylib"), "build/bin/libGLESv2.dylib");
    }
    stage_angle_artifacts.step.dependOn(&run_angle_uptodate.step);

    var stage_dawn_artifacts = b.addUpdateSourceFiles();
    stage_dawn_artifacts.addCopyFileToSource(b.path("build/dawn.out/include/webgpu.h"), "src/ext/dawn/include/webgpu.h");
    if (target.result.os.tag == .windows) {
        stage_angle_artifacts.addCopyFileToSource(b.path("build/dawn.out/bin/webgpu.dll"), "build/bin/webgpu.dll");
        stage_angle_artifacts.addCopyFileToSource(b.path("build/dawn.out/bin/webgpu.lib"), "build/bin/webgpu.lib");
    } else {
        stage_angle_artifacts.addCopyFileToSource(b.path("build/dawn.out/bin/libwebgpu.dylib"), "build/bin/libwebgpu.dylib");
    }
    stage_dawn_artifacts.step.dependOn(&run_dawn_uptodate.step);

    // generate GLES API spec from OpenGL XML registry
    // TODO port this to C or Zig
    // TODO use python package dependency to avoid system dependency
    const python_exe_name = if (target.result.os.tag == .windows) "python.exe" else "python3";
    var python_gen_gles_spec_run: *Build.Step.Run = b.addSystemCommand(&.{python_exe_name});
    python_gen_gles_spec_run.addArg("scripts/gles_gen.py");
    python_gen_gles_spec_run.addPrefixedFileArg("--spec=", b.path("src/ext/gl.xml"));
    const gles_api_header = python_gen_gles_spec_run.addPrefixedOutputFileArg("--header=", "orca_gl31.h");
    const gles_api_json = python_gen_gles_spec_run.addPrefixedOutputFileArg("--json=", "gles_api.json");
    const gles_api_log = python_gen_gles_spec_run.addPrefixedOutputFileArg("--log=", "gles_gen.log");

    var stage_gles_api_spec_artifacts = b.addUpdateSourceFiles();
    stage_gles_api_spec_artifacts.step.dependOn(&python_gen_gles_spec_run.step);
    stage_gles_api_spec_artifacts.addCopyFileToSource(gles_api_header, "src/graphics/orca_gl31.h");
    stage_gles_api_spec_artifacts.addCopyFileToSource(gles_api_json, "src/wasmbind/gles_api.json");
    stage_gles_api_spec_artifacts.addCopyFileToSource(gles_api_log, "build/gles_gen.log");

    // generate wasm bindings

    const orca_runtime_bindgen_core: *Build.Step.UpdateSourceFiles = generateWasmBindings(b, .{
        .exe = bindgen_exe,
        .api = "core",
        .spec_path = "src/wasmbind/core_api.json",
        .host_bindings_path = "src/wasmbind/core_api_bind_gen.c",
        .guest_bindings_path = "src/wasmbind/core_api_stubs.c",
    });

    const orca_runtime_bindgen_surface: *Build.Step.UpdateSourceFiles = generateWasmBindings(b, .{
        .exe = bindgen_exe,
        .api = "surface",
        .spec_path = "src/wasmbind/surface_api.json",
        .host_bindings_path = "src/wasmbind/surface_api_bind_gen.c",
        .guest_bindings_path = "src/graphics/orca_surface_stubs.c",
        .guest_include_path = "graphics/graphics.h",
    });

    const orca_runtime_bindgen_clock: *Build.Step.UpdateSourceFiles = generateWasmBindings(b, .{
        .exe = bindgen_exe,
        .api = "clock",
        .spec_path = "src/wasmbind/clock_api.json",
        .host_bindings_path = "src/wasmbind/clock_api_bind_gen.c",
        .guest_include_path = "platform/platform_clock.h",
    });

    const orca_runtime_bindgen_io: *Build.Step.UpdateSourceFiles = generateWasmBindings(b, .{
        .exe = bindgen_exe,
        .api = "io",
        .spec_path = "src/wasmbind/io_api.json",
        .host_bindings_path = "src/wasmbind/io_api_bind_gen.c",
        .guest_bindings_path = "src/platform/orca_io_stubs.c",
        .guest_include_path = "platform/platform_io_dialog.h",
    });

    const orca_runtime_bindgen_gles: *Build.Step.UpdateSourceFiles = generateWasmBindings(b, .{
        .exe = bindgen_exe,
        .api = "gles",
        .spec_path = "src/wasmbind/gles_api.json",
        .host_bindings_path = "src/wasmbind/gles_api_bind_gen.c",
    });
    orca_runtime_bindgen_gles.step.dependOn(&stage_gles_api_spec_artifacts.step);

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
    try orca_platform_compile_flags.append("-std=c11");
    try orca_platform_compile_flags.append("-DOC_BUILD_DLL");
    try orca_platform_compile_flags.append("-D_USE_MATH_DEFINES");
    try orca_platform_compile_flags.append("-fno-sanitize=undefined");
    if (optimize == .Debug) {
        try orca_platform_compile_flags.append("-DOC_DEBUG");
        try orca_platform_compile_flags.append("-DOC_LOG_COMPILE_DEBUG");
    }
    if (target.result.os.tag.isDarwin()) {
        try orca_platform_compile_flags.append(compile_flag_min_macos_version);
    }
    // if (target.result.os.tag == .windows) {
    //     try orca_platform_compile_flags.append("-Wl,--delayload=libEGL.dll");
    //     try orca_platform_compile_flags.append("-Wl,--delayload=libGLESv2.dll");
    //     try orca_platform_compile_flags.append("-Wl,--delayload=webgpu.dll");
    // }
    // if (target.result.os.tag.isDarwin()) {}

    var orca_platform_lib = b.addSharedLibrary(.{
        .name = "orca_platform",
        .target = target,
        .optimize = optimize,
        .win32_manifest = b.path("src/app/win32_manifest.manifest"),
    });

    orca_platform_lib.addIncludePath(b.path("src"));
    orca_platform_lib.addIncludePath(b.path("src/ext"));
    orca_platform_lib.addIncludePath(b.path("src/ext/angle/include"));
    orca_platform_lib.addIncludePath(b.path("src/ext/dawn/include"));

    var orca_platform_files = std.ArrayList([]const u8).init(b.allocator);
    try orca_platform_files.append("src/orca.c");

    orca_platform_lib.addLibraryPath(b.path("build/bin"));

    if (target.result.os.tag == .windows) {
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
        orca_platform_lib.linkSystemLibrary("webgpu"); // todo DELAYLOAD?
    } else if (target.result.os.tag.isDarwin()) {
        orca_platform_lib.linkFramework("Carbon");
        orca_platform_lib.linkFramework("Cocoa");
        orca_platform_lib.linkFramework("Metal");
        orca_platform_lib.linkFramework("QuartzCore");
        orca_platform_lib.linkFramework("UniformTypeIdentifiers");

        // TODO add @rpath stuff?
        // orca_platform_lib.addRPath("@rpath");
        orca_platform_lib.linkSystemLibrary2("EGL", .{ .weak = true });
        orca_platform_lib.linkSystemLibrary2("GLESv2", .{ .weak = true });
        orca_platform_lib.linkSystemLibrary2("webgpu", .{ .weak = true });

        try orca_platform_files.append("src/orca.m");
    }

    orca_platform_lib.addCSourceFiles(.{
        .files = orca_platform_files.items,
        .flags = orca_platform_compile_flags.items,
    });

    orca_platform_lib.step.dependOn(&stage_angle_artifacts.step);
    orca_platform_lib.step.dependOn(&stage_dawn_artifacts.step);
    orca_platform_lib.step.dependOn(&update_wgpu_header.step);

    orca_platform_lib.step.dependOn(&orca_runtime_bindgen_core.step);
    orca_platform_lib.step.dependOn(&orca_runtime_bindgen_surface.step);
    orca_platform_lib.step.dependOn(&orca_runtime_bindgen_clock.step);
    orca_platform_lib.step.dependOn(&orca_runtime_bindgen_io.step);
    orca_platform_lib.step.dependOn(&orca_runtime_bindgen_gles.step);

    const orca_platform_install: *Build.Step.InstallArtifact = b.addInstallArtifact(orca_platform_lib, .{});

    const build_orca_platform_step = b.step("orca-platform", "Build the Orca platform layer from source.");
    build_orca_platform_step.dependOn(&orca_platform_install.step);

    // wasm3

    var wasm3_sources = CSources.init(b);
    defer wasm3_sources.deinit();
    try wasm3_sources.collect("src/ext/wasm3/source");

    var wasm3_lib = b.addStaticLibrary(.{
        .name = "wasm3",
        .target = target,
        .optimize = optimize,
    });

    var wasm3_compile_flags = std.ArrayList([]const u8).init(b.allocator);
    try wasm3_compile_flags.append("-fno-sanitize=undefined");
    if (target.result.os.tag.isDarwin()) {
        try wasm3_compile_flags.append("-foptimize-sibling-calls");
        try wasm3_compile_flags.append("-Wno-extern-initializer");
        try wasm3_compile_flags.append("-Dd_m3VerboseErrorMessages");
        try wasm3_compile_flags.append(compile_flag_min_macos_version);
    }

    wasm3_lib.addIncludePath(b.path("src/ext/wasm3/source"));
    wasm3_lib.addCSourceFiles(.{
        .files = wasm3_sources.files.items,
        .flags = wasm3_compile_flags.items,
    });
    wasm3_lib.linkLibC();

    // orca runtime exe

    var orca_runtime_compile_flags = std.ArrayList([]const u8).init(b.allocator);
    defer orca_runtime_compile_flags.deinit();
    try orca_runtime_compile_flags.append("-DOC_WASM_BACKEND_WASM3=1");
    try orca_runtime_compile_flags.append("-DOC_WASM_BACKEND_BYTEBOX=0"); // TODO remove bytebox support
    try orca_runtime_compile_flags.append("-fno-sanitize=undefined");
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

    orca_runtime_exe.step.dependOn(&stage_angle_artifacts.step);
    orca_runtime_exe.step.dependOn(&stage_dawn_artifacts.step);

    orca_runtime_exe.step.dependOn(&orca_runtime_bindgen_core.step);
    orca_runtime_exe.step.dependOn(&orca_runtime_bindgen_surface.step);
    orca_runtime_exe.step.dependOn(&orca_runtime_bindgen_clock.step);
    orca_runtime_exe.step.dependOn(&orca_runtime_bindgen_io.step);
    orca_runtime_exe.step.dependOn(&orca_runtime_bindgen_gles.step);

    const install_runtime_exe: *Build.Step.InstallArtifact = b.addInstallArtifact(orca_runtime_exe, .{});

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

    const wasm_optimize: std.builtin.OptimizeMode = .ReleaseSmall;

    // zig fmt: off
    const libc_flags: []const []const u8 = &.{
        // need to provide absolute paths to these since we're overriding the default zig lib dir
        b.fmt("-I{s}", .{b.pathFromRoot("src")}),
        "-isystem", b.pathFromRoot("src/orca-libc/src/include"),
        "-isystem", b.pathFromRoot("src/orca-libc/src/include/private"),
        b.fmt("-I{s}", .{b.pathFromRoot("src/orca-libc/src/arch")}),
        b.fmt("-I{s}", .{b.pathFromRoot("src/orca-libc/src/internal")}),

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
        .optimize = wasm_optimize,
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

    // wasm libc
    //
    // NOTE - libc must be built in a 2-stage pass by compiling all the c files into .objs individually and then linking them
    // all together at the end into a static lib. There are a couple reasons for this:
    // 1. The build runner invokes zig.exe with commandline args corresponding to its inputs, and the commandline gets too
    //    long if all the C files are added directly to the static lib. :(
    // 2. Generating a unity build file doesn't work because the .c files have been written assuming individual compilation
    //    and there are lots of constants with conflicting names in different files. For example, see "huge" in csinh.c
    //    and csinhf.c
    // 3. Only one .c file is allowed to correspond to an obj file. We can't combine multiple C files into one obj.

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
    try wasm_libc_objs.ensureUnusedCapacity(1024); // there are 496 .c files in the libc so this should be enough

    for (wasm_libc_source_paths) |path| {
        wasm_libc_sources.files.shrinkRetainingCapacity(0);
        try wasm_libc_sources.collect(path);

        const libc_group: []const u8 = std.fs.path.basename(path);

        for (wasm_libc_sources.files.items) |filepath| {
            const filename: []const u8 = std.fs.path.basename(filepath);
            const obj_name: []const u8 = try std.mem.join(b.allocator, "_", &.{ "libc", libc_group, filename });
            var obj = b.addObject(.{
                .name = obj_name,
                .target = wasm_target,
                .optimize = wasm_optimize,
                .single_threaded = true,
                .link_libc = false,
                .zig_lib_dir = b.path("src/orca-libc"), // ensures c stdlib headers bundled with zig are ignored
            });
            obj.addCSourceFiles(.{
                .files = &.{filepath},
                .flags = libc_flags,
            });
            try wasm_libc_objs.append(obj);
        }
    }

    var wasm_libc_lib = b.addStaticLibrary(.{
        .name = "c",
        .target = wasm_target,
        .optimize = wasm_optimize,
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

    const build_libc_step = b.step("orca-libc", "Build the Orca libC from source.");
    build_libc_step.dependOn(&libc_install.step);
    build_libc_step.dependOn(&dummy_crt_install.step);

    /////////////////////////////////////////////////////////
    // Orca wasm SDK

    const wasm_sdk_flags: []const []const u8 = &.{
        // "-Isrc",
        // "-Isrc/ext",
        // "-Isrc/orca-libc/include",
        b.fmt("-I{s}", .{b.pathFromRoot("src")}),
        b.fmt("-I{s}", .{b.pathFromRoot("src/ext")}),
        b.fmt("-I{s}", .{b.pathFromRoot("src/orca-libc/include")}),
        "--no-standard-libraries",
        "-D__ORCA__",
        // "-Wl,--no-entry",
        // "-Wl,--export-dynamic",
        // "-Wl,--relocatable"
    };

    var wasm_sdk_obj = b.addObject(.{
        .name = "orca_wasm",
        .target = wasm_target,
        .optimize = wasm_optimize,
        .link_libc = false,
        .single_threaded = true,
        .zig_lib_dir = b.path("src/orca-libc"),
    });
    wasm_sdk_obj.addCSourceFile(.{
        .file = b.path("src/orca.c"),
        .flags = wasm_sdk_flags,
    });

    wasm_sdk_obj.step.dependOn(&stage_angle_artifacts.step);
    wasm_sdk_obj.step.dependOn(&stage_dawn_artifacts.step);

    wasm_sdk_obj.step.dependOn(&orca_runtime_bindgen_core.step);
    wasm_sdk_obj.step.dependOn(&orca_runtime_bindgen_surface.step);
    wasm_sdk_obj.step.dependOn(&orca_runtime_bindgen_clock.step);
    wasm_sdk_obj.step.dependOn(&orca_runtime_bindgen_io.step);
    wasm_sdk_obj.step.dependOn(&orca_runtime_bindgen_gles.step);

    var wasm_sdk_lib = b.addStaticLibrary(.{
        .name = "orca_wasm",
        .target = wasm_target,
        .optimize = wasm_optimize,
        .link_libc = false,
        .single_threaded = true,
    });
    wasm_sdk_lib.addObject(wasm_sdk_obj);

    const wasm_sdk_install: *Build.Step.InstallArtifact = b.addInstallArtifact(wasm_sdk_lib, .{});

    const build_wasm_sdk_step = b.step("orca-wasm-sdk", "Build the Orca wasm sdk from source.");
    build_wasm_sdk_step.dependOn(&wasm_sdk_install.step);

    /////////////////////////////////////////////////////////
    // Orca CLI tool and dependencies

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

    // curl

    // Original code MIT licensed from: https://github.com/jiacai2050/zig-curl/blob/main/libs/curl.zig
    const curl_lib = b.addStaticLibrary(.{
        .name = "curl",
        .target = target,
        .optimize = optimize,
        .link_libc = true,
    });

    var curl_sources = CSources.init(b);
    try curl_sources.collect("src/ext/curl/lib");
    try curl_sources.collect("src/ext/curl/lib/vauth");
    try curl_sources.collect("src/ext/curl/lib/vtls");
    try curl_sources.collect("src/ext/curl/lib/vssh");
    try curl_sources.collect("src/ext/curl/lib/vquic");

    for (curl_sources.files.items) |path| {
        curl_lib.addCSourceFile(.{
            .file = b.path(path),
            .flags = &.{"-std=gnu89"},
        });
    }

    curl_lib.addIncludePath(b.path("src/ext/curl/lib"));
    curl_lib.addIncludePath(b.path("src/ext/curl/include"));
    curl_lib.addIncludePath(b.path("src/ext/zlib"));

    curl_lib.root_module.addCMacro("BUILDING_LIBCURL", "1");
    curl_lib.root_module.addCMacro("CURL_STATICLIB", "1");
    curl_lib.root_module.addCMacro("CURL_DISABLE_LDAP", "1");
    curl_lib.root_module.addCMacro("CURL_DISABLE_LDAPS", "1");
    // curl_lib.root_module.addCMacro("USE_MBEDTLS", "1");
    curl_lib.root_module.addCMacro("CURL_DISABLE_DICT", "1");
    curl_lib.root_module.addCMacro("CURL_DISABLE_FILE", "1");
    curl_lib.root_module.addCMacro("CURL_DISABLE_FTP", "1");
    curl_lib.root_module.addCMacro("CURL_DISABLE_GOPHER", "1");
    curl_lib.root_module.addCMacro("CURL_DISABLE_IMAP", "1");
    curl_lib.root_module.addCMacro("CURL_DISABLE_MQTT", "1");
    curl_lib.root_module.addCMacro("CURL_DISABLE_POP3", "1");
    curl_lib.root_module.addCMacro("CURL_DISABLE_RTSP", "1");
    curl_lib.root_module.addCMacro("CURL_DISABLE_SMB", "1");
    curl_lib.root_module.addCMacro("CURL_DISABLE_SMTP", "1");
    curl_lib.root_module.addCMacro("CURL_DISABLE_TELNET", "1");
    curl_lib.root_module.addCMacro("CURL_DISABLE_TFTP", "1");
    curl_lib.root_module.addCMacro("HAVE_LIBZ", "1");
    curl_lib.root_module.addCMacro("HAVE_ZLIB_H", "1");

    if (target.result.os.tag == .windows) {
        curl_lib.linkSystemLibrary("ws2_32");
        curl_lib.linkSystemLibrary("wldap32");
        curl_lib.linkSystemLibrary("advapi32");
        curl_lib.linkSystemLibrary("crypt32");
        curl_lib.linkSystemLibrary("gdi32");
        curl_lib.linkSystemLibrary("user32");
        curl_lib.linkSystemLibrary("bcrypt");
    } else {
        curl_lib.root_module.addCMacro("CURL_EXTERN_SYMBOL", "__attribute__ ((__visibility__ (\"default\"))");

        if (target.result.isDarwin() == false) {
            curl_lib.root_module.addCMacro("ENABLE_IPV6", "1");
            curl_lib.root_module.addCMacro("HAVE_GETHOSTBYNAME_R", "1");
            curl_lib.root_module.addCMacro("HAVE_MSG_NOSIGNAL", "1");
        }

        if (target.result.os.tag == .linux) {
            curl_lib.root_module.addCMacro("HAVE_LINUX_TCP_H", "1");
        }

        curl_lib.root_module.addCMacro("HAVE_ALARM", "1");
        curl_lib.root_module.addCMacro("HAVE_ALLOCA_H", "1");
        curl_lib.root_module.addCMacro("HAVE_ARPA_INET_H", "1");
        curl_lib.root_module.addCMacro("HAVE_ARPA_TFTP_H", "1");
        curl_lib.root_module.addCMacro("HAVE_ASSERT_H", "1");
        curl_lib.root_module.addCMacro("HAVE_BASENAME", "1");
        curl_lib.root_module.addCMacro("HAVE_BOOL_T", "1");
        curl_lib.root_module.addCMacro("HAVE_BUILTIN_AVAILABLE", "1");
        curl_lib.root_module.addCMacro("HAVE_CLOCK_GETTIME_MONOTONIC", "1");
        curl_lib.root_module.addCMacro("HAVE_DLFCN_H", "1");
        curl_lib.root_module.addCMacro("HAVE_ERRNO_H", "1");
        curl_lib.root_module.addCMacro("HAVE_FCNTL", "1");
        curl_lib.root_module.addCMacro("HAVE_FCNTL_H", "1");
        curl_lib.root_module.addCMacro("HAVE_FCNTL_O_NONBLOCK", "1");
        curl_lib.root_module.addCMacro("HAVE_FREEADDRINFO", "1");
        curl_lib.root_module.addCMacro("HAVE_FTRUNCATE", "1");
        curl_lib.root_module.addCMacro("HAVE_GETADDRINFO", "1");
        curl_lib.root_module.addCMacro("HAVE_GETEUID", "1");
        curl_lib.root_module.addCMacro("HAVE_GETPPID", "1");
        curl_lib.root_module.addCMacro("HAVE_GETHOSTBYNAME", "1");
        curl_lib.root_module.addCMacro("HAVE_GETHOSTBYNAME_R_6", "1");
        curl_lib.root_module.addCMacro("HAVE_GETHOSTNAME", "1");
        curl_lib.root_module.addCMacro("HAVE_GETPPID", "1");
        curl_lib.root_module.addCMacro("HAVE_GETPROTOBYNAME", "1");
        curl_lib.root_module.addCMacro("HAVE_GETPEERNAME", "1");
        curl_lib.root_module.addCMacro("HAVE_GETSOCKNAME", "1");
        curl_lib.root_module.addCMacro("HAVE_IF_NAMETOINDEX", "1");
        curl_lib.root_module.addCMacro("HAVE_GETPWUID", "1");
        curl_lib.root_module.addCMacro("HAVE_GETPWUID_R", "1");
        curl_lib.root_module.addCMacro("HAVE_GETRLIMIT", "1");
        curl_lib.root_module.addCMacro("HAVE_GETTIMEOFDAY", "1");
        curl_lib.root_module.addCMacro("HAVE_GMTIME_R", "1");
        curl_lib.root_module.addCMacro("HAVE_IFADDRS_H", "1");
        curl_lib.root_module.addCMacro("HAVE_INET_ADDR", "1");
        curl_lib.root_module.addCMacro("HAVE_INET_PTON", "1");
        curl_lib.root_module.addCMacro("HAVE_SA_FAMILY_T", "1");
        curl_lib.root_module.addCMacro("HAVE_INTTYPES_H", "1");
        curl_lib.root_module.addCMacro("HAVE_IOCTL", "1");
        curl_lib.root_module.addCMacro("HAVE_IOCTL_FIONBIO", "1");
        curl_lib.root_module.addCMacro("HAVE_IOCTL_SIOCGIFADDR", "1");
        curl_lib.root_module.addCMacro("HAVE_LDAP_URL_PARSE", "1");
        curl_lib.root_module.addCMacro("HAVE_LIBGEN_H", "1");
        curl_lib.root_module.addCMacro("HAVE_IDN2_H", "1");
        curl_lib.root_module.addCMacro("HAVE_LL", "1");
        curl_lib.root_module.addCMacro("HAVE_LOCALE_H", "1");
        curl_lib.root_module.addCMacro("HAVE_LOCALTIME_R", "1");
        curl_lib.root_module.addCMacro("HAVE_LONGLONG", "1");
        curl_lib.root_module.addCMacro("HAVE_MALLOC_H", "1");
        curl_lib.root_module.addCMacro("HAVE_MEMORY_H", "1");
        curl_lib.root_module.addCMacro("HAVE_NETDB_H", "1");
        curl_lib.root_module.addCMacro("HAVE_NETINET_IN_H", "1");
        curl_lib.root_module.addCMacro("HAVE_NETINET_TCP_H", "1");
        curl_lib.root_module.addCMacro("HAVE_NET_IF_H", "1");
        curl_lib.root_module.addCMacro("HAVE_PIPE", "1");
        curl_lib.root_module.addCMacro("HAVE_POLL", "1");
        curl_lib.root_module.addCMacro("HAVE_POLL_FINE", "1");
        curl_lib.root_module.addCMacro("HAVE_POLL_H", "1");
        curl_lib.root_module.addCMacro("HAVE_POSIX_STRERROR_R", "1");
        curl_lib.root_module.addCMacro("HAVE_PTHREAD_H", "1");
        curl_lib.root_module.addCMacro("HAVE_PWD_H", "1");
        curl_lib.root_module.addCMacro("HAVE_RECV", "1");
        curl_lib.root_module.addCMacro("HAVE_SELECT", "1");
        curl_lib.root_module.addCMacro("HAVE_SEND", "1");
        curl_lib.root_module.addCMacro("HAVE_FSETXATTR", "1");
        curl_lib.root_module.addCMacro("HAVE_FSETXATTR_5", "1");
        curl_lib.root_module.addCMacro("HAVE_SETJMP_H", "1");
        curl_lib.root_module.addCMacro("HAVE_SETLOCALE", "1");
        curl_lib.root_module.addCMacro("HAVE_SETRLIMIT", "1");
        curl_lib.root_module.addCMacro("HAVE_SETSOCKOPT", "1");
        curl_lib.root_module.addCMacro("HAVE_SIGACTION", "1");
        curl_lib.root_module.addCMacro("HAVE_SIGINTERRUPT", "1");
        curl_lib.root_module.addCMacro("HAVE_SIGNAL", "1");
        curl_lib.root_module.addCMacro("HAVE_SIGNAL_H", "1");
        curl_lib.root_module.addCMacro("HAVE_SIGSETJMP", "1");
        curl_lib.root_module.addCMacro("HAVE_SOCKADDR_IN6_SIN6_SCOPE_ID", "1");
        curl_lib.root_module.addCMacro("HAVE_SOCKET", "1");
        curl_lib.root_module.addCMacro("HAVE_STDBOOL_H", "1");
        curl_lib.root_module.addCMacro("HAVE_STDINT_H", "1");
        curl_lib.root_module.addCMacro("HAVE_STDIO_H", "1");
        curl_lib.root_module.addCMacro("HAVE_STDLIB_H", "1");
        curl_lib.root_module.addCMacro("HAVE_STRCASECMP", "1");
        curl_lib.root_module.addCMacro("HAVE_STRDUP", "1");
        curl_lib.root_module.addCMacro("HAVE_STRERROR_R", "1");
        curl_lib.root_module.addCMacro("HAVE_STRINGS_H", "1");
        curl_lib.root_module.addCMacro("HAVE_STRING_H", "1");
        curl_lib.root_module.addCMacro("HAVE_STRSTR", "1");
        curl_lib.root_module.addCMacro("HAVE_STRTOK_R", "1");
        curl_lib.root_module.addCMacro("HAVE_STRTOLL", "1");
        curl_lib.root_module.addCMacro("HAVE_STRUCT_SOCKADDR_STORAGE", "1");
        curl_lib.root_module.addCMacro("HAVE_STRUCT_TIMEVAL", "1");
        curl_lib.root_module.addCMacro("HAVE_SYS_IOCTL_H", "1");
        curl_lib.root_module.addCMacro("HAVE_SYS_PARAM_H", "1");
        curl_lib.root_module.addCMacro("HAVE_SYS_POLL_H", "1");
        curl_lib.root_module.addCMacro("HAVE_SYS_RESOURCE_H", "1");
        curl_lib.root_module.addCMacro("HAVE_SYS_SELECT_H", "1");
        curl_lib.root_module.addCMacro("HAVE_SYS_SOCKET_H", "1");
        curl_lib.root_module.addCMacro("HAVE_SYS_STAT_H", "1");
        curl_lib.root_module.addCMacro("HAVE_SYS_TIME_H", "1");
        curl_lib.root_module.addCMacro("HAVE_SYS_TYPES_H", "1");
        curl_lib.root_module.addCMacro("HAVE_SYS_UIO_H", "1");
        curl_lib.root_module.addCMacro("HAVE_SYS_UN_H", "1");
        curl_lib.root_module.addCMacro("HAVE_TERMIOS_H", "1");
        curl_lib.root_module.addCMacro("HAVE_TERMIO_H", "1");
        curl_lib.root_module.addCMacro("HAVE_TIME_H", "1");
        curl_lib.root_module.addCMacro("HAVE_UNAME", "1");
        curl_lib.root_module.addCMacro("HAVE_UNISTD_H", "1");
        curl_lib.root_module.addCMacro("HAVE_UTIME", "1");
        curl_lib.root_module.addCMacro("HAVE_UTIMES", "1");
        curl_lib.root_module.addCMacro("HAVE_UTIME_H", "1");
        curl_lib.root_module.addCMacro("HAVE_VARIADIC_MACROS_C99", "1");
        curl_lib.root_module.addCMacro("HAVE_VARIADIC_MACROS_GCC", "1");
        curl_lib.root_module.addCMacro("OS", "\"Linux\"");
        curl_lib.root_module.addCMacro("RANDOM_FILE", "\"/dev/urandom\"");
        curl_lib.root_module.addCMacro("RECV_TYPE_ARG1", "int");
        curl_lib.root_module.addCMacro("RECV_TYPE_ARG2", "void *");
        curl_lib.root_module.addCMacro("RECV_TYPE_ARG3", "size_t");
        curl_lib.root_module.addCMacro("RECV_TYPE_ARG4", "int");
        curl_lib.root_module.addCMacro("RECV_TYPE_RETV", "ssize_t");
        curl_lib.root_module.addCMacro("SEND_QUAL_ARG2", "const");
        curl_lib.root_module.addCMacro("SEND_TYPE_ARG1", "int");
        curl_lib.root_module.addCMacro("SEND_TYPE_ARG2", "void *");
        curl_lib.root_module.addCMacro("SEND_TYPE_ARG3", "size_t");
        curl_lib.root_module.addCMacro("SEND_TYPE_ARG4", "int");
        curl_lib.root_module.addCMacro("SEND_TYPE_RETV", "ssize_t");
        curl_lib.root_module.addCMacro("SIZEOF_INT", "4");
        curl_lib.root_module.addCMacro("SIZEOF_SHORT", "2");
        curl_lib.root_module.addCMacro("SIZEOF_LONG", "8");
        curl_lib.root_module.addCMacro("SIZEOF_OFF_T", "8");
        curl_lib.root_module.addCMacro("SIZEOF_CURL_OFF_T", "8");
        curl_lib.root_module.addCMacro("SIZEOF_SIZE_T", "8");
        curl_lib.root_module.addCMacro("SIZEOF_TIME_T", "8");
        curl_lib.root_module.addCMacro("STDC_HEADERS", "1");
        curl_lib.root_module.addCMacro("TIME_WITH_SYS_TIME", "1");
        curl_lib.root_module.addCMacro("USE_THREADS_POSIX", "1");
        curl_lib.root_module.addCMacro("USE_UNIX_SOCKETS", "1");
        curl_lib.root_module.addCMacro("_FILE_OFFSET_BITS", "64");
    }

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
    try orca_tool_compile_flags.append("-fno-sanitize=undefined"); // stb_image appears to invoke undefined behavior :(
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
    if (target.result.os.tag == .windows) {
        orca_tool_exe.linkSystemLibrary("shlwapi");
        orca_tool_exe.linkSystemLibrary("shell32");
        orca_tool_exe.linkSystemLibrary("ole32");
        orca_tool_exe.linkSystemLibrary("kernel32");
    } else if (target.result.os.tag.isDarwin()) {
        orca_tool_exe.linkFramework("Cocoa");
        orca_tool_exe.linkFramework("SystemConfiguration");
        orca_tool_exe.linkFramework("CoreFoundation");
        orca_tool_exe.linkFramework("CoreServices");
        orca_tool_exe.linkFramework("SystemConfiguration");
        orca_tool_exe.linkFramework("Security");
    }

    orca_tool_exe.step.dependOn(&curl_lib.step);
    orca_tool_exe.step.dependOn(&z_lib.step);
    orca_tool_exe.linkLibC();

    const orca_tool_install: *Build.Step.InstallArtifact = b.addInstallArtifact(orca_tool_exe, .{});

    const build_tool_step = b.step("orca-tool", "Build the Orca CLI tool from source.");
    build_tool_step.dependOn(&orca_tool_install.step);

    ///////////////////////////////////////////////////////////////
    // zig build orca

    const build_orca = b.step("orca", "Build all orca binaries");
    build_orca.dependOn(build_orca_platform_step);
    build_orca.dependOn(build_runtime_step);
    build_orca.dependOn(build_libc_step);
    build_orca.dependOn(build_wasm_sdk_step);
    build_orca.dependOn(build_tool_step);

    ///////////////////////////////////////////////////////////////
    // package-sdk and install-sdk commands

    const package_sdk_exe: *Build.Step.Compile = b.addExecutable(.{
        .name = "package_sdk",
        .root_source_file = b.path("src/build/package_sdk.zig"),
        .target = target,
        .optimize = .Debug,
    });

    // zig build orca-install

    var orca_install = b.addRunArtifact(package_sdk_exe);
    orca_install.addArg("--dev-install");
    orca_install.addPrefixedFileArg("--artifacts-path=", b.path("build"));
    orca_install.addPrefixedFileArg("--resources-path=", b.path("resources"));
    orca_install.addPrefixedFileArg("--src-path=", b.path("src"));
    orca_install.step.dependOn(build_orca);

    const opt_sdk_version = b.option([]const u8, "sdk-version", "Override current git version for sdk packaging.");
    if (opt_sdk_version) |sdk_version| {
        const version = try std.mem.join(b.allocator, "", &.{ "--version=", sdk_version });
        orca_install.addArg(version);
    }

    const orca_install_step = b.step("orca-install", "Build and install orca in orca system path as a dev build");
    orca_install_step.dependOn(&orca_install.step);

    // zig build package-sdk

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

    // package command
    const package_sdk_step = b.step("package-sdk", "Packages the Orca SDK for a release.");
    package_sdk_step.dependOn(&package_sdk_to_dir.step);

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
        "build/bin",
        "build/orca-libc",
        "build/gles_gen.log",
        "build/sketches",
        "build/tests",
        "src/ext/angle",
        "src/ext/dawn",
        "scripts/files",
        "scripts/__pycache",

        // files
        "src/graphics/orca_surface_stubs.c",
        "src/platform/orca_io_stubs.c",
        "src/wasmbind/clock_api_bind_gen.c",
        "src/wasmbind/core_api_bind_gen.c",
        "src/wasmbind/core_api_stubs.c",
        "src/wasmbind/gles_api.json",
        "src/wasmbind/gles_api_bind_gen.c",
        "src/wasmbind/io_api_bind_gen.c",
        "src/wasmbind/surface_api_bind_gen.c",
    };
    for (clean_paths) |path| {
        var remove_dir = b.addRemoveDirTree(b.path(path));
        clean_step.dependOn(&remove_dir.step);
    }

    b.getUninstallStep().dependOn(clean_step);

    /////////////////////////////////////////////////////////////////
    // sketches

    var sketches = b.step("sketches", "Build all sketches into build/sketches");

    const sketches_folders: []const []const u8 = &.{
        //"atlas", // bitrotted
        "canvas",
        "canvas_test",
        "canvas_triangle_stress",
        "check-bleeding",
        "colorspace",
        "image",
        "keyboard",
        "minimalD3D12",
        "minimalDawnWebGPU",
        // "multi_surface", // bitrotted
        "perf_text",
        // "render_thread", // bitrotted
        "simpleWindow",
        "smiley",
        //"smooth_resize", // bitrotted
        // "test-clear", // wasm bundle test - probably should be in samples?
        "tiger",
        //"triangleGL", // openGL API no longer supported
        "triangleGLES",
        "triangleMetal",
        // "triangleWGPU", // bitrotted
        "ui",
    };

    const sketches_install_opts = Build.Step.InstallArtifact.Options{
        .dest_dir = .{ .override = .{ .custom = "sketches" } },
    };

    const orca_platform_sketches_install: *Build.Step.InstallArtifact = b.addInstallArtifact(orca_platform_lib, sketches_install_opts);
    sketches.dependOn(&orca_platform_sketches_install.step);

    var stage_sketch_dependency_artifacts = b.addUpdateSourceFiles();
    stage_sketch_dependency_artifacts.step.dependOn(&run_angle_uptodate.step);
    stage_sketch_dependency_artifacts.addCopyFileToSource(b.path("sketches/resources/CMUSerif-Roman.ttf"), "build/sketches/resources/CMUSerif-Roman.ttf");
    stage_sketch_dependency_artifacts.addCopyFileToSource(b.path("sketches/resources/Courier.ttf"), "build/sketches/resources/Courier.ttf");
    stage_sketch_dependency_artifacts.addCopyFileToSource(b.path("sketches/resources/gamma-1.0-or-2.2.png"), "build/sketches/resources/gamma-1.0-or-2.2.png");
    stage_sketch_dependency_artifacts.addCopyFileToSource(b.path("sketches/resources/gamma_dalai_lama_gray.png"), "build/sketches/resources/gamma_dalai_lama_gray.png");
    stage_sketch_dependency_artifacts.addCopyFileToSource(b.path("sketches/resources/gradient_srgb.png"), "build/sketches/resources/gradient_srgb.png");
    stage_sketch_dependency_artifacts.addCopyFileToSource(b.path("sketches/resources/OpenSansLatinSubset.ttf"), "build/sketches/resources/OpenSansLatinSubset.ttf");
    stage_sketch_dependency_artifacts.addCopyFileToSource(b.path("sketches/resources/OpenSans-Regular.ttf"), "build/sketches/resources/OpenSans-Regular.ttf");
    stage_sketch_dependency_artifacts.addCopyFileToSource(b.path("sketches/resources/OpenSans-Bold.ttf"), "build/sketches/resources/OpenSans-Bold.ttf");
    stage_sketch_dependency_artifacts.addCopyFileToSource(b.path("sketches/resources/Top512.png"), "build/sketches/resources/Top512.png");
    stage_sketch_dependency_artifacts.addCopyFileToSource(b.path("sketches/resources/triceratops.png"), "build/sketches/resources/triceratops.png");
    if (target.result.os.tag == .windows) {
        stage_sketch_dependency_artifacts.addCopyFileToSource(b.path("build/angle.out/bin/d3dcompiler_47.dll"), "build/sketches/d3dcompiler_47.dll");
        stage_sketch_dependency_artifacts.addCopyFileToSource(b.path("build/angle.out/bin/libEGL.dll"), "build/sketches/libEGL.dll");
        stage_sketch_dependency_artifacts.addCopyFileToSource(b.path("build/angle.out/bin/libEGL.dll.lib"), "build/sketches/libEGL.dll.lib");
        stage_sketch_dependency_artifacts.addCopyFileToSource(b.path("build/angle.out/bin/libGLESv2.dll"), "build/sketches/libGLESv2.dll");
        stage_sketch_dependency_artifacts.addCopyFileToSource(b.path("build/angle.out/bin/libGLESv2.dll.lib"), "build/sketches/libGLESv2.dll.lib");
        stage_sketch_dependency_artifacts.addCopyFileToSource(b.path("build/dawn.out/bin/webgpu.dll"), "build/sketches/webgpu.dll");
        stage_sketch_dependency_artifacts.addCopyFileToSource(b.path("build/dawn.out/bin/webgpu.lib"), "build/sketches/webgpu.lib");
    } else if (target.result.os.tag.isDarwin()) {
        stage_sketch_dependency_artifacts.addCopyFileToSource(b.path("build/angle.out/bin/libEGL.dylib"), "build/sketches/libEGL.dylib");
        stage_sketch_dependency_artifacts.addCopyFileToSource(b.path("build/angle.out/bin/libGLESv2.dylib"), "build/sketches/libGLESv2.dylib");
        stage_sketch_dependency_artifacts.addCopyFileToSource(b.path("build/dawn.out/bin/libwebgpu.dylib"), "build/sketches/libwebgpu.dylib");
    }
    stage_sketch_dependency_artifacts.step.dependOn(&run_angle_uptodate.step);
    stage_sketch_dependency_artifacts.step.dependOn(&run_dawn_uptodate.step);
    sketches.dependOn(&stage_sketch_dependency_artifacts.step);

    for (sketches_folders) |sketch| {
        const sketch_source: []const u8 = b.pathJoin(&.{ "sketches", sketch, "main.c" });
        if (pathExists(cwd, sketch_source) == false) {
            continue;
        }

        if (std.mem.eql(u8, "triangleMetal", sketch) and target.result.os.tag.isDarwin() == false) {
            continue;
        }

        const flags: []const []const u8 = &.{
            "-Isrc",
            "-Isrc/ext",
            "-Isrc/ext/angle/include",
            "-Isrc/ext/dawn/include",
            "-Isrc/util",
            "-Isrc/platform",
        };

        var sketch_exe: *Build.Step.Compile = b.addExecutable(.{
            .name = sketch,
            .target = target,
            .optimize = optimize,
        });
        sketch_exe.addCSourceFiles(.{
            .files = &.{sketch_source},
            .flags = flags,
        });
        sketch_exe.linkLibC();
        sketch_exe.linkLibrary(orca_platform_lib);

        const install: *Build.Step.InstallArtifact = b.addInstallArtifact(sketch_exe, sketches_install_opts);
        sketches.dependOn(&install.step);
    }

    /////////////////////////////////////////////////////////////////
    // tests

    var tests = b.step("test", "Build and run all tests");

    const TestConfig = struct {
        name: []const u8,
        testfile: []const u8 = "main.c",
        run: bool = false,
        wasm: bool = false,
    };

    // several tests require UI interactions so we won't run them all automatically, but configure
    // only some of them to be run
    const test_configs: []const TestConfig = &.{
        .{
            .name = "bulkmem",
            .wasm = true,
        },
        .{
            .name = "file_dialog",
        },
        .{
            .name = "file_open_request",
        },
        .{
            .name = "files",
            .run = true,
        },
        .{
            .name = "perf",
            .testfile = "driver.c",
        },
        .{
            .name = "wasm_tests",
            .wasm = true,
        },
    };

    const tests_install_opts = Build.Step.InstallArtifact.Options{
        .dest_dir = .{ .override = .{ .custom = "tests" } },
    };

    const orca_platform_tests_install: *Build.Step.InstallArtifact = b.addInstallArtifact(orca_platform_lib, tests_install_opts);
    tests.dependOn(&orca_platform_tests_install.step);

    var stage_test_dependency_artifacts = b.addUpdateSourceFiles();
    if (target.result.os.tag == .windows) {
        stage_test_dependency_artifacts.addCopyFileToSource(b.path("build/angle.out/bin/d3dcompiler_47.dll"), "build/tests/d3dcompiler_47.dll");
        stage_test_dependency_artifacts.addCopyFileToSource(b.path("build/angle.out/bin/libEGL.dll"), "build/tests/libEGL.dll");
        stage_test_dependency_artifacts.addCopyFileToSource(b.path("build/angle.out/bin/libEGL.dll.lib"), "build/tests/libEGL.dll.lib");
        stage_test_dependency_artifacts.addCopyFileToSource(b.path("build/angle.out/bin/libGLESv2.dll"), "build/tests/libGLESv2.dll");
        stage_test_dependency_artifacts.addCopyFileToSource(b.path("build/angle.out/bin/libGLESv2.dll.lib"), "build/tests/libGLESv2.dll.lib");
        stage_test_dependency_artifacts.addCopyFileToSource(b.path("build/dawn.out/bin/webgpu.dll"), "build/tests/webgpu.dll");
        stage_test_dependency_artifacts.addCopyFileToSource(b.path("build/dawn.out/bin/webgpu.lib"), "build/tests/webgpu.lib");
    } else {
        stage_test_dependency_artifacts.addCopyFileToSource(b.path("build/angle.out/bin/libEGL.dylib"), "build/tests/libEGL.dylib");
        stage_test_dependency_artifacts.addCopyFileToSource(b.path("build/angle.out/bin/libGLESv2.dylib"), "build/tests/libGLESv2.dylib");
        stage_test_dependency_artifacts.addCopyFileToSource(b.path("build/dawn.out/bin/libwebgpu.dylib"), "build/tests/libwebgpu.dylib");
    }
    stage_test_dependency_artifacts.step.dependOn(&run_angle_uptodate.step);
    stage_test_dependency_artifacts.step.dependOn(&run_dawn_uptodate.step);
    tests.dependOn(&stage_test_dependency_artifacts.step);

    for (test_configs) |config| {
        // TODO add support for building wasm samples
        if (config.wasm) {
            continue;
        }

        const test_source: []const u8 = b.pathJoin(&.{ "tests", config.name, config.testfile });

        var test_exe: *Build.Step.Compile = b.addExecutable(.{
            .name = config.name,
            .target = target,
            .optimize = optimize,
        });
        test_exe.addCSourceFiles(.{
            .files = &.{test_source},
            .flags = &.{b.fmt("-I{s}", .{b.pathFromRoot("src")})},
        });
        test_exe.linkLibC();
        test_exe.linkLibrary(orca_platform_lib);

        if (target.result.os.tag == .windows) {
            test_exe.linkSystemLibrary("shlwapi");
        }

        const install: *Build.Step.InstallArtifact = b.addInstallArtifact(test_exe, tests_install_opts);
        tests.dependOn(&install.step);

        if (config.run) {
            if (config.wasm) {
                // TODO add support for running wasm tests
                const fail = b.addFail("Running is currently not supported for wasm tests.");
                tests.dependOn(&fail.step);
            } else {
                const test_dir_path = b.path(b.pathJoin(&.{ "tests", config.name }));

                const run_test = b.addRunArtifact(test_exe);
                run_test.addPrefixedFileArg("--test-dir=", test_dir_path); // allows tests to access their data files
                run_test.step.dependOn(&stage_test_dependency_artifacts.step);
                run_test.step.dependOn(&install.step); // causes test exe working dir to be build\tests\ instead of zig-cache

                tests.dependOn(&run_test.step);
            }
        }
    }
}
