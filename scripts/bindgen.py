#!/usr/bin/env python3

from argparse import ArgumentParser
import json


def needs_arg_ptr_stub(decl):
	res = (decl['ret']['tag'] == 'S')
	for arg in decl['args']:
		if arg['type']['tag'] == 'S':
			res = True
	return(res)

def printError(str):
	# This prints a string with a red foreground color.
	# See this link for an explanation of console escape codes: https://stackabuse.com/how-to-print-colored-text-in-python/
	print("\x1b[38;5;196m" + "error: " + str + "\033[0;0m")


def bindgen(apiName, spec, **kwargs):
	guest_stubs_path = kwargs.get("guest_stubs")
	guest_include = kwargs.get("guest_include")
	wasm3_bindings_path = kwargs.get("wasm3_bindings")

	if guest_stubs_path == None:
		guest_stubs_path = 'bindgen_' + apiName + '_guest_stubs.c'
	if wasm3_bindings_path == None:
		wasm3_bindings_path = 'bindgen_' + apiName + '_wasm3_bindings.c'

	host_bindings = open(wasm3_bindings_path, 'w')
	guest_bindings = None

	specFile = open(spec, 'r')
	data = json.load(specFile)

	for decl in data:
		if needs_arg_ptr_stub(decl):
			guest_bindings = open(guest_stubs_path, 'w')
			if guest_include != None:
				s = '#include"' + guest_include + '"\n\n'
				print(s, file=guest_bindings)
			break

	for decl in data:

		name = decl['name']
		cname = decl.get('cname', name)

		if needs_arg_ptr_stub(decl):
			argPtrStubName = name + '_argptr_stub'
			# pointer arg stub declaration
			s = ''
			if decl['ret']['tag'] == 'S':
				s += 'void'
			else:
				s += decl['ret']['name']

			s += ' ORCA_IMPORT(' + argPtrStubName + ') ('

			if decl['ret']['tag'] == 'S':
				s += decl['ret']['name'] + '* __retArg'
				if len(decl['args']) > 0:
					s += ', '

			for i, arg in enumerate(decl['args']):
				s += arg['type']['name']
				if arg['type']['tag'] == 'S':
					s += '*'
				s += ' ' + arg['name']
				if i+1 < len(decl['args']):
					s += ', '
			s += ');\n\n'

			# forward function to pointer arg stub declaration
			s += decl['ret']['name'] + ' ' + name + '('
			for i, arg in enumerate(decl['args']):
				s += arg['type']['name'] + ' ' + arg['name']
				if i+1 < len(decl['args']):
					s += ', '
			s += ')\n'
			s += '{\n'
			s += '\t'
			if decl['ret']['tag'] == 'S':
				s += decl['ret']['name'] + ' __ret;\n\t'
			elif decl['ret']['tag'] != 'v':
				s += decl['ret']['name']
				s += ' __ret = '
			s += argPtrStubName + '('

			if decl['ret']['tag'] == 'S':
				s += '&__ret'
				if len(decl['args']) > 0:
					s += ', '

			for i, arg in enumerate(decl['args']):
				if arg['type']['tag'] == 'S':
					s += '&'

				s += arg['name']
				if i+1 < len(decl['args']):
					s += ', '
			s += ');\n'
			if decl['ret']['tag'] != 'v':
				s += '\treturn(__ret);\n'
			s += '}\n\n'

			print(s, file=guest_bindings)

		# host-side stub
		s = 'const void* ' + cname + '_stub(IM3Runtime runtime, IM3ImportContext _ctx, uint64_t* _sp, void* _mem)'

		gen_stub = decl.get('gen_stub', True)
		if gen_stub == False:
			s += ';\n\n'
		else:
			s += '\n{\n'


			# NOTE: check and cast arguments
			retTag = decl['ret']['tag']

			firstArgIndex = 0
			if retTag != 'v':
				firstArgIndex = 1

				if retTag == 'S':
					retTypeName = decl['ret']['name']
					retTypeCName = decl['ret'].get('cname', retTypeName)
					s += retTypeCName + '* __retPtr = (' + retTypeCName + '*)((char*)_mem + *(i32*)&_sp[0]);\n'

					s += '\t{\n'
					s += '\t\tOC_ASSERT(((char*)__retPtr >= (char*)_mem) && (((char*)__retPtr - (char*)_mem) < m3_GetMemorySize(runtime)), "return pointer is out of bounds");\n'
					s += '\t\tOC_ASSERT((char*)__retPtr + sizeof(' + retTypeCName + ') <= ((char*)_mem + m3_GetMemorySize(runtime)), "return pointer is out of bounds");\n'
					s += '\t}\n'

			for argIndex, arg in enumerate(decl['args']):

				argName = arg['name']
				typeName = arg['type']['name']
				typeCName = arg['type'].get('cname', typeName)
				argTag = arg['type']['tag']

				s += '\t'

				if argTag == 'i':
					s += typeCName + ' ' + argName + ' = ('+typeCName+')*(i32*)&_sp[' + str(firstArgIndex + argIndex) + '];\n'
				elif argTag == 'I':
					s += typeCName + ' ' + argName + ' = ('+typeCName+')*(i64*)&_sp[' + str(firstArgIndex + argIndex) + '];\n'
				elif argTag == 'f':
					s += typeCName + ' ' + argName + ' = ('+typeCName+')*(f32*)&_sp[' + str(firstArgIndex + argIndex) + '];\n'
				elif argTag == 'F':
					s += typeCName + ' ' + argName + ' = ('+typeCName+')*(f64*)&_sp[' + str(firstArgIndex + argIndex) + '];\n'
				elif argTag == 'p':
					s += typeCName + ' ' + argName + ' = ('+ typeCName +')((char*)_mem + *(u32*)&_sp[' + str(firstArgIndex + argIndex) + ']);\n'
				elif argTag == 'S':
					s += typeCName + ' ' + argName + ' = *('+ typeCName +'*)((char*)_mem + *(u32*)&_sp[' + str(firstArgIndex + argIndex) + ']);\n'
				else:
					print('unrecognized type ' + c + ' in procedure signature\n')
					break

			# check pointer arg length
			for arg in decl['args']:

				argName = arg['name']
				typeName = arg['type']['name']
				typeCName = arg['type'].get('cname', typeName)
				argTag = arg['type']['tag']
				argLen = arg.get('len')

				if argTag == 'p':
					if argLen == None:
						printError("binding '" + name + "' missing pointer length decoration for param '" + argName + "'")
					else:
						s += '\t{\n'
						s += '\t\tOC_ASSERT(((char*)'+ argName + ' >= (char*)_mem) && (((char*)'+ argName +' - (char*)_mem) < m3_GetMemorySize(runtime)), "parameter \''+argName+'\' is out of bounds");\n'
						s += '\t\tOC_ASSERT((char*)' + argName + ' + '

						proc = argLen.get('proc')
						if proc != None:
							s += proc + '(runtime, '
							lenProcArgs = argLen['args']
							for i, lenProcArg in enumerate(lenProcArgs):
								s += lenProcArg
								if i < len(lenProcArgs)-1:
									s += ', '
							s += ')'
						else:
							components =  argLen.get('components')
							countArg = argLen.get('count')

							if components != None:
								s += str(components)
								if countArg != None:
									s += '*'
							if countArg != None:
								s += countArg

						if typeCName.endswith('**') or (typeCName.startswith('void') == False and typeCName.startswith('const void') == False):
							s += '*sizeof('+typeCName[:-1]+')'

						s += ' <= ((char*)_mem + m3_GetMemorySize(runtime)), "parameter \''+argName+'\' is out of bounds");\n'
						s += '\t}\n'

			s += '\t'

			if retTag == 'i':
				s += '*((i32*)&_sp[0]) = (i32)'
			elif retTag == 'I':
				s += '*((i64*)&_sp[0]) = (i64)'
			elif retTag == 'f':
				s += '*((f32*)&_sp[0]) = (f32)'
			elif retTag == 'F':
				s += '*((f64*)&_sp[0]) = (f64)'
			elif retTag == 'S':
				s += '*__retPtr = '
			elif retTag == 'p':
				print("Warning: " + name + ": pointer return type not supported yet")

			s += cname + '('

			for i, arg in enumerate(decl['args']):
				s += arg['name']

				if i+1 < len(decl['args']):
					s += ', '

			s += ');\n\treturn(0);\n}\n\n'

		print(s, file=host_bindings)

	# link function
	s = 'int bindgen_link_' + apiName + '_api(IM3Module module)\n{\n'
	s += '	M3Result res;\n'
	s += '	int ret = 0;\n'

	for decl in data:
		name = decl['name']
		cname = decl.get('cname', name)

		if needs_arg_ptr_stub(decl):
			name = name + '_argptr_stub'

		m3Sig = ''
		if decl['ret']['tag'] == 'S':
			m3Sig += 'v'
		else:
			m3Sig += decl['ret']['tag']

		m3Sig += '('
		if decl['ret']['tag'] == 'S':
			m3Sig += 'i'
		for arg in decl['args']:
			tag = arg['type']['tag']
			if tag == 'p' or tag == 'S':
				tag = 'i'
			m3Sig += tag
		m3Sig += ')'


		s += '	res = m3_LinkRawFunction(module, "*", "' + name + '", "' + m3Sig + '", ' + cname + '_stub);\n'
		s += '	if(res != m3Err_none && res != m3Err_functionLookupFailed)\n'
		s += '	{\n'
		s += '		oc_log_error("Couldn\'t link function ' + name + ' (%s)\\n", res);\n'
		s += '		ret = -1;\n'
		s += '	}\n\n'

	s += '\treturn(ret);\n}\n'

	print(s, file=host_bindings)


if __name__ == "__main__":
	parser = ArgumentParser(prog='bindgen.py')
	parser.add_argument('api')
	parser.add_argument('spec')
	parser.add_argument('-g', '--guest-stubs')
	parser.add_argument('--guest-include')
	parser.add_argument('--wasm3-bindings')

	args = parser.parse_args()

	apiName = args.api
	spec = args.spec
	guest_stubs_path = args.guest_stubs
	if guest_stubs_path == None:
		guest_stubs_path = 'bindgen_' + apiName + '_guest_stubs.c'

	wasm3_bindings_path = args.wasm3_bindings
	if wasm3_bindings_path == None:
		wasm3_bindings_path = 'bindgen_' + apiName + '_wasm3_bindings.c'

	bindgen(apiName, spec,
		guest_stubs=guest_stubs_path,
		guest_include=args.guest_include,
		wasm3_bindings=wasm3_bindings_path,
	)
