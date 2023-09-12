const std = @import("std");
const orca_c = @cImport({
    @cDefine("__ORCA__", "");
    @cInclude("orca.h");
});

pub const str8 = orca_c.oc_str8;
pub const vec2 = struct {
    x: f32,
    y: f32,
};

///////////////////////////////////////////////////////////////////////////////////////////////////
// platform

pub const Platform = enum(c_uint) {
    MacOS,
    Windows,
};

pub fn getHostPlatform() Platform {
    return @enumFromInt(orca_c.oc_get_host_platform());
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// platform debug

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

        orca_c.oc_assert_fail(source.file.ptr, source.fn_name.ptr, line, "<unknown failure>", format_buf[0..].ptr);
    }
}

pub fn abort(comptime fmt: []const u8, args: anytype, source: std.builtin.SourceLocation) void {
    var format_buf: [512:0]u8 = undefined;
    _ = std.fmt.bufPrintZ(&format_buf, fmt, args) catch 0;
    var line: c_int = @intCast(source.line);

    orca_c.oc_abort_ext(source.file.ptr, source.fn_name.ptr, line, format_buf[0..].ptr);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// app

pub const requestQuit = orca_c.oc_request_quit;

const MouseCursor = enum(c_uint) {
    Arrow,
    Resize0,
    Resize90,
    Resize45,
    Resize135,
    Text,
};
pub fn setCursor(cursor: MouseCursor) void {
    orca_c.oc_set_cursor(@intFromEnum(cursor));
}

pub const KeyCodes = struct {
    pub const unknown: c_int = orca_c.OC_KEY_UNKNOWN;
    pub const space: c_int = orca_c.OC_KEY_SPACE;
    pub const apostrophe: c_int = orca_c.OC_KEY_APOSTROPHE;
    pub const comma: c_int = orca_c.OC_KEY_COMMA;
    pub const minus: c_int = orca_c.OC_KEY_MINUS;
    pub const period: c_int = orca_c.OC_KEY_PERIOD;
    pub const slash: c_int = orca_c.OC_KEY_SLASH;
    pub const num_0: c_int = orca_c.OC_KEY_0;
    pub const num_1: c_int = orca_c.OC_KEY_1;
    pub const num_2: c_int = orca_c.OC_KEY_2;
    pub const num_3: c_int = orca_c.OC_KEY_3;
    pub const num_4: c_int = orca_c.OC_KEY_4;
    pub const num_5: c_int = orca_c.OC_KEY_5;
    pub const num_6: c_int = orca_c.OC_KEY_6;
    pub const num_7: c_int = orca_c.OC_KEY_7;
    pub const num_8: c_int = orca_c.OC_KEY_8;
    pub const num_9: c_int = orca_c.OC_KEY_9;
    pub const semicolon: c_int = orca_c.OC_KEY_SEMICOLON;
    pub const equal: c_int = orca_c.OC_KEY_EQUAL;
    pub const a: c_int = orca_c.OC_KEY_A;
    pub const b: c_int = orca_c.OC_KEY_B;
    pub const c: c_int = orca_c.OC_KEY_C;
    pub const d: c_int = orca_c.OC_KEY_D;
    pub const e: c_int = orca_c.OC_KEY_E;
    pub const f: c_int = orca_c.OC_KEY_F;
    pub const g: c_int = orca_c.OC_KEY_G;
    pub const h: c_int = orca_c.OC_KEY_H;
    pub const i: c_int = orca_c.OC_KEY_I;
    pub const j: c_int = orca_c.OC_KEY_J;
    pub const k: c_int = orca_c.OC_KEY_K;
    pub const l: c_int = orca_c.OC_KEY_L;
    pub const m: c_int = orca_c.OC_KEY_M;
    pub const n: c_int = orca_c.OC_KEY_N;
    pub const o: c_int = orca_c.OC_KEY_O;
    pub const p: c_int = orca_c.OC_KEY_P;
    pub const q: c_int = orca_c.OC_KEY_Q;
    pub const r: c_int = orca_c.OC_KEY_R;
    pub const s: c_int = orca_c.OC_KEY_S;
    pub const t: c_int = orca_c.OC_KEY_T;
    pub const u: c_int = orca_c.OC_KEY_U;
    pub const v: c_int = orca_c.OC_KEY_V;
    pub const w: c_int = orca_c.OC_KEY_W;
    pub const x: c_int = orca_c.OC_KEY_X;
    pub const y: c_int = orca_c.OC_KEY_Y;
    pub const z: c_int = orca_c.OC_KEY_Z;
    pub const left_bracket: c_int = orca_c.OC_KEY_LEFT_BRACKET;
    pub const backslash: c_int = orca_c.OC_KEY_BACKSLASH;
    pub const right_bracket: c_int = orca_c.OC_KEY_RIGHT_BRACKET;
    pub const grave_accent: c_int = orca_c.OC_KEY_GRAVE_ACCENT;
    pub const world_1: c_int = orca_c.OC_KEY_WORLD_1;
    pub const world_2: c_int = orca_c.OC_KEY_WORLD_2;
    pub const escape: c_int = orca_c.OC_KEY_ESCAPE;
    pub const enter: c_int = orca_c.OC_KEY_ENTER;
    pub const tab: c_int = orca_c.OC_KEY_TAB;
    pub const backspace: c_int = orca_c.OC_KEY_BACKSPACE;
    pub const insert: c_int = orca_c.OC_KEY_INSERT;
    pub const delete: c_int = orca_c.OC_KEY_DELETE;
    pub const right: c_int = orca_c.OC_KEY_RIGHT;
    pub const left: c_int = orca_c.OC_KEY_LEFT;
    pub const down: c_int = orca_c.OC_KEY_DOWN;
    pub const up: c_int = orca_c.OC_KEY_UP;
    pub const page_up: c_int = orca_c.OC_KEY_PAGE_UP;
    pub const page_down: c_int = orca_c.OC_KEY_PAGE_DOWN;
    pub const home: c_int = orca_c.OC_KEY_HOME;
    pub const end: c_int = orca_c.OC_KEY_END;
    pub const caps_lock: c_int = orca_c.OC_KEY_CAPS_LOCK;
    pub const scroll_lock: c_int = orca_c.OC_KEY_SCROLL_LOCK;
    pub const num_lock: c_int = orca_c.OC_KEY_NUM_LOCK;
    pub const print_screen: c_int = orca_c.OC_KEY_PRINT_SCREEN;
    pub const pause: c_int = orca_c.OC_KEY_PAUSE;
    pub const function_1: c_int = orca_c.OC_KEY_F1;
    pub const function_2: c_int = orca_c.OC_KEY_F2;
    pub const function_3: c_int = orca_c.OC_KEY_F3;
    pub const function_4: c_int = orca_c.OC_KEY_F4;
    pub const function_5: c_int = orca_c.OC_KEY_F5;
    pub const function_6: c_int = orca_c.OC_KEY_F6;
    pub const function_7: c_int = orca_c.OC_KEY_F7;
    pub const function_8: c_int = orca_c.OC_KEY_F8;
    pub const function_9: c_int = orca_c.OC_KEY_F9;
    pub const function_10: c_int = orca_c.OC_KEY_F10;
    pub const function_11: c_int = orca_c.OC_KEY_F11;
    pub const function_12: c_int = orca_c.OC_KEY_F12;
    pub const function_13: c_int = orca_c.OC_KEY_F13;
    pub const function_14: c_int = orca_c.OC_KEY_F14;
    pub const function_15: c_int = orca_c.OC_KEY_F15;
    pub const function_16: c_int = orca_c.OC_KEY_F16;
    pub const function_17: c_int = orca_c.OC_KEY_F17;
    pub const function_18: c_int = orca_c.OC_KEY_F18;
    pub const function_19: c_int = orca_c.OC_KEY_F19;
    pub const function_20: c_int = orca_c.OC_KEY_F20;
    pub const function_21: c_int = orca_c.OC_KEY_F21;
    pub const function_22: c_int = orca_c.OC_KEY_F22;
    pub const function_23: c_int = orca_c.OC_KEY_F23;
    pub const function_24: c_int = orca_c.OC_KEY_F24;
    pub const function_25: c_int = orca_c.OC_KEY_F25;
    pub const kp_0: c_int = orca_c.OC_KEY_KP_0;
    pub const kp_1: c_int = orca_c.OC_KEY_KP_1;
    pub const kp_2: c_int = orca_c.OC_KEY_KP_2;
    pub const kp_3: c_int = orca_c.OC_KEY_KP_3;
    pub const kp_4: c_int = orca_c.OC_KEY_KP_4;
    pub const kp_5: c_int = orca_c.OC_KEY_KP_5;
    pub const kp_6: c_int = orca_c.OC_KEY_KP_6;
    pub const kp_7: c_int = orca_c.OC_KEY_KP_7;
    pub const kp_8: c_int = orca_c.OC_KEY_KP_8;
    pub const kp_9: c_int = orca_c.OC_KEY_KP_9;
    pub const kp_decimal: c_int = orca_c.OC_KEY_KP_DECIMAL;
    pub const kp_divide: c_int = orca_c.OC_KEY_KP_DIVIDE;
    pub const kp_multiply: c_int = orca_c.OC_KEY_KP_MULTIPLY;
    pub const kp_subtract: c_int = orca_c.OC_KEY_KP_SUBTRACT;
    pub const kp_add: c_int = orca_c.OC_KEY_KP_ADD;
    pub const kp_enter: c_int = orca_c.OC_KEY_KP_ENTER;
    pub const kp_equal: c_int = orca_c.OC_KEY_KP_EQUAL;
    pub const left_shift: c_int = orca_c.OC_KEY_LEFT_SHIFT;
    pub const left_control: c_int = orca_c.OC_KEY_LEFT_CONTROL;
    pub const left_alt: c_int = orca_c.OC_KEY_LEFT_ALT;
    pub const left_super: c_int = orca_c.OC_KEY_LEFT_SUPER;
    pub const right_shift: c_int = orca_c.OC_KEY_RIGHT_SHIFT;
    pub const right_control: c_int = orca_c.OC_KEY_RIGHT_CONTROL;
    pub const right_alt: c_int = orca_c.OC_KEY_RIGHT_ALT;
    pub const right_super: c_int = orca_c.OC_KEY_RIGHT_SUPER;
    pub const menu: c_int = orca_c.OC_KEY_MENU;
    pub const count: c_int = orca_c.OC_KEY_COUNT;
};

pub const MouseButtons = struct {
    pub const left: c_int = orca_c.OC_MOUSE_LEFT;
    pub const right: c_int = orca_c.OC_MOUSE_RIGHT;
    pub const middle: c_int = orca_c.OC_MOUSE_MIDDLE;
    pub const ext1: c_int = orca_c.OC_MOUSE_EXT1;
    pub const ext2: c_int = orca_c.OC_MOUSE_EXT2;
};

pub fn windowSetTitle(title: [:0]const u8) void {
    var title_str8: str8 = .{
        .ptr = @constCast(title.ptr),
        .len = title.len,
    };
    orca_c.oc_window_set_title(title_str8);
}

pub fn windowSetSize(size: vec2) void {
    const c_size: *const orca_c.oc_vec2 = @ptrCast(&size);
    orca_c.oc_window_set_size(c_size.*);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// clock

pub const Clock = enum(c_uint) {
    Monotonic,
    Uptime,
    Date,
};

pub fn clockTime(clock: Clock) f64 {
    return orca_c.oc_clock_time(@intFromEnum(clock));
}
