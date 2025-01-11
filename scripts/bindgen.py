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

def tag_to_valtype(tag, binding_name):
    if tag == 'i':
        return 'WA_TYPE_I32'
    elif tag == 'I':
        return 'WA_TYPE_I64'
    elif tag == 'f':
        return 'WA_TYPE_F32'
    elif tag == 'F':
        return 'WA_TYPE_F64'
    printError('Unknown tag ' + tag + ' for binding ' + binding_name)
    return 'WA_TYPE_I32'

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
            elif len(decl['args']) == 0:
                s += 'void'

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

            if len(decl['args']) == 0:
                s += 'void'

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
        s = 'void ' + cname + '_stub(const i64* restrict _params, i64* restrict _returns, u8* _mem, oc_wasm* wasm)'

        gen_stub = decl.get('gen_stub', True)
        if gen_stub == False:
            s += ';\n\n'
        else:
            s += '\n{\n'


            # NOTE: check and cast arguments
            retTag = decl['ret']['tag']

            firstArgIndex = 0
            if retTag == 'S':
                firstArgIndex = 1
                retTypeName = decl['ret']['name']
                retTypeCName = decl['ret'].get('cname', retTypeName)
                s += '\t' + retTypeCName + '* __retPtr = (' + retTypeCName + '*)((char*)_mem + *(i32*)&_params[0]);\n'

                s += '\t{\n'
                s += '\t\tOC_ASSERT_DIALOG(((char*)__retPtr >= (char*)_mem) && (((char*)__retPtr - (char*)_mem) < oc_wasm_mem_size(wasm)), "return pointer is out of bounds");\n'
                s += '\t\tOC_ASSERT_DIALOG((char*)__retPtr + sizeof(' + retTypeCName + ') <= ((char*)_mem + oc_wasm_mem_size(wasm)), "return pointer is out of bounds");\n'
                s += '\t}\n'

            for argIndex, arg in enumerate(decl['args']):

                argName = arg['name']
                typeName = arg['type']['name']
                typeCName = arg['type'].get('cname', typeName)
                argTag = arg['type']['tag']

                s += '\t'

                if argTag == 'i':
                    s += typeCName + ' ' + argName + ' = ('+typeCName+')*(i32*)&_params[' + str(firstArgIndex + argIndex) + '];\n'
                elif argTag == 'I':
                    s += typeCName + ' ' + argName + ' = ('+typeCName+')*(i64*)&_params[' + str(firstArgIndex + argIndex) + '];\n'
                elif argTag == 'f':
                    s += typeCName + ' ' + argName + ' = ('+typeCName+')*(f32*)&_params[' + str(firstArgIndex + argIndex) + '];\n'
                elif argTag == 'F':
                    s += typeCName + ' ' + argName + ' = ('+typeCName+')*(f64*)&_params[' + str(firstArgIndex + argIndex) + '];\n'
                elif argTag == 'p':
                    s += typeCName + ' ' + argName + ' = ('+ typeCName +')((char*)_mem + *(u32*)&_params[' + str(firstArgIndex + argIndex) + ']);\n'
                elif argTag == 'S':
                    s += typeCName + ' ' + argName + ' = *('+ typeCName +'*)((char*)_mem + *(u32*)&_params[' + str(firstArgIndex + argIndex) + ']);\n'
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
                        s += '\t\tOC_ASSERT_DIALOG(((char*)'+ argName + ' >= (char*)_mem) && (((char*)'+ argName +' - (char*)_mem) < oc_wasm_mem_size(wasm)), "parameter \''+argName+'\' is out of bounds");\n'
                        s += '\t\tOC_ASSERT_DIALOG((char*)' + argName + ' + '

                        proc = argLen.get('proc')
                        if proc != None:
                            s += proc + '(wasm, '
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

                        s += ' <= ((char*)_mem + oc_wasm_mem_size(wasm)), "parameter \''+argName+'\' is out of bounds");\n'
                        s += '\t}\n'

            s += '\t'

            if retTag == 'i':
                s += '*((i32*)&_returns[0]) = (i32)'
            elif retTag == 'I':
                s += '*((i64*)&_returns[0]) = (i64)'
            elif retTag == 'f':
                s += '*((f32*)&_returns[0]) = (f32)'
            elif retTag == 'F':
                s += '*((f64*)&_returns[0]) = (f64)'
            elif retTag == 'S':
                s += '*__retPtr = '
            elif retTag == 'p':
                printError(name + ": pointer return type not supported yet")

            s += cname + '('

            for i, arg in enumerate(decl['args']):
                s += arg['name']

                if i+1 < len(decl['args']):
                    s += ', '

            s += ');\n\n}\n'

        print(s, file=host_bindings)

    # link function
    s = 'int bindgen_link_' + apiName + '_api(oc_wasm* wasm)\n{\n'
    s += '\twa_status status;\n'
    s += '\tint ret = 0;\n\n'

    for decl in data:
        name = decl['name']
        cname = decl.get('cname', name)

        if needs_arg_ptr_stub(decl):
            name = name + '_argptr_stub'

        num_args = len(decl['args'])
        num_returns = 1

        if decl['ret']['tag'] == 'S':
            num_args += 1
            num_returns = 0
        if decl['ret']['tag'] == 'v':
            num_returns = 0;
        # num_returns = 0 if decl['ret']['tag'] == 'S' or decl['ret']['tag'] == 'v' else 1

        param_types = ''
        if num_args == 0:
            param_types = '\t\twa_value_type paramTypes[1];\n'
        else:
            param_types = '\t\twa_value_type paramTypes[] = {'

            if decl['ret']['tag'] == 'S':
                param_types += 'WA_TYPE_I32, '
            for arg in decl['args']:
                tag = arg['type']['tag']
                if tag == 'p' or tag == 'S':
                    tag = 'i'
                param_types += tag_to_valtype(tag, name) + ', '

            param_types += '};\n'

        return_types = ''
        if num_returns == 0:
            return_types = '\t\twa_value_type returnTypes[1];\n\n'
        else:
            return_types = '\t\twa_value_type returnTypes[] = {'
            return_types += tag_to_valtype(decl['ret']['tag'], name)
            return_types += '};\n\n'

        s += '\t{\n'
        s += param_types
        s += return_types;
        s += '\t\toc_wasm_binding binding = ' + '{0' + '};\n' #need to split this up so python doesn't think it's a format specifier :/
        s += '\t\tbinding.importName = OC_STR8("' + name + '");\n';
        s += '\t\tbinding.proc = ' + cname + '_stub;\n';
        s += '\t\tbinding.countParams = ' + str(num_args) + ';\n';
        s += '\t\tbinding.countReturns = ' + str(num_returns) + ';\n';
        s += '\t\tbinding.params = paramTypes;\n'
        s += '\t\tbinding.returns = returnTypes;\n'
        s += '\t\tstatus = oc_wasm_add_binding(wasm, &binding);\n'
        s += '\t\tif(wa_status_is_fail(status))\n'
        s += '\t\t{\n'
        s += '\t\t\toc_log_error("Couldn\'t link function ' + name + ' (%s)\\n", wa_status_str8(status).ptr);\n'
        s += '\t\t\tret = -1;\n'
        s += '\t\t}\n'
        s += '\t}\n\n'


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
