------
**DISCLAIMER: This project is very much a Work In Progress. Expect bugs, missing and/or incomplete features, unstable APIs, and sparse documentation. Some current issues might be a show stopper for you, so make sure you can build and run the sample apps before jumping in.**

**If you do choose to try out Orca anyway, thank you! We'll do our best to answer your questions, and we'd really appreciate your feedback!**

------

# Orca

![Example Orca apps](doc/images/orca-apps-lg.webp)

Orca is a development platform and runtime environment for cross-platform, sandboxed graphical applications. In this early MVP you can:

- Receive mouse and keyboard input.
- Draw paths, images and text using a 2D vector graphics API.
- Draw 2D/3D graphics using OpenGL ES 3.1 (minus a few features like mapped buffers)
- Build user interfaces using our UI API and default widgets.
- Read and write files using a capability-based API.

To learn more about the project and its goals, read the [announcement post](https://orca-app.dev/posts/230607/orca_announcement.html).

## Installing

The Orca command-line tools must be installed to your system in order to use them in your own projects.

### Requirements

- Windows 10 or later, or Mac 13 or later (Linux is not yet supported)
- Clang version 11.0 or newer
	- **Windows users:** `clang` can be installed via the Visual Studio installer. Search for "C++ Clang Compiler".
	- **Mac users:** Apple's built-in `clang` does not support WebAssembly. We recommend installing `clang` via [Homebrew](https://brew.sh/) with `brew install llvm`.
- **Clang runtime builtins.** When targeting WebAssembly, `clang` relies on builtins found in `libclang_rt.builtins-wasm32`, but most distributions of `clang` don't ship with this file. To know where `clang` expects to find this file, you can run `clang --target=wasm32 -print-libgcc-file-name`. If this file doesn't exist you will need to download it from https://github.com/WebAssembly/wasi-sdk/releases. 

### Installation Instructions

Download the orca tool and SDK from https://github.com/orca-app/orca/releases/latest, and put the orca folder where you want orca to be installed.

- **Windows:**  
	- Download `orca-windows.tar.gz`  
	- Extract: `tar -xzf orca-windows.tar.gz`

- **ARM Mac:**  
	- Download `orca-mac-arm64.tar.gz`  
	- Extract: `tar -xzf orca-mac-arm64.tar.gz`

- **Intel Mac:**  
	- Download `orca-mac-x64.tar.gz`  
	- Extract: `tar -xzf orca-mac-x64.tar.gz`

Add the orca directory to your PATH environment variable:  

- **Windows Instructions:** https://learn.microsoft.com/en-us/previous-versions/office/developer/sharepoint-2010/ee537574(v=office.14)
- **Mac Instructions:** https://support.apple.com/guide/terminal/use-environment-variables-apd382cc5fa-4f58-4449-b20a-41c53c006f8f/mac

Finally, verify that Orca is successfully installed by running the `orca version` command.

```
orca version
```

If you encounter any errors, see the FAQ below.

Once the `orca` tools are installed and on your PATH, you can use them from anywhere.

### Building the sample Orca apps

The `samples` directory contains several sample apps that demonstrate various Orca features. To build one, `cd` to a sample project's directory and run its build script. For example, for the `breakout` sample:

```
cd samples/breakout
# Windows
build.bat
# Mac
./build.sh
```

On Windows this creates a `Breakout` directory in `samples/breakout`. You can launch the app by running `Breakout/bin/Breakout.exe`. On macOS this creates a `Breakout.app` bundle in `samples/breakout` that you can double-click to run.

## Writing an Orca app

Orca apps are WebAssembly modules that use the Orca APIs. The process for creating an Orca application is:

1. Compile a WebAssembly module using your language and toolchain of choice.
2. Bundle the WebAssembly module into a native executable using the Orca command-line tools.

For a more thorough overview, please read the [Quick Start Guide](./doc/QuickStart.md), which will walk you through building a simple application.

The following additional resources may also help you familiarize yourself with Orca and its APIs:

- The [samples folder](./samples) contains sample applications that show various aspects of the Orca API and core library:
	- [`breakout`](./samples/breakout) is a small breakout game making use of the vector graphics API.
	- [`clock`](./samples/clock) is a simple clock showcasing vector graphics and the time API.
	- [`triangle`](./samples/triangle) shows how to draw a spinning triangle using the GLES API.
	- [`fluid`](./samples/fluid) is a fluid simulation using a more complex GLES setup.
	- [`ui`](./samples/ui) showcases the UI API and Orca's default UI widgets.
- The [API Cheatsheets](./doc/cheatsheets) provide a list of Orca API functions, grouped by topic.

## Building Orca from source

See [./doc/building.md](./doc/building.md).


## FAQ

**What platforms does Orca support?**

We currently support Windows 10 and up, and macOS 13 and up. We plan to expand to more platforms in the future.

**What languages can I use with Orca?**

In principle, you can use any language and toolchain that can produce a WebAssembly module and bind to the Orca APIs. However, several important parts of Orca, such as the UI, are provided as part of the core library and are written in C. Therefore, at this early stage, it may be difficult to use any language other than C or C-style C++.

We're currently working with contributors to add support for Odin and Zig, and we look forward to expanding the number of officially-supported languages in the future.

**Which WebAssembly features does Orca support?**

We currently use [wasm3](https://github.com/wasm3/wasm3) for our interpreter. We therefore support whatever features wasm3 supports. In practice this means all WebAssembly 1.0 features, bulk memory operations, and a couple other small features.


**I am getting errors saying that `orca` is not found.**

Please ensure that you have installed Orca to your system per the installation instructions above. Please also ensure that the Orca install directory is on your PATH.

**I am getting errors from wasm-ld saying libclang_rt.builtins-wasm32.a is not found.**

Please ensure that you downloaded and installed `libclang_rt.builtins-wasm32.a` into clang's library directory as per the requirements instructions above.

## License

Orca is distributed under the terms of the  MIT license or the Apache License version 2.0, at your option. Copyright and License details can be found in [LICENSE.txt](./LICENSE.txt)
