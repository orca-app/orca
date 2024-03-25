import traceback
import subprocess


errors = []
warnings = []


class Entry:
    def __init__(self, msg):
        self.msgs = [msg]

    def more(self, *msgs):
        if len(msgs) == 0:
            msgs = [""]
        for msg in msgs:
            self.msgs.append(msg)

def log_error(msg):
    msg = f"ERROR: {msg}"
    entry = Entry(msg)
    errors.append(entry)
    return entry


def log_warning(msg):
    msg = f"WARNING: {msg}"
    entry = Entry(msg)
    warnings.append(entry)
    return entry


def log_finish(success):
    if success and len(errors) + len(warnings) == 0:
        print("Task completed successfully.")
        return

    print()

    result_str = "succeeded" if success else "failed"
    errors_str = "1 error" if len(errors) == 1 else f"{len(errors)} errors"
    warnings_str = "1 warning" if len(warnings) == 1 else f"{len(warnings)} warnings"

    if len(errors) > 0 and len(warnings) > 0:
        print(f"Task {result_str} with {errors_str} and {warnings_str}:")
        for entry in warnings:
            print("\n".join(entry.msgs))
        for entry in errors:
            print("\n".join(entry.msgs))
    elif len(errors) > 0:
        print(f"Task {result_str} with {errors_str}:")
        for entry in errors:
            print("\n".join(entry.msgs))
    elif len(warnings) > 0:
        print(f"Task {result_str} with {warnings_str}:")
        for entry in warnings:
            print("\n".join(entry.msgs))


def shellish(func):
    def shellfunc(*args, **kwargs):
        exitcode = 0
        try:
            func(*args, **kwargs)
        except subprocess.CalledProcessError as err:
            msg = log_error(f"The following command failed with code {err.returncode}:")
            msg.more(" ".join(err.cmd))
            exitcode = err.returncode
        except SystemExit as err:
            exitcode = err.code
        except Exception as err:
            log_error(err)
            print(traceback.format_exc())
            exitcode = 1
        except:
            print("something went REALLY wrong and also Ben does not know how to handle Python errors")
            exitcode = 1
        finally:
            log_finish(exitcode == 0)
            if exitcode == 0 and len(errors):
                exitcode = 1
            exit(exitcode)
    return shellfunc
