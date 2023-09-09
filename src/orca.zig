const c = @cImport({
    @cDefine("__ORCA__", "");
    @cInclude("orca.h");
});

// export var oc_rawEvent: c.oc_event = c.oc_rawEvent;

pub fn log_info(fmt: []const u8) void {
    c.oc_log_ext(c.OC_LOG_LEVEL_INFO, "UnknownFunc", "UnknownFile", 0, fmt.ptr);
}

// ORCA_API void oc_log_ext(oc_log_level level,
//                          const char* function,
//                          const char* file,
//                          int line,
//                          const char* fmt,
//                          ...);
