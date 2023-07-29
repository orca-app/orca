#!/usr/bin/env python3

from argparse import ArgumentParser
import json

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

host_bindings = open(wasm3_bindings_path, 'w')
guest_bindings = None

specFile = open(spec, 'r')
data = json.load(specFile)

def needs_arg_ptr_stub(decl):
	res = (decl['ret']['tag'] == 'S')
	for arg in decl['args']:
		if arg['type']['tag'] == 'S':
			res = True
	return(res)

for decl in data:
	if needs_arg_ptr_stub(decl):
		guest_bindings = open(guest_stubs_path, 'w')
		if args.guest_include != None:
			s = '#include"' + args.guest_include + '"\n\n'
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
		s += '\n{\n\t'
		retTag = decl['ret']['tag']

		if retTag == 'i':
			s += '*((i32*)&_sp[0]) = '
		elif retTag == 'I':
			s += '*((i64*)&_sp[0]) = '
		elif retTag == 'f':
			s += '*((f32*)&_sp[0]) = '
		elif retTag == 'F':
			s += '*((f64*)&_sp[0]) = '
		elif retTag == 'S':
			retTypeName = decl['ret']['name']
			retTypeCName = decl['ret'].get('cname', retTypeName)
			s += '*(' + retTypeCName + '*)((char*)_mem + *(i32*)&_sp[0]) = '

		s += cname + '('

		firstArgIndex = 0
		if retTag != 'v':
			firstArgIndex = 1

		for i, arg in enumerate(decl['args']):
			typeName = arg['type']['name']
			typeCName = arg['type'].get('cname', typeName)
			argTag = arg['type']['tag']
			if argTag == 'i':
				s += '*(i32*)&_sp[' + str(firstArgIndex + i) + ']'
			elif argTag == 'I':
				s += '*(i64*)&_sp[' + str(firstArgIndex + i) + ']'
			elif argTag == 'f':
				s += '*(f32*)&_sp[' + str(firstArgIndex + i) + ']'
			elif argTag == 'F':
				s += '*(f64*)&_sp[' + str(firstArgIndex + i) + ']'
			elif argTag == 'p':
				s += '(void*)((char*)_mem + *(i32*)&_sp[' + str(firstArgIndex + i) + '])'
			elif argTag == 'S':
				s += '*(' + typeCName + '*)((char*)_mem + *(i32*)&_sp[' + str(firstArgIndex + i) + '])'
			else:
				print('unrecognized type ' + c + ' in procedure signature\n')
				break

			if i+1 < len(decl['args']):
				s += ', '

		s += ');\n\treturn(0);\n}\n\n'

	print(s, file=host_bindings)

# link function
s = 'int bindgen_link_' + apiName + '_api(IM3Module module)\n{\n\t'
s += 'M3Result res;\n'

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


	s += '\tres = m3_LinkRawFunction(module, "*", "' + name + '", "' + m3Sig + '", ' + cname + '_stub);\n'
	s += '\tif(res != m3Err_none && res != m3Err_functionLookupFailed) { log_error("error: %s\\n", res); return(-1); }\n\n'


s += '\treturn(0);\n}\n'

print(s, file=host_bindings)
