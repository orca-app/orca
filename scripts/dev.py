import glob
import os
import platform
import re
import urllib.request
import shutil
import subprocess
from zipfile import ZipFile
import tarfile

from . import checksum
from .bindgen import bindgen
from .checksum import dirsum
from .gles_gen import gles_gen
from .log import *
from .utils import pushd, removeall, yeetdir, yeetfile
from .embed_text_files import *
from .version import orca_version

ANGLE_VERSION = "2023-07-05"
MAC_SDK_DIR = "/Library/Developer/CommandLineTools/SDKs/MacOSX.sdk"

def attach_dev_commands(subparsers):
    build_cmd = subparsers.add_parser("build-runtime", help="Build the Orca runtime from source.")
    build_cmd.add_argument("--release", action="store_true", help="compile Orca in release mode (default is debug)")
    build_cmd.set_defaults(func=dev_shellish(build_runtime))

    tool_cmd = subparsers.add_parser("build-tool", help="Build the Orca CLI tool from source.")
    tool_cmd.add_argument("--version", help="embed a version string in the Orca CLI tool (default is git commit hash)")
    tool_cmd.add_argument("--release", action="store_true",
        help="compile Orca CLI tool in release mode (default is debug)")
    tool_cmd.set_defaults(func=dev_shellish(build_tool))

    clean_cmd = subparsers.add_parser("clean", help="Delete all build artifacts and start fresh.")
    clean_cmd.set_defaults(func=dev_shellish(clean))

    install_cmd = subparsers.add_parser("install", help="Install a dev build of the Orca tools into the system Orca directory.")
    install_cmd.set_defaults(func=dev_shellish(install))


def dev_shellish(func):
    source_dir = get_source_root()
    def func_from_source(args):
        os.chdir(source_dir)
        func(args)
    return shellish(func_from_source)


def build_runtime(args):
    ensure_programs()
    ensure_angle()

    build_platform_layer("lib", args.release)
    build_wasm3(args.release)
    build_orca(args.release)
    build_sdk(args.release)

    with open("build/orcaruntime.sum", "w") as f:
        f.write(runtime_checksum())


def runtime_checksum_last():
    try:
        with open("build/orcaruntime.sum", "r") as f:
            return f.read()
    except FileNotFoundError:
        return None


def runtime_checksum():
    if platform.system() == "Windows":
        excluded_dirs = ["src\\tool", "src\\ext\\curl", "src\\ext\\zlib", "src\\ext\\microtar"]
    elif platform.system() == "Darwin":
        excluded_dirs = ["src/tool", "src/ext/curl", "src/ext/zlib", "src/ext/microtar"]
    else:
        log_error(f"can't generate runtime_checksum for unknown platform '{platform.system()}'")
        exit(1)
    return dirsum("src", excluded_dirs=excluded_dirs)


def tool_checksum():
    tool_dir = "src\\tool" if platform.system() == "Windows" else "src/tool"
    return dirsum(tool_dir)


def tool_checksum_last():
    try:
        with open("build/orcatool.sum", "r") as f:
            return f.read()
    except FileNotFoundError:
        return None


def clean(args):
    yeetdir("build")
    yeetdir("src/ext/angle")
    yeetdir("scripts/files")
    yeetdir("scripts/__pycache__")


def build_platform_layer(target, release):
    print("Building Orca platform layer...")

    os.makedirs("build/bin", exist_ok=True)
    os.makedirs("build/lib", exist_ok=True)

    if target == "lib":
        if platform.system() == "Windows":
            build_platform_layer_lib_win(release)
        elif platform.system() == "Darwin":
            build_platform_layer_lib_mac(release)
        else:
            log_error(f"can't build platform layer for unknown platform '{platform.system()}'")
            exit(1)
    elif target == "test":
        with pushd("examples/test_app"):
            # TODO?
            subprocess.run(["./build.sh"])
    elif target == "clean":
        removeall("bin")
    else:
        log_error(f"unrecognized platform layer target '{target}'")
        exit(1)


def build_platform_layer_lib_win(release):
    embed_text_files("src\\graphics\\glsl_shaders.h", "glsl_", [
        "src\\graphics\\glsl_shaders\\common.glsl",
        "src\\graphics\\glsl_shaders\\blit_vertex.glsl",
        "src\\graphics\\glsl_shaders\\blit_fragment.glsl",
        "src\\graphics\\glsl_shaders\\path_setup.glsl",
        "src\\graphics\\glsl_shaders\\segment_setup.glsl",
        "src\\graphics\\glsl_shaders\\backprop.glsl",
        "src\\graphics\\glsl_shaders\\merge.glsl",
        "src\\graphics\\glsl_shaders\\raster.glsl",
        "src\\graphics\\glsl_shaders\\balance_workgroups.glsl",
    ])

    includes = [
        "/I", "src",
        "/I", "src/ext",
        "/I", "src/ext/angle/include",
    ]
    libs = [
        "user32.lib",
        "opengl32.lib",
        "gdi32.lib",
        "shcore.lib",
        "delayimp.lib",
        "dwmapi.lib",
        "comctl32.lib",
        "ole32.lib",
        "shell32.lib",
        "shlwapi.lib",
        "dxgi.lib",
        "dxguid.lib",
        "/LIBPATH:build/lib",
        "libEGL.dll.lib",
        "libGLESv2.dll.lib",
        "/DELAYLOAD:libEGL.dll",
        "/DELAYLOAD:libGLESv2.dll",
    ]

    subprocess.run([
        "cl", "/nologo",
        "/we4013", "/Zi", "/Zc:preprocessor",
        "/DOC_BUILD_DLL",
        "/std:c11", "/experimental:c11atomics",
        *includes,
        "src/orca.c", "/Fo:build/bin/orca.o",
        "/LD", "/link",
        "/MANIFEST:EMBED", "/MANIFESTINPUT:src/app/win32_manifest.xml",
        *libs,
        "/OUT:build/bin/orca.dll",
        "/IMPLIB:build/bin/orca.dll.lib",
    ], check=True)

def build_platform_layer_lib_mac(release):
    flags = ["-mmacos-version-min=10.15.4"]
    cflags = ["-std=c11"]
    debug_flags = ["-O3"] if release else ["-g", "-DOC_DEBUG", "-DOC_LOG_COMPILE_DEBUG"]
    ldflags = [f"-L{MAC_SDK_DIR}/usr/lib", f"-F{MAC_SDK_DIR}/System/Library/Frameworks/"]
    includes = ["-Isrc", "-Isrc/util", "-Isrc/platform", "-Isrc/ext", "-Isrc/ext/angle/include"]

    # compile metal shader
    subprocess.run([
        "xcrun", "-sdk", "macosx", "metal",
        # TODO: shaderFlagParam
        "-fno-fast-math", "-c",
        "-o", "build/mtl_renderer.air",
        "src/graphics/mtl_renderer.metal",
    ], check=True)
    subprocess.run([
        "xcrun", "-sdk", "macosx", "metallib",
        "-o", "build/bin/mtl_renderer.metallib",
        "build/mtl_renderer.air",
    ], check=True)

    # compile platform layer. We use one compilation unit for all C code, and one
    # compilation unit for all Objective-C code
    subprocess.run([
        "clang",
        *debug_flags, "-c",
        "-o", "build/orca_c.o",
        *cflags, *flags, *includes,
        "src/orca.c"
    ], check=True)
    subprocess.run([
        "clang",
        *debug_flags, "-c",
        "-o", "build/orca_objc.o",
        *flags, *includes,
        "src/orca.m"
    ], check=True)

    # build dynamic library
    subprocess.run([
        "ld",
        *ldflags, "-dylib",
        "-o", "build/bin/liborca.dylib",
        "build/orca_c.o", "build/orca_objc.o",
        "-Lbuild/bin", "-lc",
        "-framework", "Carbon", "-framework", "Cocoa", "-framework", "Metal", "-framework", "QuartzCore",
        "-weak-lEGL", "-weak-lGLESv2",
    ], check=True)

    # change dependent libs path to @rpath
    subprocess.run([
        "install_name_tool",
        "-change", "./libEGL.dylib", "@rpath/libEGL.dylib",
        "build/bin/liborca.dylib",
    ], check=True)
    subprocess.run([
        "install_name_tool",
        "-change", "./libGLESv2.dylib", "@rpath/libGLESv2.dylib",
        "build/bin/liborca.dylib",
    ], check=True)

    # add executable path to rpath. Client executable can still add its own
    # rpaths if needed, e.g. @executable_path/libs/ etc.
    subprocess.run([
        "install_name_tool",
        "-id", "@rpath/liborca.dylib",
        "build/bin/liborca.dylib",
    ], check=True)


def build_wasm3(release):
    print("Building wasm3...")

    os.makedirs("build/bin", exist_ok=True)
    os.makedirs("build/lib", exist_ok=True)
    os.makedirs("build/obj", exist_ok=True)

    if platform.system() == "Windows":
        build_wasm3_lib_win(release)
    elif platform.system() == "Darwin":
        build_wasm3_lib_mac(release)
    else:
        log_error(f"can't build wasm3 for unknown platform '{platform.system()}'")
        exit(1)


def build_wasm3_lib_win(release):
    for f in glob.iglob("./src/ext/wasm3/source/*.c"):
        name = os.path.splitext(os.path.basename(f))[0]
        subprocess.run([
            "cl", "/nologo",
            "/Zi", "/Zc:preprocessor", "/c",
            "/O2",
            f"/Fo:build/obj/{name}.obj",
            "/I", "./src/ext/wasm3/source",
            f,
        ], check=True)
    subprocess.run([
        "lib", "/nologo", "/out:build/bin/wasm3.lib",
        "build/obj/*.obj",
    ], check=True)


def build_wasm3_lib_mac(release):
    includes = ["-Isrc/ext/wasm3/source"]
    debug_flags = ["-g", "-O2"]
    flags = [
        *debug_flags,
        "-foptimize-sibling-calls",
        "-Wno-extern-initializer",
        "-Dd_m3VerboseErrorMessages",
        "-mmacos-version-min=10.15.4"
    ]

    for f in glob.iglob("src/ext/wasm3/source/*.c"):
        name = os.path.splitext(os.path.basename(f))[0] + ".o"
        subprocess.run([
            "clang", "-c", *flags, *includes,
            "-o", f"build/obj/{name}",
            f,
        ], check=True)
    subprocess.run(["libtool", "-static", "-o", "build/lib/libwasm3.a", "-no_warning_for_no_symbols", *glob.glob("build/obj/*.o")], check=True)
    subprocess.run(["rm", "-rf", "build/obj"], check=True)


def build_orca(release):
    print("Building Orca runtime...")

    os.makedirs("build/bin", exist_ok=True)
    os.makedirs("build/lib", exist_ok=True)

    if platform.system() == "Windows":
        build_orca_win(release)
    elif platform.system() == "Darwin":
        build_orca_mac(release)
    else:
        log_error(f"can't build Orca for unknown platform '{platform.system()}'")
        exit(1)


def build_orca_win(release):

    gen_all_bindings()

    # compile orca
    includes = [
        "/I", "src",
        "/I", "src/ext",
        "/I", "src/ext/angle/include",
        "/I", "src/ext/wasm3/source",
    ]

    subprocess.run([
        "cl",
        "/c",
        "/Zi", "/Zc:preprocessor",
        "/std:c11", "/experimental:c11atomics",
        *includes,
        "src/runtime.c",
        "/Fo:build/bin/runtime.obj",
    ], check=True)

def build_orca_mac(release):

    includes = [
        "-Isrc",
        "-Isrc/ext",
        "-Isrc/ext/angle/include",
        "-Isrc/ext/wasm3/source"
    ]
    libs = ["-Lbuild/bin", "-Lbuild/lib", "-lorca", "-lwasm3"]
    debug_flags = ["-O2"] if release else ["-g", "-DOC_DEBUG -DOC_LOG_COMPILE_DEBUG"]
    flags = [
        *debug_flags,
        "-mmacos-version-min=10.15.4"]

    gen_all_bindings()

    # compile orca
    subprocess.run([
        "clang", *flags, *includes, *libs,
        "-o", "build/bin/orca_runtime",
        "src/runtime.c",
    ], check=True)

    # fix libs imports
    subprocess.run([
        "install_name_tool",
        "-change", "build/bin/liborca.dylib", "@rpath/liborca.dylib",
        "build/bin/orca_runtime",
    ], check=True)
    subprocess.run([
        "install_name_tool",
        "-add_rpath", "@executable_path/",
        "build/bin/orca_runtime",
    ], check=True)


def gen_all_bindings():
    gles_gen("src/ext/gl.xml",
        "src/wasmbind/gles_api.json",
        "src/graphics/orca_gl31.h",
        log_file='./build/gles_gen.log'
    )

    bindgen("gles", "src/wasmbind/gles_api.json",
        wasm3_bindings="src/wasmbind/gles_api_bind_gen.c",
    )

    bindgen("core", "src/wasmbind/core_api.json",
        guest_stubs="src/wasmbind/core_api_stubs.c",
        wasm3_bindings="src/wasmbind/core_api_bind_gen.c",
    )

    bindgen("surface", "src/wasmbind/surface_api.json",
        guest_stubs="src/graphics/orca_surface_stubs.c",
        guest_include="graphics/graphics.h",
        wasm3_bindings="src/wasmbind/surface_api_bind_gen.c",
    )

    bindgen("clock", "src/wasmbind/clock_api.json",
        guest_include="platform/platform_clock.h",
        wasm3_bindings="src/wasmbind/clock_api_bind_gen.c",
    )
    bindgen("io", "src/wasmbind/io_api.json",
        guest_include="platform/platform_io_dialog.h",
        guest_stubs="src/platform/orca_io_stubs.c",
        wasm3_bindings="src/wasmbind/io_api_bind_gen.c",
    )


def build_sdk(release):
    print("Building Orca wasm SDK...")
    debug_flags = ["-O2"] if release else ["-g", "-DOC_DEBUG -DOC_LOG_COMPILE_DEBUG"]
    flags = [
        *debug_flags,
        "--target=wasm32",
        "--no-standard-libraries",
        "-mbulk-memory",
        "-D__ORCA__",
        "-Wl,--no-entry",
        "-Wl,--export-dynamic",
        "-Wl,--relocatable"]

    includes = [
        "-isystem", "src/libc-shim/include",
        "-I", "src",
        "-I", "src/ext"]

    libc_src = glob.glob("src/libc-shim/src/*.c")

    clang = 'clang'
    if platform.system() == "Darwin":
        try:
            brew_llvm = subprocess.check_output(["brew", "--prefix", "llvm@15", "--installed"]).decode().strip()
        except subprocess.CalledProcessError:
            brew_llvm = subprocess.check_output(["brew", "--prefix", "llvm", "--installed"]).decode().strip()
        clang = os.path.join(brew_llvm, 'bin', 'clang')

    # compile sdk
    subprocess.run([
        clang, *flags, *includes,
        "-o", "build/bin/liborca_wasm.a",
        "src/orca.c", *libc_src,
    ], check=True)

def ensure_programs():
    if platform.system() == "Windows":
        MSVC_MAJOR, MSVC_MINOR = 19, 35
        try:
            cl_only = subprocess.run(["cl"], capture_output=True, text=True)
            desc = cl_only.stderr.splitlines()[0]

            detect = subprocess.run(["cl", "/EP", "scripts\\msvc_version.txt"], capture_output=True, text=True)
            parts = [x for x in detect.stdout.splitlines() if x]
            version, arch = int(parts[0]), parts[1]
            major, minor = int(version / 100), version % 100

            if arch != "x64":
                msg = log_error("MSVC is not running in 64-bit mode. Make sure you are running in")
                msg.more("an x64 Visual Studio command prompt, such as the \"x64 Native Tools")
                msg.more("Command Prompt\" from your Start Menu.")
                msg.more()
                msg.more("MSVC reported itself as:")
                msg.more(desc)
                exit(1)

            if major < MSVC_MAJOR or minor < MSVC_MINOR:
                msg = log_error(f"Your version of MSVC is too old. You have version {major}.{minor},")
                msg.more(f"but version {MSVC_MAJOR}.{MSVC_MINOR} or greater is required.")
                msg.more()
                msg.more("MSVC reported itself as:")
                msg.more(desc)
                msg.more()
                msg.more("Please update Visual Studio to the latest version and try again.")
                exit(1)
        except FileNotFoundError:
            msg = log_error("MSVC was not found on your system.")
            msg.more("If you have already installed Visual Studio, make sure you are running in an")
            msg.more("x64 Visual Studio command prompt, such as the \"x64 Native Tools Command")
            msg.more("Prompt\" from your Start Menu. Otherwise, download and install Visual Studio,")
            msg.more("and ensure that your installation includes \"Desktop development with C++\"")
            msg.more("and \"C++ Clang Compiler\": https://visualstudio.microsoft.com/")
            exit(1)

    if platform.system() == "Darwin":
        try:
            subprocess.run(["clang", "-v"], stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
        except FileNotFoundError:
            msg = log_error("clang was not found on your system.")
            msg.more("Run the following to install it:")
            msg.more()
            msg.more("  brew install llvm")
            msg.more()
            exit(1)

        if not os.path.exists(MAC_SDK_DIR):
            msg = log_error("The Xcode command-line tools are not installed.")
            msg.more("Run the following to install them:")
            msg.more()
            msg.more("  xcode-select --install")
            msg.more()
            exit(1)


def ensure_angle():
    if not verify_angle():
        download_angle()
        print("Verifying ANGLE download...")
        if not verify_angle():
            log_error("automatic ANGLE download failed")
            exit(1)


def verify_angle():
    checkfiles = None
    if platform.system() == "Windows":
        checkfiles = [
            "build/bin/libEGL.dll",
            "build/lib/libEGL.dll.lib",
            "build/bin/libGLESv2.dll",
            "build/lib/libGLESv2.dll.lib",
        ]
    elif platform.system() == "Darwin":
        checkfiles = [
            "build/bin/libEGL.dylib",
            "build/bin/libGLESv2.dylib",
        ]

    if checkfiles is None:
        log_warning("could not verify if the correct version of ANGLE is present")
        return False

    ok = True
    for file in checkfiles:
        if not os.path.isfile(file):
            ok = False
            continue
        if not checksum.checkfile(file):
            ok = False
            continue

    return ok


def download_angle():
    print("Downloading ANGLE...")
    if platform.system() == "Windows":
        build = "windows-2019"
    elif platform.system() == "Darwin":
        build = "macos-jank"
    else:
        log_error(f"could not automatically download ANGLE for unknown platform {platform.system()}")
        return

    os.makedirs("scripts/files", exist_ok=True)
    filename = f"angle-{build}-{ANGLE_VERSION}.zip"
    filepath = f"scripts/files/{filename}"
    url = f"https://github.com/HandmadeNetwork/build-angle/releases/download/{ANGLE_VERSION}/{filename}"
    with urllib.request.urlopen(url) as response:
        with open(filepath, "wb") as out:
            shutil.copyfileobj(response, out)

    if not checksum.checkfile(filepath):
        log_error(f"ANGLE download did not match checksum")
        exit(1)

    print("Extracting ANGLE...")
    with ZipFile(filepath, "r") as anglezip:
        anglezip.extractall(path="scripts/files")

    shutil.copytree(f"scripts/files/angle/include", "src/ext/angle/include", dirs_exist_ok=True)
    shutil.copytree(f"scripts/files/angle/bin", "build/bin", dirs_exist_ok=True)
    if platform.system() == "Windows":
        shutil.copytree(f"scripts/files/angle/lib", "build/lib", dirs_exist_ok=True)

def build_libcurl():
    if platform.system() == "Windows":
        if not os.path.exists("src/ext/curl/builds/static/"):
            print("Building libcurl...")
            with pushd("src/ext/curl/winbuild"):
                subprocess.run("nmake /f Makefile.vc mode=static MACHINE=x64", check=True)
            shutil.copytree(
                "src/ext/curl/builds/libcurl-vc-x64-release-static-ipv6-sspi-schannel/",
                "src/ext/curl/builds/static",
                dirs_exist_ok=True)

    elif platform.system() == "Darwin":
        if not os.path.exists("src/ext/curl/builds/static/"):
            print("Building libcurl...")
            os.makedirs("src/ext/curl/builds", exist_ok=True)
            with pushd("src/ext/curl/builds"):
                prefix = os.path.join(os.getcwd(), "static")
                subprocess.run([
                    "../configure",
                    "--with-secure-transport",
                    "--disable-shared",
                    "--disable-ldap", "--disable-ldaps", "--disable-aws",
                    "--disable-manual", "--disable-debug",
                    "--disable-dependency-tracking",
                    "--without-brotli", "--without-zstd", "--without-libpsl",
                    "--without-librtmp", "--without-zlib", "--without-nghttp2",
                    "--without-libidn2",
                    f"--prefix={prefix}",
                ] , check=True)
                subprocess.run("make", check=True)
                subprocess.run(["make", "install"], check=True)

    else:
        log_error(f"can't build libcurl for unknown platform '{platform.system()}'")
        exit(1)


def build_zlib():
    if platform.system() == "Windows":
        if not os.path.exists("src/ext/zlib/build/zlib.lib"):
            print("Building zlib...")
            os.makedirs("src/ext/zlib/build", exist_ok=True)
            with pushd("src/ext/zlib/build"):
                subprocess.run("nmake /f ../win32/Makefile.msc TOP=.. zlib.lib", check=True)

    elif platform.system() == "Darwin":
        if not os.path.exists("src/ext/zlib/build/libz.a"):
            print("Building zlib...")
            os.makedirs("src/ext/zlib/build", exist_ok=True)
            with pushd("src/ext/zlib/build"):
                subprocess.run(["../configure", "--static"], check=True)
                subprocess.run(["make", "libz.a"], check=True)

    else:
        log_error(f"can't build zlib for unknown platform '{platform.system()}'")
        exit(1)


def build_tool_win(release, version, outname):
    includes = [
        "/I", "..",
        "/I", "../ext/stb",
        "/I", "../ext/curl/builds/static/include",
        "/I", "../ext/zlib",
        "/I", "../ext/microtar"
    ]

    # debug_flags = ["/O2"] if release else ["/Zi", "/DOC_DEBUG", "/DOC_LOG_COMPILE_DEBUG", "/W3"]
    debug_flags = ["/O2"] if release else ["/Zi", "/DOC_DEBUG", "/DOC_LOG_COMPILE_DEBUG"]

    libs = [
        "shlwapi.lib",
        "shell32.lib",
        "ole32.lib",

        # libs needed by curl
        "advapi32.lib",
        "crypt32.lib",
        "normaliz.lib",
        "ws2_32.lib",
        "wldap32.lib",
        "/LIBPATH:../ext/curl/builds/static/lib",
        "libcurl_a.lib",

        "/LIBPATH:../ext/zlib/build",
        "zlib.lib",
    ]

    subprocess.run([
        "cl",
        "/nologo",
        "/Zc:preprocessor",
        "/std:c11", "/experimental:c11atomics",
        *debug_flags,
        *includes,
        "/DFLAG_IMPLEMENTATION",
        "/DOC_NO_APP_LAYER",
        "/DOC_BUILD_DLL",
        "/DCURL_STATICLIB",
        f"/DORCA_TOOL_VERSION={version}",
        "/MD",
        f"/Febuild/bin/{outname}",
        "main.c",
        "/link",
        *libs,
    ], check=True)

def build_tool_mac(release, version, outname):
    includes = [
        "-I", "..",
        "-I", "../ext/curl/builds/static/include",
        "-I", "../ext/zlib",
        "-I", "../ext/microtar"
    ]

    debug_flags = ["-O2"] if release else ["-g", "-DOC_DEBUG", "-DOC_LOG_COMPILE_DEBUG"]

    libs = [
        "-framework", "Cocoa",

        # libs needed by curl
        "-framework", "SystemConfiguration",
        "-framework", "CoreFoundation",
        "-framework", "CoreServices",
        "-framework", "SystemConfiguration",
        "-framework", "Security",
        "-L../ext/curl/builds/static/lib", "-lcurl",

        "-L../ext/zlib/build", "-lz",
    ]
    subprocess.run([
        "clang",
        "-mmacos-version-min=10.15.4",
        "-std=c11",
        *debug_flags,
        *includes,
        "-D", "FLAG_IMPLEMENTATION",
        "-D", "OC_NO_APP_LAYER",
        "-D", "OC_BUILD_DLL",
        "-D", "CURL_STATICLIB",
        "-D", f"ORCA_TOOL_VERSION={version}",
        *libs,
        "-MJ", "build/main.json",
        "-o", f"build/bin/{outname}",
        "main.c",
    ], check=True)

    with open("build/compile_commands.json", "w") as f:
        f.write("[\n")
        with open("build/main.json") as m:
            f.write(m.read())
        f.write("]")

def build_tool(args):
    print("Building Orca CLI tool...")

    ensure_programs()
    build_libcurl()
    build_zlib()

    os.makedirs("build/bin", exist_ok=True)

    with pushd("src/tool"):
        os.makedirs("build/bin", exist_ok=True)

        if args.version == None:
            res = subprocess.run(["git", "rev-parse", "--short", "HEAD"], check=True, capture_output=True, text=True)
            version = res.stdout.strip()
        else:
            version = args.version

        outname = "orca.exe" if platform.system() == "Windows" else "orca"

        if platform.system() == "Windows":
            build_tool_win(args.release, version, outname)
        elif platform.system() == "Darwin":
            build_tool_mac(args.release, version, outname)
        else:
            log_error(f"can't build cli tool for unknown platform '{platform.system()}'")
            exit(1)

    shutil.copy(f"src/tool/build/bin/{outname}", "build/bin/")

    with open("build/orcatool.sum", "w") as f:
        f.write(tool_checksum())

def prompt(msg):
    while True:
        answer = input(f"{msg} (y/n)> ")
        if answer.lower() in ["y", "yes"]:
            return True
        elif answer.lower() in ["n", "no"]:
            return False
        else:
            print("Please enter \"yes\" or \"no\" and press return.")


def system_orca_dir():
    try:
        res = subprocess.run(["orca", "install-path"], check=True, capture_output=True, text=True)
        install_path = res.stdout.strip()
        return install_path
    except subprocess.CalledProcessError:
        print("You must install the Orca cli tool and add the directory where you")
        print("installed it to your PATH before the dev tooling can determine the")
        print("system Orca directory. You can download the cli tool from:")
        print("https://github.com/orca-app/orca/releases/latest")
        exit(1)

def install(args):
    if runtime_checksum_last() is None:
        print("You must build the Orca runtime before you can install it to your")
        print("system. Please run the following command first:")
        print()
        print("orcadev build-runtime")
        exit(1)

    if runtime_checksum() != runtime_checksum_last():
        print("Your build of the Orca runtime is out of date. We recommend that you")
        print("rebuild the runtime first with `orcadev build-runtime`.")
        if not prompt("Do you wish to install the runtime anyway?"):
            return
        print()

    if tool_checksum_last() is None:
        print("You must build the Orca tool before you can install it to your")
        print("system. Please run the following command first:")
        print()
        print("orcadev build-tool")
        exit(1)

    if tool_checksum() != tool_checksum_last():
        print("Your build of the Orca tool is out of date. We recommend that you")
        print("rebuild the tool first with `orcadev build-tool`.")
        if not prompt("Do you wish to install the tool anyway?"):
            return
        print()

    orca_dir = system_orca_dir()
    version = orca_version()
    dest = os.path.join(orca_dir, version)

    bin_dir = os.path.join(dest, "bin")
    libc_dir = os.path.join(dest, "orca-libc")
    res_dir = os.path.join(dest, "resources")
    src_dir = os.path.join(dest, "src")

    yeetdir(dest)
    os.makedirs(bin_dir, exist_ok=True)
    os.makedirs(libc_dir, exist_ok=True)
    os.makedirs(res_dir, exist_ok=True)
    os.makedirs(src_dir, exist_ok=True)

    tool_path = "build\\bin\\orca.exe" if platform.system() == "Windows" else "build/bin/orca"

    shutil.copy(tool_path, bin_dir)
    shutil.copytree("src", src_dir, dirs_exist_ok=True)
    shutil.copytree("resources", res_dir, dirs_exist_ok=True)
    if platform.system() == "Windows":
        shutil.copy("build\\bin\\orca.dll", bin_dir)
        shutil.copy("build\\bin\\orca.dll.lib", bin_dir)
        shutil.copy("build\\bin\\wasm3.lib", bin_dir)
        shutil.copy("build\\bin\\runtime.obj", bin_dir)
        shutil.copy("build\\bin\\libEGL.dll", bin_dir)
        shutil.copy("build\\bin\\libGLESv2.dll", bin_dir)
    else:
        shutil.copy("build/bin/liborca.dylib", bin_dir)
        shutil.copy("build/bin/mtl_renderer.metallib", bin_dir)
        shutil.copy("build/bin/orca_runtime", bin_dir)
        shutil.copy("build/bin/libEGL.dylib", bin_dir)
        shutil.copy("build/bin/libGLESv2.dylib", bin_dir)

    shutil.copy(tool_path, orca_dir)

    with open(os.path.join(orca_dir, "current_version"), "w") as f:
        f.write(version)

    # TODO(shaw): should dev versions and their checksums be added to all_versions file?

    print()
    print("A dev build of Orca has been installed to the following directory:")
    print(dest)
    print()


# Gets the root directory of the current Orca source checkout.
# This is copy-pasted to the command-line tool so it can work before loading anything.
def get_source_root():
    dir = os.getcwd()
    while True:
        try:
            os.stat(os.path.join(dir, ".orcasource"))
            return dir
        except FileNotFoundError:
            pass

        newdir = os.path.dirname(dir)
        if newdir == dir:
            raise Exception(f"Directory {os.getcwd()} does not seem to be part of the Orca source code.")
        dir = newdir


def yeet(path):
    os.makedirs(path, exist_ok=True)
    shutil.rmtree(path)
