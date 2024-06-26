#include "wa_types.h"
#include "wasm_tables.c"

//-------------------------------------------------------------------------
// Input
//-------------------------------------------------------------------------
typedef enum wa_input_status
{
    WA_INPUT_OK = 0,
    WA_INPUT_EOF,
    WA_INPUT_OVERFLOW,
} wa_input_status;

typedef struct wa_input
{
    char* contents;
    u64 len;
    u64 offset;
    wa_input_status status;

} wa_input;

char* wa_input_head(wa_input* file)
{
    return (file->contents + file->offset);
}

bool wa_input_end(wa_input* file)
{
    return (file->offset >= file->len);
}

void wa_input_seek(wa_input* file, u64 offset)
{
    if(offset > file->len)
    {
        file->offset = file->len;
        file->status = WA_INPUT_EOF;
    }
    else
    {
        file->offset = offset;
    }
}

int wa_read_status(wa_input* file)
{
    return (file->status);
}

u8 wa_read_byte(wa_input* file)
{
    if(file->status)
    {
        return (0);
    }
    if(file->offset + sizeof(u8) > file->len)
    {
        file->status = WA_INPUT_EOF;
        return (0);
    }

    u8 r = *(u8*)&file->contents[file->offset];
    file->offset += sizeof(u8);
    return (r);
}

u32 wa_read_raw_u32(wa_input* file)
{
    if(file->status)
    {
        return (0);
    }
    if(file->offset + sizeof(u32) > file->len)
    {
        file->status = WA_INPUT_EOF;
        return (0);
    }

    u32 r = *(u32*)&file->contents[file->offset];
    file->offset += sizeof(u32);
    return (r);
}

u64 wa_read_leb128_u64(wa_input* file)
{
    if(file->status)
    {
        return (0);
    }

    char byte = 0;
    u64 shift = 0;
    u64 res = 0;

    do
    {
        if(file->offset + sizeof(char) > file->len)
        {
            file->status = WA_INPUT_EOF;
            return (0);
        }
        byte = file->contents[file->offset];
        file->offset++;

        res |= ((u64)byte & 0x7f) << shift;
        shift += 7;
    }
    while(byte & 0x80);

    return (res);
}

i64 wa_read_leb128_i64(wa_input* file)
{
    if(file->status)
    {
        return (0);
    }

    char byte = 0;
    u64 shift = 0;
    i64 res = 0;

    do
    {
        if(file->offset + sizeof(char) > file->len)
        {
            file->status = WA_INPUT_EOF;
            return (0);
        }
        byte = file->contents[file->offset];
        file->offset++;

        res |= ((u64)byte & 0x7f) << shift;
        shift += 7;
    }
    while(byte & 0x80);

    if(shift < 64 && (byte & 0x40))
    {
        res |= (~0ULL << shift);
    }

    return (res);
}

u32 wa_read_leb128_u32(wa_input* file)
{
    if(file->status)
    {
        return (0);
    }

    u64 r = wa_read_leb128_u64(file);
    if(r >= INT32_MAX)
    {
        r = 0;
        file->status = WA_INPUT_OVERFLOW;
    }
    return (u32)r;
}

f32 wa_read_f32(wa_input* file)
{
    if(file->status)
    {
        return (0);
    }
    if(file->offset + sizeof(f32) > file->len)
    {
        file->status = WA_INPUT_EOF;
        return (0);
    }
    f32 f = *(f32*)&file->contents[file->offset];
    file->offset += sizeof(f32);
    return f;
}

f64 wa_read_f64(wa_input* file)
{
    if(file->status)
    {
        return (0);
    }
    if(file->offset + sizeof(f64) > file->len)
    {
        file->status = WA_INPUT_EOF;
        return (0);
    }
    f64 f = *(f64*)&file->contents[file->offset];
    file->offset += sizeof(f64);
    return f;
}

oc_str8 wa_read_name(wa_input* file, oc_arena* arena)
{
    u32 len = wa_read_leb128_u32(file);
    if(file->status)
    {
        return (oc_str8){ 0 };
    }
    if(file->offset + len > file->len)
    {
        file->status = WA_INPUT_EOF;
        return (oc_str8){ 0 };
    }
    oc_str8 res = {
        .ptr = &file->contents[file->offset],
        .len = len,
    };
    file->offset += len;
    return (res);
}

//-------------------------------------------------------------------------
// Parsing
//-------------------------------------------------------------------------

void wa_parse_sections(wa_input* input, wa_module* module)
{
    u32 magic = wa_read_raw_u32(input);
    u32 version = wa_read_raw_u32(input);

    if(wa_read_status(input))
    {
        oc_log_error("Couldn't read wasm magic number and version.\n");
        exit(-1);
    }
    //TODO check magic / version

    while(!wa_input_end(input))
    {
        u8 sectionID = wa_read_byte(input);
        u32 sectionLen = wa_read_leb128_u32(input);

        u64 contentOffset = input->offset;
        //TODO: check read errors

        if(wa_read_status(input))
        {
            oc_log_error("Input error.\n");
            exit(-1);
        }

        switch(sectionID)
        {
            case 0:
                break;

                //TODO: check if section was already defined...

            case 1:
            {
                module->toc.types.offset = input->offset;
                module->toc.types.len = sectionLen;
            }
            break;

            case 2:
            {
                module->toc.imports.offset = input->offset;
                module->toc.imports.len = sectionLen;
            }
            break;

            case 3:
            {
                module->toc.functions.offset = input->offset;
                module->toc.functions.len = sectionLen;
            }
            break;

            case 4:
            {
                module->toc.tables.offset = input->offset;
                module->toc.tables.len = sectionLen;
            }
            break;

            case 5:
            {
                module->toc.memory.offset = input->offset;
                module->toc.memory.len = sectionLen;
            }
            break;

            case 6:
            {
                module->toc.globals.offset = input->offset;
                module->toc.globals.len = sectionLen;
            }
            break;

            case 7:
            {
                module->toc.exports.offset = input->offset;
                module->toc.exports.len = sectionLen;
            }
            break;

            case 8:
            {
                module->toc.start.offset = input->offset;
                module->toc.start.len = sectionLen;
            }
            break;

            case 9:
            {
                module->toc.elements.offset = input->offset;
                module->toc.elements.len = sectionLen;
            }
            break;

            case 10:
            {
                module->toc.code.offset = input->offset;
                module->toc.code.len = sectionLen;
            }
            break;

            case 11:
            {
                module->toc.data.offset = input->offset;
                module->toc.data.len = sectionLen;
            }
            break;

            case 12:
            {
                module->toc.dataCount.offset = input->offset;
                module->toc.dataCount.len = sectionLen;
            }
            break;

            default:
            {
                oc_log_error("Unknown section identifier %i.\n", sectionID);
                exit(-1);
            }
        }
        wa_input_seek(input, contentOffset + sectionLen);
    }
}

void wa_parse_types(oc_arena* arena, wa_input* input, wa_module* module)
{
    //NOTE: parse types section
    wa_input_seek(input, module->toc.types.offset);
    {
        u64 startOffset = input->offset;

        module->typeCount = wa_read_leb128_u32(input);
        module->types = oc_arena_push_array(arena, wa_func_type, module->typeCount);

        for(u32 typeIndex = 0; typeIndex < module->typeCount; typeIndex++)
        {
            wa_func_type* type = &module->types[typeIndex];

            u8 b = wa_read_byte(input);

            if(b != 0x60)
            {
                oc_log_error("Unexpected prefix 0x%02x for function type.\n", b);
                exit(-1);
            }

            type->paramCount = wa_read_leb128_u32(input);
            type->params = oc_arena_push_array(arena, wa_value_type, type->paramCount);
            for(u32 typeIndex = 0; typeIndex < type->paramCount; typeIndex++)
            {
                type->params[typeIndex] = wa_read_leb128_u32(input);
            }

            type->returnCount = wa_read_leb128_u32(input);
            type->returns = oc_arena_push_array(arena, wa_value_type, type->returnCount);
            for(u32 typeIndex = 0; typeIndex < type->returnCount; typeIndex++)
            {
                type->returns[typeIndex] = wa_read_leb128_u32(input);
            }
        }

        //NOTE: check section size
        if(input->offset - startOffset != module->toc.types.len)
        {
            oc_log_error("Size of type section does not match declared size (declared = %llu, actual = %llu)\n",
                         module->toc.types.len,
                         input->offset - startOffset);
            exit(-1);
        }
    }
}

void wa_parse_imports(oc_arena* arena, wa_input* input, wa_module* module)
{
    //NOTE: parse import section
    wa_input_seek(input, module->toc.imports.offset);
    {
        module->importCount = wa_read_leb128_u32(input);
        module->imports = oc_arena_push_array(arena, wa_import, module->importCount);

        for(u32 importIndex = 0; importIndex < module->importCount; importIndex++)
        {
            wa_import* import = &module->imports[importIndex];

            import->moduleName = wa_read_name(input, arena);
            import->importName = wa_read_name(input, arena);
            import->kind = wa_read_byte(input);

            switch((u32)import->kind)
            {
                case WA_IMPORT_FUNCTION:
                {
                    import->index = wa_read_leb128_u32(input);
                    if(import->index >= module->functionCount)
                    {
                        oc_log_error("Invalid type index in function import (function count: %u, got index %u)\n",
                                     module->functionCount,
                                     import->index);
                        exit(-1);
                    }
                }
                break;
                case WA_IMPORT_TABLE:
                {
                    import->type = wa_read_byte(input);
                    if(import->type != WA_TYPE_FUNC_REF && import->type != WA_TYPE_EXTERN_REF)
                    {
                        oc_log_error("Invalid type 0x%02x in table import \n",
                                     import->type);
                        exit(-1);
                    }
                    u8 b = wa_read_byte(input);
                    if(b != WA_LIMIT_MIN && b != WA_LIMIT_MIN_MAX)
                    {
                        oc_log_error("Invalid limit kind 0x%02x in table import\n",
                                     b);
                        exit(-1);
                    }
                    import->limits.kind = b;

                    import->limits.min = wa_read_leb128_u32(input);
                    if(import->limits.kind == WA_LIMIT_MIN_MAX)
                    {
                        import->limits.max = wa_read_leb128_u32(input);
                    }
                }
                break;
                case WA_IMPORT_MEMORY:
                {
                    u8 b = wa_read_byte(input);
                    if(b != WA_LIMIT_MIN && b != WA_LIMIT_MIN_MAX)
                    {
                        oc_log_error("Invalid limit kind 0x%02x in table import\n",
                                     b);
                        exit(-1);
                    }
                    import->limits.kind = b;

                    import->limits.min = wa_read_leb128_u32(input);
                    if(import->limits.kind == WA_LIMIT_MIN_MAX)
                    {
                        import->limits.max = wa_read_leb128_u32(input);
                    }
                }
                break;
                case WA_IMPORT_GLOBAL:
                {
                    //TODO
                }
                break;
                default:
                {
                    oc_log_error("Unknown import kind 0x%02x\n", (u8)import->kind);
                    exit(-1);
                }
            }
        }
    }
}

void wa_parse_functions(oc_arena* arena, wa_input* input, wa_module* module)
{
    //NOTE: parse function section
    wa_input_seek(input, module->toc.functions.offset);

    module->functionCount = wa_read_leb128_u32(input);
    module->functions = oc_arena_push_array(arena, wa_func, module->functionCount);

    for(u32 funcIndex = 0; funcIndex < module->functionCount; funcIndex++)
    {
        wa_func* func = &module->functions[funcIndex];
        u32 typeIndex = wa_read_leb128_u32(input);

        if(typeIndex >= module->typeCount)
        {
            oc_log_error("Invalid type index %i in function section\n", typeIndex);
            exit(-1);
        }
        func->type = &module->types[typeIndex];
    }
}

void wa_parse_exports(oc_arena* arena, wa_input* input, wa_module* module)
{
    //NOTE: parse export section
    wa_input_seek(input, module->toc.exports.offset);

    module->exportCount = wa_read_leb128_u32(input);
    module->exports = oc_arena_push_array(arena, wa_export, module->exportCount);

    for(u32 exportIndex = 0; exportIndex < module->exportCount; exportIndex++)
    {
        wa_export* export = &module->exports[exportIndex];

        export->name = wa_read_name(input, arena);
        export->kind = wa_read_byte(input);
        export->index = wa_read_leb128_u32(input);

        switch((u32) export->kind)
        {
            case WA_EXPORT_FUNCTION:
            {
                if(export->index >= module->functionCount)
                {
                    oc_log_error("Invalid type index in function export (function count: %u, got index %u)\n",
                                 module->functionCount,
                                 export->index);
                    exit(-1);
                }
            }
            break;
            case WA_EXPORT_TABLE:
            {
                //TODO
            }
            break;
            case WA_EXPORT_MEMORY:
            {
                //TODO
            }
            break;
            case WA_EXPORT_GLOBAL:
            {
                //TODO
            }
            break;
            default:
            {
                oc_log_error("Unknown export kind 0x%02x\n", (u8) export->kind);
                exit(-1);
            }
        }
    }
}
