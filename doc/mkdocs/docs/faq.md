# FAQ

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