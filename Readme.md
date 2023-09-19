------
**DISCLAIMER: This project is very much a Work In Progress. We are making it accessible in this very early state so that participants in the [2023 Wheel Reinvention Jam](https://handmade.network/jam/2023) can try it out and possibly use it as their jamming platform. Expect bugs, missing and/or incomplete features, unstable APIs, and sparse documentation. Some current issues might be a show stopper for you, so make sure you can build and run the sample apps before jumping in.**

**If you do choose to try out Orca anyway, thank you! We'll do our best to answer your questions, and we'd really appreciate your feedback!**

------

# Orca

![Example Orca apps](doc/images/orca-apps-lg.webp)

Orca is a development platform and runtime environment for cross-platform, sandboxed graphical WebAssembly applications. In this early MVP you can:

- Receive mouse and keyboard input.
- Draw paths, images and text using a 2D vector graphics API.
- Draw 2D/3D graphics using OpenGL ES 3.1 (minus a few features)
- Build user interfaces using our UI API and default widgets.
- Read and write files using a capability-based API.

To learn more about the project and its goals, read the [announcement post](https://orca-app.dev/posts/230607/orca_announcement.html).

## Installing

The Orca command-line tools must be installed to your system in order to use them in your own projects.

**At this early stage, you must build Orca yourself - in the future, there will be fewer dependencies and this installation process will be streamlined.**

### Requirements

- Windows or Mac (Linux is not yet supported)
- [Python 3.8](https://www.python.org/) or newer (for command line tools)
- Clang
	- **Windows users:** `clang` can be installed via the Visual Studio installer. Search for "C++ Clang Compiler".
	- **Mac users:** Apple's built-in `clang` does not support WebAssembly. We recommend installing `clang` via [Homebrew](https://brew.sh/) with `brew install llvm`.
- MSVC (Visual Studio 2022 17.5 or newer) (Windows only)
	- This can be installed through the [Visual Studio Community](https://visualstudio.microsoft.com/) installer. Ensure that your Visual Studio installation includes "Desktop development with C++".
	- Please note the version requirement! Orca requires C11 atomics, which were only added to MSVC in late 2022.

### Installation instructions

**Windows users:** You must perform all the following actions from a 64-bit Visual Studio command prompt. We recommend searching for "x64 Native Tools Command Prompt".

Clone the repo, then `cd` into the `orca` directory:

```
git clone https://git.handmade.network/hmn/orca.git
cd orca
```

Build the Orca runtime:

```
python orca dev build-runtime
```

Install the Orca dev tools. If on Windows, the tool can automatically add `orca` to your PATH. Otherwise, you must manually add the Orca install directory to your PATH, e.g. by updating `.zshrc` or `.bashrc`.

```
python orca dev install
```

Finally, verify that Orca is successfully installed by running the `orca version` command. Note the lack of `./`!

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

## FAQ

**What platforms does Orca support?**

We currently support Windows 10 and up, and macOS 10.15 and up. We plan to expand to more platforms in the future.

**What languages can I use with Orca?**

In principle, you can use any language and toolchain that can produce a WebAssembly module and bind to the Orca APIs. However, several important parts of Orca, such as the UI, are provided as part of the core library, which must be compiled to WebAssembly with your app, and is written in C. Therefore, at this early stage, it may be difficult to use any language other than C.

C-style C++ is possible but requires compiling the core library in C as a separate object file, and then adding that object to your compile command when building your app.

We're currently working with contributors to add support for Odin and Zig, and we look forward to expanding the number of officially-supported languages in the future. 

**Which WebAssembly features does Orca support?**

We currently use [wasm3](https://github.com/wasm3/wasm3) for our interpreter. We therefore support whatever features wasm3 supports. In practice this means all WebAssembly 1.0 features, bulk memory operations, and a couple other small features.

**I am getting "unsupported OS" errors when building on Windows.**

You are likely running from the wrong kind of Visual Studio command prompt. Search for "x64 Native Tools Command Prompt" or run `vcvarsall.bat` with `x64` for the architecture.

To verify that you are in the correct type of command prompt, simply run `cl` with no arguments, and verify that you are building for x64.

**I am getting errors about atomics when building the runtime on Windows.**

Please ensure that you have the latest version of Visual Studio and MSVC installed. The Orca runtime requires the use of C11 atomics, which were not added to MSVC until late 2022.

**I am getting errors saying that `orca` is not found.**

Please ensure that you have installed Orca to your system per the installation instructions above. Please also ensure that the Orca install directory is on your PATH. The installation path is printed when running `./orca dev install`.

## License

Orca is distributed under the terms of the GNU Affero General Public License version 3, with additional terms in accordance with section 7 of AGPLv3. These additional terms ensure that:

- Modified versions of Orca must reasonably inform users that they are modified.
- You can distribute your application's WebAssembly modules under the terms of your choice, and are not required to license them under the terms of the AGPLv3.

Copyright and License details can be found in [LICENSE.txt](./LICENSE.txt)
