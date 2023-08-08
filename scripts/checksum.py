import hashlib
import json

from .log import *


def checkfile(filepath):
    newsum = filesum(filepath)

    sums = {}
    with open("scripts/checksums.json", "r") as sumsfile:
        sums = json.loads(sumsfile.read())
    if filepath not in sums:
        msg = log_warning(f"no checksum saved for file {filepath}")
        msg.more(f"file had checksum: {newsum}")
        return False
    sum = sums[filepath]

    if sum != newsum:
        msg = log_warning(f"checksums did not match for {filepath}:")
        msg.more(f"expected: {sum}")
        msg.more(f"     got: {newsum}")
        return False

    return True


def filesum(filepath):
    with open(filepath, "rb") as file:
        return hashlib.sha256(file.read()).hexdigest()
