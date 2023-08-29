import glob
import os
import shutil

from contextlib import contextmanager

@contextmanager
def pushd(new_dir):
    previous_dir = os.getcwd()
    os.chdir(new_dir)
    try:
        yield
    finally:
        os.chdir(previous_dir)


def removeall(dir):
    [os.remove(f) for f in glob.iglob("{}/*".format(dir), recursive=True)]
    os.removedirs(dir)


def yeetdir(path):
    os.makedirs(path, exist_ok=True)
    shutil.rmtree(path)


def yeetfile(path):
    if os.path.exists(path):
        os.remove(path)
