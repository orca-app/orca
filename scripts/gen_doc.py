import os
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

def gen_indent(indent, width=4):
    s = ""
    for i in range(indent):
        for j in range(width):
            s += " "
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
    return s

def gen_type(typeSpec, typeName, indent):

    s = ""

    typeKind = typeSpec["kind"]

    if typeKind == "variadic-param":
        pass
    elif typeKind == "struct" or typeKind == "enum" or typeKind == "union":
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
            for i, constant in enumerate(typeSpec["constants"]):
                s += gen_enum_constant(constant, indent+1)
                if i != len(typeSpec["constants"])-1:
                    s += ","
                s += "\n"

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
    elif typeKind == "proc":
        s += gen_type(typeSpec["return"], None, 0)
        if typeName != None:
            s += f" (*{typeName})"
        s += "("
        s += gen_param_list(typeSpec)
        s += ")"
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

    if typeSpec["kind"] != "proc":
        s += f" {typeName};"
    s += "\n"

    return s

def gen_param_list(spec):
    s = "("
    for i, param in enumerate(spec["params"]):
        s += gen_type(param["type"], None, 0)
        s += " " + param["name"]
        if i != len(spec["params"])-1:
            s += ", "
    s += ")"
    return s

def gen_proc(spec):
    s = ""
    if "visibility" in spec:
        s += spec["visibility"]
        s += " "

    s += gen_type(spec["return"], None, 0)
    s += " "
    s += spec["name"]
    s += gen_param_list(spec)
    s += ";\n"

    return s


###########################################################################
# Generate documentation from json spec
###########################################################################

def escape_underscores(name):
    return name.replace("_", "\\_")

def make_dir_name(name):
    return name.replace("/", "_")

def doc_fields(desc, indent=0):

    s = ""
    for field in desc["fields"]:
        name = field["name"]
        typeKind = field["type"]["kind"]

        s += gen_indent(indent)
        if name == "" and (typeKind == "struct" or typeKind == "union"):
            s += f"- Anonymous <code>{typeKind}</code>"
        else:
            s += f"- <code>{name}</code> "

        if "doc" in field:
            s += doc_text(field["doc"])
        s += "\n"

        if typeKind == "struct" or typeKind == "union":
            s += doc_fields(field["type"], indent+1)

    s += "\n"

    return s

def doc_enum_constants(desc):
    s = "**Enum Constants**\n\n"
    for constant in desc["constants"]:
        name = constant["name"]
        s += f"- <code>{name}</code> "
        if "doc" in constant:
            s += doc_text(constant["doc"])
        s += "\n"

    s += "\n"
    return s

def doc_type(desc):
    name = desc["name"]
    kind = desc["type"]["kind"]

    if name != "":
        s = f"#### <pre>{name}</pre>\n\n"
    else:
        s = f"#### Anonymous <code>{kind}</code>\n\n"

    s += "```\n"
    s += "typedef "
    s += gen_type(desc["type"], name, 0)
    if name != "" and desc["type"]["kind"] != "proc":
        s += f" {name};"
    else:
        s += ";"
    s += "\n```\n\n"

    if "doc" in desc:
        s += doc_text(desc["doc"])
        s += "\n\n"

    if kind == "struct" or kind == "union":
        if "fields" in desc["type"]:
            s += "**Fields**\n\n"
            s += doc_fields(desc["type"])
    elif kind == "enum":
        s += doc_enum_constants(desc["type"])
    else:
        s += doc_typedef(name, desc["type"])

    if "remarks" in desc:
        s += "\n**Remarks**\n\n"
        s += doc_text(desc["remarks"])
        s += "\n"

    if "note" in desc:
        s += "\n**Note**\n\n"
        s += doc_text(desc["note"])
        s += "\n"

    s += "\n---\n\n"
    return s

def doc_typedef(name, desc):
    return ""

def doc_macro(desc):
    name = desc["name"]

    s = f"#### <pre>{name}</pre>\n\n"

    s += "```\n"
    s += f"#define {name}("
    for i, param in enumerate(desc["params"]):
        s += param["name"]
        if i != len(desc["params"])-1:
            s += ", "
    s += ")"
    s += "\n```\n\n"

    if "doc" in desc:
        s += doc_text(desc["doc"])
        s += "\n\n"

    s += "**Parameters**\n\n"
    for param in desc["params"]:
        paramName = param["name"]
        s += f"- <code>{paramName}</code> "
        if "doc" in param:
            s += doc_text(param["doc"])
        s += "\n"

    if "return" in desc and "doc" in desc["return"]:
        s += "\n**Return**\n\n"
        s += doc_text(desc["return"]["doc"])
        s += "\n"

    if "remarks" in desc:
        s += "\n**Remarks**\n\n"
        s += doc_text(desc["remarks"])
        s += "\n"

    if "note" in desc:
        s += "\n**Note**\n\n"
        s += doc_text(desc["note"])
        s += "\n"

    s += "\n---\n\n"
    return s

def doc_proc(desc):
    name = desc["name"]

    s = f"#### <pre>{name}</pre>\n\n"

    s += "```\n"
    s += gen_proc(desc)
    s += "```\n\n"

    if "doc" in desc:
        s += doc_text(desc["doc"])
        s += "\n\n"

    if len(desc["params"]):
        s += "**Parameters**\n\n"
        for param in desc["params"]:
            paramName = param["name"]
            s += f"- <code>{paramName}</code> "
            if "doc" in param:
                s += doc_text(param["doc"])
            s += "\n"

    if "doc" in desc["return"]:
        s += "\n**Return**\n\n"
        s += doc_text(desc["return"]["doc"])
        s += "\n"

    if "remarks" in desc:
        s += "\n**Remarks**\n\n"
        s += doc_text(desc["remarks"])
        s += "\n"

    if "note" in desc:
        s += "\n**Note**\n\n"
        s += doc_text(desc["note"])
        s += "\n"

    s += "\n---\n\n"
    return s



def find_entry_match(desc, dump):

    kind = desc["kind"]
    name = desc["name"]

    if kind == "typename" and name == "":
        # this can be an anonymous enum, so we'll just check the constants independently
        if desc["type"]["kind"] == "enum":
            for constant in desc["type"]["constants"]:
                if not find_entry_match(constant, dump):
                    return False
            return True
        else:
            print(f"error: unexpected anonymous type of kind {kind}")
            return False

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

def doc_text(doc):
    if isinstance(doc, str):
        s = doc
    elif isinstance(doc, list):
        s = ""
        for line in doc:
            s += line + "\n"

    return s



def doc_module(outDir, module, dump):
    moduleName = module["name"]

    print(f"process module {moduleName}")

    contents = module["contents"]
    modules = [x for x in contents if x["kind"] == "module"]
    types = [x for x in contents if x["kind"] == "typename"]
    macros = [x for x in contents if x["kind"] == "macro"]
    procs = [x for x in contents if x["kind"] == "proc"]

    s = f"# {moduleName}\n\n"

    if "brief" in module:
        s += doc_text(module["brief"])
        s += "\n\n"

    if "doc" in module:
        s += doc_text(module["doc"])
        s += "\n\n"

    ok = True
    if ok and len(modules):
        s += "## Modules\n\n"
        subDir = os.path.join(outDir, make_dir_name(moduleName))

        for subModule in modules:
            subModuleName = subModule["name"]
            subModulePath = os.path.join(make_dir_name(moduleName), make_dir_name(subModuleName) + ".md")
            s += f"- [{subModuleName}]({subModulePath}) "
            if "brief" in subModule:
                s += doc_text(subModule["brief"])
            s += "\n"

            doc_module(subDir, subModule, dump)

        s += "\n---\n\n"

    if ok and len(types):
        s += "## Types\n\n"
        for e in types:
            if not find_entry_match(e, dump):
                ok = False
                break
            s += doc_type(e)

    if ok and len(macros):
        s += "## Macros\n\n"
        for e in macros:
            #if not find_entry_match(e, dump):
            #    return ("", False)
            s += doc_macro(e)

    if ok and len(procs):
        s += "## Functions\n\n"
        for e in procs:
            if not find_entry_match(e, dump):
                ok = False
                break
            s += doc_proc(e)

    if ok == False:
        print("Couldn't complete doc generation")
        exit(-1)


    if not os.path.exists(outDir):
        os.makedirs(outDir)

    docName = outDir + "/" + make_dir_name(moduleName) + ".md"
    with open(docName, "w") as f:
        print(s, file=f, end='', flush=True)





parser = ArgumentParser(prog='gen_doc')
parser.add_argument("desc", help="a json description of the API")
parser.add_argument("outDir", help="the output directory for the documentation.")

args = parser.parse_args()

# get api dump from header files
dump = api_dump.get_api_entries()

# load manual API desc
with open(args.desc, "r") as f:
    desc = json.load(f)

# generate documentation files
s = "# API reference\n\n"
s += "### Modules\n\n"

docDir = os.path.join(args.outDir, "docs/api")

for module in desc:
    if module["kind"] == "module":
        moduleName = module["name"]
        modulePath = moduleName + ".md"

        s += f"- [{moduleName}]({modulePath})\n"
        if "brief" in module:
                s += doc_text(module["brief"])
        s += "\n"

        doc_module(docDir, module, dump)

s += "\n---\n\n"
with open(os.path.join(docDir, "api_reference.md"), "w") as f:
    print(s, file=f, end="", flush=True)


def gen_mkdoc_api_nav(dirPath, desc, indent):
    s = ""
    for module in desc:
        moduleName = module["name"]
        modulePath = os.path.join(dirPath, make_dir_name(moduleName) + ".md")

        subModules = [x for x in module["contents"] if x["kind"] == "module"]
        if len(subModules):
            s += gen_indent(indent, width=2)
            s += f"- {moduleName}:\n"
            s += gen_indent(indent+1, width=2)
            s += f"- Overview: '{modulePath}'\n"
            s += gen_mkdoc_api_nav(os.path.join(dirPath, make_dir_name(moduleName)), subModules, indent+1)
        else:
            s += gen_indent(indent, width=2)
            s += f"- {moduleName}: '{modulePath}'\n"


    return s

def gen_mkdoc_yaml(outDir, desc):
    s = ("site_name: Orca Documentation\n"
         "site_url: https://docs.orca-app.dev\n"
         "site_description: Official Orca Documentation\n"
         "\n"
         "copyright: Copyright &copy; 2024 Martin Fouilleul and the Orca project contributors\n"
         "\n"
         "extra_css:\n"
         "  - css/extra.css\n"
         "theme: readthedocs\n"
         "\n"
         "nav:\n"
         "  - Home: 'index.md'\n"
         "  - Getting Started:\n"
         "    - Installation: 'install.md'\n"
         "    - Quick Start: 'QuickStart.md'\n"
         "  - User Guide:\n"
         "    - API Spec:\n"
         "      - Overview: 'api_reference.md'\n")

    s += gen_mkdoc_api_nav("api", desc, 3)

    s += ("  - Developper Guide:\n"
          "    - Building: 'building.md'\n"
          "  - FAQ: 'faq.md'\n")

    with open(os.path.join(outDir, "mkdocs.yml"), "w") as f:
        print(s, file=f, end="", flush=True)

gen_mkdoc_yaml(args.outDir, desc)
