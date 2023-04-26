## Angle install on macOS

* Get ninja if needed: `brew install ninja`
* Get the `depot_tools`repo: `git clone https://chromium.googlesource.com/* chromium/tools/depot_tools.git`
* Set path: `export PATH=/path/to/depot_tools:$PATH`

* Maybe necessary to fiddle with certificates here, otherwise `fetch angle` fails in the subsequent steps.

```
cd /Applications/Python\ 3.6
sudo ./Install\ Certificates.command
```
* Fetch angle:

```
mkdir angle 
cd angle
fetch angle
```
* Generate build config: `gn gen out/Debug`

	* To see available arguments: `gn args out/Debug --list`
	* To change arguments: `gn args out/Debug`

For example, to generate dwarf dsyms files, set:

```	
enable_dsyms=true
use_debug_fission=true
symbol_level=2
```

We also need to set `is_component_build=false` in order to have self-contained librarries.

Then, build with `autoninja -C out/Debug`and wait until you pass out.

## Angle install on windows

* need Python3 (can install through win app store)
* need Windows SDK
* clone `depot_tools`: `git clone https://chromium.googlesource.com/chromium/tools/depot_tools.git`
or download and unzip bundle at [https://storage.googleapis.com/chrome-infra/depot_tools.zip](https://storage.googleapis.com/chrome-infra/depot_tools.zip)
* set `depot_tools` in path env variable through control panel>System and security>system>advanced system settings
* run `gclient` in a cmd shell
* set `DEPOT_TOOLS_WIN_TOOLCHAIN=0`
* `mkdir angle`
* `cd angle`
* `fetch angle`
* wait a million years

* if it fails when running `python3 third_party/depot_tools/download_from_google_storage.py ...`
  -> open `DEPS` and change `third_party/depot_tools` to `../depot/tools`
* run `gclient sync` to complete previous step

* `gn gen out/Debug`
* `gn args out/Debug` and edit arguments:
	* `angle_enable_vulkan = false`
	* `angle_build_tests = false`
	* `is_component_build = false`

* link with `libEGL.dll.lib` and `libGLESv2.dll.lib`
* put `libEGL.dll` and `libGLESv2.dll` in same directory as executable

## To get debugging kinda working with renderdoc:

Run `gn args out/Debug` and set
	* `angle_enable_trace = true`
	* `angle_enable_annotator_run_time_checks = true`

* `autoninja -C out/Debug`
* wait a while

In renderdoc, set env variables
`RENDERDOC_HOOK_EGL 0` (if you want to trace underlying native API)
`RENDERDOC_HOOK_EGL 1` (if you want to trace EGL calls. You also need to put `libEGL` in the renderdoc folder so it's found when capturing stuff. Unfortunately though, that seems to provoke crashes...)

`ANGLE_ENABLE_DEBUG_MARKERS 1` (to turn on debug markers)
