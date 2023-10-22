import os

from .log import *


def attach_source_commands(subparsers):
    source_cmd = subparsers.add_parser("source", help="Commands for helping compile the Orca source code into your project.")
    source_sub = source_cmd.add_subparsers(required=True, title="commands")

    cflags_cmd = source_sub.add_parser("cflags", help="Get help setting up a C or C++ compiler to compile the Orca source.")
    cflags_cmd.add_argument("srcdir", nargs="?", default="TODO-NEED-REAL-INSTALL-DIR-OR-SOMETHING", help="the directory containing the Orca source code (defaults to system installation)")
    cflags_cmd.set_defaults(func=shellish(cflags))


def cflags(args):
    if not os.path.exists(os.path.join(args.srcdir, "orca.h")):
        log_error(f"The provided path does not seem to contain the Orca source code: {args.srcdir}")
        exit(1)
    
    def path_contains(a, b):
        a_abs = os.path.abspath(a)
        b_abs = os.path.abspath(b)
        return os.path.commonpath([a_abs, b_abs]) == a_abs

    def nicepath(path):
        path_abs = os.path.abspath(path)
        if path_contains(os.getcwd(), path_abs):
            return os.path.relpath(path_abs)
        else:
            return path_abs
    
    include = nicepath(args.srcdir)
    orcac = nicepath(os.path.join(args.srcdir, "orca.c"))
    extinclude = nicepath(os.path.join(args.srcdir, "ext"))
    sysinclude = nicepath(os.path.join(args.srcdir, "libc-shim/include"))
    libcsource = nicepath(os.path.join(args.srcdir, "libc-shim/src/*.c"))

    print("To compile Orca as part of your C or C++ project, you must:")
    print(f"> Put the following directory on your SYSTEM include search path:")
    print(f"  {sysinclude}")
    print(f"> Put the following directories on your include search path:")
    print(f"  {include}")
    print(f"  {extinclude}")
    print(f"> Compile the following file as a single translation unit:")
    print(f"  {orcac}")
    print(f"> Compile the following files as separate translation units:")
    print(f"  {libcsource}")
    print()
    print("The following clang flags are also required:")
    print("> --target=wasm32         (to compile to wasm)")
    print("> --no-standard-libraries (to use only our libc shim)")
    print("> -mbulk-memory           (to enable memset/memcpy intrinsics, which are required)")
    print("> -D__ORCA__              (to signal that the Orca source code is being compiled to run on Orca itself)")
    print("> -Wl,--no-entry         (to prevent wasm-ld from looking for a _start symbol)")
    print("> -Wl,--export-dynamic   (to expose your module's functions to Orca)")
    print()
    print("And the following clang flags are recommended:")
    print("> -g -O2                  (to compile with optimizations and debug info)")
    print()
    print("Complete clang example:")
    print()
    print(f"clang --target=wasm32 --no-standard-libraries -mbulk-memory -g -O2 -D__ORCA__ -Wl,--no-entry -Wl,--export-dynamic -isystem \"{sysinclude}\" -I \"{include}\" -I \"{extinclude}\" \"{orcac}\" \"{libcsource}\" your-main.c")
    print()
