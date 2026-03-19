#include "json.c"

int main()
{
    oc_str8 string = { 0 };
    oc_file file = oc_file_open(OC_STR8("test.json"), OC_FILE_ACCESS_READ, OC_FILE_OPEN_DEFAULT);

    oc_arena arena = { 0 };
    oc_arena_init(&arena);

    string.len = oc_file_size(file);
    string.ptr = oc_arena_push(&arena, string.len);

    oc_file_read(file, string.len, string.ptr);
    oc_file_close(file);

    json_node* node = json_parse_str8(&arena, string);
    json_print(node, 0);
    return (0);
}
