const std = @import("std");
const testing = std.testing;
const expectEqual = testing.expectEqual;

const core = @import("core.zig");
const Limits = core.Limits;
const MemoryInstance = core.MemoryInstance;

test "StackVM.Integration" {
    const wasm_filepath = "zig-out/lib/mandelbrot.wasm";

    var allocator = std.testing.allocator;

    var cwd = std.fs.cwd();
    var wasm_data: []u8 = try cwd.readFileAlloc(allocator, wasm_filepath, 1024 * 1024 * 128);
    defer allocator.free(wasm_data);

    const module_def_opts = core.ModuleDefinitionOpts{
        .debug_name = std.fs.path.basename(wasm_filepath),
    };
    var module_def = try core.createModuleDefinition(allocator, module_def_opts);
    defer module_def.destroy();

    try module_def.decode(wasm_data);

    var module_inst = try core.createModuleInstance(.Stack, module_def, allocator);
    defer module_inst.destroy();
}

test "MemoryInstance.init" {
    {
        const limits = Limits{
            .min = 0,
            .max = null,
        };
        var memory = MemoryInstance.init(limits, null);
        defer memory.deinit();
        try expectEqual(memory.limits.min, 0);
        try expectEqual(memory.limits.max, MemoryInstance.k_max_pages);
        try expectEqual(memory.size(), 0);
        try expectEqual(memory.mem.Internal.items.len, 0);
    }

    {
        const limits = Limits{
            .min = 25,
            .max = 25,
        };
        var memory = MemoryInstance.init(limits, null);
        defer memory.deinit();
        try expectEqual(memory.limits.min, 0);
        try expectEqual(memory.limits.max, limits.max);
        try expectEqual(memory.mem.Internal.items.len, 0);
    }
}

test "MemoryInstance.Internal.grow" {
    {
        const limits = Limits{
            .min = 0,
            .max = null,
        };
        var memory = MemoryInstance.init(limits, null);
        defer memory.deinit();
        try expectEqual(memory.grow(0), true);
        try expectEqual(memory.grow(1), true);
        try expectEqual(memory.size(), 1);
        try expectEqual(memory.grow(1), true);
        try expectEqual(memory.size(), 2);
        try expectEqual(memory.grow(MemoryInstance.k_max_pages - memory.size()), true);
        try expectEqual(memory.size(), MemoryInstance.k_max_pages);
    }

    {
        const limits = Limits{
            .min = 0,
            .max = 25,
        };
        var memory = MemoryInstance.init(limits, null);
        defer memory.deinit();
        try expectEqual(memory.grow(25), true);
        try expectEqual(memory.size(), 25);
        try expectEqual(memory.grow(1), false);
        try expectEqual(memory.size(), 25);
    }
}

test "MemoryInstance.Internal.growAbsolute" {
    {
        const limits = Limits{
            .min = 0,
            .max = null,
        };
        var memory = MemoryInstance.init(limits, null);
        defer memory.deinit();
        try expectEqual(memory.growAbsolute(0), true);
        try expectEqual(memory.size(), 0);
        try expectEqual(memory.growAbsolute(1), true);
        try expectEqual(memory.size(), 1);
        try expectEqual(memory.growAbsolute(5), true);
        try expectEqual(memory.size(), 5);
        try expectEqual(memory.growAbsolute(MemoryInstance.k_max_pages), true);
        try expectEqual(memory.size(), MemoryInstance.k_max_pages);
    }

    {
        const limits = Limits{
            .min = 0,
            .max = 25,
        };
        var memory = MemoryInstance.init(limits, null);
        defer memory.deinit();
        try expectEqual(memory.growAbsolute(25), true);
        try expectEqual(memory.size(), 25);
        try expectEqual(memory.growAbsolute(26), false);
        try expectEqual(memory.size(), 25);
    }
}
