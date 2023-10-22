#!/usr/bin/env python

import platform
import os
import subprocess

os.makedirs("build", exist_ok=True)

res = subprocess.run(["git", "rev-parse", "--short", "HEAD"], check=True, capture_output=True, text=True)
githash = res.stdout.strip()

outname = "orca.exe" if platform.system() == "Windows" else "orca"

subprocess.run([
    "clang",
    "-std=c11",
    "-I", "..",
    "-D", "FLAG_IMPLEMENTATION",
    "-D", f"ORCA_TOOL_VERSION={githash}",
    "-MJ", "build/main.json",
    "-o", f"build/{outname}",
    "main.c",
], check=True)

with open("build/compile_commands.json", "w") as f:
    f.write("[\n")
    with open("build/main.json") as m:
        f.write(m.read())
    f.write("]")
