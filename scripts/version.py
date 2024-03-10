import os
import subprocess
import sys

from .log import *
from .utils import pushd


def src_dir():
    # Fragile path adjustments! Yay!
    return os.path.normpath(os.path.join(os.path.abspath(__file__), "../../src"))


def orca_version():
    with pushd(src_dir()):
        try:
            res = subprocess.run(["git", "rev-parse", "--short", "HEAD"], check=True, capture_output=True, text=True)
            version = res.stdout.strip()
            return f"dev-{version}"
        except subprocess.CalledProcessError:
            log_warning("failed to look up current git hash for version number")
            return "dev-unknown"
