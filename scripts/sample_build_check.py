import platform
import re
import shutil
import subprocess
import textwrap

CLANG_MAJOR, CLANG_MINOR = 11, 0

def printw(str=""):
    print(textwrap.fill(str))

def print_clang_install_info(upgrade):
    if platform.system() == "Windows":
        printw(f"Please install Clang {CLANG_MAJOR}.{CLANG_MINOR} or newer. We recommend installing Clang via the Visual Studio installer. In the installer, search for \"C++ Clang Compiler\".")
    elif platform.system() == "Darwin":
        printw(f"Please install Clang {CLANG_MAJOR}.{CLANG_MINOR} or newer. We recommend installing Clang via Homebrew:")
        printw()
        if upgrade:
            printw("  brew upgrade llvm")
        else:
            printw("  brew install llvm")
        printw()
    else:
        printw(f"Please install Clang {CLANG_MAJOR}.{CLANG_MINOR} or newer.")

try:
    out = subprocess.run(["clang", "--version"], capture_output=True, text=True, check=True)
    m = re.search(r"clang version (\d+)\.(\d+)\.(\d+)", out.stdout)
    major, minor, patch = int(m.group(1)), int(m.group(2)), int(m.group(3))
    if major < CLANG_MAJOR or minor < CLANG_MINOR:
        printw(f"ERROR: Your version of Clang is too old. You have version {major}.{minor}.{patch}, but version {CLANG_MAJOR}.{CLANG_MINOR} or greater is required.")
        printw()
        printw("This script is currently running the clang located at:")
        printw(shutil.which("clang"))
        printw()
        print_clang_install_info(True)
        exit(1)
except FileNotFoundError:
    printw("ERROR: clang is not installed. The Orca samples require Clang in order to compile C programs to WebAssembly.")
    printw()
    print_clang_install_info(False)
    exit(1)
except subprocess.CalledProcessError:
    printw("WARNING: Could not check Clang version. You may encounter build errors.")

try:
    subprocess.run(["orca", "version"], capture_output=True, shell=True, check=True)
except subprocess.CalledProcessError:
    printw("ERROR: The Orca tooling has not been installed to your system or is not on your PATH. From the root of the Orca source code, please run the following commands:")
    printw()
    printw("  python orca dev build-runtime")
    printw("  python orca dev install")
    exit(1)
