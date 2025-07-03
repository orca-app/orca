# Building

The following instructions are only relevant for those who want to develop the orca runtime or orca cli tool.

## Requirements

All of the installation requirements for regular users also apply for developers, with these additions:

- [Python 3.10](https://www.python.org/) or newer
- Zig 0.14.x

## Building from source

First clone the orca repo: `git clone https://github.com/orca-app/orca.git`

#### Building Orca

The build system is setup as a one-click system to build everything needed and install it to the system orca directory:

- `cd` to the orca directory and run `zig build`

#### Installing a dev version of the tooling and SDK

- If you have not yet setup Orca locally yet and added it to your path, you'll need to specify the the directory to which it will install by running `zig build -Dsdk-path=<orca_install_path>`

- Inside the repo, run `zig build -Dsdk-path=<orca_install_path>`. This will install a dev version of the tooling and SDK into `<orca_install_path>`. 
- Make sure `<orca_install_path>` is in your `PATH` environment variable.

You can then use this dev version normally through the `orca` command line tool.

### FAQ

**I am getting errors about atomics when building the runtime on Windows.**

Please ensure that you have the latest version of Visual Studio and MSVC installed. The Orca runtime requires the use of C11 atomics, which were not added to MSVC until late 2022.
