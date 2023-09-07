/************************************************************/ /**
*
*	@file: cheatsheet_util.h
*	@author: Martin Fouilleul
*	@date: 05/09/2023
*
*****************************************************************/

//----------------------------------------------------------------
// Arenas
//----------------------------------------------------------------

void oc_arena_init(oc_arena* arena);
void oc_arena_init_with_options(oc_arena* arena, oc_arena_options* options);
void oc_arena_cleanup(oc_arena* arena);

void* oc_arena_push(oc_arena* arena, u64 size);
#define oc_arena_push_type(arena, type)
#define oc_arena_push_array(arena, type, count)
void oc_arena_clear(oc_arena* arena);

oc_arena_scope oc_arena_scope_begin(oc_arena* arena);
void oc_arena_scope_end(oc_arena_scope scope);

oc_arena_scope oc_scratch_begin(void);
oc_arena_scope oc_scratch_begin_next(oc_arena* used);
#define oc_scratch_end(scope)

//----------------------------------------------------------------
// Lists
//----------------------------------------------------------------

void oc_list_init(oc_list* list);
bool oc_list_empty(oc_list* list);

oc_list_elt* oc_list_begin(oc_list* list);
oc_list_elt* oc_list_end(oc_list* list);
oc_list_elt* oc_list_last(oc_list* list);

#define oc_list_next(elt)
#define oc_list_prev(elt)
#define oc_list_entry(ptr, type, member)
#define oc_list_next_entry(list, elt, type, member)
#define oc_list_prev_entry(list, elt, type, member)
#define oc_list_first_entry(list, type, member)
#define oc_list_last_entry(list, type, member)
#define oc_list_pop_entry(list, type, member)

void oc_list_insert(oc_list* list, oc_list_elt* afterElt, oc_list_elt* elt);
void oc_list_insert_before(oc_list* list, oc_list_elt* beforeElt, oc_list_elt* elt);
void oc_list_remove(oc_list* list, oc_list_elt* elt);
void oc_list_push(oc_list* list, oc_list_elt* elt);
oc_list_elt* oc_list_pop(oc_list* list);
void oc_list_push_back(oc_list* list, oc_list_elt* elt);
oc_list_elt* oc_list_pop_back(oc_list* list);

#define oc_list_for(list, elt, type, member)
#define oc_list_for_reverse(list, elt, type, member)
#define oc_list_for_safe(list, elt, type, member)

//----------------------------------------------------------------
// Strings / string lists / path strings
//----------------------------------------------------------------

oc_str8 oc_str8_from_buffer(u64 len, char* buffer);
oc_str8 oc_str8_slice(oc_str8 s, u64 start, u64 end);

oc_str8 oc_str8_push_buffer(oc_arena* arena, u64 len, char* buffer);
oc_str8 oc_str8_push_cstring(oc_arena* arena, const char* str);
oc_str8 oc_str8_push_copy(oc_arena* arena, oc_str8 s);
oc_str8 oc_str8_push_slice(oc_arena* arena, oc_str8 s, u64 start, u64 end);
oc_str8 oc_str8_pushfv(oc_arena* arena, const char* format, va_list args);
oc_str8 oc_str8_pushf(oc_arena* arena, const char* format, ...);

int oc_str8_cmp(oc_str8 s1, oc_str8 s2);

char* oc_str8_to_cstring(oc_arena* arena, oc_str8 string);

void oc_str8_list_push(oc_arena* arena, oc_str8_list* list, oc_str8 str);
void oc_str8_list_pushf(oc_arena* arena, oc_str8_list* list, const char* format, ...);

oc_str8 oc_str8_list_collate(oc_arena* arena, oc_str8_list list, oc_str8 prefix, oc_str8 separator, oc_str8 postfix);
oc_str8 oc_str8_list_join(oc_arena* arena, oc_str8_list list);
oc_str8_list oc_str8_split(oc_arena* arena, oc_str8 str, oc_str8_list separators);

oc_str8 oc_path_slice_directory(oc_str8 path);
oc_str8 oc_path_slice_filename(oc_str8 path);
oc_str8_list oc_path_split(oc_arena* arena, oc_str8 path);
oc_str8 oc_path_join(oc_arena* arena, oc_str8_list elements);
oc_str8 oc_path_append(oc_arena* arena, oc_str8 parent, oc_str8 relPath);
bool oc_path_is_absolute(oc_str8 path);

//----------------------------------------------------------------
// Debugging
//----------------------------------------------------------------
#define oc_log_info(message, ...)
#define oc_log_warning(message, ...)
#define oc_log_error(message, ...)

#define OC_ASSERT(test, message, ...)
#define OC_ABORT(message, ...)
