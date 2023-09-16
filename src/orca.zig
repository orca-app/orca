const std = @import("std");
const orca_c = @cImport({
    @cDefine("__ORCA__", "");
    @cInclude("orca.h");
});

pub const Str8 = orca_c.oc_str8;
pub const Str32 = orca_c.oc_str32;
pub const Utf32 = orca_c.oc_utf32;
pub const Vec2 = extern struct {
    x: f32,
    y: f32,
};

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
// [DEBUG]
//------------------------------------------------------------------------------------------

pub fn logInfo(comptime fmt: []const u8, args: anytype, source: std.builtin.SourceLocation) void {
    logExt(orca_c.OC_LOG_LEVEL_INFO, fmt, args, source);
}

pub fn logWarning(comptime fmt: []const u8, args: anytype, source: std.builtin.SourceLocation) void {
    logExt(orca_c.OC_LOG_LEVEL_WARNING, fmt, args, source);
}

pub fn logError(comptime fmt: []const u8, args: anytype, source: std.builtin.SourceLocation) void {
    logExt(orca_c.OC_LOG_LEVEL_ERROR, fmt, args, source);
}

pub fn logExt(comptime level: orca_c.oc_log_level, comptime fmt: []const u8, args: anytype, source: std.builtin.SourceLocation) void {
    var format_buf: [512:0]u8 = undefined;
    _ = std.fmt.bufPrintZ(&format_buf, fmt, args) catch 0; // just discard NoSpaceLeft error for now
    var line: c_int = @intCast(source.line);

    orca_c.oc_log_ext(level, source.fn_name.ptr, source.file.ptr, line, format_buf[0..].ptr);
}

pub fn assert(condition: bool, comptime fmt: []const u8, args: anytype, source: std.builtin.SourceLocation) void {
    if (condition == false) {
        var format_buf: [512:0]u8 = undefined;
        _ = std.fmt.bufPrintZ(&format_buf, fmt, args) catch 0;
        var line: c_int = @intCast(source.line);

        orca_c.oc_assert_fail(source.file.ptr, source.fn_name.ptr, line, "", format_buf[0..].ptr);
    }
}

pub fn abort(comptime fmt: []const u8, args: anytype, source: std.builtin.SourceLocation) void {
    var format_buf: [512:0]u8 = undefined;
    _ = std.fmt.bufPrintZ(&format_buf, fmt, args) catch 0;
    var line: c_int = @intCast(source.line);

    orca_c.oc_abort_ext(source.file.ptr, source.fn_name.ptr, line, format_buf[0..].ptr);
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
// [STRINGS]
//------------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------------
// [UTF8]
//------------------------------------------------------------------------------------------

const UnicodeRange = extern struct {
    first_code_point: Utf32,
    count: u32,
};

const Utf8Dec = extern struct {
    codepoint: Utf32, // decoded codepoint
    size: u32, // size of corresponding oc_utf8 sequence
};

const Utf8 = struct {

    // getting sizes / offsets / indices
    extern fn oc_utf8_size_from_leading_char(leadingChar: c_char) u32;
    extern fn oc_utf8_codepoint_size(codePoint: Utf32) u32;
    extern fn oc_utf8_codepoint_count_for_string(string: Str8) u64;
    extern fn oc_utf8_byte_count_for_codepoints(codePoints: Str32) u64;
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
    extern fn oc_utf8_encode(dst: [*]u8, codePoint: Utf32) Str8; //NOTE: encode codepoint into backing buffer dst
    extern fn oc_utf8_to_codepoints(maxCount: u64, backing: [*]Utf32, string: Str8) Str32;
    extern fn oc_utf8_from_codepoints(maxBytes: u64, backing: [*]c_char, codePoints: Str32) Str8;
    extern fn oc_utf8_push_to_codepoints(arena: *Arena, string: Str8) Str32;
    extern fn oc_utf8_push_from_codepoints(arena: *Arena, codePoints: Str32) Str8;

    pub const decode = oc_utf8_decode;
    pub const decodeAt = oc_utf8_decode_at;
    pub const encode = oc_utf8_encode;
    pub const toCodepoints = oc_utf8_to_codepoints;
    pub const fromCodepoints = oc_utf8_from_codepoints;
    pub const pushToCodepoints = oc_utf8_push_to_codepoints;
    pub const pushFromCodepoints = oc_utf8_push_from_codepoints;
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

pub const KeyCode = enum(c_uint) {
    Unknown = orca_c.OC_KEY_UNKNOWN,
    Space = orca_c.OC_KEY_SPACE,
    Apostrophe = orca_c.OC_KEY_APOSTROPHE,
    Comma = orca_c.OC_KEY_COMMA,
    Minus = orca_c.OC_KEY_MINUS,
    Period = orca_c.OC_KEY_PERIOD,
    Slash = orca_c.OC_KEY_SLASH,
    Num0 = orca_c.OC_KEY_0,
    Num1 = orca_c.OC_KEY_1,
    Num2 = orca_c.OC_KEY_2,
    Num3 = orca_c.OC_KEY_3,
    Num4 = orca_c.OC_KEY_4,
    Num5 = orca_c.OC_KEY_5,
    Num6 = orca_c.OC_KEY_6,
    Num7 = orca_c.OC_KEY_7,
    Num8 = orca_c.OC_KEY_8,
    Num9 = orca_c.OC_KEY_9,
    Semicolon = orca_c.OC_KEY_SEMICOLON,
    Equal = orca_c.OC_KEY_EQUAL,
    A = orca_c.OC_KEY_A,
    B = orca_c.OC_KEY_B,
    C = orca_c.OC_KEY_C,
    D = orca_c.OC_KEY_D,
    E = orca_c.OC_KEY_E,
    F = orca_c.OC_KEY_F,
    G = orca_c.OC_KEY_G,
    H = orca_c.OC_KEY_H,
    I = orca_c.OC_KEY_I,
    J = orca_c.OC_KEY_J,
    K = orca_c.OC_KEY_K,
    L = orca_c.OC_KEY_L,
    M = orca_c.OC_KEY_M,
    N = orca_c.OC_KEY_N,
    O = orca_c.OC_KEY_O,
    P = orca_c.OC_KEY_P,
    Q = orca_c.OC_KEY_Q,
    R = orca_c.OC_KEY_R,
    S = orca_c.OC_KEY_S,
    T = orca_c.OC_KEY_T,
    U = orca_c.OC_KEY_U,
    V = orca_c.OC_KEY_V,
    W = orca_c.OC_KEY_W,
    X = orca_c.OC_KEY_X,
    Y = orca_c.OC_KEY_Y,
    Z = orca_c.OC_KEY_Z,
    LeftBracket = orca_c.OC_KEY_LEFT_BRACKET,
    Backslash = orca_c.OC_KEY_BACKSLASH,
    RightBracket = orca_c.OC_KEY_RIGHT_BRACKET,
    GraveAccent = orca_c.OC_KEY_GRAVE_ACCENT,
    World1 = orca_c.OC_KEY_WORLD_1,
    World2 = orca_c.OC_KEY_WORLD_2,
    Escape = orca_c.OC_KEY_ESCAPE,
    Enter = orca_c.OC_KEY_ENTER,
    Tab = orca_c.OC_KEY_TAB,
    Backspace = orca_c.OC_KEY_BACKSPACE,
    Insert = orca_c.OC_KEY_INSERT,
    Delete = orca_c.OC_KEY_DELETE,
    Right = orca_c.OC_KEY_RIGHT,
    Left = orca_c.OC_KEY_LEFT,
    Down = orca_c.OC_KEY_DOWN,
    Up = orca_c.OC_KEY_UP,
    PageUp = orca_c.OC_KEY_PAGE_UP,
    PageDown = orca_c.OC_KEY_PAGE_DOWN,
    Home = orca_c.OC_KEY_HOME,
    End = orca_c.OC_KEY_END,
    CapsLock = orca_c.OC_KEY_CAPS_LOCK,
    ScrollLock = orca_c.OC_KEY_SCROLL_LOCK,
    NumLock = orca_c.OC_KEY_NUM_LOCK,
    PrintScreen = orca_c.OC_KEY_PRINT_SCREEN,
    Pause = orca_c.OC_KEY_PAUSE,
    F1 = orca_c.OC_KEY_F1,
    F2 = orca_c.OC_KEY_F2,
    F3 = orca_c.OC_KEY_F3,
    F4 = orca_c.OC_KEY_F4,
    F5 = orca_c.OC_KEY_F5,
    F6 = orca_c.OC_KEY_F6,
    F7 = orca_c.OC_KEY_F7,
    F8 = orca_c.OC_KEY_F8,
    F9 = orca_c.OC_KEY_F9,
    F10 = orca_c.OC_KEY_F10,
    F11 = orca_c.OC_KEY_F11,
    F12 = orca_c.OC_KEY_F12,
    F13 = orca_c.OC_KEY_F13,
    F14 = orca_c.OC_KEY_F14,
    F15 = orca_c.OC_KEY_F15,
    F16 = orca_c.OC_KEY_F16,
    F17 = orca_c.OC_KEY_F17,
    F18 = orca_c.OC_KEY_F18,
    F19 = orca_c.OC_KEY_F19,
    F20 = orca_c.OC_KEY_F20,
    F21 = orca_c.OC_KEY_F21,
    F22 = orca_c.OC_KEY_F22,
    F23 = orca_c.OC_KEY_F23,
    F24 = orca_c.OC_KEY_F24,
    F25 = orca_c.OC_KEY_F25,
    Kp0 = orca_c.OC_KEY_KP_0,
    Kp1 = orca_c.OC_KEY_KP_1,
    Kp2 = orca_c.OC_KEY_KP_2,
    Kp3 = orca_c.OC_KEY_KP_3,
    Kp4 = orca_c.OC_KEY_KP_4,
    Kp5 = orca_c.OC_KEY_KP_5,
    Kp6 = orca_c.OC_KEY_KP_6,
    Kp7 = orca_c.OC_KEY_KP_7,
    Kp8 = orca_c.OC_KEY_KP_8,
    Kp9 = orca_c.OC_KEY_KP_9,
    KpDecimal = orca_c.OC_KEY_KP_DECIMAL,
    KpDivide = orca_c.OC_KEY_KP_DIVIDE,
    KpMultiply = orca_c.OC_KEY_KP_MULTIPLY,
    KpSubtract = orca_c.OC_KEY_KP_SUBTRACT,
    KpAdd = orca_c.OC_KEY_KP_ADD,
    KpEnter = orca_c.OC_KEY_KP_ENTER,
    KpEqual = orca_c.OC_KEY_KP_EQUAL,
    LeftShift = orca_c.OC_KEY_LEFT_SHIFT,
    LeftControl = orca_c.OC_KEY_LEFT_CONTROL,
    LeftAlt = orca_c.OC_KEY_LEFT_ALT,
    LeftSuper = orca_c.OC_KEY_LEFT_SUPER,
    RightShift = orca_c.OC_KEY_RIGHT_SHIFT,
    RightControl = orca_c.OC_KEY_RIGHT_CONTROL,
    RightAlt = orca_c.OC_KEY_RIGHT_ALT,
    RightSuper = orca_c.OC_KEY_RIGHT_SUPER,
    Menu = orca_c.OC_KEY_MENU,
    Count = orca_c.OC_KEY_COUNT,
};

pub const MouseButton = enum(c_uint) {
    Left = orca_c.OC_MOUSE_LEFT,
    Right = orca_c.OC_MOUSE_RIGHT,
    Middle = orca_c.OC_MOUSE_MIDDLE,
    Ext1 = orca_c.OC_MOUSE_EXT1,
    Ext2 = orca_c.OC_MOUSE_EXT2,
};

//------------------------------------------------------------------------------------------
// [APP] windows
//------------------------------------------------------------------------------------------

pub fn windowSetTitle(title: [:0]const u8) void {
    var title_str8: Str8 = .{
        .ptr = @constCast(title.ptr),
        .len = title.len,
    };
    orca_c.oc_window_set_title(title_str8);
}

extern fn oc_window_set_size(size: Vec2) void;
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

// typedef struct oc_file_dialog_desc
// {
//     oc_file_dialog_kind kind;
//     oc_file_dialog_flags flags;
//     oc_str8 title;
//     oc_str8 okLabel;
//     oc_file startAt;
//     oc_str8 startPath;
//     oc_str8_list filters;

//     //... later customization options with checkboxes / radiobuttons
// } oc_file_dialog_desc;

const FileDialogButton = enum(c_uint) {
    Cancel,
    Ok,
};

const FileDialogResult = extern struct {
    button: FileDialogButton,
    path: Str8,
    selection: Str8List,
};

// typedef enum
// {
//     OC_FILE_DIALOG_CANCEL = 0,
//     OC_FILE_DIALOG_OK,
// } oc_file_dialog_button;

// typedef struct oc_file_dialog_result
// {
//     oc_file_dialog_button button;
//     oc_str8 path;
//     oc_str8_list selection;

// } oc_file_dialog_result;

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
// [GRAPHICS]: resources
//------------------------------------------------------------------------------------------

pub const Surface = extern struct {
    h: u64,

    extern fn oc_surface_canvas() Surface;
    extern fn oc_surface_gles() Surface;
    extern fn oc_surface_select(surface: Surface) void;
    extern fn oc_surface_present(surface: Surface) void;
    extern fn oc_surface_get_size(surface: Surface) Vec2;
    extern fn oc_surface_contents_scaling(surface: Surface) Vec2;
    extern fn oc_surface_bring_to_front(surface: Surface) void;
    extern fn oc_surface_send_to_back(surface: Surface) void;
    // extern fn oc_surface_render_commands(
    //     surface: Surface,
    //     color: Color,
    //     primitive_count: u32,
    //     primitives: [*]Primitive,
    //     elt_count: u32,
    //     elements: [*]PathElt,
    // ) void;
    extern fn oc_render(surface: Surface, canvas: Canvas) void;

    pub const canvas = oc_surface_canvas;
    pub const gles = oc_surface_gles;
    pub const select = oc_surface_select;
    pub const present = oc_surface_present;
    pub const getSize = oc_surface_get_size;
    pub const contentsScaling = oc_surface_contents_scaling;
    pub const bringToFront = oc_surface_bring_to_front;
    pub const sendToBack = oc_surface_send_to_back;
    // pub const renderCommands = oc_surface_render_commands;
    pub const render = oc_render;
};

pub const Canvas = extern struct {
    h: u64,

    extern fn oc_canvas_nil() Canvas;
    extern fn oc_canvas_is_nil(canvas: Canvas) bool;
    extern fn oc_canvas_create() Canvas;
    extern fn oc_canvas_destroy(canvas: Canvas) void;
    extern fn oc_canvas_set_current(canvas: Canvas) Canvas;

    pub const nil = oc_canvas_nil;
    pub const isNil = oc_canvas_is_nil;
    pub const create = oc_canvas_create;
    pub const destroy = oc_canvas_destroy;
    pub const setCurrent = oc_canvas_set_current;
};

pub const Image = extern struct {
    h: u64,

    pub const create = oc_image_create;
    pub const destroy = oc_image_destroy;
    pub const uploadRegionRGBA = oc_image_upload_region_rgba8;
    pub const size = oc_image_size;

    extern fn oc_image_create(surface: Surface, width: u32, height: u32) Image;
    extern fn oc_image_destroy(image: Image) void;
    extern fn oc_image_upload_region_rgba8(image: Image, region: Rect, pixels: [*]u8) void;
    extern fn oc_image_size(image: Image) Vec2;
    extern fn oc_image_draw(image: Image, rect: Rect) void;
    extern fn oc_image_draw_region(image: Image, srcRegion: Rect, dstRegion: Rect) void;

    pub const create = oc_image_create;
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
// [GRAPHICS]: fonts
//------------------------------------------------------------------------------------------

//NOTE(martin): the following int valued functions return -1 if font is invalid or codepoint is not present in font//
//TODO(martin): add enum error codes
//NOTE(martin): if you need to process more than one codepoint, first convert your codepoints to glyph indices, then use the
//              glyph index versions of the functions, which can take an array of glyph indices.

const Font = extern struct {
    h: u64,

    extern fn oc_font_nil() Font;
    extern fn oc_font_create_from_memory(mem: Str8, rangeCount: u32, ranges: [*]UnicodeRange) Font;
    extern fn oc_font_destroy(font: Font) void;
    extern fn oc_font_get_extents(font: Font) FontExtents;
    extern fn oc_font_get_scaled_extents(font: Font, emSize: f32) FontExtents;
    extern fn oc_font_get_scale_for_em_pixels(font: Font, emSize: f32) f32;
    extern fn oc_font_get_glyph_indices(font: Font, codePoints: Str32, backing: Str32) Str32;
    extern fn oc_font_push_glyph_indices(font: Font, arena: *Arena, codePoints: Str32) Str32;
    extern fn oc_font_get_glyph_index(font: Font, codePoint: Utf32) u32;
    extern fn oc_font_get_codepoint_extents(font: Font, codePoint: Utf32, outExtents: *TextExtents) c_int;
    extern fn oc_font_get_glyph_extents(font: Font, glyphIndices: Str32, outExtents: *TextExtents) c_int;
    extern fn oc_text_bounding_box_utf32(font: Font, fontSize: f32, text: Str32) Rect;
    extern fn oc_text_bounding_box(font: Font, fontSize: f32, text: Str8) Rect;

    pub const nil = oc_font_nil;
    pub fn createFromMemory(mem: Str8, ranges: []UnicodeRange) Font {
        return oc_font_create_from_memory(mem, @intCast(ranges.len), ranges.ptr);
    }
    pub const destroy = oc_font_destroy;
    pub const getExtents = oc_font_get_extents;
    pub const getScaledExtents = oc_font_get_scaled_extents;
    pub const getScaleForEmPixels = oc_font_get_scale_for_em_pixels;
    pub const getGlyphIndices = oc_font_get_glyph_indices;
    pub const pushGlyphIndices = oc_font_push_glyph_indices;
    pub const getGlyphIndex = oc_font_get_glyph_index;
    pub const getCodepointExtents = oc_font_get_codepoint_extents;
    pub const getGlyphExtents = oc_font_get_glyph_extents;
    pub const boundingBoxUtf32 = oc_text_bounding_box_utf32;
    pub const boundingBox = oc_text_bounding_box;
};

const JointType = enum(c_uint) {
    Miter,
    Bevel,
    None,
};

const CapType = enum(c_uint) {
    None,
    Square,
};

const FontExtents = extern struct {
    ascent: f32, // the extent above the baseline (by convention a positive value extends above the baseline)
    descent: f32, // the extent below the baseline (by convention, positive value extends below the baseline)
    leading: f32, // spacing between one row's descent and the next row's ascent
    x_height: f32, // height of the lower case letter 'x'
    cap_height: f32, // height of the upper case letter 'M'
    width: f32, // maximum width of the font
};

const TextExtents = extern struct {
    x_bearing: f32,
    y_bearing: f32,
    width: f32,
    height: f32,
    x_advance: f32,
    y_advance: f32,
};

//------------------------------------------------------------------------------------------
// [GRAPHICS] Matrix / Clip stack
//------------------------------------------------------------------------------------------

pub const Mat2x3 = extern struct {
    m: [6]f32,

    extern fn oc_matrix_push(matrix: Mat2x3) void;
    extern fn oc_matrix_pop() void;
    extern fn oc_matrix_top() Mat2x3;

    pub const push = oc_matrix_push;
    pub const pop = oc_matrix_pop;
    pub const top = oc_matrix_top;
};

pub const clip = struct {
    extern fn oc_clip_push(x: f32, y: f32, w: f32, h: f32) void;
    extern fn oc_clip_pop() void;
    extern fn oc_clip_top() Rect;

    pub const push = oc_clip_push;
    pub const pop = oc_clip_pop;
    pub const top = oc_clip_top;
};

//------------------------------------------------------------------------------------------
// [GRAPHICS]: graphics attributes setting/getting
//------------------------------------------------------------------------------------------

extern fn oc_set_color(color: Color) void;
extern fn oc_set_color_rgba(r: f32, g: f32, b: f32, a: f32) void;
extern fn oc_set_width(width: f32) void;
extern fn oc_set_tolerance(tolerance: f32) void;
extern fn oc_set_joint(joint: JointType) void;
extern fn oc_set_max_joint_excursion(maxJointExcursion: f32) void;
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

extern fn oc_glyph_outlines(glyphIndices: Str32) Rect;
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
extern fn oc_arc(x: f32, y: f32, r: f32, arcAngle: f32, startAngle: f32) void;

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

    const getStatus = oc_file_get_status;
    const size = oc_file_size;
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

//------------------------------------------------------------------------------------------
// [FILE IO] complete io queue api
//------------------------------------------------------------------------------------------

extern fn oc_io_wait_single_req(req: *IoReq) IoCmp;

pub const ioWaitSingleReq = oc_io_wait_single_req;

//------------------------------------------------------------------------------------------
// [FILE IO] Asking users for file capabilities
//------------------------------------------------------------------------------------------

const FileOpenWithDialogElt = extern struct {
    list_elt: ListElt,
    file: File,
};

const FileOpenWithDialogResult = extern struct {
    button: FileDialogButton,
    file: File,
    selection: List,
};

extern fn oc_file_open_with_request(path: Str8, rights: FileAccessFlags, flags: FileOpenFlags) File;
extern fn oc_file_open_with_dialog(arena: *Arena, rights: FileAccessFlags, flags: FileOpenFlags, desc: *FileDialogDesc) FileOpenWithDialogResult;
