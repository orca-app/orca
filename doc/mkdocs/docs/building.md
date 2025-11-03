# Building

The following instructions are only relevant for those who want to develop the orca runtime or orca cli tool.

## Requirements

All of the installation requirements for regular users also apply for developers, with these additions:

- [Python 3.10](https://www.python.org/) or newer
- [Zig 0.15.2](https://ziglang.org/download/)

## Building from source

First clone the orca repo: `git clone https://github.com/orca-app/orca.git`

#### Building Orca

The build system is setup as a one-click system to build everything needed and install it to the system orca directory:

- `cd` to the orca directory and run `zig build`

#### Installing a dev version of the tooling and SDK

- If you have not yet setup Orca locally and added it to your path, you'll need to specify the directory to which it will install by running `zig build -Dsdk-path=<orca_install_path>`

- Make sure `<orca_install_path>` is in your `PATH` environment variable.

- Once you have the Orca tool in your `PATH`, you can install new dev versions to the same location by simply running `zig build`.

You can then use this dev version normally through the `orca` command line tool.
