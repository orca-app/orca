#!/usr/bin/env python3

import argparse
import sys

from .dev import attach_dev_commands


parser = argparse.ArgumentParser()
parser.add_argument("-?", action="help", help=argparse.SUPPRESS)

subparsers = parser.add_subparsers(required=True, title="commands")
attach_dev_commands(subparsers)

# Hack to run -h by default if no arguments are provided
argv = sys.argv[1:]
if len(argv) == 0:
    argv = ["-h"]

args = parser.parse_args(argv)
args.func(args)
