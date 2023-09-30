const std = @import("std");
const AllocError = std.mem.Allocator.Error;

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

    pub const Output = opaque {
        extern var OC_LOG_DEFAULT_OUTPUT: ?*Output;
        extern fn oc_log_set_output(output: *Output) void;

        pub inline fn default() ?*Output {
            return OC_LOG_DEFAULT_OUTPUT;
        }

        const set = oc_log_set_output;
    };

    extern fn oc_log_set_level(level: Level) void;
    extern fn oc_log_ext(level: Level, function: [*]const u8, file: [*]const u8, line: c_int, fmt: [*]const u8, ...) void;

    const setLevel = oc_log_set_level;

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

        oc_assert_fail(source.file.ptr, source.fn_name.ptr, line, "assertion failed", format_buf[0..].ptr);
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

    pub fn entry(self: *ListElt, comptime ParentType: type, comptime field_name_in_parent: []const u8) *ParentType {
        return @fieldParentPtr(ParentType, field_name_in_parent, self);
    }

    pub fn nextEntry(comptime ParentType: type, comptime field_name_in_parent: []const u8, elt_parent: *ParentType) ?*ParentType {
        const elt: ?*ListElt = @field(elt_parent, field_name_in_parent);
        if (elt.next) |next| {
            return next.entry(ParentType, field_name_in_parent);
        }

        return null;
    }

    pub fn prevEntry(comptime ParentType: type, comptime field_name_in_parent: []const u8, elt_parent: *ParentType) ?*ParentType {
        const elt: ?*ListElt = @field(elt_parent, field_name_in_parent);
        if (elt.prev) |prev| {
            return prev.entry(ParentType, field_name_in_parent);
        }

        return null;
    }
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

    pub fn firstEntry(self: *List, comptime EltParentType: type, comptime field_name_in_parent: []const u8) ?*EltParentType {
        if (self.first) |elt| {
            return elt.entry(EltParentType, field_name_in_parent);
        }
        return null;
    }

    pub fn lastEntry(self: *List, comptime EltParentType: type, comptime field_name_in_parent: []const u8) ?*EltParentType {
        if (self.last) |elt| {
            return elt.entry(EltParentType, field_name_in_parent);
        }
        return null;
    }

    const IterDirection = enum {
        Forward,
        Backward,
    };

    pub fn makeIter(comptime direction: IterDirection, comptime EltParentType: type, comptime field_name_in_parent: []const u8) type {
        return struct {
            const Self = @This();

            item: ?*ListElt,

            pub fn next(self: *Self) ?*EltParentType {
                if (self.item) |elt| {
                    var entry: *EltParentType = elt.entry(EltParentType, field_name_in_parent);
                    self.item = if (direction == .Forward) elt.next else elt.prev;
                    return entry;
                }
                return null;
            }
        };
    }

    pub fn iter(self: *const List, comptime EltParentType: type, comptime field_name_in_parent: []const u8) makeIter(.Forward, EltParentType, field_name_in_parent) {
        const Iter = makeIter(.Forward, EltParentType, field_name_in_parent);
        return Iter{
            .item = self.first,
        };
    }

    pub fn iterReverse(self: *const List, comptime EltParentType: type, comptime field_name_in_parent: []const u8) makeIter(.Backward, EltParentType, field_name_in_parent) {
        const Iter = makeIter(.Backward, EltParentType, field_name_in_parent);
        return Iter{
            .item = self.last,
        };
    }

    pub fn insert(self: *List, after_elt: ?*ListElt, elt: *ListElt) void {
        elt.prev = after_elt;
        elt.next = after_elt.next;
        if (after_elt.next) |elt_next| {
            after_elt.next.prev = elt_next;
        } else {
            self.last = elt;
        }
        after_elt.next = elt;
    }

    pub fn insertBefore(self: *List, before_elt: ?*ListElt, elt: *ListElt) void {
        elt.next = before_elt;
        elt.prev = before_elt.prev;
        if (before_elt.prev) |elt_prev| {
            before_elt.prev.next = elt_prev;
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
        if (self.first) |elt_first| {
            elt_first.prev = elt;
        } else {
            self.last = elt;
        }
        self.first = elt;
    }

    pub fn pop(self: *List) ?*ListElt {
        if (self.first) |elt_begin| {
            remove(self, elt_begin);
            return elt_begin;
        }
        return null;
    }

    pub fn popEntry(self: *List, comptime EltParentType: type, comptime field_name_in_parent: []const u8) ?*EltParentType {
        if (self.pop()) |elt| {
            return elt.entry(EltParentType, field_name_in_parent);
        }
        return null;
    }

    pub fn pushBack(self: *List, elt: *ListElt) void {
        elt.prev = self.last;
        elt.next = null;
        if (self.last) |last_elt| {
            last_elt.next = elt;
        } else {
            self.first = elt;
        }
        self.last = elt;
    }

    pub fn popBack(self: *List) ?*ListElt {
        if (self.last) |last_elt| {
            remove(self, last_elt);
            return last_elt;
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

pub const ArenaScope = extern struct {
    arena: *Arena,
    chunk: *ArenaChunk,
    offset: u64,

    extern fn oc_arena_scope_end(scope: ArenaScope) void;
    pub const end = oc_arena_scope_end;
};

pub const ArenaOptions = extern struct {
    base: ?*BaseAllocator,
    reserve: u64,
};

pub const Arena = extern struct {
    base: ?*BaseAllocator,
    chunks: List,
    current_chunk: ?*ArenaChunk,

    extern fn oc_arena_init(arena: *Arena) void;
    extern fn oc_arena_init_with_options(arena: *Arena, options: *ArenaOptions) void;
    extern fn oc_arena_cleanup(arena: *Arena) void;
    extern fn oc_arena_push(arena: *Arena, size: u64) ?[*]u8;
    extern fn oc_arena_push_aligned(arena: *Arena, size: u64, alignment: u32) ?[*]u8;
    extern fn oc_arena_clear(arena: *Arena) void;
    extern fn oc_arena_scope_begin(arena: *Arena) ArenaScope;

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

    pub const clear = oc_arena_clear;
    pub const scopeBegin = oc_arena_scope_begin;

    pub fn push(arena: *Arena, size: usize) AllocError![]u8 {
        return arena.pushAligned(size, 1);
    }

    pub fn pushAligned(arena: *Arena, size: usize, alignment: u32) AllocError![]u8 {
        if (oc_arena_push_aligned(arena, size, alignment)) |mem| {
            return mem[0..size];
        }
        return AllocError.OutOfMemory;
    }

    pub fn pushType(arena: *Arena, comptime T: type) AllocError!*T {
        var mem: []u8 = try arena.pushAligned(@sizeOf(T), @alignOf(T));
        assert(mem.len >= @sizeOf(T), "need at least {} bytes, but got {}", .{ mem.len, @sizeOf(T) }, @src());
        var p: *T = @alignCast(@ptrCast(mem.ptr));
        return p;
    }

    pub fn pushArray(arena: *Arena, comptime T: type, count: usize) AllocError![]T {
        var mem: []u8 = try arena.pushAligned(@sizeOf(T) * count, @alignOf(T));
        std.debug.assert(mem.len >= @sizeOf(T) * count);
        var items: [*]T = @alignCast(@ptrCast(mem.ptr));
        return items[0..count];
    }

    pub fn pushStr(arena: *Arena, str: []u8) AllocError![]u8 {
        var result = try arena.pushArray(u8, str.len + 1);
        @memcpy(result[0..str.len], str);
        result[str.len] = 0;
        return result;
    }

    pub const scratchBegin = oc_scratch_begin;
    pub const scratchBeginNext = oc_scratch_begin_next;
};

pub const Pool = extern struct {
    arena: Arena,
    free_list: List,
    block_size: u64,
};

//------------------------------------------------------------------------------------------
// [STRINGS]
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

            pub fn push(list: *StrList, arena: *Arena, str: Str) AllocError!void {
                var elt: *StrListElt = try arena.pushType(StrListElt);
                elt.string = str;
                list.list.pushBack(&elt.list_elt);
                list.elt_count += 1;
                list.len += str.len;
            }

            pub fn pushSlice(list: *StrList, arena: *Arena, str: []const CharType) AllocError!void {
                try list.push(arena, Str.fromSlice(str));
            }

            pub fn pushf(list: *StrList, arena: *Arena, comptime format: []const u8, args: anytype) AllocError!void {
                var str = try Str.pushf(arena, format, args);
                try list.push(arena, str);
            }

            pub fn iter(list: *const StrList) List.makeIter(.Forward, StrListElt, "list_elt") {
                return list.list.iter(StrListElt, "list_elt");
            }

            pub fn iterReverse(list: *const StrList) List.makeIter(.Backward, StrListElt, "list_elt") {
                return list.list.iterReverse(StrListElt, "list_elt");
            }

            pub fn find(list: *const StrList, needle: *const Str) ?*StrListElt {
                return list.findSlice(needle.slice());
            }

            pub fn findSlice(list: *const StrList, needle: []const CharType) ?*StrListElt {
                var iterator = list.iter();
                while (iterator.next()) |elt_string| {
                    if (std.mem.eql(CharType, elt_string.string.slice(), needle)) {
                        return elt_string;
                    }
                }
                return null;
            }

            pub fn contains(list: *const StrList, needle: *const Str) bool {
                return list.findSlice(needle.slice()) != null;
            }

            pub fn containsSlice(list: *const StrList, needle: []const CharType) bool {
                return list.findSlice(needle) != null;
            }

            pub fn join(list: *const StrList, arena: *Arena) AllocError!Str {
                const empty = Str{ .ptr = null, .len = 0 };
                return try list.collate(arena, empty, empty, empty);
            }

            pub fn collate(list: *const StrList, arena: *Arena, prefix: Str, separator: Str, postfix: Str) AllocError!Str {
                var str: Str = undefined;
                str.len = @intCast(prefix.len + list.len + (list.elt_count - 1) * separator.len + postfix.len);
                str.ptr = (try arena.pushArray(CharType, str.len + 1)).ptr;
                @memcpy(str.ptr.?[0..prefix.len], prefix.slice());

                var offset = prefix.len;

                var iterator = list.iter();
                var index: usize = 0;
                while (iterator.next()) |list_str| {
                    if (index > 0) {
                        @memcpy(str.ptr.?[offset .. offset + separator.len], separator.slice());
                        offset += separator.len;
                    }
                    @memcpy(str.ptr.?[offset .. offset + list_str.string.len], list_str.string.slice());
                    offset += list_str.string.len;
                    index += 1;
                }

                @memcpy(str.ptr.?[offset .. offset + postfix.len], postfix.slice());
                str.ptr.?[str.len] = 0;
                return str;
            }
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

        pub fn pushf(arena: *Arena, comptime format: []const u8, args: anytype) AllocError!Str {
            if (CharType != u8) {
                @compileError("pushf() is only supported for Str8");
            }

            var str: Str = undefined;
            str.len = @intCast(std.fmt.count(format, args));
            str.ptr = (try arena.pushArray(CharType, str.len + 1)).ptr;
            _ = std.fmt.bufPrintZ(str.ptr.?[0 .. str.len + 1], format, args) catch unreachable;
            return str;
        }

        pub fn join(arena: *Arena, strings: []const []const CharType) AllocError!Str {
            const empty = &[_]CharType{};
            return collate(arena, strings, empty, empty, empty);
        }

        pub fn collate(
            arena: *Arena,
            strings: []const []const CharType,
            prefix: []const CharType,
            separator: []const CharType,
            postfix: []const CharType,
        ) AllocError!Str {
            var strings_total_len: usize = 0;
            for (strings) |s| {
                strings_total_len += s.len;
            }

            var str: Str = undefined;
            str.len = prefix.len + strings_total_len + (strings.len - 1) * separator.len + postfix.len;
            str.ptr = (try arena.pushArray(CharType, str.len + 1)).ptr;
            @memcpy(str.ptr.?[0..prefix.len], prefix);

            var offset = prefix.len;

            for (strings, 0..) |list_str, index| {
                if (index > 0) {
                    @memcpy(str.ptr.?[offset .. offset + separator.len], separator);
                    offset += separator.len;
                }
                @memcpy(str.ptr.?[offset .. offset + list_str.len], list_str);
                offset += list_str.len;
            }

            @memcpy(str.ptr.?[offset .. offset + postfix.len], postfix);
            offset += postfix.len;
            str.ptr.?[str.len] = 0;
            return str;
        }

        pub fn split(str: *const Str, arena: *Arena, separators: StrList) AllocError!StrList {
            var list = StrList.init();
            if (str.ptr == null) {
                return list;
            }

            const ptr = str.ptr.?;

            var offset: usize = 0;
            var offset_substring: usize = 0;

            while (offset < str.len) {
                const haystack = ptr[offset..str.len];
                var separator_iter = separators.iter();
                while (separator_iter.next()) |list_sep| {
                    if (std.mem.startsWith(CharType, haystack, list_sep.string.slice()) and list_sep.string.len > 0) {
                        var substr = ptr[offset_substring..offset];
                        if (separators.containsSlice(substr)) {
                            substr = ptr[offset..offset];
                        }
                        try list.pushSlice(arena, substr);

                        // -1 / +1 to account for offset += 1 at the end of the loop
                        offset += list_sep.string.len - 1;
                        offset_substring = offset + 1;
                        break;
                    }
                }

                offset += 1;
            }

            if (offset_substring != offset) {
                var substr = ptr[offset_substring..offset];
                try list.pushSlice(arena, substr);
            }

            return list;
        }

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

pub const Str8List = Str8.StrList;
pub const Str16List = Str16.StrList;
pub const Str32List = Str32.StrList;

pub const Str8ListElt = Str8.StrListElt;
pub const Str16ListElt = Str16.StrListElt;
pub const Str32ListElt = Str32.StrListElt;

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

pub const EventType = enum(c_uint) {
    none,
    keyboard_mods,
    keyboard_key,
    keyboard_char,
    mouse_button,
    mouse_move,
    mouse_wheel,
    mouse_enter,
    mouse_leave,
    clipboard_paste,
    window_resize,
    window_move,
    window_focus,
    window_unfocus,
    window_hide,
    window_show,
    window_close,
    pathdrop,
    frame,
    quit,
};

pub const KeyAction = enum(c_uint) {
    NoAction,
    Press,
    Release,
    Repeat,
};

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
};

pub const key_code_count: usize = 349;

pub const KeymodFlags = packed struct(c_uint) {
    none: bool,
    alt: bool,
    shift: bool,
    ctrl: bool,
    cmd: bool,
    main_modified: bool, // cmd on Mac, ctrl on Win32
    _: u26,
};

pub const MouseButton = enum(c_uint) {
    Left = 0x00,
    Right = 0x01,
    Middle = 0x02,
    Ext1 = 0x03,
    Ext2 = 0x04,
};

pub const mouse_button_count: usize = 5;

/// Keyboard and mouse buttons input
pub const KeyEvent = extern struct {
    action: KeyAction,
    scan_code: ScanCode,
    key_code: KeyCode,
    button: MouseButton,
    mods: KeymodFlags,
    click_count: u8,
};

/// Character input
pub const CharEvent = extern struct {
    codepoint: Utf32,
    sequence: [8]u8,
    sequence_len: u8,
};

/// Mouse move/scroll
pub const MouseEvent = extern struct {
    x: f32,
    y: f32,
    delta_x: f32,
    delta_y: f32,
    mods: KeymodFlags,
};

/// Window resize / move
pub const MoveEvent = extern struct {
    frame: Rect,
    content: Rect,
};

pub const CEvent = extern struct {
    window: Window,
    type: EventType,
    data: extern union {
        key: KeyEvent,
        character: CharEvent,
        mouse: MouseEvent,
        move: MoveEvent,
        paths: Str8List,
    },

    pub fn event(self: *CEvent) Event {
        return .{ .window = self.window, .event = switch (self.type) {
            .none => .none,
            .keyboard_mods => .{ .keyboard_mods = self.data.key },
            .keyboard_key => .{ .keyboard_key = self.data.key },
            .keyboard_char => .{ .keyboard_char = self.data.character },
            .mouse_button => .{ .mouse_button = self.data.key },
            .mouse_move => .{ .mouse_move = self.data.mouse },
            .mouse_wheel => .{ .mouse_wheel = self.data.mouse },
            .mouse_enter => .{ .mouse_enter = self.data.mouse },
            .mouse_leave => .{ .mouse_leave = self.data.mouse },
            .clipboard_paste => .clipboard_paste,
            .window_resize => .{ .window_resize = self.data.move },
            .window_move => .{ .window_move = self.data.move },
            .window_focus => .window_focus,
            .window_unfocus => .window_unfocus,
            .window_hide => .window_hide,
            .window_show => .window_show,
            .window_close => .window_close,
            .pathdrop => .{ .pathdrop = self.data.paths },
            .frame => .frame,
            .quit => .quit,
        } };
    }
};

pub const Event = struct {
    window: Window,
    event: union(EventType) {
        none,
        keyboard_mods: KeyEvent,
        keyboard_key: KeyEvent,
        keyboard_char: CharEvent,
        mouse_button: KeyEvent,
        mouse_move: MouseEvent,
        mouse_wheel: MouseEvent,
        mouse_enter: MouseEvent,
        mouse_leave: MouseEvent,
        clipboard_paste,
        window_resize: MoveEvent,
        window_move: MoveEvent,
        window_focus,
        window_unfocus,
        window_hide,
        window_show,
        window_close,
        pathdrop: Str8List,
        frame,
        quit,
    },
};

//------------------------------------------------------------------------------------------
// [APP] windows
//------------------------------------------------------------------------------------------

pub const Window = extern struct {
    h: u64,
};

extern fn oc_window_set_title(title: Str8) void;
extern fn oc_window_set_size(size: Vec2) void;

pub fn windowSetTitle(title: []const u8) void {
    oc_window_set_title(Str8.fromSlice(title));
}

pub const windowSetSize = oc_window_set_size;

//------------------------------------------------------------------------------------------
// [CLOCK]
//------------------------------------------------------------------------------------------

pub const clock = struct {
    pub const Kind = enum(c_uint) {
        Monotonic,
        Uptime,
        Date,
    };

    extern fn oc_clock_time(clock: Kind) f64;
    pub const time = oc_clock_time;
};

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
// [MATH] Matrix stack
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
// [GRAPHICS]: path primitives
//------------------------------------------------------------------------------------------

const PathEltType = enum(c_uint) {
    Move,
    Line,
    Quadratic,
    Cubic,
};

const PathElt = extern struct {
    type: PathEltType,
    p: [3]Vec2,
};

const PathDescriptor = extern struct {
    start_index: u32,
    count: u32,
    start_point: Vec2,
};

const Attributes = extern struct {
    width: f32,
    tolerance: f32,
    color: Color,
    joint: JointType,
    max_joint_excursion: f32,
    cap: CapType,

    font: Font,
    font_size: f32,

    image: Image,
    src_region: Rect,

    transform: Mat2x3,
    clip: Rect,
};

const PrimitiveCmd = enum(c_uint) {
    Fill,
    Stroke,
    Jump,
};

const Primitive = extern struct {
    cmd: PrimitiveCmd,
    attributes: Attributes,
    data: extern union {
        path: PathDescriptor,
        rect: Rect,
        jump: u32,
    },
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
    extern fn oc_surface_render_commands(
        surface: Surface,
        color: Color,
        primitive_count: u32,
        primitives: [*]Primitive,
        elt_count: u32,
        elements: [*]PathElt,
    ) void;

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
    pub fn renderCommands(surface: Surface, color: Color, primitives: []Primitive, elements: []PathElt) void {
        oc_surface_render_commands(surface, color, @intCast(primitives.len), primitives.ptr, @intCast(elements.len), elements.ptr);
    }
};

pub const Image = extern struct {
    h: u64,

    extern fn oc_image_nil() Image;
    extern fn oc_image_is_nil(image: Image) bool;
    extern fn oc_image_create(surface: Surface, width: u32, height: u32) Image;
    extern fn oc_image_create_from_rgba8(surface: Surface, width: u32, height: u32, pixels: [*]const u8) Image;
    extern fn oc_image_create_from_memory(surface: Surface, mem: Str8, flip: bool) Image;
    extern fn oc_image_create_from_file(surface: Surface, file: File, flip: bool) Image;
    extern fn oc_image_create_from_path(surface: Surface, path: Str8, flip: bool) Image;
    extern fn oc_image_destroy(image: Image) void;
    extern fn oc_image_upload_region_rgba8(image: Image, region: Rect, pixels: [*]const u8) void;
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

pub const Rect = extern struct {
    x: f32,
    y: f32,
    w: f32,
    h: f32,

    pub fn xywh(x: f32, y: f32, w: f32, h: f32) Rect {
        return .{ .x = x, .y = y, .w = w, .h = h };
    }

    pub fn xyArray(rect: *Rect) *[2]f32 {
        return @ptrCast(rect);
    }

    pub fn whArray(rect: *Rect) *[2]f32 {
        return @ptrCast(&rect.w);
    }

    pub fn array(rect: *Rect) *[4]f32 {
        return @ptrCast(rect);
    }
};

pub const Color = extern struct {
    r: f32 = 1.0,
    g: f32 = 1.0,
    b: f32 = 1.0,
    a: f32 = 1.0,

    pub fn rgba(r: f32, g: f32, b: f32, a: f32) Color {
        return .{ .r = r, .g = g, .b = b, .a = a };
    }

    pub fn toArray(color: *Color) *[4]f32 {
        return @ptrCast(color);
    }

    pub fn toRgba8(color: *const Color) u32 {
        var c: u32 = 0;
        c |= @as(u32, @intFromFloat(color.r * 255.0));
        c |= @as(u32, @intFromFloat(color.g * 255.0)) << 8;
        c |= @as(u32, @intFromFloat(color.b * 255.0)) << 16;
        c |= @as(u32, @intFromFloat(color.a * 255.0)) << 24;
        return c;
    }
};

//------------------------------------------------------------------------------------------
// [GRAPHICS]: Canvas
//------------------------------------------------------------------------------------------

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

    // vector graphics

    extern fn oc_clear() void;
    extern fn oc_fill() void;
    extern fn oc_stroke() void;

    pub const clear = oc_clear;
    pub const fill = oc_fill;
    pub const stroke = oc_stroke;

    //  attributes setting/getting

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

    // path construction

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

    // shape helpers

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
};

//------------------------------------------------------------------------------------------
// [UI]: input state
//------------------------------------------------------------------------------------------

pub const KeyState = extern struct {
    last_update: u64,
    transition_count: u32,
    repeat_count: u32,
    down: bool,
    sys_clicked: bool,
    sys_double_clicked: bool,
    sys_triple_clicked: bool,
};

pub const KeyboardState = extern struct {
    keys: [key_code_count]KeyState,
    mods: KeymodFlags,
};

pub const MouseButtonsState = extern struct {
    left: KeyState,
    right: KeyState,
    middle: KeyState,
    ext1: KeyState,
    ext2: KeyState,

    pub fn array(self: *MouseButtonsState) [mouse_button_count]KeyState {
        return .{
            self.left,
            self.right,
            self.middle,
            self.ext1,
            self.ext2,
        };
    }
};

pub const MouseState = extern struct {
    last_update: u64,
    pos_valid: bool,
    pos: Vec2,
    delta: Vec2,
    wheel: Vec2,
    buttons: MouseButtonsState,
};

pub const TextState = extern struct {
    last_update: u64,
    backing: [64]Utf32,
    codepoints: Str32,
};

pub const ClipboardState = extern struct {
    last_update: u64,
    pasted_text: Str8,
};

pub const InputState = extern struct {
    frame_counter: u64,
    keyboard: KeyboardState,
    mouse: MouseState,
    text: TextState,
    clipboard: ClipboardState,

    extern fn oc_input_process_event(arena: *Arena, state: *InputState, event: *CEvent) void;
    extern fn oc_input_next_frame(state: *InputState) void;

    extern fn oc_key_down(state: *InputState, key: KeyCode) bool;
    extern fn oc_key_press_count(state: *InputState, key: KeyCode) u8;
    extern fn oc_key_release_count(state: *InputState, key: KeyCode) u8;
    extern fn oc_key_repeat_count(state: *InputState, key: KeyCode) u8;

    extern fn oc_key_down_scancode(state: *InputState, key: ScanCode) bool;
    extern fn oc_key_press_count_scancode(state: *InputState, key: ScanCode) u8;
    extern fn oc_key_release_count_scancode(state: *InputState, key: ScanCode) u8;
    extern fn oc_key_repeat_count_scancode(state: *InputState, key: ScanCode) u8;

    extern fn oc_mouse_down(state: *InputState, button: MouseButton) bool;
    extern fn oc_mouse_pressed(state: *InputState, button: MouseButton) u8;
    extern fn oc_mouse_released(state: *InputState, button: MouseButton) u8;
    extern fn oc_mouse_clicked(state: *InputState, button: MouseButton) bool;
    extern fn oc_mouse_double_clicked(state: *InputState, button: MouseButton) bool;
    extern fn oc_mouse_position(state: *InputState) Vec2;
    extern fn oc_mouse_delta(state: *InputState) Vec2;
    extern fn oc_mouse_wheel(state: *InputState) Vec2;

    extern fn oc_input_text_utf32(arena: *Arena, state: *InputState) Str32;
    extern fn oc_input_text_utf8(arena: *Arena, state: *InputState) Str8;

    extern fn oc_clipboard_pasted(state: *InputState) bool;
    extern fn oc_clipboard_pasted_text(state: *InputState) Str8;

    extern fn oc_key_mods(state: *InputState) KeymodFlags;

    pub fn processEvent(self: *InputState, arena: *Arena, event: *CEvent) void {
        oc_input_process_event(arena, self, event);
    }

    pub const nextFrame = oc_input_next_frame;

    pub const keyDown = oc_key_down;
    pub const keyPressCount = oc_key_press_count;
    pub const keyReleaseCount = oc_key_release_count;
    pub const keyRepeatCount = oc_key_repeat_count;

    pub const keyDownScancode = oc_key_down_scancode;
    pub const keyPressCountScancode = oc_key_press_count_scancode;
    pub const keyReleaseCountScancode = oc_key_release_count_scancode;
    pub const keyRepeatCountScancode = oc_key_repeat_count_scancode;

    pub const mouseDown = oc_mouse_down;
    pub const mousePressed = oc_mouse_pressed;
    pub const mouseReleased = oc_mouse_released;
    pub const mouseClicked = oc_mouse_clicked;
    pub const mouseDoubleClicked = oc_mouse_double_clicked;
    pub const mousePosition = oc_mouse_position;
    pub const mouseDelta = oc_mouse_delta;
    pub const mouseWheel = oc_mouse_wheel;

    pub const inputTextUtf32 = oc_input_text_utf32;
    pub const inputTextUtf8 = oc_input_text_utf8;

    pub const clipboardPasted = oc_clipboard_pasted;

    pub fn clipboardPastedText(self: *InputState) []u8 {
        return oc_clipboard_pasted_text(self).slice();
    }

    pub const keyMods = oc_key_mods;
};

//------------------------------------------------------------------------------------------
// [UI]: structs
//------------------------------------------------------------------------------------------

pub const UiKey = extern struct {
    hash: u64,
};

pub const UiAxis = enum(c_uint) { X, Y };

pub const UiLayoutMargin = struct {
    x: ?f32 = null,
    y: ?f32 = null,

    pub fn array(self: *UiLayoutMargin) *[2]?f32 {
        return @ptrCast(self);
    }
};

pub const UiAlignment = enum(c_uint) { Start, End, Center };

pub const UiLayoutAlignment = struct {
    x: ?UiAlignment = null,
    y: ?UiAlignment = null,

    pub fn array(self: *UiLayoutAlignment) *[2]?UiAlignment {
        return @ptrCast(self);
    }
};

pub const UiLayout = struct {
    axis: ?UiAxis = null,
    spacing: ?f32 = null,
    margin: UiLayoutMargin = .{},
    alignment: UiLayoutAlignment = .{},
};

pub const UiSizeKind = enum(c_uint) {
    Text,
    Pixels,
    Children,
    Parent,
    ParentMinusPixels,
};

pub const UiSizeCustom = struct {
    kind: UiSizeKind = .Text,
    value: f32 = 0,
    relax: f32 = 0,
    min_size: f32 = 0,
};

pub const UiSize = union(enum) {
    text,
    pixels: f32,
    children,
    fill_parent,
    parent: f32,
    parent_minus_pixels: f32,
    custom: UiSizeCustom,
};

pub const UiBoxSize = struct {
    width: ?UiSize = null,
    height: ?UiSize = null,

    pub fn array(self: *UiBoxSize) *[2]?UiSize {
        return @ptrCast(self);
    }
};

pub const UiBoxFloating = struct {
    x: ?bool = null,
    y: ?bool = null,

    pub fn array(self: *UiBoxFloating) *[2]?bool {
        return @ptrCast(self);
    }
};

pub const FloatTarget = struct {
    x: ?f32 = null,
    y: ?f32 = null,

    pub fn array(self: *FloatTarget) *[2]f32 {
        return @ptrCast(self);
    }
};

//NOTE: flags for axis-dependent properties (e.g. FloatX/Y) need to be consecutive bits
//      in order to play well with axis agnostic functions. Using explicit bit shifts here
//      so that arithmetic on flags is possible
pub const UiAnimationMask = enum(c_uint) {
    None = 0,
    SizeWidth = 1 << 1,
    SizeHeight = 1 << 2,
    LayoutAxis = 1 << 3,
    LayoutAlignX = 1 << 4,
    LayoutAlignY = 1 << 5,
    LayoutSpacing = 1 << 6,
    LayoutMarginX = 1 << 7,
    LayoutMarginY = 1 << 8,
    FloatX = 1 << 9,
    FloatY = 1 << 10,
    Color = 1 << 11,
    BgColor = 1 << 12,
    BorderColor = 1 << 13,
    BorderSize = 1 << 14,
    Roundness = 1 << 15,

    FontSize = 1 << 17,
};

pub const UiStyle = struct {
    size: UiBoxSize = .{},
    layout: UiLayout = .{},
    floating: UiBoxFloating = .{},
    float_target: FloatTarget = .{},
    color: ?Color = null,
    bg_color: ?Color = null,
    border_color: ?Color = null,
    font: ?Font = null,
    font_size: ?f32 = null,
    border_size: ?f32 = null,
    roundness: ?f32 = null,
    animation_time: ?f32 = null,
    animation_mask: UiAnimationMask = .None,
};

pub const UiPalette = extern struct {
    red0: Color,
    red1: Color,
    red2: Color,
    red3: Color,
    red4: Color,
    red5: Color,
    red6: Color,
    red7: Color,
    red8: Color,
    red9: Color,
    orange0: Color,
    orange1: Color,
    orange2: Color,
    orange3: Color,
    orange4: Color,
    orange5: Color,
    orange6: Color,
    orange7: Color,
    orange8: Color,
    orange9: Color,
    amber0: Color,
    amber1: Color,
    amber2: Color,
    amber3: Color,
    amber4: Color,
    amber5: Color,
    amber6: Color,
    amber7: Color,
    amber8: Color,
    amber9: Color,
    yellow0: Color,
    yellow1: Color,
    yellow2: Color,
    yellow3: Color,
    yellow4: Color,
    yellow5: Color,
    yellow6: Color,
    yellow7: Color,
    yellow8: Color,
    yellow9: Color,
    lime0: Color,
    lime1: Color,
    lime2: Color,
    lime3: Color,
    lime4: Color,
    lime5: Color,
    lime6: Color,
    lime7: Color,
    lime8: Color,
    lime9: Color,
    light_green0: Color,
    light_green1: Color,
    light_green2: Color,
    light_green3: Color,
    light_green4: Color,
    light_green5: Color,
    light_green6: Color,
    light_green7: Color,
    light_green8: Color,
    light_green9: Color,
    green0: Color,
    green1: Color,
    green2: Color,
    green3: Color,
    green4: Color,
    green5: Color,
    green6: Color,
    green7: Color,
    green8: Color,
    green9: Color,
    teal0: Color,
    teal1: Color,
    teal2: Color,
    teal3: Color,
    teal4: Color,
    teal5: Color,
    teal6: Color,
    teal7: Color,
    teal8: Color,
    teal9: Color,
    cyan0: Color,
    cyan1: Color,
    cyan2: Color,
    cyan3: Color,
    cyan4: Color,
    cyan5: Color,
    cyan6: Color,
    cyan7: Color,
    cyan8: Color,
    cyan9: Color,
    light_blue0: Color,
    light_blue1: Color,
    light_blue2: Color,
    light_blue3: Color,
    light_blue4: Color,
    light_blue5: Color,
    light_blue6: Color,
    light_blue7: Color,
    light_blue8: Color,
    light_blue9: Color,
    blue0: Color,
    blue1: Color,
    blue2: Color,
    blue3: Color,
    blue4: Color,
    blue5: Color,
    blue6: Color,
    blue7: Color,
    blue8: Color,
    blue9: Color,
    indigo0: Color,
    indigo1: Color,
    indigo2: Color,
    indigo3: Color,
    indigo4: Color,
    indigo5: Color,
    indigo6: Color,
    indigo7: Color,
    indigo8: Color,
    indigo9: Color,
    violet0: Color,
    violet1: Color,
    violet2: Color,
    violet3: Color,
    violet4: Color,
    violet5: Color,
    violet6: Color,
    violet7: Color,
    violet8: Color,
    violet9: Color,
    purple0: Color,
    purple1: Color,
    purple2: Color,
    purple3: Color,
    purple4: Color,
    purple5: Color,
    purple6: Color,
    purple7: Color,
    purple8: Color,
    purple9: Color,
    pink0: Color,
    pink1: Color,
    pink2: Color,
    pink3: Color,
    pink4: Color,
    pink5: Color,
    pink6: Color,
    pink7: Color,
    pink8: Color,
    pink9: Color,
    grey0: Color,
    grey1: Color,
    grey2: Color,
    grey3: Color,
    grey4: Color,
    grey5: Color,
    grey6: Color,
    grey7: Color,
    grey8: Color,
    grey9: Color,
    black: Color,
    white: Color,
};

/// Visualized in doc/UIColors.md
pub const ui_dark_palette = @extern(*UiPalette, .{ .name = "OC_UI_DARK_PALETTE" });

/// Visualized in doc/UIColors.md
pub const ui_light_palette = @extern(*UiPalette, .{ .name = "OC_UI_LIGHT_PALETTE" });

pub const UiTheme = extern struct {
    white: Color,
    primary: Color,
    primary_hover: Color,
    primary_active: Color,
    border: Color,
    fill0: Color,
    fill1: Color,
    fill2: Color,
    bg0: Color,
    bg1: Color,
    bg2: Color,
    bg3: Color,
    bg4: Color,
    text0: Color,
    text1: Color,
    text2: Color,
    text3: Color,
    slider_thumb_border: Color,
    elevated_border: Color,

    roundness_small: f32,
    roundness_medium: f32,
    roundness_large: f32,

    palette: *UiPalette,
};

pub const ui_dark_theme = @extern(*UiTheme, .{ .name = "OC_UI_DARK_THEME" });
pub const ui_light_theme = @extern(*UiTheme, .{ .name = "OC_UI_LIGHT_THEME" });

pub const UiTag = extern struct {
    hash: u64,
};

pub const UiSelectorKind = enum(c_uint) {
    any,
    owner,
    text,
    tag,
    status,
    key,
};

pub const UiStatus = packed struct(u8) {
    _: u1 = 0,

    hover: bool = false,
    hot: bool = false,
    active: bool = false,
    dragging: bool = false,

    __: u3 = 0,

    pub fn empty(self: UiStatus) bool {
        return !self.hover and !self.hot and !self.active and !self.dragging;
    }
};

pub const UiSelectorOp = enum(c_uint) {
    Descendant,
    And,
};

pub const UiSelector = struct {
    op: UiSelectorOp = .Descendant,
    sel: union(UiSelectorKind) {
        any,
        owner,
        text: []u8,
        key: UiKey,
        tag: UiTag,
        status: UiStatus,
    },
};

pub const UiPattern = extern struct {
    l: List,

    extern fn oc_ui_pattern_push(arena: *Arena, pattern: *UiPattern, selector: UiSelectorInternal) void;
    extern fn oc_ui_pattern_all() UiPattern;
    extern fn oc_ui_pattern_owner() UiPattern;

    pub fn init() UiPattern {
        return .{ .l = List.init() };
    }

    /// Push the selector onto frame arena and insert it into the pattern's linked list.
    /// Underlying UiSelector implementation has a ListElt within it that is not exposed to the Zig interface
    /// in order to simplify the conversion.
    ///
    /// WARN: You can use a pattern in multiple rules, but be aware that a pattern is referencing
    ///       an underlying list of selectors, hence pushing to a pattern also modifies rules in
    ///       which the pattern was previously used!
    pub fn push(self: *UiPattern, arena: *Arena, selector: UiSelector) void {
        oc_ui_pattern_push(arena, self, uiConvertSelector(selector));
    }

    pub const all = oc_ui_pattern_all;
    pub const owner = oc_ui_pattern_owner;
};

pub const UiStyleRule = struct {
    box_elt: ListElt,
    build_elt: ListElt,
    tmp_elt: ListElt,

    owner: *UiBox,
    pattern: UiPattern,
    style: *UiStyle,
};

pub const UiSig = extern struct {
    box: *UiBox,

    mouse: Vec2,
    delta: Vec2,
    wheel: Vec2,

    pressed: bool,
    released: bool,
    clicked: bool,
    doubleClicked: bool,
    tripleClicked: bool,
    rightPressed: bool,

    dragging: bool,
    hovering: bool,

    pasted: bool,
};

pub const UiBoxDrawProc = *fn (box: *UiBox, data: ?*anyopaque) callconv(.C) void;

pub const UiOverflowAllow = packed struct {
    x: bool = false,
    y: bool = false,

    pub fn array(self: *UiOverflowAllow) *[2]bool {
        return @ptrCast(self);
    }
};

pub const UiFlags = packed struct(c_uint) {
    clickable: bool = false, // 0
    scroll_wheel_x: bool = false, // 1
    scroll_wheel_y: bool = false, // 2
    block_mouse: bool = false, // 3
    hot_animation: bool = false, // 4
    active_animation: bool = false, // 5
    overflow_allow: UiOverflowAllow = .{}, // 6-7
    clip: bool = false, // 8
    draw_background: bool = false, // 9
    draw_foreground: bool = false, // 10
    draw_border: bool = false, // 11
    draw_text: bool = false, // 12
    draw_proc: bool = false, // 13
    _: u2 = 0, // 14-15

    overlay: bool = false, // 16
    __: u15 = 0,
};

pub const UiBox = extern struct {
    // hierarchy
    list_elt: ListElt,
    children: List,
    parent: ?*UiBox,

    overlay_elt: ListElt,

    // keying and caching
    bucket_elt: ListElt,
    key: UiKey,
    frame_counter: u64,

    // builder-provided info
    flags: UiFlags,
    string: Str8,
    tags: List,

    draw_proc: UiBoxDrawProc,
    draw_data: *anyopaque,

    // styling
    before_rules: List,
    after_rules: List,

    target_style: ?*UiStyle,
    style: UiStyleInternal,
    z: u32,

    float_pos: Vec2,
    children_sum: [2]f32,
    spacing: [2]f32,
    min_size: [2]f32,
    rect: Rect,

    // signals
    sig: ?*UiSig,

    // stateful behavior
    fresh: bool,
    closed: bool,
    parent_closed: bool,
    dragging: bool,
    hot: bool,
    active: bool,
    scroll: Vec2,
    pressed_mouse: Vec2,

    // animation data
    hot_transition: f32,
    active_transition: f32,
};

pub const UiInputText = extern struct {
    count: u8,
    codepoints: [64]Utf32,
};

const UiCSize = extern struct {
    kind: UiSizeKind,
    value: f32 = 0,
    relax: f32 = 0,
    min_size: f32 = 0,

    fn fromUiSize(ui_size: UiSize) UiCSize {
        return switch (ui_size) {
            .text => .{ .kind = .Text },
            .pixels => |pixels| .{ .kind = .Pixels, .value = pixels },
            .children => .{ .kind = .Children },
            .fill_parent => .{ .kind = .Parent, .value = 1 },
            .parent => |fraction| .{ .kind = .Parent, .value = fraction },
            .parent_minus_pixels => |pixels| .{ .kind = .ParentMinusPixels, .value = pixels },
            .custom => |custom| .{
                .kind = custom.kind,
                .value = custom.value,
                .relax = custom.relax,
                .min_size = custom.min_size,
            },
        };
    }
};

pub const UiStackElt = extern struct {
    parent: ?*UiStackElt,
    elt: extern union {
        box: *UiBox,
        size: UiCSize,
        clip: Rect,
    },
};

pub const UiTagElt = extern struct {
    list_elt: ListElt,
    tag: UiTag,
};

pub const UiEditMove = enum(c_uint) {
    none,
    char,
    word,
    line,
};

pub const UiContext = extern struct {
    is_init: bool,

    input: InputState,

    frame_counter: u64,
    frame_time: f64,
    last_frame_duration: f64,

    frame_arena: Arena,
    box_pool: Pool,
    box_map: [1024]List,

    root: *UiBox,
    overlay: *UiBox,
    overlay_list: List,
    box_stack: *UiStackElt,
    clip_stack: *UiStackElt,

    next_box_before_rules: List,
    next_box_after_rules: List,
    next_box_tags: List,

    z: u32,
    hovered: ?*UiBox,

    focus: ?*UiBox,
    edit_cursor: i32,
    edit_mark: i32,
    edit_first_displayed_char: i32,
    edit_cursor_blink_start: f64,
    edit_selection_mode: UiEditMove,
    edit_word_selection_initial_cursor: i32,
    edit_word_selection_initial_mark: i32,

    theme: *UiTheme,

    extern fn oc_ui_init(context: *UiContext) void;
    pub const init = oc_ui_init;
};

const UiLayoutAlignmentInternal = extern struct {
    x: UiAlignment,
    y: UiAlignment,
};

const UiLayoutMarginInternal = extern struct {
    x: f32,
    y: f32,
};

const UiLayoutInternal = extern struct {
    axis: UiAxis,
    spacing: f32,
    margin: UiLayoutMarginInternal,
    alignment: UiLayoutAlignmentInternal,
};

const UiBoxSizeInternal = extern struct {
    width: UiCSize,
    height: UiCSize,
};

const UiBoxFloatingInternal = extern struct {
    x: bool,
    y: bool,
};

const UiFloatTargetInternal = extern struct {
    x: f32,
    y: f32,
};

const UiStyleInternal = extern struct {
    size: UiBoxSizeInternal,
    layout: UiLayoutInternal,
    floating: UiBoxFloatingInternal,
    float_target: UiFloatTargetInternal,
    color: Color,
    bg_color: Color,
    border_color: Color,
    font: Font,
    font_size: f32,
    border_size: f32,
    roundness: f32,
    animation_time: f32,
    animation_mask: UiAnimationMask,
};

const UiStyleMaskInternal = packed struct(u64) {
    _: u1 = 0,
    size_width: bool = false,
    size_height: bool = false,
    layout_axis: bool = false,
    layout_align_x: bool = false,
    layout_align_y: bool = false,
    layout_spacing: bool = false,
    layout_margin_x: bool = false,
    layout_margin_y: bool = false,
    float_x: bool = false,
    float_y: bool = false,
    color: bool = false,
    bg_color: bool = false,
    border_color: bool = false,
    border_size: bool = false,
    roundness: bool = false,
    font: bool = false,
    font_size: bool = false,
    animation_time: bool = false,
    animation_mask: bool = false,
    __: u44 = 0,
};

//------------------------------------------------------------------------------------------
// [UI]: context initialization and frame cycle
//------------------------------------------------------------------------------------------

extern fn oc_ui_get_context() *UiContext;
extern fn oc_ui_set_context(context: *UiContext) void;

extern fn oc_ui_process_event(event: *CEvent) void;
extern fn oc_ui_begin_frame(size: Vec2, default_style: *UiStyleInternal, mask: UiStyleMaskInternal) void;
extern fn oc_ui_end_frame() void;
extern fn oc_ui_draw() void;
extern fn oc_ui_set_theme(theme: *UiTheme) void;

pub const uiGetContext = oc_ui_get_context;
pub const uiSetContext = oc_ui_set_context;
pub const uiProcessCEvent = oc_ui_process_event;

pub const uiEndFrame = oc_ui_end_frame;
pub const uiDraw = oc_ui_draw;
pub const uiSetTheme = oc_ui_set_theme;

pub fn uiBeginFrame(size: Vec2, default_style: *UiStyle) void {
    var default_style_and_mask = uiConvertStyle(default_style);
    oc_ui_begin_frame(size, &default_style_and_mask.style, default_style_and_mask.mask);
}

const UiStyleAndMaskInternal = struct {
    style: UiStyleInternal,
    mask: UiStyleMaskInternal,
};

fn uiConvertStyle(style: *const UiStyle) UiStyleAndMaskInternal {
    var style_internal: UiStyleInternal = std.mem.zeroes(UiStyleInternal);
    var mask: UiStyleMaskInternal = .{};
    if (style.size.width) |width| {
        style_internal.size.width = UiCSize.fromUiSize(width);
        mask.size_width = true;
    }
    if (style.size.height) |height| {
        style_internal.size.height = UiCSize.fromUiSize(height);
        mask.size_height = true;
    }
    if (style.layout.axis) |axis| {
        style_internal.layout.axis = axis;
        mask.layout_axis = true;
    }
    if (style.layout.alignment.x) |x| {
        style_internal.layout.alignment.x = x;
        mask.layout_align_x = true;
    }
    if (style.layout.alignment.y) |y| {
        style_internal.layout.alignment.y = y;
        mask.layout_align_y = true;
    }
    if (style.layout.spacing) |spacing| {
        style_internal.layout.spacing = spacing;
        mask.layout_spacing = true;
    }
    if (style.layout.margin.x) |x| {
        style_internal.layout.margin.x = x;
        mask.layout_margin_x = true;
    }
    if (style.layout.margin.y) |y| {
        style_internal.layout.margin.y = y;
        mask.layout_margin_y = true;
    }
    if (style.floating.x) |x| {
        style_internal.floating.x = x;
        if (style.float_target.x) |target_x| {
            style_internal.float_target.x = target_x;
        }
        mask.float_x = true;
    }
    if (style.floating.y) |y| {
        style_internal.floating.y = y;
        if (style.float_target.y) |target_y| {
            style_internal.float_target.y = target_y;
        }
        mask.float_y = true;
    }
    if (style.color) |color| {
        style_internal.color = color;
        mask.color = true;
    }
    if (style.bg_color) |bg_color| {
        style_internal.bg_color = bg_color;
        mask.bg_color = true;
    }
    if (style.border_color) |border_color| {
        style_internal.border_color = border_color;
        mask.border_color = true;
    }
    if (style.border_size) |border_size| {
        style_internal.border_size = border_size;
        mask.border_size = true;
    }
    if (style.roundness) |roundness| {
        style_internal.roundness = roundness;
        mask.roundness = true;
    }
    if (style.font) |font| {
        style_internal.font = font;
        mask.font = true;
    }
    if (style.font_size) |font_size| {
        style_internal.font_size = font_size;
        mask.font_size = true;
    }
    if (style.animation_time) |animation_time| {
        style_internal.animation_time = animation_time;
        mask.animation_time = true;
    }
    if (style.animation_mask != .None) {
        style_internal.animation_mask = @enumFromInt(
            @intFromEnum(style_internal.animation_mask) | @intFromEnum(style.animation_mask),
        );
        mask.animation_mask = true;
    }

    return .{ .style = style_internal, .mask = mask };
}

//------------------------------------------------------------------------------------------
// [UI]: box keys
//------------------------------------------------------------------------------------------

extern fn oc_ui_key_make_str8(string: Str8) UiKey;
extern fn oc_ui_key_make_path(path: Str8List) UiKey;

pub fn uiKeyMake(string: []const u8) UiKey {
    return oc_ui_key_make_str8(Str8.fromSlice(string));
}

pub const uiKeyMakePath = oc_ui_key_make_path;

//------------------------------------------------------------------------------------------
// [UI]: box hierarchy building
//------------------------------------------------------------------------------------------

extern fn oc_ui_box_make_str8(string: Str8, flags: UiFlags) *UiBox;
extern fn oc_ui_box_begin_str8(string: Str8, flags: UiFlags) *UiBox;
extern fn oc_ui_box_end() *UiBox;

extern fn oc_ui_box_push(box: *UiBox) void;
extern fn oc_ui_box_pop() void;
extern fn oc_ui_box_top() ?*UiBox;

extern fn oc_ui_box_lookup_key(key: UiKey) ?*UiBox;
extern fn oc_ui_box_lookup_str8(string: Str8) ?*UiBox;

extern fn oc_ui_box_set_draw_proc(box: UiBox, proc: UiBoxDrawProc, data: ?*anyopaque) void;

pub fn uiBoxMake(string: []const u8, flags: UiFlags) *UiBox {
    return oc_ui_box_make_str8(Str8.fromSlice(string), flags);
}

pub fn uiBoxBegin(string: []const u8, flags: UiFlags) *UiBox {
    return oc_ui_box_begin_str8(Str8.fromSlice(string), flags);
}

pub const uiBoxEnd = oc_ui_box_end;

pub const uiBoxPush = oc_ui_box_push;
pub const uiBoxPop = oc_ui_box_pop;
pub const uiBoxTop = oc_ui_box_top;

pub const uiBoxLookupKey = oc_ui_box_lookup_key;

pub fn uiBoxLookupStr(string: []const u8) ?*UiBox {
    return oc_ui_box_lookup_str8(Str8.fromSlice(string));
}

pub const uiBoxSetDrawProc = oc_ui_box_set_draw_proc;

//------------------------------------------------------------------------------------------
// [UI]: box status and signals
//------------------------------------------------------------------------------------------

extern fn oc_ui_box_closed(box: *UiBox) bool;
extern fn oc_ui_box_set_closed(box: *UiBox, closed: bool) void;

extern fn oc_ui_box_active(box: *UiBox) bool;
extern fn oc_ui_box_activate(box: *UiBox) void;
extern fn oc_ui_box_deactivate(box: *UiBox) void;

extern fn oc_ui_box_hot(box: *UiBox) bool;
extern fn oc_ui_box_set_hot(box: *UiBox, hot: bool) void;

extern fn oc_ui_box_sig(box: *UiBox) UiSig;

pub const uiBoxClosed = oc_ui_box_closed;
pub const uiBoxSetClosed = oc_ui_box_set_closed;

pub const uiBoxActive = oc_ui_box_active;
pub const uiBoxActivate = oc_ui_box_activate;
pub const uiBoxDeactivate = oc_ui_box_deactivate;

pub const uiBoxHot = oc_ui_box_hot;
pub const uiBoxSetHot = oc_ui_box_set_hot;

pub const uiBoxSig = oc_ui_box_sig;

//------------------------------------------------------------------------------------------
// [UI]: tagging
//------------------------------------------------------------------------------------------

extern fn oc_ui_tag_make_str8(string: Str8) UiTag;
extern fn oc_ui_tag_box_str8(box: *UiBox, string: Str8) void;
extern fn oc_ui_tag_next_str8(string: Str8) void;

pub fn uiTagMake(string: []const u8) UiTag {
    return oc_ui_tag_make_str8(Str8.fromSlice(string));
}

pub fn uiTagBox(box: *UiBox, string: []const u8) void {
    oc_ui_tag_box_str8(box, Str8.fromSlice(string));
}

pub fn uiTagNext(string: []const u8) void {
    oc_ui_tag_next_str8(Str8.fromSlice(string));
}

//------------------------------------------------------------------------------------------
// [UI]: styling
//------------------------------------------------------------------------------------------

const UiSelectorDataInternal = extern union {
    text: Str8,
    key: UiKey,
    tag: UiTag,
    status: UiStatus,
};

const UiSelectorInternal = extern struct {
    list_elt: ListElt,
    kind: UiSelectorKind,
    op: UiSelectorOp,
    data: UiSelectorDataInternal,
};

extern fn oc_ui_style_next(style: *UiStyleInternal, mask: UiStyleMaskInternal) void;
extern fn oc_ui_style_match_before(pattern: UiPattern, style: *UiStyleInternal, mask: UiStyleMaskInternal) void;
extern fn oc_ui_style_match_after(patterh: UiPattern, style: *UiStyleInternal, mask: UiStyleMaskInternal) void;

pub fn uiStyleNext(style: UiStyle) void {
    var style_and_mask = uiConvertStyle(&style);
    oc_ui_style_next(&style_and_mask.style, style_and_mask.mask);
}

pub fn uiStyleMatchBefore(pattern: UiPattern, style: UiStyle) void {
    var style_and_mask = uiConvertStyle(&style);
    oc_ui_style_match_before(pattern, &style_and_mask.style, style_and_mask.mask);
}

pub fn uiStyleMatchAfter(pattern: UiPattern, style: UiStyle) void {
    var style_and_mask = uiConvertStyle(&style);
    oc_ui_style_match_after(pattern, &style_and_mask.style, style_and_mask.mask);
}

pub fn uiApplyStyle(dst: *UiStyle, src: *UiStyle) void {
    if (src.size.width) |width| {
        dst.size.width = width;
    }
    if (src.size.height) |height| {
        dst.size.height = height;
    }
    if (src.layout.axis) |axis| {
        dst.layout.axis = axis;
    }
    if (src.layout.alignment.x) |x| {
        dst.layout.alignment.x = x;
    }
    if (src.layout.alignment.y) |y| {
        dst.layout.alignment.y = y;
    }
    if (src.layout.spacing) |spacing| {
        dst.layout.spacing = spacing;
    }
    if (src.layout.margin.x) |x| {
        dst.layout.margin.x = x;
    }
    if (src.layout.margin.y) |y| {
        dst.layout.margin.y = y;
    }
    if (src.floating.x) |x| {
        dst.floating.x = x;
    }
    if (src.float_target.x) |x| {
        dst.float_target.x = x;
    }
    if (src.floating.y) |y| {
        dst.floating.y = y;
    }
    if (src.float_target.y) |y| {
        dst.float_target.y = y;
    }
    if (src.color) |color| {
        dst.color = color;
    }
    if (src.bg_color) |bg_color| {
        dst.bg_color = bg_color;
    }
    if (src.border_color) |border_color| {
        dst.border_color = border_color;
    }
    if (src.border_size) |border_size| {
        dst.border_size = border_size;
    }
    if (src.roudness) |roudness| {
        dst.roudness = roudness;
    }
    if (src.font) |font| {
        dst.font = font;
    }
    if (src.font_size) |font_size| {
        dst.font_size = font_size;
    }
    if (src.animation_time) |animation_time| {
        dst.animation_time = animation_time;
    }
    if (src.animation_mask) |animation_mask| {
        dst.animation_mask = animation_mask;
    }
}

fn uiConvertSelector(selector: UiSelector) UiSelectorInternal {
    var data: UiSelectorDataInternal = switch (selector.sel) {
        .any, .owner => std.mem.zeroes(UiSelectorDataInternal),
        .text => |text| .{ .text = Str8.fromSlice(text) },
        .key => |key| .{ .key = key },
        .tag => |tag| .{ .tag = tag },
        .status => |status| .{ .status = status },
    };

    return UiSelectorInternal{
        .list_elt = .{ .prev = null, .next = null },
        .kind = selector.sel,
        .op = selector.op,
        .data = data,
    };
}

//------------------------------------------------------------------------------------------
// [UI]: basic widget helpers
//------------------------------------------------------------------------------------------

extern fn oc_ui_label_str8(label: Str8) UiSig;
extern fn oc_ui_button_str8(label: Str8) UiSig;
extern fn oc_ui_checkbox_str8(label: Str8, checked: *bool) UiSig;
extern fn oc_ui_slider_str8(label: Str8, value: *f32) *UiBox;
extern fn oc_ui_scrollbar_str8(label: Str8, thumbRatio: f32, scrollValue: *f32) *UiBox;
extern fn oc_ui_tooltip_str8(label: Str8) void;

extern fn oc_ui_panel_begin_str8(name: Str8, flags: UiFlags) void;
extern fn oc_ui_panel_end() void;

extern fn oc_ui_menu_bar_begin_str8(label: Str8) void;
extern fn oc_ui_menu_bar_end() void;
extern fn oc_ui_menu_begin_str8(label: Str8) void;
extern fn oc_ui_menu_end() void;
extern fn oc_ui_menu_button_str8(name: Str8) UiSig;

const UiTextBoxResultInternal = extern struct {
    changed: bool,
    accepted: bool,
    text: Str8,
};
extern fn oc_ui_text_box_str8(name: Str8, arena: Arena, text: Str8) UiTextBoxResultInternal;

const UiSelectPopupInfoInternal = extern struct {
    changed: bool,
    selected_index: c_int,
    option_count: c_int,
    options: [*]Str8,
    placeholder: Str8,
};
extern fn oc_ui_select_popup_str8(name: Str8, info: *UiSelectPopupInfoInternal) UiSelectPopupInfoInternal;

const UiRadioGroupInfoInternal = extern struct {
    changed: bool,
    selected_index: c_int,
    option_count: c_int,
    options: [*]Str8,
};
extern fn oc_ui_radio_group_str8(name: Str8, info: UiRadioGroupInfoInternal) UiRadioGroupInfoInternal;

pub fn uiLabel(label: []const u8) UiSig {
    return oc_ui_label_str8(Str8.fromSlice(label));
}

pub fn uiButton(label: []const u8) UiSig {
    return oc_ui_button_str8(Str8.fromSlice(label));
}

pub fn uiCheckbox(label: []const u8, checked: *bool) UiSig {
    return oc_ui_checkbox_str8(Str8.fromSlice(label), checked);
}

pub fn uiSlider(label: []const u8, value: *f32) *UiBox {
    return oc_ui_slider_str8(Str8.fromSlice(label), value);
}

pub fn uiScrollbar(label: []const u8, thumbRatio: f32, scrollValue: *f32) *UiBox {
    return oc_ui_scrollbar_str8(Str8.fromSlice(label), thumbRatio, scrollValue);
}

pub fn uiTooltip(label: []const u8) void {
    oc_ui_tooltip_str8(Str8.fromSlice(label));
}

pub fn uiPanelBegin(name: []const u8, flags: UiFlags) void {
    oc_ui_panel_begin_str8(Str8.fromSlice(name), flags);
}

pub fn uiPanelEnd() void {
    oc_ui_panel_end();
}

pub fn uiMenuBarBegin(label: []const u8) void {
    oc_ui_menu_bar_begin_str8(Str8.fromSlice(label));
}

pub fn uiMenuBarEnd() void {
    oc_ui_menu_bar_end();
}

pub fn uiMenuBegin(label: []const u8) void {
    oc_ui_menu_begin_str8(Str8.fromSlice(label));
}

pub fn uiMenuEnd() void {
    oc_ui_menu_end();
}

pub fn uiMenuButton(name: []const u8) UiSig {
    return oc_ui_menu_button_str8(Str8.fromSlice(name));
}

pub const UiTextBoxResult = struct {
    changed: bool,
    accepted: bool,
    text: []u8,
};

pub fn uiTextBox(name: []const u8, arena: Arena, text: []const u8) UiTextBoxResult {
    var result_internal = oc_ui_text_box_str8(Str8.fromSlice(name), arena, Str8.fromSlice(text));
    return .{
        .changed = result_internal.changed,
        .accepted = result_internal.accepted,
        .text = result_internal.text.slice(),
    };
}

pub const UiSelectPopupInfo = struct {
    changed: bool = false,
    selected_index: ?usize,
    options: [][]const u8,
    placeholder: []const u8 = "",
};

pub fn uiSelectPopup(name: []const u8, info: *UiSelectPopupInfo) UiSelectPopupInfo {
    var info_internal = UiSelectPopupInfoInternal{
        .changed = info.changed,
        .selected_index = if (info.selected_index) |selected_index| @intCast(selected_index) else -1,
        .option_count = @intCast(info.options.len),
        .options = @ptrCast(info.options.ptr),
        .placeholder = Str8.fromSlice(info.placeholder),
    };
    var result_internal = oc_ui_select_popup_str8(Str8.fromSlice(name), &info_internal);
    return .{
        .changed = result_internal.changed,
        .selected_index = if (result_internal.selected_index >= 0) @intCast(result_internal.selected_index) else null,
        .options = @ptrCast(result_internal.options[0..@intCast(result_internal.option_count)]),
        .placeholder = result_internal.placeholder.slice(),
    };
}

pub const UiRadioGroupInfo = struct {
    changed: bool = false,
    selected_index: ?usize,
    options: [][]const u8,
};

pub fn uiRadioGroup(name: []const u8, info: *UiRadioGroupInfo) UiRadioGroupInfo {
    var info_internal = UiRadioGroupInfoInternal{
        .changed = info.changed,
        .selected_index = if (info.selected_index) |selected_index| @intCast(selected_index) else -1,
        .option_count = @intCast(info.options.len),
        .options = @ptrCast(info.options.ptr),
    };
    var result_internal = oc_ui_radio_group_str8(Str8.fromSlice(name), info_internal);
    return .{
        .changed = result_internal.changed,
        .selected_index = if (result_internal.selected_index >= 0) @intCast(result_internal.selected_index) else null,
        .options = @ptrCast(result_internal.options[0..@intCast(result_internal.option_count)]),
    };
}

//------------------------------------------------------------------------------------------
// [GRAPHICS]: GLES
//------------------------------------------------------------------------------------------

// TODO

//------------------------------------------------------------------------------------------
// [UI]
//------------------------------------------------------------------------------------------

// TODO

//------------------------------------------------------------------------------------------
// [FILE IO] basic API
//------------------------------------------------------------------------------------------

pub const File = extern struct {
    const OpenFlags = packed struct(u16) {
        _: u1 = 0,

        append: bool = false,
        truncate: bool = false,
        create: bool = false,

        symlink: bool = false,
        no_follow: bool = false,
        restrict: bool = false,

        __: u9 = 0,
    };

    const AccessFlags = packed struct(u16) {
        _: u1 = 0,

        read: bool = false,
        write: bool = false,

        __: u13 = 0,
    };

    const Whence = enum(c_uint) {
        Set,
        End,
        Current,
    };

    const Type = enum(c_uint) {
        Unknown,
        Regular,
        Directory,
        Symlink,
        Block,
        Character,
        Fifo,
        Socket,
    };

    const Perm = packed struct(u16) {
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

    const Status = extern struct {
        uid: u64,
        type: Type,
        perm: Perm,
        size: u64,

        creation_date: DateStamp,
        access_date: DateStamp,
        modification_date: DateStamp,
    };

    const DialogKind = enum(c_uint) {
        Save,
        Open,
    };

    const DialogFlags = packed struct(u32) {
        files: bool,
        directories: bool,
        multiple: bool,
        create_directories: bool,
    };

    const DialogDesc = extern struct {
        kind: DialogKind,
        flags: DialogFlags,
        title: Str8,
        ok_label: Str8,
        start_at: File,
        start_path: Str8,
        filters: Str8List,
    };

    const DialogButton = enum(c_uint) {
        Cancel,
        Ok,
    };

    const OpenWithDialogElt = extern struct {
        list_elt: ListElt,
        file: File,
    };

    const OpenWithDialogResult = extern struct {
        button: DialogButton,
        file: File,
        selection: List,
    };

    h: u64,

    extern fn oc_file_nil() File;
    extern fn oc_file_is_nil(file: File) bool;
    extern fn oc_file_open(path: Str8, rights: AccessFlags, flags: OpenFlags) File;
    extern fn oc_file_open_at(dir: File, path: Str8, rights: AccessFlags, flags: OpenFlags) File;
    extern fn oc_file_close(file: File) void;
    extern fn oc_file_last_error(file: File) io.Error;
    extern fn oc_file_pos(file: File) i64;
    extern fn oc_file_seek(file: File, offset: i64, whence: Whence) i64;
    extern fn oc_file_write(file: File, size: u64, buffer: [*]u8) u64;
    extern fn oc_file_read(file: File, size: u64, buffer: [*]u8) u64;

    extern fn oc_file_get_status(file: File) Status;
    extern fn oc_file_size(file: File) u64;

    extern fn oc_file_open_with_request(path: Str8, rights: AccessFlags, flags: OpenFlags) File;
    extern fn oc_file_open_with_dialog(arena: *Arena, rights: AccessFlags, flags: OpenFlags, desc: *DialogDesc) OpenWithDialogResult;

    pub const nil = oc_file_nil;
    pub const isNil = oc_file_is_nil;
    pub const open = oc_file_open;
    pub const openAt = oc_file_open_at;
    pub const close = oc_file_close;
    pub const lastError = oc_file_last_error;
    pub const pos = oc_file_pos;
    pub const seek = oc_file_seek;

    pub fn write(self: File, size: u64, buffer: []u8) u64 {
        assert(size <= buffer.len, "Trying to write more than the buffer length: {d}/{d}", .{ size, buffer.len }, @src());
        return @intCast(oc_file_write(self, size, buffer.ptr));
    }

    pub fn read(self: File, size: u64, buffer: []u8) u64 {
        assert(size <= buffer.len, "Trying to read more than the buffer length: {d}/{d}", .{ size, buffer.len }, @src());
        return @intCast(oc_file_read(self, size, buffer.ptr));
    }

    pub const getStatus = oc_file_get_status;

    pub fn getSize(self: File) usize {
        return @intCast(oc_file_size(self));
    }

    pub const openWithRequest = oc_file_open_with_request;
    pub const openWithDialog = oc_file_open_with_dialog;
};

//------------------------------------------------------------------------------------------
// [FILE IO] low-level io queue api
//------------------------------------------------------------------------------------------

pub const io = struct {
    pub const ReqId = u16;
    pub const Op = u32;

    pub const OpEnum = enum(c_uint) {
        OpenAt = 0,
        Close,
        FStat,
        Seek,
        Read,
        Write,
        Error,
    };

    pub const Req = extern struct {
        id: ReqId,
        op: Op,
        handle: File,

        offset: i64,
        size: u64,

        buffer: extern union {
            data: ?[*]u8,
            unused: u64, // This is a horrible hack to get the same layout on wasm and on host
        },

        type: extern union {
            open: extern struct {
                rights: File.AccessFlags,
                flags: File.OpenFlags,
            },
            whence: File.Whence,
        },
    };

    pub const Error = enum(i32) {
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

    pub const Cmp = extern struct {
        id: ReqId,
        err: Error,
        data: extern union {
            result: i64,
            size: u64,
            offset: i64,
            handle: File,
        },
    };

    extern fn oc_io_wait_single_req(req: *Req) Cmp;

    pub const waitSingleReq = oc_io_wait_single_req;
};
