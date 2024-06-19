import glob
import os
import sys
import platform
import re
import urllib.request
import shutil
import subprocess
from zipfile import ZipFile
import tarfile
import json

from . import checksum
from .bindgen import bindgen
from .checksum import dirsum
from .gles_gen import gles_gen
from .log import *
from .utils import pushd, removeall, yeetdir, yeetfile
from .embed_text_files import *
from .embed_shaders import *

ANGLE_VERSION = "2023-07-05"
MAC_SDK_DIR = "/Library/Developer/CommandLineTools/SDKs/MacOSX.sdk"
MACOS_VERSION_MIN = "13.0.0"

def attach_dev_commands(subparsers):
    build_cmd = subparsers.add_parser("build", help="Build Orca from source.")
    build_cmd.add_argument("--version", help="embed a version string in the Orca CLI tool (default is git commit hash)")
    build_cmd.add_argument("--release", action="store_true", help="compile in release mode (default is debug)")
    build_cmd.add_argument("--wasm-backend", help="specify a wasm backend. Options: wasm3 (default), bytebox")
    build_cmd.set_defaults(func=dev_shellish(build_all))

    platform_layer_cmd = subparsers.add_parser("build-platform-layer", help="Build the Orca platform layer from source.")
    platform_layer_cmd.add_argument("--release", action="store_true", help="compile in release mode (default is debug)")
    platform_layer_cmd.set_defaults(func=dev_shellish(build_platform_layer))

    dawn_cmd = subparsers.add_parser("build-dawn", help="Build Dawn")
    dawn_cmd.add_argument("--release", action="store_true", help="compile in release mode (default is debug)")
    dawn_cmd.add_argument("--force", action="store_true", help="force rebuild even if up-to-date artifacts exist")
    dawn_cmd.add_argument("--parallel", type=int)
    dawn_cmd.set_defaults(func=dev_shellish(build_dawn))

    angle_cmd = subparsers.add_parser("build-angle", help="Build Angle")
    angle_cmd.add_argument("--release", action="store_true", help="compile in release mode (default is debug)")
    angle_cmd.add_argument("--force", action="store_true", help="force rebuild even if up-to-date artifacts exist")
    angle_cmd.set_defaults(func=dev_shellish(build_angle))

    runtime_cmd = subparsers.add_parser("build-runtime", help="Build the Orca runtime from source.")
    runtime_cmd.add_argument("--release", action="store_true", help="compile in release mode (default is debug)")
    runtime_cmd.add_argument("--wasm-backend", help="specify a wasm backend. Options: wasm3 (default), bytebox")
    runtime_cmd.set_defaults(func=dev_shellish(build_runtime))

    libc_cmd = subparsers.add_parser("build-orca-libc", help="Build the Orca libC from source.")
    libc_cmd.add_argument("--release", action="store_true", help="compile in release mode (default is debug)")
    libc_cmd.set_defaults(func=dev_shellish(build_libc))

    sdk_cmd = subparsers.add_parser("build-wasm-sdk", help="Build the Orca wasm sdk from source.")
    sdk_cmd.add_argument("--release", action="store_true", help="compile in release mode (default is debug)")
    sdk_cmd.set_defaults(func=dev_shellish(build_sdk))

    tool_cmd = subparsers.add_parser("build-tool", help="Build the Orca CLI tool from source.")
    tool_cmd.add_argument("--version", help="embed a version string in the Orca CLI tool (default is git commit hash)")
    tool_cmd.add_argument("--release", action="store_true", help="compile Orca CLI tool in release mode (default is debug)")
    tool_cmd.set_defaults(func=dev_shellish(build_tool))

    package_cmd = subparsers.add_parser("package-sdk", help="Packages the Orca SDK for a release.")
    package_cmd.add_argument("--target")
    package_cmd.add_argument("dest")
    package_cmd.set_defaults(func=dev_shellish(package_sdk))

    install_cmd = subparsers.add_parser("install", help="Install a dev build of the Orca tools into the system Orca directory.")
    install_cmd.add_argument("--version")
    install_cmd.add_argument("install_dir", nargs='?')
    install_cmd.set_defaults(func=dev_shellish(install))

    clean_cmd = subparsers.add_parser("clean", help="Delete all build artifacts and start fresh.")
    clean_cmd.set_defaults(func=dev_shellish(clean))

def dev_shellish(func):
    source_dir = get_source_root()
    def func_from_source(args):
        os.chdir(source_dir)
        func(args)
    return shellish(func_from_source)


def build_all(args):
    ensure_programs()

    build_runtime_internal(args.release, args.wasm_backend) # this also builds the platform layer
    build_libc_internal(args.release)
    build_sdk_internal(args.release)
    build_tool(args)

    with open("build/orcaruntime.sum", "w") as f:
        f.write(runtime_checksum())


#------------------------------------------------------
# build dawn
#------------------------------------------------------

def dawn_required_commit():
    with open("deps/dawn-commit.txt", "r") as f:
        DAWN_COMMIT = f.read().strip() # hardcoded commit for now
    return DAWN_COMMIT

def dawn_required_files():
    artifacts = []
    if platform.system() == "Windows":
        artifacts = ["bin/webgpu.lib", "bin/webgpu.dll", "include/webgpu.h"]
    else:
        artifacts = ["bin/libwebgpu.dylib", "include/webgpu.h"]

    return artifacts

def check_dawn():

    DAWN_COMMIT = dawn_required_commit()
    artifacts = dawn_required_files()

    up_to_date = False
    messages = []


    if os.path.exists("build/dawn.out/dawn.json"):
        with pushd("build/dawn.out"):
            with open("dawn.json", "r") as f:
                sums = json.loads(f.read())

                up_to_date = True

                for artifact in artifacts:
                    if artifact in sums:
                        if os.path.isfile(artifact):
                            s = checksum.filesum(artifact)
                            if sums[artifact]['commit'] != DAWN_COMMIT:
                                messages.append(f"build/dawn.out/{artifact} doesn't match dawn commit.\n  note: expected {DAWN_COMMIT}, got {sums[artifact]['commit']}")
                                up_to_date = False
                            elif s != sums[artifact]['sum']:
                                messages.append(f"build/dawn.out/{artifact} doesn't match checksum.\n  note: expected {sums[artifact]['sum']}, got {s}")
                                up_to_date = False
                        else:
                            messages.append(f"build/dawn.out/{artifact} not found")
                            up_to_date = False
                            break
                    else:
                        messages.append(f"build/dawn.out/{artifact} is not listed in checksum file")
                        up_to_date = False
    else:
        messages = ["build/dawn.out/dawn.json not found"]

    if up_to_date:
        os.makedirs("src/ext/dawn/include", exist_ok=True)
        shutil.copytree("build/dawn.out/include", "src/ext/dawn/include/", dirs_exist_ok=True)

        os.makedirs("build/bin", exist_ok=True)
        shutil.copytree("build/dawn.out/bin", "build/bin", dirs_exist_ok=True)

    return (up_to_date, messages)


def build_dawn(args):
    ensure_programs()
    build_dawn_internal(args.release, args.parallel, args.force)

def build_dawn_internal(release, jobs, force):
    print("Building Dawn...")

    os.makedirs("build/bin", exist_ok=True)

    # TODO ensure requirements

    DAWN_COMMIT = dawn_required_commit()

    # check if we already have the binary
    if not force:
        dawn_ok, _ = check_dawn()
        if dawn_ok:
            print("  * already up to date")
            print("Done")
            return

    with pushd("build"):
        # get depot tools repo
        print("  * checking depot tools")
        if not os.path.exists("depot_tools"):
            subprocess.run([
                "git", "clone", "--depth=1", "--no-tags", "--single-branch",
                "https://chromium.googlesource.com/chromium/tools/depot_tools.git"
            ], check=True)
        os.environ["PATH"] = os.path.join(os.getcwd(), "depot_tools") + os.pathsep + os.environ["PATH"]
        os.environ["DEPOT_TOOLS_WIN_TOOLCHAIN"] = "0"

        # get dawn repo
        print("  * checking dawn")
        if not os.path.exists("dawn"):
            subprocess.run([
                "git", "clone", "--no-tags", "--single-branch",
                "https://dawn.googlesource.com/dawn"
            ], check=True)

        with pushd("dawn"):
            subprocess.run([
                "git", "restore", "."
            ], check=True)

            subprocess.run([
                "git", "pull", "--force", "--no-tags"
            ], check=True)

            subprocess.run([
                "git", "checkout", "--force", DAWN_COMMIT
            ], check=True)

            shutil.copy("scripts/standalone.gclient", ".gclient")

            shell = True if platform.system() == "Windows" else False
            subprocess.run(["gclient", "sync"], shell=shell, check=True)

        print("  * preparing build")

        with open("dawn/src/dawn/native/CMakeLists.txt", "a") as f:
            s = """add_library(webgpu SHARED ${DAWN_PLACEHOLDER_FILE})
common_compile_options(webgpu)
target_link_libraries(webgpu PRIVATE dawn_native)
target_link_libraries(webgpu PUBLIC dawn_headers)
target_compile_definitions(webgpu PRIVATE WGPU_IMPLEMENTATION WGPU_SHARED_LIBRARY)
target_sources(webgpu PRIVATE ${WEBGPU_DAWN_NATIVE_PROC_GEN})"""
            f.write(s)

        # apply windows patch
        with pushd("dawn"):
            subprocess.run(["git", "apply", "-v", "../../deps/dawn-d3d12-transparent.diff"], check=True)

        mode = "Release" if release else "Debug"

        if platform.system() == "Windows":
            backends = [
                "-D", "DAWN_ENABLE_D3D12=ON",
                "-D", "DAWN_ENABLE_D3D11=OFF",
                "-D", "DAWN_ENABLE_METAL=OFF",
                "-D", "DAWN_ENABLE_NULL=OFF",
                "-D", "DAWN_ENABLE_DESKTOP_GL=OFF",
                "-D", "DAWN_ENABLE_OPENGLES=OFF",
                "-D", "DAWN_ENABLE_VULKAN=OFF"
            ]
        else:
            backends = [
                "-D", "DAWN_ENABLE_METAL=ON",
                "-D", "DAWN_ENABLE_NULL=OFF",
                "-D", "DAWN_ENABLE_DESKTOP_GL=OFF",
                "-D", "DAWN_ENABLE_OPENGLES=OFF",
                "-D", "DAWN_ENABLE_VULKAN=OFF"
            ]

        subprocess.run([
            "cmake",
            "-S", "dawn",
            "-B", "dawn.build",
            "-D", f"CMAKE_BUILD_TYPE={mode}",
            "-D", "CMAKE_POLICY_DEFAULT_CMP0091=NEW",
            "-D", "BUILD_SHARED_LIBS=OFF",
            "-D", "BUILD_SAMPLES=ON",
            *backends,
            "-D", "DAWN_BUILD_SAMPLES=ON",
            "-D", "TINT_BUILD_SAMPLES=OFF",
            "-D", "TINT_BUILD_DOCS=OFF",
            "-D", "TINT_BUILD_TESTS=OFF",
            "-D", "TINT_BUILD_CMD_TOOLS=ON",
            "-D", "TINT_BUILD_SPV_READER=ON",
            "-D", "TINT_BUILD_SPV_WRITER=ON",
        ], check = True)

        parallel = ["--parallel"]
        if jobs != None:
            parallel.append(str(jobs))

        print("  * building")
        subprocess.run([
            "cmake", "--build", "dawn.build", "--config", mode, "--target", "webgpu", *parallel
        ], check = True)

        subprocess.run([
            "cmake", "--build", "dawn.build", "--config", mode, "--target", "tint_cmd_tint_cmd", *parallel
        ], check = True)

        # package result
        print("  * copying build artifacts...")
        sums = dict()

        os.makedirs("dawn.out/include", exist_ok=True)
        os.makedirs("dawn.out/bin", exist_ok=True)

        shutil.copy("dawn.build/gen/include/dawn/webgpu.h", "dawn.out/include/")

        sums['include/webgpu.h'] = {
            "commit": DAWN_COMMIT,
            "sum": checksum.filesum('dawn.out/include/webgpu.h')
        }

        if platform.system() == "Windows":
            shutil.copy(f"dawn.build/{mode}/webgpu.dll", "dawn.out/bin/")
            shutil.copy(f"dawn.build/src/dawn/native/{mode}/webgpu.lib", "dawn.out/bin/")

            sums['bin/webgpu.dll'] = {
                "commit": DAWN_COMMIT,
                "sum": checksum.filesum('dawn.out/bin/webgpu.dll')
            }
            sums['bin/webgpu.lib'] = {
                "commit": DAWN_COMMIT,
                "sum": checksum.filesum('dawn.out/bin/webgpu.lib')
            }
        else:
            shutil.copy("dawn.build/src/dawn/native/libwebgpu.dylib", "dawn.out/bin/")

            sums['bin/libwebgpu.dylib'] = {
                "commit": DAWN_COMMIT,
                "sum": checksum.filesum('dawn.out/bin/libwebgpu.dylib')
            }

    # save artifacts checksums
    with open('build/dawn.out/dawn.json', 'w') as f:
        json.dump(sums, f)

    print("Done")


#------------------------------------------------------
# build angle
#------------------------------------------------------

def angle_required_commit():
    with open("deps/angle-commit.txt", "r") as f:
        ANGLE_COMMIT = f.read().strip() # hardcoded commit for now
    return ANGLE_COMMIT

def dawn_required_files():
    artifacts = []
    if platform.system() == "Windows":
        artifacts = ["bin/webgpu.lib", "bin/webgpu.dll", "include/webgpu.h"]
    else:
        artifacts = ["bin/libwebgpu.dylib", "include/webgpu.h"]

    return artifacts

#############################################################################
#TODO: coalesce with check_angle
# use a checksum for the whole output directory
#############################################################################
def check_angle():

    ANGLE_COMMIT = angle_required_commit()

    up_to_date = True
    messages = []

    if os.path.exists("build/angle.out/angle.json"):
        with pushd("build/angle.out"):
            with open("angle.json", "r") as f:
                sums = json.loads(f.read())
                if 'commit' not in sums or 'sum' not in sums:
                    messages = ["malformed build/angle.out/angle.json"]
                    up_to_date = False
                elif sums['commit'] != ANGLE_COMMIT:
                    messages.append(f"build/angle.out doesn't match the required angle commit.\n  note: expected {ANGLE_COMMIT}, got {sums['commit']}")
                    up_to_date = False
                else:
                    s = checksum.dirsum('.', excluded_files=["angle.json", ".DS_Store"])
                    if s != sums['sum']:
                        messages.append(f"build/angle.out doesn't match checksum.\n  note: expected {sums['sum']}, got {s}")
                        up_to_date = False
    else:
        messages = [["build/angle.out/angle.json not found"]]
        up_to_date = False

    if up_to_date:
        os.makedirs("src/ext/angle/include", exist_ok=True)
        shutil.copytree("build/angle.out/include", "src/ext/angle/include/", dirs_exist_ok=True)

        os.makedirs("build/bin", exist_ok=True)
        shutil.copytree("build/angle.out/bin", "build/bin", dirs_exist_ok=True)

    return (up_to_date, messages)

def build_angle(args):
    ensure_programs()
    build_angle_internal(args.release, args.force)

def build_angle_internal(release, force):
    print("Building Angle...")

    os.makedirs("build/bin", exist_ok=True)

    # TODO ensure requirements

    ANGLE_COMMIT = angle_required_commit()

    # check if we already have the binary
    if not force:
        angle_ok, _ = check_angle()
        if angle_ok:
            print("  * already up to date")
            print("Done")
            return

    with pushd("build"):
        # get depot tools repo
        print("  * checking depot tools")
        if not os.path.exists("depot_tools"):
            subprocess.run([
                "git", "clone", "--depth=1", "--no-tags", "--single-branch",
                "https://chromium.googlesource.com/chromium/tools/depot_tools.git"
            ], check=True)
        os.environ["PATH"] = os.path.join(os.getcwd(), "depot_tools") + os.pathsep + os.environ["PATH"]
        os.environ["DEPOT_TOOLS_WIN_TOOLCHAIN"] = "0"

        # get angle repo
        print("  * checking angle")
        if not os.path.exists("angle"):
            subprocess.run([
                "git", "clone", "--no-tags", "--single-branch",
                "https://chromium.googlesource.com/angle/angle"
            ], check=True)

        with pushd("angle"):
            subprocess.run([
                "git", "fetch", "--no-tags"
            ], check=True)

            subprocess.run([
                "git", "reset", "--hard", ANGLE_COMMIT
            ], check=True)

            subprocess.run([
                sys.executable, "scripts/bootstrap.py"
            ], check=True)

            shell = True if platform.system() == "Windows" else False
            subprocess.run(["gclient", "sync"], shell=shell, check=True)

            print("  * preparing build")

            mode = "Release" if release else "Debug"
            is_debug = "false" if release else "true"

            gnargs = [
                "angle_build_all=false",
                "angle_build_tests=false",
                f"is_debug={is_debug}",
                "is_component_build=false",
            ]

            if platform.system() == "Windows":
                gnargs += [
                    "angle_enable_d3d9=false",
                    "angle_enable_gl=false",
                    "angle_enable_vulkan=false",
                    "angle_enable_null=false",
                    "angle_has_frame_capture=false"
                ]
            else:
                gnargs += [
                    #NOTE(martin): oddly enough, this is needed to avoid deprecation errors when _not_ using OpenGL,
                    #              because angle uses some CGL APIs to detect GPUs.
                    "treat_warnings_as_errors=false",
                    "angle_enable_metal=true",
                    "angle_enable_gl=false",
                    "angle_enable_vulkan=false",
                    "angle_enable_null=false"
                ]

            gnargString = ' '.join(gnargs)

            subprocess.run(["gn", "gen", f"out/{mode}", f"--args={gnargString}"], shell=shell, check=True)

            print("  * building")
            subprocess.run(["autoninja", "-C", f"out/{mode}", "libEGL", "libGLESv2"], shell=shell, check=True)

        # package result
        print("  * copying build artifacts...")

        yeetdir("angle.out")
        os.makedirs("angle.out/include", exist_ok=True)
        os.makedirs("angle.out/bin", exist_ok=True)

        # - includes
        shutil.copytree("angle/include/KHR", "angle.out/include/KHR", dirs_exist_ok=True)
        shutil.copytree("angle/include/EGL", "angle.out/include/EGL", dirs_exist_ok=True)
        shutil.copytree("angle/include/GLES", "angle.out/include/GLES", dirs_exist_ok=True)
        shutil.copytree("angle/include/GLES2", "angle.out/include/GLES2", dirs_exist_ok=True)
        shutil.copytree("angle/include/GLES3", "angle.out/include/GLES3", dirs_exist_ok=True)

        # - libs
        if platform.system() == "Windows":
            shutil.copy(f"angle/out/{mode}/libEGL.dll", "angle.out/bin/")
            shutil.copy(f"angle/out/{mode}/libGLESv2.dll", "angle.out/bin/")

            shutil.copy(f"angle/out/{mode}/libEGL.dll.lib", "angle.out/bin/")
            shutil.copy(f"angle/out/{mode}/libGLESv2.dll.lib", "angle.out/bin/")

            subprocess.run(["copy", "/y",
                            "%ProgramFiles(x86)%\\Windows Kits\\10\\Redist\\D3D\\x64\\d3dcompiler_47.dll",
                            "angle.out\\bin"], shell=True, check=True)
        else:
            shutil.copy(f"angle/out/{mode}/libEGL.dylib", "angle.out/bin")
            shutil.copy(f"angle/out/{mode}/libGLESv2.dylib", "angle.out/bin")

    # - sums
    sums = {
        "commit": ANGLE_COMMIT,
        "sum": checksum.dirsum("build/angle.out")
    }

    # save artifacts checksums
    with open('build/angle.out/angle.json', 'w') as f:
        json.dump(sums, f)

    print("Done")


#------------------------------------------------------
# build runtime
#------------------------------------------------------

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
    debug_flags = ["/O2", "/Zi"] if release else ["/O1", "/Zi"]

    for f in glob.iglob("./src/ext/wasm3/source/*.c"):
        name = os.path.splitext(os.path.basename(f))[0]
        subprocess.run([
            "cl", "/nologo",
            *debug_flags,
            "/Zc:preprocessor", "/c",
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
    debug_flags = ["-g", "-O2"] if release else ["-g", "-O1"]
    flags = [
        *debug_flags,
        "-foptimize-sibling-calls",
        "-Wno-extern-initializer",
        "-Dd_m3VerboseErrorMessages",
        f"-mmacos-version-min={MACOS_VERSION_MIN}"
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

def build_bytebox(release):
    print("Building bytebox...")
    args = ["zig", "build", "-Doptimize=ReleaseFast"]
    subprocess.run(args, cwd="src/ext/bytebox/")

    if platform.system() == "Windows":
        shutil.copy("src/ext/bytebox/zig-out/lib/bytebox.lib", "build/bin")
    elif platform.system() == "Darwin":
        shutil.copy("src/ext/bytebox/zig-out/lib/libbytebox.a", "build/bin")
    else:
        log_error(f"can't build bytebox for unknown platform '{platform.system()}'")
        exit(1)


def build_runtime(args):
    ensure_programs()
    build_runtime_internal(args.release, args.wasm_backend)

def build_runtime_internal(release, wasm_backend):
    build_platform_layer_internal(release)

    if wasm_backend == "bytebox":
        build_bytebox(release)
    else:
        build_wasm3(release)

    print("Building Orca runtime...")

    os.makedirs("build/bin", exist_ok=True)
    os.makedirs("build/lib", exist_ok=True)

    if platform.system() == "Windows":
        build_runtime_win(release, wasm_backend)
    elif platform.system() == "Darwin":
        build_runtime_mac(release, wasm_backend)
    else:
        log_error(f"can't build Orca for unknown platform '{platform.system()}'")
        exit(1)

def build_runtime_win(release, wasm_backend):

    gen_all_bindings()

    # compile orca
    includes = [
        "/I", "src",
        "/I", "src/ext",
        "/I", "src/ext/angle/include",
    ]

    defines = []
    link_commands = ["build/bin/orca.dll.lib"]

    debug_flags = ["/O2", "/Zi"] if release else ["/Zi", "/DOC_DEBUG", "/DOC_LOG_COMPILE_DEBUG"]

    if wasm_backend == "bytebox":
        includes += ["/I", "src/ext/bytebox/zig-out/include"]
        defines += ["/DOC_WASM_BACKEND_WASM3=0", "/DOC_WASM_BACKEND_BYTEBOX=1"]

        # Bytebox uses zig libraries that depend on NTDLL on Windows. Additionally, some zig APIs expect there to be
        # a large stack to use for scratch space, as zig stacks are 8MB by default. When linking bytebox, we must
        # ensure we provide the same amount of stack space, else risk stack overflows.
        link_commands += ["build/bin/bytebox.lib", "ntdll.lib", "/STACK:8388608,8388608"]
    else:
        includes += ["/I", "src/ext/wasm3/source"]
        defines += ["/DOC_WASM_BACKEND_WASM3=1", "/DOC_WASM_BACKEND_BYTEBOX=0"]
        link_commands += ["build/bin/wasm3.lib"]

    compile_args=[
        "cl",
        *debug_flags,
        "/Zc:preprocessor",
        "/std:c11", "/experimental:c11atomics",
        *defines,
        *includes,
        "src/runtime.c",
        "/Fe:build/bin/orca_runtime.exe",
        "/link",
        *link_commands
    ]

    backend_deps = "ntdll.lib ";

    subprocess.run(compile_args, check=True)

def build_runtime_mac(release, wasm_backend):

    includes = [
        "-Isrc",
        "-Isrc/ext",
        "-Isrc/ext/angle/include",
    ]

    defines = []

    libs = ["-Lbuild/bin", "-Lbuild/lib", "-lorca"]

    if wasm_backend == "bytebox":
        includes += ["-Isrc/ext/bytebox/zig-out/include"]
        defines += ["-DOC_WASM_BACKEND_WASM3=0", "-DOC_WASM_BACKEND_BYTEBOX=1"]
        libs += ["-lbytebox"]
    else:
        includes += ["-Isrc/ext/wasm3/source"]
        defines += ["-DOC_WASM_BACKEND_WASM3=1", "-DOC_WASM_BACKEND_BYTEBOX=0"]
        libs += ["-lwasm3"]

    debug_flags = ["-O2"] if release else ["-g", "-DOC_DEBUG -DOC_LOG_COMPILE_DEBUG"]
    flags = [
        *debug_flags,
        "--std=c11",
        f"-mmacos-version-min={MACOS_VERSION_MIN}"]

    gen_all_bindings()

    # compile orca
    subprocess.run([
        "clang", *flags, *defines, *includes, *libs,
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

#------------------------------------------------------
# build platform layer
#------------------------------------------------------
def build_platform_layer(args):
    ensure_programs()
    build_platform_layer_internal(args.release)

def build_platform_layer_internal(release):
    print("Building Orca platform layer...")

    angle_ok, angle_messages = check_angle()
    if not angle_ok:
        msg = log_error("Angle files are not present or don't match required commit.")
        msg.more(f"Angle commit: {angle_required_commit()}")
        for entry in angle_messages:
            msg.more(f"  * {entry}")
        cmd = "./orcadev" if platform.system() == "Darwin" else "orcadev.bat"
        msg.more("")
        msg.more(f"You can build the required files by running '{cmd} build-angle'")
        msg.more("")
        msg.more("Alternatively you can trigger a CI run to build the binaries on github:")
        msg.more("  * For Windows, go to https://github.com/orca-app/orca/actions/workflows/build-angle-win.yaml")
        msg.more("  * For macOS, go to https://github.com/orca-app/orca/actions/workflows/build-angle-mac.yaml")
        msg.more("  * Click on \"Run workflow\" to tigger a new run, or download artifacts from a previous run")
        msg.more("  * Put the contents of the artifacts folder in './build/angle.out'")
        exit()

    dawn_ok, dawn_messages = check_dawn()
    if not dawn_ok:
        msg = log_error("Dawn files are not present or don't match required commit.")
        msg.more(f"Dawn commit: {dawn_required_commit()}")
        msg.more("Dawn required files:")
        for entry in dawn_messages:
            msg.more(f"  * {entry}")
        cmd = "./orcadev" if platform.system() == "Darwin" else "orcadev.bat"
        msg.more("")
        msg.more(f"You can build the required files by running '{cmd} build-dawn'")
        msg.more("")
        msg.more("Alternatively you can trigger a CI run to build the binaries on github:")
        msg.more("  * For Windows, go to https://github.com/orca-app/orca/actions/workflows/build-dawn-win.yaml")
        msg.more("  * For macOS, go to https://github.com/orca-app/orca/actions/workflows/build-dawn-mac.yaml")
        msg.more("  * Click on \"Run workflow\" to tigger a new run, or download artifacts from a previous run")
        msg.more("  * Put the contents of the artifacts folder in './build/dawn.out'")
        exit()

    os.makedirs("build/bin", exist_ok=True)
    os.makedirs("build/lib", exist_ok=True)

    if platform.system() == "Windows":
        build_platform_layer_lib_win(release)
    elif platform.system() == "Darwin":
        build_platform_layer_lib_mac(release)
    else:
        log_error(f"can't build platform layer for unknown platform '{platform.system()}'")
        exit(1)


def build_platform_layer_lib_win(release):
    embed_shaders(
        outputPath = "src/graphics/wgpu_renderer_shaders.h",
        prefix = "oc_wgsl_",
        commonPath = "src/graphics/wgsl_shaders/common.wgsl",
        inputFiles = [
            "src/graphics/wgsl_shaders/path_setup.wgsl",
            "src/graphics/wgsl_shaders/segment_setup.wgsl",
            "src/graphics/wgsl_shaders/backprop.wgsl",
            "src/graphics/wgsl_shaders/chunk.wgsl",
            "src/graphics/wgsl_shaders/merge.wgsl",
            "src/graphics/wgsl_shaders/balance_workgroups.wgsl",
            "src/graphics/wgsl_shaders/raster.wgsl",
            "src/graphics/wgsl_shaders/blit.wgsl",
            "src/graphics/wgsl_shaders/final_blit.wgsl",
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
        "dxgi.lib",
        "dxguid.lib",
        "d3d11.lib",
        "dcomp.lib",
        "shcore.lib",
        "delayimp.lib",
        "dwmapi.lib",
        "comctl32.lib",
        "ole32.lib",
        "shell32.lib",
        "shlwapi.lib",
        "/LIBPATH:build/lib",
        "/LIBPATH:build/bin",
        "libEGL.dll.lib",
        "libGLESv2.dll.lib",
        "/DELAYLOAD:libEGL.dll",
        "/DELAYLOAD:libGLESv2.dll",
        "webgpu.lib",
        "/DELAYLOAD:webgpu.dll"
    ]

    debug_flags = ["/O2", "/Zi"] if release else ["/Zi", "/DOC_DEBUG", "/DOC_LOG_COMPILE_DEBUG"]

    subprocess.run([
        "cl", "/nologo",
        "/we4013",
        *debug_flags,
        "/Zc:preprocessor",
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

    build_dir = os.path.join("build", "bin")
    # ensure orca.exe debug pdb doesn't override orca.dll pdb
    shutil.copy(os.path.join(build_dir, "orca.pdb"), os.path.join(build_dir, "orca_dll.pdb"))

def build_platform_layer_lib_mac(release):

    embed_shaders(
        outputPath = "src/graphics/wgpu_renderer_shaders.h",
        prefix = "oc_wgsl_",
        commonPath = "src/graphics/wgsl_shaders/common.wgsl",
        inputFiles = [
            "src/graphics/wgsl_shaders/path_setup.wgsl",
            "src/graphics/wgsl_shaders/segment_setup.wgsl",
            "src/graphics/wgsl_shaders/backprop.wgsl",
            "src/graphics/wgsl_shaders/chunk.wgsl",
            "src/graphics/wgsl_shaders/merge.wgsl",
            "src/graphics/wgsl_shaders/balance_workgroups.wgsl",
            "src/graphics/wgsl_shaders/raster.wgsl",
            "src/graphics/wgsl_shaders/blit.wgsl",
            "src/graphics/wgsl_shaders/final_blit.wgsl",
        ])

    flags = [f"-mmacos-version-min={MACOS_VERSION_MIN}"]
    cflags = ["-std=c11"]
    debug_flags = ["-O3"] if release else ["-g", "-DOC_DEBUG", "-DOC_LOG_COMPILE_DEBUG"]
    ldflags = [f"-L{MAC_SDK_DIR}/usr/lib", f"-F{MAC_SDK_DIR}/System/Library/Frameworks/"]
    includes = ["-Isrc", "-Isrc/ext", "-Isrc/ext/angle/include", "-Isrc/ext/dawn/include"]

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
        "-weak-lEGL", "-weak-lGLESv2", "-weak-lwebgpu"
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

#------------------------------------------------------
# build wasm SDK
#------------------------------------------------------
def build_sdk(args):
    ensure_programs()
    build_sdk_internal(args.release)

def build_sdk_internal(release):
    print("Building Orca wasm SDK...")

    includes = [
        "-I", "src",
        "-I", "src/ext",
        "-I", "build/orca-libc/include",
    ]

    # we still use -O1 for debug, otherwise clang generates an ungodly amount of locals for no reason
    debug_flags = ["-O2", "-DNDEBUG"] if release else ["-g", "-O1"]

    flags = [
        *debug_flags,
        "--target=wasm32",
        "--no-standard-libraries",
        "-mbulk-memory",
        "-D__ORCA__",
        "-Wl,--no-entry",
        "-Wl,--export-dynamic",
        "-Wl,--relocatable"
    ]

    clang = 'clang'

    #NOTE(martin): this is an extremely stupid workaround to play well with github CI runners, which
    # have llvm clang only accessible through $(brew --prefix llvm@15), whereas locally we could want to
    # use another version.
    # TODO: we should probably pass a flag to inform the script it's running in CI. This would avoid picking
    # llvm 15 when a later version is available locally?
    if platform.system() == "Darwin":
        try:
            brew_llvm = subprocess.check_output(["brew", "--prefix", "llvm@15", "--installed"], stderr=subprocess.DEVNULL).decode().strip()
        except subprocess.CalledProcessError:
            brew_llvm = subprocess.check_output(["brew", "--prefix", "llvm", "--installed"]).decode().strip()
        clang = os.path.join(brew_llvm, 'bin', 'clang')

    # compile sdk

    subprocess.run([
        clang, *flags, *includes,
         "-o", "build/bin/liborca_wasm.a",
         "src/orca.c"
    ], check=True)

#------------------------------------------------------
# build libc
#------------------------------------------------------
def build_libc(args):
    ensure_programs()
    build_lib_internal(args.release)

def build_libc_internal(release):
    print("Building orca-libc...")

    # create directory and copy header files
    os.makedirs("build/orca-libc", exist_ok=True)
    os.makedirs("build/orca-libc/lib", exist_ok=True)
    os.makedirs("build/orca-libc/include", exist_ok=True)

    shutil.copytree(f"src/orca-libc/include", "build/orca-libc/include", dirs_exist_ok=True)

    # compile flags, include, etc
    cfiles = []
    dirs = os.listdir("src/orca-libc/src")
    for directory in dirs:
        cfiles.extend(glob.glob('src/orca-libc/src/' + directory + '/*.c'))

    includes = [
        "-Isrc",
        "-isystem", "src/orca-libc/include",
        "-isystem", "src/orca-libc/include/private",
        "-Isrc/orca-libc/src/arch",
        "-Isrc/orca-libc/src/internal"
    ]

    warning_flags = [
        "-Wall", "-Wextra", "-Werror", "-Wno-null-pointer-arithmetic", "-Wno-unused-parameter", "-Wno-sign-compare", "-Wno-unused-variable", "-Wno-unused-function", "-Wno-ignored-attributes", "-Wno-missing-braces", "-Wno-ignored-pragmas", "-Wno-unused-but-set-variable", "-Wno-unknown-warning-option", "-Wno-parentheses", "-Wno-shift-op-parentheses", "-Wno-bitwise-op-parentheses", "-Wno-logical-op-parentheses", "-Wno-string-plus-int", "-Wno-dangling-else", "-Wno-unknown-pragmas"
    ]

    debug_flags = ["-O2", "-DNDEBUG"] if release else ["-g"]

    flags = [
        *debug_flags,
        *warning_flags,
        "--target=wasm32",
        "--std=c11",
        "-D__ORCA__",
        "--no-standard-libraries",
        "-fno-trapping-math",
        "-mbulk-memory",
        "-DBULK_MEMORY_THRESHOLD=32",
        "-mthread-model", "single",
        "-Wl,--relocatable"
    ]

    clang = 'clang'
    llvm_ar = 'llvm-ar'

    #NOTE(martin): this is an extremely stupid workaround to play well with github CI runners, which
    # have llvm clang only accessible through $(brew --prefix llvm@15), whereas locally we could want to
    # use another version.
    # TODO: we should probably pass a flag to inform the script it's running in CI. This would avoid picking
    # llvm 15 when a later version is available locally?
    if platform.system() == "Darwin":
        try:
            brew_llvm = subprocess.check_output(["brew", "--prefix", "llvm@15", "--installed"], stderr=subprocess.DEVNULL).decode().strip()
        except subprocess.CalledProcessError:
            brew_llvm = subprocess.check_output(["brew", "--prefix", "llvm", "--installed"]).decode().strip()
        clang = os.path.join(brew_llvm, 'bin', 'clang')
        llvm_ar = os.path.join(brew_llvm, 'bin', 'llvm-ar')

    # compile dummy CRT
    subprocess.run([
        clang, *flags, *includes,
        "-o", "build/orca-libc/lib/crt1.o",
        "src/orca-libc/src/crt/crt1.c"
    ], check=True)

    # compile standard lib
    subprocess.run([
        clang, *flags, *includes,
        "-o", "build/orca-libc/lib/libc.o",
         *cfiles
    ], check=True)

    subprocess.run([
        llvm_ar, "crs",
        "build/orca-libc/lib/libc.a",
        "build/orca-libc/lib/libc.o"
    ], check=True)

#------------------------------------------------------
# build CLI tool
#------------------------------------------------------
def build_tool(args):
    print("Building Orca CLI tool...")

    ensure_programs()
    build_libcurl()
    build_zlib()

    os.makedirs("build/bin", exist_ok=True)

    if args.version == None:
        res = subprocess.run(["git", "rev-parse", "--short", "HEAD"], check=True, capture_output=True, text=True)
        version = res.stdout.strip()
    else:
        version = args.version

    if platform.system() == "Windows":
        build_tool_win(args.release, version)
    elif platform.system() == "Darwin":
        build_tool_mac(args.release, version)
    else:
        log_error(f"can't build cli tool for unknown platform '{platform.system()}'")
        exit(1)

    with open("build/orcatool.sum", "w") as f:
        f.write(tool_checksum())


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


def build_tool_win(release, version):
    includes = [
        "/I", "src",
        "/I", "src/ext/stb",
        "/I", "src/ext/curl/builds/static/include",
        "/I", "src/ext/zlib",
        "/I", "src/ext/microtar"
    ]

    debug_flags = ["/O2"] if release else ["/Zi", "/DOC_DEBUG", "/DOC_LOG_COMPILE_DEBUG"]

    libs = [
        "shlwapi.lib",
        "shell32.lib",
        "ole32.lib",
        "Kernel32.lib",

        # libs needed by curl
        "advapi32.lib",
        "crypt32.lib",
        "normaliz.lib",
        "ws2_32.lib",
        "wldap32.lib",
        "/LIBPATH:src/ext/curl/builds/static/lib",
        "libcurl_a.lib",

        "/LIBPATH:src/ext/zlib/build",
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
        f"/Febuild/bin/orca.exe",
        "src/tool/main.c",
        "/link",
        *libs,
    ], check=True)

def build_tool_mac(release, version):
    includes = [
        "-I", "src",
        "-I", "src/ext/curl/builds/static/include",
        "-I", "src/ext/zlib",
        "-I", "src/ext/microtar"
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
        "-Lsrc/ext/curl/builds/static/lib", "-lcurl",

        "-Lsrc/ext/zlib/build", "-lz",
    ]

    subprocess.run([
        "clang",
        f"-mmacos-version-min={MACOS_VERSION_MIN}",
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
        "-o", f"build/bin/orca",
        "src/tool/main.c",
    ], check=True)

    with open("build/compile_commands.json", "w") as f:
        f.write("[\n")
        with open("build/main.json") as m:
            f.write(m.read())
        f.write("]")

def tool_checksum():
    tool_dir = "src\\tool" if platform.system() == "Windows" else "src/tool"
    return dirsum(tool_dir)


def tool_checksum_last():
    try:
        with open("build/orcatool.sum", "r") as f:
            return f.read()
    except FileNotFoundError:
        return None

#------------------------------------------------------
# package
#------------------------------------------------------
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

def package_sdk_internal(dest, target):
    bin_dir = os.path.join(dest, "bin")
    libc_dir = os.path.join(dest, "orca-libc")
    res_dir = os.path.join(dest, "resources")
    src_dir = os.path.join(dest, "src")

    yeetdir(dest)
    os.makedirs(bin_dir, exist_ok=True)
    os.makedirs(res_dir, exist_ok=True)
    os.makedirs(src_dir, exist_ok=True)

    if target == "Windows":
        shutil.copy(os.path.join("build", "bin", "orca.exe"), bin_dir)
        shutil.copy(os.path.join("build", "bin", "orca.dll"), bin_dir)
        shutil.copy(os.path.join("build", "bin", "orca_runtime.exe"), bin_dir)
        shutil.copy(os.path.join("build", "bin", "liborca_wasm.a"), bin_dir)
        shutil.copy(os.path.join("build", "bin", "libEGL.dll"), bin_dir)
        shutil.copy(os.path.join("build", "bin", "libGLESv2.dll"), bin_dir)
        shutil.copy(os.path.join("build", "bin", "webgpu.dll"), bin_dir)
    else:
        shutil.copy(os.path.join("build", "bin", "orca"), bin_dir)
        shutil.copy(os.path.join("build", "bin", "orca_runtime"), bin_dir)
        shutil.copy(os.path.join("build", "bin", "liborca.dylib"), bin_dir)
        shutil.copy(os.path.join("build", "bin", "liborca_wasm.a"), bin_dir)
        shutil.copy(os.path.join("build", "bin", "libEGL.dylib"), bin_dir)
        shutil.copy(os.path.join("build", "bin", "libGLESv2.dylib"), bin_dir)
        shutil.copy(os.path.join("build", "bin", "libwebgpu.dylib"), bin_dir)


    shutil.copytree(os.path.join("build", "orca-libc"), libc_dir, dirs_exist_ok=True)
    shutil.copytree("resources", res_dir, dirs_exist_ok=True)

    ignore_patterns = shutil.ignore_patterns(
        '*.[!h]', # exclude anything that is not a header
        'tool',
        'ext',
        'orca-libc'
    )

    def ignore(dirName, files):
        # ignore everything that's not a header
        result = [ x for x in files if os.path.isfile(os.path.join(dirName, x)) and x[-2:] != ".h" ]

        # exclude or include specific sub-directories
        ignoreDirs = []
        onlyDirs = []

        if dirName == 'src':
            ignoreDirs = ['tool', 'orca-libc', 'wasm']
        elif dirName == 'src/ext/curl':
            onlyDirs = ['include']
        elif dirName == 'src/ext/wasm3':
            onlyDirs = ['source']
        elif dirName == 'src/ext/zlib':
            ignoreDirs = ['build']

        if len(ignoreDirs):
            result += [x for x in files if x in ignoreDirs]

        if len(onlyDirs):
            result += [x for x in files if os.path.isdir(os.path.join(dirName, x)) and x not in onlyDirs]

        return result

    shutil.copytree("src", src_dir, dirs_exist_ok=True, ignore=ignore)

    for dirpath, dirnames, filenames in os.walk(src_dir, topdown=False):
        if not os.listdir(dirpath):
            os.rmdir(dirpath)


def package_sdk(args):
    dest = args.dest
    target = platform.system() if args.target == None else args.target
    package_sdk_internal(dest, target)

#------------------------------------------------------
# install
#------------------------------------------------------
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

    orca_dir = system_orca_dir() if args.install_dir == None else args.install_dir
    version = f"dev-{orca_commit()}" if args.version == None else args.version
    dest = os.path.join(orca_dir, version)

    print(f"Installing dev build of Orca in {dest}")

    package_sdk_internal(dest, platform.system())

    tool_path = os.path.join("build", "bin", "orca.exe") if platform.system() == "Windows" else os.path.join("build","bin","orca")
    shutil.copy(tool_path, orca_dir)

    with open(os.path.join(orca_dir, "current_version"), "w") as f:
        f.write(version)

    # TODO(shaw): should dev versions and their checksums be added to all_versions file?

#------------------------------------------------------
# Clean
#------------------------------------------------------
def clean(args):
    print("Removing build artifacts...")
    yeetdir("build")
    yeetdir("src/ext/angle")
    yeetdir("src/ext/dawn")
    yeetdir("scripts/files")
    yeetdir("scripts/__pycache__")


#------------------------------------------------------
# utils
#------------------------------------------------------
def ensure_programs():
    missing = []

    if platform.system() == "Windows":
        MSVC_MAJOR, MSVC_MINOR = 19, 35

        # Get where Visual Studio is installed
        where = subprocess.run(
            ["%ProgramFiles(x86)%\\Microsoft Visual Studio\\Installer\\vswhere.exe",
            "-latest",
            "-requires", "Microsoft.VisualStudio.Workload.NativeDesktop",
            "-property", "installationPath"],
            capture_output=True,
            shell=True,
            text=True)

        if len(where.stdout) == 0:
            missing.append(["Visual Studio"])
        else:
            # get the environment after calling vcvarsall.bat and update our own env with it
            os.environ["__VSCMD_ARG_NO_LOGO"] = "1"
            varsall = subprocess.run(
                [os.path.join(where.stdout.strip(), "VC\\Auxiliary\\Build\\vcvarsall.bat"), "amd64", ">nul"
                "&&",
                "python", "-c", "import os; print(repr(os.environ))"],
                capture_output=True,
                text=True)

            env = eval(varsall.stdout.splitlines()[-1].strip('environ'))
            os.environ.update(env)

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
                missing.append(["Visual Studio"])

        try:
            subprocess.run(["clang", "-v"], stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
        except FileNotFoundError:
            missing.append(["clang", "To install it, launch Visual Studio Installer, click Modify, and check \"Desktop development with C++\" and \"C++ Clang Compiler\""])

        try:
            subprocess.run(["cmake", "--version"], stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
        except FileNotFoundError:
            missing.append(["cmake", "To install it, launch Visual Studio Installer, click Modify, and check \"Desktop development with C++\" and \"C++ Clang Compiler\""])

        try:
            subprocess.run(["git", "-v"], stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
        except FileNotFoundError:
            missing.append(["git", "You can install git for Windows from https://gitforwindows.org/"])

    if platform.system() == "Darwin":

        try:
            subprocess.run(["clang", "-v"], stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
        except FileNotFoundError:
            missing.append(["clang", "To install it, run: brew install llvm"])

        if not os.path.exists(MAC_SDK_DIR):
            missing.append(["XCode command-line tools", "To install it, run: xcode-select --install"])

        try:
            subprocess.run(["cmake", "--version"], stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
        except FileNotFoundError:
            missing.append(["cmake", "To install it, run: brew install cmake"])

        try:
            subprocess.run(["git", "-v"], stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
        except FileNotFoundError:
            missing.append(["git", "To install it, run: xcode-select --install"])

    if len(missing):
        msg = log_error("The following required tools were not found on your system")
        for entry in missing:
            msg.more(f"* {entry[0]}")
            if len(note) > 1:
                msg.more(f" note: {entry[1]}")
        exit(1)

# def ensure_angle():
#     if not verify_angle():
#         download_angle()
#         print("Verifying ANGLE download...")
#         if not verify_angle():
#             log_error("automatic ANGLE download failed")
#             exit(1)


# def verify_angle():
#     checkfiles = None
#     if platform.system() == "Windows":
#         checkfiles = [
#             "build/bin/libEGL.dll",
#             "build/lib/libEGL.dll.lib",
#             "build/bin/libGLESv2.dll",
#             "build/lib/libGLESv2.dll.lib",
#         ]
#     elif platform.system() == "Darwin":
#         checkfiles = [
#             "build/bin/libEGL.dylib",
#             "build/bin/libGLESv2.dylib",
#         ]

#     if checkfiles is None:
#         log_warning("could not verify if the correct version of ANGLE is present")
#         return False

#     ok = True
#     for file in checkfiles:
#         if not os.path.isfile(file):
#             ok = False
#             continue
#         if not checksum.checkfile(file):
#             ok = False
#             continue

#     return ok


# def download_angle():
#     print("Downloading ANGLE...")
#     if platform.system() == "Windows":
#         build = "windows-2019"
#     elif platform.system() == "Darwin":
#         build = "macos-jank"
#     else:
#         log_error(f"could not automatically download ANGLE for unknown platform {platform.system()}")
#         return

#     os.makedirs("scripts/files", exist_ok=True)
#     filename = f"angle-{build}-{ANGLE_VERSION}.zip"
#     filepath = f"scripts/files/{filename}"
#     url = f"https://github.com/HandmadeNetwork/build-angle/releases/download/{ANGLE_VERSION}/{filename}"
#     with urllib.request.urlopen(url) as response:
#         with open(filepath, "wb") as out:
#             shutil.copyfileobj(response, out)

#     if not checksum.checkfile(filepath):
#         log_error(f"ANGLE download did not match checksum")
#         exit(1)

#     print("Extracting ANGLE...")
#     with ZipFile(filepath, "r") as anglezip:
#         anglezip.extractall(path="scripts/files")

#     shutil.copytree(f"scripts/files/angle/include", "src/ext/angle/include", dirs_exist_ok=True)
#     shutil.copytree(f"scripts/files/angle/bin", "build/bin", dirs_exist_ok=True)
#     if platform.system() == "Windows":
#         shutil.copytree(f"scripts/files/angle/lib", "build/lib", dirs_exist_ok=True)

def prompt(msg):
    while True:
        answer = input(f"{msg} (y/n)> ")
        if answer.lower() in ["y", "yes"]:
            return True
        elif answer.lower() in ["n", "no"]:
            return False
        else:
            print("Please enter \"yes\" or \"no\" and press return.")


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


def src_dir():
    # Fragile path adjustments! Yay!
    return os.path.normpath(os.path.join(os.path.abspath(__file__), "../../src"))


def orca_commit():
    with pushd(src_dir()):
        try:
            res = subprocess.run(["git", "rev-parse", "--short", "HEAD"], check=True, capture_output=True, text=True)
            commit = res.stdout.strip()
            return commit
        except subprocess.CalledProcessError:
            log_warning("failed to look up current git hash")
            return "unknown"


def yeet(path):
    os.makedirs(path, exist_ok=True)
    shutil.rmtree(path)
