# Installing

Clone the repo: `git clone git@git.handmade.network:hmn/orca.git`.

Cd to orca and build the runtime:

```
cd orca
./orca dev build-runtime
```

Install the orca tools:

```
./orca dev install
```

# Building the sample orca apps:

Cd to the sample project directory and run its build script:

```
cd samples/pong
./build.sh
```

On macOS this creates a `Pong.app` bundle in `samples/pong` that you can double click to run.
On Windows this creates a `Pong` directory in `samples/pong`. You can launch the app by running `Pong/bin/orca.exe`.