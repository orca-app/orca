/*************************************************************************
*
*  Orca
*  Copyright 2023 Martin Fouilleul and the Orca project contributors
*  See LICENSE.txt for licensing information
*
**************************************************************************/
#include <stdio.h>

#include "util/typedefs.h"
#include "util/memory.h"
#include "util/strings.h"

#include "dwarf.h"
#include "reader.h"

//------------------------------------------------------------------------
// op info array
//------------------------------------------------------------------------

#define X_OP_INFO(name, val, opdCount, ...) \
    [name] = { .count = opdCount, .opd = { __VA_ARGS__ } },

const dw_op_info DW_OP_INFO[] = {
    DW_OP_LIST(X_OP_INFO)
};

#undef X_OP_INFO

//------------------------------------------------------------------------
// structs
//------------------------------------------------------------------------

typedef struct dw_parser dw_parser;
typedef void (*dw_error_callback)(dw_parser* parser, u64 loc, oc_str8 message, void* user);

typedef struct dw_parser
{
    oc_arena* arena;
    wa_reader rootReader;
    dw_sections sections;
    dw_error_callback errorCallback;
    void* userData;
} dw_parser;

typedef struct dw_line_machine
{
    u64 address;
    u64 opIndex; // always 0 for non vliw
    u64 file;
    u64 line;
    u64 column;
    bool isStmt;
    bool basicBlock;
    bool endSequence;
    bool prologueEnd;
    bool epilogueBegin;
    u64 isa; // could omit ?
    u64 discriminator;

    /*NOTE(martin):
        linkers may remove entities from a linked object, and instead of fixing up dwarf info, they
        can emit "tombstone addresses" (= the max address for the target address size).
        A DW_LNE_set_address opcode with a tombstone address invalidates all entries up to the next
        DW_LNE_set_address or DW_LNE_end_sequence opcode.
        This is actually a Dwarf v6 proposal (see https://dwarfstd.org/issues/200609.1.html), and
        it isn't mentionned anywhere in the Dwarf version <= 5 specs, but lld uses this anyway.

        Seems like the "spec" is more what you'd call guidelines than actual rules.
    */
    bool tombstone;

} dw_line_machine;

typedef struct dw_file_entry_elt
{
    oc_list_elt listElt;
    dw_file_entry entry;
} dw_file_entry_elt;

//------------------------------------------------------------------------
// errors
//------------------------------------------------------------------------

void dw_parse_error_str8(dw_parser* parser, u64 loc, oc_str8 message)
{
    if(parser->errorCallback)
    {
        parser->errorCallback(parser, loc, message, parser->userData);
    }
    else
    {
        oc_log_error("Dwarf parse error: %.*s\n", oc_str8_ip(message));
    }
}

void dw_parse_error(dw_parser* parser, u64 loc, const char* fmt, ...)
{
    oc_arena_scope scratch = oc_scratch_begin_next(parser->arena);
    va_list ap;
    va_start(ap, fmt);
    oc_str8 message = oc_str8_pushfv(scratch.arena, fmt, ap);
    va_end(ap);

    dw_parse_error_str8(parser, loc, message);

    oc_scratch_end(scratch);
}

void dw_parser_read_error_callback(wa_reader* reader, oc_str8 message, void* user)
{
    dw_parser* parser = (dw_parser*)parser;
    dw_parse_error_str8(parser, wa_reader_absolute_loc(reader), message);
}

//------------------------------------------------------------------------
// read helpers
//------------------------------------------------------------------------

void wa_read_inital_length(wa_reader* reader, u64* length, u8* format)
{
    *format = DW_DWARF32;
    *length = wa_read_u32(reader);

    if(*length >= 0xfffffff0)
    {
        *format = DW_DWARF64;
        *length = wa_read_u64(reader);
    }
}

//------------------------------------------------------------------------
// Line info
//------------------------------------------------------------------------

void wa_read_file_entries(dw_parser* parser,
                          wa_reader* reader,
                          u64* entryCount,
                          dw_file_entry** entries,
                          dw_sections* sections,
                          dw_line_program_header* header,
                          bool directories)
{
    u8 formatCount = 0;
    dw_file_entry_format_elt* format = 0;

    if(header->version == 5)
    {
        formatCount = wa_read_u8(reader);
    }
    else
    {
        if(directories)
        {
            formatCount = 1;
        }
        else
        {
            formatCount = 4;
        }
    }

    oc_arena_scope scratch = oc_scratch_begin_next(parser->arena);
    format = oc_arena_push_array(scratch.arena, dw_file_entry_format_elt, formatCount);

    if(header->version == 5)
    {
        for(int i = 0; i < formatCount; i++)
        {
            format[i].content = wa_read_leb128_u64(reader);
            format[i].form = wa_read_leb128_u64(reader);

            if(format[i].content <= DW_LNCT_MD5)
            {
                if(directories)
                {
                    header->dirFlags |= (1 << format[i].content);
                }
                else
                {
                    header->fileFlags |= (1 << format[i].content);
                }
            }
        }
        *entryCount = wa_read_leb128_u64(reader);
    }
    else
    {
        if(directories)
        {
            header->dirFlags = DW_LNCT_has_path;

            format[0].content = DW_LNCT_path;
            format[0].form = DW_FORM_string;
        }
        else
        {
            header->dirFlags = DW_LNCT_has_path
                             | DW_LNCT_has_directory_index
                             | DW_LNCT_has_timestamp
                             | DW_LNCT_has_size;

            format[0].content = DW_LNCT_path;
            format[0].form = DW_FORM_string;
            format[1].content = DW_LNCT_directory_index;
            format[1].form = DW_FORM_udata;

            format[2].content = DW_LNCT_timestamp;
            format[2].form = DW_FORM_udata;
            format[3].content = DW_LNCT_size;
            format[3].form = DW_FORM_udata;
        }
    }

    oc_list entryList = { 0 };
    u64 entryIndex = 0;

    while(wa_reader_has_more(reader))
    {
        if(header->version == 5 && entryIndex >= *entryCount)
        {
            break;
        }
        else if(header->version == 4 && wa_reader_peek_u8(reader) == 0)
        {
            wa_reader_skip(reader, 1);
            break;
        }

        dw_file_entry_elt* elt = oc_arena_push_type(scratch.arena, dw_file_entry_elt);
        oc_list_push_back(&entryList, &elt->listElt);

        dw_file_entry* entry = &elt->entry;

        for(int fmtIndex = 0; fmtIndex < formatCount; fmtIndex++)
        {
            u64 content = format[fmtIndex].content;
            u64 form = format[fmtIndex].form;

            switch(content)
            {
                case DW_LNCT_path:
                {
                    switch(form)
                    {
                        case DW_FORM_string:
                        {
                            entry->path = wa_read_cstring(reader);
                        }
                        break;
                        case DW_FORM_line_strp:
                        case DW_FORM_strp:
                        case DW_FORM_strp_sup:
                        {
                            u64 strp = 0;
                            if(header->addressSize == 4)
                            {
                                strp = wa_read_u32(reader);
                            }
                            else
                            {
                                strp = wa_read_u64(reader);
                            }
                            dw_section section = { 0 };
                            if(form == DW_FORM_line_strp)
                            {
                                section = sections->lineStr;
                            }
                            else if(form == DW_FORM_strp)
                            {
                                section = sections->str;
                            }
                            else
                            {
                                //TODO: supplementary string section
                                printf("error: unsupported supplementary string section\n");
                                exit(-1);
                            }

                            wa_reader strReader = wa_reader_subreader(&parser->rootReader, section.offset, section.len);
                            wa_reader_seek(&strReader, strp);
                            entry->path = wa_read_cstring(&strReader);
                        }
                        break;

                        default:
                            printf("unsupported form code for %s file entry format\n",
                                   dw_get_line_header_entry_format_string(content));
                            exit(-1);
                            break;
                    }
                }
                break;
                case DW_LNCT_directory_index:
                {
                    switch(form)
                    {
                        case DW_FORM_data1:
                        {
                            entry->dirIndex = wa_read_u8(reader);
                        }
                        break;
                        case DW_FORM_data2:
                        {
                            entry->dirIndex = wa_read_u16(reader);
                        }
                        break;
                        case DW_FORM_udata:
                        {
                            entry->dirIndex = wa_read_leb128_u64(reader);
                        }
                        break;

                        default:
                            printf("unsupported form code for %s file entry format\n",
                                   dw_get_line_header_entry_format_string(content));
                            exit(-1);
                            break;
                    }
                }
                break;
                case DW_LNCT_timestamp:
                {
                    switch(form)
                    {
                        case DW_FORM_data4:
                        {
                            entry->timestamp = wa_read_u32(reader);
                        }
                        break;
                        case DW_FORM_data8:
                        {
                            entry->timestamp = wa_read_u64(reader);
                        }
                        break;
                        case DW_FORM_udata:
                        {
                            entry->timestamp = wa_read_leb128_u64(reader);
                        }
                        break;
                        case DW_FORM_block:
                        {
                            //NOTE(martin): I don't know how to interpret the block, so just warn and skip it
                            u64 len = wa_read_leb128_u64(reader);
                            oc_str8 str = wa_read_bytes(reader, len);

                            printf("warning: unsupported form DW_FORM_block in %s file entry format\n",
                                   dw_get_line_header_entry_format_string(content));
                        }
                        break;

                        default:
                            printf("unsupported form code for %s file entry format\n",
                                   dw_get_line_header_entry_format_string(content));
                            exit(-1);
                            break;
                    }
                }
                break;
                case DW_LNCT_size:
                {
                    switch(form)
                    {
                        case DW_FORM_data1:
                        {
                            entry->size = wa_read_u8(reader);
                        }
                        break;
                        case DW_FORM_data2:
                        {
                            entry->size = wa_read_u16(reader);
                        }
                        break;
                        case DW_FORM_data4:
                        {
                            entry->size = wa_read_u32(reader);
                        }
                        break;
                        case DW_FORM_data8:
                        {
                            entry->size = wa_read_u64(reader);
                        }
                        break;
                        case DW_FORM_udata:
                        {
                            entry->size = wa_read_leb128_u64(reader);
                        }
                        break;

                        default:
                            printf("unsupported form code for %s file entry format\n",
                                   dw_get_line_header_entry_format_string(content));
                            exit(-1);
                            break;
                    }
                }
                break;
                case DW_LNCT_MD5:
                {
                    if(form != DW_FORM_data16)
                    {
                        printf("unsupported form code for %s file entry format\n",
                               dw_get_line_header_entry_format_string(content));
                        exit(-1);
                    }
                    oc_str8 md5 = wa_read_bytes(reader, 16);
                    if(md5.len == 16)
                    {
                        memcpy(entry->md5, md5.ptr, 16);
                    }
                }
                break;
                case DW_LNCT_lo_user:
                {
                }
                break;
                case DW_LNCT_hi_user:
                {
                }
                break;

                default:
                {
                    if(content >= DW_LNCT_lo_user && content <= DW_LNCT_hi_user)
                    {
                        printf("error: unsupported vendor-defined content description\n");
                        //TODO: just skip
                        exit(-1);
                    }
                    else
                    {
                        printf("error: unrecognized directory entry content description\n");
                        exit(-1);
                    }
                }
                break;
            }
        }

        OC_DEBUG_ASSERT(entry->path.ptr);

        entryIndex++;
    }
    OC_DEBUG_ASSERT(header->version == 4 || *entryCount == entryIndex);

    *entryCount = entryIndex;

    *entries = oc_arena_push_array(parser->arena, dw_file_entry, *entryCount);
    {
        oc_list_for_indexed(entryList, it, dw_file_entry_elt, listElt)
        {
            (*entries)[it.index] = it.elt->entry;
        }
    }

    oc_scratch_end(scratch);
}

int wa_read_line_program_header(dw_parser* parser, wa_reader* reader, dw_line_program_header* header, dw_sections* sections)
{
    header->offset = wa_reader_offset(reader);

    u8 dwarfFormat = 0;
    wa_read_inital_length(reader, &header->unitLength, &dwarfFormat);

    header->version = wa_read_u16(reader);

    if(header->version != 5 && header->version != 4)
    {
        printf("error: DWARF version %i not supported\n", header->version);
        exit(-1);
    }

    if(header->version == 5)
    {
        header->addressSize = wa_read_u8(reader);
        if(header->addressSize != 4 && header->addressSize != 8)
        {
            oc_log_error("address size should be 4 or 8\n");
            exit(-1);
        }

        header->segmentSelectorSize = wa_read_u8(reader);
    }
    else
    {
        //NOTE: we set 4 by default as it is the address size on wasm.
        //TODO: THIS SHOULD CHANGE IF WE SWITCH TO WASM64!
        //TODO: allow configuring the "default target address size" from outside
        header->addressSize = 4;
    }

    if(dwarfFormat == DW_DWARF32)
    {
        header->headerLength = wa_read_u32(reader);
    }
    else
    {
        header->headerLength = wa_read_u64(reader);
    }
    u64 headerLengthBase = wa_reader_offset(reader);

    header->minInstructionLength = wa_read_u8(reader);
    header->maxOperationsPerInstruction = wa_read_u8(reader);
    header->defaultIsStmt = wa_read_u8(reader);
    header->lineBase = wa_read_u8(reader);
    header->lineRange = wa_read_u8(reader);
    header->opcodeBase = wa_read_u8(reader);

    //TODO: support non-12 sizes
    for(int i = 0; i < 12; i++)
    {
        header->standardOpcodeLength[i] = wa_read_u8(reader);
    }

    // directories
    wa_read_file_entries(parser, reader, &header->dirEntryCount, &header->dirEntries, sections, header, true);

    // files
    wa_read_file_entries(parser, reader, &header->fileEntryCount, &header->fileEntries, sections, header, false);

    //NOTE: return offset from start to beginning of line program code
    return (headerLengthBase + header->headerLength - header->offset);
}

void dw_line_machine_reset(dw_line_machine* m, bool defaultIsStmt)
{
    memset(m, 0, sizeof(dw_line_machine));
    m->file = 1;
    m->line = 1;
    m->isStmt = defaultIsStmt;
}

typedef struct dw_line_entry_elt
{
    oc_list_elt listElt;
    dw_line_entry entry;

} dw_line_entry_elt;

typedef struct dw_line_table_elt
{
    oc_list_elt listElt;
    dw_line_table table;

} dw_line_table_elt;

void dw_line_machine_emit_row(oc_arena* arena, dw_line_machine* m, oc_list* rowList, u32* rowCount)
{
    if(!m->tombstone)
    {
        dw_line_entry_elt* elt = oc_arena_push_type(arena, dw_line_entry_elt);

        elt->entry.address = m->address;
        elt->entry.line = m->line;
        elt->entry.column = m->column;
        elt->entry.file = m->file;
        elt->entry.isa = m->isa;
        elt->entry.opIndex = m->opIndex;

        if(m->isStmt)
        {
            elt->entry.flags |= DW_LINE_STMT;
        }
        if(m->basicBlock)
        {
            elt->entry.flags |= DW_LINE_BASIC_BLOCK;
        }
        if(m->endSequence)
        {
            elt->entry.flags |= DW_LINE_SEQUENCE_END;
        }
        if(m->prologueEnd)
        {
            elt->entry.flags |= DW_LINE_PROLOGUE_END;
        }
        if(m->epilogueBegin)
        {
            elt->entry.flags |= DW_LINE_EPILOGUE_BEGIN;
        }

        oc_list_push_back(rowList, &elt->listElt);

        (*rowCount)++;
    }
}

dw_line_info dw_load_line_info(dw_parser* parser, dw_sections* sections)
{
    dw_line_info lineInfo = { 0 };

    wa_reader reader = wa_reader_subreader(&parser->rootReader, sections->line.offset, sections->line.len);

    oc_arena_scope scratch = oc_scratch_begin_next(parser->arena);
    oc_list tablesList = { 0 };
    u64 tableCount = 0;

    while(wa_reader_has_more(&reader))
    {
        u64 unitStart = wa_reader_offset(&reader);
        dw_line_program_header header = { 0 };

        wa_read_line_program_header(parser, &reader, &header, sections);

        u64 unitLineInfoEnd = unitStart + header.addressSize + header.unitLength;

        if(unitLineInfoEnd > reader.contents.len)
        {
            oc_log_error("inconsistent size information in line program header\n");
            exit(-1);
        }

        dw_line_table_elt* table = oc_arena_push_type(scratch.arena, dw_line_table_elt);
        oc_list_push_back(&tablesList, &table->listElt);

        table->table.header = header;

        oc_list rowsList = { 0 };
        u32 rowCount = 0;

        dw_line_machine machine;
        dw_line_machine_reset(&machine, header.defaultIsStmt);

        //TODO: use subreader
        while(wa_reader_offset(&reader) < unitLineInfoEnd)
        {
            u8 opcode = wa_read_u8(&reader);

            if(opcode >= header.opcodeBase)
            {
                // special opcode
                opcode -= header.opcodeBase;
                u64 opAdvance = opcode / header.lineRange;

                machine.line += header.lineBase + (opcode % header.lineRange);
                machine.address += header.minInstructionLength * ((machine.opIndex + opAdvance) / header.maxOperationsPerInstruction);
                machine.opIndex = (machine.opIndex + opAdvance) % header.maxOperationsPerInstruction;

                dw_line_machine_emit_row(scratch.arena, &machine, &rowsList, &rowCount);

                machine.basicBlock = false;
                machine.prologueEnd = false;
                machine.epilogueBegin = false;
                machine.discriminator = 0;
            }
            else if(opcode == 0)
            {
                // extended opcode
                u32 opcodeSize = wa_read_leb128_u32(&reader);
                opcode = wa_read_u8(&reader);

                switch(opcode)
                {
                    case DW_LNE_end_sequence:
                    {
                        machine.endSequence = true;
                        dw_line_machine_emit_row(scratch.arena, &machine, &rowsList, &rowCount);
                        dw_line_machine_reset(&machine, header.defaultIsStmt);
                    }
                    break;
                    case DW_LNE_set_address:
                    {
                        u64 tombstoneAddress = 0;
                        u64 address = 0;
                        if(header.addressSize == 4)
                        {
                            tombstoneAddress = 0xffffffff;
                            address = wa_read_u32(&reader);
                        }
                        else if(header.addressSize == 8)
                        {
                            tombstoneAddress = 0xffffffffffffffff;
                            address = wa_read_u64(&reader);
                        }
                        else
                        {
                            OC_ASSERT(0);
                        }
                        machine.tombstone = (address == tombstoneAddress);
                        machine.address = address;
                        machine.opIndex = 0;
                    }
                    break;
                    case DW_LNE_set_discriminator:
                    {
                        u64 disc = wa_read_leb128_u64(&reader);
                        machine.discriminator = disc;
                    }
                    break;

                    default:
                    {
                        if(opcode >= DW_LNE_lo_user && opcode <= DW_LNE_hi_user)
                        {
                            printf("error: unsupported user opcode\n");
                            exit(-1);
                        }
                        else
                        {
                            oc_log_error("unrecognized line program opcode\n");
                            exit(-1);
                        }
                    }
                    break;
                }
            }
            else
            {
                // standard opcode
                switch(opcode)
                {
                    case DW_LNS_copy:
                    {
                        dw_line_machine_emit_row(scratch.arena, &machine, &rowsList, &rowCount);
                        machine.discriminator = 0;
                        machine.basicBlock = false;
                        machine.prologueEnd = false;
                        machine.epilogueBegin = false;
                    }
                    break;
                    case DW_LNS_advance_pc:
                    {
                        u64 opAdvance = wa_read_leb128_u64(&reader);
                        machine.address += header.minInstructionLength * ((machine.opIndex + opAdvance) / header.maxOperationsPerInstruction);
                        machine.opIndex = (machine.opIndex + opAdvance) % header.maxOperationsPerInstruction;
                    }
                    break;
                    case DW_LNS_advance_line:
                    {
                        int64_t lineAdvance = wa_read_leb128_i64(&reader);
                        machine.line += lineAdvance;
                    }
                    break;
                    case DW_LNS_set_file:
                    {
                        u64 file = wa_read_leb128_u64(&reader);
                        machine.file = file;
                    }
                    break;
                    case DW_LNS_set_column:
                    {
                        u64 column = wa_read_leb128_u64(&reader);
                        machine.column = column;
                    }
                    break;
                    case DW_LNS_negate_stmt:
                    {
                        machine.isStmt = !machine.isStmt;
                    }
                    break;
                    case DW_LNS_set_basic_block:
                    {
                        machine.basicBlock = true;
                    }
                    break;
                    case DW_LNS_const_add_pc:
                    {
                        // advance line and opIndex by same increments as special opcode 255
                        opcode = 255 - header.opcodeBase;
                        u64 opAdvance = opcode / header.lineRange;

                        machine.address += header.minInstructionLength * ((machine.opIndex + opAdvance) / header.maxOperationsPerInstruction);
                        machine.opIndex = (machine.opIndex + opAdvance) % header.maxOperationsPerInstruction;
                    }
                    break;
                    case DW_LNS_fixed_advance_pc:
                    {
                        uint16_t inc = wa_read_u16(&reader);
                        machine.address += inc;
                        machine.opIndex = 0;
                    }
                    break;
                    case DW_LNS_set_prologue_end:
                    {
                        machine.prologueEnd = true;
                    }
                    break;
                    case DW_LNS_set_epilogue_begin:
                    {
                        machine.epilogueBegin = true;
                    }
                    break;
                    case DW_LNS_set_isa:
                    {
                        u64 isa = wa_read_leb128_u64(&reader);
                        machine.isa = isa;
                    }
                    break;

                    default:
                    {
                        oc_log_error("unrecognized line program opcode\n");
                        exit(-1);
                    }
                    break;
                }
            }
        }

        table->table.entryCount = rowCount;
        table->table.entries = oc_arena_push_array(parser->arena, dw_line_entry, rowCount);

        oc_list_for_indexed(rowsList, rowIt, dw_line_entry_elt, listElt)
        {
            table->table.entries[rowIt.index] = rowIt.elt->entry;
        }

        tableCount++;
    }

    lineInfo.tableCount = tableCount;
    lineInfo.tables = oc_arena_push_array(parser->arena, dw_line_table, tableCount);

    oc_list_for_indexed(tablesList, tableIt, dw_line_table_elt, listElt)
    {
        lineInfo.tables[tableIt.index] = tableIt.elt->table;
    }

    oc_scratch_end(scratch);

    return lineInfo;
}

//------------------------------------------------------------------------
// Parsing info
//------------------------------------------------------------------------

/*
typedef struct dw_loclist_table
{
    u64 unitLength;
    dw_format format;
    u16 version;
    u8 addressSize;
    u8 segmentSelectorSize;
    u32 offsetEntryCount;
    u64* offsets;

    //...

} dw_loclist_table;

dw_loclist_table* dw_parse_loclist_table(oc_arean* arena, oc_str8 section)
{
    dw_loclist_table* table = oc_arena_push_type(arena, dw_loclist_table);

    u64 offset = 0;
    u32 length32 = 0;
    u8 dwarfFormat = DW_DWARF32;

    offset += wa_read_u32(&length32, section.ptr, section.len, offset);

    if(length32 >= 0xfffffff0)
    {
        offset += wa_read_u64(&table->unitLength, section.ptr, section.len, offset);
        dwarfFormat = DW_DWARF64;
    }
    else
    {
        table->unitLength = length32;
    }

    //TODO

    return table;
}
*/

dw_abbrev_table* dw_load_abbrev_table(dw_parser* parser, dw_section section, u64 offset)
{
    //TODO: check if we already loaded this table

    dw_abbrev_table* table = oc_arena_push_type(parser->arena, dw_abbrev_table);

    //NOTE: we don't know the number of entries or the number of attributes for
    //      each entry before we parse them, so we first push parsed struct to
    //      linked list, and copy them at the end to fixed arrays.

    typedef struct dw_abbrev_entry_elt
    {
        oc_list_elt listElt;
        dw_abbrev_entry entry;

    } dw_abbrev_entry_elt;

    typedef struct dw_abbrev_attr_elt
    {
        oc_list_elt listElt;
        dw_abbrev_attr attr;
    } dw_abbrev_attr_elt;

    oc_list entries = { 0 };
    oc_arena_scope scratch = oc_scratch_begin_next(parser->arena);

    wa_reader reader = wa_reader_subreader(&parser->rootReader, section.offset, section.len);
    wa_reader_seek(&reader, offset);

    while(wa_reader_has_more(&reader))
    {
        u64 abbrevCode = wa_read_leb128_u64(&reader);
        if(abbrevCode == 0)
        {
            //NOTE: null entry, this is the end of the table
            break;
        }

        dw_abbrev_entry_elt* entryElt = oc_arena_push_type(scratch.arena, dw_abbrev_entry_elt);
        dw_abbrev_entry* entry = &entryElt->entry;

        entry->code = abbrevCode;

        entry->tag = wa_read_leb128_u32(&reader);
        entry->hasChildren = wa_read_u8(&reader);

        //NOTE: parse attributes
        oc_list attributes = { 0 };
        entry->attrCount = 0;

        while(wa_reader_has_more(&reader))
        {
            u32 attrName = wa_read_leb128_u32(&reader);
            u32 attrForm = wa_read_leb128_u32(&reader);

            if(attrName == 0 && attrForm == 0)
            {
                //NOTE: end of attributes
                break;
            }

            dw_abbrev_attr_elt* attrElt = oc_arena_push_type(scratch.arena, dw_abbrev_attr_elt);
            dw_abbrev_attr* attr = &attrElt->attr;

            attr->name = attrName;
            attr->form = attrForm;

            if(attr->form == DW_FORM_implicit_const)
            {
                attr->implicitConst = wa_read_leb128_i64(&reader);
            }

            oc_list_push_back(&attributes, &attrElt->listElt);
            entry->attrCount++;
        }

        //NOTE: copy attributes to fixed array
        entry->attributes = oc_arena_push_array(parser->arena, dw_abbrev_attr, entry->attrCount);

        oc_list_for_indexed(attributes, attrIt, dw_abbrev_attr_elt, listElt)
        {
            entry->attributes[attrIt.index] = attrIt.elt->attr;
        }

        oc_list_push_back(&entries, &entryElt->listElt);
        table->entryCount++;
    }

    //NOTE: copy entries to fixed array
    table->entries = oc_arena_push_array(parser->arena, dw_abbrev_entry, table->entryCount);
    u64 entryIndex = 0;

    oc_list_for_indexed(entries, entryIt, dw_abbrev_entry_elt, listElt)
    {
        table->entries[entryIt.index] = entryIt.elt->entry;
    }

    oc_scratch_end(scratch);

    return table;
}

dw_expr dw_parse_expr(dw_parser* parser, wa_reader* reader, dw_dwarf_format format)
{
    u64 cap = 0;
    dw_expr expr = { 0 };
    bool quit = false;

    while(!quit && wa_reader_has_more(reader))
    {
        if(expr.codeLen >= cap)
        {
            //NOTE: grow code array if needed
            cap = (u32)((f32)cap * 1.5 + 8);
            dw_expr_instr* newCode = oc_malloc_array(dw_expr_instr, cap);
            if(expr.code)
            {
                memcpy(newCode, expr.code, expr.codeLen * sizeof(dw_expr_instr));
                free(expr.code);
            }
            expr.code = newCode;
        }

        dw_expr_instr* instr = &expr.code[expr.codeLen];
        expr.codeLen++;

        instr->op = wa_read_u8(reader);

        if(instr->op > DW_OP_hi_user)
        {
            dw_parse_error(parser, wa_reader_absolute_loc(reader), "Out of bounds dwarp opcode 0x%02hhu", instr->op);
            break;
        }
        const dw_op_info* info = &DW_OP_INFO[instr->op];

        instr->operands = oc_arena_push_array(parser->arena, dw_val, info->count);

        //NOTE: special-case the opcodes whose encoding depend on the first opcode (sigh)
        if(instr->op == DW_OP_implicit_value)
        {
            instr->operands = oc_arena_push_array(parser->arena, dw_val, 2);
            u64 len = wa_read_leb128_u64(reader);
            instr->operands[0].valU64 = len;
            instr->operands[1].string = wa_read_bytes(reader, len);
        }
        else if(instr->op == DW_OP_WASM_location)
        {
            instr->operands = oc_arena_push_array(parser->arena, dw_val, 2);
            u8 kind = instr->operands[0].valU8 = wa_read_u8(reader);

            switch(kind)
            {
                case 0x00:
                case 0x01:
                case 0x02:
                {
                    //NOTE: wasm local
                    instr->operands[1].valU32 = wa_read_leb128_u32(reader);
                }
                break;

                case 0x03:
                {
                    //NOTE: wasm global, u32
                    instr->operands[1].valU32 = wa_read_u32(reader);
                }
                break;
                default:
                    dw_parse_error(parser, wa_reader_absolute_loc(reader), "unrecognized WASM location kind %hhu\n", kind);
                    quit = true;
            }
        }
        else
        {
            for(u32 opdIndex = 0; opdIndex < info->count; opdIndex++)
            {
                switch(info->opd[opdIndex])
                {
                    case DW_OPD_U8:
                        instr->operands[opdIndex].valU8 = wa_read_u8(reader);
                        break;
                    case DW_OPD_I8:
                        instr->operands[opdIndex].valI8 = wa_read_i8(reader);
                        break;
                    case DW_OPD_U16:
                        instr->operands[opdIndex].valU16 = wa_read_u16(reader);
                        break;
                    case DW_OPD_I16:
                        instr->operands[opdIndex].valI16 = wa_read_i16(reader);
                        break;
                    case DW_OPD_U32:
                        instr->operands[opdIndex].valU32 = wa_read_u32(reader);
                        break;
                    case DW_OPD_I32:
                        instr->operands[opdIndex].valI32 = wa_read_i32(reader);
                        break;
                    case DW_OPD_U64:
                        instr->operands[opdIndex].valU64 = wa_read_u64(reader);
                        break;
                    case DW_OPD_I64:
                        instr->operands[opdIndex].valI64 = wa_read_i64(reader);
                        break;
                    case DW_OPD_ULEB:
                        instr->operands[opdIndex].valU64 = wa_read_leb128_u64(reader);
                        break;
                    case DW_OPD_SLEB:
                        instr->operands[opdIndex].valI64 = wa_read_leb128_i64(reader);
                        break;
                    case DW_OPD_ADDR:
                        //TODO: always 32bit for wasm, but we should depend on dwarf info for that...
                        instr->operands[opdIndex].valU64 = wa_read_u32(reader);
                        break;
                    case DW_OPD_REF:
                        if(format == DW_DWARF32)
                        {
                            instr->operands[opdIndex].valU64 = wa_read_u32(reader);
                        }
                        else
                        {
                            instr->operands[opdIndex].valU64 = wa_read_u64(reader);
                        }
                        break;
                }
            }
        }
    }

    //NOTE: copy expr code to arena and free temp code
    dw_expr_instr* code = oc_arena_push_array(parser->arena, dw_expr_instr, expr.codeLen);
    if(expr.code)
    {
        memcpy(code, expr.code, expr.codeLen * sizeof(dw_expr_instr));
        free(expr.code);
    }
    expr.code = code;

    return expr;
}

dw_loc dw_parse_loclist(dw_parser* parser, dw_unit* unit, dw_section section, u64 offset)
{
    //TODO: parse from debug loclist.
    // in v4, offset is an offset from the beginning of the debug_loc section
    // in v5, offset in an offset from the beginning of the debug_loclists section
    dw_loc loc = { 0 };

    oc_arena_scope scratch = oc_scratch_begin_next(parser->arena);

    wa_reader reader = wa_reader_subreader(&parser->rootReader, section.offset, section.len);
    wa_reader_seek(&reader, offset);

    if(unit->version == 4)
    {
        typedef struct dw_loc_entry_elt
        {
            oc_list_elt listElt;
            dw_loc_entry entry;
        } dw_loc_entry_elt;

        oc_list list = { 0 };

        while(offset < section.len)
        {
            u64 start = 0;
            u64 end = 0;
            dw_expr expr = { 0 };

            if(unit->addressSize == 4)
            {
                start = wa_read_u32(&reader);
                end = wa_read_u32(&reader);
                if(start == 0xffffffff)
                {
                    start = 0xffffffffffffffff;
                }
            }
            else if(unit->addressSize == 8)
            {
                start = wa_read_u64(&reader);
                end = wa_read_u64(&reader);
            }
            else
            {
                OC_ASSERT(0);
            }

            if(start == 0 && end == 0)
            {
                //NOTE end of list entry
                break;
            }
            else if(start != 0xffffffffffffffff)
            {
                //NOTE normal entry
                u16 length = wa_read_u16(&reader);

                wa_reader exprReader = wa_reader_subreader(&reader, wa_reader_offset(&reader), length);
                expr = dw_parse_expr(parser, &exprReader, unit->format);

                wa_reader_skip(&reader, length);
            }

            dw_loc_entry_elt* elt = oc_arena_push_type(scratch.arena, dw_loc_entry_elt);
            elt->entry = (dw_loc_entry){
                .start = start,
                .end = end,
                .expr = expr,
            };

            oc_list_push_back(&list, &elt->listElt);
            loc.entryCount++;
        }

        loc.entries = oc_arena_push_array(parser->arena, dw_loc_entry, loc.entryCount);
        oc_list_for_indexed(list, it, dw_loc_entry_elt, listElt)
        {
            loc.entries[it.index] = it.elt->entry;
        }
    }
    else
    {
        //TODO
        OC_ASSERT(0);
    }

    oc_scratch_end(scratch);

    return loc;
}

void dw_parse_form_value(dw_parser* parser, wa_reader* reader, dw_attr* res, dw_unit* unit, dw_sections* sections, dw_attr_name name, dw_form form)
{
    switch(form)
    {
        //-----------------------
        // address class
        //-----------------------
        case DW_FORM_addr:
        {
            if(unit->addressSize == 4)
            {
                res->valU64 = wa_read_u32(reader);
            }
            else
            {
                res->valU64 = wa_read_u64(reader);
            }
        }
        break;
        case DW_FORM_addrx1:
        {
            u8 indOffset = wa_read_u8(reader);
            //TODO: get address from debug_addr section
        }
        break;
        case DW_FORM_addrx2:
        {
            u16 indOffset = wa_read_u16(reader);
            //TODO: get address from debug_addr section
        }
        break;
        case DW_FORM_addrx3:
        {
            u64 indOffset = 0;
            u8* indOffset8 = (u8*)&indOffset;
            indOffset8[0] = wa_read_u8(reader);
            indOffset8[1] = wa_read_u8(reader);
            indOffset8[2] = wa_read_u8(reader);
            //TODO: get address from debug_addr section
        }
        break;
        case DW_FORM_addrx4:
        {
            u32 indOffset = wa_read_u32(reader);
            //TODO: get address from debug_addr section
        }
        break;
        case DW_FORM_addrx:
        {
            u64 indOffset = wa_read_leb128_u64(reader);
            //TODO: get address from debug_addr section
        }
        break;

        //-----------------------
        // block class
        //-----------------------
        case DW_FORM_block1:
        {
            u8 len = wa_read_u8(reader);
            res->string = wa_read_bytes(reader, len);
        }
        break;
        case DW_FORM_block2:
        {
            u16 len = wa_read_u16(reader);
            res->string = wa_read_bytes(reader, len);
        }
        break;
        case DW_FORM_block4:
        {
            u32 len = wa_read_u32(reader);
            res->string = wa_read_bytes(reader, len);
        }
        break;
        case DW_FORM_block:
        {
            u64 len = wa_read_leb128_u64(reader);
            res->string = wa_read_bytes(reader, len);
        }
        break;

        //-----------------------
        // constant class
        //-----------------------
        case DW_FORM_data1:
        case DW_FORM_data2:
        case DW_FORM_data4:
        case DW_FORM_data8:
        case DW_FORM_data16:
        {
            switch(form)
            {
                case DW_FORM_data1:
                {
                    res->valU64 = wa_read_u8(reader);
                }
                break;
                case DW_FORM_data2:
                {
                    res->valU64 = wa_read_u16(reader);
                }
                break;
                case DW_FORM_data4:
                {
                    res->valU64 = wa_read_u32(reader);
                }
                break;
                case DW_FORM_data8:
                {
                    res->valU64 = wa_read_u64(reader);
                }
                break;

                case DW_FORM_data16:
                {
                    dw_parse_error(parser, wa_reader_absolute_loc(reader), "DW_FORM_data16 unsupported for now.");
                }
                break;

                default:
                    OC_ASSERT(0, "unreachable");
                    break;
            }
        }
        break;
        case DW_FORM_sdata:
        {
            res->valI64 = wa_read_leb128_i64(reader);
        }
        break;
        case DW_FORM_udata:
        {
            res->valU64 = wa_read_leb128_u64(reader);
        }
        break;
        case DW_FORM_implicit_const:
        {
            res->valI64 = res->abbrev->implicitConst;
        }
        break;

        //-----------------------
        // exprloc class
        //-----------------------
        case DW_FORM_exprloc:
        {
            u64 len = wa_read_leb128_u64(reader);

            wa_reader exprReader = wa_reader_subreader(reader, wa_reader_offset(reader), len);
            dw_expr expr = dw_parse_expr(parser, &exprReader, unit->format);

            wa_reader_skip(reader, len);

            res->loc = (dw_loc){
                .single = true,
                .entryCount = 1,
                .entries = oc_arena_push_type(parser->arena, dw_loc_entry),
            };
            res->loc.entries[0] = (dw_loc_entry){
                .expr = expr,
            };
        }
        break;

        //-----------------------
        // flag class
        //-----------------------
        case DW_FORM_flag:
        {
            res->valU8 = wa_read_u8(reader);
        }
        break;
        case DW_FORM_flag_present:
        {
            res->valU8 = 1;
        }
        break;

        //-----------------------
        // loclist class
        //-----------------------
        case DW_FORM_loclistx:
        {
            u64 listOffset = wa_read_leb128_u64(reader);
            //TODO: extract list of entries form debug_loclists section
        }
        break;

        //-----------------------
        // rnglist class
        //-----------------------
        case DW_FORM_rnglistx:
        {
            u64 rngListIndex = wa_read_leb128_u64(reader);
            //TODO: extract rnglist of entries form debug_rnglists section
        }
        break;

        //-----------------------
        // reference class
        //-----------------------
        //NOTE: we store refs as u64 offset relative to the start of the debug section
        case DW_FORM_ref1:
        {
            res->valU64 = unit->start + wa_read_u8(reader);
        }
        break;
        case DW_FORM_ref2:
        {
            res->valU64 = unit->start + wa_read_u16(reader);
        }
        break;
        case DW_FORM_ref4:
        {
            res->valU64 = unit->start + wa_read_u32(reader);
        }
        break;
        case DW_FORM_ref8:
        {
            res->valU64 = unit->start + wa_read_u64(reader);
        }
        break;
        case DW_FORM_ref_udata:
        {
            res->valU64 = unit->start + wa_read_leb128_u64(reader);
        }
        break;

        case DW_FORM_ref_addr:
        {
            if(unit->format == DW_DWARF32)
            {
                res->valU64 = wa_read_u32(reader);
            }
            else
            {
                res->valU64 = wa_read_u64(reader);
            }
        }
        break;

        case DW_FORM_ref_sig8:
        {
            u64 sig = wa_read_u64(reader);
            //TODO: store the signature and find corresponding def later?
        }
        break;

        case DW_FORM_ref_sup4:
        {
            u32 supOffset = wa_read_u32(reader);
            //TODO: support supplementary object files??
        }
        break;

        case DW_FORM_ref_sup8:
        {
            u64 supOffset = wa_read_u64(reader);
            //TODO: support supplementary object files??
        }
        break;

        //-----------------------
        // string class
        //-----------------------
        case DW_FORM_string:
        {
            res->string = wa_read_cstring(reader);
        }
        break;

        //TODO extract strings from string section
        case DW_FORM_strp:
        case DW_FORM_line_strp:
        case DW_FORM_strp_sup:
        {
            u64 strOffset = 0;
            if(unit->format == DW_DWARF32)
            {
                strOffset = wa_read_u32(reader);
            }
            else
            {
                strOffset = wa_read_u64(reader);
            }

            dw_section* strSection = 0;
            if(form == DW_FORM_strp)
            {
                strSection = &sections->str;
            }
            else if(form == DW_FORM_line_strp)
            {
                strSection = &sections->lineStr;
            }
            else
            {
                oc_log_warning("unsupported form DW_FORM_str_sup (string from supplementary object file)\n");
            }

            if(strSection)
            {
                wa_reader strReader = wa_reader_subreader(&parser->rootReader, strSection->offset, strSection->len);
                wa_reader_seek(&strReader, strOffset);
                res->string = wa_read_cstring(&strReader);
            }
        }
        break;

        case DW_FORM_strx1:
        case DW_FORM_strx2:
        case DW_FORM_strx3:
        case DW_FORM_strx4:
        case DW_FORM_strx:
        {
            u64 index = 0;
            switch(form)
            {
                case DW_FORM_strx1:
                {
                    index = wa_read_u8(reader);
                }
                break;
                case DW_FORM_strx2:
                {
                    index = wa_read_u16(reader);
                }
                break;
                case DW_FORM_strx3:
                {
                    u16 index16 = wa_read_u16(reader);
                    u8 index8 = wa_read_u8(reader);
                    memcpy(&index, &index16, sizeof(u16));
                    memcpy(((char*)&index) + sizeof(u16), &index8, sizeof(u8));
                }
                break;
                case DW_FORM_strx4:
                {
                    index = wa_read_u32(reader);
                }
                break;
                case DW_FORM_strx:
                {
                    index = wa_read_leb128_u64(reader);
                }
                break;

                default:
                    OC_ASSERT(0, "unreachable");
            }

            u64 strOffset = 0;

            //NOTE: compute start of offsets table
            //TODO: take into account string offset base...

            wa_reader strOffsetReader = wa_reader_subreader(&parser->rootReader, sections->strOffsets.offset, sections->strOffsets.len);

            u8 strOffsetFormat = DW_DWARF32;
            u64 strOffsetLength = 0;

            wa_read_inital_length(&strOffsetReader, &strOffsetLength, &strOffsetFormat);

            u16 strOffsetVersion = wa_read_u16(&strOffsetReader);
            u16 padding = wa_read_u16(&strOffsetReader);

            if(strOffsetFormat == DW_DWARF32)
            {
                wa_reader_skip(&strOffsetReader, index * 4);
                strOffset = wa_read_u32(&strOffsetReader);
            }
            else
            {
                wa_reader_skip(&strOffsetReader, index * 8);
                strOffset = wa_read_u64(&strOffsetReader);
            }

            wa_reader strReader = wa_reader_subreader(&parser->rootReader, sections->str.offset, sections->str.len);
            wa_reader_seek(&strReader, strOffset);
            res->string = wa_read_cstring(&strReader);
        }
        break;

        //-----------------------
        // sec_offset (belongs to multiple classes)
        //-----------------------
        //TODO use sec offset according to class
        case DW_FORM_sec_offset:
        {
            u64 addrOffset = 0;
            if(unit->format == DW_DWARF32)
            {
                addrOffset = wa_read_u32(reader);
            }
            else
            {
                addrOffset = wa_read_u64(reader);
            }

            switch(name)
            {
                case DW_AT_location:
                {
                    res->loc = dw_parse_loclist(parser, unit, sections->loc, addrOffset);
                }
                break;

                default:
                    //TODO
                    break;
            }
        }
        break;

        //-----------------------
        // indirect is a special case, see p207 of Dwarf5 spec.
        //-----------------------
        case DW_FORM_indirect:
        {
            u64 indForm = wa_read_leb128_u64(reader);
            dw_parse_form_value(parser, reader, res, unit, sections, name, indForm);
        }
        break;

        default:
            //TODO
            dw_parse_error(parser, wa_reader_absolute_loc(reader), "unsupported form %s\n", dw_get_form_string(form));
            break;
    }
}

void dw_parse_die(dw_parser* parser, wa_reader* reader, dw_die* die, dw_sections* sections, dw_unit* unit)
{
    //NOTE: find abbreviation
    die->abbrevCode = wa_read_leb128_u64(reader);
    if(die->abbrevCode == 0)
    {
        goto end;
    }

    die->abbrev = 0;
    for(u64 abbrevIndex = 0; abbrevIndex < unit->abbrev->entryCount; abbrevIndex++)
    {
        if(unit->abbrev->entries[abbrevIndex].code == die->abbrevCode)
        {
            die->abbrev = &unit->abbrev->entries[abbrevIndex];
            break;
        }
    }

    if(!die->abbrev)
    {
        dw_parse_error(parser, wa_reader_absolute_loc(reader), "Couldn't find abbrev code %llu\n", die->abbrevCode);
        return;
    }

    die->attributes = oc_arena_push_array(parser->arena, dw_attr, die->abbrev->attrCount);

    for(u32 attrIndex = 0; attrIndex < die->abbrev->attrCount; attrIndex++)
    {
        dw_attr* attr = &die->attributes[attrIndex];
        attr->abbrev = &die->abbrev->attributes[attrIndex];

        dw_parse_form_value(parser, reader, attr, unit, sections, attr->abbrev->name, attr->abbrev->form);

        //TODO: some forms need interpretation based on the attr name,
        //      review how we parse forms w/ context specific meaning
    }

end:
    return;
}

void dw_parse_info(dw_parser* parser, dw_sections* sections, dw_info* info)
{
    wa_reader reader = wa_reader_subreader(&parser->rootReader, sections->info.offset, sections->info.len);

    typedef struct dw_unit_elt
    {
        oc_list_elt listElt;
        dw_unit unit;
    } dw_unit_elt;

    oc_list units = { 0 };
    info->unitCount = 0;

    oc_arena_scope scratch = oc_scratch_begin_next(parser->arena);

    while(wa_reader_has_more(&reader))
    {
        dw_unit_elt* unitElt = oc_arena_push_type(scratch.arena, dw_unit_elt);
        dw_unit* unit = &unitElt->unit;

        unit->start = wa_reader_offset(&reader);

        wa_read_inital_length(&reader, &unit->initialLength, &unit->format);

        unit->version = wa_read_u16(&reader);

        if(unit->version >= 5)
        {
            unit->type = wa_read_u8(&reader);
        }
        else
        {
            unit->type = DW_UT_compile;
        }

        if(unit->type == DW_UT_compile || unit->type == DW_UT_partial)
        {
            u8 addressSize;
            u64 abbrevOffset = 0;

            if(unit->version >= 5)
            {
                //NOTE: debug abbrev offset and address size are ordered differently in dwarf v4 and v5, because.. why not?
                unit->addressSize = wa_read_u8(&reader);

                if(unit->format == DW_DWARF32)
                {
                    unit->abbrevOffset = wa_read_u32(&reader);
                }
                else
                {
                    unit->abbrevOffset = wa_read_u64(&reader);
                }
            }
            else
            {
                if(unit->format == DW_DWARF32)
                {
                    unit->abbrevOffset = wa_read_u32(&reader);
                }
                else
                {
                    unit->abbrevOffset = wa_read_u64(&reader);
                }
                unit->addressSize = wa_read_u8(&reader);
            }

            unit->abbrev = dw_load_abbrev_table(parser, sections->abbrev, unit->abbrevOffset);

            //NOTE(martin): read DIEs for unit
            dw_die* parentDIE = 0;
            do
            {
                dw_die* die = oc_arena_push_type(parser->arena, dw_die);
                die->start = wa_reader_offset(&reader);
                if(parentDIE)
                {
                    die->parent = parentDIE;
                    oc_list_push_back(&parentDIE->children, &die->parentElt);
                }
                else
                {
                    //NOTE: This is the root DIE
                    unit->rootDie = die;
                }

                dw_parse_die(parser, &reader, die, sections, unit);

                if(die->abbrevCode == 0)
                {
                    //NOTE: this is a null entry that marks the end of a siblings chain
                    parentDIE = parentDIE->parent;
                }
                else
                {
                    if(die->abbrev->hasChildren)
                    {
                        parentDIE = die;
                    }
                }
            }
            while(parentDIE != 0);
        }
        else
        {
            oc_log_warning("first DIE is not a compile unit DIE\n");
        }

        //NOTE: Add unit to unit list
        oc_list_push_back(&units, &unitElt->listElt);
        info->unitCount++;

        // skip to next unit
        wa_reader_seek(&reader, unit->start + unit->initialLength + (unit->format == DW_DWARF32 ? 4 : 8));
    }
    //NOTE: copy units in fixed array
    info->units = oc_arena_push_array(parser->arena, dw_unit, info->unitCount);
    oc_list_for_indexed(units, unitIt, dw_unit_elt, listElt)
    {
        info->units[unitIt.index] = unitIt.elt->unit;
    }

    oc_scratch_end(scratch);
}

dw_info* dw_parse_dwarf(dw_parser* parser)
{
    dw_info* dwarf = oc_arena_push_type(parser->arena, dw_info);

    wa_reader_set_error_callback(&parser->rootReader, dw_parser_read_error_callback, parser);

    dw_parse_info(parser, &parser->sections, dwarf);

    dwarf->line = dw_load_line_info(parser, &parser->sections);

    return dwarf;
}

//------------------------------------------------------------------------
// traversing DIEs
//------------------------------------------------------------------------
dw_die* dw_die_next(dw_die* root, dw_die* die)
{
    if(!oc_list_empty(die->children))
    {
        die = oc_list_first_entry(die->children, dw_die, parentElt);
    }
    else if(die->parentElt.next)
    {
        die = oc_list_entry(die->parentElt.next, dw_die, parentElt);
    }
    else if(die->parent && die->parent != root && die->parent->parentElt.next)
    {
        die = oc_list_entry(die->parent->parentElt.next, dw_die, parentElt);
    }
    else
    {
        die = 0;
    }

    return die;
}

dw_die* dw_die_find_next_with_tags(dw_die* root, dw_die* start, u64 count, dw_tag* tags)
{
    dw_die* die = start;

    while(die)
    {
        if(die != start && die->abbrev)
        {
            for(u64 i = 0; i < count; i++)
            {
                if(die->abbrev->tag == tags[i])
                {
                    return die;
                }
            }
        }

        if(!oc_list_empty(die->children))
        {
            die = oc_list_first_entry(die->children, dw_die, parentElt);
        }
        else if(die->parentElt.next)
        {
            die = oc_list_entry(die->parentElt.next, dw_die, parentElt);
        }
        else if(die->parent && die->parent != root && die->parent->parentElt.next)
        {
            die = oc_list_entry(die->parent->parentElt.next, dw_die, parentElt);
        }
        else
        {
            break;
        }
    }
    return 0;
}

dw_die* dw_die_find_next_with_tag(dw_die* root, dw_die* start, dw_tag tag)
{
    return dw_die_find_next_with_tags(root, start, 1, &tag);
}

dw_attr* dw_die_get_attr(dw_die* die, dw_attr_name name)
{
    dw_attr* res = 0;
    for(u64 attrIndex = 0; attrIndex < die->abbrev->attrCount; attrIndex++)
    {
        dw_attr* attr = &die->attributes[attrIndex];
        if(attr->abbrev->name == name)
        {
            res = attr;
            break;
        }
    }
    return res;
}

//------------------------------------------------------------------------
// debug printing
//------------------------------------------------------------------------

void dw_print_indent(u64 indent)
{
    for(u64 i = 0; i < indent; i++)
    {
        printf("  ");
    }
}

void dw_print_expr(dw_unit* unit, dw_expr expr)
{
    u64 pc = 0;
    while(pc < expr.codeLen)
    {
        dw_expr_instr* instr = &expr.code[pc];

        printf("%.*s ", oc_str8_ip(dw_op_get_string(instr->op)));

        const dw_op_info* info = &DW_OP_INFO[instr->op];

        if(instr->op == DW_OP_implicit_value)
        {
            for(u64 i = 0; i < instr->operands[1].string.len; i++)
            {
                printf("0x%0hhx ", instr->operands[1].string.ptr[i]);
            }
        }
        else if(instr->op == DW_OP_WASM_location)
        {
            printf("0x%0hhx ", instr->operands[0].valU8);
            printf("0x%0x ", instr->operands[1].valU32);
        }
        else
        {
            for(u32 opdIndex = 0; opdIndex < info->count; opdIndex++)
            {
                dw_val* val = &instr->operands[opdIndex];
                switch(info->opd[opdIndex])
                {
                    case DW_OPD_U8:
                    case DW_OPD_I8:
                        printf("0x%0hhx ", val->valU8);
                        break;
                    case DW_OPD_U16:
                    case DW_OPD_I16:
                        printf("0x%0hx ", val->valU16);
                        break;
                    case DW_OPD_U32:
                    case DW_OPD_I32:
                    case DW_OPD_ADDR:
                        printf("0x%0x ", val->valU32);
                        break;
                    case DW_OPD_U64:
                    case DW_OPD_I64:
                    case DW_OPD_ULEB:
                    case DW_OPD_SLEB:
                    case DW_OPD_REF:
                        printf("0x%0llx ", val->valU64);
                        break;
                }
            }
        }
    }
}

void dw_print_loc(dw_unit* unit, dw_loc loc, int indent)
{
    if(loc.single)
    {
        dw_print_expr(unit, loc.entries[0].expr);
    }
    else
    {
        u64 baseOffset = 0; //TODO: should be unit's base offset
        for(u64 i = 0; i < loc.entryCount; i++)
        {
            if(i)
            {
                printf("            "); // space of address
                dw_print_indent(indent);
            }
            dw_loc_entry* entry = &loc.entries[i];
            if(entry->start == 0xffffffffffffffff)
            {
                printf("0x%08llx:", entry->end);
                baseOffset = entry->end;
            }
            else
            {
                printf("[0x%08llx, 0x%08llx): ", entry->start + baseOffset, entry->end + baseOffset);
                dw_print_expr(unit, entry->expr);
            }
            if(i < loc.entryCount - 1)
            {
                printf("\n");
            }
        }
    }
}

void dw_print_die(dw_unit* unit, dw_die* die, u32 indent)
{
    printf("0x%08llx: ", die->start);
    dw_print_indent(indent);

    if(die->abbrevCode == 0)
    {
        printf("NULL\n\n");
    }
    else
    {
        printf("%s\n", dw_get_tag_string(die->abbrev->tag));

        for(u64 attrIndex = 0; attrIndex < die->abbrev->attrCount; attrIndex++)
        {
            dw_attr* attr = &die->attributes[attrIndex];

            dw_print_indent(indent + 7);
            printf("%s", dw_get_attr_name_string(attr->abbrev->name));
            printf("\t[%s]", dw_get_form_string(attr->abbrev->form));

            if(attr->abbrev->form == DW_FORM_string
               || attr->abbrev->form == DW_FORM_strp
               || attr->abbrev->form == DW_FORM_line_strp
               || attr->abbrev->form == DW_FORM_strx1
               || attr->abbrev->form == DW_FORM_strx2
               || attr->abbrev->form == DW_FORM_strx3
               || attr->abbrev->form == DW_FORM_strx4
               || attr->abbrev->form == DW_FORM_strx)
            {
                printf("\t(\"%.*s\")", oc_str8_ip(attr->string));
            }
            else if(attr->abbrev->name == DW_AT_location || attr->abbrev->name == DW_AT_frame_base)
            {
                printf("\t(");
                dw_print_loc(unit, attr->loc, indent + 2);
                printf(")");
            }
            printf("\n");
        }

        printf("\n");
        oc_list_for(die->children, child, dw_die, parentElt)
        {
            dw_print_die(unit, child, indent + 1);
        }
    }
}

void dw_print_debug_info(dw_info* info)
{
    for(u64 unitIndex = 0; unitIndex < info->unitCount; unitIndex++)
    {
        dw_unit* unit = &info->units[unitIndex];

        printf("0x%016llx\n", unit->start);
        printf("    type: %s\n", dw_get_cu_type_string(unit->type));
        printf("    version: %i\n", unit->version);

        dw_print_die(unit, unit->rootDie, 0);
    }
}

void dw_print_line_table_header(void)
{
    printf("Address            Line   Column File   ISA Discriminator OpIndex Flags\n"
           "------------------ ------ ------ ------ --- ------------- ------- -------------\n");
}

void dw_print_line_info(dw_line_info* info)
{
    printf(".debug_line contents:\n");
    for(u64 tableIndex = 0; tableIndex < info->tableCount; tableIndex++)
    {
        dw_line_table* table = &info->tables[tableIndex];

        printf("debug_line[0x%08llx]\n", table->header.offset);
        printf("Line table prologue:\n");
        printf("    total_length: 0x%08llx\n", table->header.unitLength);
        printf("          format: %s\n", table->header.addressSize == 4 ? "DWARF32" : "DWARF64");
        printf("         version: %i\n", table->header.version);
        if(table->header.version >= 5)
        {
            printf("    address_size: %i\n", table->header.addressSize);
            printf(" seg_select_size: %i\n", table->header.segmentSelectorSize);
        }
        printf(" prologue_length: 0x%08llx\n", table->header.headerLength);
        printf(" min_inst_length: %i\n", table->header.minInstructionLength);
        printf("max_ops_per_inst: %i\n", table->header.maxOperationsPerInstruction);
        printf(" default_is_stmt: %i\n", table->header.defaultIsStmt ? 1 : 0);
        printf("       line_base: %i\n", table->header.lineBase);
        printf("      line_range: %i\n", table->header.lineRange);
        printf("     opcode_base: %i\n", table->header.opcodeBase);

        for(int i = 0; i < 12; i++)
        {
            printf("standard_opcode_lengths[%s] = %i\n",
                   dw_get_line_standard_opcode_string(i + 1),
                   table->header.standardOpcodeLength[i]);
        }

        for(int i = 0; i < table->header.dirEntryCount; i++)
        {
            int displayIndex = (table->header.version >= 5) ? i : i + 1;
            printf("include_directories[%3i] = \"%.*s\"\n", displayIndex, oc_str8_ip(table->header.dirEntries[i].path));
        }
        for(int i = 0; i < table->header.fileEntryCount; i++)
        {
            dw_file_entry* entry = &(table->header.fileEntries[i]);

            if(table->header.version < 5)
            {
                printf("file_names[%3i]:\n"
                       "           name: \"%.*s\"\n"
                       "      dir_index: %llu\n"
                       "       mod_time: 0x%08llx\n"
                       "         length: 0x%08llx\n",
                       i + 1,
                       oc_str8_ip(entry->path),
                       entry->dirIndex,
                       entry->timestamp,
                       entry->size);
            }
            else
            {
                printf("file_names[%3i]:\n"
                       "           name: \"%.*s\"\n"
                       "      dir_index: %llu\n",
                       i,
                       oc_str8_ip(entry->path),
                       entry->dirIndex);

                if(table->header.fileFlags & DW_LNCT_has_timestamp)
                {
                    printf("       mod_time: 0x%08llx\n",
                           entry->timestamp);
                }

                if(table->header.fileFlags & DW_LNCT_has_size)
                {
                    printf("         length: 0x%08llx\n",
                           entry->size);
                }

                if(table->header.fileFlags & DW_LNCT_has_MD5)
                {
                    printf("   md5_checksum: ");
                    for(int i = 0; i < 16; i++)
                    {
                        printf("%02hhx", entry->md5[i]);
                    }
                    printf("\n");
                }
            }
        }

        {
            /*NOTE(martin):
                This reproduces the behaviour of llvm-dwarfdump, which only prints the
                table headings if the program code is non empty.
                This is NOT be the same as an _empty table_: eg llvm-dwarfdump does produce
                headings even if the table is entirely tombstoned.
            */
            u64 unitLengthSize = (table->header.addressSize == 4) ? 4 : 12;
            u64 programEnd = unitLengthSize + table->header.unitLength;
            u64 prologueStart = unitLengthSize + 2 + table->header.addressSize;
            if(table->header.version >= 5)
            {
                prologueStart += 2;
            }
            u64 prologueEnd = prologueStart + table->header.headerLength;

            if(prologueEnd < programEnd)
            {
                printf("\n");
                dw_print_line_table_header();
            }
        }

        for(u64 rowIndex = 0; rowIndex < table->entryCount; rowIndex++)
        {
            dw_line_entry* entry = &table->entries[rowIndex];

            printf("0x%016llx %6llu %6llu %6llu %3llu %13llu %7llu %s%s%s%s%s\n",
                   entry->address,
                   entry->line,
                   entry->column,
                   entry->file,
                   entry->isa,
                   entry->discriminator,
                   entry->opIndex,
                   (entry->flags & DW_LINE_STMT) ? " is_stmt" : "",
                   (entry->flags & DW_LINE_BASIC_BLOCK) ? " basic_block" : "",
                   (entry->flags & DW_LINE_SEQUENCE_END) ? " end_sequence" : "",
                   (entry->flags & DW_LINE_PROLOGUE_END) ? " prologue_end" : "",
                   (entry->flags & DW_LINE_EPILOGUE_BEGIN) ? " epilogue_begin" : "");
        }

        printf("\n");
    }
}
