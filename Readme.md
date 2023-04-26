# Install

Clone the repo with the `--recurse-submodules` option.

Get and build ANGLE (see `milepost/ext/angle_install_notes.md`), and put `libEGL.dylib` and `libGLESv2.dylib` in `milepost/bin`.

Cd to orca and build milepost (the platform layer), wasm3 (the wasm runtime), then orca:

```
cd Orca
./build.sh milepost
./build.sh wasm3
./build.sh orca
```

Build the sample orca app:
```
pushd samples/pong ; ./build.sh ; popd
```
This creates a `Pong.app` bundle in `samples/pong` that you can double click to run.

You can also build milepost example apps like so:

```
cd milepost
pushd examples/canvas ; ./build.sh ; popd
pushd examples/perf_text ; ./build.sh ; popd
pushd examples/tiger ; ./build.sh ; popd
``` 
