import hashlib
import json
import os

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


# -----------------------------------------------------------------------------
# Directory-hashing implementation pulled from the checksumdir package on pypi.
# Licensed under the MIT license.
# -----------------------------------------------------------------------------

def dirsum(
    dirname,
    hash_func=hashlib.sha1,
    excluded_files=None,
    excluded_dirs=None,
    ignore_hidden=False,
    followlinks=False,
    excluded_extensions=None,
    include_paths=False
):
    if not excluded_files:
        excluded_files = []

    if not excluded_dirs:
        excluded_dirs = []

    if not excluded_extensions:
        excluded_extensions = []

    if not os.path.isdir(dirname):
        raise TypeError("{} is not a directory.".format(dirname))

    hashvalues = []
    for root, dirs, files in os.walk(dirname, topdown=True, followlinks=followlinks):
        if ignore_hidden and re.search(r"/\.", root):
            continue

        skip_dir = False
        parent_dir = root
        while parent_dir:
            if parent_dir in excluded_dirs:
                skip_dir = True
                break
            parent_dir = os.path.dirname(parent_dir)
        if skip_dir:
            continue

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

