// This is a zig program run as part of the orca build process to generate bindings in C for wasm
// functions.

const std = @import("std");

const MAX_FILE_SIZE = 1024 * 1024 * 128;

const Options = struct {
    arena: std.mem.Allocator,
    api_name: []const u8,
    spec_path: []const u8,
    bindings_path: []const u8,
    guest_stubs_path: ?[]const u8,
    guest_include_path: ?[]const u8,

    fn parse(args: []const [:0]const u8, arena: std.mem.Allocator) !Options {
        var api_name: ?[]const u8 = null;
        var spec_path: ?[]const u8 = null;
        var bindings_path: ?[]const u8 = null;
        var guest_stubs_path: ?[]const u8 = null;
        var guest_include_path: ?[]const u8 = null;

        for (args, 0..) |raw_arg, i| {
            if (i == 0) {
                continue;
            }

            var split_iter = std.mem.splitScalar(u8, raw_arg, '=');
            const arg: []const u8 = split_iter.next().?;
            if (std.mem.eql(u8, arg, "--api-name")) {
                api_name = split_iter.next();
            } else if (std.mem.eql(u8, arg, "--spec-path")) {
                spec_path = split_iter.next();
            } else if (std.mem.eql(u8, arg, "--bindings-path")) {
                bindings_path = split_iter.next();
            } else if (std.mem.eql(u8, arg, "--guest-stubs-path")) {
                guest_stubs_path = split_iter.next();
            } else if (std.mem.eql(u8, arg, "--guest-include-path")) {
                guest_include_path = split_iter.next();
            }
        }

        var missing_arg: ?[]const u8 = null;
        if (api_name == null) {
            missing_arg = "api-name";
        } else if (spec_path == null) {
            missing_arg = "spec-path";
        } else if (bindings_path == null) {
            missing_arg = "bindings-path";
        }

        if (missing_arg) |arg| {
            std.log.err("Missing required arg: {s}", .{arg});
            return error.MissingRequiredArgument;
        }

        return Options{
            .arena = arena,
            .api_name = api_name.?,
            .spec_path = spec_path.?,
            .bindings_path = bindings_path.?,
            .guest_stubs_path = guest_stubs_path,
            .guest_include_path = guest_include_path,
        };
    }
};

const BindingUntyped = struct {
    const CType = struct {
        name: []const u8,
        cname: ?[]const u8 = null,
        tag: []const u8,
    };

    const Arg = struct {
        // Basically a union
        const Length = struct {
            // call a function with these args to determine the length
            proc: ?[]const u8 = null,
            args: ?[]const []const u8 = null,

            // length specified by another argument
            count: ?[]const u8 = null,

            // hardcoded length
            components: ?u32 = null,
        };

        name: []const u8,
        type: CType,
        len: ?Length = null,
    };

    name: []const u8,
    cname: []const u8,
    ret: CType,
    args: []Arg,
};

const Binding = struct {
    const Tag = enum {
        Void,
        Struct,
        Pointer,
        Int32,
        Int64,
        Float32,
        Float64,

        fn fromStr(tag: []const u8, binding_name: []const u8) !Tag {
            if (std.mem.eql(u8, tag, "i")) {
                return .Int32;
            } else if (std.mem.eql(u8, tag, "I")) {
                return .Int64;
            } else if (std.mem.eql(u8, tag, "f")) {
                return .Float32;
            } else if (std.mem.eql(u8, tag, "F")) {
                return .Float64;
            } else if (std.mem.eql(u8, tag, "S")) {
                return .Struct;
            } else if (std.mem.eql(u8, tag, "p")) {
                return .Pointer;
            } else if (std.mem.eql(u8, tag, "v")) {
                return .Void;
            }

            std.log.err("Unknown tag type {s} in binding {s}", .{ tag, binding_name });
            return error.UnknownTag;
        }

        fn toValtype(tag: Tag, binding_name: []const u8) ![]const u8 {
            std.debug.assert(tag != .Struct);

            return switch (tag) {
                .Int32 => "OC_WASM_VALTYPE_I32",
                .Int64 => "OC_WASM_VALTYPE_I64",
                .Float32 => "OC_WASM_VALTYPE_F32",
                .Float64 => "OC_WASM_VALTYPE_F64",
                else => {
                    std.log.err("Cannot convert {} tag to valtype in binding {s}", .{ tag, binding_name });
                    return error.StructToValtype;
                },
            };
        }
    };

    const CType = struct {
        name: []const u8,
        cname: []const u8,
        tag: Tag,

        fn fromUntyped(untyped: BindingUntyped.CType, binding_name: []const u8) !CType {
            return .{
                .name = untyped.name,
                .cname = if (untyped.cname) |cname| cname else untyped.name,
                .tag = try Tag.fromStr(untyped.tag, binding_name),
            };
        }
    };

    const Arg = struct {
        const LengthType = enum {
            proc,
            components,
            count,
        };
        const Length = union(LengthType) {
            proc: struct {
                name: []const u8,
                args: []const []const u8,
            },
            components: struct {
                num: u32,
                count_arg: ?[]const u8,
            },
            count: []const u8,
        };

        name: []const u8,
        type: CType,
        len: ?Length,
    };

    name: []const u8,
    cname: []const u8,
    ret: CType,
    args: []Arg,
    needs_stub: bool,

    fn fromUntyped(opts: Options, untyped_bindings: []const BindingUntyped) ![]const Binding {
        const bindings = try opts.arena.alloc(Binding, untyped_bindings.len);
        for (untyped_bindings, bindings) |untyped, *binding| {
            binding.name = untyped.name;
            binding.cname = untyped.cname;
            binding.ret = try CType.fromUntyped(untyped.ret, binding.name);
            binding.args = try opts.arena.alloc(Arg, untyped.args.len);
            for (untyped.args, binding.args) |untyped_arg, *binding_arg| {
                binding_arg.name = untyped_arg.name;
                binding_arg.type = try CType.fromUntyped(untyped_arg.type, binding.name);
                binding_arg.len = null;
                if (untyped_arg.len) |len| {
                    if (len.proc) |proc_name| {
                        const args: []const []const u8 = if (len.args) |len_args| len_args else &.{};
                        binding_arg.len = Arg.Length{
                            .proc = .{
                                .name = proc_name,
                                .args = args,
                            },
                        };
                    }

                    if (len.components) |components| {
                        if (binding_arg.len == null) {
                            binding_arg.len = Arg.Length{
                                .components = .{
                                    .num = components,
                                    .count_arg = len.count,
                                },
                            };
                        } else {
                            std.log.err("Binding {s} arg {s} has invalid length settings", .{ binding.name, binding_arg.name });
                            return error.InvalidBindingArgLength;
                        }
                    } else if (len.count) |count| {
                        if (binding_arg.len == null) {
                            binding_arg.len = Arg.Length{ .count = count };
                        } else {
                            std.log.err("Binding {s} arg {s} has invalid length settings", .{ binding.name, binding_arg.name });
                            return error.InvalidBindingArgLength;
                        }
                    }
                }
            }

            binding.needs_stub = binding.ret.tag == .Struct;
            if (binding.needs_stub == false) {
                for (binding.args) |arg| {
                    if (arg.type.tag == .Struct) {
                        binding.needs_stub = true;
                        break;
                    }
                }
            }
        }

        return bindings;
    }
};

const GeneratedBindings = struct {
    host: []const u8,
    guest: []const u8,
};

fn generateBindings(opts: Options, bindings: []const Binding) !GeneratedBindings {
    var bindings_host_array = std.ArrayList(u8).init(opts.arena);
    try bindings_host_array.ensureUnusedCapacity(1024 * 1024);
    var host = bindings_host_array.writer();

    var bindings_guest_array = std.ArrayList(u8).init(opts.arena);
    try bindings_guest_array.ensureUnusedCapacity(1024 * 1024);
    var guest = bindings_guest_array.writer();

    if (opts.guest_include_path) |path| {
        for (bindings) |binding| {
            if (binding.needs_stub) {
                try guest.print("#include \"{s}\"\n\n", .{path});
            }
            break;
        }
    }

    for (bindings) |binding| {
        if (binding.needs_stub) {
            const argptr_stub_name = try std.mem.join(opts.arena, "", &.{ binding.name, "_argptr_stub" });

            // pointer arg stub declaration
            if (binding.ret.tag == .Struct) {
                try guest.writeAll("void");
            } else {
                try guest.writeAll(binding.ret.name);
            }
            try guest.print(" ORCA_IMPORT({s}) (", .{argptr_stub_name});

            if (binding.ret.tag == .Struct) {
                try guest.writeAll(binding.ret.name);
                try guest.writeAll("* __retArg");
                if (binding.args.len > 0) {
                    try guest.writeAll(", ");
                }
            } else if (binding.args.len == 0) {
                try guest.writeAll("void");
            }

            for (binding.args, 0..) |arg, i| {
                try guest.writeAll(arg.type.name);
                if (arg.type.tag == .Struct) {
                    try guest.writeAll("*");
                }
                try guest.print(" {s}", .{arg.name});
                if (i + 1 < binding.args.len) {
                    try guest.writeAll(", ");
                }
            }
            try guest.writeAll(");\n\n");

            // forward function to pointer arg stub declaration

            try guest.print("{s} {s}(", .{ binding.ret.name, binding.name });

            if (binding.args.len == 0) {
                try guest.writeAll("void");
            }

            for (binding.args, 0..) |arg, i| {
                try guest.print("{s} {s}", .{ arg.type.name, arg.name });
                if (i + 1 < binding.args.len) {
                    try guest.writeAll(", ");
                }
            }
            try guest.writeAll(")\n");
            try guest.writeAll("{\n");
            try guest.writeAll("\t");

            if (binding.ret.tag == .Struct) {
                try guest.print("{s} __ret;\n\t", .{binding.ret.name});
            } else if (binding.ret.tag != .Void) {
                try guest.print("{s} __ret = ", .{binding.ret.name});
            }
            try guest.print("{s}(", .{argptr_stub_name});

            if (binding.ret.tag == .Struct) {
                try guest.writeAll("&__ret");
                if (binding.args.len > 0) {
                    try guest.writeAll(", ");
                }
            }

            for (binding.args, 0..) |arg, i| {
                if (arg.type.tag == .Struct) {
                    try guest.writeAll("&");
                }

                try guest.writeAll(arg.name);
                if (i + 1 < binding.args.len) {
                    try guest.writeAll(", ");
                }
            }
            try guest.writeAll(");\n");

            if (binding.ret.tag != .Void) {
                try guest.writeAll("\treturn(__ret);\n");
            }
            try guest.writeAll("}\n\n");
        }

        // host-side stub
        try host.print(
            "void {s}_stub(const i64* restrict _params, i64* restrict _returns, u8* _mem, oc_wasm* wasm)\n",
            .{binding.cname},
        );
        try host.writeAll("{\n");

        const first_arg_index: u32 = if (binding.ret.tag == .Struct) 1 else 0;
        if (binding.ret.tag == .Struct) {
            try host.print("\t{s}* __retPtr = ({s}*)((char*)_mem + *(i32*)&_params[0]);\n", .{ binding.ret.cname, binding.ret.cname });

            try host.writeAll("\t{\n");
            try host.writeAll("\t\tOC_ASSERT_DIALOG(((char*)__retPtr >= (char*)_mem) && (((char*)__retPtr - (char*)_mem) < oc_wasm_mem_size(wasm)), \"return pointer is out of bounds\");\n");
            try host.print(
                "\t\tOC_ASSERT_DIALOG((char*)__retPtr + sizeof({s}) <= ((char*)_mem + oc_wasm_mem_size(wasm)), \"return pointer is out of bounds\");\n",
                .{binding.ret.cname},
            );
            try host.writeAll("\t}\n");
        }

        for (binding.args, 0..) |arg, i| {
            try host.writeAll("\t");

            const argtype = arg.type;

            const arg_index = i + first_arg_index;

            switch (argtype.tag) {
                .Int32 => try host.print("{s} {s} = ({s})*(i32*)&_params[{}];\n", .{ argtype.cname, arg.name, argtype.cname, arg_index }),
                .Int64 => try host.print("{s} {s} = ({s})*(i64*)&_params[{}];\n", .{ argtype.cname, arg.name, argtype.cname, arg_index }),
                .Float32 => try host.print("{s} {s} = ({s})*(f32*)&_params[{}];\n", .{ argtype.cname, arg.name, argtype.cname, arg_index }),
                .Float64 => try host.print("{s} {s} = ({s})*(f64*)&_params[{}];\n", .{ argtype.cname, arg.name, argtype.cname, arg_index }),
                .Pointer => try host.print("{s} {s} = ({s})((char*)_mem + *(u32*)&_params[{}]);\n", .{ argtype.cname, arg.name, argtype.cname, arg_index }),
                .Struct => try host.print("{s} {s} = *({s}*)((char*)_mem + *(u32*)&_params[{}]);\n", .{ argtype.cname, arg.name, argtype.cname, arg_index }),
                else => {
                    std.log.err("Found invalid void type for arg {s} in binding {s}", .{ arg.name, binding.name });
                    return error.InvalidVoidArgType;
                },
            }
        }

        // check pointer arg length
        for (binding.args) |arg| {
            if (arg.type.tag == .Pointer) {
                if (arg.len) |len| {
                    try host.writeAll("\t{\n");
                    try host.print(
                        "\t\tOC_ASSERT_DIALOG(((char*){s} >= (char*)_mem) && (((char*){s} - (char*)_mem) < oc_wasm_mem_size(wasm)), \"parameter '{s}' is out of bounds\");\n",
                        .{ arg.name, arg.name, arg.name },
                    );
                    try host.print("\t\tOC_ASSERT_DIALOG((char*){s} + ", .{arg.name});

                    switch (len) {
                        .proc => |proc| {
                            try host.print("{s}(wasm, ", .{proc.name});
                            for (proc.args, 0..) |proc_arg, i| {
                                try host.print("{s}", .{proc_arg});
                                if (i + 1 < proc.args.len) {
                                    try host.writeAll(", ");
                                }
                            }
                            try host.writeAll(")");
                        },
                        .components => |components| {
                            try host.print("{}", .{components.num});
                            if (components.count_arg) |count_arg| {
                                try host.print("*{s}", .{count_arg});
                            }
                        },
                        .count => |count_arg| {
                            try host.print("{s}", .{count_arg});
                        },
                    }

                    const cname = arg.type.cname;
                    if (std.mem.endsWith(u8, cname, "**") or (std.mem.startsWith(u8, cname, "void") == false and std.mem.startsWith(u8, cname, "const void") == false)) {
                        try host.print("*sizeof({s})", .{cname[0 .. cname.len - 1]});
                    }

                    try host.print(" <= ((char*)_mem + oc_wasm_mem_size(wasm)), \"parameter '{s}' is out of bounds\");\n", .{arg.name});
                    try host.writeAll("\t}\n");
                } else {
                    std.log.err("Binding {s} missing pointer length decoration for param '{s}'", .{ binding.name, arg.name });
                    return error.MissingRequiredPointerLength;
                }
            }
        }

        try host.writeAll("\t");
        switch (binding.ret.tag) {
            .Void => {},
            .Int32 => try host.writeAll("*((i32*)&_returns[0]) = (i32)"),
            .Int64 => try host.writeAll("*((i64*)&_returns[0]) = (i64)"),
            .Float32 => try host.writeAll("*((f32*)&_returns[0]) = (f32)"),
            .Float64 => try host.writeAll("*((f64*)&_returns[0]) = (f64)"),
            .Struct => try host.writeAll("*__retPtr = "),
            .Pointer => {
                std.log.err("Binding {s} has pointer return type, but this isn't supported yet.", .{binding.name});
                return error.UnsupportedPointerReturn;
            },
        }

        try host.print("{s}(", .{binding.cname});

        for (binding.args, 0..) |arg, i| {
            try host.writeAll(arg.name);
            if (i + 1 < binding.args.len) {
                try host.writeAll(", ");
            }
        }
        try host.writeAll(");\n}\n\n");
    }

    // link function
    try host.print("int bindgen_link_{s}_api(oc_wasm* wasm)\n", .{opts.api_name});
    try host.writeAll("{\n\toc_wasm_status status;\n");
    try host.writeAll("\tint ret = 0;\n\n");

    // for decl in data:
    for (bindings) |binding| {
        const name: []const u8 = if (binding.needs_stub) try std.mem.join(opts.arena, "", &.{ binding.name, "_argptr_stub" }) else binding.name;
        const num_args: usize = binding.args.len + @as(usize, if (binding.ret.tag == .Struct) 1 else 0);
        const num_returns: usize = if (binding.ret.tag == .Struct or binding.ret.tag == .Void) 0 else 1;

        const param_types: []const u8 = blk: {
            var params_str = std.ArrayList(u8).init(opts.arena);
            try params_str.ensureUnusedCapacity(1024 * 4);
            var writer = params_str.writer();

            if (num_args == 0) {
                try writer.writeAll("\t\toc_wasm_valtype paramTypes[1];\n");
            } else {
                try writer.writeAll("\t\toc_wasm_valtype paramTypes[] = {");

                if (binding.ret.tag == .Struct) {
                    try writer.writeAll("OC_WASM_VALTYPE_I32, ");
                }
                for (binding.args) |arg| {
                    var tag = arg.type.tag;
                    if (tag == .Pointer or tag == .Struct) {
                        tag = .Int32;
                    }
                    const valtype_str: []const u8 = try tag.toValtype(name);
                    try writer.print("{s}, ", .{valtype_str});
                }

                try writer.writeAll("};\n");
            }

            break :blk params_str.items;
        };

        const return_types: []const u8 = blk: {
            if (num_returns == 0) {
                break :blk "\t\toc_wasm_valtype returnTypes[1];\n\n";
            } else {
                const return_str = try binding.ret.tag.toValtype(name);
                break :blk try std.fmt.allocPrint(opts.arena, "\t\toc_wasm_valtype returnTypes[] = {{ {s} }};\n\n", .{return_str});
            }
        };

        try host.writeAll("\t{\n");
        try host.print("{s}", .{param_types});
        try host.print("{s}", .{return_types});
        try host.print("\t\toc_wasm_binding binding = {{0}};\n", .{}); // double {{ }} escapes the zig format specifier to be = {0}
        try host.print("\t\tbinding.importName = OC_STR8(\"{s}\");\n", .{name});
        try host.print("\t\tbinding.proc = {s}_stub;\n", .{binding.cname});
        try host.print("\t\tbinding.countParams = {};\n", .{num_args});
        try host.print("\t\tbinding.countReturns = {};\n", .{num_returns});
        try host.print("\t\tbinding.params = paramTypes;\n", .{});
        try host.print("\t\tbinding.returns = returnTypes;\n", .{});
        try host.print("\t\tstatus = oc_wasm_add_binding(wasm, &binding);\n", .{});
        try host.print("\t\tif(oc_wasm_status_is_fail(status))\n", .{});
        try host.writeAll("\t\t{\n");
        try host.print("\t\t\toc_log_error(\"Couldn't link function {s} (%s)\\n\", oc_wasm_status_str8(status).ptr);\n", .{name});
        try host.writeAll("\t\t\tret = -1;\n");
        try host.writeAll("\t\t}\n");
        try host.writeAll("\t}\n\n");
    }

    try host.writeAll("\treturn(ret);\n}\n");

    return GeneratedBindings{
        .host = bindings_host_array.items,
        .guest = bindings_guest_array.items,
    };
}

fn writeBindings(path: []const u8, data: []const u8) !void {
    const cwd = std.fs.cwd();

    if (std.fs.path.dirname(path)) |dir_path| {
        cwd.makeDir(dir_path) catch |e| {
            if (e != error.PathAlreadyExists) {
                return e;
            }
        };
    }

    const file: std.fs.File = try cwd.createFile(path, .{ .read = false, .truncate = true });
    defer file.close();
    try file.writeAll(data);
}

pub fn main() !void {
    var arena = std.heap.ArenaAllocator.init(std.heap.page_allocator);
    defer arena.deinit();
    const allocator: std.mem.Allocator = arena.allocator();

    const args: []const [:0]u8 = try std.process.argsAlloc(allocator);
    defer std.process.argsFree(allocator, args);

    const opts = try Options.parse(args, allocator);

    const bindings_json: []const u8 = std.fs.cwd().readFileAlloc(opts.arena, opts.spec_path, MAX_FILE_SIZE) catch |e| {
        std.log.err("Failed to read bindings spec file from path {s}: {}", .{ opts.spec_path, e });
        return e;
    };

    const bindings_untyped = std.json.parseFromSliceLeaky([]BindingUntyped, opts.arena, bindings_json, .{}) catch |e| {
        std.log.err("Failed to parse json. Was the json malformed, or does the binding generator need to be updated? Error: {}", .{e});
        return e;
    };

    const bindings: []const Binding = try Binding.fromUntyped(opts, bindings_untyped);
    const generated: GeneratedBindings = try generateBindings(opts, bindings);

    std.debug.assert(generated.host.len > 0);
    try writeBindings(opts.bindings_path, generated.host);

    if (generated.guest.len > 0) {
        try writeBindings(opts.guest_stubs_path.?, generated.guest);
    }
}

test "serialization with nulls works" {
    var arena = std.heap.ArenaAllocator.init(std.testing.allocator);
    defer arena.deinit();
    const allocator: std.mem.Allocator = arena.allocator();

    const Test1 = struct {
        foo: u32 = 0,
        bar: ?u32 = null,
    };

    const json =
        \\[
        \\    {
        \\        "foo": 1,
        \\        "bar": 2
        \\    },
        \\    {
        \\        "foo": 3
        \\    }
        \\]
    ;
    const results = try std.json.parseFromSliceLeaky([]Test1, allocator, json, .{});
    try std.testing.expect(results.len == 2);
    try std.testing.expect(results[0].foo == 1);
    try std.testing.expect(results[0].bar != null);
    try std.testing.expect(results[0].bar == 2);
    try std.testing.expect(results[1].foo == 3);
    try std.testing.expect(results[1].bar == null);
}
