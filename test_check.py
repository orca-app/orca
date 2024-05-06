import subprocess
import json
import re

def iterate_json(d):
    if isinstance(d, dict):

        yield d
        for key, value in d.items():
            for x in iterate_json(value):
                yield x
    elif isinstance(d, list):
        for v in d:
            for x in iterate_json(v):
                yield x
    else:
        yield d


#def check_proc(cdecl, spec):
#    for i, param in enumerate(spec["params"]):

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


def type_from_ast(generatedEntries, astType):
    return "void"



def generate_proc_entry(generatedEntries, specProcs, ast):
    proc = {
        "kind": "proc",
        "name": ast["name"],
    }

    #TODO: parse return type
    params = []
    if "inner" in ast:
        astParams = [x for x in ast["inner"] if x["kind"] == "ParmVarDecl"]
        for astParam in astParams:
            param = {
                "name": astParam["name"],
                "type": type_from_ast(generatedEntries, astParam["type"])
            }
            params.append(param)

    proc["params"] = params

    generatedEntries.append(proc)


# get clang ast dump and extract API decls
r = subprocess.run(['clang',
                    '-Xclang', '-ast-dump=json',
                    '-DOC_PLATFORM_ORCA=1',
                    '-I', 'src',
                    '-I', 'src/ext',
                    '-c',
                    'src/orca.h'],
                    capture_output=True)

astDump = json.loads(r.stdout)
apiNames = get_public_api_names()
astProcs = {x["name"]:x for x in iterate_json(astDump) if isinstance(x, dict) and "kind" in x and x["kind"] == "FunctionDecl" and x["name"] in apiNames}

# Load bindings spec

with open("api/test.json", "r") as f:
    spec = json.load(f)

specProcs = {x["name"]:x for x in spec if x["kind"] == "proc"}
generatedEntries = []
# check each ast proc against spec procs

for astProc in astProcs.values():
    name = astProc["name"]
    if name not in specProcs:
        print(f"warning: procedure {name} has no binding entry")
        generate_proc_entry(generatedEntries, specProcs, astProc)
    else:
        print(f"binding entry found for procedure {name}")
        #TODO: check entry

print(generatedEntries)
