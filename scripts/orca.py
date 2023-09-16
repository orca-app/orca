#!/usr/bin/env python3

import argparse
import sys

from .bundle import attach_bundle_commands
from .source import attach_source_commands
from .dev import attach_dev_commands
from .version import attach_version_command

parser = argparse.ArgumentParser()
parser.add_argument("-?", action="help", help=argparse.SUPPRESS)

subparsers = parser.add_subparsers(required=True, title="commands")
attach_bundle_commands(subparsers)
attach_source_commands(subparsers)
attach_dev_commands(subparsers)
attach_version_command(subparsers)

# Hack to run the actual version command if we pass -v or --version.
# Using argparse action="version" requires us to pass a single string
# and doesn't allow us to run our own custom version-printing function.
argv = sys.argv[1:]

if len(argv) == 0:
    argv = ["-h"]
elif argv[0] in ["-v", "--version"]:
    argv = ["version"]

args = parser.parse_args(argv)
args.func(args)
