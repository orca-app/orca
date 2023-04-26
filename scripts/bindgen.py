#!/usr/bin/env python3
import sys

if len(sys.argv) < 2:
	print("bindgen require an api name\n")
	exit(-1);

apiName = sys.argv[1]
cdir = ''

if len(sys.argv) > 2:
	cdir = sys.argv[2]

inPath = cdir + '/bindgen_' + apiName + '_api.txt'
outPath = cdir + '/bindgen_' + apiName + '_api.c'

inFile = open(inPath, 'r')
outFile = open(outPath, 'w')

stubs = []
links = []

def gen_stub(name, sig, native_name):
	if native_name == None:
		native_name = name

	src = 'const void* ' + name + '_stub(IM3Runtime runtime, IM3ImportContext _ctx, uint64_t * _sp, void * _mem)\n'
	src += '{\n'
	spIndex = 0

	parsingRet = True
	retCount = 0
	argCount = 0
	retString = ''
	argString = ''

	for index, c in enumerate(sig):
		if parsingRet:
			if retCount > 1:
				print('unsupported multiple return types\n')
				break
			if c == '(':
				parsingRet = False
				continue
			elif c == ')':
				print('unexpected ) while parsing return type\n')
				break;
			elif c == 'v':
				continue
			elif c == 'i':
				retString = '*((i32*)&_sp[0]) = '
			elif c == 'I':
				retString = '*((i64*)&_sp[0]) = '
			elif c == 'f':
				retString = '*((f32*)&_sp[0]) = '
			elif c == 'F':
				retString = '*((f64*)&_sp[0]) = '
			elif c == 'p':
				print('returning pointers is not supported yet\n')
				break
			else:
				print('unrecognized type ' + c + ' in procedure return\n')
				break
			retCount += 1
		else:
			argIndex = argCount + retCount
			if c == ')':
				break
			elif c == 'v':
				break
			elif c == 'i':
				argString += '*(i32*)&_sp[' + str(argIndex) + ']'
			elif c == 'I':
				argString += '*(i64*)&_sp[' + str(argIndex) + ']'
			elif c == 'f':
				argString += '*(f32*)&_sp[' + str(argIndex) + ']'
			elif c == 'F':
				argString += '*(f64*)&_sp[' + str(argIndex) + ']'
			elif c == 'p':
				argString += '(void*)((char*)_mem + *(i32*)&_sp[' + str(argIndex) + '])'
			else:
				print('unrecognized type ' + c + ' in procedure signature\n')
				break

			if index+2 < len(sig):
				argString += ', '
			argCount += 1

	src += '\t' + retString + native_name + '(' + argString + ');\n'
	src += '\treturn(0);\n'
	src += '}\n'
	stubs.append(src)

def gen_link(name, sig):
	m3_Sig = ''
	for c in sig:
		if c == 'p':
			m3_Sig += 'i'
		else:
			m3_Sig += c

	src = '\tres = m3_LinkRawFunction(module, "*", "' + name + '", "' + m3_Sig + '", ' + name + '_stub);\n'
	src += '\tif(res != m3Err_none && res != m3Err_functionLookupFailed) { log_error("error: %s\\n", res); return(-1); }\n\n'
	links.append(src)

for line in inFile:
	if line.isspace():
		continue
	desc = line.split()

	gen_stub(desc[0], desc[1], desc[2] if len(desc) > 2 else None)
	gen_link(desc[0], desc[1])

linkProc = 'int bindgen_link_' + apiName + '_api(IM3Module module)\n'
linkProc += '{\n'
linkProc += '\tM3Result res;\n'

for link in links:
	linkProc += link

linkProc += '\treturn(0);\n'
linkProc += '}\n'

for stub in stubs:
	outFile.write(stub)

outFile.write('\n')
outFile.write(linkProc)

inFile.close()
outFile.close()
