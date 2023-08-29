import os
import subprocess
import sys

from .log import *
from .utils import pushd


# Checks if the Orca tool should use a source checkout of Orca instead of a system install.
# This is copy-pasted to the command-line tool so it can work before loading anything.
#
# Returns: (use source, source directory, is actually the source's tool)
def check_if_source():
    def path_is_in_orca_source(path):
        dir = path
        while True:
            try:
                os.stat(os.path.join(dir, ".orcaroot"))
                return (True, dir)
            except FileNotFoundError:
                pass

            newdir = os.path.dirname(dir)
            if newdir == dir:
                return (False, None)
            dir = newdir

    in_source, current_source_dir = path_is_in_orca_source(os.getcwd())
    script_is_source, script_source_dir = path_is_in_orca_source(os.path.dirname(os.path.abspath(__file__)))

    use_source = in_source or script_is_source
    source_dir = current_source_dir or script_source_dir
    return (use_source, source_dir, script_is_source)


def is_orca_source():
    use_source, _, _ = check_if_source()
    return use_source


def actual_install_dir():
    # The path adjustment in here is technically sort of fragile because it depends
    # on the current location of this actual file. But oh well.
    if is_orca_source():
        raise Exception("actual_install_dir should not be called when using the source version of the Orca tools")
    return os.path.normpath(os.path.join(os.path.abspath(__file__), "../../.."))


def src_dir():
    # More fragile path adjustments! Yay!
    if is_orca_source():
        return os.path.normpath(os.path.join(os.path.abspath(__file__), "../../src"))
    else:
        return os.path.normpath(os.path.join(os.path.abspath(__file__), "../../../src"))


def orca_version():
    is_source, source_dir, _ = check_if_source()
    if is_source:
        with pushd(source_dir):
            version = "unknown"
            if os.path.exists(".git"):
                try:
                    res = subprocess.run(["git", "rev-parse", "--short", "HEAD"], check=True, capture_output=True, text=True)
                    version = res.stdout.strip()
                except subprocess.CalledProcessError:
                    log_warning("failed to look up current git hash for version number")
            return f"dev-{version}"
    else:
        try:
            with open(os.path.join(actual_install_dir(), ".orcaversion"), "r") as f:
                version = f.read().strip()
                return version
        except FileNotFoundError:
            return "dev-unknown"


def attach_version_command(subparsers):
    version_cmd = subparsers.add_parser("version", help="Print the current Orca version.")
    version_cmd.set_defaults(func=print_orca_version)


def print_orca_version(args):
    use_source, source_dir, _ = check_if_source()

    # This function prints the bare version number to stdout and everything else
    # to stderr. This makes it easy to use the version number in shell pipelines
    # without requiring extra flags or parsing a weird output format.
    sys.stdout.write(orca_version() + "\n")
    if use_source:
        sys.stderr.write(f"Orca is running from a source checkout.\n")
        sys.stderr.write(f"Source dir: {source_dir}\n")
    else:
        sys.stderr.write(f"Orca is running from a system installation.\n")
        sys.stderr.write(f"Install dir: {actual_install_dir()}\n")
