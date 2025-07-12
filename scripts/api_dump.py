import os
import sys
import subprocess
import re
import json
import clang.cindex as cindex

def type_struct(entries, ast, tu):
    print("type struct")
    pretty_print_ast(ast, 0, sys.stdout)
    return "TODO"


def test_match(desc, ast):

    for key in ast.keys():
        if key not in desc:
            return False
        elif type(ast[key]) != type(desc[key]):
            return False
        elif isinstance(ast[key], str):
            if ast[key] != desc[key]:
                return False

        elif isinstance(ast[key], list):
            if len(ast[key]) != len(desc[key]):
                return False

            for descIt, astIt in zip(desc[key], ast[key]):
                if not test_match(descIt, astIt):
                    return False
        elif isinstance(ast[key], dict):
            if not test_match(desc[key], ast[key]):
                return False

    return True


def type_struct_or_union(entries, decl, tu):
    fields = []
    for child in decl.get_children():
        if child.kind == cindex.CursorKind.FIELD_DECL:

            field = {
                "name": child.spelling,
                "type": type_from_ast(entries, child.type, tu)
            }

            #NOTE: fields whose type is anonymous have two decl, one for the _anonymous type_ and one the _named field_.
            #      so for example, a field:
            #
            #      struct { int x; } foo;
            #
            #      would genereate:
            #
            #      {
            #          "name": "",
            #          "type": {
            #              "kind": "struct",
            #              "fields": [
            #                  {
            #                      "name": "x",
            #                      "type": {
            #                          "kind": "i32"
            #                      }
            #                  }
            #              ]
            #           }
            #      },
            #      {
            #          "name": "foo",
            #          "type": {
            #              "kind": "struct",
            #              "fields": [
            #                  {
            #                      "name": "x",
            #                      "type": {
            #                          "kind": "i32"
            #                      }
            #                  }
            #              ]
            #           }
            #      }
            #
            #      so here if previous field was an anonymous decl with a matching type, we remove the anonymous decl.
            if len(fields) > 0 and fields[-1]["name"] == "" and test_match(fields[-1]["type"], field["type"]):
                fields = fields[:-1]

            fields.append(field)
        elif child.kind == cindex.CursorKind.STRUCT_DECL or child.kind == cindex.CursorKind.UNION_DECL:
            field = {
                "name": "",
                "type": type_struct_or_union(entries, child.type.get_declaration(), tu)
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
                "kind": "enum-constant",
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

def type_proc_from_decl(entries, decl, procType, tu):

    params = []
    # for child in decl.get_children():
    #     if child.kind == cindex.CursorKind.PARM_DECL:
    #         param = {
    #             "name": child.spelling,
    #             "type": type_from_ast(entries, child.type, tu)
    #         }
    #         params.append(param)

    for i, arg in enumerate(procType.argument_types()):
        param = {
            "name": "arg" + str(i),
            "type": type_from_ast(entries, arg, tu)
        }
        params.append(param)

    t = {
        "kind": "proc",
        "return": type_from_ast(entries, procType.get_result(), tu),
        "params": params
    }
    return t

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
        "f64",
        "size_t"
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
    elif ast.kind == cindex.TypeKind.SCHAR:
        t = {
            "kind": "char"
        }
    elif ast.kind == cindex.TypeKind.UCHAR:
        t = {
            "kind": "u8"
        }
    elif ast.kind == cindex.TypeKind.CHAR_S:
        t = {
            "kind": "char"
        }
    elif ast.kind == cindex.TypeKind.SHORT:
        t = {
            "kind": "i16"
        }
    elif ast.kind == cindex.TypeKind.USHORT:
        t = {
            "kind": "u16"
        }
    elif ast.kind == cindex.TypeKind.INT:
        t = {
            "kind": "i32"
        }
    elif ast.kind == cindex.TypeKind.UINT:
        t = {
            "kind": "u32"
        }
    elif ast.kind == cindex.TypeKind.LONG:
        t = {
            "kind": "long"
        }
    elif ast.kind == cindex.TypeKind.ULONG:
        t = {
            "kind": "unsigned long"
        }
    elif ast.kind == cindex.TypeKind.LONGLONG:
        t = {
            "kind": "long long"
        }
    elif ast.kind == cindex.TypeKind.ULONGLONG:
        t = {
            "kind": "unsigned long long"
        }
    elif ast.kind == cindex.TypeKind.FLOAT:
        t = {
            "kind": "f32"
        }
    elif ast.kind == cindex.TypeKind.DOUBLE:
        t = {
            "kind": "f64"
        }
    elif ast.kind == cindex.TypeKind.LONGDOUBLE:
        t = {
            "kind": "long double"
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
            "count": ast.element_count
        }
    elif ast.kind == cindex.TypeKind.ELABORATED:

        if ast.spelling.startswith("enum (unnamed"):
            print(f"unnamed enum {ast.spelling} with kind {ast.kind}")

        if is_primitive_typedef(ast.spelling):
            t = {
                "kind": ast.spelling
            }
        else:
            decl = ast.get_declaration()
            if decl.is_anonymous():
                if decl.kind == cindex.CursorKind.STRUCT_DECL or decl.kind == cindex.CursorKind.UNION_DECL:
                    t = type_struct_or_union(entries, decl, tu)
                elif decl.kind == cindex.CursorKind.ENUM_DECL:
                    t = type_enum(entries, decl, tu)
            else:
                t = {
                    "kind": "namedType",
                    "name": ast.spelling
                }

    elif ast.kind == cindex.TypeKind.RECORD:
        #TODO: could be a union?
        t = {
            "kind": "namedType",
            "name": ast.spelling.removeprefix("struct ")
        }

    elif ast.kind == cindex.TypeKind.FUNCTIONPROTO:
        t = type_proc_from_decl(entries, ast.get_declaration(), ast, tu)
    else:
        print(f"error: unrecognized TypeKind.{ast.kind.spelling} for {ast.spelling}")
        return "NONE"

    return t

def add_entry_for_cursor(entries, entry, cursor):
    fileName = cursor.extent.start.file.name
    moduleName, _ = os.path.splitext(os.path.basename(fileName))

    if fileName not in entries:
        entries[fileName] = {
            "kind": "file",
            "name": fileName,
            "contents": []
        }

    entries[fileName]["contents"].append(entry)

def generate_proc_entry(entries, ast, tu):

    proc = {
        "kind": "proc",
        "name": ast.spelling,
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

    if ast.type.kind != cindex.TypeKind.FUNCTIONNOPROTO and ast.type.is_function_variadic():
        param = {
            "name": "...",
            "type": {
                "kind": "variadic-param"
            }
        }
        params.append(param)

    proc["params"] = params

    add_entry_for_cursor(entries, proc, ast)

def find_entry(entries, kind, name):
    if isinstance(entries, list):
        for entry in entries:
            candidate = find_entry(entry, kind, name)
            if candidate != None:
                return candidate

    elif isinstance(entries, dict):
        if "name" in entries and "kind" in entries and entries["kind"] == kind and entries["name"] == name:
            return entries
        for entry in entries.values():
            candidate = find_entry(entry, kind, name)
            if candidate != None:
                return candidate

    return None

def generate_type_entry(entries, ast, tu):
    if (ast.spelling == "uint8_t"
       or ast.spelling == "uint16_t"
       or ast.spelling == "uint32_t"
       or ast.spelling == "uint64_t"
       or ast.spelling == "int8_t"
       or ast.spelling == "int16_t"
       or ast.spelling == "int32_t"
       or ast.spelling == "int64_t"
       or ast.spelling == "size_t"):
        return

    #TODO: eliminate duplicate entries for forward typedefs of structs/union...

    entry = {
        "kind": "typename",
        "name": ast.spelling if not ast.is_anonymous() else "",
    }

    if ast.kind == cindex.CursorKind.TYPEDEF_DECL:

        underlying = ast.underlying_typedef_type

        if underlying.kind == cindex.TypeKind.RECORD or underlying.kind == cindex.TypeKind.ENUM:
            decl = underlying.get_declaration()

            if decl.kind == cindex.CursorKind.STRUCT_DECL or decl.kind == cindex.CursorKind.UNION_DECL:
                t = type_struct_or_union(entries, decl, tu)
            elif decl.kind == cindex.CursorKind.ENUM_DECL:
                t = type_enum(entries, decl, tu)
            else:
                print(f"error: unrecognized {decl.kind} in type {ast.spelling}")
                t = "NONE"
        elif underlying.kind == cindex.TypeKind.FUNCTIONPROTO:
            decl = underlying.get_declaration()
            t = type_proc_from_decl(entries, decl, underlying, tu)
        elif underlying.kind == cindex.TypeKind.POINTER and underlying.get_pointee().kind == cindex.TypeKind.FUNCTIONPROTO:
            decl = underlying.get_declaration()
            t = type_proc_from_decl(entries, decl, underlying.get_pointee(), tu)
        else:
            t = type_from_ast(entries, underlying, tu)

    elif ast.kind == cindex.CursorKind.STRUCT_DECL or ast.kind == cindex.CursorKind.UNION_DECL:
        t = type_struct_or_union(entries, ast, tu)
    elif ast.kind == cindex.CursorKind.ENUM_DECL:
        t = type_enum(entries, ast, tu)
    else:
        print(f"error: unrecognized {ast.kind} in type {ast.spelling}")
        t = "NONE"

    entry["type"] = t

    add_entry_for_cursor(entries, entry, ast)


def get_api_entries():
    # Get clang ast dump
    index = cindex.Index.create()
    tu = index.parse('src/orca.h', args=['--target=wasm32', '--sysroot=src/orca-libc', '-I', 'src', '-I', 'src/ext'])

    # Dump api
    entries = dict()

    for cursor in tu.cursor.get_children():
        cursorFile = cursor.extent.start.file
        if (cursorFile != None
            and cursorFile.name.startswith('src')
            and not cursorFile.name.startswith('src/orca-libc')
            and not cursorFile.name.startswith('src/ext')):

            if cursor.kind == cindex.CursorKind.FUNCTION_DECL:
                generate_proc_entry(entries, cursor, tu)

            if (cursor.kind == cindex.CursorKind.TYPEDEF_DECL
                or cursor.kind == cindex.CursorKind.STRUCT_DECL
                or cursor.kind == cindex.CursorKind.UNION_DECL
                or cursor.kind == cindex.CursorKind.ENUM_DECL):
                generate_type_entry(entries, cursor, tu)

    # TODO: also extract macros

    return entries

if __name__ == "__main__":
    entries = get_api_entries()
    dump = json.dumps(list(entries.values()), indent=4)
    with open("api_dump.json", "w") as f:
        print(dump, file=f)
