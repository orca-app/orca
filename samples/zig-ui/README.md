### Build and run
Zig version `0.11.0` or greater is required for this sample. To build and run:
```
orca dev build-runtime
zig build run
```

These two commands build the runtime - the native host executable - and the sample as a loadable wasm library, then runs it. To only build the sample without running it, use `zig build bundle`.

The `build.zig` is set up such that `orca.zig` is the root file, and the sample's `main.zig` is a module. This is because `orca.zig` exports the C bindings based on handlers exposed in `main.zig`, which allows the zig handlers defined in user code to return errors if they wish. See the bottom of `orca.zig` for a full list of all supported handlers and their signatures.

### Warning
Zig bindings for Orca are in-progress and experimental. You may encounter bugs since not all the bound APIs have been tested extensively - this sample is currently the only code doing so! Additionally, not all APIs have zig coverage yet, notably:
* `gles`
As more APIs get tested, there is a possibility of breaking changes. Please report any bugs you find on the Handmade discord in the #orca channel.
