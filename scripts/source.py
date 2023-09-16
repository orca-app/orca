import hashlib
import json
import os
import re
import shutil

from .log import *
from .utils import yeetdir
from .version import src_dir, orca_version


def attach_source_commands(subparsers):
    source_cmd = subparsers.add_parser("source", help="Commands for helping compile the Orca source code into your project.")
    source_sub = source_cmd.add_subparsers(required=True, title="commands")

    cflags_cmd = source_sub.add_parser("cflags", help="Get help setting up a C or C++ compiler to compile the Orca source.")
    cflags_cmd.add_argument("srcdir", nargs="?", default=src_dir(), help="the directory containing the Orca source code (defaults to system installation)")
    cflags_cmd.set_defaults(func=shellish(cflags))

    vendor_cmd = source_sub.add_parser("vendor", help="Copy the Orca source code into your project.")
    vendor_cmd.add_argument("dir", type=str, help="the directory into which the Orca source code will be copied")
    vendor_cmd.set_defaults(func=shellish(vendor))


def vendor(args):
    # Verify that we are ok to vendor into the requested dir.
    if os.path.exists(args.dir):
        try:
            with open(vendor_file_path(args.dir), "r") as f:
                vendor_info = json.load(f)
                version = vendor_info["version"]
            print(f"Orca version {version} is currently installed in that directory.")

            if vendor_checksum(args.dir) != vendor_info["checksum"]:
                log_error(f"The contents of your vendor directory have been modified. This command will exit to avoid overwriting any local changes. To proceed, manually delete {args.dir} and try again.")
                exit(1)
        except FileNotFoundError:
            if len(os.listdir(args.dir)) > 0:
                log_error(f"The requested directory already exists and does not appear to contain Orca source code. To avoid deleting anything important, please either provide the correct path or manually empty {args.dir} first.")
                exit(1)

    yeetdir(args.dir)
    shutil.copytree(src_dir(), args.dir)
    with open(vendor_file_path(args.dir), "w") as f:
        json.dump({
            "version": orca_version(),
            "checksum": vendor_checksum(args.dir),
        }, f, indent=2)
    print(f"Version {orca_version()} of the Orca source code has been copied to {args.dir}.")


def vendor_file_path(vendor_dir):
    return os.path.join(vendor_dir, ".orcavendor")


def vendor_checksum(dir):
    return dirhash(dir, excluded_extensions=["orcavendor"])


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
    print(f"clang --target=wasm32 --no-standard-libraries -mbulk-memory -g -O2 -D__ORCA__ -Wl,--no-entry -Wl,--export-dynamic -isystem {sysinclude} -I {include} -I {extinclude} {orcac} {libcsource} your-main.c")
    print()
    if not path_contains(os.getcwd(), args.srcdir):
        print("If these paths look crazy to you, consider vendoring the source code into your")
        print("project using `orca source vendor`.")
        print()


# -----------------------------------------------------------------------------
# Directory-hashing implementation pulled from the checksumdir package on pypi.
# Licensed under the MIT license.
# -----------------------------------------------------------------------------

def dirhash(
    dirname,
    hash_func=hashlib.sha1,
    excluded_files=None,
    ignore_hidden=False,
    followlinks=False,
    excluded_extensions=None,
    include_paths=False
):
    if not excluded_files:
        excluded_files = []

    if not excluded_extensions:
        excluded_extensions = []

    if not os.path.isdir(dirname):
        raise TypeError("{} is not a directory.".format(dirname))

    hashvalues = []
    for root, dirs, files in os.walk(dirname, topdown=True, followlinks=followlinks):
        if ignore_hidden and re.search(r"/\.", root):
            continue

        dirs.sort()
        files.sort()

        for fname in files:
            if ignore_hidden and fname.startswith("."):
                continue

            if fname.split(".")[-1:][0] in excluded_extensions:
                continue

            if fname in excluded_files:
                continue

            hashvalues.append(_filehash(os.path.join(root, fname), hash_func))

            if include_paths:
                hasher = hash_func()
                # get the resulting relative path into array of elements
                path_list = os.path.relpath(os.path.join(root, fname)).split(os.sep)
                # compute the hash on joined list, removes all os specific separators
                hasher.update(''.join(path_list).encode('utf-8'))
                hashvalues.append(hasher.hexdigest())

    return _reduce_hash(hashvalues, hash_func)


def _filehash(filepath, hashfunc):
    hasher = hashfunc()
    blocksize = 64 * 1024

    if not os.path.exists(filepath):
        return hasher.hexdigest()

    with open(filepath, "rb") as fp:
        while True:
            data = fp.read(blocksize)
            if not data:
                break
            hasher.update(data)
    return hasher.hexdigest()


def _reduce_hash(hashlist, hashfunc):
    hasher = hashfunc()
    for hashvalue in sorted(hashlist):
        hasher.update(hashvalue.encode("utf-8"))
    return hasher.hexdigest()
