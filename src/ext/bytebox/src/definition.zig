// This file contains types and code shared between both the ModuleDefinition and VMs

const std = @import("std");
const AllocError = std.mem.Allocator.Error;

const common = @import("common.zig");
const StableArray = common.StableArray;

const opcodes = @import("opcode.zig");
const Opcode = opcodes.Opcode;
const WasmOpcode = opcodes.WasmOpcode;

// HACK: just get the code working, will need to resolve anything dependent on this eventually
const inst = @import("instance.zig");
const ModuleInstance = inst.ModuleInstance;
const Store = inst.Store;
const GlobalInstance = inst.GlobalInstance;

pub const MalformedError = error{
    MalformedMagicSignature,
    MalformedUnexpectedEnd,
    MalformedUnsupportedWasmVersion,
    MalformedSectionId,
    MalformedTypeSentinel,
    MalformedLEB128,
    MalformedMissingZeroByte,
    MalformedTooManyLocals,
    MalformedFunctionCodeSectionMismatch,
    MalformedMissingDataCountSection,
    MalformedDataCountMismatch,
    MalformedDataType,
    MalformedIllegalOpcode,
    MalformedReferenceType,
    MalformedSectionSizeMismatch,
    MalformedInvalidImport,
    MalformedLimits,
    MalformedMultipleStartSections,
    MalformedElementType,
    MalformedUTF8Encoding,
    MalformedMutability,
    MalformedCustomSection,

    MalformedValType,
    MalformedBytecode,
};

pub const ValidationError = error{
    ValidationTypeMismatch,
    ValidationTypeMustBeNumeric,
    ValidationUnknownType,
    ValidationUnknownFunction,
    ValidationUnknownGlobal,
    ValidationUnknownLocal,
    ValidationUnknownTable,
    ValidationUnknownMemory,
    ValidationUnknownElement,
    ValidationUnknownData,
    ValidationOutOfBounds,
    ValidationTypeStackHeightMismatch,
    ValidationBadAlignment,
    ValidationUnknownLabel,
    ValidationImmutableGlobal,
    ValidationBadConstantExpression,
    ValidationGlobalReferencingMutableGlobal,
    ValidationUnknownBlockTypeIndex,
    ValidationSelectArity,
    ValidationMultipleMemories,
    ValidationMemoryInvalidMaxLimit,
    ValidationMemoryMaxPagesExceeded,
    ValidationConstantExpressionGlobalMustBeImport,
    ValidationConstantExpressionGlobalMustBeImmutable,
    ValidationStartFunctionType,
    ValidationLimitsMinMustNotBeLargerThanMax,
    ValidationConstantExpressionTypeMismatch,
    ValidationDuplicateExportName,
    ValidationFuncRefUndeclared,
    ValidationIfElseMismatch,
    ValidationInvalidLaneIndex,
};

pub const i8x16 = @Vector(16, i8);
pub const u8x16 = @Vector(16, u8);
pub const i16x8 = @Vector(8, i16);
pub const u16x8 = @Vector(8, u16);
pub const i32x4 = @Vector(4, i32);
pub const u32x4 = @Vector(4, u32);
pub const i64x2 = @Vector(2, i64);
pub const u64x2 = @Vector(2, u64);
pub const f32x4 = @Vector(4, f32);
pub const f64x2 = @Vector(2, f64);
pub const v128 = f32x4;

const Section = enum(u8) { Custom, FunctionType, Import, Function, Table, Memory, Global, Export, Start, Element, Code, Data, DataCount };

const k_function_type_sentinel_byte: u8 = 0x60;
const k_block_type_void_sentinel_byte: u8 = 0x40;

fn decodeFloat(comptime T: type, reader: anytype) !T {
    return switch (T) {
        f32 => @as(f32, @bitCast(try reader.readIntLittle(u32))),
        f64 => @as(f64, @bitCast(try reader.readIntLittle(u64))),
        else => unreachable,
    };
}

fn decodeVec(reader: anytype) !v128 {
    var bytes: [16]u8 = undefined;
    _ = try reader.read(&bytes);
    return std.mem.bytesToValue(v128, &bytes);
}

pub const ValType = enum(c_int) {
    I32,
    I64,
    F32,
    F64,
    V128,
    FuncRef,
    ExternRef,

    fn bytecodeToValtype(byte: u8) !ValType {
        return switch (byte) {
            0x7B => .V128,
            0x7F => .I32,
            0x7E => .I64,
            0x7D => .F32,
            0x7C => .F64,
            0x70 => .FuncRef,
            0x6F => .ExternRef,
            else => {
                return error.MalformedValType;
            },
        };
    }

    fn decode(reader: anytype) !ValType {
        return try bytecodeToValtype(try reader.readByte());
    }

    fn decodeReftype(reader: anytype) !ValType {
        var valtype = try decode(reader);
        if (isRefType(valtype) == false) {
            return error.MalformedReferenceType;
        }
        return valtype;
    }

    pub fn isRefType(valtype: ValType) bool {
        return switch (valtype) {
            .FuncRef => true,
            .ExternRef => true,
            else => false,
        };
    }

    pub fn count() comptime_int {
        return @typeInfo(ValType).Enum.fields.len;
    }
};

pub const Val = extern union {
    I32: i32,
    I64: i64,
    F32: f32,
    F64: f64,
    V128: v128,
    FuncRef: extern struct {
        index: u32, // index into functions
        module_instance: ?*ModuleInstance, // TODO make this an index as well

        // fn getModule(self: @This()) *ModuleInstance {
        //     return @as(*ModuleInstance, @alignCast(@ptrCast(self.module_instance)));
        // }
    },
    ExternRef: u32, // TODO figure out what this indexes

    const k_null_funcref: u32 = std.math.maxInt(u32);

    pub fn default(valtype: ValType) Val {
        return switch (valtype) {
            .I32 => Val{ .I32 = 0 },
            .I64 => Val{ .I64 = 0 },
            .F32 => Val{ .F32 = 0.0 },
            .F64 => Val{ .F64 = 0.0 },
            .V128 => Val{ .V128 = f32x4{ 0, 0, 0, 0 } },
            .FuncRef => nullRef(.FuncRef) catch unreachable,
            .ExternRef => nullRef(.ExternRef) catch unreachable,
        };
    }

    pub fn nullRef(valtype: ValType) !Val {
        return switch (valtype) {
            .FuncRef => funcrefFromIndex(Val.k_null_funcref),
            .ExternRef => Val{ .ExternRef = Val.k_null_funcref },
            else => error.MalformedBytecode,
        };
    }

    pub fn funcrefFromIndex(index: u32) Val {
        return Val{ .FuncRef = .{ .index = index, .module_instance = null } };
    }

    pub fn isNull(v: Val) bool {
        // Because FuncRef.index is located at the same memory location as ExternRef, this passes for both types
        return v.FuncRef.index == k_null_funcref;
    }

    pub fn eql(valtype: ValType, v1: Val, v2: Val) bool {
        return switch (valtype) {
            .I32 => v1.I32 == v2.I32,
            .I64 => v1.I64 == v2.I64,
            .F32 => v1.F32 == v2.F32,
            .F64 => v1.F64 == v2.F64,
            .V128 => @reduce(.And, v1.V128 == v2.V128),
            .FuncRef => std.meta.eql(v1.FuncRef, v2.FuncRef),
            .ExternRef => v1.ExternRef == v2.ExternRef,
        };
    }
};

test "Val.isNull" {
    const v1: Val = try Val.nullRef(.FuncRef);
    const v2: Val = try Val.nullRef(.ExternRef);

    try std.testing.expect(v1.isNull() == true);
    try std.testing.expect(v2.isNull() == true);

    const v3 = Val.funcrefFromIndex(12);
    const v4 = Val{ .ExternRef = 234 };

    try std.testing.expect(v3.isNull() == false);
    try std.testing.expect(v4.isNull() == false);
}

pub const TaggedVal = struct {
    val: Val,
    type: ValType,

    pub fn nullRef(valtype: ValType) !TaggedVal {
        return TaggedVal{
            .val = try Val.nullRef(valtype),
            .type = valtype,
        };
    }

    pub fn funcrefFromIndex(index: u32) TaggedVal {
        return TaggedVal{
            .val = Val.funcrefFromIndex(index),
            .type = .FuncRef,
        };
    }

    pub const HashMapContext = struct {
        pub fn hash(self: @This(), v: TaggedVal) u64 {
            _ = self;

            var hasher = std.hash.Wyhash.init(0);
            hasher.update(std.mem.asBytes(&v.val));
            hasher.update(std.mem.asBytes(&v.type));
            return hasher.final();
        }

        pub fn eql(self: @This(), a: TaggedVal, b: TaggedVal) bool {
            _ = self;

            return a.type == b.type and Val.eql(a.type, a.val, b.val);
        }
    };
};

pub const Limits = struct {
    min: u32,
    max: ?u32,

    fn decode(reader: anytype) !Limits {
        const has_max = try reader.readByte();
        if (has_max > 1) {
            return error.MalformedLimits;
        }
        const min = try common.decodeLEB128(u32, reader);
        var max: ?u32 = null;

        switch (has_max) {
            0 => {},
            1 => {
                max = try common.decodeLEB128(u32, reader);
                if (max.? < min) {
                    return error.ValidationLimitsMinMustNotBeLargerThanMax;
                }
            },
            else => unreachable,
        }

        return Limits{
            .min = min,
            .max = max,
        };
    }
};

const BlockType = enum {
    Void,
    ValType,
    TypeIndex,
};

pub const BlockTypeValue = union(BlockType) {
    Void: void,
    ValType: ValType,
    TypeIndex: u32,

    fn getBlocktypeParamTypes(blocktype: BlockTypeValue, module_def: *const ModuleDefinition) []const ValType {
        switch (blocktype) {
            else => return &BlockTypeStatics.empty,
            .TypeIndex => |index| return module_def.types.items[index].getParams(),
        }
    }

    fn getBlocktypeReturnTypes(blocktype: BlockTypeValue, module_def: *const ModuleDefinition) []const ValType {
        switch (blocktype) {
            .Void => return &BlockTypeStatics.empty,
            .ValType => |v| return switch (v) {
                .I32 => &BlockTypeStatics.valtype_i32,
                .I64 => &BlockTypeStatics.valtype_i64,
                .F32 => &BlockTypeStatics.valtype_f32,
                .F64 => &BlockTypeStatics.valtype_f64,
                .V128 => &BlockTypeStatics.valtype_v128,
                .FuncRef => &BlockTypeStatics.reftype_funcref,
                .ExternRef => &BlockTypeStatics.reftype_externref,
            },
            .TypeIndex => |index| return module_def.types.items[index].getReturns(),
        }
    }

    fn toU64(value: BlockTypeValue) u64 {
        comptime {
            std.debug.assert(@sizeOf(BlockTypeValue) == @sizeOf(u64));
        }
        const value_ptr: *const BlockTypeValue = &value;
        return @as(*const u64, @ptrCast(value_ptr)).*;
    }

    fn fromU64(value: u64) BlockTypeValue {
        const value_ptr: *const u64 = &value;
        return @as(*const BlockTypeValue, @ptrCast(value_ptr)).*;
    }
};

pub const BlockTypeStatics = struct {
    const empty = [_]ValType{};
    const valtype_i32 = [_]ValType{.I32};
    const valtype_i64 = [_]ValType{.I64};
    const valtype_f32 = [_]ValType{.F32};
    const valtype_f64 = [_]ValType{.F64};
    const valtype_v128 = [_]ValType{.V128};
    const reftype_funcref = [_]ValType{.FuncRef};
    const reftype_externref = [_]ValType{.ExternRef};
};

const ConstantExpressionType = enum {
    Value,
    Global,
};

pub const ConstantExpression = union(ConstantExpressionType) {
    Value: TaggedVal,
    Global: u32, // global index

    const ExpectedGlobalMut = enum {
        Any,
        Immutable,
    };

    fn decode(reader: anytype, module_def: *const ModuleDefinition, comptime expected_global_mut: ExpectedGlobalMut, expected_valtype: ValType) !ConstantExpression {
        const opcode = try WasmOpcode.decode(reader);

        const expr = switch (opcode) {
            .I32_Const => ConstantExpression{ .Value = TaggedVal{ .type = .I32, .val = .{ .I32 = try common.decodeLEB128(i32, reader) } } },
            .I64_Const => ConstantExpression{ .Value = TaggedVal{ .type = .I64, .val = .{ .I64 = try common.decodeLEB128(i64, reader) } } },
            .F32_Const => ConstantExpression{ .Value = TaggedVal{ .type = .F32, .val = .{ .F32 = try decodeFloat(f32, reader) } } },
            .F64_Const => ConstantExpression{ .Value = TaggedVal{ .type = .F64, .val = .{ .F64 = try decodeFloat(f64, reader) } } },
            .V128_Const => ConstantExpression{ .Value = TaggedVal{ .type = .V128, .val = .{ .V128 = try decodeVec(reader) } } },
            .Ref_Null => ConstantExpression{ .Value = try TaggedVal.nullRef(try ValType.decode(reader)) },
            .Ref_Func => ConstantExpression{ .Value = TaggedVal.funcrefFromIndex(try common.decodeLEB128(u32, reader)) },
            .Global_Get => ConstantExpression{ .Global = try common.decodeLEB128(u32, reader) },
            else => return error.ValidationBadConstantExpression,
        };

        if (opcode == .Global_Get) {
            try ModuleValidator.validateGlobalIndex(expr.Global, module_def);

            if (module_def.imports.globals.items.len <= expr.Global) {
                return error.ValidationConstantExpressionGlobalMustBeImport;
            }

            if (expected_global_mut == .Immutable) {
                if (expr.Global < module_def.imports.globals.items.len) {
                    if (module_def.imports.globals.items[expr.Global].mut != .Immutable) {
                        return error.ValidationConstantExpressionGlobalMustBeImmutable;
                    }
                } else {
                    const local_index: usize = module_def.imports.globals.items.len - expr.Global;
                    if (module_def.globals.items[local_index].mut != .Immutable) {
                        return error.ValidationConstantExpressionGlobalMustBeImmutable;
                    }
                }
            }

            var global_valtype: ValType = undefined;
            if (expr.Global < module_def.imports.globals.items.len) {
                const global_import_def: *const GlobalImportDefinition = &module_def.imports.globals.items[expr.Global];
                global_valtype = global_import_def.valtype;
            } else {
                const local_index: usize = module_def.imports.globals.items.len - expr.Global;
                const global_def: *const GlobalDefinition = &module_def.globals.items[local_index];
                global_valtype = global_def.valtype;
            }

            if (global_valtype != expected_valtype) {
                return error.ValidationConstantExpressionTypeMismatch;
            }
        } else {
            if (expr.Value.type != expected_valtype) {
                return error.ValidationConstantExpressionTypeMismatch;
            }
        }

        const end = @as(WasmOpcode, @enumFromInt(try reader.readByte()));
        if (end != .End) {
            return error.ValidationBadConstantExpression;
        }

        return expr;
    }

    pub fn resolve(self: *const ConstantExpression, store: *Store) Val {
        switch (self.*) {
            .Value => |val| {
                return val.val;
            },
            .Global => |global_index| {
                std.debug.assert(global_index < store.imports.globals.items.len + store.globals.items.len);
                const global: *GlobalInstance = store.getGlobal(global_index);
                return global.value;
            },
        }
    }

    pub fn resolveTo(self: *const ConstantExpression, store: *Store, comptime T: type) T {
        const val: Val = self.resolve(store);
        switch (T) {
            i32 => return val.I32,
            u32 => return @as(u32, @bitCast(val.I32)),
            i64 => return val.I64,
            u64 => return @as(u64, @bitCast(val.I64)),
            f32 => return val.F64,
            f64 => return val.F64,
            else => unreachable,
        }
    }

    pub fn resolveType(self: *const ConstantExpression, module_def: *const ModuleDefinition) ValType {
        switch (self.*) {
            .Value => |val| return val.type,
            .Global => |index| {
                if (index < module_def.imports.globals.items.len) {
                    const global_import_def: *const GlobalImportDefinition = &module_def.imports.globals.items[index];
                    return global_import_def.valtype;
                } else {
                    const local_index: usize = module_def.imports.globals.items.len - index;
                    const global_def: *const GlobalDefinition = &module_def.globals.items[local_index];
                    return global_def.valtype;
                }
                unreachable;
            },
        }
    }
};

pub const FunctionTypeDefinition = struct {
    types: std.ArrayList(ValType),
    num_params: u32,

    pub fn getParams(self: *const FunctionTypeDefinition) []const ValType {
        return self.types.items[0..self.num_params];
    }
    pub fn getReturns(self: *const FunctionTypeDefinition) []const ValType {
        return self.types.items[self.num_params..];
    }
    pub fn calcNumReturns(self: *const FunctionTypeDefinition) u32 {
        const total: u32 = @as(u32, @intCast(self.types.items.len));
        return total - self.num_params;
    }

    pub const SortContext = struct {
        const Self = @This();

        pub fn hash(_: Self, f: *FunctionTypeDefinition) u64 {
            var seed: u64 = 0;
            if (f.types.items.len > 0) {
                seed = std.hash.Murmur2_64.hash(std.mem.sliceAsBytes(f.types.items));
            }
            return std.hash.Murmur2_64.hashWithSeed(std.mem.asBytes(&f.num_params), seed);
        }

        pub fn eql(_: Self, a: *const FunctionTypeDefinition, b: *const FunctionTypeDefinition) bool {
            if (a.num_params != b.num_params or a.types.items.len != b.types.items.len) {
                return false;
            }

            for (a.types.items, 0..) |typeA, i| {
                var typeB = b.types.items[i];
                if (typeA != typeB) {
                    return false;
                }
            }

            return true;
        }

        fn less(context: Self, a: *FunctionTypeDefinition, b: *FunctionTypeDefinition) bool {
            var ord = Self.order(context, a, b);
            return ord == std.math.Order.lt;
        }

        fn order(context: Self, a: *FunctionTypeDefinition, b: *FunctionTypeDefinition) std.math.Order {
            var hashA = Self.hash(context, a);
            var hashB = Self.hash(context, b);

            if (hashA < hashB) {
                return std.math.Order.lt;
            } else if (hashA > hashB) {
                return std.math.Order.gt;
            } else {
                return std.math.Order.eq;
            }
        }
    };
};

pub const FunctionDefinition = struct {
    type_index: usize,
    instructions_begin: usize,
    instructions_end: usize,
    continuation: usize,
    locals: std.ArrayList(ValType), // TODO use a slice of a large contiguous array instead

    pub fn instructions(func: FunctionDefinition, module_def: ModuleDefinition) []Instruction {
        return module_def.code.instructions.items[func.instructions_begin..func.instructions_end];
    }

    pub fn numParamsAndLocals(func: FunctionDefinition, module_def: ModuleDefinition) usize {
        const func_type: *const FunctionTypeDefinition = func.typeDefinition(module_def);
        const param_types: []const ValType = func_type.getParams();
        return param_types.len + func.locals.items.len;
    }

    pub fn typeDefinition(func: FunctionDefinition, module_def: ModuleDefinition) *const FunctionTypeDefinition {
        return &module_def.types.items[func.type_index];
    }
};

const ExportType = enum(u8) {
    Function = 0x00,
    Table = 0x01,
    Memory = 0x02,
    Global = 0x03,
};

pub const ExportDefinition = struct {
    name: []const u8,
    index: u32,
};

pub const FunctionExport = struct {
    params: []const ValType,
    returns: []const ValType,
};

pub const FunctionHandleType = enum(u8) {
    Export,
    Import,
};

pub const FunctionHandle = struct {
    index: u32,
    type: FunctionHandleType,
};

pub const GlobalMut = enum(u8) {
    Immutable = 0,
    Mutable = 1,

    fn decode(reader: anytype) !GlobalMut {
        const byte = try reader.readByte();
        const value = std.meta.intToEnum(GlobalMut, byte) catch {
            return error.MalformedMutability;
        };
        return value;
    }
};

pub const GlobalDefinition = struct {
    valtype: ValType,
    mut: GlobalMut,
    expr: ConstantExpression,
};

pub const GlobalExport = struct {
    val: *Val,
    valtype: ValType,
    mut: GlobalMut,
};

pub const TableDefinition = struct {
    reftype: ValType,
    limits: Limits,
};

pub const MemoryDefinition = struct {
    limits: Limits,

    pub const k_page_size: usize = 64 * 1024;
    pub const k_max_pages: usize = std.math.powi(usize, 2, 16) catch unreachable;
};

pub const ElementMode = enum {
    Active,
    Passive,
    Declarative,
};

pub const ElementDefinition = struct {
    table_index: u32,
    mode: ElementMode,
    reftype: ValType,
    offset: ?ConstantExpression,
    elems_value: std.ArrayList(Val),
    elems_expr: std.ArrayList(ConstantExpression),
};

pub const DataMode = enum {
    Active,
    Passive,
};

pub const DataDefinition = struct {
    bytes: std.ArrayList(u8),
    memory_index: ?u32,
    offset: ?ConstantExpression,
    mode: DataMode,

    fn decode(reader: anytype, module_def: *const ModuleDefinition, allocator: std.mem.Allocator) !DataDefinition {
        var data_type: u32 = try common.decodeLEB128(u32, reader);
        if (data_type > 2) {
            return error.MalformedDataType;
        }

        var memory_index: ?u32 = null;
        if (data_type == 0x00) {
            memory_index = 0;
        } else if (data_type == 0x02) {
            memory_index = try common.decodeLEB128(u32, reader);
        }

        var mode = DataMode.Passive;
        var offset: ?ConstantExpression = null;
        if (data_type == 0x00 or data_type == 0x02) {
            mode = DataMode.Active;
            offset = try ConstantExpression.decode(reader, module_def, .Immutable, .I32);
        }

        var num_bytes = try common.decodeLEB128(u32, reader);
        var bytes = std.ArrayList(u8).init(allocator);
        try bytes.resize(num_bytes);
        var num_read = try reader.read(bytes.items);
        if (num_read != num_bytes) {
            return error.MalformedUnexpectedEnd;
        }

        if (memory_index) |index| {
            if (module_def.imports.memories.items.len + module_def.memories.items.len <= index) {
                return error.ValidationUnknownMemory;
            }
        }

        return DataDefinition{
            .bytes = bytes,
            .memory_index = memory_index,
            .offset = offset,
            .mode = mode,
        };
    }
};

pub const ImportNames = struct {
    module_name: []const u8,
    import_name: []const u8,
};

const FunctionImportDefinition = struct {
    names: ImportNames,
    type_index: u32,
};

const TableImportDefinition = struct {
    names: ImportNames,
    reftype: ValType,
    limits: Limits,
};

const MemoryImportDefinition = struct {
    names: ImportNames,
    limits: Limits,
};

const GlobalImportDefinition = struct {
    names: ImportNames,
    valtype: ValType,
    mut: GlobalMut,
};

const MemArg = struct {
    alignment: u32,
    offset: u32,

    fn decode(reader: anytype, comptime bitwidth: u32) !MemArg {
        std.debug.assert(bitwidth % 8 == 0);
        var memarg = MemArg{
            .alignment = try common.decodeLEB128(u32, reader),
            .offset = try common.decodeLEB128(u32, reader),
        };
        const bit_alignment = std.math.powi(u32, 2, memarg.alignment) catch return error.ValidationBadAlignment;
        if (bit_alignment > bitwidth / 8) {
            return error.ValidationBadAlignment;
        }
        return memarg;
    }
};

pub const MemoryOffsetAndLaneImmediates = struct {
    offset: u32,
    laneidx: u8,
};

pub const CallIndirectImmediates = struct {
    type_index: u32,
    table_index: u32,
};

pub const BranchTableImmediates = struct {
    label_ids: std.ArrayList(u32), // TODO optimize to make less allocations
    fallback_id: u32,
};

pub const TablePairImmediates = struct {
    index_x: u32,
    index_y: u32,
};

pub const BlockImmediates = struct {
    blocktype: BlockTypeValue,
    num_returns: u32,
    continuation: u32,
};

pub const IfImmediates = struct {
    blocktype: BlockTypeValue,
    num_returns: u32,
    else_continuation: u32,
    end_continuation: u32,
};

const InstructionImmediatesTypes = enum(u8) {
    Void,
    ValType,
    ValueI32,
    ValueF32,
    ValueI64,
    ValueF64,
    ValueVec,
    Index,
    LabelId,
    MemoryOffset,
    MemoryOffsetAndLane,
    Block,
    CallIndirect,
    TablePair,
    If,
    VecShuffle16,
};

pub const InstructionImmediates = union(InstructionImmediatesTypes) {
    Void: void,
    ValType: ValType,
    ValueI32: i32,
    ValueF32: f32,
    ValueI64: i64,
    ValueF64: f64,
    ValueVec: v128,
    Index: u32,
    LabelId: u32,
    MemoryOffset: u32,
    MemoryOffsetAndLane: MemoryOffsetAndLaneImmediates,
    Block: BlockImmediates,
    CallIndirect: CallIndirectImmediates,
    TablePair: TablePairImmediates,
    If: IfImmediates,
    VecShuffle16: [16]u8,
};

pub const Instruction = struct {
    opcode: Opcode,
    immediate: InstructionImmediates,

    fn decode(reader: anytype, module: *ModuleDefinition) !Instruction {
        const Helpers = struct {
            fn decodeBlockType(_reader: anytype, _module: *const ModuleDefinition) !InstructionImmediates {
                var blocktype: BlockTypeValue = undefined;

                const blocktype_raw = try _reader.readByte();
                const valtype_or_err = ValType.bytecodeToValtype(blocktype_raw);
                if (std.meta.isError(valtype_or_err)) {
                    if (blocktype_raw == k_block_type_void_sentinel_byte) {
                        blocktype = BlockTypeValue{ .Void = {} };
                    } else {
                        _reader.context.pos -= 1; // move the stream backwards 1 byte to reconstruct the integer
                        var index_33bit = try common.decodeLEB128(i33, _reader);
                        if (index_33bit < 0) {
                            return error.MalformedBytecode;
                        }
                        var index: u32 = @as(u32, @intCast(index_33bit));
                        if (index < _module.types.items.len) {
                            blocktype = BlockTypeValue{ .TypeIndex = index };
                        } else {
                            return error.ValidationUnknownBlockTypeIndex;
                        }
                    }
                } else {
                    var valtype: ValType = valtype_or_err catch unreachable;
                    blocktype = BlockTypeValue{ .ValType = valtype };
                }

                const num_returns: u32 = @as(u32, @intCast(blocktype.getBlocktypeReturnTypes(_module).len));

                return InstructionImmediates{
                    .Block = BlockImmediates{
                        .blocktype = blocktype,
                        .num_returns = num_returns,
                        .continuation = std.math.maxInt(u32), // will be set later in the code decode
                    },
                };
            }

            fn decodeTablePair(_reader: anytype) !InstructionImmediates {
                const elem_index = try common.decodeLEB128(u32, _reader);
                const table_index = try common.decodeLEB128(u32, _reader);

                return InstructionImmediates{
                    .TablePair = TablePairImmediates{
                        .index_x = elem_index,
                        .index_y = table_index,
                    },
                };
            }

            fn decodeMemoryOffsetAndLane(_reader: anytype, comptime bitwidth: u32) !InstructionImmediates {
                const memarg = try MemArg.decode(_reader, bitwidth);
                const laneidx = try _reader.readByte();
                return InstructionImmediates{ .MemoryOffsetAndLane = MemoryOffsetAndLaneImmediates{
                    .offset = memarg.offset,
                    .laneidx = laneidx,
                } };
            }
        };

        const wasm_op: WasmOpcode = try WasmOpcode.decode(reader);
        var opcode: Opcode = wasm_op.toOpcode();
        var immediate = InstructionImmediates{ .Void = {} };

        switch (opcode) {
            .Select_T => {
                const num_types = try common.decodeLEB128(u32, reader);
                if (num_types != 1) {
                    return error.ValidationSelectArity;
                }
                immediate = InstructionImmediates{ .ValType = try ValType.decode(reader) };
            },
            .Local_Get => {
                immediate = InstructionImmediates{ .Index = try common.decodeLEB128(u32, reader) };
            },
            .Local_Set => {
                immediate = InstructionImmediates{ .Index = try common.decodeLEB128(u32, reader) };
            },
            .Local_Tee => {
                immediate = InstructionImmediates{ .Index = try common.decodeLEB128(u32, reader) };
            },
            .Global_Get => {
                immediate = InstructionImmediates{ .Index = try common.decodeLEB128(u32, reader) };
            },
            .Global_Set => {
                immediate = InstructionImmediates{ .Index = try common.decodeLEB128(u32, reader) };
            },
            .Table_Get => {
                immediate = InstructionImmediates{ .Index = try common.decodeLEB128(u32, reader) };
            },
            .Table_Set => {
                immediate = InstructionImmediates{ .Index = try common.decodeLEB128(u32, reader) };
            },
            .I32_Const => {
                immediate = InstructionImmediates{ .ValueI32 = try common.decodeLEB128(i32, reader) };
            },
            .I64_Const => {
                immediate = InstructionImmediates{ .ValueI64 = try common.decodeLEB128(i64, reader) };
            },
            .F32_Const => {
                immediate = InstructionImmediates{ .ValueF32 = try decodeFloat(f32, reader) };
            },
            .F64_Const => {
                immediate = InstructionImmediates{ .ValueF64 = try decodeFloat(f64, reader) };
            },
            .Block => {
                immediate = try Helpers.decodeBlockType(reader, module);
            },
            .Loop => {
                immediate = try Helpers.decodeBlockType(reader, module);
            },
            .If => {
                const block_immediates: InstructionImmediates = try Helpers.decodeBlockType(reader, module);
                immediate = InstructionImmediates{
                    .If = IfImmediates{
                        .blocktype = block_immediates.Block.blocktype,
                        .num_returns = block_immediates.Block.num_returns,
                        .else_continuation = block_immediates.Block.continuation,
                        .end_continuation = block_immediates.Block.continuation,
                    },
                };
            },
            .IfNoElse => unreachable, // we convert the If opcode to IfNoElse only after reaching the end of the block, not when decoding the opcode and immediates
            .Branch => {
                immediate = InstructionImmediates{ .LabelId = try common.decodeLEB128(u32, reader) };
            },
            .Branch_If => {
                immediate = InstructionImmediates{ .LabelId = try common.decodeLEB128(u32, reader) };
            },
            .Branch_Table => {
                const table_length = try common.decodeLEB128(u32, reader);

                var label_ids = std.ArrayList(u32).init(module.allocator);
                try label_ids.ensureTotalCapacity(table_length);

                var index: u32 = 0;
                while (index < table_length) : (index += 1) {
                    var id = try common.decodeLEB128(u32, reader);
                    label_ids.addOneAssumeCapacity().* = id;
                }
                var fallback_id = try common.decodeLEB128(u32, reader);

                var branch_table = BranchTableImmediates{
                    .label_ids = label_ids,
                    .fallback_id = fallback_id,
                };

                for (module.code.branch_table.items, 0..) |*item, i| {
                    if (item.fallback_id == branch_table.fallback_id) {
                        if (std.mem.eql(u32, item.label_ids.items, branch_table.label_ids.items)) {
                            immediate = InstructionImmediates{ .Index = @as(u32, @intCast(i)) };
                            break;
                        }
                    }
                }

                if (std.meta.activeTag(immediate) == .Void) {
                    immediate = InstructionImmediates{ .Index = @as(u32, @intCast(module.code.branch_table.items.len)) };
                    try module.code.branch_table.append(branch_table);
                } else {
                    // don't need this anymore since we're reusing the existing one
                    branch_table.label_ids.deinit();
                }
            },
            .Call => {
                immediate = InstructionImmediates{ .Index = try common.decodeLEB128(u32, reader) }; // function index
            },
            .Call_Indirect => {
                immediate = InstructionImmediates{ .CallIndirect = .{
                    .type_index = try common.decodeLEB128(u32, reader),
                    .table_index = try common.decodeLEB128(u32, reader),
                } };
            },
            .I32_Load => {
                var memarg = try MemArg.decode(reader, 32);
                immediate = InstructionImmediates{ .MemoryOffset = memarg.offset };
            },
            .I64_Load => {
                var memarg = try MemArg.decode(reader, 64);
                immediate = InstructionImmediates{ .MemoryOffset = memarg.offset };
            },
            .F32_Load => {
                var memarg = try MemArg.decode(reader, 32);
                immediate = InstructionImmediates{ .MemoryOffset = memarg.offset };
            },
            .F64_Load => {
                var memarg = try MemArg.decode(reader, 64);
                immediate = InstructionImmediates{ .MemoryOffset = memarg.offset };
            },
            .I32_Load8_S => {
                var memarg = try MemArg.decode(reader, 8);
                immediate = InstructionImmediates{ .MemoryOffset = memarg.offset };
            },
            .I32_Load8_U => {
                var memarg = try MemArg.decode(reader, 8);
                immediate = InstructionImmediates{ .MemoryOffset = memarg.offset };
            },
            .I32_Load16_S => {
                var memarg = try MemArg.decode(reader, 16);
                immediate = InstructionImmediates{ .MemoryOffset = memarg.offset };
            },
            .I32_Load16_U => {
                var memarg = try MemArg.decode(reader, 16);
                immediate = InstructionImmediates{ .MemoryOffset = memarg.offset };
            },
            .I64_Load8_S => {
                var memarg = try MemArg.decode(reader, 8);
                immediate = InstructionImmediates{ .MemoryOffset = memarg.offset };
            },
            .I64_Load8_U => {
                var memarg = try MemArg.decode(reader, 8);
                immediate = InstructionImmediates{ .MemoryOffset = memarg.offset };
            },
            .I64_Load16_S => {
                var memarg = try MemArg.decode(reader, 16);
                immediate = InstructionImmediates{ .MemoryOffset = memarg.offset };
            },
            .I64_Load16_U => {
                var memarg = try MemArg.decode(reader, 16);
                immediate = InstructionImmediates{ .MemoryOffset = memarg.offset };
            },
            .I64_Load32_S => {
                var memarg = try MemArg.decode(reader, 32);
                immediate = InstructionImmediates{ .MemoryOffset = memarg.offset };
            },
            .I64_Load32_U => {
                var memarg = try MemArg.decode(reader, 32);
                immediate = InstructionImmediates{ .MemoryOffset = memarg.offset };
            },
            .I32_Store => {
                var memarg = try MemArg.decode(reader, 32);
                immediate = InstructionImmediates{ .MemoryOffset = memarg.offset };
            },
            .I64_Store => {
                var memarg = try MemArg.decode(reader, 64);
                immediate = InstructionImmediates{ .MemoryOffset = memarg.offset };
            },
            .F32_Store => {
                var memarg = try MemArg.decode(reader, 32);
                immediate = InstructionImmediates{ .MemoryOffset = memarg.offset };
            },
            .F64_Store => {
                var memarg = try MemArg.decode(reader, 64);
                immediate = InstructionImmediates{ .MemoryOffset = memarg.offset };
            },
            .I32_Store8 => {
                var memarg = try MemArg.decode(reader, 8);
                immediate = InstructionImmediates{ .MemoryOffset = memarg.offset };
            },
            .I32_Store16 => {
                var memarg = try MemArg.decode(reader, 16);
                immediate = InstructionImmediates{ .MemoryOffset = memarg.offset };
            },
            .I64_Store8 => {
                var memarg = try MemArg.decode(reader, 8);
                immediate = InstructionImmediates{ .MemoryOffset = memarg.offset };
            },
            .I64_Store16 => {
                var memarg = try MemArg.decode(reader, 16);
                immediate = InstructionImmediates{ .MemoryOffset = memarg.offset };
            },
            .I64_Store32 => {
                var memarg = try MemArg.decode(reader, 32);
                immediate = InstructionImmediates{ .MemoryOffset = memarg.offset };
            },
            .Memory_Size => {
                var reserved = try reader.readByte();
                if (reserved != 0x00) {
                    return error.MalformedMissingZeroByte;
                }
            },
            .Memory_Grow => {
                var reserved = try reader.readByte();
                if (reserved != 0x00) {
                    return error.MalformedMissingZeroByte;
                }
            },
            .Memory_Init => {
                try ModuleValidator.validateMemoryIndex(module);

                if (module.data_count == null) {
                    return error.MalformedMissingDataCountSection;
                }

                immediate = InstructionImmediates{ .Index = try common.decodeLEB128(u32, reader) }; // dataidx

                var reserved = try reader.readByte();
                if (reserved != 0x00) {
                    return error.MalformedMissingZeroByte;
                }
            },
            .Ref_Null => {
                var valtype = try ValType.decode(reader);
                if (valtype.isRefType() == false) {
                    return error.MalformedBytecode;
                }

                immediate = InstructionImmediates{ .ValType = valtype };
            },
            .Ref_Func => {
                immediate = InstructionImmediates{ .Index = try common.decodeLEB128(u32, reader) }; // funcidx
            },
            .Data_Drop => {
                immediate = InstructionImmediates{ .Index = try common.decodeLEB128(u32, reader) }; // dataidx
            },
            .Memory_Copy => {
                var reserved = try reader.readByte();
                if (reserved != 0x00) {
                    return error.MalformedMissingZeroByte;
                }
                reserved = try reader.readByte();
                if (reserved != 0x00) {
                    return error.MalformedMissingZeroByte;
                }
            },
            .Memory_Fill => {
                var reserved = try reader.readByte();
                if (reserved != 0x00) {
                    return error.MalformedMissingZeroByte;
                }
            },
            .Table_Init => {
                immediate = try Helpers.decodeTablePair(reader);
            },
            .Elem_Drop => {
                immediate = InstructionImmediates{ .Index = try common.decodeLEB128(u32, reader) }; // elemidx
            },
            .Table_Copy => {
                immediate = try Helpers.decodeTablePair(reader);
            },
            .Table_Grow => {
                immediate = InstructionImmediates{ .Index = try common.decodeLEB128(u32, reader) }; // elemidx
            },
            .Table_Size => {
                immediate = InstructionImmediates{ .Index = try common.decodeLEB128(u32, reader) }; // elemidx
            },
            .Table_Fill => {
                immediate = InstructionImmediates{ .Index = try common.decodeLEB128(u32, reader) }; // elemidx
            },
            .V128_Load => {
                var memarg = try MemArg.decode(reader, 128);
                immediate = InstructionImmediates{ .MemoryOffset = memarg.offset };
            },
            .V128_Load8x8_S, .V128_Load8x8_U => {
                var memarg = try MemArg.decode(reader, 8 * 8);
                immediate = InstructionImmediates{ .MemoryOffset = memarg.offset };
            },
            .V128_Load16x4_S, .V128_Load16x4_U => {
                var memarg = try MemArg.decode(reader, 16 * 4);
                immediate = InstructionImmediates{ .MemoryOffset = memarg.offset };
            },
            .V128_Load32x2_S, .V128_Load32x2_U => {
                var memarg = try MemArg.decode(reader, 32 * 2);
                immediate = InstructionImmediates{ .MemoryOffset = memarg.offset };
            },
            .V128_Load8_Splat => {
                var memarg = try MemArg.decode(reader, 8);
                immediate = InstructionImmediates{ .MemoryOffset = memarg.offset };
            },
            .V128_Load16_Splat => {
                var memarg = try MemArg.decode(reader, 16);
                immediate = InstructionImmediates{ .MemoryOffset = memarg.offset };
            },
            .V128_Load32_Splat => {
                var memarg = try MemArg.decode(reader, 32);
                immediate = InstructionImmediates{ .MemoryOffset = memarg.offset };
            },
            .V128_Load64_Splat => {
                var memarg = try MemArg.decode(reader, 64);
                immediate = InstructionImmediates{ .MemoryOffset = memarg.offset };
            },
            .I8x16_Extract_Lane_S, .I8x16_Extract_Lane_U, .I8x16_Replace_Lane, .I16x8_Extract_Lane_S, .I16x8_Extract_Lane_U, .I16x8_Replace_Lane, .I32x4_Extract_Lane, .I32x4_Replace_Lane, .I64x2_Extract_Lane, .I64x2_Replace_Lane, .F32x4_Extract_Lane, .F32x4_Replace_Lane, .F64x2_Extract_Lane, .F64x2_Replace_Lane => {
                immediate = InstructionImmediates{ .Index = try reader.readByte() }; // laneidx
            },
            .V128_Store => {
                var memarg = try MemArg.decode(reader, 128);
                immediate = InstructionImmediates{ .MemoryOffset = memarg.offset };
            },
            .V128_Const => {
                immediate = InstructionImmediates{ .ValueVec = try decodeVec(reader) };
            },
            .I8x16_Shuffle => {
                var lane_indices: [16]u8 = undefined;
                for (&lane_indices) |*v| {
                    const laneidx: u8 = try reader.readByte();
                    v.* = laneidx;
                }
                immediate = InstructionImmediates{ .VecShuffle16 = lane_indices };
            },
            .V128_Load8_Lane => {
                immediate = try Helpers.decodeMemoryOffsetAndLane(reader, 8);
            },
            .V128_Load16_Lane => {
                immediate = try Helpers.decodeMemoryOffsetAndLane(reader, 16);
            },
            .V128_Load32_Lane => {
                immediate = try Helpers.decodeMemoryOffsetAndLane(reader, 32);
            },
            .V128_Load64_Lane => {
                immediate = try Helpers.decodeMemoryOffsetAndLane(reader, 64);
            },
            .V128_Store8_Lane => {
                immediate = try Helpers.decodeMemoryOffsetAndLane(reader, 8);
            },
            .V128_Store16_Lane => {
                immediate = try Helpers.decodeMemoryOffsetAndLane(reader, 16);
            },
            .V128_Store32_Lane => {
                immediate = try Helpers.decodeMemoryOffsetAndLane(reader, 32);
            },
            .V128_Store64_Lane => {
                immediate = try Helpers.decodeMemoryOffsetAndLane(reader, 64);
            },
            .V128_Load32_Zero => {
                var memarg = try MemArg.decode(reader, 128);
                immediate = InstructionImmediates{ .MemoryOffset = memarg.offset };
            },
            .V128_Load64_Zero => {
                var memarg = try MemArg.decode(reader, 128);
                immediate = InstructionImmediates{ .MemoryOffset = memarg.offset };
            },
            else => {},
        }

        return .{
            .opcode = opcode,
            .immediate = immediate,
        };
    }
};

const CustomSection = struct {
    name: []const u8,
    data: std.ArrayList(u8),
};

pub const NameCustomSection = struct {
    const NameAssoc = struct {
        name: []const u8,
        func_index: usize,

        fn cmp(_: void, a: NameAssoc, b: NameAssoc) bool {
            return a.func_index < b.func_index;
        }

        fn order(_: void, a: NameAssoc, b: NameAssoc) std.math.Order {
            if (a.func_index < b.func_index) {
                return .lt;
            } else if (a.func_index > b.func_index) {
                return .gt;
            } else {
                return .eq;
            }
        }
    };

    // all string slices here are static strings or point into CustomSection.data - no need to free
    module_name: []const u8,
    function_names: std.ArrayList(NameAssoc),

    // function_index_to_local_names_begin: std.hash_map.AutoHashMap(u32, u32),
    // local_names: std.ArrayList([]const u8),

    fn init(allocator: std.mem.Allocator) NameCustomSection {
        return NameCustomSection{
            .module_name = "",
            .function_names = std.ArrayList(NameAssoc).init(allocator),
        };
    }

    fn deinit(self: *NameCustomSection) void {
        self.function_names.deinit();
    }

    fn decode(self: *NameCustomSection, module_definition: *const ModuleDefinition, bytes: []const u8) MalformedError!void {
        self.decodeInternal(module_definition, bytes) catch |err| {
            std.debug.print("NameCustomSection.decode: caught error from internal: {}", .{err});
            return MalformedError.MalformedCustomSection;
        };
    }

    fn decodeInternal(self: *NameCustomSection, module_definition: *const ModuleDefinition, bytes: []const u8) !void {
        const DecodeHelpers = struct {
            fn readName(stream: anytype) ![]const u8 {
                var reader = stream.reader();
                const name_length = try common.decodeLEB128(u32, reader);
                const name: []const u8 = stream.buffer[stream.pos .. stream.pos + name_length];
                try stream.seekBy(name_length);
                return name;
            }
        };

        var fixed_buffer_stream = std.io.fixedBufferStream(bytes);
        var reader = fixed_buffer_stream.reader();

        while (try fixed_buffer_stream.getPos() != try fixed_buffer_stream.getEndPos()) {
            const section_code = try reader.readByte();
            const section_size = try common.decodeLEB128(u32, reader);

            switch (section_code) {
                0 => {
                    self.module_name = try DecodeHelpers.readName(&fixed_buffer_stream);
                },
                1 => {
                    const num_func_names = try common.decodeLEB128(u32, reader);
                    try self.function_names.ensureTotalCapacity(num_func_names);

                    var index: u32 = 0;
                    while (index < num_func_names) : (index += 1) {
                        const func_index = try common.decodeLEB128(u32, reader);
                        const func_name: []const u8 = try DecodeHelpers.readName(&fixed_buffer_stream);
                        self.function_names.appendAssumeCapacity(NameAssoc{
                            .name = func_name,
                            .func_index = func_index,
                        });
                    }

                    std.sort.heap(NameAssoc, self.function_names.items, {}, NameAssoc.cmp);
                },
                2 => { // TODO locals
                    try fixed_buffer_stream.seekBy(section_size);
                },
                else => {
                    try fixed_buffer_stream.seekBy(section_size);
                },
            }
        }

        if (self.module_name.len == 0) {
            if (module_definition.debug_name.len > 0) {
                self.module_name = module_definition.debug_name;
            } else {
                self.module_name = "<unknown_module>";
            }
        }
    }

    pub fn getModuleName(self: *const NameCustomSection) []const u8 {
        return self.module_name;
    }

    pub fn findFunctionName(self: *const NameCustomSection, func_index: usize) []const u8 {
        if (func_index < self.function_names.items.len) {
            if (self.function_names.items[func_index].func_index == func_index) {
                return self.function_names.items[func_index].name;
            } else {
                const temp_nameassoc = NameAssoc{
                    .name = "",
                    .func_index = func_index,
                };

                if (std.sort.binarySearch(NameAssoc, temp_nameassoc, self.function_names.items, {}, NameAssoc.order)) |found_index| {
                    return self.function_names.items[found_index].name;
                }
            }
        }
        return "<unknown_function>";
    }

    // fn findFunctionLocalName(self: *NameCustomSection, func_index: u32, local_index: u32) []const u8 {
    //     return "<unknown_local>";
    // }
};

const ModuleValidator = struct {
    const ControlFrame = struct {
        opcode: Opcode,
        start_types: []const ValType,
        end_types: []const ValType,
        types_stack_height: usize,
        is_function: bool,
        is_unreachable: bool,
    };

    // Note that we use a nullable ValType here to map to the "Unknown" value type as described in the wasm spec
    // validation algorithm: https://webassembly.github.io/spec/core/appendix/algorithm.html
    type_stack: std.ArrayList(?ValType),
    control_stack: std.ArrayList(ControlFrame),
    control_types: StableArray(ValType),

    fn init(allocator: std.mem.Allocator) ModuleValidator {
        return ModuleValidator{
            .type_stack = std.ArrayList(?ValType).init(allocator),
            .control_stack = std.ArrayList(ControlFrame).init(allocator),
            .control_types = StableArray(ValType).init(1 * 1024 * 1024),
        };
    }

    fn deinit(self: *ModuleValidator) void {
        self.type_stack.deinit();
        self.control_stack.deinit();
        self.control_types.deinit();
    }

    fn validateTypeIndex(index: u64, module: *const ModuleDefinition) !void {
        if (module.types.items.len <= index) {
            return error.ValidationUnknownType;
        }
    }

    fn validateGlobalIndex(index: u64, module: *const ModuleDefinition) !void {
        if (module.imports.globals.items.len + module.globals.items.len <= index) {
            return error.ValidationUnknownGlobal;
        }
    }

    fn validateTableIndex(index: u64, module: *const ModuleDefinition) !void {
        if (module.imports.tables.items.len + module.tables.items.len <= index) {
            return error.ValidationUnknownTable;
        }
    }

    fn getTableReftype(module: *const ModuleDefinition, index: u64) !ValType {
        if (index < module.imports.tables.items.len) {
            return module.imports.tables.items[index].reftype;
        }

        const local_index = index - module.imports.tables.items.len;
        if (local_index < module.tables.items.len) {
            return module.tables.items[local_index].reftype;
        }

        return error.ValidationUnknownTable;
    }

    fn validateFunctionIndex(index: u64, module: *const ModuleDefinition) !void {
        if (module.imports.functions.items.len + module.functions.items.len <= index) {
            return error.ValidationUnknownFunction;
        }
    }

    fn validateMemoryIndex(module: *const ModuleDefinition) !void {
        if (module.imports.memories.items.len + module.memories.items.len < 1) {
            return error.ValidationUnknownMemory;
        }
    }

    fn validateElementIndex(index: u64, module: *const ModuleDefinition) !void {
        if (module.elements.items.len <= index) {
            return error.ValidationUnknownElement;
        }
    }

    fn validateDataIndex(index: u64, module: *const ModuleDefinition) !void {
        if (module.data_count.? <= index) {
            return error.ValidationUnknownData;
        }
    }

    fn beginValidateCode(self: *ModuleValidator, module: *const ModuleDefinition, func: *const FunctionDefinition) !void {
        try validateTypeIndex(func.type_index, module);

        const func_type_def: *const FunctionTypeDefinition = &module.types.items[func.type_index];

        try self.pushControl(Opcode.Call, func_type_def.getParams(), func_type_def.getReturns());
    }

    fn validateCode(self: *ModuleValidator, module: *const ModuleDefinition, func: *const FunctionDefinition, instruction: Instruction) !void {
        const Helpers = struct {
            fn popReturnTypes(validator: *ModuleValidator, types: []const ValType) !void {
                var i = types.len;
                while (i > 0) {
                    i -= 1;
                    try validator.popType(types[i]);
                }
            }

            fn enterBlock(validator: *ModuleValidator, module_: *const ModuleDefinition, instruction_: Instruction) !void {
                const blocktype: BlockTypeValue = switch (instruction_.immediate) {
                    .Block => |v| v.blocktype,
                    .If => |v| v.blocktype,
                    else => unreachable,
                };

                var start_types: []const ValType = blocktype.getBlocktypeParamTypes(module_);
                var end_types: []const ValType = blocktype.getBlocktypeReturnTypes(module_);
                try popReturnTypes(validator, start_types);

                try validator.pushControl(instruction_.opcode, start_types, end_types);
            }

            fn getLocalValtype(validator: *const ModuleValidator, module_: *const ModuleDefinition, func_: *const FunctionDefinition, locals_index: u64) !ValType {
                var i = validator.control_stack.items.len - 1;
                while (i >= 0) : (i -= 1) {
                    const frame: *const ControlFrame = &validator.control_stack.items[i];
                    if (frame.is_function) {
                        const func_type: *const FunctionTypeDefinition = &module_.types.items[func_.type_index];
                        if (locals_index < func_type.num_params) {
                            return func_type.getParams()[locals_index];
                        } else {
                            if (func_.locals.items.len <= locals_index - func_type.num_params) {
                                return error.ValidationUnknownLocal;
                            }
                            return func_.locals.items[locals_index - func_type.num_params];
                        }
                    }
                }
                unreachable;
            }

            const GlobalMutablilityRequirement = enum {
                None,
                Mutable,
            };

            fn getGlobalValtype(module_: *const ModuleDefinition, global_index: u64, required_mutability: GlobalMutablilityRequirement) !ValType {
                if (global_index < module_.imports.globals.items.len) {
                    const global: *const GlobalImportDefinition = &module_.imports.globals.items[global_index];
                    if (required_mutability == .Mutable and global.mut == .Immutable) {
                        return error.ValidationImmutableGlobal;
                    }
                    return global.valtype;
                }

                const module_global_index = global_index - module_.imports.globals.items.len;
                if (module_global_index < module_.globals.items.len) {
                    const global: *const GlobalDefinition = &module_.globals.items[module_global_index];
                    if (required_mutability == .Mutable and global.mut == .Immutable) {
                        return error.ValidationImmutableGlobal;
                    }
                    return global.valtype;
                }

                return error.ValidationUnknownGlobal;
            }

            fn vecLaneTypeToValtype(comptime T: type) ValType {
                return switch (T) {
                    i8 => .I32,
                    u8 => .I32,
                    i16 => .I32,
                    u16 => .I32,
                    i32 => .I32,
                    i64 => .I64,
                    f32 => .F32,
                    f64 => .F64,
                    else => @compileError("unsupported lane type"),
                };
            }

            fn validateNumericUnaryOp(validator: *ModuleValidator, pop_type: ValType, push_type: ValType) !void {
                try validator.popType(pop_type);
                try validator.pushType(push_type);
            }

            fn validateNumericBinaryOp(validator: *ModuleValidator, pop_type: ValType, push_type: ValType) !void {
                try validator.popType(pop_type);
                try validator.popType(pop_type);
                try validator.pushType(push_type);
            }

            fn validateLoadOp(validator: *ModuleValidator, module_: *const ModuleDefinition, load_type: ValType) !void {
                try validator.popType(.I32);
                try validateMemoryIndex(module_);
                try validator.pushType(load_type);
            }

            fn validateStoreOp(validator: *ModuleValidator, module_: *const ModuleDefinition, store_type: ValType) !void {
                try validateMemoryIndex(module_);
                try validator.popType(store_type);
                try validator.popType(.I32);
            }

            fn validateVectorLane(comptime T: type, laneidx: u32) !void {
                const vec_type_info = @typeInfo(T).Vector;
                if (vec_type_info.len <= laneidx) {
                    return error.ValidationInvalidLaneIndex;
                }
            }

            fn validateLoadLaneOp(validator: *ModuleValidator, module_: *const ModuleDefinition, instruction_: Instruction, comptime T: type) !void {
                try validateVectorLane(T, instruction_.immediate.MemoryOffsetAndLane.laneidx);
                try validator.popType(.V128);
                try validator.popType(.I32);
                try validateMemoryIndex(module_);
                try validator.pushType(.V128);
            }

            fn validateStoreLaneOp(validator: *ModuleValidator, module_: *const ModuleDefinition, instruction_: Instruction, comptime T: type) !void {
                try validateVectorLane(T, instruction_.immediate.MemoryOffsetAndLane.laneidx);
                try validator.popType(.V128);
                try validator.popType(.I32);
                try validateMemoryIndex(module_);
            }

            fn validateVecExtractLane(comptime T: type, validator: *ModuleValidator, instruction_: Instruction) !void {
                try validateVectorLane(T, instruction_.immediate.Index);
                const lane_valtype = vecLaneTypeToValtype(@typeInfo(T).Vector.child);
                try validator.popType(.V128);
                try validator.pushType(lane_valtype);
            }

            fn validateVecReplaceLane(comptime T: type, validator: *ModuleValidator, instruction_: Instruction) !void {
                try validateVectorLane(T, instruction_.immediate.Index);
                const lane_valtype = vecLaneTypeToValtype(@typeInfo(T).Vector.child);
                try validator.popType(lane_valtype);
                try validator.popType(.V128);
                try validator.pushType(.V128);
            }

            fn getControlTypes(validator: *ModuleValidator, control_index: usize) ![]const ValType {
                if (validator.control_stack.items.len <= control_index) {
                    return error.ValidationUnknownLabel;
                }
                const stack_index = validator.control_stack.items.len - control_index - 1;
                var frame: *ControlFrame = &validator.control_stack.items[stack_index];
                return if (frame.opcode != .Loop) frame.end_types else frame.start_types;
            }

            fn markFrameInstructionsUnreachable(validator: *ModuleValidator) !void {
                var frame: *ControlFrame = &validator.control_stack.items[validator.control_stack.items.len - 1];
                try validator.type_stack.resize(frame.types_stack_height);
                frame.is_unreachable = true;
            }

            fn popPushFuncTypes(validator: *ModuleValidator, type_index: usize, module_: *const ModuleDefinition) !void {
                const func_type: *const FunctionTypeDefinition = &module_.types.items[type_index];

                try popReturnTypes(validator, func_type.getParams());
                for (func_type.getReturns()) |valtype| {
                    try validator.pushType(valtype);
                }
            }
        };
        switch (instruction.opcode) {
            .Invalid => unreachable,
            .Unreachable => {
                try Helpers.markFrameInstructionsUnreachable(self);
            },
            .DebugTrap, .Noop => {},
            .Drop => {
                _ = try self.popAnyType();
            },
            .Block => {
                try Helpers.enterBlock(self, module, instruction);
            },
            .Loop => {
                try Helpers.enterBlock(self, module, instruction);
            },
            .If, .IfNoElse => {
                try self.popType(.I32);
                try Helpers.enterBlock(self, module, instruction);
            },
            .Else => {
                const frame: ControlFrame = try self.popControl();
                if (frame.opcode.isIf() == false) {
                    return error.ValidationIfElseMismatch;
                }
                try self.pushControl(.Else, frame.start_types, frame.end_types);
            },
            .End => {
                const frame: ControlFrame = try self.popControl();

                // if must have matching else block when returns are expected and the params don't match
                if (frame.opcode.isIf() and !std.mem.eql(ValType, frame.start_types, frame.end_types)) {
                    return error.ValidationTypeMismatch;
                }

                if (self.control_stack.items.len > 0) {
                    for (frame.end_types) |valtype| {
                        try self.pushType(valtype);
                    }
                }
                try self.freeControlTypes(&frame);
            },
            .Branch => {
                const control_index: u64 = instruction.immediate.LabelId;
                const block_return_types: []const ValType = try Helpers.getControlTypes(self, control_index);

                try Helpers.popReturnTypes(self, block_return_types);
                try Helpers.markFrameInstructionsUnreachable(self);
            },
            .Branch_If => {
                const control_index: u64 = instruction.immediate.LabelId;
                const block_return_types: []const ValType = try Helpers.getControlTypes(self, control_index);
                try self.popType(.I32);

                try Helpers.popReturnTypes(self, block_return_types);
                for (block_return_types) |valtype| {
                    try self.pushType(valtype);
                }
            },
            .Branch_Table => {
                var immediates: *const BranchTableImmediates = &module.code.branch_table.items[instruction.immediate.Index];

                const fallback_block_return_types: []const ValType = try Helpers.getControlTypes(self, immediates.fallback_id);

                try self.popType(.I32);

                for (immediates.label_ids.items) |control_index| {
                    const block_return_types: []const ValType = try Helpers.getControlTypes(self, control_index);

                    if (fallback_block_return_types.len != block_return_types.len) {
                        return error.ValidationTypeMismatch;
                    }

                    // Seems like the wabt validation code for br_table is implemented by "peeking" at types on the stack
                    // instead of actually popping/pushing them. This allows certain block type mismatches to be considered
                    // valid when the current block is marked unreachable.
                    const frame: *const ControlFrame = &self.control_stack.items[control_index];
                    const type_stack: []const ?ValType = self.type_stack.items[frame.types_stack_height..];

                    var i: usize = block_return_types.len;
                    while (i > 0) : (i -= 1) {
                        if (!frame.is_unreachable and frame.types_stack_height < type_stack.len) {
                            if (type_stack[type_stack.len - i] != block_return_types[block_return_types.len - i]) {
                                return error.ValidationTypeMismatch;
                            }
                        }
                    }
                }

                try Helpers.popReturnTypes(self, fallback_block_return_types);
                try Helpers.markFrameInstructionsUnreachable(self);
            },
            .Return => {
                const block_return_types: []const ValType = try Helpers.getControlTypes(self, self.control_stack.items.len - 1);
                try Helpers.popReturnTypes(self, block_return_types);
                try Helpers.markFrameInstructionsUnreachable(self);
            },
            .Call => {
                const func_index: u64 = instruction.immediate.Index;
                if (module.imports.functions.items.len + module.functions.items.len <= func_index) {
                    return error.ValidationUnknownFunction;
                }

                var type_index: usize = module.getFuncTypeIndex(func_index);
                try Helpers.popPushFuncTypes(self, type_index, module);
            },
            .Call_Indirect => {
                const immediates: CallIndirectImmediates = instruction.immediate.CallIndirect;

                try validateTypeIndex(immediates.type_index, module);
                try validateTableIndex(immediates.table_index, module);

                try self.popType(.I32);

                try Helpers.popPushFuncTypes(self, immediates.type_index, module);
            },
            .Select => {
                try self.popType(.I32);
                const valtype1_or_null: ?ValType = try self.popAnyType();
                const valtype2_or_null: ?ValType = try self.popAnyType();
                if (valtype1_or_null == null) {
                    try self.pushType(valtype2_or_null);
                } else if (valtype2_or_null == null) {
                    try self.pushType(valtype1_or_null);
                } else {
                    const valtype1 = valtype1_or_null.?;
                    const valtype2 = valtype2_or_null.?;
                    if (valtype1 != valtype2) {
                        return error.ValidationTypeMismatch;
                    }
                    if (valtype1.isRefType()) {
                        return error.ValidationTypeMustBeNumeric;
                    }
                    try self.pushType(valtype1);
                }
            },
            .Select_T => {
                const valtype: ValType = instruction.immediate.ValType;
                try self.popType(.I32);
                try self.popType(valtype);
                try self.popType(valtype);
                try self.pushType(valtype);
            },
            .Local_Get => {
                const valtype = try Helpers.getLocalValtype(self, module, func, instruction.immediate.Index);
                try self.pushType(valtype);
            },
            .Local_Set => {
                const valtype = try Helpers.getLocalValtype(self, module, func, instruction.immediate.Index);
                try self.popType(valtype);
            },
            .Local_Tee => {
                const valtype = try Helpers.getLocalValtype(self, module, func, instruction.immediate.Index);
                try self.popType(valtype);
                try self.pushType(valtype);
            },
            .Global_Get => {
                const valtype = try Helpers.getGlobalValtype(module, instruction.immediate.Index, .None);
                try self.pushType(valtype);
            },
            .Global_Set => {
                const valtype = try Helpers.getGlobalValtype(module, instruction.immediate.Index, .Mutable);
                try self.popType(valtype);
            },
            .Table_Get => {
                const reftype = try getTableReftype(module, instruction.immediate.Index);
                try self.popType(.I32);
                try self.pushType(reftype);
            },
            .Table_Set => {
                const reftype = try getTableReftype(module, instruction.immediate.Index);
                try self.popType(reftype);
                try self.popType(.I32);
            },
            .I32_Load, .I32_Load8_S, .I32_Load8_U, .I32_Load16_S, .I32_Load16_U => {
                try Helpers.validateLoadOp(self, module, .I32);
            },
            .I64_Load, .I64_Load8_S, .I64_Load8_U, .I64_Load16_S, .I64_Load16_U, .I64_Load32_S, .I64_Load32_U => {
                try Helpers.validateLoadOp(self, module, .I64);
            },
            .F32_Load => {
                try Helpers.validateLoadOp(self, module, .F32);
            },
            .F64_Load => {
                try Helpers.validateLoadOp(self, module, .F64);
            },
            .I32_Store, .I32_Store8, .I32_Store16 => {
                try Helpers.validateStoreOp(self, module, .I32);
            },
            .I64_Store, .I64_Store8, .I64_Store16, .I64_Store32 => {
                try Helpers.validateStoreOp(self, module, .I64);
            },
            .F32_Store => {
                try Helpers.validateStoreOp(self, module, .F32);
            },
            .F64_Store => {
                try Helpers.validateStoreOp(self, module, .F64);
            },
            .Memory_Size => {
                try validateMemoryIndex(module);
                try self.pushType(.I32);
            },
            .Memory_Grow => {
                try validateMemoryIndex(module);
                try self.popType(.I32);
                try self.pushType(.I32);
            },
            .I32_Const => {
                try self.pushType(.I32);
            },
            .I64_Const => {
                try self.pushType(.I64);
            },
            .F32_Const => {
                try self.pushType(.F32);
            },
            .F64_Const => {
                try self.pushType(.F64);
            },
            .I32_Eqz, .I32_Clz, .I32_Ctz, .I32_Popcnt => {
                try Helpers.validateNumericUnaryOp(self, .I32, .I32);
            },
            .I32_Eq,
            .I32_NE,
            .I32_LT_S,
            .I32_LT_U,
            .I32_GT_S,
            .I32_GT_U,
            .I32_LE_S,
            .I32_LE_U,
            .I32_GE_S,
            .I32_GE_U,
            .I32_Add,
            .I32_Sub,
            .I32_Mul,
            .I32_Div_S,
            .I32_Div_U,
            .I32_Rem_S,
            .I32_Rem_U,
            .I32_And,
            .I32_Or,
            .I32_Xor,
            .I32_Shl,
            .I32_Shr_S,
            .I32_Shr_U,
            .I32_Rotl,
            .I32_Rotr,
            => {
                try Helpers.validateNumericBinaryOp(self, .I32, .I32);
            },
            .I64_Clz, .I64_Ctz, .I64_Popcnt => {
                try Helpers.validateNumericUnaryOp(self, .I64, .I64);
            },
            .I64_Eqz => {
                try Helpers.validateNumericUnaryOp(self, .I64, .I32);
            },
            .I64_Eq, .I64_NE, .I64_LT_S, .I64_LT_U, .I64_GT_S, .I64_GT_U, .I64_LE_S, .I64_LE_U, .I64_GE_S, .I64_GE_U => {
                try Helpers.validateNumericBinaryOp(self, .I64, .I32);
            },
            .I64_Add,
            .I64_Sub,
            .I64_Mul,
            .I64_Div_S,
            .I64_Div_U,
            .I64_Rem_S,
            .I64_Rem_U,
            .I64_And,
            .I64_Or,
            .I64_Xor,
            .I64_Shl,
            .I64_Shr_S,
            .I64_Shr_U,
            .I64_Rotl,
            .I64_Rotr,
            => {
                try Helpers.validateNumericBinaryOp(self, .I64, .I64);
            },
            .F32_EQ, .F32_NE, .F32_LT, .F32_GT, .F32_LE, .F32_GE => {
                try Helpers.validateNumericBinaryOp(self, .F32, .I32);
            },
            .F32_Add, .F32_Sub, .F32_Mul, .F32_Div, .F32_Min, .F32_Max, .F32_Copysign => {
                try Helpers.validateNumericBinaryOp(self, .F32, .F32);
            },
            .F32_Abs, .F32_Neg, .F32_Ceil, .F32_Floor, .F32_Trunc, .F32_Nearest, .F32_Sqrt => {
                try Helpers.validateNumericUnaryOp(self, .F32, .F32);
            },
            .F64_Abs, .F64_Neg, .F64_Ceil, .F64_Floor, .F64_Trunc, .F64_Nearest, .F64_Sqrt => {
                try Helpers.validateNumericUnaryOp(self, .F64, .F64);
            },
            .F64_EQ, .F64_NE, .F64_LT, .F64_GT, .F64_LE, .F64_GE => {
                try Helpers.validateNumericBinaryOp(self, .F64, .I32);
            },
            .F64_Add, .F64_Sub, .F64_Mul, .F64_Div, .F64_Min, .F64_Max, .F64_Copysign => {
                try Helpers.validateNumericBinaryOp(self, .F64, .F64);
            },
            .I32_Wrap_I64 => {
                try Helpers.validateNumericUnaryOp(self, .I64, .I32);
            },
            .I32_Trunc_F32_S, .I32_Trunc_F32_U => {
                try Helpers.validateNumericUnaryOp(self, .F32, .I32);
            },
            .I32_Trunc_F64_S, .I32_Trunc_F64_U => {
                try Helpers.validateNumericUnaryOp(self, .F64, .I32);
            },
            .I64_Extend_I32_S, .I64_Extend_I32_U => {
                try Helpers.validateNumericUnaryOp(self, .I32, .I64);
            },
            .I64_Trunc_F32_S, .I64_Trunc_F32_U => {
                try Helpers.validateNumericUnaryOp(self, .F32, .I64);
            },
            .I64_Trunc_F64_S, .I64_Trunc_F64_U => {
                try Helpers.validateNumericUnaryOp(self, .F64, .I64);
            },
            .F32_Convert_I32_S, .F32_Convert_I32_U => {
                try Helpers.validateNumericUnaryOp(self, .I32, .F32);
            },
            .F32_Convert_I64_S, .F32_Convert_I64_U => {
                try Helpers.validateNumericUnaryOp(self, .I64, .F32);
            },
            .F32_Demote_F64 => {
                try Helpers.validateNumericUnaryOp(self, .F64, .F32);
            },
            .F64_Convert_I32_S, .F64_Convert_I32_U => {
                try Helpers.validateNumericUnaryOp(self, .I32, .F64);
            },
            .F64_Convert_I64_S, .F64_Convert_I64_U => {
                try Helpers.validateNumericUnaryOp(self, .I64, .F64);
            },
            .F64_Promote_F32 => {
                try Helpers.validateNumericUnaryOp(self, .F32, .F64);
            },
            .I32_Reinterpret_F32 => {
                try Helpers.validateNumericUnaryOp(self, .F32, .I32);
            },
            .I64_Reinterpret_F64 => {
                try Helpers.validateNumericUnaryOp(self, .F64, .I64);
            },
            .F32_Reinterpret_I32 => {
                try Helpers.validateNumericUnaryOp(self, .I32, .F32);
            },
            .F64_Reinterpret_I64 => {
                try Helpers.validateNumericUnaryOp(self, .I64, .F64);
            },
            .I32_Extend8_S, .I32_Extend16_S => {
                try Helpers.validateNumericUnaryOp(self, .I32, .I32);
            },
            .I64_Extend8_S, .I64_Extend16_S, .I64_Extend32_S => {
                try Helpers.validateNumericUnaryOp(self, .I64, .I64);
            },
            .Ref_Null => {
                try self.pushType(instruction.immediate.ValType);
            },
            .Ref_Is_Null => {
                var valtype_or_null: ?ValType = try self.popAnyType();
                if (valtype_or_null) |valtype| {
                    if (valtype.isRefType() == false) {
                        return error.ValidationTypeMismatch;
                    }
                }
                try self.pushType(.I32);
            },
            .Ref_Func => {
                const func_index: u32 = instruction.immediate.Index;
                try validateFunctionIndex(func_index, module);

                const is_referencing_current_function: bool = module.imports.functions.items.len <= func_index and
                    &module.functions.items[func_index - module.imports.functions.items.len] == func;

                // references to the current function must be declared in element segments
                if (is_referencing_current_function) {
                    var needs_declaration: bool = true;
                    skip_outer: for (module.elements.items) |elem_def| {
                        if (elem_def.mode == .Declarative and elem_def.reftype == .FuncRef) {
                            if (elem_def.elems_value.items.len > 0) {
                                for (elem_def.elems_value.items) |val| {
                                    if (val.FuncRef.index == func_index) {
                                        needs_declaration = false;
                                        break :skip_outer;
                                    }
                                }
                            } else {
                                for (elem_def.elems_expr.items) |expr| {
                                    if (std.meta.activeTag(expr) == .Value) {
                                        if (expr.Value.val.FuncRef.index == func_index) {
                                            needs_declaration = false;
                                            break :skip_outer;
                                        }
                                    }
                                }
                            }
                        }
                    }

                    if (needs_declaration) {
                        return error.ValidationFuncRefUndeclared;
                    }
                }

                try self.pushType(.FuncRef);
            },
            .I32_Trunc_Sat_F32_S, .I32_Trunc_Sat_F32_U => {
                try Helpers.validateNumericUnaryOp(self, .F32, .I32);
            },
            .I32_Trunc_Sat_F64_S, .I32_Trunc_Sat_F64_U => {
                try Helpers.validateNumericUnaryOp(self, .F64, .I32);
            },
            .I64_Trunc_Sat_F32_S, .I64_Trunc_Sat_F32_U => {
                try Helpers.validateNumericUnaryOp(self, .F32, .I64);
            },
            .I64_Trunc_Sat_F64_S, .I64_Trunc_Sat_F64_U => {
                try Helpers.validateNumericUnaryOp(self, .F64, .I64);
            },
            .Memory_Init => {
                try validateMemoryIndex(module);
                try validateDataIndex(instruction.immediate.Index, module);
                try self.popType(.I32);
                try self.popType(.I32);
                try self.popType(.I32);
            },
            .Data_Drop => {
                if (module.data_count != null) {
                    try validateDataIndex(instruction.immediate.Index, module);
                } else {
                    return error.MalformedMissingDataCountSection;
                }
            },
            .Memory_Copy, .Memory_Fill => {
                try validateMemoryIndex(module);
                try self.popType(.I32);
                try self.popType(.I32);
                try self.popType(.I32);
            },
            .Table_Init => {
                const pair: TablePairImmediates = instruction.immediate.TablePair;
                const elem_index = pair.index_x;
                const table_index = pair.index_y;
                try validateTableIndex(table_index, module);
                try validateElementIndex(elem_index, module);

                const elem_reftype: ValType = module.elements.items[elem_index].reftype;
                const table_reftype: ValType = module.tables.items[table_index].reftype;

                if (elem_reftype != table_reftype) {
                    return error.ValidationTypeMismatch;
                }

                try self.popType(.I32);
                try self.popType(.I32);
                try self.popType(.I32);
            },
            .Elem_Drop => {
                try validateElementIndex(instruction.immediate.Index, module);
            },
            .Table_Copy => {
                const pair: TablePairImmediates = instruction.immediate.TablePair;
                const dest_table_index = pair.index_x;
                const src_table_index = pair.index_y;
                try validateTableIndex(dest_table_index, module);
                try validateTableIndex(src_table_index, module);

                const dest_reftype: ValType = module.tables.items[dest_table_index].reftype;
                const src_reftype: ValType = module.tables.items[src_table_index].reftype;

                if (dest_reftype != src_reftype) {
                    return error.ValidationTypeMismatch;
                }

                try self.popType(.I32);
                try self.popType(.I32);
                try self.popType(.I32);
            },
            .Table_Grow => {
                try validateTableIndex(instruction.immediate.Index, module);

                try self.popType(.I32);
                if (try self.popAnyType()) |init_type| {
                    var table_reftype: ValType = try getTableReftype(module, instruction.immediate.Index);
                    if (init_type != table_reftype) {
                        return error.ValidationTypeMismatch;
                    }
                }

                try self.pushType(.I32);
            },
            .Table_Size => {
                try validateTableIndex(instruction.immediate.Index, module);
                try self.pushType(.I32);
            },
            .Table_Fill => {
                try validateTableIndex(instruction.immediate.Index, module);
                try self.popType(.I32);
                if (try self.popAnyType()) |valtype| {
                    var table_reftype: ValType = try getTableReftype(module, instruction.immediate.Index);
                    if (valtype != table_reftype) {
                        return error.ValidationTypeMismatch;
                    }
                }
                try self.popType(.I32);
            },
            .V128_Load,
            .V128_Load8x8_S,
            .V128_Load8x8_U,
            .V128_Load16x4_S,
            .V128_Load16x4_U,
            .V128_Load32x2_S,
            .V128_Load32x2_U,
            .V128_Load8_Splat,
            .V128_Load16_Splat,
            .V128_Load32_Splat,
            .V128_Load64_Splat,
            => {
                try Helpers.validateLoadOp(self, module, .V128);
            },
            .I8x16_Splat, .I16x8_Splat, .I32x4_Splat => {
                try Helpers.validateNumericUnaryOp(self, .I32, .V128);
            },
            .I64x2_Splat => {
                try Helpers.validateNumericUnaryOp(self, .I64, .V128);
            },
            .F32x4_Splat => {
                try Helpers.validateNumericUnaryOp(self, .F32, .V128);
            },
            .F64x2_Splat => {
                try Helpers.validateNumericUnaryOp(self, .F64, .V128);
            },
            .I8x16_Extract_Lane_S, .I8x16_Extract_Lane_U => {
                try Helpers.validateVecExtractLane(i8x16, self, instruction);
            },
            .I8x16_Replace_Lane => {
                try Helpers.validateVecReplaceLane(i8x16, self, instruction);
            },
            .I16x8_Extract_Lane_S, .I16x8_Extract_Lane_U => {
                try Helpers.validateVecExtractLane(i16x8, self, instruction);
            },
            .I16x8_Replace_Lane => {
                try Helpers.validateVecReplaceLane(i16x8, self, instruction);
            },
            .I32x4_Extract_Lane => {
                try Helpers.validateVecExtractLane(i32x4, self, instruction);
            },
            .I32x4_Replace_Lane => {
                try Helpers.validateVecReplaceLane(i32x4, self, instruction);
            },
            .I64x2_Extract_Lane => {
                try Helpers.validateVecExtractLane(i64x2, self, instruction);
            },
            .I64x2_Replace_Lane => {
                try Helpers.validateVecReplaceLane(i64x2, self, instruction);
            },
            .F32x4_Extract_Lane => {
                try Helpers.validateVecExtractLane(f32x4, self, instruction);
            },
            .F32x4_Replace_Lane => {
                try Helpers.validateVecReplaceLane(f32x4, self, instruction);
            },
            .F64x2_Extract_Lane => {
                try Helpers.validateVecExtractLane(f64x2, self, instruction);
            },
            .F64x2_Replace_Lane => {
                try Helpers.validateVecReplaceLane(f64x2, self, instruction);
            },
            .V128_Store => {
                try Helpers.validateStoreOp(self, module, .V128);
            },
            .V128_Const => {
                try self.pushType(.V128);
            },
            .I8x16_Shuffle => {
                for (instruction.immediate.VecShuffle16) |v| {
                    if (v >= 32) {
                        return ValidationError.ValidationInvalidLaneIndex;
                    }
                }
                try Helpers.validateNumericBinaryOp(self, .V128, .V128);
            },
            .I8x16_Swizzle => {
                try Helpers.validateNumericBinaryOp(self, .V128, .V128);
            },
            .V128_Not,
            .F32x4_Demote_F64x2_Zero,
            .F64x2_Promote_Low_F32x4,
            .I8x16_Abs,
            .I8x16_Neg,
            .I8x16_Popcnt,
            .F32x4_Ceil,
            .F32x4_Floor,
            .F32x4_Trunc,
            .F32x4_Nearest,
            .F64x2_Ceil,
            .F64x2_Floor,
            .F64x2_Trunc,
            .F64x2_Nearest,
            .I16x8_Extadd_Pairwise_I8x16_S,
            .I16x8_Extadd_Pairwise_I8x16_U,
            .I32x4_Extadd_Pairwise_I16x8_S,
            .I32x4_Extadd_Pairwise_I16x8_U,
            .I16x8_Abs,
            .I16x8_Neg,
            .I16x8_Extend_Low_I8x16_S,
            .I16x8_Extend_High_I8x16_S,
            .I16x8_Extend_Low_I8x16_U,
            .I16x8_Extend_High_I8x16_U,
            .I32x4_Abs,
            .I32x4_Neg,
            .I32x4_Extend_Low_I16x8_S,
            .I32x4_Extend_High_I16x8_S,
            .I32x4_Extend_Low_I16x8_U,
            .I32x4_Extend_High_I16x8_U,
            .I64x2_Abs,
            .I64x2_Neg,
            .I64x2_Extend_Low_I32x4_S,
            .I64x2_Extend_High_I32x4_S,
            .I64x2_Extend_Low_I32x4_U,
            .I64x2_Extend_High_I32x4_U,
            .F32x4_Abs,
            .F32x4_Neg,
            .F32x4_Sqrt,
            .F64x2_Abs,
            .F64x2_Neg,
            .F64x2_Sqrt,
            .F32x4_Trunc_Sat_F32x4_S,
            .F32x4_Trunc_Sat_F32x4_U,
            .F32x4_Convert_I32x4_S,
            .F32x4_Convert_I32x4_U,
            .I32x4_Trunc_Sat_F64x2_S_Zero,
            .I32x4_Trunc_Sat_F64x2_U_Zero,
            .F64x2_Convert_Low_I32x4_S,
            .F64x2_Convert_Low_I32x4_U,
            => {
                try Helpers.validateNumericUnaryOp(self, .V128, .V128);
            },
            .I8x16_EQ,
            .I8x16_NE,
            .I8x16_LT_S,
            .I8x16_LT_U,
            .I8x16_GT_S,
            .I8x16_GT_U,
            .I8x16_LE_S,
            .I8x16_LE_U,
            .I8x16_GE_S,
            .I8x16_GE_U,
            .I16x8_EQ,
            .I16x8_NE,
            .I16x8_LT_S,
            .I16x8_LT_U,
            .I16x8_GT_S,
            .I16x8_GT_U,
            .I16x8_LE_S,
            .I16x8_LE_U,
            .I16x8_GE_S,
            .I16x8_GE_U,
            .I32x4_EQ,
            .I32x4_NE,
            .I32x4_LT_S,
            .I32x4_LT_U,
            .I32x4_GT_S,
            .I32x4_GT_U,
            .I32x4_LE_S,
            .I32x4_LE_U,
            .I32x4_GE_S,
            .I32x4_GE_U,
            .F32x4_EQ,
            .F32x4_NE,
            .F32x4_LT,
            .F32x4_GT,
            .F32x4_LE,
            .F32x4_GE,
            .F64x2_EQ,
            .F64x2_NE,
            .F64x2_LT,
            .F64x2_GT,
            .F64x2_LE,
            .F64x2_GE,
            .I64x2_EQ,
            .I64x2_NE,
            .I64x2_LT_S,
            .I64x2_GT_S,
            .I64x2_LE_S,
            .I64x2_GE_S,
            .I64x2_Extmul_Low_I32x4_S,
            .I64x2_Extmul_High_I32x4_S,
            .I64x2_Extmul_Low_I32x4_U,
            .I64x2_Extmul_High_I32x4_U,
            => {
                try Helpers.validateNumericBinaryOp(self, .V128, .V128);
            },
            .V128_AnyTrue,
            .I8x16_AllTrue,
            .I8x16_Bitmask,
            .I16x8_AllTrue,
            .I16x8_Bitmask,
            .I32x4_AllTrue,
            .I32x4_Bitmask,
            .I64x2_AllTrue,
            .I64x2_Bitmask,
            => {
                try Helpers.validateNumericUnaryOp(self, .V128, .I32);
            },
            .V128_Load8_Lane => {
                try Helpers.validateLoadLaneOp(self, module, instruction, i8x16);
            },
            .V128_Load16_Lane => {
                try Helpers.validateLoadLaneOp(self, module, instruction, i16x8);
            },
            .V128_Load32_Lane => {
                try Helpers.validateLoadLaneOp(self, module, instruction, i32x4);
            },
            .V128_Load64_Lane => {
                try Helpers.validateLoadLaneOp(self, module, instruction, i64x2);
            },
            .V128_Store8_Lane => {
                try Helpers.validateStoreLaneOp(self, module, instruction, i8x16);
            },
            .V128_Store16_Lane => {
                try Helpers.validateStoreLaneOp(self, module, instruction, i16x8);
            },
            .V128_Store32_Lane => {
                try Helpers.validateStoreLaneOp(self, module, instruction, i32x4);
            },
            .V128_Store64_Lane => {
                try Helpers.validateStoreLaneOp(self, module, instruction, i64x2);
            },
            .V128_Load32_Zero => {
                try Helpers.validateLoadOp(self, module, .V128);
            },
            .V128_Load64_Zero => {
                try Helpers.validateLoadOp(self, module, .V128);
            },
            .V128_And,
            .V128_AndNot,
            .V128_Or,
            .V128_Xor,
            .I8x16_Narrow_I16x8_S,
            .I8x16_Narrow_I16x8_U,
            .I8x16_Add,
            .I8x16_Add_Sat_S,
            .I8x16_Add_Sat_U,
            .I8x16_Sub,
            .I8x16_Sub_Sat_S,
            .I8x16_Sub_Sat_U,
            .I8x16_Min_S,
            .I8x16_Min_U,
            .I8x16_Max_S,
            .I8x16_Max_U,
            .I8x16_Avgr_U,
            .I16x8_Narrow_I32x4_S,
            .I16x8_Narrow_I32x4_U,
            .I16x8_Add,
            .I16x8_Add_Sat_S,
            .I16x8_Add_Sat_U,
            .I16x8_Sub,
            .I16x8_Sub_Sat_S,
            .I16x8_Sub_Sat_U,
            .I16x8_Mul,
            .I16x8_Min_S,
            .I16x8_Min_U,
            .I16x8_Max_S,
            .I16x8_Max_U,
            .I16x8_Avgr_U,
            .I16x8_Q15mulr_Sat_S,
            .I16x8_Extmul_Low_I8x16_S,
            .I16x8_Extmul_High_I8x16_S,
            .I16x8_Extmul_Low_I8x16_U,
            .I16x8_Extmul_High_I8x16_U,
            .I32x4_Add,
            .I32x4_Sub,
            .I32x4_Mul,
            .I32x4_Min_S,
            .I32x4_Min_U,
            .I32x4_Max_S,
            .I32x4_Max_U,
            .I32x4_Dot_I16x8_S,
            .I32x4_Extmul_Low_I16x8_S,
            .I32x4_Extmul_High_I16x8_S,
            .I32x4_Extmul_Low_I16x8_U,
            .I32x4_Extmul_High_I16x8_U,
            .I64x2_Add,
            .I64x2_Sub,
            .I64x2_Mul,
            .F32x4_Add,
            .F32x4_Sub,
            .F32x4_Mul,
            .F32x4_Div,
            .F32x4_Min,
            .F32x4_Max,
            .F32x4_PMin,
            .F32x4_PMax,
            .F64x2_Add,
            .F64x2_Sub,
            .F64x2_Mul,
            .F64x2_Div,
            .F64x2_Min,
            .F64x2_Max,
            .F64x2_PMin,
            .F64x2_PMax,
            => {
                try Helpers.validateNumericBinaryOp(self, .V128, .V128);
            },
            .I8x16_Shl,
            .I8x16_Shr_S,
            .I8x16_Shr_U,
            .I16x8_Shl,
            .I16x8_Shr_S,
            .I16x8_Shr_U,
            .I32x4_Shl,
            .I32x4_Shr_S,
            .I32x4_Shr_U,
            .I64x2_Shl,
            .I64x2_Shr_S,
            .I64x2_Shr_U,
            => {
                try self.popType(.I32);
                try self.popType(.V128);
                try self.pushType(.V128);
            },
            .V128_Bitselect => {
                try self.popType(.V128);
                try self.popType(.V128);
                try self.popType(.V128);
                try self.pushType(.V128);
            },
        }
    }

    fn endValidateCode(self: *ModuleValidator) !void {
        try self.type_stack.resize(0);
        try self.control_stack.resize(0);
        try self.control_types.resize(0);
    }

    fn pushType(self: *ModuleValidator, valtype: ?ValType) !void {
        try self.type_stack.append(valtype);
    }

    fn popAnyType(self: *ModuleValidator) !?ValType {
        const top_frame: *const ControlFrame = &self.control_stack.items[self.control_stack.items.len - 1];
        const types: []?ValType = self.type_stack.items;

        if (top_frame.is_unreachable and types.len == top_frame.types_stack_height) {
            return null;
        }

        if (self.type_stack.items.len <= top_frame.types_stack_height) {
            return error.ValidationTypeMismatch;
        }
        return self.type_stack.pop();
    }

    fn popType(self: *ModuleValidator, expected_or_null: ?ValType) !void {
        const valtype_or_null = try self.popAnyType();
        if (valtype_or_null != expected_or_null and valtype_or_null != null and expected_or_null != null) {
            return error.ValidationTypeMismatch;
        }
    }

    fn pushControl(self: *ModuleValidator, opcode: Opcode, start_types: []const ValType, end_types: []const ValType) !void {
        const control_types_start_index: usize = self.control_types.items.len;
        try self.control_types.appendSlice(start_types);
        var control_start_types: []const ValType = self.control_types.items[control_types_start_index..self.control_types.items.len];

        const control_types_end_index: usize = self.control_types.items.len;
        try self.control_types.appendSlice(end_types);
        var control_end_types: []const ValType = self.control_types.items[control_types_end_index..self.control_types.items.len];

        try self.control_stack.append(ControlFrame{
            .opcode = opcode,
            .start_types = control_start_types,
            .end_types = control_end_types,
            .types_stack_height = self.type_stack.items.len,
            .is_function = true,
            .is_unreachable = false,
        });

        if (opcode != .Call) {
            for (start_types) |valtype| {
                try self.pushType(valtype);
            }
        }
    }

    fn popControl(self: *ModuleValidator) !ControlFrame {
        const frame: *const ControlFrame = &self.control_stack.items[self.control_stack.items.len - 1];

        var i = frame.end_types.len;
        while (i > 0) : (i -= 1) {
            if (frame.is_unreachable and self.type_stack.items.len == frame.types_stack_height) {
                break;
            }
            try self.popType(frame.end_types[i - 1]);
        }

        if (self.type_stack.items.len != frame.types_stack_height) {
            return error.ValidationTypeStackHeightMismatch;
        }

        _ = self.control_stack.pop();

        return frame.*;
    }

    fn freeControlTypes(self: *ModuleValidator, frame: *const ControlFrame) !void {
        var num_used_types: usize = frame.start_types.len + frame.end_types.len;
        try self.control_types.resize(self.control_types.items.len - num_used_types);
    }
};

pub const ModuleDefinitionOpts = struct {
    debug_name: []const u8 = "",
};

pub const ModuleDefinition = struct {
    const Code = struct {
        instructions: std.ArrayList(Instruction),

        wasm_address_to_instruction_index: std.AutoHashMap(u32, u32),

        // Instruction.immediate indexes these arrays depending on the opcode
        branch_table: std.ArrayList(BranchTableImmediates),
    };

    const Imports = struct {
        functions: std.ArrayList(FunctionImportDefinition),
        tables: std.ArrayList(TableImportDefinition),
        memories: std.ArrayList(MemoryImportDefinition),
        globals: std.ArrayList(GlobalImportDefinition),
    };

    const Exports = struct {
        functions: std.ArrayList(ExportDefinition),
        tables: std.ArrayList(ExportDefinition),
        memories: std.ArrayList(ExportDefinition),
        globals: std.ArrayList(ExportDefinition),
    };

    allocator: std.mem.Allocator,

    code: Code,

    types: std.ArrayList(FunctionTypeDefinition),
    imports: Imports,
    functions: std.ArrayList(FunctionDefinition),
    globals: std.ArrayList(GlobalDefinition),
    tables: std.ArrayList(TableDefinition),
    memories: std.ArrayList(MemoryDefinition),
    elements: std.ArrayList(ElementDefinition),
    exports: Exports,
    datas: std.ArrayList(DataDefinition),
    custom_sections: std.ArrayList(CustomSection),

    name_section: NameCustomSection,

    debug_name: []const u8,
    start_func_index: ?u32 = null,
    data_count: ?u32 = null,

    is_decoded: bool = false,

    pub fn create(allocator: std.mem.Allocator, opts: ModuleDefinitionOpts) AllocError!*ModuleDefinition {
        var def = try allocator.create(ModuleDefinition);
        def.* = ModuleDefinition{
            .allocator = allocator,
            .code = Code{
                .instructions = std.ArrayList(Instruction).init(allocator),
                .wasm_address_to_instruction_index = std.AutoHashMap(u32, u32).init(allocator),
                .branch_table = std.ArrayList(BranchTableImmediates).init(allocator),
            },
            .types = std.ArrayList(FunctionTypeDefinition).init(allocator),
            .imports = Imports{
                .functions = std.ArrayList(FunctionImportDefinition).init(allocator),
                .tables = std.ArrayList(TableImportDefinition).init(allocator),
                .memories = std.ArrayList(MemoryImportDefinition).init(allocator),
                .globals = std.ArrayList(GlobalImportDefinition).init(allocator),
            },
            .functions = std.ArrayList(FunctionDefinition).init(allocator),
            .globals = std.ArrayList(GlobalDefinition).init(allocator),
            .tables = std.ArrayList(TableDefinition).init(allocator),
            .memories = std.ArrayList(MemoryDefinition).init(allocator),
            .elements = std.ArrayList(ElementDefinition).init(allocator),
            .exports = Exports{
                .functions = std.ArrayList(ExportDefinition).init(allocator),
                .tables = std.ArrayList(ExportDefinition).init(allocator),
                .memories = std.ArrayList(ExportDefinition).init(allocator),
                .globals = std.ArrayList(ExportDefinition).init(allocator),
            },
            .datas = std.ArrayList(DataDefinition).init(allocator),
            .custom_sections = std.ArrayList(CustomSection).init(allocator),
            .name_section = NameCustomSection.init(allocator),
            .debug_name = try allocator.dupe(u8, opts.debug_name),
        };
        return def;
    }

    pub fn decode(self: *ModuleDefinition, wasm: []const u8) anyerror!void {
        std.debug.assert(self.is_decoded == false);

        self.decode_internal(wasm) catch |e| {
            var wrapped_error: anyerror = switch (e) {
                error.EndOfStream => error.MalformedUnexpectedEnd,
                else => e,
            };
            return wrapped_error;
        };
    }

    fn decode_internal(self: *ModuleDefinition, wasm: []const u8) anyerror!void {
        const DecodeHelpers = struct {
            fn readRefValue(valtype: ValType, reader: anytype) !Val {
                switch (valtype) {
                    .FuncRef => {
                        const func_index = try common.decodeLEB128(u32, reader);
                        return Val.funcrefFromIndex(func_index);
                    },
                    .ExternRef => {
                        unreachable; // TODO
                    },
                    else => unreachable,
                }
            }

            // TODO move these names into a string pool
            fn readName(reader: anytype, _allocator: std.mem.Allocator) ![]const u8 {
                const name_length = try common.decodeLEB128(u32, reader);

                var name: []u8 = try _allocator.alloc(u8, name_length);
                errdefer _allocator.free(name);
                var read_length = try reader.read(name);
                if (read_length != name_length) {
                    return error.MalformedUnexpectedEnd;
                }

                if (std.unicode.utf8ValidateSlice(name) == false) {
                    return error.MalformedUTF8Encoding;
                }

                return name;
            }
        };

        var allocator = self.allocator;
        var validator = ModuleValidator.init(allocator);
        defer validator.deinit();

        var stream = std.io.fixedBufferStream(wasm);
        var reader = stream.reader();

        // wasm header
        {
            const magic = try reader.readIntBig(u32);
            if (magic != 0x0061736D) {
                return error.MalformedMagicSignature;
            }
            const version = try reader.readIntLittle(u32);
            if (version != 1) {
                return error.MalformedUnsupportedWasmVersion;
            }
        }

        var num_functions_parsed: u32 = 0;

        while (stream.pos < stream.buffer.len) {
            const section_id: Section = std.meta.intToEnum(Section, try reader.readByte()) catch {
                return error.MalformedSectionId;
            };
            const section_size_bytes: usize = try common.decodeLEB128(u32, reader);
            const section_start_pos = stream.pos;

            switch (section_id) {
                .Custom => {
                    if (section_size_bytes == 0) {
                        return error.MalformedUnexpectedEnd;
                    }

                    var name = try DecodeHelpers.readName(reader, allocator);
                    errdefer allocator.free(name);

                    var section = CustomSection{
                        .name = name,
                        .data = std.ArrayList(u8).init(allocator),
                    };

                    const name_length: usize = stream.pos - section_start_pos;
                    const data_length: usize = section_size_bytes - name_length;
                    try section.data.resize(data_length);
                    const data_length_read = try reader.read(section.data.items);
                    if (data_length != data_length_read) {
                        return error.MalformedUnexpectedEnd;
                    }

                    try self.custom_sections.append(section);

                    if (std.mem.eql(u8, section.name, "name")) {
                        try self.name_section.decode(self, section.data.items);
                    }
                },
                .FunctionType => {
                    const num_types = try common.decodeLEB128(u32, reader);

                    try self.types.ensureTotalCapacity(num_types);

                    var types_index: u32 = 0;
                    while (types_index < num_types) : (types_index += 1) {
                        const sentinel = try reader.readByte();
                        if (sentinel != k_function_type_sentinel_byte) {
                            return error.MalformedTypeSentinel;
                        }

                        const num_params = try common.decodeLEB128(u32, reader);

                        var func = FunctionTypeDefinition{ .num_params = num_params, .types = std.ArrayList(ValType).init(allocator) };
                        errdefer func.types.deinit();

                        var params_left = num_params;
                        while (params_left > 0) {
                            params_left -= 1;

                            var param_type = try ValType.decode(reader);
                            try func.types.append(param_type);
                        }

                        const num_returns = try common.decodeLEB128(u32, reader);
                        var returns_left = num_returns;
                        while (returns_left > 0) {
                            returns_left -= 1;

                            var return_type = try ValType.decode(reader);
                            try func.types.append(return_type);
                        }

                        try self.types.append(func);
                    }
                },
                .Import => {
                    const num_imports = try common.decodeLEB128(u32, reader);

                    var import_index: u32 = 0;
                    while (import_index < num_imports) : (import_index += 1) {
                        var module_name: []const u8 = try DecodeHelpers.readName(reader, allocator);
                        errdefer allocator.free(module_name);

                        var import_name: []const u8 = try DecodeHelpers.readName(reader, allocator);
                        errdefer allocator.free(import_name);

                        const names = ImportNames{
                            .module_name = module_name,
                            .import_name = import_name,
                        };

                        const desc = try reader.readByte();
                        switch (desc) {
                            0x00 => {
                                const type_index = try common.decodeLEB128(u32, reader);
                                try ModuleValidator.validateTypeIndex(type_index, self);
                                try self.imports.functions.append(FunctionImportDefinition{
                                    .names = names,
                                    .type_index = type_index,
                                });
                            },
                            0x01 => {
                                const valtype = try ValType.decode(reader);
                                if (valtype.isRefType() == false) {
                                    return error.MalformedInvalidImport;
                                }
                                const limits = try Limits.decode(reader);
                                try self.imports.tables.append(TableImportDefinition{
                                    .names = names,
                                    .reftype = valtype,
                                    .limits = limits,
                                });
                            },
                            0x02 => {
                                const limits = try Limits.decode(reader);
                                try self.imports.memories.append(MemoryImportDefinition{
                                    .names = names,
                                    .limits = limits,
                                });
                            },
                            0x03 => {
                                const valtype = try ValType.decode(reader);
                                const mut = try GlobalMut.decode(reader);

                                try self.imports.globals.append(GlobalImportDefinition{
                                    .names = names,
                                    .valtype = valtype,
                                    .mut = mut,
                                });
                            },
                            else => return error.MalformedInvalidImport,
                        }
                    }
                },
                .Function => {
                    const num_funcs = try common.decodeLEB128(u32, reader);

                    try self.functions.ensureTotalCapacity(num_funcs);

                    var func_index: u32 = 0;
                    while (func_index < num_funcs) : (func_index += 1) {
                        var func = FunctionDefinition{
                            .type_index = try common.decodeLEB128(u32, reader),
                            .locals = std.ArrayList(ValType).init(allocator),

                            // we'll fix these up later when we find them in the Code section
                            .instructions_begin = 0,
                            .instructions_end = 0,
                            .continuation = 0,
                        };

                        self.functions.addOneAssumeCapacity().* = func;
                    }
                },
                .Table => {
                    const num_tables = try common.decodeLEB128(u32, reader);

                    try self.tables.ensureTotalCapacity(num_tables);

                    var table_index: u32 = 0;
                    while (table_index < num_tables) : (table_index += 1) {
                        const valtype = try ValType.decode(reader);
                        if (valtype.isRefType() == false) {
                            return error.InvalidTableType;
                        }

                        const limits = try Limits.decode(reader);

                        try self.tables.append(TableDefinition{
                            .reftype = valtype,
                            .limits = limits,
                        });
                    }
                },
                .Memory => {
                    const num_memories = try common.decodeLEB128(u32, reader);

                    if (num_memories > 1) {
                        return error.ValidationMultipleMemories;
                    }

                    try self.memories.ensureTotalCapacity(num_memories);

                    var memory_index: u32 = 0;
                    while (memory_index < num_memories) : (memory_index += 1) {
                        var limits = try Limits.decode(reader);

                        if (limits.min > MemoryDefinition.k_max_pages) {
                            return error.ValidationMemoryMaxPagesExceeded;
                        }

                        if (limits.max) |max| {
                            if (max < limits.min) {
                                return error.ValidationMemoryInvalidMaxLimit;
                            }
                            if (max > MemoryDefinition.k_max_pages) {
                                return error.ValidationMemoryMaxPagesExceeded;
                            }
                        }

                        var def = MemoryDefinition{
                            .limits = limits,
                        };
                        try self.memories.append(def);
                    }
                },
                .Global => {
                    const num_globals = try common.decodeLEB128(u32, reader);

                    try self.globals.ensureTotalCapacity(num_globals);

                    var global_index: u32 = 0;
                    while (global_index < num_globals) : (global_index += 1) {
                        var valtype = try ValType.decode(reader);
                        var mut = try GlobalMut.decode(reader);

                        const expr = try ConstantExpression.decode(reader, self, .Immutable, valtype);

                        if (std.meta.activeTag(expr) == .Value) {
                            if (expr.Value.type == .FuncRef) {
                                if (expr.Value.val.isNull() == false) {
                                    const index: u32 = expr.Value.val.FuncRef.index;
                                    try ModuleValidator.validateFunctionIndex(index, self);
                                }
                            }
                        }

                        try self.globals.append(GlobalDefinition{
                            .valtype = valtype,
                            .expr = expr,
                            .mut = mut,
                        });
                    }
                },
                .Export => {
                    const num_exports = try common.decodeLEB128(u32, reader);

                    var export_names = std.StringHashMap(void).init(allocator);
                    defer export_names.deinit();

                    var export_index: u32 = 0;
                    while (export_index < num_exports) : (export_index += 1) {
                        var name: []const u8 = try DecodeHelpers.readName(reader, allocator);
                        errdefer allocator.free(name);

                        {
                            var getOrPutResult = try export_names.getOrPut(name);
                            if (getOrPutResult.found_existing == true) {
                                return error.ValidationDuplicateExportName;
                            }
                        }

                        const exportType = @as(ExportType, @enumFromInt(try reader.readByte()));
                        const item_index = try common.decodeLEB128(u32, reader);
                        const def = ExportDefinition{ .name = name, .index = item_index };

                        switch (exportType) {
                            .Function => {
                                try ModuleValidator.validateFunctionIndex(item_index, self);
                                try self.exports.functions.append(def);
                            },
                            .Table => {
                                try ModuleValidator.validateTableIndex(item_index, self);
                                try self.exports.tables.append(def);
                            },
                            .Memory => {
                                if (self.imports.memories.items.len + self.memories.items.len <= item_index) {
                                    return error.ValidationUnknownMemory;
                                }
                                try self.exports.memories.append(def);
                            },
                            .Global => {
                                try ModuleValidator.validateGlobalIndex(item_index, self);
                                try self.exports.globals.append(def);
                            },
                        }
                    }
                },
                .Start => {
                    if (self.start_func_index != null) {
                        return error.MalformedMultipleStartSections;
                    }

                    self.start_func_index = try common.decodeLEB128(u32, reader);

                    if (self.imports.functions.items.len + self.functions.items.len <= self.start_func_index.?) {
                        return error.ValidationUnknownFunction;
                    }

                    var func_type_index: usize = undefined;
                    if (self.start_func_index.? < self.imports.functions.items.len) {
                        func_type_index = self.imports.functions.items[self.start_func_index.?].type_index;
                    } else {
                        var local_func_index = self.start_func_index.? - self.imports.functions.items.len;
                        func_type_index = self.functions.items[local_func_index].type_index;
                    }

                    const func_type: *const FunctionTypeDefinition = &self.types.items[func_type_index];
                    if (func_type.types.items.len > 0) {
                        return error.ValidationStartFunctionType;
                    }
                },
                .Element => {
                    const ElementHelpers = struct {
                        fn readOffsetExpr(_reader: anytype, _module: *const ModuleDefinition) !ConstantExpression {
                            var expr = try ConstantExpression.decode(_reader, _module, .Immutable, .I32);
                            return expr;
                        }

                        fn readElemsVal(elems: *std.ArrayList(Val), valtype: ValType, _reader: anytype, _module: *const ModuleDefinition) !void {
                            const num_elems = try common.decodeLEB128(u32, _reader);
                            try elems.ensureTotalCapacity(num_elems);

                            var elem_index: u32 = 0;
                            while (elem_index < num_elems) : (elem_index += 1) {
                                const ref: Val = try DecodeHelpers.readRefValue(valtype, _reader);
                                if (valtype == .FuncRef) {
                                    try ModuleValidator.validateFunctionIndex(ref.FuncRef.index, _module);
                                }
                                try elems.append(ref);
                            }
                        }

                        fn readElemsExpr(elems: *std.ArrayList(ConstantExpression), _reader: anytype, _module: *const ModuleDefinition, expected_reftype: ValType) !void {
                            const num_elems = try common.decodeLEB128(u32, _reader);
                            try elems.ensureTotalCapacity(num_elems);

                            var elem_index: u32 = 0;
                            while (elem_index < num_elems) : (elem_index += 1) {
                                var expr = try ConstantExpression.decode(_reader, _module, .Any, expected_reftype);
                                try elems.append(expr);
                            }
                        }

                        fn readNullElemkind(_reader: anytype) !void {
                            var null_elemkind = try _reader.readByte();
                            if (null_elemkind != 0x00) {
                                return error.MalformedBytecode;
                            }
                        }
                    };

                    const num_segments = try common.decodeLEB128(u32, reader);

                    try self.elements.ensureTotalCapacity(num_segments);

                    var segment_index: u32 = 0;
                    while (segment_index < num_segments) : (segment_index += 1) {
                        var flags = try common.decodeLEB128(u32, reader);

                        var def = ElementDefinition{
                            .mode = ElementMode.Active,
                            .reftype = ValType.FuncRef,
                            .table_index = 0,
                            .offset = null,
                            .elems_value = std.ArrayList(Val).init(allocator),
                            .elems_expr = std.ArrayList(ConstantExpression).init(allocator),
                        };
                        errdefer def.elems_value.deinit();
                        errdefer def.elems_expr.deinit();

                        switch (flags) {
                            0x00 => {
                                def.offset = try ElementHelpers.readOffsetExpr(reader, self);
                                try ElementHelpers.readElemsVal(&def.elems_value, def.reftype, reader, self);
                            },
                            0x01 => {
                                def.mode = .Passive;
                                try ElementHelpers.readNullElemkind(reader);
                                try ElementHelpers.readElemsVal(&def.elems_value, def.reftype, reader, self);
                            },
                            0x02 => {
                                def.table_index = try common.decodeLEB128(u32, reader);
                                def.offset = try ElementHelpers.readOffsetExpr(reader, self);
                                try ElementHelpers.readNullElemkind(reader);
                                try ElementHelpers.readElemsVal(&def.elems_value, def.reftype, reader, self);
                            },
                            0x03 => {
                                def.mode = .Declarative;
                                try ElementHelpers.readNullElemkind(reader);
                                try ElementHelpers.readElemsVal(&def.elems_value, def.reftype, reader, self);
                            },
                            0x04 => {
                                def.offset = try ElementHelpers.readOffsetExpr(reader, self);
                                try ElementHelpers.readElemsExpr(&def.elems_expr, reader, self, def.reftype);
                            },
                            0x05 => {
                                def.mode = .Passive;
                                def.reftype = try ValType.decodeReftype(reader);
                                try ElementHelpers.readElemsExpr(&def.elems_expr, reader, self, def.reftype);
                            },
                            0x06 => {
                                def.table_index = try common.decodeLEB128(u32, reader);
                                def.offset = try ElementHelpers.readOffsetExpr(reader, self);
                                def.reftype = try ValType.decodeReftype(reader);
                                try ElementHelpers.readElemsExpr(&def.elems_expr, reader, self, def.reftype);
                            },
                            0x07 => {
                                def.mode = .Declarative;
                                def.reftype = try ValType.decodeReftype(reader);
                                try ElementHelpers.readElemsExpr(&def.elems_expr, reader, self, def.reftype);
                            },
                            else => {
                                return error.MalformedElementType;
                            },
                        }

                        try self.elements.append(def);
                    }
                },
                .Code => {
                    const BlockData = struct {
                        begin_index: u32,
                        opcode: Opcode,
                    };
                    var block_stack = std.ArrayList(BlockData).init(allocator);
                    defer block_stack.deinit();

                    var if_to_else_offsets = std.AutoHashMap(u32, u32).init(allocator);
                    defer if_to_else_offsets.deinit();

                    var instructions = &self.code.instructions;

                    const num_codes = try common.decodeLEB128(u32, reader);

                    if (num_codes != self.functions.items.len) {
                        return error.MalformedFunctionCodeSectionMismatch;
                    }

                    const wasm_code_address_begin: usize = stream.pos;

                    var code_index: u32 = 0;
                    while (code_index < num_codes) {
                        const code_size = try common.decodeLEB128(u32, reader);
                        const code_begin_pos = stream.pos;

                        var func_def: *FunctionDefinition = &self.functions.items[code_index];

                        // parse locals
                        {
                            const num_locals = try common.decodeLEB128(u32, reader);

                            const TypeCount = struct {
                                valtype: ValType,
                                count: u32,
                            };
                            var local_types = std.ArrayList(TypeCount).init(allocator);
                            defer local_types.deinit();
                            try local_types.ensureTotalCapacity(num_locals);

                            var locals_total: usize = 0;
                            var locals_index: u32 = 0;
                            try func_def.locals.ensureTotalCapacity(num_locals);
                            while (locals_index < num_locals) : (locals_index += 1) {
                                const n = try common.decodeLEB128(u32, reader);
                                const local_type = try ValType.decode(reader);

                                locals_total += n;
                                if (locals_total >= std.math.maxInt(u32)) {
                                    return error.MalformedTooManyLocals;
                                }
                                local_types.appendAssumeCapacity(TypeCount{ .valtype = local_type, .count = n });
                            }

                            try func_def.locals.ensureTotalCapacity(locals_total);

                            for (local_types.items) |type_count| {
                                func_def.locals.appendNTimesAssumeCapacity(type_count.valtype, type_count.count);
                            }
                        }

                        func_def.instructions_begin = @intCast(instructions.items.len);
                        try block_stack.append(BlockData{
                            .begin_index = @intCast(func_def.instructions_begin),
                            .opcode = .Block,
                        });

                        try validator.beginValidateCode(self, func_def);

                        var parsing_code = true;
                        while (parsing_code) {
                            const instruction_index = @as(u32, @intCast(instructions.items.len));

                            const wasm_instruction_address = stream.pos - wasm_code_address_begin;

                            var instruction: Instruction = try Instruction.decode(reader, self);

                            if (instruction.opcode.beginsBlock()) {
                                try block_stack.append(BlockData{
                                    .begin_index = instruction_index,
                                    .opcode = instruction.opcode,
                                });
                            } else if (instruction.opcode == .Else) {
                                const block: *const BlockData = &block_stack.items[block_stack.items.len - 1];
                                try if_to_else_offsets.putNoClobber(block.begin_index, instruction_index);
                                // the else gets the matching if's immediates
                                instruction.immediate = instructions.items[block.begin_index].immediate;
                                // and the if will have its else_continuation updated when .End is parsed
                            } else if (instruction.opcode == .End) {
                                const block: BlockData = block_stack.orderedRemove(block_stack.items.len - 1);
                                if (block_stack.items.len == 0) {
                                    parsing_code = false;

                                    func_def.continuation = instruction_index;

                                    block_stack.clearRetainingCapacity();

                                    num_functions_parsed += 1;
                                } else {
                                    var block_instruction: *Instruction = &instructions.items[block.begin_index];

                                    // fixup the block continuations in previously-emitted Instructions
                                    if (block.opcode == .Loop) {
                                        block_instruction.immediate.Block.continuation = block.begin_index;
                                    } else {
                                        switch (block_instruction.immediate) {
                                            .Block => |*v| v.continuation = instruction_index,
                                            .If => |*v| {
                                                v.end_continuation = instruction_index;
                                                v.else_continuation = instruction_index;
                                            },
                                            else => unreachable,
                                        }

                                        var else_index_or_null = if_to_else_offsets.get(block.begin_index);
                                        if (else_index_or_null) |index| {
                                            var else_instruction: *Instruction = &instructions.items[index];
                                            else_instruction.immediate = block_instruction.immediate;
                                            block_instruction.immediate.If.else_continuation = index;
                                        } else if (block_instruction.opcode == .If) {
                                            block_instruction.opcode = .IfNoElse;
                                        }
                                    }
                                }
                            }

                            try validator.validateCode(self, func_def, instruction);

                            try self.code.wasm_address_to_instruction_index.put(@as(u32, @intCast(wasm_instruction_address)), instruction_index);

                            switch (instruction.opcode) {
                                .Noop => {}, // no need to emit noops since they don't do anything
                                else => {
                                    try instructions.append(instruction);
                                },
                            }
                        }

                        try validator.endValidateCode();

                        func_def.instructions_end = @intCast(instructions.items.len);

                        const code_actual_size = stream.pos - code_begin_pos;
                        if (code_actual_size != code_size) {
                            return error.MalformedSectionSizeMismatch;
                        }

                        code_index += 1;
                    }

                    // TODO flatten all instructions into a binary stream
                },
                .Data => {
                    const num_datas = try common.decodeLEB128(u32, reader);

                    if (self.data_count != null and num_datas != self.data_count.?) {
                        return error.MalformedDataCountMismatch;
                    }

                    var data_index: u32 = 0;
                    while (data_index < num_datas) : (data_index += 1) {
                        var data = try DataDefinition.decode(reader, self, allocator);
                        try self.datas.append(data);
                    }
                },
                .DataCount => {
                    self.data_count = try common.decodeLEB128(u32, reader);
                    try self.datas.ensureTotalCapacity(self.data_count.?);
                },
            }

            var consumed_bytes = stream.pos - section_start_pos;
            if (section_size_bytes != consumed_bytes) {
                return error.MalformedSectionSizeMismatch;
            }
        }

        for (self.elements.items) |elem_def| {
            if (elem_def.mode == .Active) {
                const valtype = try ModuleValidator.getTableReftype(self, elem_def.table_index);
                if (elem_def.reftype != valtype) {
                    return error.ValidationTypeMismatch;
                }
            }
        }

        if (self.imports.memories.items.len + self.memories.items.len > 1) {
            return error.ValidationMultipleMemories;
        }

        if (num_functions_parsed != self.functions.items.len) {
            return error.MalformedFunctionCodeSectionMismatch;
        }
    }

    pub fn destroy(self: *ModuleDefinition) void {
        self.code.instructions.deinit();
        self.code.wasm_address_to_instruction_index.deinit();
        for (self.code.branch_table.items) |*item| {
            item.label_ids.deinit();
        }
        self.code.branch_table.deinit();

        for (self.imports.functions.items) |*item| {
            self.allocator.free(item.names.module_name);
            self.allocator.free(item.names.import_name);
        }
        for (self.imports.tables.items) |*item| {
            self.allocator.free(item.names.module_name);
            self.allocator.free(item.names.import_name);
        }
        for (self.imports.memories.items) |*item| {
            self.allocator.free(item.names.module_name);
            self.allocator.free(item.names.import_name);
        }
        for (self.imports.globals.items) |*item| {
            self.allocator.free(item.names.module_name);
            self.allocator.free(item.names.import_name);
        }

        for (self.exports.functions.items) |*item| {
            self.allocator.free(item.name);
        }
        for (self.exports.tables.items) |*item| {
            self.allocator.free(item.name);
        }
        for (self.exports.memories.items) |*item| {
            self.allocator.free(item.name);
        }
        for (self.exports.globals.items) |*item| {
            self.allocator.free(item.name);
        }

        for (self.types.items) |*item| {
            item.types.deinit();
        }
        for (self.functions.items) |*item| {
            item.locals.deinit();
        }
        for (self.elements.items) |*item| {
            item.elems_value.deinit();
            item.elems_expr.deinit();
        }

        self.types.deinit();
        self.imports.functions.deinit();
        self.imports.tables.deinit();
        self.imports.memories.deinit();
        self.imports.globals.deinit();
        self.functions.deinit();
        self.globals.deinit();
        self.tables.deinit();
        self.memories.deinit();
        self.elements.deinit();
        self.exports.functions.deinit();
        self.exports.tables.deinit();
        self.exports.memories.deinit();
        self.exports.globals.deinit();
        self.datas.deinit();
        self.name_section.deinit();

        for (self.custom_sections.items) |*item| {
            self.allocator.free(item.name);
            item.data.deinit();
        }
        self.custom_sections.deinit();

        self.allocator.free(self.debug_name);

        var allocator = self.allocator;
        allocator.destroy(self);
    }

    pub fn getCustomSection(self: *const ModuleDefinition, name: []const u8) ?[]u8 {
        for (self.custom_sections.items) |section| {
            if (std.mem.eql(u8, section.name, name)) {
                return section.data.items;
            }
        }

        return null;
    }

    pub fn getFunctionExport(self: *const ModuleDefinition, func_handle: FunctionHandle) FunctionExport {
        const type_index = switch (func_handle.type) {
            .Export => self.functions.items[func_handle.index].type_index,
            .Import => self.imports.functions.items[func_handle.index].type_index,
        };

        const type_def: *const FunctionTypeDefinition = &self.types.items[type_index];
        var params: []const ValType = type_def.getParams();
        var returns: []const ValType = type_def.getReturns();

        return FunctionExport{
            .params = params,
            .returns = returns,
        };
    }

    pub fn dump(self: *const ModuleDefinition, writer: anytype) !void {
        const Helpers = struct {
            fn function(_writer: anytype, functype: *const FunctionTypeDefinition) !void {
                const params: []const ValType = functype.getParams();
                const returns: []const ValType = functype.getReturns();

                try _writer.print("(", .{});
                for (params, 0..) |v, i| {
                    try _writer.print("{s}", .{valtype(v)});
                    if (i != params.len - 1) {
                        try _writer.print(", ", .{});
                    }
                }
                try _writer.print(") -> ", .{});

                if (returns.len == 0) {
                    try _writer.print("void", .{});
                } else {
                    for (returns, 0..) |v, i| {
                        try _writer.print("{s}", .{valtype(v)});
                        if (i != returns.len - 1) {
                            try _writer.print(", ", .{});
                        }
                    }
                }

                try _writer.print("\n", .{});
            }

            fn limits(_writer: anytype, l: *const Limits) !void {
                try _writer.print("limits (min {}, max {?})\n", .{ l.min, l.max });
            }

            fn valtype(v: ValType) []const u8 {
                return switch (v) {
                    .I32 => "i32",
                    .I64 => "i64",
                    .F32 => "f32",
                    .F64 => "f64",
                    .V128 => "v128",
                    .FuncRef => "funcref",
                    .ExternRef => "externref",
                };
            }

            fn mut(m: GlobalMut) []const u8 {
                return switch (m) {
                    .Immutable => "immutable",
                    .Mutable => "mutable",
                };
            }
        };

        try writer.print("Imports:\n", .{});

        try writer.print("\tFunctions: {}\n", .{self.imports.functions.items.len});
        for (self.imports.functions.items) |*import| {
            try writer.print("\t\t{s}.{s}", .{ import.names.module_name, import.names.import_name });
            try Helpers.function(writer, &self.types.items[import.type_index]);
        }

        try writer.print("\tGlobals: {}\n", .{self.imports.globals.items.len});
        for (self.imports.globals.items) |import| {
            try writer.print("\t\t{s}.{s}: type {s}, mut: {s}\n", .{
                import.names.module_name,
                import.names.import_name,
                Helpers.valtype(import.valtype),
                Helpers.mut(import.mut),
            });
        }

        try writer.print("\tMemories: {}\n", .{self.imports.memories.items.len});
        for (self.imports.memories.items) |import| {
            try writer.print("\t\t{s}.{s}: ", .{ import.names.module_name, import.names.import_name });
            try Helpers.limits(writer, &import.limits);
        }

        try writer.print("\tTables: {}\n", .{self.imports.tables.items.len});
        for (self.imports.tables.items) |import| {
            try writer.print("\t\t{s}.{s}: type {s}, ", .{
                import.names.module_name,
                import.names.import_name,
                Helpers.valtype(import.reftype),
            });
            try Helpers.limits(writer, &import.limits);
        }

        try writer.print("Exports:\n", .{});

        try writer.print("\tFunctions: {}\n", .{self.exports.functions.items.len});
        for (self.exports.functions.items) |*ex| {
            try writer.print("\t\t{s}", .{ex.name});
            const func_type: *const FunctionTypeDefinition = &self.types.items[self.getFuncTypeIndex(ex.index)];
            try Helpers.function(writer, func_type);
        }

        try writer.print("\tGlobal: {}\n", .{self.exports.globals.items.len});
        for (self.exports.globals.items) |*ex| {
            var valtype: ValType = undefined;
            var mut: GlobalMut = undefined;
            if (ex.index < self.imports.globals.items.len) {
                valtype = self.imports.globals.items[ex.index].valtype;
                mut = self.imports.globals.items[ex.index].mut;
            } else {
                const instance_index: usize = ex.index - self.imports.globals.items.len;
                valtype = self.globals.items[instance_index].valtype;
                mut = self.globals.items[instance_index].mut;
            }
            try writer.print("\t\t{s}: type {s}, mut: {s}\n", .{ ex.name, Helpers.valtype(valtype), Helpers.mut(mut) });
        }

        try writer.print("\tMemories: {}\n", .{self.exports.memories.items.len});
        for (self.exports.memories.items) |*ex| {
            var limits: *const Limits = undefined;
            if (ex.index < self.imports.memories.items.len) {
                limits = &self.imports.memories.items[ex.index].limits;
            } else {
                const instance_index: usize = ex.index - self.imports.memories.items.len;
                limits = &self.memories.items[instance_index].limits;
            }
            try writer.print("\t\t{s}: ", .{ex.name});
            try Helpers.limits(writer, limits);
        }

        try writer.print("\tTables: {}\n", .{self.exports.tables.items.len});
        for (self.exports.tables.items) |*ex| {
            var reftype: ValType = undefined;
            var limits: *const Limits = undefined;
            if (ex.index < self.imports.tables.items.len) {
                reftype = self.imports.tables.items[ex.index].reftype;
                limits = &self.imports.tables.items[ex.index].limits;
            } else {
                const instance_index: usize = ex.index - self.imports.tables.items.len;
                reftype = self.tables.items[instance_index].reftype;
                limits = &self.tables.items[instance_index].limits;
            }
            try writer.print("\t\t{s}: type {s}, ", .{ ex.name, Helpers.valtype(reftype) });
            try Helpers.limits(writer, limits);
        }
    }

    fn getFuncTypeIndex(self: *const ModuleDefinition, func_index: usize) usize {
        if (func_index < self.imports.functions.items.len) {
            const func_def: *const FunctionImportDefinition = &self.imports.functions.items[func_index];
            return func_def.type_index;
        } else {
            const module_func_index = func_index - self.imports.functions.items.len;
            const func_def: *const FunctionDefinition = &self.functions.items[module_func_index];
            return func_def.type_index;
        }
    }
};
