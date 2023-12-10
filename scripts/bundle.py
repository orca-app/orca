#!/usr/bin/env python3

import os
import platform
import shutil
import subprocess
from argparse import ArgumentParser

from .log import *


def attach_bundle_commands(subparsers):
	mkapp_cmd = subparsers.add_parser("bundle", help="Package a WebAssembly module into a standalone Orca application.")
	init_parser(mkapp_cmd)


def init_parser(parser):
	parser.add_argument("-d", "--resource", action="append", dest="resource_files", help="copy a file to the app's resource directory")
	parser.add_argument("-D", "--resource-dir", action="append", dest="resource_dirs", help="copy a directory to the app's resource directory")
	parser.add_argument("-i", "--icon", help="an image file to use as the application's icon")
	parser.add_argument("-C", "--out-dir", default=os.getcwd(), help="where to place the final application bundle (defaults to the current directory)")
	parser.add_argument("-n", "--name", default="out", help="the app's name")
	parser.add_argument("-O", "--orca-dir", default=".")
	parser.add_argument("--version", default="0.0.0", help="a version number to embed in the application bundle")
	parser.add_argument("--mtl-enable-capture", action='store_true', help="Enable Metal frame capture for the application bundle (macOS only)")
	parser.add_argument("module", help="a .wasm file containing the application's wasm module")
	parser.set_defaults(func=shellish(make_app))


def make_app(args):
	#-----------------------------------------------------------
	# Dispatch to platform-specific function
	#-----------------------------------------------------------
	platformName = platform.system()
	if platformName == 'Darwin':
		macos_make_app(args)
	elif platformName == 'Windows':
		windows_make_app(args)
	elif platformName == 'Linux':
		linux_make_app(args)
	else:
		log_error("Platform '" +  platformName + "' is not supported for now...")
		exit(1)


def macos_make_app(args):
	#-----------------------------------------------------------
	#NOTE: make bundle directory structure
	#-----------------------------------------------------------
	app_name = args.name
	bundle_name = app_name + '.app'
	bundle_path = os.path.join(args.out_dir, bundle_name)
	contents_dir = os.path.join(bundle_path, 'Contents')
	exe_dir = os.path.join(contents_dir, 'MacOS')
	res_dir = os.path.join(contents_dir, 'resources')
	guest_dir = os.path.join(contents_dir, 'app')
	wasm_dir = os.path.join(guest_dir, 'wasm')
	data_dir = os.path.join(guest_dir, 'data')

	if os.path.exists(bundle_path):
		shutil.rmtree(bundle_path)
	os.mkdir(bundle_path)
	os.mkdir(contents_dir)
	os.mkdir(exe_dir)
	os.mkdir(res_dir)
	os.mkdir(guest_dir)
	os.mkdir(wasm_dir)
	os.mkdir(data_dir)

	#-----------------------------------------------------------
	#NOTE: copy orca runtime executable and libraries
	#-----------------------------------------------------------
	orca_exe = os.path.join(args.orca_dir, 'build/bin/orca_runtime')
	orca_lib = os.path.join(args.orca_dir, 'build/bin/liborca.dylib')
	gles_lib = os.path.join(args.orca_dir, 'src/ext/angle/bin/libGLESv2.dylib')
	egl_lib = os.path.join(args.orca_dir, 'src/ext/angle/bin/libEGL.dylib')
	renderer_lib = os.path.join(args.orca_dir, 'build/bin/mtl_renderer.metallib')

	shutil.copy(orca_exe, exe_dir)
	shutil.copy(orca_lib, exe_dir)
	shutil.copy(gles_lib, exe_dir)
	shutil.copy(egl_lib, exe_dir)
	shutil.copy(renderer_lib, exe_dir)

	#-----------------------------------------------------------
	#NOTE: copy wasm module and data
	#-----------------------------------------------------------
	shutil.copy(args.module, os.path.join(wasm_dir, 'module.wasm'))

	if args.resource_files != None:
		for resource in args.resource_files:
			shutil.copytree(resource, os.path.join(data_dir, os.path.basename(resource)), dirs_exist_ok=True)

	if args.resource_dirs != None:
		for resource_dir in args.resource_dirs:
			for resource in os.listdir(resource_dir):
				src = os.path.join(resource_dir, resource)
				if os.path.isdir(src):
					shutil.copytree(src, os.path.join(data_dir, os.path.basename(resource)), dirs_exist_ok=True)
				else:
					shutil.copy(src, data_dir)

	#-----------------------------------------------------------
	#NOTE: copy runtime resources
	#-----------------------------------------------------------
	# default fonts
	shutil.copy(os.path.join(args.orca_dir, 'resources/Menlo.ttf'), res_dir)
	shutil.copy(os.path.join(args.orca_dir, 'resources/Menlo Bold.ttf'), res_dir)

	#-----------------------------------------------------------
	#NOTE make icon
	#-----------------------------------------------------------
	src_image = args.icon

	#if src_image == None:
	#	src_image = orca_dir + '/resources/default_app_icon.png'

	if src_image != None:
		iconset = os.path.splitext(src_image)[0] + '.iconset'

		if os.path.exists(iconset):
			shutil.rmtree(iconset)

		os.mkdir(iconset)

		size = 16
		for i in range(0, 7):
			size_str = str(size)
			icon = 'icon_' + size_str + 'x' + size_str + '.png'
			subprocess.run(['sips', '-z', size_str, size_str, src_image, '--out', iconset + '/' + icon],
		               	stdout = subprocess.DEVNULL,
		               	stderr = subprocess.DEVNULL)

			size_str_retina = str(size*2)
			icon = 'icon_' + size_str + 'x' + size_str + '@2x.png'
			subprocess.run(['sips', '-z', size_str_retina, size_str_retina, src_image, '--out', iconset + '/' + icon],
		               	stdout = subprocess.DEVNULL,
		               	stderr = subprocess.DEVNULL)

			size = size*2

		subprocess.run(['iconutil', '-c', 'icns', '-o', os.path.join(res_dir, 'icon.icns'), iconset])
		shutil.rmtree(iconset)

	#-----------------------------------------------------------
	#NOTE: write plist file
	#-----------------------------------------------------------
	version = args.version
	bundle_sig = "????"
	icon_file = ''

	plist_contents = f"""
	<?xml version="1.0" encoding="UTF-8"?>
	<!DOCTYPE plist PUBLIC "-//Apple//DTD PLIST 1.0//EN" "http://www.apple.com/DTDs/PropertyList-1.0.dtd">
	<plist version="1.0">
		<dict>
			<key>CFBundleName</key>
			<string>{app_name}</string>
			<key>CFBundleDisplayName</key>
			<string>{app_name}</string>
			<key>CFBundleIdentifier</key>
			<string>{app_name}</string>
			<key>CFBundleVersion</key>
			<string>{version}</string>
			<key>CFBundlePackageType</key>
			<string>APPL</string>
			<key>CFBundleSignature</key>
			<string>{bundle_sig}</string>
			<key>CFBundleExecutable</key>
			<string>orca_runtime</string>
			<key>CFBundleIconFile</key>
			<string>icon.icns</string>
			<key>NSHighResolutionCapable</key>
			<string>True</string>
	"""
	if args.mtl_enable_capture == True:
		plist_contents += f"""
			<key>MetalCaptureEnabled</key>
			<true/>"""

	plist_contents += f"""
	</dict>
	</plist>
	"""

	plist_file = open(contents_dir + '/Info.plist', 'w')
	print(plist_contents, file=plist_file)

def windows_make_app(args):
	#-----------------------------------------------------------
	#NOTE: make bundle directory structure
	#-----------------------------------------------------------
	app_name = args.name
	bundle_name = app_name
	bundle_dir = os.path.join(args.out_dir, bundle_name)
	exe_dir = os.path.join(bundle_dir, 'bin')
	res_dir = os.path.join(bundle_dir, 'resources')
	guest_dir = os.path.join(bundle_dir, 'app')
	wasm_dir = os.path.join(guest_dir, 'wasm')
	data_dir = os.path.join(guest_dir, 'data')

	if os.path.exists(bundle_dir):
		shutil.rmtree(bundle_dir)
	os.mkdir(bundle_dir)
	os.mkdir(exe_dir)
	os.mkdir(res_dir)
	os.mkdir(guest_dir)
	os.mkdir(wasm_dir)
	os.mkdir(data_dir)

	#-----------------------------------------------------------
	#NOTE: copy orca runtime executable and libraries
	#-----------------------------------------------------------
	orca_exe = os.path.join(args.orca_dir, 'build/bin/orca_runtime.exe')
	orca_lib = os.path.join(args.orca_dir, 'build/bin/orca.dll')
	gles_lib = os.path.join(args.orca_dir, 'src/ext/angle/bin/libGLESv2.dll')
	egl_lib = os.path.join(args.orca_dir, 'src/ext/angle/bin/libEGL.dll')

	shutil.copy(orca_exe, os.path.join(exe_dir, app_name + '.exe'))
	shutil.copy(orca_lib, exe_dir)
	shutil.copy(gles_lib, exe_dir)
	shutil.copy(egl_lib, exe_dir)

	#-----------------------------------------------------------
	#NOTE: copy wasm module and data
	#-----------------------------------------------------------

	shutil.copy(args.module, wasm_dir + '/module.wasm')

	if args.resource_files != None:
		for resource in args.resource_files:
			shutil.copytree(resource, data_dir + '/' + os.path.basename(resource), dirs_exist_ok=True)

	if args.resource_dirs != None:
		for resource_dir in args.resource_dirs:
			for resource in os.listdir(resource_dir):
				src = resource_dir + '/' + resource
				if os.path.isdir(src):
					shutil.copytree(src, data_dir + '/' + os.path.basename(resource), dirs_exist_ok=True)
				else:
					shutil.copy(src, data_dir)

	#-----------------------------------------------------------
	#NOTE: copy runtime resources
	#-----------------------------------------------------------
	# default fonts
	shutil.copy(os.path.join(args.orca_dir, 'resources/Menlo.ttf'), res_dir)
	shutil.copy(os.path.join(args.orca_dir, 'resources/Menlo Bold.ttf'), res_dir)

	#-----------------------------------------------------------
	#NOTE make icon
	#-----------------------------------------------------------
	#TODO

def linux_make_app(args):
	app_name = args.name
	bundle_name = app_name
	bundle_dir = os.path.join(args.out_dir, bundle_name)
	exe_dir = os.path.join(bundle_dir, 'bin')
	lib_dir = os.path.join(bundle_dir, 'lib')
	res_dir = os.path.join(bundle_dir, 'resources')
	guest_dir = os.path.join(bundle_dir, 'app')
	wasm_dir = os.path.join(guest_dir, 'wasm')
	data_dir = os.path.join(guest_dir, 'data')

	if os.path.exists(bundle_dir):
		shutil.rmtree(bundle_dir)
	os.mkdir(bundle_dir)
	os.mkdir(exe_dir)
	os.mkdir(res_dir)
	os.mkdir(guest_dir)
	os.mkdir(wasm_dir)
	os.mkdir(data_dir)

	orca_exe = os.path.join(args.orca_dir, 'build/bin/orca_runtime')

	shutil.copy(orca_exe, os.path.join(exe_dir, app_name))
	shutil.copy(args.module, os.path.join(wasm_dir, 'module.wasm'))

	if args.resource_files != None:
		for resource in args.resource_files:
			shutil.copytree(resource, os.path.join(data_dir, os.path.basename(resource)), dirs_exist_ok=True)
	if args.resource_dirs != None:
		for resource_dir in args.resource_dirs:
			for resource in os.listdir(resource_dir):
				src = os.path.join(resource_dir, resource)
				if os.path.isdir(src):
					shutil.copytree(src, os.path.join(data_dir, os.path.basename(resource)), dirs_exist_ok=True)
				else:
					shutil.copy(src, data_dir)
	shutil.copy(os.path.join(args.orca_dir, 'resources/Menlo.ttf'), res_dir)
	shutil.copy(os.path.join(args.orca_dir, 'resources/Menlo Bold.ttf'), res_dir)

if __name__ == "__main__":
	parser = ArgumentParser(prog='mkapp')
	init_parser(parser)

	args = parser.parse_args()
	make_app(args)
