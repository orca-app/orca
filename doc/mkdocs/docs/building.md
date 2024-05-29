# Building

The following instructions are only relevant for those who want to develop the orca runtime or orca cli tool.

## Requirements

All of the installation requirements for regular users also apply for developers, with these additions:

- [Python 3.10](https://www.python.org/) or newer
- MSVC (Visual Studio 2022 17.5 or newer) (Windows only)
	- This can be installed through the [Visual Studio Community](https://visualstudio.microsoft.com/) installer. Ensure that your Visual Studio installation includes "Desktop development with C++".
	- Please note the version requirement! Orca requires C11 atomics, which were only added to MSVC in late 2022.
- Xcode command-line tools (Mac only)
	- These can be installed with `xcode-select --install`.

## Building from source


First clone the orca repo: `git clone https://github.com/orca-app/orca.git`

#### Angle and Dawn

Orca depends on Angle for OpenGL ES, and Dawn for WebGPU. You can either build them locally, or grab a precompiled binary built in CI:

- To build locally: 
	- `cd` to the orca directory 
	- run `./orcadev build-angle --release`
	- run `./orcadev build-dawn --release`

- To use a precompiled binary: 
	- Go to [https://github.com/orca-app/orca/actions/workflows/build-all.yaml](https://github.com/orca-app/orca/actions/workflows/build-all.yaml)
	- Download the artifacts from a previous run. 
		- On ARM macs, download `angle-mac-arm64` and `dawn-mac-arm64`.
		- On Intel macs, download `angle-mac-x64` and `dawn-mac-x64`.
		- On Windows, download `angle-windows-x64` and `dawn-windows-x64`.  
	- Unzip the artifacts and put their content in the `build` directory at the root of the orca repo (you can create that directory if it doesn't exist).

You only need to do this once, until we change the Angle or Dawn versions we depend on.

#### Building Orca
	
- `cd` to the orca directory and run `./orcadev build` (macOS) or `orcadev build` (Windows)
- If this is the first time you build orca, and you have skipped the previous section, this will print a message telling you you first need to build Angle and Dawn. 

#### Installing a dev version of the tooling and SDK

- Inside the repo, run `./orcadev install directory`. This will install a dev version of the tooling and SDK into `directory`. 
- Make sure `directory` is in your `PATH` environment variable.

You can then use this dev version normally through the `orca` command line tool.


### FAQ

**I am getting errors about atomics when building the runtime on Windows.**

Please ensure that you have the latest version of Visual Studio and MSVC installed. The Orca runtime requires the use of C11 atomics, which were not added to MSVC until late 2022.
