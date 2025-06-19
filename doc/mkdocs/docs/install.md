# Installation

## Requirements

- **Operating System:** Windows 10 or later, or Mac 13 or later (Linux is not yet supported)
- **Compiler:** Clang version 11.0 or newer
	- Windows users: `clang` can be installed via the Visual Studio installer. Search for "C++ Clang Compiler".
	- Mac users: Apple's built-in `clang` does not support WebAssembly. We recommend installing `clang` via [Homebrew](https://brew.sh/) with `brew install llvm`.
- **Clang runtime builtins:** When targeting WebAssembly, `clang` relies on builtins found in `libclang_rt.builtins-wasm32`, but most distributions of `clang` don't ship with this file. To know where `clang` expects to find this file, you can run `clang --target=wasm32 -print-libgcc-file-name`. If this file doesn't exist you will need to download it from [https://github.com/WebAssembly/wasi-sdk/releases](https://github.com/WebAssembly/wasi-sdk/releases). 

## Installation Instructions

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

- **Windows Instructions:** [https://learn.microsoft.com/en-us/previous-versions/office/developer/sharepoint-2010/ee537574(v=office.14)](https://learn.microsoft.com/en-us/previous-versions/office/developer/sharepoint-2010/ee537574(v=office.14))  
- **Mac Instructions:** [https://support.apple.com/guide/terminal/use-environment-variables-apd382cc5fa-4f58-4449-b20a-41c53c006f8f/mac](https://support.apple.com/guide/terminal/use-environment-variables-apd382cc5fa-4f58-4449-b20a-41c53c006f8f/mac)

Finally, verify that Orca is successfully installed by running the `orca version` command.

```
orca version
```

If you encounter any errors, see the [FAQ](./faq.md).

Once the `orca` tools are installed and on your PATH, you can use them from anywhere.

## Building the sample apps

The `samples` directory contains several sample apps that demonstrate various Orca features. To build one, `cd` to a sample project's directory and run its build script. For example, for the `breakout` sample:

```
cd samples/breakout
# Windows
build.bat
# Mac
./build.sh
```

On Windows this creates a `Breakout` directory in `samples/breakout`. You can launch the app by running `Breakout/bin/Breakout.exe`. On macOS this creates a `Breakout.app` bundle in `samples/breakout` that you can double-click to run.
