#include <stdio.h>

#define OC_NO_APP_LAYER 1
#include "orca.h"

#include "wa_types.h"
#include "wasm_tables.c"

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

enum
{
    WA_TYPE_STACK_MAX_LEN = 512,
};

typedef union wa_code
{
    wa_instr_op opcode;
    wa_instr_immediate operand;
} wa_code;

typedef struct wa_parse_context
{
    oc_arena* arena;
    wa_module* module;

    u64 typeStackLen;
    wa_value_type typeStack[WA_TYPE_STACK_MAX_LEN];

    u64 codeCap;
    u64 codeLen;
    wa_code* code;

} wa_parse_context;

void wa_emit(wa_parse_context* context, wa_code code)
{
    //TODO emit code
}

int main(int argc, char** argv)
{
    oc_arena arena = { 0 };
    oc_arena_init(&arena);

    wa_input input = {};
    {
        oc_file file = oc_file_open(OC_STR8("test.wasm"), OC_FILE_ACCESS_READ, OC_FILE_OPEN_NONE);

        input.len = oc_file_size(file);
        input.contents = oc_arena_push(&arena, input.len);

        oc_file_read(file, input.len, input.contents);
        oc_file_close(file);
    }

    wa_module module = {
        .len = input.len,
        .bytes = input.contents,
    };

    u32 magic = wa_read_raw_u32(&input);
    u32 version = wa_read_raw_u32(&input);

    if(wa_read_status(&input))
    {
        oc_log_error("Couldn't read wasm magic number and version.\n");
        exit(-1);
    }
    //TODO check magic / version

    while(!wa_input_end(&input))
    {
        u8 sectionID = wa_read_byte(&input);
        u32 sectionLen = wa_read_leb128_u32(&input);

        u64 contentOffset = input.offset;
        //TODO: check read errors

        if(wa_read_status(&input))
        {
            oc_log_error("Input error.\n");
            exit(-1);
        }

        oc_log_info("section identifier %i at index 0x%08llx of size %u\n", sectionID, contentOffset, sectionLen);

        switch(sectionID)
        {
            case 0:
                break;

                //TODO: check if section was already defined...

            case 1:
            {
                module.toc.types.offset = input.offset;
                module.toc.types.len = sectionLen;
            }
            break;

            case 2:
            {
                module.toc.imports.offset = input.offset;
                module.toc.imports.len = sectionLen;
            }
            break;

            case 3:
            {
                module.toc.functions.offset = input.offset;
                module.toc.functions.len = sectionLen;
            }
            break;

            case 4:
            {
                module.toc.tables.offset = input.offset;
                module.toc.tables.len = sectionLen;
            }
            break;

            case 5:
            {
                module.toc.memory.offset = input.offset;
                module.toc.memory.len = sectionLen;
            }
            break;

            case 6:
            {
                module.toc.globals.offset = input.offset;
                module.toc.globals.len = sectionLen;
            }
            break;

            case 7:
            {
                module.toc.exports.offset = input.offset;
                module.toc.exports.len = sectionLen;
            }
            break;

            case 8:
            {
                module.toc.start.offset = input.offset;
                module.toc.start.len = sectionLen;
            }
            break;

            case 9:
            {
                module.toc.elements.offset = input.offset;
                module.toc.elements.len = sectionLen;
            }
            break;

            case 10:
            {
                module.toc.code.offset = input.offset;
                module.toc.code.len = sectionLen;
            }
            break;

            case 11:
            {
                module.toc.data.offset = input.offset;
                module.toc.data.len = sectionLen;
            }
            break;

            case 12:
            {
                module.toc.dataCount.offset = input.offset;
                module.toc.dataCount.len = sectionLen;
            }
            break;

            default:
            {
                oc_log_error("Unknown section identifier %i.\n", sectionID);
                exit(-1);
            }
        }
        wa_input_seek(&input, contentOffset + sectionLen);
    }

    oc_arena_scope scratch = oc_scratch_begin();

    //NOTE: parse code section
    wa_input_seek(&input, module.toc.code.offset);

    module.code.entryCount = wa_read_leb128_u32(&input);
    module.code.entries = oc_arena_push_array(&arena, wa_module_code_entry, module.code.entryCount);

    for(u32 entryIndex = 0; entryIndex < module.code.entryCount; entryIndex++)
    {
        wa_module_code_entry* entry = &module.code.entries[entryIndex];

        u32 len = wa_read_leb128_u32(&input);
        u32 startOffset = input.offset;

        // read locals
        u32 localEntryCount = wa_read_leb128_u32(&input);
        u32* counts = oc_arena_push_array(scratch.arena, u32, localEntryCount);
        wa_value_type* types = oc_arena_push_array(scratch.arena, wa_value_type, localEntryCount);

        entry->localCount = 0;
        for(u32 localEntryIndex = 0; localEntryIndex < localEntryCount; localEntryIndex++)
        {
            counts[localEntryIndex] = wa_read_leb128_u32(&input);
            types[localEntryIndex] = wa_read_byte(&input);
            entry->localCount += counts[localEntryIndex];

            //TODO: validate types
        }

        entry->locals = oc_arena_push_array(&arena, wa_value_type, entry->localCount);
        u32 localIndex = 0;

        for(u32 localEntryIndex = 0; localEntryIndex < localEntryCount; localEntryIndex++)
        {
            u32 count = counts[localEntryIndex];
            wa_value_type type = types[localEntryIndex];

            for(int i = 0; i < count; i++)
            {
                entry->locals[localIndex + i] = type;
            }
            localIndex += count;
        }

        // read body
        u64 instrCap = 4;
        u64 instrCount = 0;
        wa_module_instruction* instructions = oc_arena_push_array(scratch.arena, wa_module_instruction, instrCap);

        wa_parse_context context = {
            .arena = &arena,
            .module = &module,
            .codeCap = 4,
            .code = oc_arena_push_array(scratch.arena, wa_code, 4),
        };

        while(!wa_input_end(&input))
        {
            if(instrCount >= instrCap)
            {
                wa_module_instruction* tmp = instructions;
                instrCap = instrCap * 1.5;
                instructions = oc_arena_push_array(scratch.arena, wa_module_instruction, instrCap);
                memcpy(instructions, tmp, instrCount * sizeof(wa_module_instruction));
            }
            wa_module_instruction* instr = &instructions[instrCount];

            u8 byte = wa_read_byte(&input);

            if(byte == WA_INSTR_PREFIX_EXTENDED)
            {
                u32 code = wa_read_leb128_u32(&input);
                if(code >= wa_instr_decode_extended_len)
                {
                    oc_log_error("Invalid extended instruction %i\n", code);
                    exit(-1);
                }
                instr->op = wa_instr_decode_extended[code];
            }
            else if(byte == WA_INSTR_PREFIX_VECTOR)
            {
                u32 code = wa_read_leb128_u32(&input);
                if(code >= wa_instr_decode_vector_len)
                {
                    oc_log_error("Invalid vector instruction %i\n", code);
                    exit(-1);
                }
                instr->op = wa_instr_decode_vector[code];
            }
            else
            {
                if(byte >= wa_instr_decode_basic_len)
                {
                    oc_log_error("Invalid basic instruction 0x%02x\n", byte);
                    exit(-1);
                }
                instr->op = wa_instr_decode_basic[byte];
            }

            printf("%s\n", wa_instr_strings[instr->op]);

            const wa_instr_info* info = &wa_instr_infos[instr->op];

            // parse immediate

            for(int immIndex = 0; immIndex < info->immCount; immIndex++)
            {
                switch(info->imm[immIndex])
                {
                    case WA_IMM_ZERO:
                    {
                        wa_read_byte(&input);
                    }
                    break;
                    case WA_IMM_I32:
                    {
                        instr->imm[immIndex].immI32 = wa_read_leb128_u32(&input);
                    }
                    break;
                    case WA_IMM_I64:
                    {
                        instr->imm[immIndex].immI64 = wa_read_leb128_u64(&input);
                    }
                    break;
                    case WA_IMM_F32:
                    {
                        instr->imm[immIndex].immF32 = wa_read_f32(&input);
                    }
                    break;
                    case WA_IMM_F64:
                    {
                        instr->imm[immIndex].immF64 = wa_read_f64(&input);
                    }
                    break;
                    case WA_IMM_VALUE_TYPE:
                    {
                        instr->imm[immIndex].valueType = wa_read_byte(&input);
                    }
                    break;
                    case WA_IMM_REF_TYPE:
                    {
                        instr->imm[immIndex].valueType = wa_read_byte(&input);
                    }
                    break;
                    case WA_IMM_LOCAL_INDEX:
                    {
                        instr->imm[immIndex].index = wa_read_leb128_u32(&input);
                    }
                    break;
                    case WA_IMM_GLOBAL_INDEX:
                    {
                        instr->imm[immIndex].index = wa_read_leb128_u32(&input);
                    }
                    break;
                    case WA_IMM_FUNC_INDEX:
                    {
                        instr->imm[immIndex].index = wa_read_leb128_u32(&input);
                    }
                    break;
                    case WA_IMM_TYPE_INDEX:
                    {
                        instr->imm[immIndex].index = wa_read_leb128_u32(&input);
                    }
                    break;
                    case WA_IMM_TABLE_INDEX:
                    {
                        instr->imm[immIndex].index = wa_read_leb128_u32(&input);
                    }
                    break;
                    case WA_IMM_ELEM_INDEX:
                    {
                        instr->imm[immIndex].index = wa_read_leb128_u32(&input);
                    }
                    break;
                    case WA_IMM_DATA_INDEX:
                    {
                        instr->imm[immIndex].index = wa_read_leb128_u32(&input);
                    }
                    break;
                    case WA_IMM_MEM_ARG:
                    {
                        instr->imm[immIndex].memArg.align = wa_read_leb128_u32(&input);
                        instr->imm[immIndex].memArg.offset = wa_read_leb128_u32(&input);
                    }
                    break;
                    case WA_IMM_LANE_INDEX:
                    {
                        instr->imm[immIndex].laneIndex = wa_read_byte(&input);
                    }
                    break;
                    /*
                    case WA_IMM_V128:
                    {
                        //TODO
                    }
                    break;
                    */
                    default:
                        OC_ASSERT(0, "unsupported immediate type");
                        break;
                }
            }
            //TODO validate instruction

            for(int opdIndex = 0; opdIndex < info->inCount; opdIndex++)
            {
                if(context.typeStackLen == 0)
                {
                    oc_log_error("unbalanced stack\n");
                    exit(-1);
                }
                context.typeStackLen--;
                wa_value_type type = context.typeStack[context.typeStackLen];

                if(type != info->in[opdIndex])
                {
                    oc_log_error("operand type mismatch\n");
                    exit(-1);
                }
            }

            for(int opdIndex = 0; opdIndex < info->outCount; opdIndex++)
            {
                if(context.typeStackLen >= WA_TYPE_STACK_MAX_LEN)
                {
                    oc_log_error("type stack overflow\n");
                    exit(-1);
                }
                context.typeStack[context.typeStackLen] = info->out[opdIndex];
                context.typeStackLen++;
            }

            //NOTE emit opcode
            wa_emit(&context, (wa_code){ .opcode = instr->op });

            //emit immediates
            for(int immIndex = 0; immIndex < info->immCount; immIndex++)
            {
                wa_emit(&context, (wa_code){ .operand = instr->imm[immIndex] });
            }

            //TODO emit operands

            instrCount++;
            if(instr->op == WA_INSTR_end)
            {
                break;
            }
        }

        // check entry length
        if(input.offset - startOffset != len)
        {
            oc_log_error("Size of code entry %i does not match declared size (declared %u, got %u)\n",
                         entryIndex,
                         len,
                         input.offset - startOffset);
            exit(-1);
        }

        entry->instrCount = instrCount;
        entry->instructions = oc_arena_push_array(&arena, wa_module_instruction, entry->instrCount);
        memcpy(entry->instructions, instructions, entry->instrCount * sizeof(wa_module_instruction));

        //TODO: validate / compile entry
    }

    oc_scratch_end(scratch);

    return (0);
}
