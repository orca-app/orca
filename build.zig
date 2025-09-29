const std = @import("std");

const Build = std.Build;
const LazyPath = Build.LazyPath;
const Step = Build.Step;
const Module = Build.Module;
const ModuleImport = Module.Import;
const CrossTarget = std.zig.CrossTarget;
const ResolvedTarget = Build.ResolvedTarget;

const MACOS_VERSION_MIN = "13.0.0";

const SourceFileCollector = struct {
    files: std.ArrayListUnmanaged([]const u8),
    extension: []const u8,
    b: *Build,

    fn init(b: *Build, extension: []const u8) SourceFileCollector {
        return .{
            .files = .empty,
            .extension = extension,
            .b = b,
        };
    }

    fn collect(sources: *SourceFileCollector, path: []const u8) !void {
        const path_from_root = sources.b.pathFromRoot(path); // ensures if the user is in a subdir the path is correct
        const cwd: std.fs.Dir = sources.b.build_root.handle;
        const dir: std.fs.Dir = try cwd.openDir(path_from_root, .{ .iterate = true });
        var iter: std.fs.Dir.Iterator = dir.iterate();
        while (try iter.next()) |entry| {
            if (entry.kind == .file and std.mem.endsWith(u8, entry.name, sources.extension)) {
                const filepath = try std.fs.path.resolve(sources.b.allocator, &.{ path, entry.name });
                try sources.files.append(sources.b.allocator, filepath);
            }
        }
    }

    fn deinit(sources: *SourceFileCollector) void {
        for (sources.files.items) |path| {
            sources.b.allocator.free(path);
        }
        sources.files.deinit(sources.b.allocator);
    }
};

fn pathExists(dir: std.fs.Dir, path: []const u8) bool {
    dir.access(path, .{}) catch {
        return false;
    };
    return true;
}

var HOME_PATH: []const u8 = "";
fn homePath(target: ResolvedTarget, b: *Build) []const u8 {
    if (HOME_PATH.len == 0) {
        var envmap: std.process.EnvMap = std.process.getEnvMap(b.allocator) catch @panic("OOM");
        defer envmap.deinit();

        if (target.result.os.tag == .windows) {
            if (envmap.get("HOMEDRIVE")) |home_drive| {
                if (envmap.get("HOMEPATH")) |home_path| {
                    HOME_PATH = b.pathJoin(&.{ home_drive, home_path });
                }
            }
        } else if (target.result.os.tag.isDarwin()) {
            if (envmap.get("HOME")) |path| {
                HOME_PATH = b.allocator.dupe(u8, path) catch @panic("OOM");
            } else {
                @panic("Unable to find home directory - missing environment variable 'HOME'. Either set this envar or avoid using ~ in the path.");
            }
        } else {
            @panic("Unimplemented"); // unimplemented for the target OS
        }
    }
    return HOME_PATH;
}

const GenerateWasmBindingsParams = struct {
    exe: *Build.Step.Compile,
    api: []const u8,
    spec_path: []const u8,
    host_bindings_path: []const u8,
    guest_bindings_path: ?[]const u8 = null,
    guest_include_path: ?[]const u8 = null,
};

fn generateWasmBindings(b: *Build, params: GenerateWasmBindingsParams) *Build.Step.UpdateSourceFiles {
    const copy_outputs_to_src: *Build.Step.UpdateSourceFiles = b.addUpdateSourceFiles();

    const run = b.addRunArtifact(params.exe);
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

const OrcaAppBuildParams = struct {
    name: []const u8,
    sources: []const []const u8,
    install: []const u8, // Orca app will be bundled to zig-out/{install}/{name}

    icon_path: ?[]const u8 = null,
    resource_path: ?[]const u8 = null,
    shaders: ?[]const []const u8 = null,
    create_run_step: bool = false, // if false or the target is not the host platform, no run step will be generated
    is_test: bool = false,
};

const OrcaAppBuildSteps = struct {
    build_or_bundle: *Step,
    run: ?*Step.Run,
};

fn buildOrcaApp(
    b: *Build,
    target: ResolvedTarget,
    wasm_sdk_lib: *Step.Compile,
    wasm_libc_lib: *Step.Compile,
    gen_header_exe: *Step.Compile,
    orca_install: *Step,
    orca_tool_path: []const u8,
    params: OrcaAppBuildParams,
) OrcaAppBuildSteps {
    var wasm_target_query: std.Target.Query = .{
        .cpu_arch = std.Target.Cpu.Arch.wasm32,
        .os_tag = std.Target.Os.Tag.freestanding,
    };
    wasm_target_query.cpu_features_add.addFeature(@intFromEnum(std.Target.wasm.Feature.bulk_memory));
    wasm_target_query.cpu_features_add.addFeature(@intFromEnum(std.Target.wasm.Feature.nontrapping_fptoint));

    const wasm_target: ResolvedTarget = b.resolveTargetQuery(wasm_target_query);
    const wasm_optimize: std.builtin.OptimizeMode = .ReleaseSmall;

    const wasm_module = b.addExecutable(.{
        .name = "module",
        .linkage = .static,
        .root_module = b.createModule(.{
            .target = wasm_target,
            .optimize = wasm_optimize,
            .single_threaded = true,
            .link_libc = false,
            .strip = false, // samples are meant to demo features like the debugger
        }),
    });
    wasm_module.entry = .disabled;
    wasm_module.rdynamic = true;
    wasm_module.addIncludePath(b.path("src"));
    wasm_module.addIncludePath(b.path("src/ext"));
    wasm_module.addIncludePath(b.path("src/orca-libc/include"));
    wasm_module.linkLibrary(wasm_sdk_lib);
    wasm_module.linkLibrary(wasm_libc_lib);
    wasm_module.addCSourceFiles(.{
        .files = params.sources,
        .flags = &.{},
    });

    if (params.shaders) |shaders| {
        const run_gen_glsl_header: *Build.Step.Run = b.addRunArtifact(gen_header_exe);
        for (shaders) |shader_path| {
            run_gen_glsl_header.addPrefixedFileArg("--file=", b.path(shader_path));
        }
        run_gen_glsl_header.addArg("--namespace=glsl_");
        run_gen_glsl_header.addPrefixedDirectoryArg("--root=", b.path(""));
        const glsl_header_path = run_gen_glsl_header.addPrefixedOutputFileArg(
            "--output=",
            b.fmt("apps/{s}/generated_headers/glsl_shaders.h", .{params.name}),
        );

        wasm_module.step.dependOn(&run_gen_glsl_header.step);
        wasm_module.addIncludePath(glsl_header_path.dirname());
    }

    // Bundling and running unavailable when cross-compiling
    // TODO - we could make building available when cross-compiling, would just need to build
    //        a host-native version of the orca toolchain
    if (params.create_run_step and target.result.os.tag == b.graph.host.result.os.tag) {
        const bundle: *Step.Run = b.addSystemCommand(&.{orca_tool_path});
        bundle.addArg("bundle");
        bundle.addArgs(&.{ "--name", params.name });
        if (params.icon_path) |icon_path| {
            bundle.addArg("--icon");
            bundle.addFileArg(b.path(icon_path));
        }
        if (params.resource_path) |resource_path| {
            bundle.addArg("--resource-dir");
            bundle.addDirectoryArg(b.path(resource_path));
        }

        const output_path = b.getInstallPath(.{ .custom = params.install }, "");
        bundle.addArgs(&.{ "--out-dir", output_path });
        bundle.addFileArg(wasm_module.getEmittedBin());

        // NOTE This dependency is necessary because only the package-sdk command can put all the
        //      needed files in place to allow the orca CLI to run properly.
        bundle.step.dependOn(orca_install);

        const run_app = Step.Run.create(b, b.fmt("run {s}", .{params.name}));
        run_app.step.dependOn(&bundle.step);

        if (target.result.os.tag == .windows) {
            const exe_path = b.pathJoin(&.{ output_path, params.name, "bin", b.fmt("{s}.exe", .{params.name}) });
            run_app.addArg(exe_path);
        } else if (target.result.os.tag.isDarwin()) {
            const app_path = b.pathJoin(&.{
                output_path,
                b.fmt("{s}.app", .{params.name}),
                "Contents",
                "MacOS",
                "orca_runtime",
            });
            const args: []const []const u8 = if (params.is_test) &.{ app_path, "--test" } else &.{app_path};
            run_app.addArgs(args);
        } else {
            @panic("Unsupported OS");
        }

        return OrcaAppBuildSteps{
            .build_or_bundle = &bundle.step,
            .run = run_app,
        };
    } else {
        return OrcaAppBuildSteps{
            .build_or_bundle = &wasm_module.step,
            .run = null,
        };
    }
}

fn installOrcaSdk(
    b: *Build,
    target: ResolvedTarget,
    build_orca: *Step,
    package_sdk_exe: *Step.Compile,
    sdk_install_path_opt: ?[]const u8,
    git_version_opt: ?[]const u8,
    opt_sdk_version: ?[]const u8,
) !*Step.Run {
    const SdkHelpers = struct {
        fn addAbsolutePathArg(b_: *Build, target_: ResolvedTarget, run: *Build.Step.Run, prefix: []const u8, path: []const u8) void {
            if (path.len == 0) {
                return;
            }

            var path_absolute: []const u8 = path;

            if (std.fs.path.isAbsolute(path_absolute) == false) {
                if (path_absolute[0] == '~') {
                    const home: []const u8 = homePath(target_, b_);
                    path_absolute = std.fs.path.join(b_.allocator, &.{ home, path_absolute[1..] }) catch @panic("OOM");
                } else {
                    path_absolute = b_.pathFromRoot(path);
                }
            }

            const sdk_path = std.mem.join(b_.allocator, "", &.{ prefix, path_absolute }) catch @panic("OOM");
            run.addArg(sdk_path);
        }
    };

    const orca_install: *Build.Step.Run = b.addRunArtifact(package_sdk_exe);
    orca_install.addPrefixedDirectoryArg("--artifacts-path=", LazyPath{ .cwd_relative = b.install_path });
    orca_install.addPrefixedDirectoryArg("--resources-path=", b.path("resources"));
    orca_install.addPrefixedDirectoryArg("--src-path=", b.path("src"));
    orca_install.addArg(b.fmt("--target-os={s}", .{@tagName(target.result.os.tag)}));

    if (sdk_install_path_opt) |sdk_install_path| {
        SdkHelpers.addAbsolutePathArg(b, target, orca_install, "--sdk-path=", sdk_install_path);
    }

    if (git_version_opt) |git_version| {
        orca_install.addArg(b.fmt("--version={s}", .{git_version}));
    }

    if (opt_sdk_version) |sdk_version| {
        const version: []const u8 = try std.mem.join(b.allocator, "", &.{ "--version=", sdk_version });
        orca_install.addArg(version);
    }

    orca_install.step.dependOn(build_orca);

    return orca_install;
}

pub fn build_libzip(b: *Build, zlib: *Build.Step.Compile, target: Build.ResolvedTarget, optimize: std.builtin.OptimizeMode) *Build.Step.Compile {
    const lib_mod = b.createModule(.{
        .target = target,
        .optimize = optimize,
    });
    lib_mod.link_libc = true;
    lib_mod.linkLibrary(zlib);

    lib_mod.addCSourceFiles(.{
        .root = b.path("src/ext/libzip/lib"),
        .files = &.{
            "zip_add.c",
            "zip_add_dir.c",
            "zip_add_entry.c",
            //"zip_algorithm_bzip2.c",
            "zip_algorithm_deflate.c",
            // "zip_algorithm_xz.c", // TODO
            //"zip_algorithm_zstd.c",
            "zip_buffer.c",
            "zip_close.c",
            // "zip_crypto_commoncrypto.c", // TODO
            // "zip_crypto_gnutls.c", // TODO
            // "zip_crypto_mbedtls.c", // TODO
            // "zip_crypto_openssl.c", // TODO
            // "zip_crypto_win.c", // TODO
            "zip_delete.c",
            "zip_dir_add.c",
            "zip_dirent.c",
            "zip_discard.c",
            "zip_entry.c",
            "zip_error.c",
            "zip_error_clear.c",
            "zip_error_get.c",
            "zip_error_get_sys_type.c",
            "zip_error_strerror.c",
            "zip_error_to_str.c",
            "zip_extra_field.c",
            "zip_extra_field_api.c",
            "zip_fclose.c",
            "zip_fdopen.c",
            "zip_file_add.c",
            "zip_file_error_clear.c",
            "zip_file_error_get.c",
            "zip_file_get_comment.c",
            "zip_file_get_external_attributes.c",
            "zip_file_get_offset.c",
            "zip_file_rename.c",
            "zip_file_replace.c",
            "zip_file_set_comment.c",
            "zip_file_set_encryption.c",
            "zip_file_set_external_attributes.c",
            "zip_file_set_mtime.c",
            "zip_file_strerror.c",
            "zip_fopen.c",
            "zip_fopen_encrypted.c",
            "zip_fopen_index.c",
            "zip_fopen_index_encrypted.c",
            "zip_fread.c",
            "zip_fseek.c",
            "zip_ftell.c",
            "zip_get_archive_comment.c",
            "zip_get_archive_flag.c",
            "zip_get_encryption_implementation.c",
            "zip_get_file_comment.c",
            "zip_get_name.c",
            "zip_get_num_entries.c",
            "zip_get_num_files.c",
            "zip_hash.c",
            "zip_io_util.c",
            "zip_libzip_version.c",
            "zip_memdup.c",
            "zip_name_locate.c",
            "zip_new.c",
            "zip_open.c",
            "zip_pkware.c",
            "zip_progress.c",
            "zip_realloc.c",
            "zip_rename.c",
            "zip_replace.c",
            "zip_set_archive_comment.c",
            "zip_set_archive_flag.c",
            "zip_set_default_password.c",
            "zip_set_file_comment.c",
            "zip_set_file_compression.c",
            "zip_set_name.c",
            "zip_source_accept_empty.c",
            "zip_source_begin_write.c",
            "zip_source_begin_write_cloning.c",
            "zip_source_buffer.c",
            "zip_source_call.c",
            "zip_source_close.c",
            "zip_source_commit_write.c",
            "zip_source_compress.c",
            "zip_source_crc.c",
            "zip_source_error.c",
            "zip_source_file_common.c",
            "zip_source_file_stdio.c",
            "zip_source_free.c",
            "zip_source_function.c",
            "zip_source_get_dostime.c",
            "zip_source_get_file_attributes.c",
            "zip_source_is_deleted.c",
            "zip_source_layered.c",
            "zip_source_open.c",
            "zip_source_pass_to_lower_layer.c",
            "zip_source_pkware_decode.c",
            "zip_source_pkware_encode.c",
            "zip_source_read.c",
            "zip_source_remove.c",
            "zip_source_rollback_write.c",
            "zip_source_seek.c",
            "zip_source_seek_write.c",
            "zip_source_stat.c",
            "zip_source_supports.c",
            "zip_source_tell.c",
            "zip_source_tell_write.c",
            "zip_source_window.c",
            "zip_source_write.c",
            "zip_source_zip.c",
            "zip_source_zip_new.c",
            "zip_stat.c",
            "zip_stat_index.c",
            "zip_stat_init.c",
            "zip_strerror.c",
            "zip_string.c",
            "zip_unchange.c",
            "zip_unchange_all.c",
            "zip_unchange_archive.c",
            "zip_unchange_data.c",
            "zip_utf-8.c",
        },
        .flags = &.{
            "-DZIP_STATIC",
        },
    });

    // "zip_winzip_aes.c",
    // "zip_source_winzip_aes_decode.c",
    // "zip_source_winzip_aes_encode.c",

    if (target.result.os.tag == .windows) {
        lib_mod.addCSourceFiles(.{
            .root = b.path("src/ext/libzip/lib"),
            .files = &.{
                "zip_random_win32.c",
                "zip_source_file_win32.c",
                "zip_source_file_win32_ansi.c",
                "zip_source_file_win32_named.c",
                "zip_source_file_win32_utf8.c",
                "zip_source_file_win32_utf16.c",
                // "zip_random_uwp.c", // TODO
            },
        });
    } else {
        lib_mod.addCSourceFiles(.{
            .root = b.path("src/ext/libzip/lib"),
            .files = &.{
                "zip_random_unix.c",
                "zip_source_file_stdio_named.c",
            },
        });
    }
    lib_mod.addIncludePath(b.path("src/ext/libzip/lib"));

    // TODO: generate this at build time
    lib_mod.addCSourceFile(.{
        .file = b.path("src/ext/libzip/zip_err_str.c"),
    });

    const config_header = b.addConfigHeader(
        .{ .style = .{ .cmake = b.path("src/ext/libzip/config.h.in") } },
        .{
            .ENABLE_FDOPEN = true,
            .HAVE___PROGNAME = true,
            .HAVE__CLOSE = true,
            .HAVE__DUP = true,
            .HAVE__FDOPEN = true,
            .HAVE__FILENO = true,
            .HAVE__FSEEKI64 = true,
            .HAVE__FSTAT64 = true,
            .HAVE__SETMODE = true,
            .HAVE__SNPRINTF = true,
            .HAVE__SNPRINTF_S = target.result.os.tag == .windows,
            .HAVE__SNWPRINTF_S = target.result.os.tag == .windows,
            .HAVE__STAT64 = true,
            .HAVE__STRDUP = true,
            .HAVE__STRICMP = true,
            .HAVE__STRTOI64 = true,
            .HAVE__STRTOUI64 = true,
            .HAVE__UNLINK = true,
            .HAVE_ARC4RANDOM = target.result.abi == .gnu,
            .HAVE_CLONEFILE = false,
            .HAVE_COMMONCRYPTO = true,
            .HAVE_CRYPTO = false, // TODO
            .HAVE_FICLONERANGE = target.result.os.tag == .linux,
            .HAVE_FILENO = true,
            .HAVE_FCHMOD = true,
            .HAVE_FSEEKO = true,
            .HAVE_FTELLO = true,
            .HAVE_GETPROGNAME = true,
            .HAVE_GNUTLS = true,
            .HAVE_LIBBZ2 = false, // TODO
            .HAVE_LIBLZMA = false, // TODO
            .HAVE_LIBZSTD = false, // TODO
            .HAVE_LOCALTIME_R = true,
            .HAVE_LOCALTIME_S = target.result.os.tag == .windows,
            .HAVE_MEMCPY_S = target.result.os.tag == .windows,
            .HAVE_MBEDTLS = true,
            .HAVE_MKSTEMP = true,
            .HAVE_NULLABLE = true,
            .HAVE_OPENSSL = true,
            .HAVE_SETMODE = false,
            .HAVE_SNPRINTF = true,
            .HAVE_SNPRINTF_S = false,
            .HAVE_STRCASECMP = true,
            .HAVE_STRDUP = true,
            .HAVE_STRERROR_S = false, //target.result.os.tag == .windows,
            .HAVE_STRERRORLEN_S = target.result.os.tag == .windows,
            .HAVE_STRICMP = true,
            .HAVE_STRNCPY_S = target.result.os.tag == .windows,
            .HAVE_STRTOLL = true,
            .HAVE_STRTOULL = true,
            .HAVE_STRUCT_TM_TM_ZONE = true,
            .HAVE_STDBOOL_H = true,
            .HAVE_STRINGS_H = true,
            .HAVE_UNISTD_H = target.result.os.tag != .windows,
            .HAVE_WINDOWS_CRYPTO = false,
            .SIZEOF_OFF_T = 8, // TODO
            .SIZEOF_SIZE_T = 8, // TODO
            .HAVE_DIRENT_H = true,
            .HAVE_FTS_H = true,
            .HAVE_NDIR_H = true,
            .HAVE_SYS_DIR_H = target.result.os.tag != .windows,
            .HAVE_SYS_NDIR_H = target.result.os.tag != .windows,
            .WORDS_BIGENDIAN = false,
            .HAVE_SHARED = false,
            .CMAKE_PROJECT_NAME = "libzip",
            .CMAKE_PROJECT_VERSION = "1.11.2",
        },
    );
    lib_mod.addConfigHeader(config_header);

    const zipconf_header = b.addConfigHeader(
        .{ .style = .{ .cmake = b.path("src/ext/libzip/zipconf.h.in") } },
        .{
            .ZIP_STATIC = true,
            .libzip_VERSION = "0.11.2",
            .libzip_VERSION_MAJOR = 0,
            .libzip_VERSION_MINOR = 11,
            .libzip_VERSION_PATCH = 2,

            .ZIP_NULLABLE_DEFINES = null,

            .LIBZIP_TYPES_INCLUDE = .@"#include <stdint.h>",

            .ZIP_INT8_T = .int8_t,
            .ZIP_UINT8_T = .uint8_t,
            .ZIP_INT16_T = .int16_t,
            .ZIP_UINT16_T = .uint16_t,
            .ZIP_INT32_T = .int32_t,
            .ZIP_UINT32_T = .uint32_t,
            .ZIP_INT64_T = .int64_t,
            .ZIP_UINT64_T = .uint64_t,
        },
    );
    lib_mod.addConfigHeader(zipconf_header);

    const lib = b.addLibrary(.{
        .linkage = .static,
        .name = "zip",
        .root_module = lib_mod,
    });

    //    lib.installHeader(b.path("src/ext/libzip/lib/zip.h"), "zip.h");
    lib.installConfigHeader(zipconf_header);
    //    b.installArtifact(lib);

    return lib;
}

pub fn build(b: *Build) !void {
    const git_version_opt: ?[]const u8 = b.option([]const u8, "version", "Specify the specific git version you want to package") orelse null;

    const cwd = b.build_root.handle;

    const target: ResolvedTarget = b.standardTargetOptions(.{});
    const optimize: std.builtin.OptimizeMode = b.standardOptimizeOption(.{});

    const compile_flag_min_macos_version: []const u8 = b.fmt("-mmacos-version-min={s}", .{MACOS_VERSION_MIN});

    /////////////////////////////////////////////////////////
    // zlib

    var z_sources = SourceFileCollector.init(b, ".c");
    defer z_sources.deinit();
    try z_sources.collect("src/ext/zlib/");

    const z_lib = b.addLibrary(.{
        .linkage = .static,
        .name = "z",
        .root_module = b.createModule(.{
            .target = target,
            .optimize = optimize,
            .link_libc = true,
            .sanitize_c = false,
        }),
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

    /////////////////////////////////////////////////////////
    // libzip
    const libzip = build_libzip(b, z_lib, target, optimize);

    /////////////////////////////////////////////////////////
    // curl - used in orca cli tool

    // Original zig build code MIT licensed from: https://github.com/jiacai2050/zig-curl/blob/main/libs/curl.zig
    // const curl_mod = b.createModule(.{
    //     .target = target,
    //     .optimize = optimize,
    //     .link_libc = true,
    //     .sanitize_c = false,
    // });

    // var curl_sources = SourceFileCollector.init(b, ".c");
    // try curl_sources.collect("src/ext/curl/lib");
    // try curl_sources.collect("src/ext/curl/lib/vauth");
    // try curl_sources.collect("src/ext/curl/lib/vtls");
    // try curl_sources.collect("src/ext/curl/lib/vssh");
    // try curl_sources.collect("src/ext/curl/lib/vquic");

    // for (curl_sources.files.items) |path| {
    //     curl_mod.addCSourceFile(.{
    //         .file = b.path(path),
    //         .flags = &.{"-std=gnu89"},
    //     });
    // }

    // curl_mod.addIncludePath(b.path("src/ext/curl/lib"));
    // curl_mod.addIncludePath(b.path("src/ext/curl/include"));
    // curl_mod.addIncludePath(b.path("src/ext/zlib"));

    // curl_mod.addCMacro("BUILDING_LIBCURL", "1");
    // curl_mod.addCMacro("CURL_STATICLIB", "1");
    // curl_mod.addCMacro("CURL_DISABLE_LDAP", "1");
    // curl_mod.addCMacro("CURL_DISABLE_LDAPS", "1");
    // // curl_mod.addCMacro("USE_MBEDTLS", "1");
    // curl_mod.addCMacro("CURL_DISABLE_DICT", "1");
    // curl_mod.addCMacro("CURL_DISABLE_FILE", "1");
    // curl_mod.addCMacro("CURL_DISABLE_FTP", "1");
    // curl_mod.addCMacro("CURL_DISABLE_GOPHER", "1");
    // curl_mod.addCMacro("CURL_DISABLE_IMAP", "1");
    // curl_mod.addCMacro("CURL_DISABLE_MQTT", "1");
    // curl_mod.addCMacro("CURL_DISABLE_POP3", "1");
    // curl_mod.addCMacro("CURL_DISABLE_RTSP", "1");
    // curl_mod.addCMacro("CURL_DISABLE_SMB", "1");
    // curl_mod.addCMacro("CURL_DISABLE_SMTP", "1");
    // curl_mod.addCMacro("CURL_DISABLE_TELNET", "1");
    // curl_mod.addCMacro("CURL_DISABLE_TFTP", "1");
    // curl_mod.addCMacro("HAVE_LIBZ", "1");
    // curl_mod.addCMacro("HAVE_ZLIB_H", "1");

    // if (target.result.os.tag == .windows) {
    //     curl_mod.addCMacro("USE_WINDOWS_SSPI", "1");
    //     curl_mod.addCMacro("USE_SCHANNEL", "1");
    //     curl_mod.addCMacro("OS", "\"win32\"");
    //     curl_mod.addCMacro("ENABLE_IPV6", "1");
    //     curl_mod.addCMacro("HAVE_GETHOSTBYNAME_R", "1");

    //     curl_mod.linkSystemLibrary("ws2_32", .{});
    //     curl_mod.linkSystemLibrary("wldap32", .{});
    //     curl_mod.linkSystemLibrary("advapi32", .{});
    //     curl_mod.linkSystemLibrary("crypt32", .{});
    //     curl_mod.linkSystemLibrary("gdi32", .{});
    //     curl_mod.linkSystemLibrary("user32", .{});
    //     curl_mod.linkSystemLibrary("bcrypt", .{});
    // } else {
    //     curl_mod.addCMacro("CURL_EXTERN_SYMBOL", "__attribute__ ((__visibility__ (\"default\"))");

    //     if (target.result.os.tag.isDarwin()) {
    //         curl_mod.addCMacro("OS", "\"mac\"");
    //     } else if (target.result.os.tag == .linux) {
    //         curl_mod.addCMacro("OS", "\"Linux\"");
    //         curl_mod.addCMacro("HAVE_LINUX_TCP_H", "1");
    //     }

    //     curl_mod.addCMacro("HAVE_ALARM", "1");
    //     curl_mod.addCMacro("HAVE_ALLOCA_H", "1");
    //     curl_mod.addCMacro("HAVE_ARPA_INET_H", "1");
    //     curl_mod.addCMacro("HAVE_ARPA_TFTP_H", "1");
    //     curl_mod.addCMacro("HAVE_ASSERT_H", "1");
    //     curl_mod.addCMacro("HAVE_BASENAME", "1");
    //     curl_mod.addCMacro("HAVE_BOOL_T", "1");
    //     curl_mod.addCMacro("HAVE_BUILTIN_AVAILABLE", "1");
    //     curl_mod.addCMacro("HAVE_CLOCK_GETTIME_MONOTONIC", "1");
    //     curl_mod.addCMacro("HAVE_DLFCN_H", "1");
    //     curl_mod.addCMacro("HAVE_ERRNO_H", "1");
    //     curl_mod.addCMacro("HAVE_FCNTL", "1");
    //     curl_mod.addCMacro("HAVE_FCNTL_H", "1");
    //     curl_mod.addCMacro("HAVE_FCNTL_O_NONBLOCK", "1");
    //     curl_mod.addCMacro("HAVE_FREEADDRINFO", "1");
    //     curl_mod.addCMacro("HAVE_FTRUNCATE", "1");
    //     curl_mod.addCMacro("HAVE_GETADDRINFO", "1");
    //     curl_mod.addCMacro("HAVE_GETEUID", "1");
    //     curl_mod.addCMacro("HAVE_GETPPID", "1");
    //     curl_mod.addCMacro("HAVE_GETHOSTBYNAME", "1");
    //     curl_mod.addCMacro("HAVE_GETHOSTBYNAME_R_6", "1");
    //     curl_mod.addCMacro("HAVE_GETHOSTNAME", "1");
    //     curl_mod.addCMacro("HAVE_GETPPID", "1");
    //     curl_mod.addCMacro("HAVE_GETPROTOBYNAME", "1");
    //     curl_mod.addCMacro("HAVE_GETPEERNAME", "1");
    //     curl_mod.addCMacro("HAVE_GETSOCKNAME", "1");
    //     curl_mod.addCMacro("HAVE_IF_NAMETOINDEX", "1");
    //     curl_mod.addCMacro("HAVE_GETPWUID", "1");
    //     curl_mod.addCMacro("HAVE_GETPWUID_R", "1");
    //     curl_mod.addCMacro("HAVE_GETRLIMIT", "1");
    //     curl_mod.addCMacro("HAVE_GETTIMEOFDAY", "1");
    //     curl_mod.addCMacro("HAVE_GMTIME_R", "1");
    //     curl_mod.addCMacro("HAVE_IFADDRS_H", "1");
    //     curl_mod.addCMacro("HAVE_INET_ADDR", "1");
    //     curl_mod.addCMacro("HAVE_INET_PTON", "1");
    //     curl_mod.addCMacro("HAVE_SA_FAMILY_T", "1");
    //     curl_mod.addCMacro("HAVE_INTTYPES_H", "1");
    //     curl_mod.addCMacro("HAVE_IOCTL", "1");
    //     curl_mod.addCMacro("HAVE_IOCTL_FIONBIO", "1");
    //     curl_mod.addCMacro("HAVE_IOCTL_SIOCGIFADDR", "1");
    //     curl_mod.addCMacro("HAVE_LDAP_URL_PARSE", "1");
    //     curl_mod.addCMacro("HAVE_LIBGEN_H", "1");
    //     curl_mod.addCMacro("HAVE_IDN2_H", "1");
    //     curl_mod.addCMacro("HAVE_LL", "1");
    //     curl_mod.addCMacro("HAVE_LOCALE_H", "1");
    //     curl_mod.addCMacro("HAVE_LOCALTIME_R", "1");
    //     curl_mod.addCMacro("HAVE_LONGLONG", "1");
    //     curl_mod.addCMacro("HAVE_MALLOC_H", "1");
    //     curl_mod.addCMacro("HAVE_MEMORY_H", "1");
    //     curl_mod.addCMacro("HAVE_NETDB_H", "1");
    //     curl_mod.addCMacro("HAVE_NETINET_IN_H", "1");
    //     curl_mod.addCMacro("HAVE_NETINET_TCP_H", "1");
    //     curl_mod.addCMacro("HAVE_NET_IF_H", "1");
    //     curl_mod.addCMacro("HAVE_PIPE", "1");
    //     curl_mod.addCMacro("HAVE_POLL", "1");
    //     curl_mod.addCMacro("HAVE_POLL_FINE", "1");
    //     curl_mod.addCMacro("HAVE_POLL_H", "1");
    //     curl_mod.addCMacro("HAVE_POSIX_STRERROR_R", "1");
    //     curl_mod.addCMacro("HAVE_PTHREAD_H", "1");
    //     curl_mod.addCMacro("HAVE_PWD_H", "1");
    //     curl_mod.addCMacro("HAVE_RECV", "1");
    //     curl_mod.addCMacro("HAVE_SELECT", "1");
    //     curl_mod.addCMacro("HAVE_SEND", "1");
    //     curl_mod.addCMacro("HAVE_FSETXATTR", "1");
    //     curl_mod.addCMacro("HAVE_FSETXATTR_5", "1");
    //     curl_mod.addCMacro("HAVE_SETJMP_H", "1");
    //     curl_mod.addCMacro("HAVE_SETLOCALE", "1");
    //     curl_mod.addCMacro("HAVE_SETRLIMIT", "1");
    //     curl_mod.addCMacro("HAVE_SETSOCKOPT", "1");
    //     curl_mod.addCMacro("HAVE_SIGACTION", "1");
    //     curl_mod.addCMacro("HAVE_SIGINTERRUPT", "1");
    //     curl_mod.addCMacro("HAVE_SIGNAL", "1");
    //     curl_mod.addCMacro("HAVE_SIGNAL_H", "1");
    //     curl_mod.addCMacro("HAVE_SIGSETJMP", "1");
    //     curl_mod.addCMacro("HAVE_SOCKADDR_IN6_SIN6_SCOPE_ID", "1");
    //     curl_mod.addCMacro("HAVE_SOCKET", "1");
    //     curl_mod.addCMacro("HAVE_STDBOOL_H", "1");
    //     curl_mod.addCMacro("HAVE_STDINT_H", "1");
    //     curl_mod.addCMacro("HAVE_STDIO_H", "1");
    //     curl_mod.addCMacro("HAVE_STDLIB_H", "1");
    //     curl_mod.addCMacro("HAVE_STRCASECMP", "1");
    //     curl_mod.addCMacro("HAVE_STRDUP", "1");
    //     curl_mod.addCMacro("HAVE_STRERROR_R", "1");
    //     curl_mod.addCMacro("HAVE_STRINGS_H", "1");
    //     curl_mod.addCMacro("HAVE_STRING_H", "1");
    //     curl_mod.addCMacro("HAVE_STRSTR", "1");
    //     curl_mod.addCMacro("HAVE_STRTOK_R", "1");
    //     curl_mod.addCMacro("HAVE_STRTOLL", "1");
    //     curl_mod.addCMacro("HAVE_STRUCT_SOCKADDR_STORAGE", "1");
    //     curl_mod.addCMacro("HAVE_STRUCT_TIMEVAL", "1");
    //     curl_mod.addCMacro("HAVE_SYS_IOCTL_H", "1");
    //     curl_mod.addCMacro("HAVE_SYS_PARAM_H", "1");
    //     curl_mod.addCMacro("HAVE_SYS_POLL_H", "1");
    //     curl_mod.addCMacro("HAVE_SYS_RESOURCE_H", "1");
    //     curl_mod.addCMacro("HAVE_SYS_SELECT_H", "1");
    //     curl_mod.addCMacro("HAVE_SYS_SOCKET_H", "1");
    //     curl_mod.addCMacro("HAVE_SYS_STAT_H", "1");
    //     curl_mod.addCMacro("HAVE_SYS_TIME_H", "1");
    //     curl_mod.addCMacro("HAVE_SYS_TYPES_H", "1");
    //     curl_mod.addCMacro("HAVE_SYS_UIO_H", "1");
    //     curl_mod.addCMacro("HAVE_SYS_UN_H", "1");
    //     curl_mod.addCMacro("HAVE_TERMIOS_H", "1");
    //     curl_mod.addCMacro("HAVE_TERMIO_H", "1");
    //     curl_mod.addCMacro("HAVE_TIME_H", "1");
    //     curl_mod.addCMacro("HAVE_UNAME", "1");
    //     curl_mod.addCMacro("HAVE_UNISTD_H", "1");
    //     curl_mod.addCMacro("HAVE_UTIME", "1");
    //     curl_mod.addCMacro("HAVE_UTIMES", "1");
    //     curl_mod.addCMacro("HAVE_UTIME_H", "1");
    //     curl_mod.addCMacro("HAVE_VARIADIC_MACROS_C99", "1");
    //     curl_mod.addCMacro("HAVE_VARIADIC_MACROS_GCC", "1");
    //     curl_mod.addCMacro("RANDOM_FILE", "\"/dev/urandom\"");
    //     curl_mod.addCMacro("RECV_TYPE_ARG1", "int");
    //     curl_mod.addCMacro("RECV_TYPE_ARG2", "void *");
    //     curl_mod.addCMacro("RECV_TYPE_ARG3", "size_t");
    //     curl_mod.addCMacro("RECV_TYPE_ARG4", "int");
    //     curl_mod.addCMacro("RECV_TYPE_RETV", "ssize_t");
    //     curl_mod.addCMacro("SEND_QUAL_ARG2", "const");
    //     curl_mod.addCMacro("SEND_TYPE_ARG1", "int");
    //     curl_mod.addCMacro("SEND_TYPE_ARG2", "void *");
    //     curl_mod.addCMacro("SEND_TYPE_ARG3", "size_t");
    //     curl_mod.addCMacro("SEND_TYPE_ARG4", "int");
    //     curl_mod.addCMacro("SEND_TYPE_RETV", "ssize_t");
    //     curl_mod.addCMacro("SIZEOF_INT", "4");
    //     curl_mod.addCMacro("SIZEOF_SHORT", "2");
    //     curl_mod.addCMacro("SIZEOF_LONG", "8");
    //     curl_mod.addCMacro("SIZEOF_OFF_T", "8");
    //     curl_mod.addCMacro("SIZEOF_CURL_OFF_T", "8");
    //     curl_mod.addCMacro("SIZEOF_SIZE_T", "8");
    //     curl_mod.addCMacro("SIZEOF_TIME_T", "8");
    //     curl_mod.addCMacro("STDC_HEADERS", "1");
    //     curl_mod.addCMacro("TIME_WITH_SYS_TIME", "1");
    //     curl_mod.addCMacro("USE_THREADS_POSIX", "1");
    //     curl_mod.addCMacro("USE_UNIX_SOCKETS", "1");
    //     curl_mod.addCMacro("_FILE_OFFSET_BITS", "64");
    // }
    // const curl_lib = b.addLibrary(.{
    //     .linkage = .static,
    //     .name = "curl",
    //     .root_module = curl_mod,
    // });

    /////////////////////////////////////////////////////////
    // Orca CLI tool

    const git_version_tool: []const u8 = blk: {
        if (git_version_opt) |git_version| {
            break :blk try b.allocator.dupe(u8, git_version);
        } else {
            const git_version = b.run(&.{ "git", "rev-parse", "--short", "HEAD" });
            break :blk std.mem.trimRight(u8, git_version, "\n");
        }
    };

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
    // binding generator

    const bindgen_exe: *Build.Step.Compile = b.addExecutable(.{
        .name = "bindgen",
        .root_module = b.createModule(.{
            .root_source_file = b.path("src/build/bindgen.zig"),
            .target = b.graph.host,
            .optimize = .Debug,
        }),
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
    const install_angle_libs = b.addInstallDirectory(.{ .source_dir = angle_lib_path, .install_dir = .bin, .install_subdir = "" });
    const install_dawn_libs = b.addInstallDirectory(.{ .source_dir = dawn_lib_path, .install_dir = .bin, .install_subdir = "" });

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

    var orca_platform_compile_flags: std.ArrayList([]const u8) = .init(b.allocator);
    defer orca_platform_compile_flags.deinit();
    try orca_platform_compile_flags.append("-std=c11");
    try orca_platform_compile_flags.append("-Werror");
    try orca_platform_compile_flags.append("-DOC_BUILD_DLL");
    try orca_platform_compile_flags.append("-D_USE_MATH_DEFINES");
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
    const orca_platform_lib = b.addLibrary(.{
        .linkage = .dynamic,
        .name = "orca_platform",
        .win32_manifest = b.path("src/app/win32_manifest.manifest"),
        .root_module = b.createModule(.{
            .target = target,
            .optimize = optimize,
        }),
    });

    orca_platform_lib.install_name = "@executable_path/liborca_platform.dylib";

    orca_platform_lib.addIncludePath(b.path("src"));
    orca_platform_lib.addIncludePath(b.path("src/ext"));
    orca_platform_lib.addIncludePath(b.path("src/ext/angle/include"));
    orca_platform_lib.addIncludePath(b.path("src/ext/dawn/include"));
    orca_platform_lib.addLibraryPath(angle_include_path);

    var orca_platform_files: std.ArrayList([]const u8) = .init(b.allocator);
    try orca_platform_files.append("src/orca.c");

    orca_platform_lib.addLibraryPath(angle_lib_path);
    orca_platform_lib.addLibraryPath(dawn_lib_path);

    if (target.result.os.tag == .windows) {
        orca_platform_lib.linkSystemLibrary("user32");
        orca_platform_lib.linkSystemLibrary("opengl32");
        orca_platform_lib.linkSystemLibrary("gdi32");
        orca_platform_lib.linkSystemLibrary("dxgi");
        orca_platform_lib.linkSystemLibrary("dxguid");
        orca_platform_lib.linkSystemLibrary("d3d11");
        orca_platform_lib.linkSystemLibrary("dcomp");
        orca_platform_lib.linkSystemLibrary("shcore");
        orca_platform_lib.linkSystemLibrary("dwmapi");
        orca_platform_lib.linkSystemLibrary("comctl32");
        orca_platform_lib.linkSystemLibrary("ole32");
        orca_platform_lib.linkSystemLibrary("shell32");
        orca_platform_lib.linkSystemLibrary("shlwapi");

        // TODO delay load these the graphics libs
        // orca_platform_lib.linkSystemLibrary("delayimp");
        orca_platform_lib.linkSystemLibrary("libEGL.dll");
        orca_platform_lib.linkSystemLibrary("libGLESv2.dll");
        orca_platform_lib.linkSystemLibrary("webgpu");
    } else if (target.result.os.tag.isDarwin()) {
        orca_platform_lib.linkFramework("Carbon");
        orca_platform_lib.linkFramework("Cocoa");
        orca_platform_lib.linkFramework("Metal");
        orca_platform_lib.linkFramework("QuartzCore");
        orca_platform_lib.linkFramework("UniformTypeIdentifiers");

        orca_platform_lib.linkSystemLibrary2("EGL", .{ .weak = true });
        orca_platform_lib.linkSystemLibrary2("GLESv2", .{ .weak = true });
        orca_platform_lib.linkSystemLibrary2("webgpu", .{ .weak = true });

        try orca_platform_files.append("src/orca.m");
    }

    orca_platform_lib.addCSourceFiles(.{
        .files = orca_platform_files.items,
        .flags = orca_platform_compile_flags.items,
    });

    orca_platform_lib.step.dependOn(&stage_angle_dawn_src.step);
    orca_platform_lib.step.dependOn(&install_angle_libs.step);
    orca_platform_lib.step.dependOn(&install_dawn_libs.step);

    orca_platform_lib.step.dependOn(&update_wgpu_header.step);

    orca_platform_lib.step.dependOn(&orca_runtime_bindgen_core.step);
    orca_platform_lib.step.dependOn(&orca_runtime_bindgen_surface.step);
    orca_platform_lib.step.dependOn(&orca_runtime_bindgen_clock.step);
    orca_platform_lib.step.dependOn(&orca_runtime_bindgen_io.step);
    orca_platform_lib.step.dependOn(&orca_runtime_bindgen_gles.step);

    const orca_platform_install_opts: Build.Step.InstallArtifact.Options = .{
        .dest_dir = .{ .override = .bin },
    };

    const orca_platform_install: *Build.Step.InstallArtifact = b.addInstallArtifact(orca_platform_lib, orca_platform_install_opts);

    // warm

    const warm_lib = b.addLibrary(.{
        .linkage = .static,
        .name = "warm",
        .root_module = b.createModule(.{
            .target = target,
            .optimize = optimize,
            .link_libc = true,
            .sanitize_c = false,
        }),
    });

    const warm_sources: []const []const u8 = &.{
        "src/warm/reader.c",
        "src/warm/instructions.c",
        "src/warm/wa_types.c",
        "src/warm/dwarf.c",
        "src/warm/debug_import.c",
        "src/warm/parser.c",
        "src/warm/compiler.c",
        "src/warm/module.c",
        "src/warm/instance.c",
        "src/warm/interpreter.c",
        "src/warm/debug_info.c",
        "src/warm/warm_adapter.c",
    };

    var warm_compile_flags: std.ArrayList([]const u8) = .init(b.allocator);
    try warm_compile_flags.append("-std=c11");
    try warm_compile_flags.append("-Werror");
    try warm_compile_flags.append("-g");
    try warm_compile_flags.append("-O0");
    try warm_compile_flags.append("-fno-sanitize=undefined");

    warm_lib.addIncludePath(b.path("src"));
    warm_lib.addCSourceFiles(.{
        .files = warm_sources,
        .flags = warm_compile_flags.items,
    });

    const warm_install_opts: Build.Step.InstallArtifact.Options = .{
        .dest_dir = .{ .override = .bin },
    };

    const warm_install: *Build.Step.InstallArtifact = b.addInstallArtifact(warm_lib, warm_install_opts);

    // orca launcher
    var orca_launcher_compile_flags: std.ArrayList([]const u8) = .init(b.allocator);
    defer orca_launcher_compile_flags.deinit();

    if (optimize == .Debug) {
        try orca_launcher_compile_flags.append("-DOC_DEBUG");
        try orca_launcher_compile_flags.append("-DOC_LOG_COMPILE_DEBUG");
    }
    try orca_launcher_compile_flags.append("-std=c11");
    try orca_launcher_compile_flags.append("-Werror");
    try orca_launcher_compile_flags.append(b.fmt("-DORCA_TOOL_VERSION={s}", .{git_version_tool}));

    const orca_launcher_exe = b.addExecutable(.{
        .name = "orca",
        .root_module = b.createModule(.{
            .target = target,
            .optimize = optimize,
        }),
    });
    orca_launcher_exe.addIncludePath(b.path("src"));
    orca_launcher_exe.addIncludePath(b.path("src/ext"));
    orca_launcher_exe.addIncludePath(angle_include_path);

    orca_launcher_exe.root_module.addRPathSpecial("@executable_path/");

    orca_launcher_exe.addCSourceFiles(.{
        .files = &.{"src/launcher/main.c"},
        .flags = orca_launcher_compile_flags.items,
    });

    orca_launcher_exe.linkLibrary(orca_platform_lib);
    orca_launcher_exe.linkLibrary(libzip);
    orca_launcher_exe.linkLibC();

    orca_launcher_exe.step.dependOn(&install_angle_libs.step);
    orca_launcher_exe.step.dependOn(&install_dawn_libs.step);

    const orca_launcher_exe_install: *Build.Step.InstallArtifact = b.addInstallArtifact(orca_launcher_exe, .{});

    // orca runtime exe

    var orca_runtime_compile_flags: std.ArrayList([]const u8) = .init(b.allocator);
    defer orca_runtime_compile_flags.deinit();

    if (optimize == .Debug) {
        try orca_runtime_compile_flags.append("-DOC_DEBUG");
        try orca_runtime_compile_flags.append("-DOC_LOG_COMPILE_DEBUG");
    }
    try orca_runtime_compile_flags.append("-std=c11");
    try orca_runtime_compile_flags.append("-Werror");

    const orca_runtime_exe = b.addExecutable(.{
        .name = "orca_runtime",
        .root_module = b.createModule(.{
            .target = target,
            .optimize = optimize,
        }),
    });

    orca_runtime_exe.addIncludePath(b.path("src"));
    orca_runtime_exe.addIncludePath(b.path("src/ext"));
    orca_runtime_exe.addIncludePath(angle_include_path);

    orca_runtime_exe.root_module.addRPathSpecial("@executable_path/");

    orca_runtime_exe.addCSourceFiles(.{
        .files = &.{"src/runtime/main.c"},
        .flags = orca_runtime_compile_flags.items,
    });

    orca_runtime_exe.linkLibrary(warm_lib);
    orca_runtime_exe.linkLibrary(orca_platform_lib);
    orca_runtime_exe.linkLibrary(libzip);
    orca_runtime_exe.linkLibC();

    orca_runtime_exe.step.dependOn(&install_angle_libs.step);
    orca_runtime_exe.step.dependOn(&install_dawn_libs.step);

    orca_runtime_exe.step.dependOn(&orca_runtime_bindgen_core.step);
    orca_runtime_exe.step.dependOn(&orca_runtime_bindgen_surface.step);
    orca_runtime_exe.step.dependOn(&orca_runtime_bindgen_clock.step);
    orca_runtime_exe.step.dependOn(&orca_runtime_bindgen_io.step);
    orca_runtime_exe.step.dependOn(&orca_runtime_bindgen_gles.step);

    const orca_runtime_exe_install: *Build.Step.InstallArtifact = b.addInstallArtifact(orca_runtime_exe, .{});

    // if (target.result.os.tag == .macos) {
    //     const run_install_name = b.addSystemCommand(&.{
    //         "install_name_tool",
    //         "-delete_rpath",
    //     });
    //     run_install_name.addDirectoryArg(orca_runtime_exe.getEmittedBin().dirname());
    //     run_install_name.addFileArg(orca_runtime_exe.getEmittedBin());
    //     orca_runtime_exe_install.step.dependOn(&run_install_name.step);
    // }

    ///////////////////////////////////////////////////////
    // orca wasm libc

    var wasm_target_query: std.Target.Query = .{
        .cpu_arch = std.Target.Cpu.Arch.wasm32,
        .os_tag = std.Target.Os.Tag.freestanding,
    };
    wasm_target_query.cpu_features_add.addFeature(@intFromEnum(std.Target.wasm.Feature.bulk_memory));
    wasm_target_query.cpu_features_add.addFeature(@intFromEnum(std.Target.wasm.Feature.nontrapping_fptoint));

    const wasm_target: ResolvedTarget = b.resolveTargetQuery(wasm_target_query);

    const wasm_optimize: std.builtin.OptimizeMode = .ReleaseSmall;

    // zig fmt: off
    const libc_flags: []const []const u8 = &.{
        // need to provide absolute paths to these since we're overriding the default zig lib dir
        "-isystem", b.pathFromRoot("src/orca-libc/src/include"),
        "-isystem", b.pathFromRoot("src/orca-libc/src/include/private"),

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

    const dummy_crt_obj = b.addObject(.{
        .name = "crt1",
        .root_module = b.createModule(.{
            .target = wasm_target,
            .optimize = wasm_optimize,
            .link_libc = false,
        }),
    });
    dummy_crt_obj.addIncludePath(b.path("src"));
    dummy_crt_obj.addIncludePath(b.path("src/orca-libc/src/arch"));
    dummy_crt_obj.addIncludePath(b.path("src/orca-libc/src/internal"));
    dummy_crt_obj.addCSourceFiles(.{
        .files = &.{"src/orca-libc/src/crt/crt1.c"},
        .flags = libc_flags,
    });

    const libc_install_opts: Build.Step.InstallArtifact.Options = .{
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

    var wasm_libc_sources = SourceFileCollector.init(b, ".c");
    defer wasm_libc_sources.deinit();

    var wasm_libc_objs: std.ArrayList(*Build.Step.Compile) = .init(b.allocator);
    try wasm_libc_objs.ensureUnusedCapacity(1024); // there are 496 .c files in the libc so this should be enough

    for (wasm_libc_source_paths) |path| {
        wasm_libc_sources.files.shrinkRetainingCapacity(0);
        try wasm_libc_sources.collect(path);

        const libc_group: []const u8 = std.fs.path.basename(path);

        for (wasm_libc_sources.files.items) |filepath| {
            const filename: []const u8 = std.fs.path.basename(filepath);
            const obj_name: []const u8 = try std.mem.join(b.allocator, "_", &.{ "libc", libc_group, filename });
            const obj = b.addObject(.{
                .name = obj_name,
                .root_module = b.createModule(.{
                    .target = wasm_target,
                    .optimize = wasm_optimize,
                    .single_threaded = true,
                    .link_libc = false,
                }),
                .zig_lib_dir = b.path("src/orca-libc"), // ensures c stdlib headers bundled with zig are ignored
            });
            obj.addIncludePath(b.path("src"));
            obj.addIncludePath(b.path("src/orca-libc/src/arch"));
            obj.addIncludePath(b.path("src/orca-libc/src/internal"));
            obj.addCSourceFiles(.{
                .files = &.{filepath},
                .flags = libc_flags,
            });
            try wasm_libc_objs.append(obj);
        }
    }

    const wasm_libc_lib = b.addLibrary(.{
        .linkage = .static,
        .name = "c",
        .root_module = b.createModule(.{
            .target = wasm_target,
            .optimize = wasm_optimize,
            .link_libc = false,
            .single_threaded = true,
        }),
    });
    for (wasm_libc_objs.items) |obj| {
        wasm_libc_lib.addObject(obj);
    }

    const libc_install: *Build.Step.InstallArtifact = b.addInstallArtifact(wasm_libc_lib, libc_install_opts);

    const libc_header_install: *Build.Step.InstallDir = b.addInstallDirectory(.{
        .source_dir = b.path("src/orca-libc/include"),
        .install_dir = .{ .custom = "orca-libc" },
        .install_subdir = "include",
    });

    /////////////////////////////////////////////////////////
    // Orca wasm SDK

    const wasm_sdk_flags: []const []const u8 = &.{
        "--no-standard-libraries",
        "-D__ORCA__",
    };

    const wasm_sdk_obj = b.addObject(.{
        .name = "orca_wasm",
        .root_module = b.createModule(.{
            .target = wasm_target,
            .optimize = .ReleaseFast,
            .single_threaded = true,
            .link_libc = false,
        }),
        .zig_lib_dir = b.path("src/orca-libc"),
    });
    wasm_sdk_obj.addIncludePath(b.path("src"));
    wasm_sdk_obj.addIncludePath(b.path("src/ext"));
    wasm_sdk_obj.addIncludePath(b.path("src/orca-libc/include"));
    wasm_sdk_obj.addCSourceFile(.{
        .file = b.path("src/orca.c"),
        .flags = wasm_sdk_flags,
    });

    wasm_sdk_obj.step.dependOn(&install_angle_libs.step);
    wasm_sdk_obj.step.dependOn(&install_dawn_libs.step);

    wasm_sdk_obj.step.dependOn(&stage_angle_dawn_src.step);

    wasm_sdk_obj.step.dependOn(&orca_runtime_bindgen_core.step);
    wasm_sdk_obj.step.dependOn(&orca_runtime_bindgen_surface.step);
    wasm_sdk_obj.step.dependOn(&orca_runtime_bindgen_clock.step);
    wasm_sdk_obj.step.dependOn(&orca_runtime_bindgen_io.step);
    wasm_sdk_obj.step.dependOn(&orca_runtime_bindgen_gles.step);

    const wasm_sdk_lib = b.addLibrary(.{
        .linkage = .static,
        .name = "orca_wasm",
        .root_module = b.createModule(.{
            .target = wasm_target,
            .optimize = wasm_optimize,
            .link_libc = false,
            .single_threaded = true,
        }),
    });
    wasm_sdk_lib.addObject(wasm_sdk_obj);

    const wask_sdk_install_opts: Build.Step.InstallArtifact.Options = .{
        .dest_dir = .{ .override = .bin },
    };

    const wasm_sdk_install: *Build.Step.InstallArtifact = b.addInstallArtifact(wasm_sdk_lib, wask_sdk_install_opts);

    ///////////////////////////////////////////////////////////////
    // makeapp sript

    const makeapp_exe = b.addExecutable(.{
        .name = "make-app",
        .root_module = b.createModule(.{
            .target = b.graph.host,
            .optimize = .Debug,
        }),
    });
    makeapp_exe.addIncludePath(b.path("src"));
    makeapp_exe.addCSourceFiles(.{
        .files = &.{
            "src/build/makeapp.c",
        },
        .flags = &.{},
    });
    if (target.result.os.tag == .windows) {
        makeapp_exe.linkSystemLibrary("user32");
        makeapp_exe.linkSystemLibrary("shlwapi");
    }
    const makeapp = b.addRunArtifact(makeapp_exe);

    makeapp.step.dependOn(&orca_platform_install.step);
    makeapp.step.dependOn(&orca_runtime_exe_install.step);
    makeapp.step.dependOn(&orca_launcher_exe_install.step);
    makeapp.step.dependOn(&libc_install.step);
    makeapp.step.dependOn(&libc_header_install.step);
    makeapp.step.dependOn(&dummy_crt_install.step);
    makeapp.step.dependOn(&wasm_sdk_install.step);

    ///////////////////////////////////////////////////////////////
    // zig build - default install step builds and installs a dev build of orca

    const build_orca = b.step("orca", "Build all orca binaries");
    build_orca.dependOn(&warm_install.step);
    build_orca.dependOn(&orca_platform_install.step);
    build_orca.dependOn(&orca_runtime_exe_install.step);
    build_orca.dependOn(&orca_launcher_exe_install.step);
    build_orca.dependOn(&libc_install.step);
    build_orca.dependOn(&libc_header_install.step);
    build_orca.dependOn(&dummy_crt_install.step);
    build_orca.dependOn(&wasm_sdk_install.step);

    build_orca.dependOn(&makeapp.step);

    const package_sdk_exe: *Build.Step.Compile = b.addExecutable(.{
        .name = "package-sdk",
        .root_module = b.createModule(.{
            .root_source_file = b.path("src/build/package_sdk.zig"),
            .target = b.graph.host,
            .optimize = .Debug,
        }),
    });

    const opt_sdk_version = b.option([]const u8, "sdk-version", "Override current git version for sdk packaging.");

    //const sdk_install_path_opt: ?[]const u8 = b.option([]const u8, "sdk-path", "Specify absolute path for installing the Orca SDK.");
    //    const orca_install: *Step.Run = try installOrcaSdk(b, target, build_orca, package_sdk_exe, sdk_install_path_opt, git_version_opt, opt_sdk_version);

    const local_install_path: []const u8 = b.pathJoin(&.{ b.install_path, "sdk" });
    const orca_install_local: *Step.Run = try installOrcaSdk(b, target, build_orca, package_sdk_exe, local_install_path, git_version_opt, opt_sdk_version);

    //b.getInstallStep().dependOn(&orca_install.step);
    b.getInstallStep().dependOn(build_orca);

    const orca_tool_local_exe_name: []const u8 = if (b.graph.host.result.os.tag == .windows) "orca.exe" else "orca";
    const orca_tool_local_path: []const u8 = b.pathJoin(&.{ local_install_path, orca_tool_local_exe_name });

    /////////////////////////////////////////////////////////////////
    // samples

    const samples = b.step("samples", "Build all samples into  (demo Orca apps)");
    {
        const SampleConfig = struct {
            name: []const u8,
            has_icon: bool,
            has_resources: bool,
            has_shaders: bool,
            sources: []const []const u8,
        };
        // zig fmt: off
        const all_samples = [_]SampleConfig{
            .{ .name = "breakout", .has_icon = true,  .has_resources = true,  .has_shaders = false, .sources = &.{"main.c"} },
            .{ .name = "clock",    .has_icon = true,  .has_resources = true,  .has_shaders = false, .sources = &.{"main.c"} },
            .{ .name = "fluid",    .has_icon = true,  .has_resources = false, .has_shaders = true,  .sources = &.{"main.c"} },
            .{ .name = "microui",  .has_icon = false, .has_resources = true,  .has_shaders = false, .sources = &.{"main.c", "microui/microui.c"} },
            .{ .name = "triangle", .has_icon = false, .has_resources = false, .has_shaders = false, .sources = &.{"main.c"} },
            .{ .name = "ui",       .has_icon = false, .has_resources = true,  .has_shaders = false, .sources = &.{"main.c"} },
        };
        // zig fmt: on

        for (all_samples) |config| {
            var sources: std.ArrayList([]const u8) = .init(b.allocator);
            try sources.ensureTotalCapacity(config.sources.len);
            for (config.sources) |shortpath| {
                const path = b.pathJoin(&.{ "samples", config.name, "src", shortpath });
                sources.appendAssumeCapacity(path);
            }

            const icon_path = if (config.has_icon) b.pathJoin(&.{ "samples", config.name, "icon.png" }) else null;
            const resource_path = if (config.has_resources) b.pathJoin(&.{ "samples", config.name, "data" }) else null;

            var shader_sources = SourceFileCollector.init(b, ".glsl");
            if (config.has_shaders) {
                const path = b.pathJoin(&.{ "samples", config.name, "src", "shaders" });
                try shader_sources.collect(path);
                std.debug.assert(shader_sources.files.items.len > 0);
            }

            const steps = buildOrcaApp(
                b,
                target,
                wasm_sdk_lib,
                wasm_libc_lib,
                gen_header_exe,
                &orca_install_local.step,
                orca_tool_local_path,
                .{
                    .name = config.name,
                    .sources = sources.items,
                    .install = "samples",

                    .icon_path = icon_path,
                    .resource_path = resource_path,
                    .shaders = if (shader_sources.files.items.len > 0) shader_sources.files.items else null,
                    .create_run_step = true,
                },
            );

            samples.dependOn(steps.build_or_bundle);

            if (steps.run) |run_sample| {
                const run_sample_step_name = b.fmt("sample-{s}", .{config.name});
                const run_sample_step_description = b.fmt("Build, bundle, and run the {s} sample", .{config.name});
                const run_sample_step = b.step(run_sample_step_name, run_sample_step_description);
                run_sample_step.dependOn(&run_sample.step);
            }
        }
    }

    /////////////////////////////////////////////////////////////////
    // sketches

    const sketches = b.step("sketches", "Build all sketches");

    const sketches_install_opts: Build.Step.InstallArtifact.Options = .{
        .dest_dir = .{ .override = .{ .custom = "sketches" } },
    };

    const orca_platform_sketches_install: *Build.Step.InstallArtifact = b.addInstallArtifact(orca_platform_lib, sketches_install_opts);
    sketches.dependOn(&orca_platform_sketches_install.step);

    {
        const resources: []const []const u8 = &.{
            "resources/CMUSerif-Roman.ttf",
            "resources/Courier.ttf",
            "resources/gamma-1.0-or-2.2.png",
            "resources/gamma_dalai_lama_gray.png",
            "resources/gradient_srgb.png",
            "resources/OpenSansLatinSubset.ttf",
            "resources/OpenSans-Regular.ttf",
            "resources/OpenSans-Bold.ttf",
            "resources/Top512.png",
            "resources/triceratops.png",
        };

        const sketches_install_path = b.pathJoin(&.{ b.install_path, "sketches" });

        const stage_sketch_dependency_artifacts = b.addUpdateSourceFiles();
        for (resources) |resource| {
            const src = b.path(b.pathJoin(&.{ "sketches", resource }));
            const dest = b.pathJoin(&.{ sketches_install_path, resource });
            stage_sketch_dependency_artifacts.addCopyFileToSource(src, dest);
        }
        sketches.dependOn(&stage_sketch_dependency_artifacts.step);

        const sketches_install_dir: Build.InstallDir = .{ .custom = "sketches" };
        const install_angle_libs_sketches = b.addInstallDirectory(.{ .source_dir = angle_lib_path, .install_dir = sketches_install_dir, .install_subdir = "" });
        const install_dawn_libs_sketches = b.addInstallDirectory(.{ .source_dir = dawn_lib_path, .install_dir = sketches_install_dir, .install_subdir = "" });
        sketches.dependOn(&install_angle_libs_sketches.step);
        sketches.dependOn(&install_dawn_libs_sketches.step);
    }

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
        // "ui", // bitrotted
    };

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

        const sketch_exe: *Build.Step.Compile = b.addExecutable(.{
            .name = sketch,
            .root_module = b.createModule(.{
                .target = target,
                .optimize = optimize,
                .link_libc = true,
            }),
        });
        sketch_exe.addCSourceFiles(.{
            .files = &.{sketch_source},
            .flags = flags,
        });
        sketch_exe.linkLibrary(orca_platform_lib);

        const install: *Build.Step.InstallArtifact = b.addInstallArtifact(sketch_exe, sketches_install_opts);
        sketches.dependOn(&install.step);
    }

    /////////////////////////////////////////////////////////////////
    // tests

    const tests = b.step("test", "Build and run all tests");

    // get wasm tools path
    var wasm_tools: LazyPath = .{ .cwd_relative = "" };
    if (target.result.os.tag == .windows) {
        if (b.lazyDependency("wasm-tools-windows-x64", .{})) |dep| {
            wasm_tools = dep.path("wasm-tools.exe");
        }
    } else if (target.result.os.tag.isDarwin()) {
        if (target.result.cpu.arch == .aarch64) {
            if (b.lazyDependency("wasm-tools-mac-arm64", .{})) |dep| {
                wasm_tools = dep.path("wasm-tools");
            }
        } else {
            if (b.lazyDependency("wasm-tools-mac-x64", .{})) |dep| {
                wasm_tools = dep.path("wasm-tools");
            }
        }
    } else {
        const fail = b.addFail("wasm-tools dependency not configured for this platform.");
        b.getInstallStep().dependOn(&fail.step);
    }

    // warm testsuite
    const wasm_tests_convert_exe = b.addExecutable(.{
        .name = "wast_convert",
        .root_module = b.createModule(.{
            .root_source_file = b.path("tests/warm/convert.zig"),
            .target = b.graph.host,
            .optimize = .Debug,
        }),
    });

    const wasm_tests_convert = b.addRunArtifact(wasm_tests_convert_exe);
    wasm_tests_convert.addPrefixedFileArg("--wasm-tools=", wasm_tools);
    wasm_tests_convert.addPrefixedDirectoryArg("--tests=", b.path("tests/warm/core"));
    const wasm_tests_dir = wasm_tests_convert.addPrefixedOutputDirectoryArg("--out=", "warm/testsuite");

    const wasm_tests_install_dir: Build.InstallDir = .{ .custom = "tests/warm/core" };
    const wasm_tests_install = b.addInstallDirectory(.{ .source_dir = wasm_tests_dir, .install_dir = wasm_tests_install_dir, .install_subdir = "" });

    const warm_test_exe = b.addExecutable(.{
        .name = "warm-test",
        .root_module = b.createModule(.{
            .target = b.graph.host,
            .optimize = .Debug,
        }),
    });
    warm_test_exe.addIncludePath(b.path("src"));
    warm_test_exe.addIncludePath(b.path("tests/warm"));
    warm_test_exe.addCSourceFiles(.{
        .files = &.{
            "tests/warm/main.c",
        },
        .flags = &.{},
    });
    warm_test_exe.linkLibrary(warm_lib);

    if (target.result.os.tag == .windows) {
        warm_test_exe.linkSystemLibrary("user32");
        warm_test_exe.linkSystemLibrary("shlwapi");
    }

    const warm_test_install = b.addInstallArtifact(warm_test_exe, .{
        .dest_dir = .{ .override = .{ .custom = "tests/warm" } },
    });

    const warm_test = b.addRunArtifact(warm_test_exe);
    warm_test.addArg("test");
    warm_test.addDirectoryArg(wasm_tests_dir);

    tests.dependOn(&wasm_tests_install.step);
    tests.dependOn(&warm_test.step);
    tests.dependOn(&warm_test_install.step);

    // api tests
    const TestConfig = struct {
        name: []const u8,
        run: bool = false,
        wasm: bool = false,
        wasm_has_resources: bool = false,
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
        },
        .{
            .name = "wasm_stdio",
            .run = true,
            .wasm = true,
            .wasm_has_resources = true,
        },
        .{
            .name = "wasm_fileio",
            .run = true,
            .wasm = true,
            .wasm_has_resources = true,
        },
    };

    for (test_configs) |config| {
        const test_source: []const u8 = b.pathJoin(&.{ "tests", config.name, "main.c" });

        if (config.wasm) {
            const test_resources: ?[]const u8 =
                if (config.wasm_has_resources) b.pathJoin(&.{ "tests", config.name, "data" }) else null;

            const steps = buildOrcaApp(
                b,
                target,
                wasm_sdk_lib,
                wasm_libc_lib,
                gen_header_exe,
                &orca_install_local.step,
                orca_tool_local_path,
                .{
                    .name = config.name,
                    .sources = &.{test_source},
                    .install = "tests",

                    .resource_path = test_resources,
                    .create_run_step = true,
                },
            );

            if (config.run) {
                if (steps.run) |run| {
                    run.addArg("--test");
                    tests.dependOn(&run.step);
                }
            } else {
                tests.dependOn(steps.build_or_bundle);
            }
        } else {
            const test_exe = b.addExecutable(.{
                .name = config.name,
                .root_module = b.createModule(.{
                    .target = target,
                    .optimize = optimize,
                    .link_libc = true,
                }),
            });
            test_exe.addIncludePath(b.path("src"));
            test_exe.addCSourceFiles(.{
                .files = &.{test_source},
                .flags = &.{},
            });
            test_exe.linkLibrary(orca_platform_lib);

            if (target.result.os.tag == .windows) {
                test_exe.linkSystemLibrary("shlwapi");
            }

            const tests_install_opts: Build.Step.InstallArtifact.Options = .{
                .dest_dir = .{ .override = .{ .custom = "tests" } },
            };

            const install: *Build.Step.InstallArtifact = b.addInstallArtifact(test_exe, tests_install_opts);

            const tests_install_dir: Build.InstallDir = .{ .custom = "tests" };

            const install_orca_platform_tests: *Build.Step.InstallArtifact = b.addInstallArtifact(orca_platform_lib, tests_install_opts);
            const install_angle_libs_tests = b.addInstallDirectory(.{ .source_dir = angle_lib_path, .install_dir = tests_install_dir, .install_subdir = "" });
            const install_dawn_libs_tests = b.addInstallDirectory(.{ .source_dir = dawn_lib_path, .install_dir = tests_install_dir, .install_subdir = "" });

            const test_dir_path = b.path(b.pathJoin(&.{ "tests", config.name }));

            if (config.run) {
                const run_test = b.addRunArtifact(test_exe);
                run_test.addPrefixedFileArg("--test-dir=", test_dir_path); // allows tests to access their data files
                run_test.step.dependOn(&install_orca_platform_tests.step);
                run_test.step.dependOn(&install_angle_libs_tests.step);
                run_test.step.dependOn(&install_dawn_libs_tests.step);
                run_test.step.dependOn(&install.step); // causes test exe working dir to be build\tests\ instead of zig-cache

                tests.dependOn(&run_test.step);
            } else {
                tests.dependOn(&install.step);
            }
        }
    }
}
