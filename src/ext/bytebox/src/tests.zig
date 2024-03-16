const std = @import("std");
const core = @import("core.zig");

test "stack vm: integration" {
    const wasm_filepath = "zig-out/lib/mandelbrot.wasm";

    var allocator = std.testing.allocator;

    var cwd = std.fs.cwd();
    var wasm_data: []u8 = try cwd.readFileAlloc(allocator, wasm_filepath, 1024 * 1024 * 128);
    defer allocator.free(wasm_data);

    const module_def_opts = core.ModuleDefinitionOpts{
        .debug_name = std.fs.path.basename(wasm_filepath),
    };
    var module_def = core.ModuleDefinition.init(allocator, module_def_opts);
    defer module_def.deinit();

    try module_def.decode(wasm_data);

    var module_inst = try core.ModuleInstance.init(&module_def, allocator);
    defer module_inst.deinit();
}
