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


def onerror(func, path, exc_info):
    """
    Error handler for ``shutil.rmtree``.

    If the error is due to an access error (read only file)
    it attempts to add write permission and then retries.

    If the error is for another reason it re-raises the error.

    Usage : ``shutil.rmtree(path, onerror=onerror)``
    """
    import stat
    # Is the error an access error?
    if not os.access(path, os.W_OK):
        os.chmod(path, stat.S_IWUSR)
        func(path)
    else:
        raise

def yeetdir(path):
    os.makedirs(path, exist_ok=True)
    shutil.rmtree(path, onerror=onerror)

def yeetfile(path):
    if os.path.exists(path):
        os.remove(path)
