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

fn basenameNoExtension(path: []const u8) []const u8 {
    const filename: []const u8 = std.fs.path.basename(path);
    if (std.mem.lastIndexOf(u8, filename, ".")) |index| {
        return filename[0..index];
    }
    return filename;
}

fn makeDir(path: []const u8) !void {
    const cwd = std.fs.cwd();
    cwd.makeDir(path) catch |e| {
        if (e != error.PathAlreadyExists) {
            return e;
        }
    };
}

// TODO move this into a Run build program
fn generateFileForStrings(b: *Build, comptime path: []const u8, prefix: []const u8, input_paths: []const []const u8) !*std.Build.Step.WriteFile {
    var concat = std.ArrayList(u8).init(b.allocator);
    defer concat.deinit();

    const filename: []const u8 = std.fs.path.basename(path);
    const filename_no_ext: []const u8 = basenameNoExtension(filename);
    const uppercase_filename: []u8 = try b.allocator.dupe(u8, filename_no_ext);
    for (uppercase_filename) |*c| {
        c.* = std.ascii.toUpper(c.*);
    }

    var writer = concat.writer();

    try writer.print("/*********************************************************************\n", .{});
    try writer.print("*\n", .{});
    try writer.print("*\tfile: {s}\n", .{filename});
    try writer.print("*\tnote: string literals auto-generated by build.zig\n", .{});
    // writer.print("*\tdate: {s}\n", .{datetime.now().strftime("%d/%m%Y")}) // TODO datetime
    try writer.print("*\n", .{});
    try writer.print("**********************************************************************/\n", .{});

    try writer.print("#ifndef __{s}_H__\n", .{uppercase_filename});
    try writer.print("#define __{s}_H__\n", .{uppercase_filename});
    try writer.print("\n\n", .{});

    const cwd = b.build_root;
    for (input_paths) |input_path| {
        const file_contents = try cwd.handle.readFileAlloc(b.allocator, input_path, 1024 * 1024 * 128);
        defer b.allocator.free(file_contents);

        const input_path_filename = std.fs.path.basename(input_path);
        try writer.print("//NOTE: string imported from {s}\n", .{input_path});
        try writer.print("const char* {s}{s} = ", .{ prefix, input_path_filename });
        try writer.print("{s}\n\n", .{file_contents});
    }

    try writer.print("#endif // __{s}_H__\n", .{uppercase_filename});

    return b.addWriteFile(path, concat.items);
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

const AngleDawnHelpers = struct {
    const Lib = enum {
        Angle,
        Dawn,

        fn str(lib: Lib) []const u8 {
            return switch (lib) {
                .Angle => "Angle",
                .Dawn => "Dawn",
            };
        }
    };

    // const FILE_EXCLUSIONS: []const []const u8 = &[_][]const u8{ ".DS_Store", "hash.txt" };

    // fn parseLibSha(b: *Build, comptime lib: Lib) []const u8 {}

    // expects file contents to be a single hash blob like "8a8c8fc280d74b34731e0e417b19bff7c967388a"
    // fn readStampFile(b: *Build, path: []const u8) ![]const u8 {
    //     const file_contents: []const u8 = try b.build_root.handle.readFileAlloc(b.allocator, path, 1024 * 4);

    //     var stamp: []const u8 = file_contents;
    //     stamp = std.mem.trimRight(u8, stamp, "\n");
    //     stamp = std.mem.trimRight(u8, stamp, "\r");
    //     return stamp;
    // }

    // fn generateCheckfileContents(b: *Build, comptime lib: Lib) ![]const u8 {
    //     // const dep = b.dependency(if (lib == .Angle) "build/angle" else "build/dawn", .{});

    //     // TODO read this from build.zig.zon somehow?
    //     const commit_stamp_path = if (lib == .Angle) "deps/angle-commit.txt" else "deps/dawn-commit.txt";
    //     const artifact_dir = if (lib == .Angle) "build/angle.out" else "build/dawn.out";

    //     const commit_stamp = try readStampFile(b, commit_stamp_path);
    //     const artifacts_hash = try Checksum.dir(b, dep.path(), &FILE_EXCLUSIONS);

    //     const checkfile_contents = try Checksum.strings(b, &.{ commit_stamp, artifacts_hash });
    //     return checkfile_contents;
    // }

    // fn checkUpToDate(b: *Build, comptime lib: Lib) !*Build.Step.CheckFile {
    //     // TODO rewrite this using a custom step
    //     const checkfile_path = if (lib == .Angle) "build/angle.out/hash.txt" else "build/dawn.out/hash.txt";
    //     const checkfile_contents = try generateCheckfileContents(b, lib);
    //     var checkfile = b.addCheckFile(b.path(checkfile_path), .{ .expected_exact = checkfile_contents });

    //     const name = if (lib == .Angle) "Angle up-to-date check" else "Dawn up-to-date check";
    //     checkfile.setName(name);

    //     return checkfile;
    // }

    //     const CheckBuildSentinel = struct {
    //         step: Build.Step,
    //         lib: AngleDawnHelpers.Lib,

    //         fn create(owner: *Build, comptime lib: AngleDawnHelpers.Lib) *CheckBuildSentinel {
    //             const name = if (lib == .Angle) "CheckAngleBuildSentinel" else "CheckDawnBuildSentinel";

    //             var check_sentinel = owner.allocator.create(CheckBuildSentinel) catch @panic("OOM");
    //             check_sentinel.step = Build.Step.init(.{
    //                 .id = .custom,
    //                 .name = name,
    //                 .owner = owner,
    //                 .makeFn = make,
    //             });
    //             check_sentinel.lib = lib;

    //             return check_sentinel;
    //         }

    //         fn make(step: *Step, prog_node: std.Progress.Node) !void {
    //             _ = prog_node;

    //             std.debug.print(">>>>>>> CheckBuildSentinel\n", .{});

    //             const check_sentinel: *CheckBuildSentinel = @fieldParentPtr("step", step);
    //             const b: *Build = step.owner;

    //             const build_dir = if (check_sentinel.lib == .Angle) "build/angle.out/" else "build/dawn.out";
    //             const sentinel_path = try std.fs.path.join(b.allocator, &.{ build_dir, "hash.txt" });
    //             const sum = try Checksum.dir(b, build_dir, FILE_EXCLUSIONS);

    //             const sentinel = b.build_root.handle.readFileAlloc(
    //                 b.allocator,
    //                 sentinel_path,
    //                 Checksum.MAX_FILE_SIZE,
    //             ) catch |err| {
    //                 return step.fail("unable to read build sentinel file '{}{s}': {s}", .{
    //                     b.build_root, sentinel_path, @errorName(err),
    //                 });
    //             };

    //             if (std.mem.eql(u8, sum, sentinel) == false) {
    //                 return step.fail("Calculated checksum ({s}) does not match sentinel at {s} ({s}). {s} is out of date and must be manually rebuilt.", .{
    //                     sum,
    //                     sentinel_path,
    //                     sentinel,
    //                     check_sentinel.lib.str(),
    //                 });
    //             }
    //         }
    //     };

    //     const WriteBuildSentinel = struct {
    //         step: Build.Step,
    //         lib: Lib,

    //         fn create(owner: *Build, comptime lib: Lib) *WriteBuildSentinel {
    //             const name = if (lib == .Angle) "WriteAngleBuildSentinel" else "WriteDawnBuildSentinel";

    //             var write_sentinel = owner.allocator.create(WriteBuildSentinel) catch @panic("OOM");
    //             write_sentinel.step = Build.Step.init(.{
    //                 .id = .custom,
    //                 .name = name,
    //                 .owner = owner,
    //                 .makeFn = make,
    //             });
    //             write_sentinel.lib = lib;

    //             return write_sentinel;
    //         }

    //         fn make(step: *Step, prog_node: std.Progress.Node) !void {
    //             _ = prog_node;

    //             std.debug.print(">>>>>>> WriteBuildSentinel\n", .{});

    //             const write_sentinel: *WriteBuildSentinel = @fieldParentPtr("step", step);
    //             const b: *Build = step.owner;

    //             const build_dir = if (write_sentinel.lib == .Angle) "build/angle.out/" else "build/dawn.out";
    //             const sentinel_path = try std.fs.path.join(b.allocator, &.{ build_dir, "hash.txt" });
    //             const sum = try Checksum.dir(b, build_dir, FILE_EXCLUSIONS);

    //             b.build_root.handle.writeFile(.{ .sub_path = sentinel_path, .data = sum }) catch |err| {
    //                 return step.fail("unable to write build sentinel file '{}{s}': {s}", .{
    //                     b.build_root, sentinel_path, @errorName(err),
    //                 });
    //             };
    //         }
    //     };
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
};

pub fn build(b: *Build) !void {
    const git_version_opt: ?[]const u8 = b.option([]const u8, "version", "Specify the specific git version you want to package") orelse null;

    const target = b.standardTargetOptions(.{});
    const optimize = b.standardOptimizeOption(.{});

    // NOTE - can use this to remove the default install/uninstall steps to fully customize the build menu
    // b.top_level_steps.clearRetainingCapacity();

    // Modify the default top-level build steps to reflect Orca's dev workflow
    {
        // b.top_level_steps.get("install").?.description = "Install a dev build of the Orca tools into the system Orca directory.";

        // var uninstall_step = b.top_level_steps.fetchSwapRemove("install").?.value;
        // uninstall_step.step.name = "all";
        // uninstall_step.description = "Delete all build artifacts and start fresh.";
        // try b.top_level_steps.put(b.allocator, uninstall_step.step.name, uninstall_step);

        var uninstall_step = b.top_level_steps.fetchSwapRemove("uninstall").?.value;
        uninstall_step.step.name = "clean";
        uninstall_step.description = "Delete all build artifacts and start fresh.";
        try b.top_level_steps.put(b.allocator, uninstall_step.step.name, uninstall_step);
    }

    // b.install_prefix = "build/zig-out";

    // const python_build_libc = b.addSystemCommand(&[_][]const u8{ "python", "orcadev", "build-orca-libc" });
    // const python_build_sdk = b.addSystemCommand(&[_][]const u8{ "python", "orcadev", "build-wasm-sdk" });

    /////////////////////////////////////////////////////////
    // depot tools

    // var depot_tools_dep = b.dependency("depot_tools", .{});

    // Dependency.path() returns a LazyPath, but the angle/dawn build tools require the
    // depot_tools to be in the PATH, which requires a path known when creating the build
    // graph. So we ensure the depot_tools will be staged in the build directory so we can
    // have a known good path to give to the other tools.
    // var stage_depot_tools: *Build.Step.WriteFile = b.addWriteFiles();
    // const depot_tools_dir = stage_depot_tools.addCopyDirectory(depot_tools_dep.path(""), "build/depot_tools", .{});

    const cwd = b.build_root.handle;

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

    var run_angle_uptodate: *Build.Step.Run = b.addRunArtifact(build_deps_exe);
    run_angle_uptodate.addArg("--check");
    run_angle_uptodate.addArg("--lib=angle");
    run_angle_uptodate.addArg(b.fmt("--sha={s}", .{shas.angle}));
    run_angle_uptodate.addArg(b.fmt("--intermediate={s}", .{deps_intermediate_path}));
    run_angle_uptodate.addPrefixedFileArg("--src=", angle_dep.path(""));
    RunHelpers.addPythonArg(run_angle_uptodate, target, b);

    const build_angle_step = b.step("angle", "Build Angle libs");
    build_angle_step.dependOn(&run_angle_build.step);

    /////////////////////////////////////////////////////////
    // dawn

    var dawn_dep: *Build.Dependency = b.dependency("dawn", .{});

    var run_dawn_build: *Build.Step.Run = b.addRunArtifact(build_deps_exe);
    run_dawn_build.addArg("--lib=dawn");
    run_dawn_build.addArg(b.fmt("--sha={s}", .{shas.dawn}));
    run_dawn_build.addArg(b.fmt("--intermediate={s}", .{deps_intermediate_path}));
    run_dawn_build.addPrefixedFileArg("--src=", dawn_dep.path(""));
    RunHelpers.addPythonArg(run_dawn_build, target, b);

    var run_dawn_uptodate: *Build.Step.Run = b.addRunArtifact(build_deps_exe);
    run_dawn_uptodate.addArg("--check");
    run_dawn_uptodate.addArg("--lib=dawn");
    run_dawn_uptodate.addArg(b.fmt("--sha={s}", .{shas.dawn}));
    run_dawn_uptodate.addArg(b.fmt("--intermediate={s}", .{deps_intermediate_path}));
    run_dawn_uptodate.addPrefixedFileArg("--src=", dawn_dep.path(""));
    RunHelpers.addPythonArg(run_dawn_uptodate, target, b);

    const build_dawn_step = b.step("dawn", "Build Dawn libs");
    build_dawn_step.dependOn(&run_dawn_build.step);

    /////////////////////////////////////////////////////////
    // Orca runtime and dependencies

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

    // platform lib

    // var angle_uptodate_check = AngleDawnHelpers.CheckBuildSentinel.create(b, .Angle);
    // var dawn_uptodate_check = AngleDawnHelpers.CheckBuildSentinel.create(b, .Dawn);

    // var copy_angle_files: *Build.Step.WriteFile = b.addWriteFiles();
    // _ = copy_angle_files.addCopyDirectory(b.path("build/angle.out/include/"), "src/ext/angle/include", .{});
    // _ = copy_angle_files.addCopyDirectory(b.path("build/angle.out/bin/"), "build/bin", .{});
    // copy_angle_files.step.dependOn(&angle_uptodate_check.step);

    // var copy_dawn_files: *Build.Step.WriteFile = b.addWriteFiles();
    // _ = copy_dawn_files.addCopyDirectory(b.path("build/dawn.out/include/"), "src/ext/dawn/include", .{});
    // _ = copy_dawn_files.addCopyDirectory(b.path("build/dawn.out/bin/"), "build/bin", .{});
    // copy_dawn_files.step.dependOn(&dawn_uptodate_check.step);

    // var install_dawn_artifacts: *Build.Step.installDirectory(.{
    //     .source_dir = b.paht
    //     })

    // os.makedirs("src/ext/dawn/include", exist_ok=True)
    // shutil.copytree("build/dawn.out/include", "src/ext/dawn/include/", dirs_exist_ok=True)

    // os.makedirs("build/bin", exist_ok=True)
    // shutil.copytree("build/dawn.out/bin", "build/bin", dirs_exist_ok=True)

    const wgpu_shaders_file = try generateFileForStrings(b, "src/graphics/wgpu_renderer_shaders.h", "oc_wgsl_", &.{
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
    });

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

    orca_platform_lib.addIncludePath(b.path("src"));
    orca_platform_lib.addIncludePath(b.path("src/ext"));
    orca_platform_lib.addIncludePath(b.path("src/ext/angle/include"));

    orca_platform_lib.addCSourceFiles(.{
        .files = &.{"src/orca.c"},
        .flags = orca_platform_compile_flags.items,
    });

    orca_platform_lib.addLibraryPath(b.path("build/lib"));
    orca_platform_lib.addLibraryPath(b.path("build/bin"));

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

    orca_platform_lib.step.dependOn(&run_angle_uptodate.step);
    // orca_platform_lib.step.dependOn(&copy_dawn_files.step);
    orca_platform_lib.step.dependOn(&wgpu_shaders_file.step); // TODO move this into a run command

    b.installArtifact(orca_platform_lib);

    // orca runtime exe

    // TODO generate all wasm bindings

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
    orca_runtime_exe.addIncludePath(b.path("src/ext/angle/include"));
    orca_runtime_exe.addIncludePath(b.path("src/ext/wasm3/source"));

    orca_runtime_exe.addCSourceFiles(.{
        .files = &.{"src/runtime.c"},
        .flags = orca_runtime_compile_flags.items,
    });

    orca_runtime_exe.linkLibrary(wasm3_lib);
    orca_runtime_exe.linkLibrary(orca_platform_lib);
    orca_runtime_exe.linkLibC();

    b.installArtifact(orca_runtime_exe);

    // TODO write checksum file
    // with open("build/orcaruntime.sum", "w") as f:
    //     f.write(runtime_checksum())

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

    const version: []const u8 = blk: {
        if (git_version_opt) |git_version| {
            break :blk try b.allocator.dupe(u8, git_version);
        } else {
            const git_version = b.run(&.{ "git", "rev-parse", "--short", "HEAD" });
            break :blk std.mem.trimRight(u8, git_version, "\n");
        }
    };
    defer b.allocator.free(version);

    var orca_tool_compile_flags = std.ArrayList([]const u8).init(b.allocator);
    defer orca_tool_compile_flags.deinit();
    try orca_tool_compile_flags.append("-DFLAG_IMPLEMENTATION");
    try orca_tool_compile_flags.append("-DOC_NO_APP_LAYER");
    try orca_tool_compile_flags.append("-DOC_BUILD_DLL");
    try orca_tool_compile_flags.append("-DCURL_STATICLIB");
    try orca_tool_compile_flags.append(b.fmt("-DORCA_TOOL_VERSION={s}", .{version}));

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

    b.installArtifact(orca_tool_exe);

    /////////////////////////////////////////////////////////////////
    // TODO bundle

    // python_build_libc.step.dependOn(&orca_runtime_exe.step);
    // python_build_sdk.step.dependOn(&python_build_libc.step);
    // orca_tool_exe.step.dependOn(&python_build_sdk.step);
    // python_build_tool.step.dependOn(&python_build_sdk.step);

    // b.getInstallStep().dependOn(&orca_tool_exe.step);

    // ensure_programs()

    // build_runtime_internal(args.release, args.wasm_backend) # this also builds the platform layer
    // build_libc_internal(args.release)
    // build_sdk_internal(args.release)
    // build_tool(args)

    // const python_install =

    // b.getInstallStep().dependOn(&python_install.step);

    /////////////////////////////////////////////////////////////////
    // zig build clean

    // TODO consider making a standalone step different from the default uninstall
    {
        var uninstall_step = b.getUninstallStep();
        const paths = [_][]const u8{
            ".zig-cache",
            "build",
            "src/ext/angle",
            "src/ext/dawn",
            "scripts/files",
            "scripts/__pycache",
            // TODO generated wasm bindings?
        };
        for (paths) |path| {
            var remove_dir = b.addRemoveDirTree(path);
            uninstall_step.dependOn(&remove_dir.step);
        }
    }

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
    // run_cmd.step.dependOn(b.getInstallStep());

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
