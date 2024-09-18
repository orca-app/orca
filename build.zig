const std = @import("std");

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
        const cwd = sources.b.build_root;
        const dir = try cwd.handle.openDir(path, .{ .iterate = true });
        var iter = dir.iterate();
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

var stringpool_data: std.ArrayList([]const u8) = undefined;
fn printf(comptime fmt: []const u8, args: anytype) ![]const u8 {
    const str = try std.fmt.allocPrint(stringpool_data.allocator, fmt, args);
    try stringpool_data.append(str);
    return str;
}

pub fn build(b: *Build) !void {
    stringpool_data = std.ArrayList([]const u8).init(b.allocator);
    defer {
        for (stringpool_data.items) |s| {
            b.allocator.free(s);
        }
        stringpool_data.deinit();
    }

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

    const python_build_runtime = b.addSystemCommand(&[_][]const u8{ "python", "orcadev", "build-runtime" });
    const python_build_libc = b.addSystemCommand(&[_][]const u8{ "python", "orcadev", "build-orca-libc" });
    const python_build_sdk = b.addSystemCommand(&[_][]const u8{ "python", "orcadev", "build-wasm-sdk" });

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

    /////////////////////////////////////////////////////////////////
    // orca cli tool

    // curl_lib.addWin32ResourceFile();

    // pub const AddCSourceFilesOptions = struct {
    //     /// When provided, `files` are relative to `root` rather than the
    //     /// package that owns the `Compile` step.
    //     root: ?LazyPath = null,
    //     files: []const []const u8,
    //     flags: []const []const u8 = &.{},
    // };

    // with pushd("src/ext/curl/winbuild"):
    //     subprocess.run("nmake /f Makefile.vc mode=static MACHINE=x64", check=True)
    // shutil.copytree(
    //     "src/ext/curl/builds/libcurl-vc-x64-release-static-ipv6-sspi-schannel/",
    //     "src/ext/curl/builds/static",
    //     dirs_exist_ok=True)

    // res = subprocess.run(["git", "rev-parse", "--short", "HEAD"], check=True, capture_output=True, text=True)

    const version: []const u8 = blk: {
        if (git_version_opt) |git_version| {
            break :blk try b.allocator.dupe(u8, git_version);
        } else {
            const git_version = b.run(&.{ "git", "rev-parse", "--short", "HEAD" });
            try stringpool_data.append(git_version);
            break :blk std.mem.trimRight(u8, git_version, "\n");
        }
    };
    defer b.allocator.free(version);

    // var orca_tool_sources = CSources.init(b);
    // try orca_tool_sources.collect("src/tool");

    var orca_tool_compile_flags = std.ArrayList([]const u8).init(b.allocator);
    defer orca_tool_compile_flags.deinit();
    try orca_tool_compile_flags.append("-DFLAG_IMPLEMENTATION");
    try orca_tool_compile_flags.append("-DOC_NO_APP_LAYER");
    try orca_tool_compile_flags.append("-DOC_BUILD_DLL");
    try orca_tool_compile_flags.append("-DCURL_STATICLIB");
    try orca_tool_compile_flags.append(try printf("-DORCA_TOOL_VERSION={s}", .{version}));

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

    // # libs needed by curl
    // "advapi32.lib",
    // "crypt32.lib",
    // "normaliz.lib",
    // "ws2_32.lib",
    // "wldap32.lib",
    // "/LIBPATH:src/ext/curl/builds/static/lib",
    // "libcurl_a.lib",

    // "/LIBPATH:src/ext/zlib/build",
    // "zlib.lib",

    // curl_lib.linkSystemLibrary("gdi32", .{ .needed = true, .preferred_link_mode = .static });
    // curl_lib.linkSystemLibrary("user32", .{ .needed = true, .preferred_link_mode = .static });

    orca_tool_exe.step.dependOn(&curl_lib.step);
    orca_tool_exe.step.dependOn(&z_lib.step);
    orca_tool_exe.linkLibC();

    const orca_tool_step = b.step("tool", "Build the Orca CLI tool from source.");
    orca_tool_step.dependOn(&orca_tool_exe.step);

    b.installArtifact(orca_tool_exe);

    // const python_build_tool = b.addSystemCommand(&[_][]const u8{ "python", "orcadev", "build-tool" });

    // const python_build_all =  //b.addSystemCommand(&[_][]const u8{ "python", "orcadev", "build" });

    python_build_libc.step.dependOn(&python_build_runtime.step);
    python_build_sdk.step.dependOn(&python_build_libc.step);
    // orca_tool_exe.step.dependOn(&python_build_sdk.step);
    // python_build_tool.step.dependOn(&python_build_sdk.step);

    b.getInstallStep().dependOn(&orca_tool_exe.step);

    // ensure_programs()

    // build_runtime_internal(args.release, args.wasm_backend) # this also builds the platform layer
    // build_libc_internal(args.release)
    // build_sdk_internal(args.release)
    // build_tool(args)

    // with open("build/orcaruntime.sum", "w") as f:
    //     f.write(runtime_checksum())

    // const python_install =

    // b.getInstallStep().dependOn(&python_install.step);

    // zig build clean
    {
        var uninstall_step = b.getUninstallStep();
        const paths = [_][]const u8{ "build", "scripts/files", "scripts/__pycache", "zig-cache" };
        for (paths) |path| {
            var remove_dir = b.addRemoveDirTree(path);
            uninstall_step.dependOn(&remove_dir.step);
        }
        // var cwd = std.fs.cwd();
        // for (paths) |path| {
        //     try cwd.deleteTree(path);
        // }
    }

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
