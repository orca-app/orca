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

fn streql(a: []const u8, b: []const u8) bool {
    return std.mem.eql(u8, a, b);
}

const BindingUntyped = struct {
    const CType = struct {
        name: []const u8,
        tag: []const u8,
    };

    const Arg = struct {
        // Basically a union
        const Length = struct {
            // call a function with these args to determine the length
            proc: ?[]const u8,
            args: ?[]const []const u8,

            // length specified by another argument
            count: ?[]const u8,

            // hardcoded length
            components: ?u32,
        };

        name: []const u8,
        type: CType,
        len: ?Length,
    };

    name: []const u8,
    cname: []const u8,
    ret: CType,
    args: []Arg,

    fn needsArgPtrStub(self: Binding) bool {
        var needs_stub: bool = streql(self.ret.tag, "S");
        for (self.args) |arg| {
            if (streql(arg.type.tag, "S")) {
                needs_stub = true;
                break;
            }
        }
        return needs_stub;
    }
};

const Binding = struct {
    const Tag = enum {
        Struct,
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
            }

            std.log.err("Unknown tag type {s} in binding {s}", .{ tag, binding_name });
            return error.UnknownTag;
        }

        fn toValtype(tag: Tag, binding_name: []const u8) ![]const u8 {
            std.debug.assert(tag != .Struct);

            return switch (tag) {
                .Int32 => "i",
                .Int64 => "I",
                .Float32 => "f",
                .Float64 => "F",
                .Struct => {
                    std.log.err("Cannot convert struct tag to valtype in binding {s}", .{binding_name});
                    return error.StructToValtype;
                },
            };
        }
    };

    const CType = struct {
        name: []const u8,
        tag: Tag,
    };

    const Arg = struct {
        const Length = union {
            proc: struct {
                name: []const u8,
                args: []const []const u8,
            },
            count: []const u8,
            components: u32,
        };

        name: []const u8,
        type: CType,
        len: ?Length,
    };

    name: []const u8,
    cname: []const u8,
    ret: CType,
    args: []Arg,

    fn fromUntyped(opts: Options, untyped_bindings: []const BindingUntyped) ![]const Binding {
        const bindings = try opts.arena.alloc(Binding, untyped_bindings.len);
        for (untyped_bindings, bindings) |untyped, *binding| {
            binding.name = untyped.name;
            binding.cname = untyped.cname;
            binding.ret = CType{
                .name = untyped.ret.name,
                .tag = try Tag.fromStr(untyped.ret.tag, binding.name),
            };
            binding.args = try opts.arena.alloc(Arg, untyped.args.len);
            for (untyped.args, binding.args) |untyped_arg, *binding_arg| {
                binding_arg.name = untyped_arg.name;
                binding_arg.type = CType{
                    .name = untyped_arg.type.name,
                    .tag = try Tag.fromStr(untyped_arg.type.tag, binding.name),
                };
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

                    if (len.count) |count| {
                        if (binding_arg.len == null) {
                            binding_arg.len = Arg.Length{ .count = count };
                        } else {
                            std.log.err("Binding {s} arg {s} has invalid length settings", .{ binding.name, binding_arg.name });
                            return error.InvalidBindingArgLength;
                        }
                    }

                    if (len.components) |components| {
                        if (binding_arg.len == null) {
                            binding_arg.len = Arg.Length{ .components = components };
                        } else {
                            std.log.err("Binding {s} arg {s} has invalid length settings", .{ binding.name, binding_arg.name });
                            return error.InvalidBindingArgLength;
                        }
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
    _ = opts;
    _ = bindings;
    return error.Unimplemented;
}

fn writeBindings(path: []const u8, data: []const u8) !void {
    const file: std.fs.File = try std.fs.cwd().openFile(path, .{ .mode = .write_only });
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
        return error.FailedToReadSpecFile;
    };

    const bindings_untyped = std.json.parseFromSliceLeaky([]BindingUntyped, opts.arena, bindings_json, .{}) catch |e| {
        std.log.err("Failed to parse json. Was the json malformed, or does the binding generator need to be updated? Error: {}", .{e});
        return error.FailedToParseJson;
    };

    const bindings: []const Binding = try Binding.fromUntyped(opts, bindings_untyped);
    const generated: GeneratedBindings = try generateBindings(opts, bindings);

    std.debug.assert(generated.host.len > 0);
    try writeBindings(opts.bindings_path, generated.host);

    if (generated.guest.len > 0) {
        try writeBindings(opts.guest_stubs_path.?, generated.guest);
    }
}
