# Typical usage, run from orca root:
#   python scripts/gles_gen.py --spec src/ext/gl.xml --header=src/graphics/orca_gl31.h --json=src/wasmbind/gles_api.json --log=zig-out/log/gles_gen.log

import xml.etree.ElementTree as et
from argparse import ArgumentParser

if __name__ == "__main__":
	from reg_modified import *
else:
	from .reg_modified import *

# remove APIs that can't be sandboxed
removeProc = [
	"glMapBuffer",
	"glMapBufferRange",
	"glUnmapBuffer",
	"glFlushMappedBufferRange",
	"glGetBufferPointerv"
]

def gen_gles_header(spec, filename, log_file):
	# Generates the GLES header, wrapping gl functions
	# prototypes in ORCA_IMPORT() macro

	gles2through31Pat = '2\.[0-9]|3\.[01]'
	allVersions = '.*'

	genOpts = CGeneratorOptions(
		filename=filename,
		apiname='gles2',
		profile='common',
		versions=gles2through31Pat,
		emitversions=allVersions,
		protectProto=False,
		procMacro='ORCA_IMPORT',
		removeProc = removeProc)

	reg = Registry()
	tree = et.parse(spec)
	reg.loadElementTree(tree)

	logFile = open(log_file, 'w')
	gen = COutputGenerator(diagFile=logFile)
	reg.setGenerator(gen)
	reg.apiGen(genOpts)

	logFile.close()


def get_bindgen_tag_for_type(typeName):
	typeToTags = {
		"void": "v",

		"GLenum": "i",
		"GLbitfield": "i",

		"GLboolean": "i",
		"GLbyte": "i",
		"GLubyte": "i",
		"GLchar": "i",

		"GLshort": "i",
		"GLushort": "i",
		"GLhalf": "i",
		"GLhalfARB": "i",

		"GLuint": "i",
		"GLint": "i",
		"GLclampx": "i",
		"GLsizei": "i",
		"GLfixed": "i",

		"GLintptr": "i",
		"GLsizeiptr": "i",

		"GLuint64": "I",
		"GLint64": "I",

		"GLfloat": "f",
		"GLclampf": "f",

		"GLdouble": "F",
		"GLclampd": "F",

		#NOTE: we treat sync objects as opaque 64bit values
		#TODO we should _also_ make sure that Wasm code treat them as 64bit values
		"GLsync": "I"
	}

	if typeName[len(typeName)-1] == '*':
		return "p"
	else:
		tag = typeToTags.get(typeName)
		return tag


def gen_compsize_len_entry(name, argName, compsizeArgs):

	entry = '\t\t\t"len": {\n'
	entry += '\t\t\t\t"proc": "orca_'+ name +'_'+argName+'_length",\n'
	entry += '\t\t\t\t"args": ['

	for i, compsizeArg in enumerate(compsizeArgs):
		entry += '"' + compsizeArg + '"'
		if i < len(compsizeArgs)-1:
			entry += ', '
	entry += ']\n'
	entry += '\t\t\t}'
	return entry

def gen_argcount_len_entry(name, argName, tokens):

	entry = '\t\t\t"len": {'
	if len(tokens) == 2:
		if tokens[1].isnumeric() == False:
			print("Warning: function " + name + ": couldn't parse parameter '" + argName + "' lenght attribute")
		entry += '"count": "' + tokens[0] + '", "components": '+ tokens[1]
	elif len(tokens) == 1:
		if tokens[0].isnumeric():
			entry += '"components":'+ tokens[0]
		else:
			entry += '"count": "'+ tokens[0] + '"'
	else:
		print("Warning: function " + name + ": couldn't parse parameter '" + argName + "' lenght attribute")

	entry += '}'
	return entry

def gen_gles_bindgen_json(spec, filename):

	# Gather gles 3.1 required functions
	tree = et.parse(spec)
	api = []

	for	feature in tree.iterfind('feature[@api="gles2"]'):
		if float(feature.get('number')) > 3.1:
			break

		for require in feature.iter('require'):
			if require.get('profile') == 'compatibility':
				continue
			for command in require.iter('command'):
				if command.get('name') not in removeProc:
					api.append(command.get('name'))

		for remove in feature.iter('remove'):
			for command in remove.iter('command'):
				api.remove(command.get('name'))

	# put all GL commands in a dict
	commands = dict()
	commandsSpec = tree.find('./commands')
	for command in commandsSpec.iter('command'):
		name = command.find('proto/name')
		commands[name.text] = command

	# TODO: Generate json descriptions for commands in api

	manualBind = [
		"glShaderSource",
		"glGetVertexAttribPointerv",
		"glVertexAttribPointer",
		"glVertexAttribIPointer",
		"glGetString",
		"glGetStringi",
		"glGetUniformIndices"
	]

	json = '[\n'
	for name in api:
		if name in manualBind:
			continue

		command = commands.get(name)
		if command == None:
			print("Couldn't find definition for required command '" + name + "'")
			exit(-1)

		proto = command.find("proto")
		ptype = proto.find("ptype")

		retType = ''
		if proto.text != None:
			retType += proto.text

		if ptype != None:
			if ptype.text != None:
				retType += ptype.text
			if ptype.tail != None:
				retType += ptype.tail

		retType = retType.strip()

		retTag = get_bindgen_tag_for_type(retType)
		if retTag == None:
			print("Couldn't find tag for GL type '" + retType + "'")
			exit(-1)

		entry = '{\n\t"name": "' + name + '",\n'
		entry += '\t"cname": "' + name +  '",\n'
		entry += '\t"ret": { "name": "' + retType + '", "tag": "' + retTag + '"},\n'

		entry += '\t"args": [ '

		# iterate through params
		for param in command.iter('param'):

			argNode = param.find('name')
			argName = argNode.text

			typeNode = param.find('ptype')

			typeName = ''

			if param.text != None:
				typeName += param.text

			if typeNode != None:
				if typeNode.text != None:
					typeName += typeNode.text
				if typeNode.tail != None:
					typeName += typeNode.tail

			typeName = typeName.strip()

			if typeName.endswith('**'):
				print("Warning: function " + name + ": parameter " + argName + " has 2 (or more) levels of indirection")

			typeTag = get_bindgen_tag_for_type(typeName)

			if typeTag == None:
				print("Couldn't find tag for GL type '" + typeName + "' in function '"+ name +"'")
				exit(-1)

			entry += '\n'
			entry += '\t\t{\n\t\t\t"name": "'+ argName +'",\n'
			entry += '\t\t\t"type": {"name": "'+ typeName +'", "tag": "'+ typeTag +'"}'

			lenString = param.get('len')

			nullStringProcWithNoLen = [
				"glBindAttribLocation",
				"glGetAttribLocation",
				"glGetUniformLocation"
			]

			drawIndirectProc = [
				"glDrawArraysIndirect",
				"glDrawElementsIndirect"
			]

			if typeTag == "p":
				if lenString == None:
					if name in drawIndirectProc:
						entry += ',\n'
						entry += gen_compsize_len_entry(name, argName, ['indirect'])
					elif name in nullStringProcWithNoLen:
						entry += ',\n'
						entry += gen_compsize_len_entry(name, argName, ['name'])
					else:
						print("Warning: function " + name + ": parameter " + argName + " has no len attribute")

				elif lenString != None:
					entry += ',\n'

					tokens = lenString.split('*')

					if lenString.startswith("COMPSIZE"):
						tmp = lenString
						if tmp.startswith("COMPSIZE("):
							tmp = tmp[len("COMPSIZE("):]
						if tmp.endswith(")"):
							tmp = tmp[:-1]

						compsizeArgs = list(filter(None, tmp.split(",")))

						if len(compsizeArgs) == 0:
							# special case glGetUniformBlockIndex which isn't specified correctly in gl.xml
							if name == 'glGetUniformBlockIndex':
								compsizeArgs = ['uniformBlockName']

						entry += gen_compsize_len_entry(name, argName, compsizeArgs)

					else:
						entry += gen_argcount_len_entry(name, argName, tokens)

			entry += '\n\t\t},'

		entry = entry[:-1]
		entry += '\n\t]\n}'

		json += entry
		json += ',\n'

	json = json[:-2]
	json += '\n]'

	# write json to jsonFile
	f = open(filename, 'w')
	f.write(json)
	f.close()

def gles_gen(spec, json, header, log_file):
	gen_gles_header(spec, header, log_file)
	gen_gles_bindgen_json(spec, json)

#----------------------------------------
# driver
#----------------------------------------

if __name__ == "__main__":
	parser = ArgumentParser()
	parser.add_argument("-s", "--spec")
	parser.add_argument("--header")
	parser.add_argument("-j", "--json")
	parser.add_argument("-l", "--log")

	args = parser.parse_args()

	glesHeader = args.header
	jsonFile = args.json

	gles_gen(args.spec, jsonFile, glesHeader, args.log or './build/gles_gen.log')
