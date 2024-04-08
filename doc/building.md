The following instructions are only relevant for those who want to develop the orca runtime or orca cli tool.

### Requirements

All of the installation requirements for regular users also apply for developers, with these additions:

- [Python 3.10](https://www.python.org/) or newer
- MSVC (Visual Studio 2022 17.5 or newer) (Windows only)
	- This can be installed through the [Visual Studio Community](https://visualstudio.microsoft.com/) installer. Ensure that your Visual Studio installation includes "Desktop development with C++".
	- Please note the version requirement! Orca requires C11 atomics, which were only added to MSVC in late 2022.
- Xcode command-line tools (Mac only)
	- These can be installed with `xcode-select --install`.

### Building 

TODO

### FAQ

**I am getting errors about atomics when building the runtime on Windows.**

Please ensure that you have the latest version of Visual Studio and MSVC installed. The Orca runtime requires the use of C11 atomics, which were not added to MSVC until late 2022.
