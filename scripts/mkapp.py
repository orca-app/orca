#!/usr/bin/env python3

import os
import platform
import shutil
import subprocess
from argparse import ArgumentParser

def macos_make_app(args):
	#-----------------------------------------------------------
	#NOTE: make bundle directory structure
	#-----------------------------------------------------------
	app_name = args.name
	bundle_name = app_name + '.app'
	bundle_path = args.out_dir + '/' + bundle_name
	contents_dir = bundle_path + '/Contents'
	exe_dir = contents_dir + '/MacOS'
	res_dir = contents_dir + '/resources'
	guest_dir = contents_dir + '/app'
	wasm_dir = guest_dir + '/wasm'
	data_dir = guest_dir + '/data'

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
	orca_exe = args.orca_dir + '/bin/orca'
	milepost_lib = args.orca_dir + '/bin/libmilepost.dylib'
	gles_lib = args.orca_dir + '/bin/libGLESv2.dylib'
	egl_lib = args.orca_dir + '/bin/libEGL.dylib'
	renderer_lib = args.orca_dir + '/bin/mtl_renderer.metallib'

	shutil.copy(orca_exe, exe_dir)
	shutil.copy(milepost_lib, exe_dir)
	shutil.copy(gles_lib, exe_dir)
	shutil.copy(egl_lib, exe_dir)
	shutil.copy(renderer_lib, exe_dir)

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
	shutil.copy(args.orca_dir + '/resources/OpenSansLatinSubset.ttf', res_dir)
	shutil.copy(args.orca_dir + '/resources/Menlo.ttf', res_dir)
	shutil.copy(args.orca_dir + '/resources/Menlo Bold.ttf', res_dir)
	shutil.copy(args.orca_dir + '/resources/Menlo Italics.ttf', res_dir)

	#-----------------------------------------------------------
	#NOTE make icon
	#-----------------------------------------------------------
	src_image=args.icon

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

		subprocess.run(['iconutil', '-c', 'icns', '-o', res_dir + '/icon.icns', iconset])
		shutil.rmtree(iconset)

	#-----------------------------------------------------------
	#NOTE: write plist file
	#-----------------------------------------------------------
	version = args.version
	bundle_sig = "????"
	icon_file = ''

	plist_contents = """
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
			<string>orca</string>
			<key>CFBundleIconFile</key>
			<string>icon.icns</string>
			<key>NSHighResolutionCapable</key>
			<string>True</string>
		</dict>
	</plist>
	""".format(app_name=app_name, version=version, bundle_sig=bundle_sig, icon_file=icon_file)

	plist_file = open(contents_dir + '/Info.plist', 'w')
	print(plist_contents, file=plist_file)

def windows_make_app(args):
	#-----------------------------------------------------------
	#NOTE: make bundle directory structure
	#-----------------------------------------------------------
	app_name = args.name
	bundle_name = app_name
	bundle_dir = args.out_dir + '/' + bundle_name
	exe_dir = bundle_dir + '/bin'
	res_dir = bundle_dir + '/resources'
	guest_dir = bundle_dir + '/app'
	wasm_dir = guest_dir + '/wasm'
	data_dir = guest_dir + '/data'

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
	orca_exe = args.orca_dir + '/bin/orca.exe'
	milepost_lib = args.orca_dir + '/bin/milepost.dll'
	gles_lib = args.orca_dir + '/milepost/bin/libGLESv2.dll'
	egl_lib = args.orca_dir + '/milepost/bin/libEGL.dll'

	shutil.copy(orca_exe, exe_dir)
	shutil.copy(milepost_lib, exe_dir)
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
	shutil.copy(args.orca_dir + '/resources/OpenSansLatinSubset.ttf', res_dir)
	shutil.copy(args.orca_dir + '/resources/Menlo.ttf', res_dir)
	shutil.copy(args.orca_dir + '/resources/Menlo Bold.ttf', res_dir)
	shutil.copy(args.orca_dir + '/resources/Menlo Italics.ttf', res_dir)

	#-----------------------------------------------------------
	#NOTE make icon
	#-----------------------------------------------------------
	#TODO

#---------------------------------------------------------------------------------------------
# NOTE: get args
#
#	mkapp.py [options] module
#
#	-n, --name		the name of the app
#	-r, --res-file	copies a file to the app bundle's resource directory
#	-R, --res-dir	copies the contents of a directory to the bundle's resource directory
#	-i, --icon		icon file
#	-D, --out-dir	output directory
#----------------------------------------------------------------------------------------------

parser = ArgumentParser(prog='mkapp')
parser.add_argument("-d", "--resource", action='append', dest='resource_files')
parser.add_argument("-D", "--resource-dir", action='append', dest='resource_dirs')
parser.add_argument("-i", "--icon")
parser.add_argument("-C", "--out-dir", default=os.getcwd())
parser.add_argument("-n", "--name", default='out')
parser.add_argument("-O", "--orca-dir", default='.')
parser.add_argument("--version", default='0.0.0')
parser.add_argument("module")

args = parser.parse_args()

#----------------------------------------------------------------------------------------------
# Dispatch to platform-specific function
#----------------------------------------------------------------------------------------------
platformName = platform.system()
if platformName == 'Darwin':
	macos_make_app(args)
elif platformName == 'Windows':
	windows_make_app(args)
else:
	print("Platform '" +  platformName + "' is not supported for now...")
