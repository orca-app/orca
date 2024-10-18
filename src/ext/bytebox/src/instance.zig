const std = @import("std");
const AllocError = std.mem.Allocator.Error;

const builtin = @import("builtin");

const common = @import("common.zig");
const StableArray = common.StableArray;

const opcodes = @import("opcode.zig");
const Opcode = opcodes.Opcode;
const WasmOpcode = opcodes.WasmOpcode;

const def = @import("definition.zig");
const ConstantExpression = def.ConstantExpression;
const FunctionDefinition = def.FunctionDefinition;
const FunctionExport = def.FunctionExport;
const FunctionHandle = def.FunctionHandle;
const FunctionHandleType = def.FunctionHandleType;
const FunctionTypeDefinition = def.FunctionTypeDefinition;
const GlobalDefinition = def.GlobalDefinition;
const GlobalMut = def.GlobalMut;
const ImportNames = def.ImportNames;
const Limits = def.Limits;
const MemoryDefinition = def.MemoryDefinition;
const ModuleDefinition = def.ModuleDefinition;
const NameCustomSection = def.NameCustomSection;
const Val = def.Val;
const ValType = def.ValType;
const GlobalExport = def.GlobalExport;

pub const UnlinkableError = error{
    UnlinkableUnknownImport,
    UnlinkableIncompatibleImportType,
};

pub const UninstantiableError = error{
    UninstantiableOutOfBoundsTableAccess,
    UninstantiableOutOfBoundsMemoryAccess,
};

pub const ExportError = error{
    ExportUnknownFunction,
    ExportUnknownGlobal,
};

pub const TrapError = error{
    TrapDebug,
    TrapUnreachable,
    TrapIntegerDivisionByZero,
    TrapIntegerOverflow,
    TrapIndirectCallTypeMismatch,
    TrapInvalidIntegerConversion,
    TrapOutOfBoundsMemoryAccess,
    TrapUndefinedElement,
    TrapUninitializedElement,
    TrapOutOfBoundsTableAccess,
    TrapStackExhausted,
    TrapUnknown,
};

pub const DebugTrace = struct {
    pub const Mode = enum {
        None,
        Function,
        Instruction,
    };

    pub fn setMode(new_mode: Mode) bool {
        if (builtin.mode == .Debug) {
            mode = new_mode;
            return true;
        }

        return false;
    }

    pub fn shouldTraceFunctions() bool {
        return builtin.mode == .Debug and mode == .Function;
    }

    pub fn shouldTraceInstructions() bool {
        return builtin.mode == .Debug and mode == .Instruction;
    }

    pub fn printIndent(indent: u32) void {
        var indent_level: u32 = 0;
        while (indent_level < indent) : (indent_level += 1) {
            std.debug.print("  ", .{});
        }
    }

    pub fn traceHostFunction(module_instance: *const ModuleInstance, indent: u32, import_name: []const u8) void {
        if (shouldTraceFunctions()) {
            _ = module_instance;
            const module_name = "<unknown_host_module>";

            printIndent(indent);
            std.debug.print("{s}!{s}\n", .{ module_name, import_name });
        }
    }

    pub fn traceFunction(module_instance: *const ModuleInstance, indent: u32, func_index: usize) void {
        if (shouldTraceFunctions()) {
            const func_name_index: usize = func_index + module_instance.module_def.imports.functions.items.len;

            const name_section: *const NameCustomSection = &module_instance.module_def.name_section;
            const module_name = name_section.getModuleName();
            const function_name = name_section.findFunctionName(func_name_index);

            printIndent(indent);
            std.debug.print("{s}!{s}\n", .{ module_name, function_name });
        }
    }

    var mode: Mode = .None;
};

pub const GlobalInstance = struct {
    def: *GlobalDefinition,
    value: Val,
};

pub const TableInstance = struct {
    refs: std.ArrayList(Val), // should only be reftypes
    reftype: ValType,
    limits: Limits,

    pub fn init(reftype: ValType, limits: Limits, allocator: std.mem.Allocator) !TableInstance {
        std.debug.assert(reftype.isRefType());

        var table = TableInstance{
            .refs = std.ArrayList(Val).init(allocator),
            .reftype = reftype,
            .limits = limits,
        };

        if (limits.min > 0) {
            try table.refs.appendNTimes(try Val.nullRef(reftype), limits.min);
        }
        return table;
    }

    pub fn deinit(table: *TableInstance) void {
        table.refs.deinit();
    }

    pub fn grow(table: *TableInstance, length: usize, init_value: Val) bool {
        const max = if (table.limits.max) |m| m else std.math.maxInt(i32);
        std.debug.assert(table.refs.items.len == table.limits.min);

        var old_length: usize = table.limits.min;
        if (old_length + length > max) {
            return false;
        }

        table.limits.min = @as(u32, @intCast(old_length + length));

        table.refs.appendNTimes(init_value, length) catch return false;
        return true;
    }

    fn init_range_val(table: *TableInstance, module: *ModuleInstance, elems: []const Val, init_length: u32, start_elem_index: u32, start_table_index: u32) !void {
        if (table.refs.items.len < start_table_index + init_length) {
            return error.TrapOutOfBoundsTableAccess;
        }

        if (elems.len < start_elem_index + init_length) {
            return error.TrapOutOfBoundsTableAccess;
        }

        var elem_range = elems[start_elem_index .. start_elem_index + init_length];
        var table_range = table.refs.items[start_table_index .. start_table_index + init_length];

        var index: u32 = 0;
        while (index < elem_range.len) : (index += 1) {
            var val: Val = elem_range[index];

            if (table.reftype == .FuncRef) {
                val.FuncRef.module_instance = module;
            }

            table_range[index] = val;
        }
    }

    fn init_range_expr(table: *TableInstance, module: *ModuleInstance, elems: []const ConstantExpression, init_length: u32, start_elem_index: u32, start_table_index: u32, store: *Store) !void {
        if (start_table_index < 0 or table.refs.items.len < start_table_index + init_length) {
            return error.TrapOutOfBoundsTableAccess;
        }

        if (start_elem_index < 0 or elems.len < start_elem_index + init_length) {
            return error.TrapOutOfBoundsTableAccess;
        }

        var elem_range = elems[start_elem_index .. start_elem_index + init_length];
        var table_range = table.refs.items[start_table_index .. start_table_index + init_length];

        var index: u32 = 0;
        while (index < elem_range.len) : (index += 1) {
            var val: Val = elem_range[index].resolve(store);

            if (table.reftype == .FuncRef) {
                val.FuncRef.module_instance = module;
            }

            table_range[index] = val;
        }
    }
};

pub const WasmMemoryResizeFunction = *const fn (mem: ?[*]u8, new_size_bytes: usize, old_size_bytes: usize, userdata: ?*anyopaque) ?[*]u8;
pub const WasmMemoryFreeFunction = *const fn (mem: ?[*]u8, size_bytes: usize, userdata: ?*anyopaque) void;

pub const WasmMemoryExternal = struct {
    resize_callback: WasmMemoryResizeFunction,
    free_callback: WasmMemoryFreeFunction,
    userdata: ?*anyopaque,
};

pub const MemoryInstance = struct {
    const BackingMemoryType = enum(u8) {
        Internal,
        External,
    };

    const BackingMemory = union(BackingMemoryType) {
        Internal: StableArray(u8),
        External: struct {
            buffer: []u8,
            params: WasmMemoryExternal,
        },
    };

    pub const k_page_size: usize = MemoryDefinition.k_page_size;
    pub const k_max_pages: usize = MemoryDefinition.k_max_pages;

    limits: Limits,
    mem: BackingMemory,

    pub fn init(limits: Limits, params: ?WasmMemoryExternal) MemoryInstance {
        const max_pages = if (limits.max) |max| @max(1, max) else k_max_pages;

        var mem = if (params == null) BackingMemory{
            .Internal = StableArray(u8).init(max_pages * k_page_size),
        } else BackingMemory{ .External = .{
            .buffer = &[0]u8{},
            .params = params.?,
        } };

        var instance = MemoryInstance{
            .limits = Limits{ .min = 0, .max = @as(u32, @intCast(max_pages)) },
            .mem = mem,
        };

        return instance;
    }

    pub fn deinit(self: *MemoryInstance) void {
        switch (self.mem) {
            .Internal => |*m| m.deinit(),
            .External => |*m| m.params.free_callback(m.buffer.ptr, m.buffer.len, m.params.userdata),
        }
    }

    pub fn size(self: *const MemoryInstance) usize {
        return switch (self.mem) {
            .Internal => |m| m.items.len / k_page_size,
            .External => |m| m.buffer.len / k_page_size,
        };
    }

    pub fn grow(self: *MemoryInstance, num_pages: usize) bool {
        if (num_pages == 0) {
            return true;
        }

        const total_pages = self.limits.min + num_pages;
        const max_pages = if (self.limits.max) |max| max else k_max_pages;

        if (total_pages > max_pages) {
            return false;
        }

        const commit_size: usize = (self.limits.min + num_pages) * k_page_size;

        switch (self.mem) {
            .Internal => |*m| m.resize(commit_size) catch return false,
            .External => |*m| {
                var new_mem: ?[*]u8 = m.params.resize_callback(m.buffer.ptr, commit_size, m.buffer.len, m.params.userdata);
                if (new_mem == null) {
                    return false;
                }
                m.buffer = new_mem.?[0..commit_size];
            },
        }

        self.limits.min = @as(u32, @intCast(total_pages));

        return true;
    }

    pub fn growAbsolute(self: *MemoryInstance, total_pages: usize) bool {
        if (self.limits.min >= total_pages) {
            return true;
        }

        const pages_to_grow = total_pages - self.limits.min;
        return self.grow(pages_to_grow);
    }

    pub fn buffer(self: *const MemoryInstance) []u8 {
        return switch (self.mem) {
            .Internal => |m| m.items,
            .External => |m| m.buffer,
        };
    }

    fn ensureMinSize(self: *MemoryInstance, size_bytes: usize) !void {
        if (self.limits.min * k_page_size < size_bytes) {
            var num_min_pages = std.math.divCeil(usize, size_bytes, k_page_size) catch unreachable;
            if (num_min_pages > self.limits.max.?) {
                return error.TrapOutOfBoundsMemoryAccess;
            }

            var needed_pages = num_min_pages - self.limits.min;
            if (self.resize(needed_pages) == false) {
                unreachable;
            }
        }
    }
};

pub const ElementInstance = struct {
    refs: std.ArrayList(Val),
    reftype: ValType,
};

const ImportType = enum(u8) {
    Host,
    Wasm,
};

const HostFunctionCallback = *const fn (userdata: ?*anyopaque, module: *ModuleInstance, params: [*]const Val, returns: [*]Val) void;

const HostFunction = struct {
    userdata: ?*anyopaque,
    func_def: FunctionTypeDefinition,
    callback: HostFunctionCallback,
};

const ImportDataWasm = struct {
    module_instance: *ModuleInstance,
    index: u32,
};

pub const FunctionImport = struct {
    name: []const u8,
    data: union(ImportType) {
        Host: HostFunction,
        Wasm: ImportDataWasm,
    },

    fn dupe(import: *const FunctionImport, allocator: std.mem.Allocator) !FunctionImport {
        var copy = import.*;
        copy.name = try allocator.dupe(u8, copy.name);
        switch (copy.data) {
            .Host => |*data| {
                var func_def = FunctionTypeDefinition{
                    .types = std.ArrayList(ValType).init(allocator),
                    .num_params = data.func_def.num_params,
                };
                try func_def.types.appendSlice(data.func_def.types.items);
                data.func_def = func_def;
            },
            .Wasm => {},
        }

        return copy;
    }

    pub fn isTypeSignatureEql(import: *const FunctionImport, type_signature: *const FunctionTypeDefinition) bool {
        var type_comparer = FunctionTypeDefinition.SortContext{};
        switch (import.data) {
            .Host => |data| {
                return type_comparer.eql(&data.func_def, type_signature);
            },
            .Wasm => |data| {
                var func_type_def: *const FunctionTypeDefinition = data.module_instance.findFuncTypeDef(data.index);
                return type_comparer.eql(func_type_def, type_signature);
            },
        }
    }
};

pub const TableImport = struct {
    name: []const u8,
    data: union(ImportType) {
        Host: *TableInstance,
        Wasm: ImportDataWasm,
    },

    fn dupe(import: *const TableImport, allocator: std.mem.Allocator) !TableImport {
        var copy = import.*;
        copy.name = try allocator.dupe(u8, copy.name);
        return copy;
    }
};

pub const MemoryImport = struct {
    name: []const u8,
    data: union(ImportType) {
        Host: *MemoryInstance,
        Wasm: ImportDataWasm,
    },

    fn dupe(import: *const MemoryImport, allocator: std.mem.Allocator) !MemoryImport {
        var copy = import.*;
        copy.name = try allocator.dupe(u8, copy.name);
        return copy;
    }
};

pub const GlobalImport = struct {
    name: []const u8,
    data: union(ImportType) {
        Host: *GlobalInstance,
        Wasm: ImportDataWasm,
    },

    fn dupe(import: *const GlobalImport, allocator: std.mem.Allocator) !GlobalImport {
        var copy = import.*;
        copy.name = try allocator.dupe(u8, copy.name);
        return copy;
    }
};

pub const ModuleImportPackage = struct {
    name: []const u8,
    instance: ?*ModuleInstance,
    userdata: ?*anyopaque,
    functions: std.ArrayList(FunctionImport),
    tables: std.ArrayList(TableImport),
    memories: std.ArrayList(MemoryImport),
    globals: std.ArrayList(GlobalImport),
    allocator: std.mem.Allocator,

    pub fn init(name: []const u8, instance: ?*ModuleInstance, userdata: ?*anyopaque, allocator: std.mem.Allocator) std.mem.Allocator.Error!ModuleImportPackage {
        return ModuleImportPackage{
            .name = try allocator.dupe(u8, name),
            .instance = instance,
            .userdata = userdata,
            .functions = std.ArrayList(FunctionImport).init(allocator),
            .tables = std.ArrayList(TableImport).init(allocator),
            .memories = std.ArrayList(MemoryImport).init(allocator),
            .globals = std.ArrayList(GlobalImport).init(allocator),
            .allocator = allocator,
        };
    }

    pub fn addHostFunction(self: *ModuleImportPackage, name: []const u8, param_types: []const ValType, return_types: []const ValType, callback: HostFunctionCallback, userdata: ?*anyopaque) std.mem.Allocator.Error!void {
        std.debug.assert(self.instance == null); // cannot add host functions to an imports that is intended to be bound to a module instance

        var type_list = std.ArrayList(ValType).init(self.allocator);
        try type_list.appendSlice(param_types);
        try type_list.appendSlice(return_types);

        try self.functions.append(FunctionImport{
            .name = try self.allocator.dupe(u8, name),
            .data = .{
                .Host = HostFunction{
                    .userdata = userdata,
                    .func_def = FunctionTypeDefinition{
                        .types = type_list,
                        .num_params = @as(u32, @intCast(param_types.len)),
                    },
                    .callback = callback,
                },
            },
        });
    }

    pub fn deinit(self: *ModuleImportPackage) void {
        self.allocator.free(self.name);

        for (self.functions.items) |*item| {
            self.allocator.free(item.name);
            switch (item.data) {
                .Host => |h| h.func_def.types.deinit(),
                else => {},
            }
        }
        self.functions.deinit();

        for (self.tables.items) |*item| {
            self.allocator.free(item.name);
        }
        self.tables.deinit();

        for (self.memories.items) |*item| {
            self.allocator.free(item.name);
        }
        self.memories.deinit();

        for (self.globals.items) |*item| {
            self.allocator.free(item.name);
        }
        self.globals.deinit();
    }
};

pub const Store = struct {
    tables: std.ArrayList(TableInstance),
    memories: std.ArrayList(MemoryInstance),
    globals: std.ArrayList(GlobalInstance),
    elements: std.ArrayList(ElementInstance),
    imports: struct {
        functions: std.ArrayList(FunctionImport),
        tables: std.ArrayList(TableImport),
        memories: std.ArrayList(MemoryImport),
        globals: std.ArrayList(GlobalImport),
    },

    fn init(allocator: std.mem.Allocator) Store {
        var store = Store{
            .imports = .{
                .functions = std.ArrayList(FunctionImport).init(allocator),
                .tables = std.ArrayList(TableImport).init(allocator),
                .memories = std.ArrayList(MemoryImport).init(allocator),
                .globals = std.ArrayList(GlobalImport).init(allocator),
            },
            .tables = std.ArrayList(TableInstance).init(allocator),
            .memories = std.ArrayList(MemoryInstance).init(allocator),
            .globals = std.ArrayList(GlobalInstance).init(allocator),
            .elements = std.ArrayList(ElementInstance).init(allocator),
        };

        return store;
    }

    fn deinit(self: *Store) void {
        for (self.tables.items) |*item| {
            item.deinit();
        }
        self.tables.deinit();

        for (self.memories.items) |*item| {
            item.deinit();
        }
        self.memories.deinit();

        self.globals.deinit();
        self.elements.deinit();
    }

    pub fn getTable(self: *Store, index: usize) *TableInstance {
        if (self.imports.tables.items.len <= index) {
            var instance_index = index - self.imports.tables.items.len;
            return &self.tables.items[instance_index];
        } else {
            var import: *TableImport = &self.imports.tables.items[index];
            return switch (import.data) {
                .Host => |data| data,
                .Wasm => |data| data.module_instance.store.getTable(data.index),
            };
        }
    }

    pub fn getMemory(self: *Store, index: usize) *MemoryInstance {
        if (self.imports.memories.items.len <= index) {
            var instance_index = index - self.imports.memories.items.len;
            return &self.memories.items[instance_index];
        } else {
            var import: *MemoryImport = &self.imports.memories.items[index];
            return switch (import.data) {
                .Host => |data| data,
                .Wasm => |data| data.module_instance.store.getMemory(data.index),
            };
        }
    }

    pub fn getGlobal(self: *Store, index: usize) *GlobalInstance { // TODO make private
        if (self.imports.globals.items.len <= index) {
            var instance_index = index - self.imports.globals.items.len;
            return &self.globals.items[instance_index];
        } else {
            var import: *GlobalImport = &self.imports.globals.items[index];
            return switch (import.data) {
                .Host => |data| data,
                .Wasm => |data| data.module_instance.store.getGlobal(data.index),
            };
        }
    }
};

pub const ModuleInstantiateOpts = struct {
    /// imports is not owned by ModuleInstance - caller must ensure its memory outlives ModuleInstance
    imports: ?[]const ModuleImportPackage = null,
    wasm_memory_external: ?WasmMemoryExternal = null,
    stack_size: usize = 0,
    enable_debug: bool = false,
};

pub const InvokeOpts = struct {
    trap_on_start: bool = false,
};

pub const DebugTrapInstructionMode = enum {
    Enable,
    Disable,
};

pub const VM = struct {
    const InitFn = *const fn (vm: *VM) void;
    const DeinitFn = *const fn (vm: *VM) void;
    const InstantiateFn = *const fn (vm: *VM, module: *ModuleInstance, opts: ModuleInstantiateOpts) anyerror!void;
    const InvokeFn = *const fn (vm: *VM, module: *ModuleInstance, handle: FunctionHandle, params: [*]const Val, returns: [*]Val, opts: InvokeOpts) anyerror!void;
    const InvokeWithIndexFn = *const fn (vm: *VM, module: *ModuleInstance, func_index: usize, params: [*]const Val, returns: [*]Val) anyerror!void;
    const ResumeInvokeFn = *const fn (vm: *VM, module: *ModuleInstance, returns: []Val) anyerror!void;
    const StepFn = *const fn (vm: *VM, module: *ModuleInstance, returns: []Val) anyerror!void;
    const SetDebugTrapFn = *const fn (vm: *VM, module: *ModuleInstance, wasm_address: u32, mode: DebugTrapInstructionMode) anyerror!bool;
    const FormatBacktraceFn = *const fn (vm: *VM, indent: u8, allocator: std.mem.Allocator) anyerror!std.ArrayList(u8);
    const FindFuncTypeDefFn = *const fn (vm: *VM, module: *ModuleInstance, local_func_index: usize) *const FunctionTypeDefinition;

    deinit_fn: DeinitFn,
    instantiate_fn: InstantiateFn,
    invoke_fn: InvokeFn,
    invoke_with_index_fn: InvokeWithIndexFn,
    resume_invoke_fn: ResumeInvokeFn,
    step_fn: StepFn,
    set_debug_trap_fn: SetDebugTrapFn,
    format_backtrace_fn: FormatBacktraceFn,
    find_func_type_def_fn: FindFuncTypeDefFn,

    allocator: std.mem.Allocator,
    mem: []u8, // VM and impl memory live here

    impl: *anyopaque,

    pub fn create(comptime T: type, allocator: std.mem.Allocator) AllocError!*VM {
        const alignment = @max(@alignOf(VM), @alignOf(T));
        const vm_alloc_size = std.mem.alignForward(usize, @sizeOf(VM), alignment);
        const impl_alloc_size = std.mem.alignForward(usize, @sizeOf(T), alignment);
        const total_alloc_size = vm_alloc_size + impl_alloc_size;

        var mem = try allocator.alloc(u8, total_alloc_size);

        var vm: *VM = @as(*VM, @alignCast(@ptrCast(mem.ptr)));
        var impl: *T = @as(*T, @alignCast(@ptrCast(mem[vm_alloc_size..].ptr)));

        vm.deinit_fn = T.deinit;
        vm.instantiate_fn = T.instantiate;
        vm.invoke_fn = T.invoke;
        vm.invoke_with_index_fn = T.invokeWithIndex;
        vm.resume_invoke_fn = T.resumeInvoke;
        vm.step_fn = T.step;
        vm.set_debug_trap_fn = T.setDebugTrap;
        vm.format_backtrace_fn = T.formatBacktrace;
        vm.find_func_type_def_fn = T.findFuncTypeDef;
        vm.allocator = allocator;
        vm.mem = mem;
        vm.impl = impl;

        T.init(vm);

        return vm;
    }

    fn destroy(vm: *VM) void {
        vm.deinit_fn(vm);

        var allocator = vm.allocator;
        var mem = vm.mem;
        allocator.free(mem);
    }

    fn instantiate(vm: *VM, module: *ModuleInstance, opts: ModuleInstantiateOpts) anyerror!void {
        try vm.instantiate_fn(vm, module, opts);
    }

    pub fn invoke(vm: *VM, module: *ModuleInstance, handle: FunctionHandle, params: [*]const Val, returns: [*]Val, opts: InvokeOpts) anyerror!void {
        try vm.invoke_fn(vm, module, handle, params, returns, opts);
    }

    pub fn invokeWithIndex(vm: *VM, module: *ModuleInstance, func_index: usize, params: [*]const Val, returns: [*]Val) anyerror!void {
        try vm.invoke_with_index_fn(vm, module, func_index, params, returns);
    }

    pub fn resumeInvoke(vm: *VM, module: *ModuleInstance, returns: []Val) anyerror!void {
        try vm.resume_invoke_fn(vm, module, returns);
    }

    pub fn step(vm: *VM, module: *ModuleInstance, returns: []Val) anyerror!void {
        try vm.step_fn(vm, module, returns);
    }

    pub fn setDebugTrap(vm: *VM, module: *ModuleInstance, wasm_address: u32, mode: DebugTrapInstructionMode) anyerror!bool {
        return try vm.set_debug_trap_fn(vm, module, wasm_address, mode);
    }

    pub fn formatBacktrace(vm: *VM, indent: u8, allocator: std.mem.Allocator) anyerror!std.ArrayList(u8) {
        return vm.format_backtrace_fn(vm, indent, allocator);
    }

    pub fn findFuncTypeDef(vm: *VM, module: *ModuleInstance, local_func_index: usize) *const FunctionTypeDefinition {
        return vm.find_func_type_def_fn(vm, module, local_func_index);
    }
};

pub const ModuleInstance = struct {
    allocator: std.mem.Allocator,
    store: Store,
    module_def: *const ModuleDefinition,
    userdata: ?*anyopaque = null, // any host data associated with this module
    is_instantiated: bool = false,
    vm: *VM,

    pub fn create(module_def: *const ModuleDefinition, vm: *VM, allocator: std.mem.Allocator) AllocError!*ModuleInstance {
        var inst = try allocator.create(ModuleInstance);
        inst.* = ModuleInstance{
            .allocator = allocator,
            .store = Store.init(allocator),
            .module_def = module_def,
            .vm = vm,
        };
        return inst;
    }

    pub fn destroy(self: *ModuleInstance) void {
        self.vm.destroy();
        self.store.deinit();

        var allocator = self.allocator;
        allocator.destroy(self);
    }

    pub fn instantiate(self: *ModuleInstance, opts: ModuleInstantiateOpts) !void {
        const Helpers = struct {
            fn areLimitsCompatible(def_limits: *const Limits, instance_limits: *const Limits) bool {
                if (def_limits.max != null and instance_limits.max == null) {
                    return false;
                }

                var def_max: u32 = if (def_limits.max) |max| max else std.math.maxInt(u32);
                var instance_max: u32 = if (instance_limits.max) |max| max else 0;

                return def_limits.min <= instance_limits.min and def_max >= instance_max;
            }

            // TODO probably should change the imports search to a hashed lookup of module_name+item_name -> array of items to make this faster
            fn findImportInMultiple(comptime T: type, names: *const ImportNames, imports_or_null: ?[]const ModuleImportPackage) UnlinkableError!*const T {
                if (imports_or_null) |_imports| {
                    for (_imports) |*module_imports| {
                        const wildcard_name = std.mem.eql(u8, module_imports.name, "*");
                        if (wildcard_name or std.mem.eql(u8, names.module_name, module_imports.name)) {
                            switch (T) {
                                FunctionImport => {
                                    if (findImportInSingle(FunctionImport, names, module_imports)) |import| {
                                        return import;
                                    }
                                    if (findImportInSingle(TableImport, names, module_imports)) |_| {
                                        return error.UnlinkableIncompatibleImportType;
                                    }
                                    if (findImportInSingle(MemoryImport, names, module_imports)) |_| {
                                        return error.UnlinkableIncompatibleImportType;
                                    }
                                    if (findImportInSingle(GlobalImport, names, module_imports)) |_| {
                                        return error.UnlinkableIncompatibleImportType;
                                    }
                                },
                                TableImport => {
                                    if (findImportInSingle(TableImport, names, module_imports)) |import| {
                                        return import;
                                    }
                                    if (findImportInSingle(FunctionImport, names, module_imports)) |_| {
                                        return error.UnlinkableIncompatibleImportType;
                                    }
                                    if (findImportInSingle(MemoryImport, names, module_imports)) |_| {
                                        return error.UnlinkableIncompatibleImportType;
                                    }
                                    if (findImportInSingle(GlobalImport, names, module_imports)) |_| {
                                        return error.UnlinkableIncompatibleImportType;
                                    }
                                },
                                MemoryImport => {
                                    if (findImportInSingle(MemoryImport, names, module_imports)) |import| {
                                        return import;
                                    }
                                    if (findImportInSingle(FunctionImport, names, module_imports)) |_| {
                                        return error.UnlinkableIncompatibleImportType;
                                    }
                                    if (findImportInSingle(TableImport, names, module_imports)) |_| {
                                        return error.UnlinkableIncompatibleImportType;
                                    }
                                    if (findImportInSingle(GlobalImport, names, module_imports)) |_| {
                                        return error.UnlinkableIncompatibleImportType;
                                    }
                                },
                                GlobalImport => {
                                    if (findImportInSingle(GlobalImport, names, module_imports)) |import| {
                                        return import;
                                    }
                                    if (findImportInSingle(FunctionImport, names, module_imports)) |_| {
                                        return error.UnlinkableIncompatibleImportType;
                                    }
                                    if (findImportInSingle(TableImport, names, module_imports)) |_| {
                                        return error.UnlinkableIncompatibleImportType;
                                    }
                                    if (findImportInSingle(MemoryImport, names, module_imports)) |_| {
                                        return error.UnlinkableIncompatibleImportType;
                                    }
                                },
                                else => unreachable,
                            }
                            break;
                        }
                    }
                }

                return error.UnlinkableUnknownImport;
            }

            fn findImportInSingle(comptime T: type, names: *const ImportNames, module_imports: *const ModuleImportPackage) ?*const T {
                var items: []const T = switch (T) {
                    FunctionImport => module_imports.functions.items,
                    TableImport => module_imports.tables.items,
                    MemoryImport => module_imports.memories.items,
                    GlobalImport => module_imports.globals.items,
                    else => unreachable,
                };

                for (items) |*item| {
                    if (std.mem.eql(u8, names.import_name, item.name)) {
                        return item;
                    }
                }

                return null;
            }
        };

        std.debug.assert(self.is_instantiated == false);

        var store: *Store = &self.store;
        var module_def: *const ModuleDefinition = self.module_def;
        var allocator = self.allocator;

        for (module_def.imports.functions.items) |*func_import_def| {
            var import_func: *const FunctionImport = try Helpers.findImportInMultiple(FunctionImport, &func_import_def.names, opts.imports);

            const type_def: *const FunctionTypeDefinition = &module_def.types.items[func_import_def.type_index];
            const is_type_signature_eql: bool = import_func.isTypeSignatureEql(type_def);

            if (is_type_signature_eql == false) {
                return error.UnlinkableIncompatibleImportType;
            }

            try store.imports.functions.append(try import_func.dupe(allocator));
        }

        for (module_def.imports.tables.items) |*table_import_def| {
            var import_table: *const TableImport = try Helpers.findImportInMultiple(TableImport, &table_import_def.names, opts.imports);

            var is_eql: bool = undefined;
            switch (import_table.data) {
                .Host => |table_instance| {
                    is_eql = table_instance.reftype == table_import_def.reftype and
                        Helpers.areLimitsCompatible(&table_import_def.limits, &table_instance.limits);
                },
                .Wasm => |data| {
                    const table_instance: *const TableInstance = data.module_instance.store.getTable(data.index);
                    is_eql = table_instance.reftype == table_import_def.reftype and
                        Helpers.areLimitsCompatible(&table_import_def.limits, &table_instance.limits);
                },
            }

            if (is_eql == false) {
                return error.UnlinkableIncompatibleImportType;
            }

            try store.imports.tables.append(try import_table.dupe(allocator));
        }

        for (module_def.imports.memories.items) |*memory_import_def| {
            var import_memory: *const MemoryImport = try Helpers.findImportInMultiple(MemoryImport, &memory_import_def.names, opts.imports);

            var is_eql: bool = undefined;
            switch (import_memory.data) {
                .Host => |memory_instance| {
                    is_eql = Helpers.areLimitsCompatible(&memory_import_def.limits, &memory_instance.limits);
                },
                .Wasm => |data| {
                    const memory_instance: *const MemoryInstance = data.module_instance.store.getMemory(data.index);
                    is_eql = Helpers.areLimitsCompatible(&memory_import_def.limits, &memory_instance.limits);
                },
            }

            if (is_eql == false) {
                return error.UnlinkableIncompatibleImportType;
            }

            try store.imports.memories.append(try import_memory.dupe(allocator));
        }

        for (module_def.imports.globals.items) |*global_import_def| {
            var import_global: *const GlobalImport = try Helpers.findImportInMultiple(GlobalImport, &global_import_def.names, opts.imports);

            var is_eql: bool = undefined;
            switch (import_global.data) {
                .Host => |global_instance| {
                    is_eql = global_import_def.valtype == global_instance.def.valtype and
                        global_import_def.mut == global_instance.def.mut;
                },
                .Wasm => |data| {
                    const global_instance: *const GlobalInstance = data.module_instance.store.getGlobal(data.index);
                    is_eql = global_import_def.valtype == global_instance.def.valtype and
                        global_import_def.mut == global_instance.def.mut;
                },
            }

            if (is_eql == false) {
                return error.UnlinkableIncompatibleImportType;
            }

            try store.imports.globals.append(try import_global.dupe(allocator));
        }

        // instantiate the rest of the needed module definitions
        try self.vm.instantiate(self, opts);

        try store.tables.ensureTotalCapacity(module_def.imports.tables.items.len + module_def.tables.items.len);

        for (module_def.tables.items) |*def_table| {
            var t = try TableInstance.init(def_table.reftype, def_table.limits, allocator);
            try store.tables.append(t);
        }

        try store.memories.ensureTotalCapacity(module_def.imports.memories.items.len + module_def.memories.items.len);

        for (module_def.memories.items) |*def_memory| {
            var memory = MemoryInstance.init(def_memory.limits, opts.wasm_memory_external);
            if (memory.grow(def_memory.limits.min) == false) {
                unreachable;
            }
            try store.memories.append(memory);
        }

        try store.globals.ensureTotalCapacity(module_def.imports.globals.items.len + module_def.globals.items.len);

        for (module_def.globals.items) |*def_global| {
            var global = GlobalInstance{
                .def = def_global,
                .value = def_global.expr.resolve(store),
            };
            if (def_global.valtype == .FuncRef) {
                global.value.FuncRef.module_instance = self;
            }
            try store.globals.append(global);
        }

        // iterate over elements and init the ones needed
        try store.elements.ensureTotalCapacity(module_def.elements.items.len);
        for (module_def.elements.items) |*def_elem| {
            var elem = ElementInstance{
                .refs = std.ArrayList(Val).init(allocator),
                .reftype = def_elem.reftype,
            };

            // instructions using passive elements just use the module definition's data to avoid an extra copy
            if (def_elem.mode == .Active) {
                std.debug.assert(def_elem.table_index < store.imports.tables.items.len + store.tables.items.len);

                var table: *TableInstance = store.getTable(def_elem.table_index);

                var start_table_index_i32: i32 = if (def_elem.offset) |offset| offset.resolveTo(store, i32) else 0;
                if (start_table_index_i32 < 0) {
                    return error.UninstantiableOutOfBoundsTableAccess;
                }

                var start_table_index = @as(u32, @intCast(start_table_index_i32));

                if (def_elem.elems_value.items.len > 0) {
                    var elems = def_elem.elems_value.items;
                    try table.init_range_val(self, elems, @as(u32, @intCast(elems.len)), 0, start_table_index);
                } else {
                    var elems = def_elem.elems_expr.items;
                    try table.init_range_expr(self, elems, @as(u32, @intCast(elems.len)), 0, start_table_index, store);
                }
            } else if (def_elem.mode == .Passive) {
                if (def_elem.elems_value.items.len > 0) {
                    try elem.refs.resize(def_elem.elems_value.items.len);
                    var index: usize = 0;
                    while (index < elem.refs.items.len) : (index += 1) {
                        elem.refs.items[index] = def_elem.elems_value.items[index];
                        if (elem.reftype == .FuncRef) {
                            elem.refs.items[index].FuncRef.module_instance = self;
                        }
                    }
                } else {
                    try elem.refs.resize(def_elem.elems_expr.items.len);
                    var index: usize = 0;
                    while (index < elem.refs.items.len) : (index += 1) {
                        elem.refs.items[index] = def_elem.elems_expr.items[index].resolve(store);
                        if (elem.reftype == .FuncRef) {
                            elem.refs.items[index].FuncRef.module_instance = self;
                        }
                    }
                }
            }

            store.elements.appendAssumeCapacity(elem);
        }

        for (module_def.datas.items) |*def_data| {
            // instructions using passive elements just use the module definition's data to avoid an extra copy
            if (def_data.mode == .Active) {
                var memory_index: u32 = def_data.memory_index.?;
                var memory: *MemoryInstance = store.getMemory(memory_index);

                const num_bytes: usize = def_data.bytes.items.len;
                const offset_begin: usize = (def_data.offset.?).resolveTo(store, u32);
                const offset_end: usize = offset_begin + num_bytes;

                const mem_buffer: []u8 = memory.buffer();

                if (mem_buffer.len < offset_end) {
                    return error.UninstantiableOutOfBoundsMemoryAccess;
                }

                var destination = mem_buffer[offset_begin..offset_end];
                std.mem.copy(u8, destination, def_data.bytes.items);
            }
        }

        if (module_def.start_func_index) |func_index| {
            var no_vals: []Val = &[0]Val{};
            try self.vm.invokeWithIndex(self, func_index, no_vals.ptr, no_vals.ptr);
        }
    }

    pub fn exports(self: *ModuleInstance, name: []const u8) !ModuleImportPackage {
        var imports = try ModuleImportPackage.init(name, self, null, self.allocator);

        for (self.module_def.exports.functions.items) |*item| {
            try imports.functions.append(FunctionImport{
                .name = try imports.allocator.dupe(u8, item.name),
                .data = .{
                    .Wasm = ImportDataWasm{
                        .module_instance = self,
                        .index = item.index,
                    },
                },
            });
        }

        for (self.module_def.exports.tables.items) |*item| {
            try imports.tables.append(TableImport{
                .name = try imports.allocator.dupe(u8, item.name),
                .data = .{
                    .Wasm = ImportDataWasm{
                        .module_instance = self,
                        .index = item.index,
                    },
                },
            });
        }

        for (self.module_def.exports.memories.items) |*item| {
            try imports.memories.append(MemoryImport{
                .name = try imports.allocator.dupe(u8, item.name),
                .data = .{
                    .Wasm = ImportDataWasm{
                        .module_instance = self,
                        .index = item.index,
                    },
                },
            });
        }

        for (self.module_def.exports.globals.items) |*item| {
            try imports.globals.append(GlobalImport{
                .name = try imports.allocator.dupe(u8, item.name),
                .data = .{
                    .Wasm = ImportDataWasm{
                        .module_instance = self,
                        .index = item.index,
                    },
                },
            });
        }

        return imports;
    }

    pub fn getFunctionHandle(self: *const ModuleInstance, func_name: []const u8) ExportError!FunctionHandle {
        for (self.module_def.exports.functions.items) |func_export| {
            if (std.mem.eql(u8, func_name, func_export.name)) {
                if (func_export.index >= self.module_def.imports.functions.items.len) {
                    var func_index: usize = func_export.index - self.module_def.imports.functions.items.len;
                    return FunctionHandle{
                        .index = @as(u32, @intCast(func_index)),
                        .type = .Export,
                    };
                } else {
                    return FunctionHandle{
                        .index = @as(u32, @intCast(func_export.index)),
                        .type = .Import,
                    };
                }
            }
        }

        for (self.store.imports.functions.items, 0..) |*func_import, i| {
            if (std.mem.eql(u8, func_name, func_import.name)) {
                return FunctionHandle{
                    .index = @as(u32, @intCast(i)),
                    .type = .Import,
                };
            }
        }

        return error.ExportUnknownFunction;
    }

    pub fn getFunctionInfo(self: *const ModuleInstance, handle: FunctionHandle) FunctionExport {
        return self.module_def.getFunctionExport(handle);
    }

    pub fn getGlobalExport(self: *ModuleInstance, global_name: []const u8) ExportError!GlobalExport {
        for (self.module_def.exports.globals.items) |*global_export| {
            if (std.mem.eql(u8, global_name, global_export.name)) {
                var global: *GlobalInstance = self.getGlobalWithIndex(global_export.index);
                return GlobalExport{
                    .val = &global.value,
                    .valtype = global.def.valtype,
                    .mut = global.def.mut,
                };
            }
        }

        return error.ExportUnknownGlobal;
    }

    pub fn invoke(self: *ModuleInstance, handle: FunctionHandle, params: [*]const Val, returns: [*]Val, opts: InvokeOpts) anyerror!void {
        try self.vm.invoke(self, handle, params, returns, opts);
    }

    /// Use to resume an invoked function after it returned error.DebugTrap
    pub fn resumeInvoke(self: *ModuleInstance, returns: []Val) anyerror!void {
        try self.vm.resumeInvoke(self, returns);
    }

    pub fn step(self: *ModuleInstance, returns: []Val) anyerror!void {
        try self.vm.step(self, returns);
    }

    pub fn setDebugTrap(self: *ModuleInstance, wasm_address: u32, mode: DebugTrapInstructionMode) anyerror!bool {
        try self.vm.setDebugTrap(self, wasm_address, mode);
    }

    pub fn memorySlice(self: *ModuleInstance, offset: usize, length: usize) []u8 {
        const memory: *MemoryInstance = self.store.getMemory(0);

        const buffer = memory.buffer();
        if (offset + length < buffer.len) {
            var data: []u8 = buffer[offset .. offset + length];
            return data;
        }

        return "";
    }

    pub fn memoryAll(self: *ModuleInstance) []u8 {
        const memory: *MemoryInstance = self.store.getMemory(0);
        const buffer = memory.buffer();
        return buffer;
    }

    pub fn memoryGrow(self: *ModuleInstance, num_pages: usize) bool {
        const memory: *MemoryInstance = self.store.getMemory(0);
        return memory.grow(num_pages);
    }

    pub fn memoryGrowAbsolute(self: *ModuleInstance, total_pages: usize) bool {
        const memory: *MemoryInstance = self.store.getMemory(0);
        return memory.growAbsolute(total_pages);
    }

    pub fn memoryWriteInt(self: *ModuleInstance, comptime T: type, value: T, offset: usize) bool {
        var bytes: [(@typeInfo(T).Int.bits + 7) / 8]u8 = undefined;
        std.mem.writeIntLittle(T, &bytes, value);

        var destination = self.memorySlice(offset, bytes.len);
        if (destination.len == bytes.len) {
            std.mem.copy(u8, destination, &bytes);
            return true;
        }

        return false;
    }

    /// Caller owns returned memory and must free via allocator.free()
    pub fn formatBacktrace(self: *ModuleInstance, indent: u8, allocator: std.mem.Allocator) anyerror!std.ArrayList(u8) {
        return self.vm.format_backtrace_fn(self.vm, indent, allocator);
    }

    fn findFuncTypeDef(self: *ModuleInstance, index: usize) *const FunctionTypeDefinition {
        const num_imports: usize = self.store.imports.functions.items.len;
        if (index >= num_imports) {
            var local_func_index: usize = index - num_imports;
            return self.vm.findFuncTypeDef(self, local_func_index);
        } else {
            var import: *const FunctionImport = &self.store.imports.functions.items[index];
            var func_type_def: *const FunctionTypeDefinition = switch (import.data) {
                .Host => |data| &data.func_def,
                .Wasm => |data| data.module_instance.findFuncTypeDef(data.index),
            };
            return func_type_def;
        }
    }

    fn getGlobalWithIndex(self: *ModuleInstance, index: usize) *GlobalInstance {
        const num_imports: usize = self.module_def.imports.globals.items.len;
        if (index >= num_imports) {
            var local_global_index: usize = index - self.module_def.imports.globals.items.len;
            return &self.store.globals.items[local_global_index];
        } else {
            var import: *const GlobalImport = &self.store.imports.globals.items[index];
            return switch (import.data) {
                .Host => |data| data,
                .Wasm => |data| data.module_instance.getGlobalWithIndex(data.index),
            };
        }
    }
};
