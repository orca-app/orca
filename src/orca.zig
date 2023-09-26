const std = @import("std");

//------------------------------------------------------------------------------------------
// [PLATFORM]
//------------------------------------------------------------------------------------------

pub const Platform = enum(c_uint) {
    MacOS,
    Windows,
};

extern fn oc_get_host_platform() Platform;
pub const getHostPlatform = oc_get_host_platform;

//------------------------------------------------------------------------------------------
// [DEBUG] Logging
//------------------------------------------------------------------------------------------

pub const log = struct {
    pub const Level = enum(c_uint) {
        Error,
        Warning,
        Info,
    };

    pub const Output = opaque {};

    extern var OC_LOG_DEFAULT_OUTPUT: ?*Output;
    extern fn oc_log_set_level(level: Level) void;
    extern fn oc_log_set_output(output: *Output) void;
    extern fn oc_log_ext(level: Level, function: [*]const u8, file: [*]const u8, line: c_int, fmt: [*]const u8, ...) void;

    const DEFAULT_OUTPUT = OC_LOG_DEFAULT_OUTPUT;

    pub fn info(comptime fmt: []const u8, args: anytype, source: std.builtin.SourceLocation) void {
        ext(Level.Info, fmt, args, source);
    }

    pub fn warn(comptime fmt: []const u8, args: anytype, source: std.builtin.SourceLocation) void {
        ext(Level.Warning, fmt, args, source);
    }

    pub fn err(comptime fmt: []const u8, args: anytype, source: std.builtin.SourceLocation) void {
        ext(Level.Error, fmt, args, source);
    }

    pub fn ext(comptime level: Level, comptime fmt: []const u8, args: anytype, source: std.builtin.SourceLocation) void {
        var format_buf: [512:0]u8 = undefined;
        _ = std.fmt.bufPrintZ(&format_buf, fmt, args) catch 0; // just discard NoSpaceLeft error for now
        var line: c_int = @intCast(source.line);

        oc_log_ext(level, source.fn_name.ptr, source.file.ptr, line, format_buf[0..].ptr);
    }
};

//------------------------------------------------------------------------------------------
// [DEBUG] Assert/Abort
//------------------------------------------------------------------------------------------

extern fn oc_abort_ext(file: [*]const u8, function: [*]const u8, line: c_int, fmt: [*]const u8, ...) void;
extern fn oc_assert_fail(file: [*]const u8, function: [*]const u8, line: c_int, src: [*]const u8, fmt: [*]const u8, ...) void;

pub fn assert(condition: bool, comptime fmt: []const u8, args: anytype, source: std.builtin.SourceLocation) void {
    if (condition == false) {
        var format_buf: [512:0]u8 = undefined;
        _ = std.fmt.bufPrintZ(&format_buf, fmt, args) catch 0;
        var line: c_int = @intCast(source.line);

        oc_assert_fail(source.file.ptr, source.fn_name.ptr, line, "", format_buf[0..].ptr);
    }
}

pub fn abort(comptime fmt: []const u8, args: anytype, source: std.builtin.SourceLocation) void {
    var format_buf: [512:0]u8 = undefined;
    _ = std.fmt.bufPrintZ(&format_buf, fmt, args) catch 0;
    var line: c_int = @intCast(source.line);

    oc_abort_ext(source.file.ptr, source.fn_name.ptr, line, format_buf[0..].ptr);
}

//------------------------------------------------------------------------------------------
// [INTRUSIVE LIST]
//------------------------------------------------------------------------------------------

pub const ListElt = extern struct {
    prev: ?*ListElt,
    next: ?*ListElt,
};

pub const List = extern struct {
    first: ?*ListElt,
    last: ?*ListElt,

    pub fn init() List {
        return .{
            .first = null,
            .last = null,
        };
    }

    pub fn begin(self: *List) ?*ListElt {
        return self.first;
    }

    pub fn end() ?*ListElt {
        return null;
    }

    pub fn last(self: *List) ?*ListElt {
        return self.last;
    }

    pub fn insert(self: *List, after_elt: *ListElt, elt: *ListElt) void {
        elt.prev = after_elt;
        elt.next = after_elt.next;
        if (after_elt.next != null) {
            after_elt.next.prev = elt;
        } else {
            self.last = elt;
        }
        after_elt.next = elt;
    }

    pub fn insertBefore(self: *List, before_elt: *ListElt, elt: *ListElt) void {
        elt.next = before_elt;
        elt.prev = before_elt.prev;
        if (before_elt.prev != null) {
            before_elt.prev.next = elt;
        } else {
            self.first = elt;
        }
        before_elt.prev = elt;
    }

    pub fn remove(self: *List, elt: *ListElt) void {
        if (elt.prev != null) {
            elt.prev.next = elt.next;
        } else {
            self.first = elt.next;
        }
        if (elt.next != null) {
            elt.next.prev = elt.prev;
        } else {
            self.last = elt.prev;
        }
        elt.prev = blk: {
            const tmp = null;
            elt.next = tmp;
            break :blk tmp;
        };
    }

    pub fn push(self: *List, elt: *ListElt) void {
        elt.next = self.first;
        elt.prev = null;
        if (self.first != null) {
            self.first.prev = elt;
        } else {
            self.last = elt;
        }
        self.first = elt;
    }

    pub fn pop(self: *List) ListElt {
        var elt: *ListElt = begin(self);
        if (elt != end(self)) {
            remove(self, elt);
            return elt;
        }
        return null;
    }

    pub fn pushBack(self: *List, elt: *ListElt) void {
        elt.prev = self.last;
        elt.next = null;
        if (self.last != null) {
            self.last.next = elt;
        } else {
            self.first = elt;
        }
        self.last = elt;
    }

    pub fn popBack(self: *List) ListElt {
        var elt: *ListElt = last(self);
        if (elt != end(self)) {
            remove(self, elt);
            return elt;
        }
        return null;
    }

    pub fn empty(self: *const List) bool {
        return (self.first == null) or (self.last == null);
    }
};

//------------------------------------------------------------------------------------------
// [MEMORY]
//------------------------------------------------------------------------------------------

pub const BaseAllocator = extern struct {
    const MemReserveFunction = *const fn (context: *BaseAllocator, size: u64) ?[*]u8;
    const MemModifyFunction = *const fn (context: *BaseAllocator, ptr: ?[*]u8, size: u64) ?[*]u8;

    func_reserve: MemReserveFunction,
    func_commit: MemModifyFunction,
    func_decommit: MemModifyFunction,
    func_release: MemModifyFunction,

    extern fn oc_base_allocator_default() *BaseAllocator;
    pub const default = oc_base_allocator_default;
};

pub const ArenaChunk = extern struct {
    list_elt: ListElt,
    ptr: ?[*]u8,
    offset: u64,
    committed: u64,
    cap: u64,
};

pub const Arena = extern struct {
    base: ?*BaseAllocator,
    chunks: List,
    current_chunk: ?*ArenaChunk,

    extern fn oc_arena_init(arena: *Arena) void;
    extern fn oc_arena_init_with_options(arena: *Arena, options: *ArenaOptions) void;
    extern fn oc_arena_cleanup(arena: *Arena) void;
    extern fn oc_arena_push(arena: *Arena, size: u64) ?[*]u8;
    extern fn oc_arena_clear(arena: *Arena) void;
    extern fn oc_arena_scope_begin(arena: *Arena) ArenaScope;
    extern fn oc_arena_scope_end(scope: ArenaScope) void;

    extern fn oc_scratch() *Arena;
    extern fn oc_scratch_next(used: *Arena) *Arena;
    extern fn oc_scratch_begin() ArenaScope;
    extern fn oc_scratch_begin_next(used: *Arena) ArenaScope;

    pub fn init() Arena {
        var arena: Arena = undefined;
        oc_arena_init(&arena);
        return arena;
    }
    pub fn initWithOptions(opts: ArenaOptions) Arena {
        var arena: Arena = undefined;
        oc_arena_init(&arena, &opts);
        return arena;
    }
    pub const deinit = oc_arena_cleanup;
    pub fn push(arena: *Arena, size: u64) ?[]u8 {
        if (oc_arena_push(arena, size)) |mem| {
            return mem[0..size];
        }
        return null;
    }
    pub const clear = oc_arena_clear;
    pub const scopeBegin = oc_arena_scope_begin;
    pub const scopeEnd = oc_arena_scope_end;

    pub fn pushType(arena: *Arena, comptime T: type) ?*T {
        if (arena.push(@sizeOf(T))) |mem| {
            std.debug.assert(mem.len >= @sizeOf(T));
            return @alignCast(@ptrCast(mem.ptr));
        }

        return null;
    }

    pub fn pushArray(arena: *Arena, comptime T: type, count: u64) ?[]T {
        if (arena.push(@sizeOf(T) * count)) |mem| {
            std.debug.assert(mem.len >= @sizeOf(T) * count);
            var items: [*]T = @alignCast(@ptrCast(mem.ptr));
            return items[0..count];
        }
        return null;
    }

    pub const scratch = oc_scratch;
    pub const scratchNext = oc_scratch_next;
    pub const scratchBegin = oc_scratch_begin;
    pub const scratchBeginNext = oc_scratch_begin_next;
    pub const scratchEnd = scopeEnd;
};

pub const ArenaScope = extern struct {
    arena: ?*Arena,
    chunk: ?*ArenaChunk,
    offset: u64,
};

pub const ArenaOptions = extern struct {
    base: ?*BaseAllocator,
    reserve: u64,
};

//------------------------------------------------------------------------------------------
// [STRINGS] u8 strings
//------------------------------------------------------------------------------------------

fn stringType(comptime CharType: type) type {
    return extern struct {
        pub const StrListElt = extern struct {
            list_elt: ListElt,
            string: Str,
        };

        pub const StrList = extern struct {
            list: List,
            elt_count: u64,
            len: u64,

            pub fn init() StrList {
                return .{
                    .list = List.init(),
                    .elt_count = 0,
                    .len = 0,
                };
            }

            pub fn push(list: *StrList, str: *Str, arena: *Arena) void {
                var elt: *StrListElt = arena.pushType(ListElt);
                elt.string = str;
                list.append(elt.list_elt);
                list.elt_count += 1;
                list.len += str.len;
            }

            pub fn pushf(list: *StrList, arena: *Arena, comptime format: []const u8, args: anytype) Str {
                var str = Str.pushf(arena, format, args);
                list.push(str, arena);
            }

            // TODO
            // pub fn join(list: *const StrList, arena: *Arena) Str;
            //     const empty = Str{ .ptr = null, .len = 0 };
            //     return list.collate(arena, empty, empty, empty));
            // }
            // pub fn collate(list: *StrList, arena: *Arena, prefix: Str, separator: Str, postfix: Str) Str;
        };

        const Str = @This();

        ptr: ?[*]CharType,
        len: usize,

        extern fn strncmp(a: ?[*]u8, b: ?[*]u8, len: usize) c_int;

        pub fn fromSlice(str: []const CharType) Str {
            return .{
                .ptr = @constCast(str.ptr),
                .len = str.len,
            };
        }

        pub fn slice(str: *const Str) []CharType {
            if (str.ptr) |p| {
                return p[0..str.len];
            }

            return &[_]CharType{};
        }

        pub fn sliceInner(str: *const Str, start: u64, end: u64) []CharType {
            if (str.ptr) |p| {
                assert(start <= end, "{}.sliceLen: start <= end", .{typename()}, @src());
                assert(end <= str.len, "{}.sliceLen: end <= str.len", .{typename()}, @src());
                return p[start..end];
            }

            return &[_]CharType{};
        }

        pub fn fromBuffer(len: u64, buffer: ?[]CharType) Str {
            return .{
                .ptr = buffer,
                .len = @intCast(len),
            };
        }

        pub fn pushBuffer(arena: *Arena, buffer: []u8) Str {
            var str: Str = undefined;
            str.len = buffer.len;
            str.ptr = arena.pushArray(CharType, buffer.len + 1);
            @memcpy(str.ptr[0..buffer.len], buffer);
            str.ptr[buffer.len] = 0;
            return str;
        }

        pub fn pushCopy(str: *const Str, arena: *Arena) Str {
            return pushBuffer(arena, str.ptr[0..str.len]);
        }

        pub fn pushSlice(str: *const Str, arena: *Arena, start: usize, end: usize) Str {
            return pushBuffer(arena, str.ptr[start..end]);
        }

        pub fn pushf(arena: *Arena, format: []const u8, args: anytype) Str {
            if (CharType != u8) {
                @compileError("pushf() is only supported for Str8");
            }

            var str: Str = undefined;
            str.len = @intCast(std.fmt.count(format, args));
            str.ptr = arena.pushArray(CharType, str.len + 1);
            _ = std.fmt.bufPrintZ(str.ptr[0 .. str.len + 1], format, args) catch unreachable;
            return str;
        }

        // TODO
        // pub fn split() void;

        pub fn cmp(a: *const Str, b: *const Str) std.math.Order {
            if (CharType != u8) {
                @compileError("cmp() is only supported for Str8");
            }
            const value = strncmp(a.ptr, b.ptr, std.math.min(a.len, b.len));
            if (value < 0) {
                return .lt;
            } else if (value == 0) {
                return .eq;
            } else {
                return .gt;
            }
        }

        fn typename() []const u8 {
            return switch (CharType) {
                u8 => "Str8",
                u16 => "Str16",
                u32 => "Str32",
                else => unreachable,
            };
        }
    };
}

pub const Str8 = stringType(u8);
pub const Str16 = stringType(u16);
pub const Str32 = stringType(u32);

pub const Str8ListElt = Str8.StrListElt;
pub const Str16ListElt = Str16.StrListElt;
pub const Str32ListElt = Str32.StrListElt;

pub const Str8List = Str8.StrList;
pub const Str16List = Str16.StrList;
pub const Str32List = Str32.StrList;

//------------------------------------------------------------------------------------------
// [UTF8]
//------------------------------------------------------------------------------------------

pub const Utf32 = u32;

pub const Utf8Dec = extern struct {
    code_point: Utf32, // decoded codepoint
    size: u32, // size of corresponding oc_utf8 sequence
};

pub const Utf8 = struct {

    // getting sizes / offsets / indices
    extern fn oc_utf8_size_from_leading_char(leadingChar: c_char) u32;
    extern fn oc_utf8_codepoint_size(code_point: Utf32) u32;
    extern fn oc_utf8_codepoint_count_for_string(string: Str8) u64;
    extern fn oc_utf8_byte_count_for_codepoints(code_points: Str32) u64;
    extern fn oc_utf8_next_offset(string: Str8, byteOffset: u64) u64;
    extern fn oc_utf8_prev_offset(string: Str8, byteOffset: u64) u64;

    pub const sizeFromLeadingChar = oc_utf8_size_from_leading_char;
    pub const codepointSize = oc_utf8_codepoint_size;
    pub const codepointCountForString = oc_utf8_codepoint_count_for_string;
    pub const byteCountForCodepoints = oc_utf8_byte_count_for_codepoints;
    pub const nextOffset = oc_utf8_next_offset;
    pub const prevOffset = oc_utf8_prev_offset;

    // encoding / decoding
    extern fn oc_utf8_decode(string: Str8) Utf8Dec; //NOTE: decode a single oc_utf8 sequence at start of string
    extern fn oc_utf8_decode_at(string: Str8, offset: u64) Utf8Dec; //NOTE: decode a single oc_utf8 sequence starting at byte offset
    extern fn oc_utf8_encode(dst: [*]u8, code_point: Utf32) Str8; //NOTE: encode codepoint into backing buffer dst
    extern fn oc_utf8_to_codepoints(maxCount: u64, backing: [*]Utf32, string: Str8) Str32;
    extern fn oc_utf8_from_codepoints(maxBytes: u64, backing: [*]c_char, code_points: Str32) Str8;
    extern fn oc_utf8_push_to_codepoints(arena: *Arena, string: Str8) Str32;
    extern fn oc_utf8_push_from_codepoints(arena: *Arena, code_points: Str32) Str8;

    pub const decode = oc_utf8_decode;
    pub const decodeAt = oc_utf8_decode_at;
    pub const encode = oc_utf8_encode;
    pub const toCodepoints = oc_utf8_to_codepoints;
    pub const fromCodepoints = oc_utf8_from_codepoints;
    pub const pushToCodepoints = oc_utf8_push_to_codepoints;
    pub const pushFromCodepoints = oc_utf8_push_from_codepoints;
};

//------------------------------------------------------------------------------------------
// [UTF8] Unicode
//------------------------------------------------------------------------------------------

pub const UnicodeRange = extern struct {
    first_code_point: Utf32,
    count: u32,

    extern const OC_UNICODE_BASIC_LATIN: UnicodeRange;
    extern const OC_UNICODE_C1_CONTROLS_AND_LATIN_1_SUPPLEMENT: UnicodeRange;
    extern const OC_UNICODE_LATIN_EXTENDED_A: UnicodeRange;
    extern const OC_UNICODE_LATIN_EXTENDED_B: UnicodeRange;
    extern const OC_UNICODE_IPA_EXTENSIONS: UnicodeRange;
    extern const OC_UNICODE_SPACING_MODIFIER_LETTERS: UnicodeRange;
    extern const OC_UNICODE_COMBINING_DIACRITICAL_MARKS: UnicodeRange;
    extern const OC_UNICODE_GREEK_COPTIC: UnicodeRange;
    extern const OC_UNICODE_CYRILLIC: UnicodeRange;
    extern const OC_UNICODE_CYRILLIC_SUPPLEMENT: UnicodeRange;
    extern const OC_UNICODE_ARMENIAN: UnicodeRange;
    extern const OC_UNICODE_HEBREW: UnicodeRange;
    extern const OC_UNICODE_ARABIC: UnicodeRange;
    extern const OC_UNICODE_SYRIAC: UnicodeRange;
    extern const OC_UNICODE_THAANA: UnicodeRange;
    extern const OC_UNICODE_DEVANAGARI: UnicodeRange;
    extern const OC_UNICODE_BENGALI_ASSAMESE: UnicodeRange;
    extern const OC_UNICODE_GURMUKHI: UnicodeRange;
    extern const OC_UNICODE_GUJARATI: UnicodeRange;
    extern const OC_UNICODE_ORIYA: UnicodeRange;
    extern const OC_UNICODE_TAMIL: UnicodeRange;
    extern const OC_UNICODE_TELUGU: UnicodeRange;
    extern const OC_UNICODE_KANNADA: UnicodeRange;
    extern const OC_UNICODE_MALAYALAM: UnicodeRange;
    extern const OC_UNICODE_SINHALA: UnicodeRange;
    extern const OC_UNICODE_THAI: UnicodeRange;
    extern const OC_UNICODE_LAO: UnicodeRange;
    extern const OC_UNICODE_TIBETAN: UnicodeRange;
    extern const OC_UNICODE_MYANMAR: UnicodeRange;
    extern const OC_UNICODE_GEORGIAN: UnicodeRange;
    extern const OC_UNICODE_HANGUL_JAMO: UnicodeRange;
    extern const OC_UNICODE_ETHIOPIC: UnicodeRange;
    extern const OC_UNICODE_CHEROKEE: UnicodeRange;
    extern const OC_UNICODE_UNIFIED_CANADIAN_ABORIGINAL_SYLLABICS: UnicodeRange;
    extern const OC_UNICODE_OGHAM: UnicodeRange;
    extern const OC_UNICODE_RUNIC: UnicodeRange;
    extern const OC_UNICODE_TAGALOG: UnicodeRange;
    extern const OC_UNICODE_HANUNOO: UnicodeRange;
    extern const OC_UNICODE_BUHID: UnicodeRange;
    extern const OC_UNICODE_TAGBANWA: UnicodeRange;
    extern const OC_UNICODE_KHMER: UnicodeRange;
    extern const OC_UNICODE_MONGOLIAN: UnicodeRange;
    extern const OC_UNICODE_LIMBU: UnicodeRange;
    extern const OC_UNICODE_TAI_LE: UnicodeRange;
    extern const OC_UNICODE_KHMER_SYMBOLS: UnicodeRange;
    extern const OC_UNICODE_PHONETIC_EXTENSIONS: UnicodeRange;
    extern const OC_UNICODE_LATIN_EXTENDED_ADDITIONAL: UnicodeRange;
    extern const OC_UNICODE_GREEK_EXTENDED: UnicodeRange;
    extern const OC_UNICODE_GENERAL_PUNCTUATION: UnicodeRange;
    extern const OC_UNICODE_SUPERSCRIPTS_AND_SUBSCRIPTS: UnicodeRange;
    extern const OC_UNICODE_CURRENCY_SYMBOLS: UnicodeRange;
    extern const OC_UNICODE_COMBINING_DIACRITICAL_MARKS_FOR_SYMBOLS: UnicodeRange;
    extern const OC_UNICODE_LETTERLIKE_SYMBOLS: UnicodeRange;
    extern const OC_UNICODE_NUMBER_FORMS: UnicodeRange;
    extern const OC_UNICODE_ARROWS: UnicodeRange;
    extern const OC_UNICODE_MATHEMATICAL_OPERATORS: UnicodeRange;
    extern const OC_UNICODE_MISCELLANEOUS_TECHNICAL: UnicodeRange;
    extern const OC_UNICODE_CONTROL_PICTURES: UnicodeRange;
    extern const OC_UNICODE_OPTICAL_CHARACTER_RECOGNITION: UnicodeRange;
    extern const OC_UNICODE_ENCLOSED_ALPHANUMERICS: UnicodeRange;
    extern const OC_UNICODE_BOX_DRAWING: UnicodeRange;
    extern const OC_UNICODE_BLOCK_ELEMENTS: UnicodeRange;
    extern const OC_UNICODE_GEOMETRIC_SHAPES: UnicodeRange;
    extern const OC_UNICODE_MISCELLANEOUS_SYMBOLS: UnicodeRange;
    extern const OC_UNICODE_DINGBATS: UnicodeRange;
    extern const OC_UNICODE_MISCELLANEOUS_MATHEMATICAL_SYMBOLS_A: UnicodeRange;
    extern const OC_UNICODE_SUPPLEMENTAL_ARROWS_A: UnicodeRange;
    extern const OC_UNICODE_BRAILLE_PATTERNS: UnicodeRange;
    extern const OC_UNICODE_SUPPLEMENTAL_ARROWS_B: UnicodeRange;
    extern const OC_UNICODE_MISCELLANEOUS_MATHEMATICAL_SYMBOLS_B: UnicodeRange;
    extern const OC_UNICODE_SUPPLEMENTAL_MATHEMATICAL_OPERATORS: UnicodeRange;
    extern const OC_UNICODE_MISCELLANEOUS_SYMBOLS_AND_ARROWS: UnicodeRange;
    extern const OC_UNICODE_CJK_RADICALS_SUPPLEMENT: UnicodeRange;
    extern const OC_UNICODE_KANGXI_RADICALS: UnicodeRange;
    extern const OC_UNICODE_IDEOGRAPHIC_DESCRIPTION_CHARACTERS: UnicodeRange;
    extern const OC_UNICODE_CJK_SYMBOLS_AND_PUNCTUATION: UnicodeRange;
    extern const OC_UNICODE_HIRAGANA: UnicodeRange;
    extern const OC_UNICODE_KATAKANA: UnicodeRange;
    extern const OC_UNICODE_BOPOMOFO: UnicodeRange;
    extern const OC_UNICODE_HANGUL_COMPATIBILITY_JAMO: UnicodeRange;
    extern const OC_UNICODE_KANBUN_KUNTEN: UnicodeRange;
    extern const OC_UNICODE_BOPOMOFO_EXTENDED: UnicodeRange;
    extern const OC_UNICODE_KATAKANA_PHONETIC_EXTENSIONS: UnicodeRange;
    extern const OC_UNICODE_ENCLOSED_CJK_LETTERS_AND_MONTHS: UnicodeRange;
    extern const OC_UNICODE_CJK_COMPATIBILITY: UnicodeRange;
    extern const OC_UNICODE_CJK_UNIFIED_IDEOGRAPHS_EXTENSION_A: UnicodeRange;
    extern const OC_UNICODE_YIJING_HEXAGRAM_SYMBOLS: UnicodeRange;
    extern const OC_UNICODE_CJK_UNIFIED_IDEOGRAPHS: UnicodeRange;
    extern const OC_UNICODE_YI_SYLLABLES: UnicodeRange;
    extern const OC_UNICODE_YI_RADICALS: UnicodeRange;
    extern const OC_UNICODE_HANGUL_SYLLABLES: UnicodeRange;
    extern const OC_UNICODE_HIGH_SURROGATE_AREA: UnicodeRange;
    extern const OC_UNICODE_LOW_SURROGATE_AREA: UnicodeRange;
    extern const OC_UNICODE_PRIVATE_USE_AREA: UnicodeRange;
    extern const OC_UNICODE_CJK_COMPATIBILITY_IDEOGRAPHS: UnicodeRange;
    extern const OC_UNICODE_ALPHABETIC_PRESENTATION_FORMS: UnicodeRange;
    extern const OC_UNICODE_ARABIC_PRESENTATION_FORMS_A: UnicodeRange;
    extern const OC_UNICODE_VARIATION_SELECTORS: UnicodeRange;
    extern const OC_UNICODE_COMBINING_HALF_MARKS: UnicodeRange;
    extern const OC_UNICODE_CJK_COMPATIBILITY_FORMS: UnicodeRange;
    extern const OC_UNICODE_SMALL_FORM_VARIANTS: UnicodeRange;
    extern const OC_UNICODE_ARABIC_PRESENTATION_FORMS_B: UnicodeRange;
    extern const OC_UNICODE_HALFWIDTH_AND_FULLWIDTH_FORMS: UnicodeRange;
    extern const OC_UNICODE_SPECIALS: UnicodeRange;
    extern const OC_UNICODE_LINEAR_B_SYLLABARY: UnicodeRange;
    extern const OC_UNICODE_LINEAR_B_IDEOGRAMS: UnicodeRange;
    extern const OC_UNICODE_AEGEAN_NUMBERS: UnicodeRange;
    extern const OC_UNICODE_OLD_ITALIC: UnicodeRange;
    extern const OC_UNICODE_GOTHIC: UnicodeRange;
    extern const OC_UNICODE_UGARITIC: UnicodeRange;
    extern const OC_UNICODE_DESERET: UnicodeRange;
    extern const OC_UNICODE_SHAVIAN: UnicodeRange;
    extern const OC_UNICODE_OSMANYA: UnicodeRange;
    extern const OC_UNICODE_CYPRIOT_SYLLABARY: UnicodeRange;
    extern const OC_UNICODE_BYZANTINE_MUSICAL_SYMBOLS: UnicodeRange;
    extern const OC_UNICODE_MUSICAL_SYMBOLS: UnicodeRange;
    extern const OC_UNICODE_TAI_XUAN_JING_SYMBOLS: UnicodeRange;
    extern const OC_UNICODE_MATHEMATICAL_ALPHANUMERIC_SYMBOLS: UnicodeRange;
    extern const OC_UNICODE_CJK_UNIFIED_IDEOGRAPHS_EXTENSION_B: UnicodeRange;
    extern const OC_UNICODE_CJK_COMPATIBILITY_IDEOGRAPHS_SUPPLEMENT: UnicodeRange;
    extern const OC_UNICODE_TAGS: UnicodeRange;
    extern const OC_UNICODE_VARIATION_SELECTORS_SUPPLEMENT: UnicodeRange;
    extern const OC_UNICODE_SUPPLEMENTARY_PRIVATE_USE_AREA_A: UnicodeRange;
    extern const OC_UNICODE_SUPPLEMENTARY_PRIVATE_USE_AREA_B: UnicodeRange;

    pub const Enum = enum {
        BasicLatin,
        C1ControlsAndLatin1Supplement,
        LatinExtendedA,
        LatinExtendedB,
        IpaExtensions,
        SpacingModifierLetters,
        CombiningDiacriticalMarks,
        GreekCoptic,
        Cyrillic,
        CyrillicSupplement,
        Armenian,
        Hebrew,
        Arabic,
        Syriac,
        Thaana,
        Devanagari,
        BengaliAssamese,
        Gurmukhi,
        Gujarati,
        Oriya,
        Tamil,
        Telugu,
        Kannada,
        Malayalam,
        Sinhala,
        Thai,
        Lao,
        Tibetan,
        Myanmar,
        Georgian,
        HangulJamo,
        Ethiopic,
        Cherokee,
        UnifiedCanadianAboriginalSyllabics,
        Ogham,
        Runic,
        Tagalog,
        Hanunoo,
        Buhid,
        Tagbanwa,
        Khmer,
        Mongolian,
        Limbu,
        TaiLe,
        KhmerSymbols,
        PhoneticExtensions,
        LatinExtendedAdditional,
        GreekExtended,
        GeneralPunctuation,
        SuperscriptsAndSubscripts,
        CurrencySymbols,
        CombiningDiacriticalMarksForSymbols,
        LetterlikeSymbols,
        NumberForms,
        Arrows,
        MathematicalOperators,
        MiscellaneousTechnical,
        ControlPictures,
        OpticalCharacterRecognition,
        EnclosedAlphanumerics,
        BoxDrawing,
        BlockElements,
        GeometricShapes,
        MiscellaneousSymbols,
        Dingbats,
        MiscellaneousMathematicalSymbolsA,
        SupplementalArrowsA,
        BraillePatterns,
        SupplementalArrowsB,
        MiscellaneousMathematicalSymbolsB,
        SupplementalMathematicalOperators,
        MiscellaneousSymbolsAndArrows,
        CjkRadicalsSupplement,
        KangxiRadicals,
        IdeographicDescriptionCharacters,
        CjkSymbolsAndPunctuation,
        Hiragana,
        Katakana,
        Bopomofo,
        HangulCompatibilityJamo,
        KanbunKunten,
        BopomofoExtended,
        KatakanaPhoneticExtensions,
        EnclosedCjkLettersAndMonths,
        CjkCompatibility,
        CjkUnifiedIdeographsExtensionA,
        YijingHexagramSymbols,
        CjkUnifiedIdeographs,
        YiSyllables,
        YiRadicals,
        HangulSyllables,
        HighSurrogateArea,
        LowSurrogateArea,
        PrivateUseArea,
        CjkCompatibilityIdeographs,
        AlphabeticPresentationForms,
        ArabicPresentationFormsA,
        VariationSelectors,
        CombiningHalfMarks,
        CjkCompatibilityForms,
        SmallFormVariants,
        ArabicPresentationFormsB,
        HalfwidthAndFullwidthForms,
        Specials,
        LinearBSyllabary,
        LinearBIdeograms,
        AegeanNumbers,
        OldItalic,
        Gothic,
        Ugaritic,
        Deseret,
        Shavian,
        Osmanya,
        CypriotSyllabary,
        ByzantineMusicalSymbols,
        MusicalSymbols,
        TaiXuanJingSymbols,
        MathematicalAlphanumericSymbols,
        CjkUnifiedIdeographsExtensionB,
        CjkCompatibilityIdeographsSupplement,
        Tags,
        VariationSelectorsSupplement,
        SupplementaryPrivateUseAreaA,
        SupplementaryPrivateUseAreaB,
    };

    pub fn range(comptime enums: []const Enum) [enums.len]UnicodeRange {
        var r: [enums.len]UnicodeRange = undefined;
        for (enums, 0..r.len) |e, i| {
            r[i] = switch (e) {
                .BasicLatin => OC_UNICODE_BASIC_LATIN,
                .C1ControlsAndLatin1Supplement => OC_UNICODE_C1_CONTROLS_AND_LATIN_1_SUPPLEMENT,
                .LatinExtendedA => OC_UNICODE_LATIN_EXTENDED_A,
                .LatinExtendedB => OC_UNICODE_LATIN_EXTENDED_B,
                .IpaExtensions => OC_UNICODE_IPA_EXTENSIONS,
                .SpacingModifierLetters => OC_UNICODE_SPACING_MODIFIER_LETTERS,
                .CombiningDiacriticalMarks => OC_UNICODE_COMBINING_DIACRITICAL_MARKS,
                .GreekCoptic => OC_UNICODE_GREEK_COPTIC,
                .Cyrillic => OC_UNICODE_CYRILLIC,
                .CyrillicSupplement => OC_UNICODE_CYRILLIC_SUPPLEMENT,
                .Armenian => OC_UNICODE_ARMENIAN,
                .Hebrew => OC_UNICODE_HEBREW,
                .Arabic => OC_UNICODE_ARABIC,
                .Syriac => OC_UNICODE_SYRIAC,
                .Thaana => OC_UNICODE_THAANA,
                .Devanagari => OC_UNICODE_DEVANAGARI,
                .BengaliAssamese => OC_UNICODE_BENGALI_ASSAMESE,
                .Gurmukhi => OC_UNICODE_GURMUKHI,
                .Gujarati => OC_UNICODE_GUJARATI,
                .Oriya => OC_UNICODE_ORIYA,
                .Tamil => OC_UNICODE_TAMIL,
                .Telugu => OC_UNICODE_TELUGU,
                .Kannada => OC_UNICODE_KANNADA,
                .Malayalam => OC_UNICODE_MALAYALAM,
                .Sinhala => OC_UNICODE_SINHALA,
                .Thai => OC_UNICODE_THAI,
                .Lao => OC_UNICODE_LAO,
                .Tibetan => OC_UNICODE_TIBETAN,
                .Myanmar => OC_UNICODE_MYANMAR,
                .Georgian => OC_UNICODE_GEORGIAN,
                .HangulJamo => OC_UNICODE_HANGUL_JAMO,
                .Ethiopic => OC_UNICODE_ETHIOPIC,
                .Cherokee => OC_UNICODE_CHEROKEE,
                .UnifiedCanadianAboriginalSyllabics => OC_UNICODE_UNIFIED_CANADIAN_ABORIGINAL_SYLLABICS,
                .Ogham => OC_UNICODE_OGHAM,
                .Runic => OC_UNICODE_RUNIC,
                .Tagalog => OC_UNICODE_TAGALOG,
                .Hanunoo => OC_UNICODE_HANUNOO,
                .Buhid => OC_UNICODE_BUHID,
                .Tagbanwa => OC_UNICODE_TAGBANWA,
                .Khmer => OC_UNICODE_KHMER,
                .Mongolian => OC_UNICODE_MONGOLIAN,
                .Limbu => OC_UNICODE_LIMBU,
                .TaiLe => OC_UNICODE_TAI_LE,
                .KhmerSymbols => OC_UNICODE_KHMER_SYMBOLS,
                .PhoneticExtensions => OC_UNICODE_PHONETIC_EXTENSIONS,
                .LatinExtendedAdditional => OC_UNICODE_LATIN_EXTENDED_ADDITIONAL,
                .GreekExtended => OC_UNICODE_GREEK_EXTENDED,
                .GeneralPunctuation => OC_UNICODE_GENERAL_PUNCTUATION,
                .SuperscriptsAndSubscripts => OC_UNICODE_SUPERSCRIPTS_AND_SUBSCRIPTS,
                .CurrencySymbols => OC_UNICODE_CURRENCY_SYMBOLS,
                .CombiningDiacriticalMarksForSymbols => OC_UNICODE_COMBINING_DIACRITICAL_MARKS_FOR_SYMBOLS,
                .LetterlikeSymbols => OC_UNICODE_LETTERLIKE_SYMBOLS,
                .NumberForms => OC_UNICODE_NUMBER_FORMS,
                .Arrows => OC_UNICODE_ARROWS,
                .MathematicalOperators => OC_UNICODE_MATHEMATICAL_OPERATORS,
                .MiscellaneousTechnical => OC_UNICODE_MISCELLANEOUS_TECHNICAL,
                .ControlPictures => OC_UNICODE_CONTROL_PICTURES,
                .OpticalCharacterRecognition => OC_UNICODE_OPTICAL_CHARACTER_RECOGNITION,
                .EnclosedAlphanumerics => OC_UNICODE_ENCLOSED_ALPHANUMERICS,
                .BoxDrawing => OC_UNICODE_BOX_DRAWING,
                .BlockElements => OC_UNICODE_BLOCK_ELEMENTS,
                .GeometricShapes => OC_UNICODE_GEOMETRIC_SHAPES,
                .MiscellaneousSymbols => OC_UNICODE_MISCELLANEOUS_SYMBOLS,
                .Dingbats => OC_UNICODE_DINGBATS,
                .MiscellaneousMathematicalSymbolsA => OC_UNICODE_MISCELLANEOUS_MATHEMATICAL_SYMBOLS_A,
                .SupplementalArrowsA => OC_UNICODE_SUPPLEMENTAL_ARROWS_A,
                .BraillePatterns => OC_UNICODE_BRAILLE_PATTERNS,
                .SupplementalArrowsB => OC_UNICODE_SUPPLEMENTAL_ARROWS_B,
                .MiscellaneousMathematicalSymbolsB => OC_UNICODE_MISCELLANEOUS_MATHEMATICAL_SYMBOLS_B,
                .SupplementalMathematicalOperators => OC_UNICODE_SUPPLEMENTAL_MATHEMATICAL_OPERATORS,
                .MiscellaneousSymbolsAndArrows => OC_UNICODE_MISCELLANEOUS_SYMBOLS_AND_ARROWS,
                .CjkRadicalsSupplement => OC_UNICODE_CJK_RADICALS_SUPPLEMENT,
                .KangxiRadicals => OC_UNICODE_KANGXI_RADICALS,
                .IdeographicDescriptionCharacters => OC_UNICODE_IDEOGRAPHIC_DESCRIPTION_CHARACTERS,
                .CjkSymbolsAndPunctuation => OC_UNICODE_CJK_SYMBOLS_AND_PUNCTUATION,
                .Hiragana => OC_UNICODE_HIRAGANA,
                .Katakana => OC_UNICODE_KATAKANA,
                .Bopomofo => OC_UNICODE_BOPOMOFO,
                .HangulCompatibilityJamo => OC_UNICODE_HANGUL_COMPATIBILITY_JAMO,
                .KanbunKunten => OC_UNICODE_KANBUN_KUNTEN,
                .BopomofoExtended => OC_UNICODE_BOPOMOFO_EXTENDED,
                .KatakanaPhoneticExtensions => OC_UNICODE_KATAKANA_PHONETIC_EXTENSIONS,
                .EnclosedCjkLettersAndMonths => OC_UNICODE_ENCLOSED_CJK_LETTERS_AND_MONTHS,
                .CjkCompatibility => OC_UNICODE_CJK_COMPATIBILITY,
                .CjkUnifiedIdeographsExtensionA => OC_UNICODE_CJK_UNIFIED_IDEOGRAPHS_EXTENSION_A,
                .YijingHexagramSymbols => OC_UNICODE_YIJING_HEXAGRAM_SYMBOLS,
                .CjkUnifiedIdeographs => OC_UNICODE_CJK_UNIFIED_IDEOGRAPHS,
                .YiSyllables => OC_UNICODE_YI_SYLLABLES,
                .YiRadicals => OC_UNICODE_YI_RADICALS,
                .HangulSyllables => OC_UNICODE_HANGUL_SYLLABLES,
                .HighSurrogateArea => OC_UNICODE_HIGH_SURROGATE_AREA,
                .LowSurrogateArea => OC_UNICODE_LOW_SURROGATE_AREA,
                .PrivateUseArea => OC_UNICODE_PRIVATE_USE_AREA,
                .CjkCompatibilityIdeographs => OC_UNICODE_CJK_COMPATIBILITY_IDEOGRAPHS,
                .AlphabeticPresentationForms => OC_UNICODE_ALPHABETIC_PRESENTATION_FORMS,
                .ArabicPresentationFormsA => OC_UNICODE_ARABIC_PRESENTATION_FORMS_A,
                .VariationSelectors => OC_UNICODE_VARIATION_SELECTORS,
                .CombiningHalfMarks => OC_UNICODE_COMBINING_HALF_MARKS,
                .CjkCompatibilityForms => OC_UNICODE_CJK_COMPATIBILITY_FORMS,
                .SmallFormVariants => OC_UNICODE_SMALL_FORM_VARIANTS,
                .ArabicPresentationFormsB => OC_UNICODE_ARABIC_PRESENTATION_FORMS_B,
                .HalfwidthAndFullwidthForms => OC_UNICODE_HALFWIDTH_AND_FULLWIDTH_FORMS,
                .Specials => OC_UNICODE_SPECIALS,
                .LinearBSyllabary => OC_UNICODE_LINEAR_B_SYLLABARY,
                .LinearBIdeograms => OC_UNICODE_LINEAR_B_IDEOGRAMS,
                .AegeanNumbers => OC_UNICODE_AEGEAN_NUMBERS,
                .OldItalic => OC_UNICODE_OLD_ITALIC,
                .Gothic => OC_UNICODE_GOTHIC,
                .Ugaritic => OC_UNICODE_UGARITIC,
                .Deseret => OC_UNICODE_DESERET,
                .Shavian => OC_UNICODE_SHAVIAN,
                .Osmanya => OC_UNICODE_OSMANYA,
                .CypriotSyllabary => OC_UNICODE_CYPRIOT_SYLLABARY,
                .ByzantineMusicalSymbols => OC_UNICODE_BYZANTINE_MUSICAL_SYMBOLS,
                .MusicalSymbols => OC_UNICODE_MUSICAL_SYMBOLS,
                .TaiXuanJingSymbols => OC_UNICODE_TAI_XUAN_JING_SYMBOLS,
                .MathematicalAlphanumericSymbols => OC_UNICODE_MATHEMATICAL_ALPHANUMERIC_SYMBOLS,
                .CjkUnifiedIdeographsExtensionB => OC_UNICODE_CJK_UNIFIED_IDEOGRAPHS_EXTENSION_B,
                .CjkCompatibilityIdeographsSupplement => OC_UNICODE_CJK_COMPATIBILITY_IDEOGRAPHS_SUPPLEMENT,
                .Tags => OC_UNICODE_TAGS,
                .VariationSelectorsSupplement => OC_UNICODE_VARIATION_SELECTORS_SUPPLEMENT,
                .SupplementaryPrivateUseAreaA => OC_UNICODE_SUPPLEMENTARY_PRIVATE_USE_AREA_A,
                .SupplementaryPrivateUseAreaB => OC_UNICODE_SUPPLEMENTARY_PRIVATE_USE_AREA_B,
            };
        }

        return r;
    }
};

//------------------------------------------------------------------------------------------
// [FONT]: fonts
//------------------------------------------------------------------------------------------

pub const Font = extern struct {
    h: u64,

    extern fn oc_font_nil() Font;
    extern fn oc_font_is_nil(font: Font) bool;

    extern fn oc_font_create_from_memory(mem: Str8, range_count: u32, ranges: [*]const UnicodeRange) Font;
    extern fn oc_font_create_from_file(file: File, range_count: u32, ranges: [*]const UnicodeRange) Font;
    extern fn oc_font_create_from_path(path: Str8, range_count: u32, ranges: [*]const UnicodeRange) Font;

    extern fn oc_font_get_glyph_indices(font: Font, code_points: Str32, backing: Str32) Str32;
    extern fn oc_font_push_glyph_indices(font: Font, arena: *Arena, code_points: Str32) Str32;
    extern fn oc_font_get_glyph_index(font: Font, code_point: Utf32) u32;
    extern fn oc_font_destroy(font: Font) void;

    extern fn oc_font_get_metrics(font: Font) FontMetrics;
    extern fn oc_font_get_metrics_unscaled(font: Font, em_size: f32) FontMetrics;
    extern fn oc_font_get_scale_for_em_pixels(font: Font, em_size: f32) f32;
    extern fn oc_font_text_metrics_utf32(font: Font, font_size: f32, code_points: Str32) TextMetrics;
    extern fn oc_font_text_metrics(font: Font, font_size: f32, text: Str8) TextMetrics;

    pub const nil = oc_font_nil;
    pub const isNil = oc_font_is_nil;
    pub fn createFromMemory(mem: []const u8, ranges: []const UnicodeRange) Font {
        return oc_font_create_from_memory(Str8.fromSlice(mem), @intCast(ranges.len), ranges.ptr);
    }
    pub fn createFromFile(file: File, ranges: []const UnicodeRange) Font {
        return oc_font_create_from_file(file, @intCast(ranges.len), ranges.ptr);
    }
    pub fn createFromPath(path: []const u8, ranges: []const UnicodeRange) Font {
        return oc_font_create_from_path(Str8.fromSlice(path), @intCast(ranges.len), ranges.ptr);
    }
    pub const getGlyphIndices = oc_font_get_glyph_indices;
    pub const pushGlyphIndices = oc_font_push_glyph_indices;
    pub const getGlyphIndex = oc_font_get_glyph_index;
    pub const destroy = oc_font_destroy;
    pub const getMetrics = oc_font_get_metrics;
    pub const getMetricsUnscaled = oc_font_get_metrics_unscaled;
    pub const getScaleForEmPixels = oc_font_get_scale_for_em_pixels;
    pub const textMetricsUtf32 = oc_font_text_metrics_utf32;
    pub const textMetrics = oc_font_text_metrics;
};

pub const JointType = enum(c_uint) {
    Miter,
    Bevel,
    None,
};

pub const CapType = enum(c_uint) {
    None,
    Square,
};

pub const FontMetrics = extern struct {
    ascent: f32, // the extent above the baseline (by convention a positive value extends above the baseline)
    descent: f32, // the extent below the baseline (by convention, positive value extends below the baseline)
    line_gap: f32, // spacing between one row's descent and the next row's ascent
    x_height: f32, // height of the lower case letter 'x'
    cap_height: f32, // height of the upper case letter 'M'
    width: f32, // maximum width of the font
};

pub const GlyphMetrics = extern struct {
    ink: Rect,
    advance: Vec2,
};

pub const TextMetrics = extern struct {
    ink: Rect,
    logical: Rect,
    advance: Vec2,
};

//------------------------------------------------------------------------------------------
// [APP]
//------------------------------------------------------------------------------------------

extern fn oc_request_quit() void;
pub const requestQuit = oc_request_quit;

//------------------------------------------------------------------------------------------
// [APP] input
//------------------------------------------------------------------------------------------

const MouseCursor = enum(c_uint) {
    Arrow,
    Resize0,
    Resize90,
    Resize45,
    Resize135,
    Text,
};
extern fn oc_set_cursor(cursor: MouseCursor) void;
pub const setCursor = oc_set_cursor;

pub const ScanCode = enum(c_uint) {
    Unknown = 0,
    Space = 32,
    Apostrophe = 39,
    Comma = 44,
    Minus = 45,
    Period = 46,
    Slash = 47,
    Num0 = 48,
    Num1 = 49,
    Num2 = 50,
    Num3 = 51,
    Num4 = 52,
    Num5 = 53,
    Num6 = 54,
    Num7 = 55,
    Num8 = 56,
    Num9 = 57,
    Semicolon = 59,
    Equal = 61,
    LeftBracket = 91,
    Backslash = 92,
    RightBracket = 93,
    GraveAccent = 96,
    A = 97,
    B = 98,
    C = 99,
    D = 100,
    E = 101,
    F = 102,
    G = 103,
    H = 104,
    I = 105,
    J = 106,
    K = 107,
    L = 108,
    M = 109,
    N = 110,
    O = 111,
    P = 112,
    Q = 113,
    R = 114,
    S = 115,
    T = 116,
    U = 117,
    V = 118,
    W = 119,
    X = 120,
    Y = 121,
    Z = 122,
    World1 = 161,
    World2 = 162,
    Escape = 256,
    Enter = 257,
    Tab = 258,
    Backspace = 259,
    Insert = 260,
    Delete = 261,
    Right = 262,
    Left = 263,
    Down = 264,
    Up = 265,
    PageUp = 266,
    PageDown = 267,
    Home = 268,
    End = 269,
    CapsLock = 280,
    ScrollLock = 281,
    NumLock = 282,
    PrintScreen = 283,
    Pause = 284,
    F1 = 290,
    F2 = 291,
    F3 = 292,
    F4 = 293,
    F5 = 294,
    F6 = 295,
    F7 = 296,
    F8 = 297,
    F9 = 298,
    F10 = 299,
    F11 = 300,
    F12 = 301,
    F13 = 302,
    F14 = 303,
    F15 = 304,
    F16 = 305,
    F17 = 306,
    F18 = 307,
    F19 = 308,
    F20 = 309,
    F21 = 310,
    F22 = 311,
    F23 = 312,
    F24 = 313,
    F25 = 314,
    Kp0 = 320,
    Kp1 = 321,
    Kp2 = 322,
    Kp3 = 323,
    Kp4 = 324,
    Kp5 = 325,
    Kp6 = 326,
    Kp7 = 327,
    Kp8 = 328,
    Kp9 = 329,
    KpDecimal = 330,
    KpDivide = 331,
    KpMultiply = 332,
    KpSubtract = 333,
    KpAdd = 334,
    KpEnter = 335,
    KpEqual = 336,
    LeftShift = 340,
    LeftControl = 341,
    LeftAlt = 342,
    LeftSuper = 343,
    RightShift = 344,
    RightControl = 345,
    RightAlt = 346,
    RightSuper = 347,
    Menu = 348,
};

pub const KeyCode = enum(c_uint) {
    Unknown = 0,
    Space = ' ',
    Apostrophe = '\'',
    Comma = ',',
    Minus = '-',
    Period = '.',
    Slash = '/',
    Num0 = '0',
    Num1 = '1',
    Num2 = '2',
    Num3 = '3',
    Num4 = '4',
    Num5 = '5',
    Num6 = '6',
    Num7 = '7',
    Num8 = '8',
    Num9 = '9',
    Semicolon = ';',
    Equal = '=',
    LeftBracket = '[',
    Backslash = '\\',
    RightBracket = ']',
    GraveAccent = '`',
    A = 'a',
    B = 'b',
    C = 'c',
    D = 'd',
    E = 'e',
    F = 'f',
    G = 'g',
    H = 'h',
    I = 'i',
    J = 'j',
    K = 'k',
    L = 'l',
    M = 'm',
    N = 'n',
    O = 'o',
    P = 'p',
    Q = 'q',
    R = 'r',
    S = 's',
    T = 't',
    U = 'u',
    V = 'v',
    W = 'w',
    X = 'x',
    Y = 'y',
    Z = 'z',
    World1 = 161,
    World2 = 162,
    Escape = 256,
    Enter = 257,
    Tab = 258,
    Backspace = 259,
    Insert = 260,
    Delete = 261,
    Right = 262,
    Left = 263,
    Down = 264,
    Up = 265,
    PageUp = 266,
    PageDown = 267,
    Home = 268,
    End = 269,
    CapsLock = 280,
    ScrollLock = 281,
    NumLock = 282,
    PrintScreen = 283,
    Pause = 284,
    F1 = 290,
    F2 = 291,
    F3 = 292,
    F4 = 293,
    F5 = 294,
    F6 = 295,
    F7 = 296,
    F8 = 297,
    F9 = 298,
    F10 = 299,
    F11 = 300,
    F12 = 301,
    F13 = 302,
    F14 = 303,
    F15 = 304,
    F16 = 305,
    F17 = 306,
    F18 = 307,
    F19 = 308,
    F20 = 309,
    F21 = 310,
    F22 = 311,
    F23 = 312,
    F24 = 313,
    F25 = 314,
    Kp0 = 320,
    Kp1 = 321,
    Kp2 = 322,
    Kp3 = 323,
    Kp4 = 324,
    Kp5 = 325,
    Kp6 = 326,
    Kp7 = 327,
    Kp8 = 328,
    Kp9 = 329,
    KpDecimal = 330,
    KpDivide = 331,
    KpMultiply = 332,
    KpSubtract = 333,
    KpAdd = 334,
    KpEnter = 335,
    KpEqual = 336,
    LeftShift = 340,
    LeftControl = 341,
    LeftAlt = 342,
    LeftSuper = 343,
    RightShift = 344,
    RightControl = 345,
    RightAlt = 346,
    RightSuper = 347,
    Menu = 348,
    Count = 349,
};

pub const MouseButton = enum(c_uint) {
    Left = 0x00,
    Right = 0x01,
    Middle = 0x02,
    Ext1 = 0x03,
    Ext2 = 0x04,
};

//------------------------------------------------------------------------------------------
// [APP] windows
//------------------------------------------------------------------------------------------

extern fn oc_window_set_title(title: Str8) void;

pub fn windowSetTitle(title: [:0]const u8) void {
    // var title_str8: Str8 = .{
    //     .ptr = @constCast(title.ptr),
    //     .len = title.len,
    // };
    oc_window_set_title(Str8.fromSlice(title));
}

extern fn oc_window_set_size(size: Vec2) void;
// pub const windowSetTitle = oc_window_set_title;
pub const windowSetSize = oc_window_set_size;

//------------------------------------------------------------------------------------------
// [APP] file dialogs
//------------------------------------------------------------------------------------------

const FileDialogKind = enum(c_uint) {
    Save,
    Open,
};

const FileDialogFlags = packed struct(u32) {
    files: bool,
    directories: bool,
    multiple: bool,
    create_directories: bool,
};

const FileDialogDesc = extern struct {
    kind: FileDialogKind,
    flags: FileDialogFlags,
    title: Str8,
    ok_label: Str8,
    start_at: File,
    start_path: Str8,
    filters: Str8List,
};

const FileDialogButton = enum(c_uint) {
    Cancel,
    Ok,
};

const FileDialogResult = extern struct {
    button: FileDialogButton,
    path: Str8,
    selection: Str8List,
};

//------------------------------------------------------------------------------------------
// [CLOCK]
//------------------------------------------------------------------------------------------

pub const ClockKind = enum(c_uint) {
    Monotonic,
    Uptime,
    Date,
};

extern fn oc_clock_time(clock: ClockKind) f64;
pub const clockTime = oc_clock_time;

//------------------------------------------------------------------------------------------
// [MATH] Vec2
//------------------------------------------------------------------------------------------

pub const Vec2 = extern struct {
    x: f32,
    y: f32,

    pub fn mul(self: Vec2, scalar: f32) Vec2 {
        return .{
            .x = self.x * scalar,
            .y = self.y * scalar,
        };
    }

    pub fn add(a: Vec2, b: Vec2) Vec2 {
        return .{
            .x = a.x + b.x,
            .y = a.y + b.y,
        };
    }
};

//------------------------------------------------------------------------------------------
// [MATH] Matrix
//------------------------------------------------------------------------------------------

pub const Mat2x3 = extern struct {
    m: [6]f32,

    extern fn oc_mat2x3_mul(m: Mat2x3, p: Vec2) Vec2;
    extern fn oc_mat2x3_mul_m(lhs: Mat2x3, rhs: Mat2x3) Mat2x3;
    extern fn oc_mat2x3_inv(x: Mat2x3) Mat2x3;
    extern fn oc_mat2x3_rotate(radians: f32) Mat2x3;
    extern fn oc_mat2x3_translate(x: f32, y: f32) Mat2x3;

    extern fn oc_matrix_push(matrix: Mat2x3) void;
    extern fn oc_matrix_pop() void;
    extern fn oc_matrix_top() Mat2x3;

    pub const mul = oc_mat2x3_mul;
    pub const mul_m = oc_mat2x3_mul_m;
    pub const inv = oc_mat2x3_inv;
    pub const rotate = oc_mat2x3_rotate;
    pub const translate = oc_mat2x3_translate;
    pub fn scale(x: f32, y: f32) Mat2x3 {
        return .{ .m = .{ x, 0, 0, 0, y, 0 } };
    }
    pub fn scaleUniform(s: f32) Mat2x3 {
        return .{ .m = .{ s, 0, 0, 0, s, 0 } };
    }

    pub const push = oc_matrix_push;
    pub const pop = oc_matrix_pop;
    pub const top = oc_matrix_top;
};

//------------------------------------------------------------------------------------------
// [GRAPHICS] Clip stack
//------------------------------------------------------------------------------------------

pub const clip = struct {
    extern fn oc_clip_push(x: f32, y: f32, w: f32, h: f32) void;
    extern fn oc_clip_pop() void;
    extern fn oc_clip_top() Rect;

    pub const push = oc_clip_push;
    pub const pop = oc_clip_pop;
    pub const top = oc_clip_top;
};

//------------------------------------------------------------------------------------------
// [GRAPHICS]: resources
//------------------------------------------------------------------------------------------

pub const Surface = extern struct {
    h: u64,

    extern fn oc_surface_nil(void) Surface;
    extern fn oc_surface_is_nil(surface: Surface) bool;
    extern fn oc_surface_canvas() Surface;
    extern fn oc_surface_gles() Surface;
    extern fn oc_surface_select(surface: Surface) void;
    extern fn oc_surface_present(surface: Surface) void;
    extern fn oc_surface_get_size(surface: Surface) Vec2;
    extern fn oc_surface_contents_scaling(surface: Surface) Vec2;
    extern fn oc_surface_bring_to_front(surface: Surface) void;
    extern fn oc_surface_send_to_back(surface: Surface) void;
    // TODO
    // extern fn oc_surface_render_commands(
    //     surface: Surface,
    //     color: Color,
    //     primitive_count: u32,
    //     primitives: [*]Primitive,
    //     elt_count: u32,
    //     elements: [*]PathElt,
    // ) void;

    pub const nil = oc_surface_nil;
    pub const isNil = oc_surface_is_nil;
    pub const canvas = oc_surface_canvas;
    pub const gles = oc_surface_gles;
    pub const select = oc_surface_select;
    pub const present = oc_surface_present;
    pub const getSize = oc_surface_get_size;
    pub const contentsScaling = oc_surface_contents_scaling;
    pub const bringToFront = oc_surface_bring_to_front;
    pub const sendToBack = oc_surface_send_to_back;
    // pub const renderCommands = oc_surface_render_commands;
};

pub const Canvas = extern struct {
    h: u64,

    extern fn oc_canvas_nil() Canvas;
    extern fn oc_canvas_is_nil(canvas: Canvas) bool;
    extern fn oc_canvas_create() Canvas;
    extern fn oc_canvas_destroy(canvas: Canvas) void;
    extern fn oc_canvas_select(canvas: Canvas) Canvas;
    extern fn oc_render(canvas: Canvas) void;

    pub const nil = oc_canvas_nil;
    pub const isNil = oc_canvas_is_nil;
    pub const create = oc_canvas_create;
    pub const destroy = oc_canvas_destroy;
    pub const select = oc_canvas_select;
    pub const render = oc_render;
};

pub const Image = extern struct {
    h: u64,

    extern fn oc_image_nil(void) Image;
    extern fn oc_image_is_nil(image: Image) bool;
    extern fn oc_image_create(surface: Surface, width: u32, height: u32) Image;
    extern fn oc_image_create_from_rgba8(surface: Surface, width: u32, height: u32, pixels: [*]u8) Image;
    extern fn oc_image_create_from_memory(surface: Surface, mem: Str8, flip: bool) Image;
    extern fn oc_image_create_from_file(surface: Surface, file: File, flip: bool) Image;
    extern fn oc_image_create_from_path(surface: Surface, path: Str8, flip: bool) Image;
    extern fn oc_image_destroy(image: Image) void;
    extern fn oc_image_upload_region_rgba8(image: Image, region: Rect, pixels: [*]u8) void;
    extern fn oc_image_size(image: Image) Vec2;
    extern fn oc_image_draw(image: Image, rect: Rect) void;
    extern fn oc_image_draw_region(image: Image, srcRegion: Rect, dstRegion: Rect) void;

    pub const nil = oc_image_nil;
    pub const isNil = oc_image_is_nil;
    pub const create = oc_image_create;
    pub const createFromRgba8 = oc_image_create_from_rgba8;
    pub const createFromMemory = oc_image_create_from_memory;
    pub const createFromFile = oc_image_create_from_file;
    pub const createFromPath = oc_image_create_from_path;
    pub const destroy = oc_image_destroy;
    pub const uploadRegionRgba8 = oc_image_upload_region_rgba8;
    pub const size = oc_image_size;
    pub const draw = oc_image_draw;
    pub const drawRegion = oc_image_draw_region;
};

pub const Rect = extern union {
    Flat: extern struct {
        x: f32,
        y: f32,
        w: f32,
        h: f32,
    },
    Pairs: extern struct {
        xy: Vec2,
        wh: Vec2,
    },
    Array: [4]f32,

    pub fn xywh(x: f32, y: f32, w: f32, h: f32) Rect {
        return .{ .Flat = .{ .x = x, .y = y, .w = w, .h = h } };
    }
};

pub const Color = extern union {
    Flat: extern struct {
        r: f32,
        g: f32,
        b: f32,
        a: f32,
    },
    Array: [4]f32,
};

//------------------------------------------------------------------------------------------
// [GRAPHICS]: graphics attributes setting/getting
//------------------------------------------------------------------------------------------

extern fn oc_set_color(color: Color) void;
extern fn oc_set_color_rgba(r: f32, g: f32, b: f32, a: f32) void;
extern fn oc_set_width(width: f32) void;
extern fn oc_set_tolerance(tolerance: f32) void;
extern fn oc_set_joint(joint: JointType) void;
extern fn oc_set_max_joint_excursion(max_joint_excursion: f32) void;
extern fn oc_set_cap(cap: CapType) void;
extern fn oc_set_font(font: Font) void;
extern fn oc_set_font_size(size: f32) void;
extern fn oc_set_text_flip(flip: bool) void;
extern fn oc_set_image(image: Image) void;
extern fn oc_set_image_source_region(region: Rect) void;

extern fn oc_get_color() Color;
extern fn oc_get_width() f32;
extern fn oc_get_tolerance() f32;
extern fn oc_get_joint() JointType;
extern fn oc_get_max_joint_excursion() f32;
extern fn oc_get_cap() CapType;
extern fn oc_get_font() Font;
extern fn oc_get_font_size() f32;
extern fn oc_get_text_flip() bool;
extern fn oc_get_image() Image;
extern fn oc_get_image_source_region() Rect;

pub const setColor = oc_set_color;
pub const setColorRgba = oc_set_color_rgba;
pub const setWidth = oc_set_width;
pub const setTolerance = oc_set_tolerance;
pub const setJoint = oc_set_joint;
pub const setMaxJointExcursion = oc_set_max_joint_excursion;
pub const setCap = oc_set_cap;
pub const setFont = oc_set_font;
pub const setFontSize = oc_set_font_size;
pub const setTextFlip = oc_set_text_flip;
pub const setImage = oc_set_image;
pub const setImageSourceRegion = oc_set_image_source_region;

pub const getColor = oc_get_color;
pub const getWidth = oc_get_width;
pub const getTolerance = oc_get_tolerance;
pub const getJoint = oc_get_joint;
pub const getMaxJointExcursion = oc_get_max_joint_excursion;
pub const getCap = oc_get_cap;
pub const getFont = oc_get_font;
pub const getFontSize = oc_get_font_size;
pub const getTextFlip = oc_get_text_flip;
pub const getImage = oc_get_image;
pub const getImageSourceRegion = oc_get_image_source_region;

//------------------------------------------------------------------------------------------
// [GRAPHICS]: construction: path
//------------------------------------------------------------------------------------------

extern fn oc_get_position() Vec2;
extern fn oc_move_to(x: f32, y: f32) void;
extern fn oc_line_to(x: f32, y: f32) void;
extern fn oc_quadratic_to(x1: f32, y1: f32, x2: f32, y2: f32) void;
extern fn oc_cubic_to(x1: f32, y1: f32, x2: f32, y2: f32, x3: f32, y3: f32) void;
extern fn oc_close_path() void;

extern fn oc_glyph_outlines(glyph_indices: Str32) Rect;
extern fn oc_codepoints_outlines(string: Str32) void;
extern fn oc_text_outlines(string: Str8) void;

pub const getPosition = oc_get_position;
pub const moveTo = oc_move_to;
pub const lineTo = oc_line_to;
pub const quadraticTo = oc_quadratic_to;
pub const cubicTo = oc_cubic_to;
pub const closePath = oc_close_path;

pub const glyphOutlines = oc_glyph_outlines;
pub const codepointsOutlines = oc_codepoints_outlines;
pub const textOutlines = oc_text_outlines;

//------------------------------------------------------------------------------------------
// [GRAPHICS]: vector graphics
//------------------------------------------------------------------------------------------

extern fn oc_clear() void;
extern fn oc_fill() void;
extern fn oc_stroke() void;

pub const clear = oc_clear;
pub const fill = oc_fill;
pub const stroke = oc_stroke;

//------------------------------------------------------------------------------------------
// [GRAPHICS]: shape helpers
//------------------------------------------------------------------------------------------

extern fn oc_rectangle_fill(x: f32, y: f32, w: f32, h: f32) void;
extern fn oc_rectangle_stroke(x: f32, y: f32, w: f32, h: f32) void;
extern fn oc_rounded_rectangle_fill(x: f32, y: f32, w: f32, h: f32, r: f32) void;
extern fn oc_rounded_rectangle_stroke(x: f32, y: f32, w: f32, h: f32, r: f32) void;
extern fn oc_ellipse_fill(x: f32, y: f32, rx: f32, ry: f32) void;
extern fn oc_ellipse_stroke(x: f32, y: f32, rx: f32, ry: f32) void;
extern fn oc_circle_fill(x: f32, y: f32, r: f32) void;
extern fn oc_circle_stroke(x: f32, y: f32, r: f32) void;
extern fn oc_arc(x: f32, y: f32, r: f32, arc_angle: f32, start_angle: f32) void;

pub const rectangleFill = oc_rectangle_fill;
pub const rectangleStroke = oc_rectangle_stroke;
pub const roundedRectangleFill = oc_rounded_rectangle_fill;
pub const roundedRectangleStroke = oc_rounded_rectangle_stroke;
pub const ellipseFill = oc_ellipse_fill;
pub const ellipseStroke = oc_ellipse_stroke;
pub const circleFill = oc_circle_fill;
pub const circleStroke = oc_circle_stroke;
pub const arc = oc_arc;

//------------------------------------------------------------------------------------------
// [GRAPHICS]: GLES
//------------------------------------------------------------------------------------------

// TODO

//------------------------------------------------------------------------------------------
// [UI]: GLES
//------------------------------------------------------------------------------------------

// TODO

//------------------------------------------------------------------------------------------
// [FILE IO] basic API
//------------------------------------------------------------------------------------------

const File = extern struct {
    h: u64,

    extern fn oc_file_nil() File;
    extern fn oc_file_is_nil(file: File) bool;
    extern fn oc_file_open(path: Str8, rights: FileAccessFlags, flags: FileOpenFlags) File;
    extern fn oc_file_open_at(dir: File, path: Str8, rights: FileAccessFlags, flags: FileOpenFlags) File;
    extern fn oc_file_close(file: File) void;
    extern fn oc_file_last_error(file: File) IoError;
    extern fn oc_file_pos(file: File) i64;
    extern fn oc_file_seek(file: File, offset: i64, whence: FileWhence) i64;
    extern fn oc_file_write(file: File, size: u64, buffer: ?[*]u8) u64;
    extern fn oc_file_read(file: File, size: u64, buffer: ?[*]u8) u64;

    extern fn oc_file_get_status(file: File) FileStatus;
    extern fn oc_file_size(file: File) u64;

    extern fn oc_file_open_with_request(path: Str8, rights: FileAccessFlags, flags: FileOpenFlags) File;
    extern fn oc_file_open_with_dialog(arena: *Arena, rights: FileAccessFlags, flags: FileOpenFlags, desc: *FileDialogDesc) FileOpenWithDialogResult;

    pub const nil = oc_file_nil;
    pub const isNil = oc_file_is_nil;
    pub const open = oc_file_open;
    pub const openAt = oc_file_open_at;
    pub const close = oc_file_close;
    pub const lastError = oc_file_last_error;
    pub const pos = oc_file_pos;
    pub const seek = oc_file_seek;
    pub const write = oc_file_write;
    pub const read = oc_file_read;

    pub const getStatus = oc_file_get_status;
    pub const size = oc_file_size;

    pub const openWithRequest = oc_file_open_with_request;
    pub const openWithDialog = oc_file_open_with_dialog;
};

const FileOpenFlags = packed struct(u16) {
    none: bool,
    append: bool,
    truncate: bool,
    create: bool,

    symlink: bool,
    no_follow: bool,
    restrict: bool,
};

const FileAccessFlags = packed struct(u16) {
    none: bool,
    read: bool,
    write: bool,
};

const FileWhence = enum(c_uint) {
    Set,
    End,
    Current,
};

const IoReqId = u16;
const IoOp = u32;

const IoOpEnum = enum(c_uint) {
    OpenAt = 0,
    Close,
    FStat,
    Seek,
    Read,
    Write,
    Error,
};

const IoReq = extern struct {
    id: IoReqId,
    op: IoOp,
    handle: File,

    offset: i64,
    size: u64,

    buffer: extern union {
        data: ?[*]u8,
        unused: u64, // This is a horrible hack to get the same layout on wasm and on host
    },

    type: extern union {
        open: extern struct {
            rights: FileAccessFlags,
            flags: FileOpenFlags,
        },
        whence: FileWhence,
    },
};

const IoError = enum(i32) {
    Ok = 0,
    Unknown,
    Op, // unsupported operation
    Handle, // invalid handle
    Prev, // previously had a fatal error (last error stored on handle)
    Arg, // invalid argument or argument combination
    Perm, // access denied
    Space, // no space left
    NoEntry, // file or directory does not exist
    Exists, // file already exists
    NotDir, // path element is not a directory
    Dir, // attempted to write directory
    MaxFiles, // max open files reached
    MaxLinks, // too many symbolic links in path
    PathLength, // path too long
    FileSize, // file too big
    Overflow, // offset too big
    NotReady, // no data ready to be read/written
    Mem, // failed to allocate memory
    Interrupt, // operation interrupted by a signal
    Physical, // physical IO error
    NoDevice, // device not found
    Walkout, // attempted to walk out of root directory
};

const IoCmp = extern struct {
    id: IoReqId,
    err: IoError,
    data: extern union {
        result: i64,
        size: u64,
        offset: i64,
        handle: File,
    },
};

const FileType = enum(c_uint) {
    Unknown,
    Regular,
    Directory,
    Symlink,
    Block,
    Character,
    Fifo,
    Socket,
};

const FilePerm = packed struct(u16) {
    other_exec: bool,
    other_write: bool,
    other_read: bool,
    group_exec: bool,
    group_write: bool,
    group_read: bool,
    owner_exec: bool,
    owner_write: bool,
    owner_read: bool,
    sticky_bit: bool,
    set_gid: bool,
    set_uid: bool,
};

const DateStamp = extern struct {
    seconds: i64, // seconds relative to NTP epoch.
    fraction: u64, // fraction of seconds elapsed since the time specified by seconds.
};

const FileStatus = extern struct {
    uid: u64,
    type: FileType,
    perm: FilePerm,
    size: u64,

    creation_date: DateStamp,
    access_date: DateStamp,
    modification_date: DateStamp,
};

const FileOpenWithDialogElt = extern struct {
    list_elt: ListElt,
    file: File,
};

const FileOpenWithDialogResult = extern struct {
    button: FileDialogButton,
    file: File,
    selection: List,
};

//------------------------------------------------------------------------------------------
// [FILE IO] complete io queue api
//------------------------------------------------------------------------------------------

extern fn oc_io_wait_single_req(req: *IoReq) IoCmp;

pub const ioWaitSingleReq = oc_io_wait_single_req;
