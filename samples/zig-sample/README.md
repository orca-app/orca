### Build and run
Zig version `0.11.0` or greater is required for this sample. To build and run:
```
orca dev build-runtime
zig build run
```

These two commands build the runtime - the native host executable - and the sample as a loadable wasm library, then runs it. To only build the sample without running it, use `zig build bundle`.

### Warning
Zig integration is still in-progress and experimental. You may encounter bugs since not all the bound APIs have been tested extensively - this sample is currently the only code doing so! Additionally, not all APIs have zig coverage yet, notably:
* `oc_str8`, `oc_str16`, `oc_str32`: partial coverage
* `oc_ui`: none
* `gles`: none

Please report any bugs you find on the Handmade discord in the #orca channel.
