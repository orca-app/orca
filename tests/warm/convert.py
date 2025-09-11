import os
import subprocess
import glob
import argparse
import json

parser = argparse.ArgumentParser()
parser.add_argument("testsuite")

args = parser.parse_args()

for wast in glob.glob(os.path.join(args.testsuite, "*.wast")):
    jsonPath = os.path.join(os.path.splitext(wast)[0] + ".json")
    try:
        subprocess.run(['wast2json', wast, '-o', jsonPath], check=True)

    except subprocess.CalledProcessError:
        print(f"ERROR: Couldn't convert {wast} to json")
