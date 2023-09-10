------
**DISCLAIMER: This project is very much a Work In Progress. We're making it accessible in this very early state so that participants to the [Wheel Reinvention Jam 2023](https://handmade.network/jam/2023) can try it out and maybe use it as their jamming platform. Expect bugs, missing and/or incomplete features, unstable APIs, and sparse documentation. Some current issues might be a show stopper for you, so make sure you can build and run the sample apps before jumping in.**

**If you do choose to try out Orca anyway, well thanks! We'll do our best to answer your questions, and we'd really appreciate to hear your feedback!**

------

# Orca
---

Orca is a devlopment platform and runtime environment for cross-platform, sandboxed graphical WebAssembly applications. In this early MVP you can:

- Receive mouse and keyboard input
- Draw paths, images and text using a 2D vector graphics API.
- Draw 2D/3D graphics using OpenGL ES 3.1 (minus a few features)
- Build user interfaces using our UI API and default widgets.
- Read and write files using a capability based API. 

## Installing

_//TODO: Ben can you complete this section?_

Clone the repo: `git clone git@git.handmade.network:hmn/orca.git`.

Cd to the orca directory and build the Orca runtime:

```
cd orca
./orca dev build-runtime
```

Install the orca dev tools:

```
./orca dev install
```

### Building the sample Orca apps

Cd to the sample project directory and run its build script:

```
cd samples/pong
./build.sh
```

On macOS this creates a `Pong.app` bundle in `samples/pong` that you can double click to run. On Windows this creates a `Pong` directory in `samples/pong`. You can launch the app by running `Pong/bin/Pong.exe`.

## Writing an Orca app

The following documents can help you write an application using the Orca APIs:

- The [Quick Start Guide](./doc/QuickStart.md) will walk you through writing and building a simple example application.
- The [samples folder](./samples) contains sample applications that show various aspects of the Orca API and support library:
	- [clock](./samples/clock) is a simple clock showcasing vector graphics and the time API. 
	- [breakout](./samples/breakout) is a mini breakout game making use of the vector graphics API.
	- 	[triangle](./samples/triangle) shows how to draw a spining triangle using the GLES API.
	-  [fluid](./samples/fluid) is a fluid simulation using a more complex GLES setup.
	-  [ui](./samples/ui) showcases the UI API and Orca's default UI widgets.  
- The [API Cheatsheets](./doc/cheatsheets) provide a list of Orca API functions, grouped by topic.  

## Building and bundling an Orca app

_//TODO: Ben, can you complete this section? Or should that be in the QuickStart guide?_

You must compile your application along with the Orca support code, into a WebAssembly module. The command `orca src cflags` can help you set up your compiler's flags to do so.

Once you have built your WebAssembly module, you can invoke the command `orca bundle` to bundle it with your apps resources and the Orca runtime to produce an application.

## License

Orca is distributed under the terms of the GNU Affero General Public License version 3, with additional terms in accordance with section 7 of AGPLv3. These additional terms ensure that:

- Modified versions of Orca must reasonably inform users that they are modified.
- You can distribute your application's WebAssembly modules under the terms of your choice, and are not required to license them under the terms of the AGPLv3.

Copyright and License details can be found in [LICENSE.txt](./LICENSE.txt)

