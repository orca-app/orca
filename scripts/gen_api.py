import os
import sys
import subprocess
import re
import json
import clang.cindex as cindex


def print_indent(indent, f):
    for i in range(indent):
        print("  ", end='', file=f)

def pretty_print_ast(index, indent, f):
    print_indent(indent, f)
    print(f"{index.kind} ({index.spelling})", file=f)

    if hasattr(index, "type"):
        print_indent(indent, f)
        print("type:", file=f)
        pretty_print_ast(index.type, indent+1, f)

    if hasattr(index, "get_children"):
        children = list(index.get_children())
        if len(children):
            print_indent(indent, f)
            print("children:", file=f)
            for child in children:
                pretty_print_ast(child, indent+1, f)


def get_public_api_names():
    # get function names of public APIs
    r = subprocess.run("clang -E -fdirectives-only -DOC_PLATFORM_ORCA=1 -I src -I src/ext src/orca.h | grep '^[[:blank:]]*ORCA_API'", shell=True, capture_output=True)
    lines = r.stdout.decode().splitlines()
    regex = re.compile('.* ([a-z_][a-z_0-9]*)\\(')

    names = []
    for line in lines:
        m = regex.search(line)
        if m == None:
            print(f"error: could match function name regex on line '{line}'")
            exit()
        names.append(m.group(1))
    return names

def type_struct(entries, ast, tu):
    print("type struct")
    pretty_print_ast(ast, 0, sys.stdout)
    return "TODO"

def type_struct_or_union(entries, decl, tu):
    fields = []
    for child in decl.get_children():
        if child.kind == cindex.CursorKind.FIELD_DECL:
            field = {
                "name": child.spelling,
                "type": type_from_ast(entries, child.type, tu)
            }
            fields.append(field)

    t = {
        "kind": "struct" if decl.kind == cindex.CursorKind.STRUCT_DECL else "union",
    }
    if len(fields):
        t["fields"] = fields

    return t

def type_enum(entries, decl, tu):
    constants = []
    for child in decl.get_children():
        if child.kind == cindex.CursorKind.ENUM_CONSTANT_DECL:
            constant = {
                "name": child.spelling,
                "value": child.enum_value
            }
        constants.append(constant)

    t = {
        "kind": "enum",
        "type": type_from_ast(entries, decl.enum_type, tu),
        "constants": constants
    }
    return t

def type_proc(entries, decl, procType, tu):

    params = []
    for child in decl.get_children():
        if child.kind == cindex.CursorKind.PARM_DECL:
            param = {
                "name": child.spelling,
                "type": type_from_ast(entries, child.type, tu)
            }

    t = {
        "return": type_from_ast(entries, procType.get_result(), tu),
        "params": params
    }
    return t

def add_named_type_if_needed(entries, decl, ast, tu):
    name = ast.spelling

    if name not in entries:

        name = name.removeprefix("struct ")
        name = name.removeprefix("union ")
        entries[name] = "in progress"

        canonical = ast.get_canonical()

        if canonical.kind == cindex.TypeKind.RECORD or canonical.kind == cindex.TypeKind.ENUM:

            decl = ast.get_canonical().get_declaration()

            if decl.kind == cindex.CursorKind.STRUCT_DECL or decl.kind == cindex.CursorKind.UNION_DECL:
                t = type_struct_or_union(entries, decl, tu)
            elif decl.kind == cindex.CursorKind.ENUM_DECL:
                t = type_enum(entries, decl, tu)
            else:
                print(f"error: unrecognized {decl.kind} in named type {name}")
                t = "NONE"

        elif canonical.kind == cindex.TypeKind.POINTER and canonical.get_pointee().kind == cindex.TypeKind.FUNCTIONPROTO:
            decl = ast.get_declaration()
            t = type_proc(entries, decl, canonical.get_pointee(), tu)

        else:
            t = type_from_ast(entries, canonical, tu)

        typename = {
            "kind": "typename",
            "name": name,
            "extents": get_extents(decl),
            "type": t
        }

        entries[name] = typename

def is_primitive_typedef(name):
    primitives = [
        "u8",
        "i8",
        "u16",
        "i16",
        "u32",
        "i32",
        "u64",
        "i64",
        "f32",
        "f64"
    ]
    return name in primitives

def type_from_ast(entries, ast, tu):

    if ast.kind == cindex.TypeKind.VOID:
        t = {
            "kind": "void",
        }
    elif ast.kind == cindex.TypeKind.BOOL:
        t = {
            "kind": "bool",
        }
    elif ast.kind == cindex.TypeKind.CHAR_S:
        t = {
            "kind": "char"
        }
    elif ast.kind == cindex.TypeKind.INT:
        t = {
            "kind": "i32"
        }
    elif ast.kind == cindex.TypeKind.UINT:
        t = {
            "kind": "u32"
        }
    elif ast.kind == cindex.TypeKind.FLOAT:
        t = {
            "kind": "f32"
        }
    elif ast.kind == cindex.TypeKind.DOUBLE:
        t = {
            "kind": "f64"
        }
    elif ast.kind == cindex.TypeKind.POINTER:
        t = {
            "kind": "pointer",
            "type": type_from_ast(entries, ast.get_pointee(), tu)
        }
    elif ast.kind == cindex.TypeKind.CONSTANTARRAY:
        t = {
            "kind": "array",
            "type": type_from_ast(entries, ast.element_type, tu),
            "len": ast.element_count
        }
    elif ast.kind == cindex.TypeKind.ELABORATED:
        if is_primitive_typedef(ast.spelling):
            #NOTE desugar primitive typedefs directly here
            canonical = ast.get_canonical()
            t = type_from_ast(entries, canonical, tu)
        else:
            t = {
                "kind": "namedType",
                "name": ast.spelling
            }
            decl = ast.get_declaration()
            add_named_type_if_needed(entries, decl, ast, tu)

    elif ast.kind == cindex.TypeKind.RECORD:
        #TODO: could be a union?
        t = {
            "kind": "namedType",
            "name": ast.spelling.removeprefix("struct ")
        }
        decl = ast.get_declaration()
        add_named_type_if_needed(entries, decl, ast, tu)

    else:
        print(f"error: unrecognized TypeKind.{ast.kind.spelling}")
        return "NONE"

    return t

def get_extents(ast):
    start = ast.extent.start
    end = ast.extent.end

    e = {
        "file": start.file.name,
        "start": {
            "line": start.line,
            "column": start.column,
            "offset": start.offset
        },
        "end": {
            "line": end.line,
            "column": end.column,
            "offset": end.offset
        }
    }
    return e

def generate_proc_entry(entries, ast, tu):

    proc = {
        "kind": "proc",
        "name": ast.spelling,
        "extents": get_extents(ast),
        "return": type_from_ast(entries, ast.type.get_result(), tu)
    }

    params = []
    if hasattr(ast, "get_children"):
        astParams = [x for x in ast.get_children() if x.kind == cindex.CursorKind.PARM_DECL]
        for astParam in astParams:
            param = {
                "name": astParam.spelling,
                "type": type_from_ast(entries, astParam.type, tu)
            }
            params.append(param)

    proc["params"] = params
    entries[proc['name']] = proc


# Get clang ast dump
index = cindex.Index.create()
tu = index.parse('src/orca.h', args=['-I', 'src', '-I', 'src/ext', '-DOC_PLATFORM_ORCA=1'])

# Store ast to file for debugging
# with open("ast.txt", "w") as f:
#     pretty_print_ast(tu.cursor, 0, f)

# Get public api names
apiNames = get_public_api_names()
procs = []

for cursor in tu.cursor.walk_preorder():
    if cursor.kind == cindex.CursorKind.FUNCTION_DECL and cursor.spelling in apiNames:
        procs.append(cursor)

# Generate bindings
entries = dict()

for proc in procs:
    generate_proc_entry(entries, proc, tu)


# Load existing bindings
oldEntries = dict()

if os.path.exists("src/api.json"):
    with open("src/api.json", "r") as f:
        oldSpec = json.load(f)

    for spec in oldSpec:
        oldEntries[spec["name"]] = spec


# for each generated bindings, check if we can instead pull the existing ones

outSpec = []


def check_entry_match(old, new):
    for key in new.keys():
        if key not in old:
            return False
        elif type(new[key]) != type(old[key]):
            return False
        elif isinstance(new[key], str):
            if new[key] != old[key]:
                return False
        elif isinstance(new[key], list):
            if len(new[key]) != len(old[key]):
                return False
            for oldIt, newIt in zip(old[key], new[key]):
                if not check_entry_match(oldIt, newIt):
                    return False
        elif isinstance(new[key], dict):
            if not check_entry_match(old[key], new[key]):
                return False

    return True

for name, entry in entries.items():
    if name in oldEntries:
        oldEntry = oldEntries[name]
        if check_entry_match(oldEntry, entry):
            outSpec.append(oldEntry)
        else:
            print(f"entry {name} didn't match headers and was replaced by an up-to-date version.")
            outSpec.append(entry)
    else:
        print(f"entry {name} was not found and was generated from the headers.")
        outSpec.append(entry)

removed = [name for name in oldEntries.keys() if name not in entries]
for name in removed:
    print(f"entry {name} was not used anymore and was removed.")

dump = json.dumps(outSpec, indent=2)

with open("src/api.json", "w") as f:
    print(dump, file=f)


# TODO compare generated api spec with comitted spec, warn about mismatch, prune old defs / generate new defs
