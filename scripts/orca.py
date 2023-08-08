#!/usr/bin/env python3

import argparse

from .bundle import attach_bundle_commands
from .dev import attach_dev_commands

parser = argparse.ArgumentParser()

subparsers = parser.add_subparsers(required=True, title='commands')
attach_bundle_commands(subparsers)
attach_dev_commands(subparsers)

args = parser.parse_args()
args.func(args)
