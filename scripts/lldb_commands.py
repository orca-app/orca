import lldb
import textwrap

def oc_list_print(debugger, command, result, internal_dict):
    args = command.split(maxsplit=2)
    if len(args) < 3:
        print("Usage: print_list <list_arg> <type_arg> <link_arg>")
        return
    list_arg = args[0].strip()
    type_arg = args[1].strip()
    link_arg = args[2].strip()

    frame = debugger.GetSelectedTarget().GetProcess().GetSelectedThread().GetSelectedFrame()
    list_val = frame.EvaluateExpression(list_arg)

    if list_val.IsValid():
        elt = list_val.GetChildMemberWithName('first')

        index = 0
        while elt.GetValueAsUnsigned() != 0:
            offset = frame.EvaluateExpression(f"offsetof({type_arg}, {link_arg})").GetValueAsSigned()
            entry = frame.EvaluateExpression(f"*({type_arg}*)((char*)({elt.GetValueAsUnsigned()}) + {offset})")

            print(f"[{index}] = 0x{elt.GetValueAsUnsigned():016x}: {entry}\n")
            elt = elt.GetChildMemberWithName('next')
            index = index + 1
    else:
        print(f"Error: Could not find variable '{list_val}'")


def oc_list_tree_node_print(frame, node, children_arg, link_arg, suffix_arg, indent):

    indentation = ''
    for i in range(indent):
        indentation = indentation + '  '

    val = node.Dereference()
    t = val.GetType()
    offset = frame.EvaluateExpression(f"offsetof({t.GetName()}, {link_arg})").GetValueAsSigned()

    displayVal = val
    if suffix_arg != '':
        displayVal = val.EvaluateExpression(f"{suffix_arg}")

    s = textwrap.indent(f"{displayVal.__str__()}", indentation)
    print(s)

    children = node.GetChildMemberWithName(children_arg)
    childElt = children.GetChildMemberWithName("first")

    while childElt.GetValueAsUnsigned() != 0:
        child = frame.EvaluateExpression(f"({t.GetName()}*)((char*)({childElt.GetValueAsUnsigned()}) + {offset})")
        oc_list_tree_node_print(frame, child, children_arg, link_arg, suffix_arg, indent+1)
        childElt = childElt.GetChildMemberWithName('next')

def oc_list_tree_print(debugger, command, result, internal_dict):
    args = command.split(maxsplit=4)
    if len(args) < 3:
        print("Usage: print_list <root> <children_member> <link_member> (suffix_arg)")
        return
    root_arg = args[0].strip()
    children_arg = args[1].strip()
    link_arg = args[2].strip()
    suffix_arg = args[3].strip() if len(args) > 3 else ''

    frame = debugger.GetSelectedTarget().GetProcess().GetSelectedThread().GetSelectedFrame()
    root_val = frame.EvaluateExpression(root_arg)

    if root_val.IsValid():
        oc_list_tree_node_print(frame, root_val, children_arg, link_arg, suffix_arg, 0)

    else:
        print(f"Error: Could not find variable '{root_val}'")


def __lldb_init_module(debugger, internal_dict):
    debugger.HandleCommand(f'command script add -f {__name__}.oc_list_print print_list')
    debugger.HandleCommand(f'command script add -f {__name__}.oc_list_tree_print print_tree')
