------
**DISCLAIMER: This project is very much a Work In Progress. Expect bugs, missing and/or incomplete features, unstable APIs, and sparse documentation. Some current issues might be a show stopper for you, so make sure you can build and run the sample apps before jumping in.**

**If you do choose to try out Orca anyway, thank you! We'll do our best to answer your questions, and we'd really appreciate your feedback!**

------

# Orca

![Example Orca apps](doc/images/orca-apps-lg.webp)

Orca is a development and runtime environment for building and distributing portable graphical applications. Orca applications can be written in C, Odin, or Zig (or any language that can compile to WebAssembly and has bindings for the Orca SDK), and run on our custom runtime outside the browser. The goal is to provide the same benefits as web apps, such as ease of distribution, portability and sandboxing, without the weight of a whole web browser and the complexity brought by the web stack.

But more than a portability layer, Orca's goal is to provide a better development and deployment platform, where you can interactively build apps in a live, observable environment, and share them as easily as sending a URL.

To learn more about the project and its goals, read the [announcement post](https://orca-app.dev/posts/230607/orca_announcement.html).

**In this very early MVP you can:**

- Receive mouse and keyboard input.
- Draw paths, images and text using a 2D vector graphics API.
- Draw 2D/3D graphics using OpenGL ES 3.1 (minus a few features like mapped buffers)
- Build user interfaces using our UI API and default widgets.
- Read and write files using a capability-based API.


## Useful Links

- [Orca website](https://orca-app.dev)
- [Orca Documentation](https://docs.orca-app.dev)
- [Orca Discord Server](https://discord.gg/t9GFHbh6)
- [Newsletter](https://orca-app.dev/newsletter.html)
- [Sponsor](https://github.com/sponsors/orca-app)

## Installing

The Orca command-line tools must be installed to your system in order to use them in your own projects.

### Requirements

- Windows 10 or later, or Mac 14 or later (Linux is not yet supported)

- Clang version 11.0 or newer
	- **Windows users:** `clang` can be installed via the Visual Studio installer. Search for "C++ Clang Compiler".
	- **Mac users:** Apple's built-in `clang` does not support WebAssembly. We recommend installing `clang` via [Homebrew](https://brew.sh/) with `brew install llvm`.
- **Clang runtime builtins.** When targeting WebAssembly, `clang` relies on builtins found in `libclang_rt.builtins-wasm32`, but most distributions of `clang` don't yet ship with this file. To know where `clang` expects to find this file, you can run `clang --target=wasm32 -print-libgcc-file-name`. If this file doesn't exist you will need to download it from [https://github.com/WebAssembly/wasi-sdk/releases](https://github.com/WebAssembly/wasi-sdk/releases). 

### Installation Instructions

Download the orca tool and SDK from [https://github.com/orca-app/orca/releases/latest](https://github.com/orca-app/orca/releases/latest), and put the orca folder where you want orca to be installed.

- **Windows:**  
	- Download `orca-windows.tar.gz`  
	- Extract: `tar -xzf orca-windows.tar.gz`

- **ARM Mac:**  
	- Download `orca-mac-arm64.tar.gz`  
	- Extract: `tar -xzf orca-mac-arm64.tar.gz`

- **Intel Mac:**  
	- Download `orca-mac-x64.tar.gz`  
	- Extract: `tar -xzf orca-mac-x64.tar.gz`

Add the orca tool directory to your PATH environment variable:  

- **Windows Instructions:** [https://learn.microsoft.com/en-us/previous-versions/office/developer/sharepoint-2010/ee537574(v=office.14)](https://learn.microsoft.com/en-us/previous-versions/office/developer/sharepoint-2010/ee537574(v=office.14))
- **Mac Instructions:** [https://support.apple.com/guide/terminal/use-environment-variables-apd382cc5fa-4f58-4449-b20a-41c53c006f8f/mac](https://support.apple.com/guide/terminal/use-environment-variables-apd382cc5fa-4f58-4449-b20a-41c53c006f8f/mac)

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

For a more thorough overview, please read the [Quick Start Guide](./doc/mkdocs/docs/QuickStart.md), which will walk you through building a simple application.

The following additional resources may also help you familiarize yourself with Orca and its APIs:

- The [samples folder](./samples) contains sample applications that show various aspects of the Orca API and core library:
	- [`breakout`](./samples/breakout) is a small breakout game making use of the vector graphics API.
	- [`clock`](./samples/clock) is a simple clock showcasing vector graphics and the time API.
	- [`triangle`](./samples/triangle) shows how to draw a spinning triangle using the GLES API.
	- [`fluid`](./samples/fluid) is a fluid simulation using a more complex GLES setup.
	- [`ui`](./samples/ui) showcases the UI API and Orca's default UI widgets.
- The [Online Documentation](https://docs.orca-app.dev) provide a list of Orca API functions, grouped by topic.

## Building Orca from source

See [Building.md](./doc/mkdocs/docs/building.md).

## Supporting

You can support Orca by donating monthly through [Github Sponsors](https://github.com/sponsors/orca-app). 

## Contributing

We welcome contributions of all kinds from everyone!

Before contributing to the Orca project, please note that the design and implementation of Orca is ultimately decided by me (Martin Fouilleul), and that I have the last word on everything that goes into the project. Since your time is valuable, I kindly ask you to get in touch and discuss the tasks you intend to tackle _before_ expanding substantial amounts of work in a PR or proposal, to avoid duplicated or wasted efforts. 

You can help us in a variety of ways:

- A good way to start contributing is by trying to make a small Orca app and sending us feedback. 
- Spread the word around you and make more people aware of Orca.
- Submit bug reports and feature proposals.
- Contribute code changes. Issues labeled as [`good first issue`](https://github.com/orca-app/orca/issues?q=state%3Aopen%20label%3A%22good%20first%20issue%22) are a good starting point to familiarize yourself with the internals of Orca.
- You can also contribute by fleshing out the [documentation](https://docs.orca-app.dev/documentation/).

Please read [Contributing](./Contributing.md) for more information and guidelines on how you can help!

**Do not use LLMs to submit issues, comments or PRs.**


## FAQ

**What platforms does Orca support?**

We currently support Windows 10 and up, and macOS 14 and up. We plan to expand to more platforms in the future.

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
