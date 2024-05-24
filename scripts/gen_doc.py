import api_dump
import json
from argparse import ArgumentParser

###########################################################################
# Generate C snippets from json spec
###########################################################################

def gen_macro(spec):
    name = spec["name"]
    s = f"#define {name}("
    for i, param in enumerate(spec["params"]):
        s += param["name"]
        if i != len(spec["params"])-1:
            s += ", "
    if "isVariadic" in spec and spec["isVariadic"] == True:
        s += ", ..."
    s += ")"
    if isinstance(spec["text"], str):
        s += " " + spec["text"] + "\n"
    else:
        s += "\\\n"
        for i, line in enumerate(spec["text"]):
            s += f"{line}"
            if i != len(spec["text"])-1:
                s += " \\"
            s += "\n"
    return s

def gen_indent(indent):
    s = ""
    for i in range(indent):
        s += "    "
    return s

def get_first_non_array_type(t):
    if t["kind"] == "array":
        return get_first_non_array_type(t["type"])
    else:
        return t

def gen_array_suffixes(t):
    if t["kind"] != "array":
        return ""
    else:
        s = gen_array_suffixes(t["type"])
        s += "[" + str(t["count"]) + "]"
        return s


def gen_field(field, indent):
    s = gen_indent(indent)
    name = field["name"]
    typeSpec = field["type"]
    typeKind = typeSpec["kind"]

    ########################################
    # this is wrong
    if typeKind == "array":
        s += gen_type(get_first_non_array_type(typeSpec), None, 0)
        s += " "
        s += name
        s += gen_array_suffixes(typeSpec)
    else:
        s += gen_type(typeSpec, None, indent)
        # could be an anonymous struct/union
        if name != "":
            s += " "
            s += name

    return s

def gen_enum_constant(constant, indent):
    s = gen_indent(indent)
    s += constant["name"]
    if "value" in constant:
        s += " = "
        s += str(constant["value"])
    s += ","
    return s

def gen_type(typeSpec, typeName, indent):

    s = ""

    typeKind = typeSpec["kind"]

    if typeKind == "struct" or typeKind == "enum" or typeKind == "union":
        s += typeKind
        if typeName != None:
            s += f" {typeName}"
        s += "\n"
        s += gen_indent(indent)
        s += "{\n"

        if typeKind == "struct" or typeKind == "union":
            if "fields" in typeSpec:
                for field in typeSpec["fields"]:
                    s += gen_field(field, indent+1)
                    s += ";\n"
        else:
            for constant in typeSpec["constants"]:
                gen_enum_constant(constant, indent+1)
                s += ";\n"

        s += gen_indent(indent)
        s += "}"
    elif typeKind == "pointer":
        if "attr" in typeSpec:
            s += typeSpec["attr"] + " "
        s += gen_type(typeSpec["type"], None, 0)
        s += "*"
    elif typeKind == "array":
        s += gen_type(typeSpec["type"], None, 0)
        s += "[" + str(typeSpec["count"]) + "]"
    elif typeKind == "namedType":
        s += typeSpec["name"]
    else:
        s += typeKind

    return s

def gen_typename_forward(spec):
    keyword = spec["keyword"]
    name = spec["name"]
    s = f"typedef {keyword} {name} {name};\n"

    return s

def gen_typename(spec):
    typeName = spec["name"]
    typeSpec = spec["type"]

    s = f"typedef "
    s += gen_type(typeSpec, typeName, 0)

    s += f" {typeName};\n"

    return s

def gen_proc(spec):
    s = ""
    if "visibility" in spec:
        s += spec["visibility"]
        s += " "

    s += gen_type(spec["return"], None, 0)
    s += " "
    s += spec["name"]
    s += "("
    for i, param in enumerate(spec["params"]):
        s += gen_type(param["type"], None, 0)
        s += " " + param["name"]
        if i != len(spec["params"])-1:
            s += ", "
    if "isVariadic" in spec and spec["isVariadic"] == True:
        s += ", ..."
    s += ");\n"

    return s


###########################################################################
# Generate documentation from json spec
###########################################################################

def escape_underscores(name):
    return name.replace("_", "\\_")

def doc_fields(desc):

    s = ""
    if "fields" in desc:
        s += "**Fields**\n\n"
        for field in desc["fields"]:
            name = field["name"]
            s += f"- **{name}** "
            if "doc" in field:
                s += field["doc"]
            s += "\n"

        s += "\n"

    return s

def doc_enum_constants(desc):
    # s += "**Constants**\n\n"
    # for constant in desc["constants"]:
    #     name = field["name"]
    #     s += f"- **{name}** \n"

    # s += "\n"
    # return s
    return ""

def doc_type(desc):
    name = desc["name"]
    kind = desc["type"]["kind"]

    s = "```\n"
    s += "typedef "
    s += gen_type(desc["type"], name, 0)
    s += "\n```\n\n"

    if "doc" in desc:
        s += desc["doc"]
        s += "\n\n"

    if kind == "struct" or kind == "union":
        s += doc_fields(desc["type"])
    elif kind == "enum":
        s += doc_enum_constants(desc)
    else:
        s += doc_typedef(name, desc["type"])

    s += "\n---\n\n"
    return s

def doc_typedef(name, desc):
    return ""

def doc_macro(desc):
    name = desc["name"]

    s = "```\n"
    s += f"#define {name}("
    for i, param in enumerate(desc["params"]):
        s += param["name"]
        if i != len(desc["params"])-1:
            s += ", "
    if "isVariadic" in desc and desc["isVariadic"] == True:
        s += "..."
    s += ")"
    s += "\n```\n\n"

    if "doc" in desc:
        s += desc["doc"]
        s += "\n\n"

    s += "**Parameters**\n\n"
    for param in desc["params"]:
        paramName = param["name"]
        s += f"- **{paramName}** "
        if "doc" in param:
            s += param["doc"]
        s += "\n"

    # document variadic arg
    s += "\n---\n\n"
    return s

def doc_proc(desc):
    name = desc["name"]

    s = "```\n"
    s += gen_proc(desc)
    s += "\n```\n\n"

    if "doc" in desc:
        s += desc["doc"]
        s += "\n\n"

    s += "**Parameters**\n\n"
    for param in desc["params"]:
        paramName = param["name"]
        s += f"- **{paramName}** "
        if "doc" in param:
            s += param["doc"]
        s += "\n"

    # document variadic arg
    s += "\n---\n\n"
    return s



def find_entry_match(desc, dump):

    kind = desc["kind"]
    name = desc["name"]

    ast = api_dump.find_entry(dump, kind, name)
    if ast == None:
        print(f"error: couldn't find entry {name} of kind {kind}")
        return False

    res = check_entry_match(desc, ast)

    if res == False:
        print("note: mismatch when checking:")
        print(json.dumps(ast))
        print("against:")
        print(json.dumps(desc))

    return res

def check_error(s, ast, desc):
    print(s)
    print("note: expected:")
    print(json.dumps(ast))
    print("note: got:")
    print(json.dumps(desc))

def check_entry_match(desc, ast, allowNameMismatch=False):

    for key in ast.keys():
        if key not in desc:
            check_error(f"error: couldn't find key {key} in object.", ast, desc)
            print("note: expected:")
            print(json.dumps(ast))
            print("note: got:")
            print(json.dumps(desc))
            return False
        elif type(ast[key]) != type(desc[key]):
            check_error(f"error: type mismatch for key {key} in object:", ast, desc)
            return False
        elif isinstance(ast[key], str):
            if allowNameMismatch and key == "name":
                # we allow argument names to not match when we're parsing a parameter list,
                # because we can't get argument names from clang when parsing a typedefed function pointer...
                return True

            if ast[key] != desc[key]:
                check_error(f"error: value mismatch for key {key} in object:", ast, desc)
                return False

        elif isinstance(ast[key], list):
            if len(ast[key]) != len(desc[key]):
                check_error(f"error: length mismatch for key {key} in object:", ast, desc)
                return False

            for descIt, astIt in zip(desc[key], ast[key]):
                if not check_entry_match(descIt, astIt, allowNameMismatch = (key == "params")):
                    return False
        elif isinstance(ast[key], dict):
            if not check_entry_match(desc[key], ast[key], allowNameMismatch):
                return False

    return True

def doc_contents(contents, dump):
    types = [x for x in contents if x["kind"] == "typename"]
    macros = [x for x in contents if x["kind"] == "macro"]
    procs = [x for x in contents if x["kind"] == "proc"]

    s = ""

    if len(types):
        s += "### Types\n\n"
        for e in types:
            if not find_entry_match(e, dump):
                return ("", False)
            s += doc_type(e)

    if len(macros):
        s += "### Macros\n\n"
        for e in macros:
            #if not find_entry_match(e, dump):
            #    return ("", False)
            s += doc_macro(e)

    if len(procs):
        s += "### Functions\n\n"
        for e in procs:
            if not find_entry_match(e, dump):
                return ("", False)
            s += doc_proc(e)

    return (s, True)




parser = ArgumentParser(prog='gen_doc')
parser.add_argument("desc", help="a json description of the API")
parser.add_argument("outDir", help="the output directory for the documentation.")

args = parser.parse_args()

# get api dump from header files
dump = api_dump.get_api_entries()

# load manual API desc
with open(args.desc, "r") as f:
    desc = json.load(f)

# generate documentation
for module in desc:
    if module["kind"] == "module":
        moduleName = module["name"]

        print(f"process module {moduleName}")

        s, ok = doc_contents(module["contents"], dump)

        if ok == False:
            print("Couldn't complete doc generation")
            exit(-1)

        s = f"# {moduleName}\n\n" + s

        docName = args.outDir + "/" + moduleName + ".md"
        with open(docName, "w") as f:
            print(s, file=f, end='', flush=True)
