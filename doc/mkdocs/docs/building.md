# Building

The following instructions are only relevant for those who want to develop the orca runtime or orca cli tool.

## Requirements

All of the installation requirements for regular users also apply for developers, with these additions:

- [Zig 0.15.2](https://ziglang.org/download/)
- [Python 3.10](https://www.python.org/) or newer (this is only needed to build the documentation and run some helper scripts, not to build Orca itself)

## Building from source

First clone the orca repo: `git clone https://github.com/orca-app/orca.git`

#### Building Orca

The build system is setup as a one-click system to build everything needed and install it to the system orca directory:

```
cd path/to/orca
zig build
```

#### Installing a dev version of the tooling and SDK

The output of the build process is in the `zig-out` folder. On macOS, this is an application bundle name `Orca.app`. On Windows, this is a folder named `orca` containing the executable and the DLLs and resources needed to run it. Make sure the path to the Orca executable is in your `PATH` environment variable. Note that on macOS, the command line executable is inside the app bundle, in `Orca.app/Contents/macOS`.
