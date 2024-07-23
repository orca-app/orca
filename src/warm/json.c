#include <math.h>
#include <stdio.h>

#define OC_NO_APP_LAYER 1
#include "orca.h"

typedef enum json_node_kind
{
    JSON_NULL,
    JSON_TRUE,
    JSON_FALSE,
    JSON_NUM_F64,
    JSON_NUM_I64,
    JSON_STRING,
    JSON_OBJECT,
    JSON_LIST,
} json_node_kind;

const char* json_node_kind_strings[] = {
    "null",
    "true",
    "false",
    "num f64",
    "num i64",
    "string",
    "object",
    "list",
};

typedef struct json_node json_node;

typedef struct json_node
{
    oc_list_elt listElt;
    oc_list children;
    u64 childCount;
    json_node* parent;

    json_node_kind kind;
    oc_str8 string;
    oc_str8 name;

    bool boolVal;
    f64 numF64;
    i64 numI64;

} json_node;

typedef enum json_lex_kind
{
    JSON_LEX_UNKNOWN,
    JSON_LEX_UNKNOWN_ESCAPE,
    JSON_LEX_EOF,
    JSON_LEX_LBRACE,
    JSON_LEX_RBRACE,
    JSON_LEX_LBRACKET,
    JSON_LEX_RBRACKET,
    JSON_LEX_COLON,
    JSON_LEX_COMMA,
    JSON_LEX_F64,
    JSON_LEX_I64,
    JSON_LEX_TRUE,
    JSON_LEX_FALSE,
    JSON_LEX_NULL,
    JSON_LEX_STRING,
} json_lex_kind;

static const char* json_lex_strings[] = {
    "unknown",
    "unsupported escape sequence",
    "end of file",
    "left brace",
    "right brace",
    "left bracket",
    "right bracket",
    "colon",
    "comma",
    "f64 num",
    "i64 num",
    "true",
    "false",
    "null",
    "string",
};

typedef struct json_lex
{
    json_lex_kind kind;
    oc_str8 string;

    bool boolVal;
    f64 numF64;
    i64 numI64;

} json_lex;

typedef struct json_parser
{
    oc_arena* arena;
    oc_str8 contents;
    u64 offset;
} json_parser;

json_lex json_lex_number(json_parser* parser)
{
    json_lex lex = { 0 };

    oc_str8 contents = parser->contents;
    u64 startOffset = parser->offset;

    u64 numberU64 = 0;
    bool minus = false;

    if(parser->offset < contents.len && contents.ptr[0] == '-')
    {
        minus = true;
        parser->offset++;
    }

    while(parser->offset < contents.len)
    {
        char c = contents.ptr[parser->offset];
        if(c >= '0' && c <= '9')
        {
            numberU64 *= 10;
            numberU64 += c - '0';
            parser->offset += 1;
        }
        else
        {
            break;
        }
    }

    f64 numberF64;
    if(parser->offset < contents.len
       && contents.ptr[parser->offset] == '.')
    {
        parser->offset += 1;

        u64 decimals = 0;
        u64 decimalCount = 0;

        while(parser->offset < contents.len)
        {
            char c = contents.ptr[parser->offset];
            if(c >= '0' && c <= '9')
            {
                decimals *= 10;
                decimals += c - '0';
                parser->offset += 1;
                decimalCount += 1;
            }
            else
            {
                break;
            }
        }

        lex.kind = JSON_LEX_F64;
        lex.numF64 = (f64)numberU64 + (f64)decimals / pow(10, decimalCount);
        if(minus)
        {
            lex.numF64 = -lex.numF64;
        }
    }
    else
    {
        lex.kind = JSON_LEX_I64;
        lex.numI64 = numberU64;
        if(minus)
        {
            lex.numI64 = -lex.numI64;
        }
    }
    lex.string.len = parser->offset - startOffset;
    return (lex);
}

json_lex json_lex_string(json_parser* parser)
{
    json_lex lex = { 0 };

    parser->offset++;
    u64 startOffset = parser->offset;

    while(parser->offset < parser->contents.len
          && parser->contents.ptr[parser->offset] != '"')
    {
        if(parser->contents.ptr[parser->offset] == '\\')
        {
            parser->offset++;
            if(parser->offset < parser->contents.len)
            {
                switch(parser->contents.ptr[parser->offset])
                {
                    //NOTE: escape sequences are converted later when using the lex
                    case 'b':
                    case 'f':
                    case 'n':
                    case 'r':
                    case 't':
                    case '"':
                    case '\\':
                        break;
                    default:
                        lex.kind = JSON_LEX_UNKNOWN_ESCAPE;
                        lex.string = oc_str8_slice(parser->contents, startOffset, parser->offset);
                        break;
                }
            }
            else
            {
                break;
            }
        }
        parser->offset++;
    }

    lex.string = oc_str8_slice(parser->contents, startOffset, parser->offset);

    if(parser->offset < parser->contents.len
       && parser->contents.ptr[parser->offset] == '"')
    {
        lex.kind = JSON_LEX_STRING;
        parser->offset++;
    }

    return (lex);
}

json_lex json_lex_bool_or_null(json_parser* parser)
{
    json_lex lex = {
        .string = {
            .ptr = parser->contents.ptr + parser->offset,
            .len = 1,
        },
    };

    u64 rem = parser->contents.len - parser->offset;

    if(rem >= 4 && !strncmp(parser->contents.ptr + parser->offset, "null", 4))
    {
        lex.kind = JSON_LEX_NULL;
        lex.string.len = 4;
        parser->offset += 4;
    }
    else if(rem >= 4 && !strncmp(parser->contents.ptr + parser->offset, "true", 4))
    {
        lex.kind = JSON_LEX_TRUE;
        lex.string.len = 4;
        parser->offset += 4;
    }
    else if(rem >= 5 && !strncmp(parser->contents.ptr + parser->offset, "false", 5))
    {
        lex.kind = JSON_LEX_FALSE;
        lex.string.len = 5;
        parser->offset += 5;
    }

    return lex;
}

bool json_is_blank(char c)
{
    return (c == ' ' || c == '\t' || c == '\n');
}

json_lex json_lex_next(json_parser* parser)
{
    json_lex lex = { 0 };

    while(parser->offset < parser->contents.len
          && json_is_blank(parser->contents.ptr[parser->offset]))
    {
        parser->offset++;
    }

    if(parser->offset >= parser->contents.len)
    {
        lex = (json_lex){
            .kind = JSON_LEX_EOF,
            .string = {
                .ptr = parser->contents.ptr + parser->offset,
                .len = 0,
            },
        };
    }
    else
    {
        char c = parser->contents.ptr[parser->offset];

        if(c == '-' || (c >= '0' && c <= '9'))
        {
            lex = json_lex_number(parser);
        }
        else
        {
            lex.string = (oc_str8){
                .ptr = parser->contents.ptr + parser->offset,
                .len = 1,
            };

            switch(c)
            {
                case '{':
                    lex.kind = JSON_LEX_LBRACE;
                    parser->offset++;
                    break;
                case '}':
                    lex.kind = JSON_LEX_RBRACE;
                    parser->offset++;
                    break;
                case '[':
                    lex.kind = JSON_LEX_LBRACKET;
                    parser->offset++;
                    break;
                case ']':
                    lex.kind = JSON_LEX_RBRACKET;
                    parser->offset++;
                    break;
                case ':':
                    lex.kind = JSON_LEX_COLON;
                    parser->offset++;
                    break;
                case ',':
                    lex.kind = JSON_LEX_COMMA;
                    parser->offset++;
                    break;
                case '"':
                    lex = json_lex_string(parser);
                    break;
                case 't':
                case 'f':
                case 'n':
                    lex = json_lex_bool_or_null(parser);
                    break;
                default:
                    lex.kind = JSON_LEX_UNKNOWN;
                    parser->offset++;
                    break;
            }
        }
    }
    return (lex);
}

json_lex json_lex_consume_if(json_parser* parser, json_lex_kind kind)
{
    u64 offset = parser->offset;
    json_lex lex = json_lex_next(parser);
    if(lex.kind != kind)
    {
        parser->offset = offset;
    }
    return (lex);
}

void json_print_lex(json_lex lex)
{
    printf("%s \"%.*s\"", json_lex_strings[lex.kind], oc_str8_ip(lex.string));
    switch(lex.kind)
    {
        case JSON_LEX_F64:
            printf(": %f ", lex.numF64);
            break;
        case JSON_LEX_I64:
            printf(": %lli ", lex.numI64);
            break;
        default:
            break;
    }
    printf("\n");
}

void json_lex_error(json_parser* parser, json_lex* lex)
{
    oc_log_error("unexpected lex %s at offset %llu\n",
                 json_lex_strings[lex->kind],
                 lex->string.ptr - parser->contents.ptr);
}

json_node* json_node_alloc(json_parser* parser, json_node_kind kind)
{
    json_node* node = oc_arena_push_type(parser->arena, json_node);
    memset(node, 0, sizeof(json_node));
    node->kind = kind;
    return (node);
}

void json_node_add_child(json_node* node, json_node* child)
{
    child->parent = node;
    oc_list_push_back(&node->children, &child->listElt);
    node->childCount++;
}

oc_str8 json_convert_escaped_string(oc_arena* arena, oc_str8 string)
{
    oc_str8 result = {
        .ptr = oc_arena_push(arena, string.len),
        .len = 0,
    };

    for(u64 offset = 0; offset < string.len; offset++)
    {
        if(string.ptr[offset] == '\\')
        {
            OC_ASSERT(offset + 1 < string.len);
            offset++;

            switch(string.ptr[offset])
            {
                case 'b':
                    result.ptr[result.len] = '\b';
                    break;
                case 'f':
                    result.ptr[result.len] = '\f';
                    break;
                case 'n':
                    result.ptr[result.len] = '\n';
                    break;
                case 'r':
                    result.ptr[result.len] = '\r';
                    break;
                case 't':
                    result.ptr[result.len] = '\t';
                    break;
                case '"':
                    result.ptr[result.len] = '\"';
                    break;
                case '\\':
                    result.ptr[result.len] = '\\';
                    break;

                default:
                    OC_ASSERT(0, "unreachable");
                    break;
            }
        }
        else
        {
            result.ptr[result.len] = string.ptr[offset];
        }
        result.len++;
    }
    return (result);
}

json_node* json_parse_item(json_parser* parser);

json_node* json_parse_object(json_parser* parser)
{
    json_node* node = json_node_alloc(parser, JSON_OBJECT);

    json_lex lex = json_lex_consume_if(parser, JSON_LEX_RBRACKET);
    if(lex.kind != JSON_LEX_RBRACE)
    {
        do
        {
            json_lex name = json_lex_next(parser);
            if(name.kind != JSON_LEX_STRING)
            {
                json_lex_error(parser, &name);
                return 0;
            }

            json_lex colon = json_lex_next(parser);
            if(colon.kind != JSON_LEX_COLON)
            {
                json_lex_error(parser, &colon);
                return 0;
            }

            json_node* child = json_parse_item(parser);
            if(!child)
            {
                return 0;
            }

            child->name = json_convert_escaped_string(parser->arena, name.string);
            json_node_add_child(node, child);

            lex = json_lex_next(parser);
        }
        while(lex.kind == JSON_LEX_COMMA);
    }

    if(lex.kind != JSON_LEX_RBRACE)
    {
        json_lex_error(parser, &lex);
        return (0);
    }

    return node;
}

json_node* json_parse_list(json_parser* parser)
{
    json_node* node = json_node_alloc(parser, JSON_LIST);

    json_lex lex = json_lex_consume_if(parser, JSON_LEX_RBRACKET);
    if(lex.kind != JSON_LEX_RBRACKET)
    {
        do
        {
            json_node* child = json_parse_item(parser);
            if(!child)
            {
                return 0;
            }
            json_node_add_child(node, child);
            lex = json_lex_next(parser);
        }
        while(lex.kind == JSON_LEX_COMMA);
    }

    if(lex.kind != JSON_LEX_RBRACKET)
    {
        json_lex_error(parser, &lex);
        return (0);
    }
    return node;
}

json_node* json_make_from_lex(json_parser* parser, json_lex* lex)
{
    json_node* node = oc_arena_push_type(parser->arena, json_node);
    memset(node, 0, sizeof(json_node));

    switch(lex->kind)
    {
        case JSON_LEX_F64:
            node->kind = JSON_NUM_F64;
            node->numF64 = lex->numF64;
            break;
        case JSON_LEX_I64:
            node->kind = JSON_NUM_I64;
            node->numI64 = lex->numI64;
            break;
        case JSON_LEX_TRUE:
            node->kind = JSON_TRUE;
            break;
        case JSON_LEX_FALSE:
            node->kind = JSON_FALSE;
            break;
        case JSON_LEX_NULL:
            node->kind = JSON_NULL;
            break;
        case JSON_LEX_STRING:
            node->kind = JSON_STRING;
            node->string = json_convert_escaped_string(parser->arena, lex->string);
            break;

        default:
            OC_ASSERT(0, "unreachable");
            break;
    }
    return (node);
}

json_node* json_parse_item(json_parser* parser)
{
    json_node* node = 0;
    json_lex lex = json_lex_next(parser);

    switch(lex.kind)
    {
        case JSON_LEX_LBRACE:
            node = json_parse_object(parser);
            break;
        case JSON_LEX_LBRACKET:
            node = json_parse_list(parser);
            break;
        case JSON_LEX_STRING:
        case JSON_LEX_I64:
        case JSON_LEX_F64:
        case JSON_LEX_TRUE:
        case JSON_LEX_FALSE:
        case JSON_LEX_NULL:
            node = json_make_from_lex(parser, &lex);
            break;
        default:
            json_lex_error(parser, &lex);
            break;
    }

    return (node);
}

json_node* json_parse_str8(oc_arena* arena, oc_str8 string)
{
    json_parser parser = {
        .arena = arena,
        .contents = string,
        .offset = 0,
    };

    json_node* node = json_parse_item(&parser);

    return (node);
}

void json_print(json_node* node, int indent)
{
    for(int i = 0; i < indent; i++)
    {
        printf("  ");
    }

    printf("%s", json_node_kind_strings[node->kind]);
    switch(node->kind)
    {
        case JSON_TRUE:
            printf(": true");
            break;
        case JSON_FALSE:
            printf(": false");
            break;
        case JSON_NULL:
            printf(": null");
            break;
        case JSON_NUM_F64:
            printf(": %f", node->numF64);
            break;
        case JSON_NUM_I64:
            printf(": %lli", node->numI64);
            break;
        case JSON_STRING:
            printf(": %.*s", oc_str8_ip(node->string));
            break;
        default:
            break;
    }
    printf("\n");

    oc_list_for(node->children, child, json_node, listElt)
    {
        int childIndent = indent + 1;
        if(node->kind == JSON_OBJECT)
        {
            for(int i = 0; i < childIndent; i++)
            {
                printf("  ");
            }
            printf("%.*s:\n", oc_str8_ip(child->name));
            childIndent++;
        }
        json_print(child, childIndent);
    }
}

json_node* json_find(json_node* node, oc_str8 name)
{
    json_node* res = 0;
    if(node->kind == JSON_OBJECT)
    {
        oc_list_for(node->children, child, json_node, listElt)
        {
            if(!oc_str8_cmp(child->name, name))
            {
                res = child;
                break;
            }
        }
    }
    return (res);
}
