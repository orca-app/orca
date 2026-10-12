const std = @import("std");

const Build = std.Build;
const LazyPath = Build.LazyPath;
const Step = Build.Step;
const Module = Build.Module;
const ModuleImport = Module.Import;
const CrossTarget = std.zig.CrossTarget;
const ResolvedTarget = Build.ResolvedTarget;

const MACOS_VERSION_MIN = "14.4.1";

pub fn build(b: *Build) !void {
    var macosFrameworkPath: LazyPath = .{ .cwd_relative = "" };
    var macosIncludePath: LazyPath = .{ .cwd_relative = "" };
    var macosLibPath: LazyPath = .{ .cwd_relative = "" };

    const target: ResolvedTarget = tgt: {
        var query = b.standardTargetOptionsQueryOnly(.{});
        const tmp: ResolvedTarget = b.resolveTargetQuery(query);

        if (tmp.result.os.tag == .macos) {
            query.os_version_min = .{ .semver = try .parse(MACOS_VERSION_MIN) };

            const res = try std.process.Child.run(.{
                .allocator = b.allocator,
                .argv = &.{
                    "xcrun",
                    "--show-sdk-path",
                },
            });
            const sdkPath = res.stdout[0..(res.stdout.len - 1)];

            macosFrameworkPath = .{
                .cwd_relative = try std.fs.path.join(b.allocator, &[_][]const u8{ sdkPath, "System/Library/Frameworks" }),
            };
            macosIncludePath = .{
                .cwd_relative = try std.fs.path.join(b.allocator, &[_][]const u8{ sdkPath, "usr/include" }),
            };
            macosLibPath = .{
                .cwd_relative = try std.fs.path.join(b.allocator, &[_][]const u8{ sdkPath, "usr/lib" }),
            };
        }
        break :tgt b.resolveTargetQuery(query);
    };
    const optimize: std.builtin.OptimizeMode = b.standardOptimizeOption(.{});

    /////////////////////////////////////////////////////////
    // angle build dependencies
    var angle_include_path: LazyPath = .{ .cwd_relative = "" };
    var angle_lib_path: LazyPath = .{ .cwd_relative = "" };
    if (b.option([]const u8, "angle-path", "Specify a local build of the Angle library.")) |user_path| {
        const user_include_path = b.pathJoin(&.{ user_path, "include" });
        const user_lib_path = b.pathJoin(&.{ user_path, "lib" });
        angle_include_path = LazyPath{ .cwd_relative = user_include_path };
        angle_lib_path = LazyPath{ .cwd_relative = user_lib_path };
    } else if (target.result.os.tag == .windows) {
        if (b.lazyDependency("angle-windows-x64", .{})) |angle_dep| {
            angle_include_path = angle_dep.path("include");
            angle_lib_path = angle_dep.path("lib");
        }
    } else if (target.result.os.tag.isDarwin()) {
        if (target.result.cpu.arch == .aarch64) {
            if (b.lazyDependency("angle-mac-arm64", .{})) |angle_dep| {
                angle_include_path = angle_dep.path("include");
                angle_lib_path = angle_dep.path("lib");
            }
        } else {
            if (b.lazyDependency("angle-mac-x64", .{})) |angle_dep| {
                angle_include_path = angle_dep.path("include");
                angle_lib_path = angle_dep.path("lib");
            }
        }
    } else {
        const fail = b.addFail("Angle not configured for this platform.");
        b.getInstallStep().dependOn(&fail.step);
    }

    /////////////////////////////////////////////////////////
    // dawn build dependencies
    var dawn_include_path: LazyPath = .{ .cwd_relative = "" };
    var dawn_lib_path: LazyPath = .{ .cwd_relative = "" };
    if (b.option([]const u8, "dawn-path", "Specify a local build of the Dawn library.")) |user_path| {
        const user_include_path = b.pathJoin(&.{ user_path, "include" });
        const user_lib_path = b.pathJoin(&.{ user_path, "lib" });
        dawn_include_path = LazyPath{ .cwd_relative = user_include_path };
        dawn_lib_path = LazyPath{ .cwd_relative = user_lib_path };
    } else if (target.result.os.tag == .windows) {
        if (b.lazyDependency("dawn-windows-x64", .{})) |dawn_dep| {
            dawn_include_path = dawn_dep.path("include");
            dawn_lib_path = dawn_dep.path("lib");
        }
    } else if (target.result.os.tag.isDarwin()) {
        if (target.result.cpu.arch == .aarch64) {
            if (b.lazyDependency("dawn-mac-arm64", .{})) |dawn_dep| {
                dawn_include_path = dawn_dep.path("include");
                dawn_lib_path = dawn_dep.path("lib");
            }
        } else {
            if (b.lazyDependency("dawn-mac-x64", .{})) |dawn_dep| {
                dawn_include_path = dawn_dep.path("include");
                dawn_lib_path = dawn_dep.path("lib");
            }
        }
    } else {
        const fail = b.addFail("Dawn not configured for this platform.");
        b.getInstallStep().dependOn(&fail.step);
    }

    /////////////////////////////////////////////////////////
    // Orca runtime and dependencies
    const stage_angle_dawn_src = b.addUpdateSourceFiles();
    stage_angle_dawn_src.addCopyFileToSource(try angle_include_path.join(b.allocator, "EGL/egl.h"), "src/ext/angle/include/EGL/egl.h");
    stage_angle_dawn_src.addCopyFileToSource(try angle_include_path.join(b.allocator, "EGL/eglext.h"), "src/ext/angle/include/EGL/eglext.h");
    stage_angle_dawn_src.addCopyFileToSource(try angle_include_path.join(b.allocator, "EGL/eglext_angle.h"), "src/ext/angle/include/EGL/eglext_angle.h");
    stage_angle_dawn_src.addCopyFileToSource(try angle_include_path.join(b.allocator, "EGL/eglplatform.h"), "src/ext/angle/include/EGL/eglplatform.h");
    stage_angle_dawn_src.addCopyFileToSource(try angle_include_path.join(b.allocator, "GLES/egl.h"), "src/ext/angle/include/GLES/egl.h");
    stage_angle_dawn_src.addCopyFileToSource(try angle_include_path.join(b.allocator, "GLES/gl.h"), "src/ext/angle/include/GLES/gl.h");
    stage_angle_dawn_src.addCopyFileToSource(try angle_include_path.join(b.allocator, "GLES/glext.h"), "src/ext/angle/include/GLES/glext.h");
    stage_angle_dawn_src.addCopyFileToSource(try angle_include_path.join(b.allocator, "GLES/glplatform.h"), "src/ext/angle/include/GLES/glplatform.h");
    stage_angle_dawn_src.addCopyFileToSource(try angle_include_path.join(b.allocator, "GLES2/gl2.h"), "src/ext/angle/include/GLES2/gl2.h");
    stage_angle_dawn_src.addCopyFileToSource(try angle_include_path.join(b.allocator, "GLES2/gl2ext.h"), "src/ext/angle/include/GLES2/gl2ext.h");
    stage_angle_dawn_src.addCopyFileToSource(try angle_include_path.join(b.allocator, "GLES2/gl2ext_angle.h"), "src/ext/angle/include/GLES2/gl2ext_angle.h");
    stage_angle_dawn_src.addCopyFileToSource(try angle_include_path.join(b.allocator, "GLES2/gl2platform.h"), "src/ext/angle/include/GLES2/gl2platform.h");
    stage_angle_dawn_src.addCopyFileToSource(try angle_include_path.join(b.allocator, "GLES3/gl3.h"), "src/ext/angle/include/GLES3/gl3.h");
    stage_angle_dawn_src.addCopyFileToSource(try angle_include_path.join(b.allocator, "GLES3/gl31.h"), "src/ext/angle/include/GLES3/gl31.h");
    stage_angle_dawn_src.addCopyFileToSource(try angle_include_path.join(b.allocator, "GLES3/gl32.h"), "src/ext/angle/include/GLES3/gl32.h");
    stage_angle_dawn_src.addCopyFileToSource(try angle_include_path.join(b.allocator, "GLES3/gl3platform.h"), "src/ext/angle/include/GLES3/gl3platform.h");
    stage_angle_dawn_src.addCopyFileToSource(try angle_include_path.join(b.allocator, "KHR/khrplatform.h"), "src/ext/angle/include/KHR/khrplatform.h");
    stage_angle_dawn_src.addCopyFileToSource(try dawn_include_path.join(b.allocator, "webgpu.h"), "src/ext/dawn/include/webgpu.h");

    // copy angle + dawn libs to output directory
    const install_angle_libs = b.addInstallDirectory(.{ .source_dir = angle_lib_path, .install_dir = .lib, .install_subdir = "" });
    const install_dawn_libs = b.addInstallDirectory(.{ .source_dir = dawn_lib_path, .install_dir = .lib, .install_subdir = "" });

    // wgpu shaders header
    const gen_header_exe: *Build.Step.Compile = b.addExecutable(.{
        .name = "gen_header",
        .root_module = b.createModule(.{
            .root_source_file = b.path("src/build/gen_header_from_files.zig"),
            .target = b.graph.host,
            .optimize = .Debug,
        }),
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

    const run_gen_wgpu_header: *Build.Step.Run = b.addRunArtifact(gen_header_exe);
    for (wgpu_shaders) |path| {
        run_gen_wgpu_header.addPrefixedFileArg("--file=", b.path(path));
    }
    run_gen_wgpu_header.addArg("--namespace=oc_wgsl_");
    run_gen_wgpu_header.addPrefixedDirectoryArg("--root=", b.path(""));
    const wgpu_header_path = run_gen_wgpu_header.addPrefixedOutputFileArg("--output=", "generated_headers/wgpu_renderer_shaders.h");

    const update_wgpu_header: *Build.Step.UpdateSourceFiles = b.addUpdateSourceFiles();
    update_wgpu_header.addCopyFileToSource(wgpu_header_path, "src/graphics/wgpu_renderer_shaders.h");

    // platform lib

    var orca_platform_compile_flags: std.ArrayList([]const u8) = .empty;
    defer orca_platform_compile_flags.deinit(b.allocator);
    try orca_platform_compile_flags.appendSlice(b.allocator, &.{
        "-std=c23",
        "-Werror",
        "-DOC_BUILD_DLL",
        "-D_USE_MATH_DEFINES",
    });
    if (optimize == .Debug) {
        try orca_platform_compile_flags.appendSlice(b.allocator, &.{
            "-DOC_DEBUG",
            "-DOC_LOG_COMPILE_DEBUG",
        });
    }
    const orca_platform_static = b.addLibrary(.{
        .linkage = .static,
        .name = "orca_static",
        .win32_manifest = b.path("src/app/win32_manifest.manifest"),
        .root_module = b.createModule(.{
            .target = target,
            .optimize = optimize,
        }),
    });

    if (target.result.os.tag.isDarwin()) {
        orca_platform_static.addSystemFrameworkPath(macosFrameworkPath);
        orca_platform_static.addSystemIncludePath(macosIncludePath);
        orca_platform_static.addLibraryPath(macosLibPath);
    }

    orca_platform_static.install_name = "@executable_path/liborca_platform.dylib";

    orca_platform_static.addIncludePath(b.path("src"));
    orca_platform_static.addIncludePath(b.path("src/ext"));
    orca_platform_static.addIncludePath(b.path("src/ext/angle/include"));
    orca_platform_static.addIncludePath(b.path("src/ext/dawn/include"));
    orca_platform_static.addLibraryPath(angle_include_path);

    var orca_platform_files: std.ArrayList([]const u8) = .empty;
    try orca_platform_files.append(b.allocator, "src/orca.c");

    orca_platform_static.addLibraryPath(angle_lib_path);
    orca_platform_static.addLibraryPath(dawn_lib_path);

    if (target.result.os.tag == .windows) {
        orca_platform_static.linkSystemLibrary("user32");
        orca_platform_static.linkSystemLibrary("opengl32");
        orca_platform_static.linkSystemLibrary("gdi32");
        orca_platform_static.linkSystemLibrary("dxgi");
        orca_platform_static.linkSystemLibrary("dxguid");
        orca_platform_static.linkSystemLibrary("d3d11");
        orca_platform_static.linkSystemLibrary("dcomp");
        orca_platform_static.linkSystemLibrary("shcore");
        orca_platform_static.linkSystemLibrary("dwmapi");
        orca_platform_static.linkSystemLibrary("comctl32");
        orca_platform_static.linkSystemLibrary("ole32");
        orca_platform_static.linkSystemLibrary("shell32");
        orca_platform_static.linkSystemLibrary("shlwapi");

        // TODO delay load these the graphics libs
        // orca_platform_static.linkSystemLibrary("delayimp");
        orca_platform_static.linkSystemLibrary("libEGL.dll");
        orca_platform_static.linkSystemLibrary("libGLESv2.dll");
        orca_platform_static.linkSystemLibrary("webgpu");
    } else if (target.result.os.tag.isDarwin()) {
        orca_platform_static.linkFramework("Carbon");
        orca_platform_static.linkFramework("Cocoa");
        orca_platform_static.linkFramework("Metal");
        orca_platform_static.linkFramework("QuartzCore");
        orca_platform_static.linkFramework("UniformTypeIdentifiers");

        orca_platform_static.linkSystemLibrary2("EGL", .{ .weak = true });
        orca_platform_static.linkSystemLibrary2("GLESv2", .{ .weak = true });
        orca_platform_static.linkSystemLibrary2("webgpu", .{ .weak = true });

        try orca_platform_files.append(b.allocator, "src/orca.m");
    }

    orca_platform_static.addCSourceFiles(.{
        .files = orca_platform_files.items,
        .flags = orca_platform_compile_flags.items,
    });

    orca_platform_static.step.dependOn(&stage_angle_dawn_src.step);
    orca_platform_static.step.dependOn(&install_angle_libs.step);
    orca_platform_static.step.dependOn(&install_dawn_libs.step);
    orca_platform_static.step.dependOn(&update_wgpu_header.step);

    b.installArtifact(orca_platform_static);

    const orca_platform_dynamic = b.addLibrary(.{
        .linkage = .dynamic,
        .name = "orca_dynamic",
        .win32_manifest = b.path("src/app/win32_manifest.manifest"),
        .root_module = orca_platform_static.root_module,
    });
    b.installArtifact(orca_platform_dynamic);

    ///////////////////////////////////////////////////////////////
    // zig build - default install step builds and installs a dev build of orca

    b.getInstallStep().dependOn(&orca_platform_static.step);
    b.getInstallStep().dependOn(&orca_platform_dynamic.step);

    /////////////////////////////////////////////////////////////////
    //TODO tests
}
