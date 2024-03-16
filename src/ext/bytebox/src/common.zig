// Lowest layer of the codebase, that contains types and code used in higher layers

const std = @import("std");

pub const StableArray = @import("zig-stable-array/stable_array.zig").StableArray;

pub fn decodeLEB128(comptime T: type, reader: anytype) !T {
    if (@typeInfo(T).Int.signedness == .signed) {
        return std.leb.readILEB128(T, reader) catch |e| {
            if (e == error.Overflow) {
                return error.MalformedLEB128;
            } else {
                return e;
            }
        };
    } else {
        return std.leb.readULEB128(T, reader) catch |e| {
            if (e == error.Overflow) {
                return error.MalformedLEB128;
            } else {
                return e;
            }
        };
    }
}

pub const ScratchAllocator = struct {
    buffer: StableArray(u8),

    const InitOpts = struct {
        max_size: usize,
    };

    fn init(opts: InitOpts) ScratchAllocator {
        return ScratchAllocator{
            .buffer = StableArray(u8).init(opts.max_size),
        };
    }

    pub fn allocator(self: *ScratchAllocator) std.mem.Allocator {
        return std.mem.Allocator.init(self, alloc, resize, free);
    }

    pub fn reset(self: *ScratchAllocator) void {
        self.buffer.resize(0) catch unreachable;
    }

    fn alloc(
        self: *ScratchAllocator,
        len: usize,
        ptr_align: u29,
        len_align: u29,
        ret_addr: usize,
    ) std.mem.Allocator.Error![]u8 {
        _ = ret_addr;
        _ = len_align;

        const alloc_size = len;
        const offset_begin = std.mem.alignForward(self.buffer.items.len, ptr_align);
        const offset_end = offset_begin + alloc_size;
        self.buffer.resize(offset_end) catch {
            return std.mem.Allocator.Error.OutOfMemory;
        };
        return self.buffer.items[offset_begin..offset_end];
    }

    fn resize(
        self: *ScratchAllocator,
        old_mem: []u8,
        old_align: u29,
        new_size: usize,
        len_align: u29,
        ret_addr: usize,
    ) ?usize {
        _ = self;
        _ = old_align;
        _ = ret_addr;

        if (new_size > old_mem.len) {
            return null;
        }
        const aligned_size: usize = if (len_align == 0) new_size else std.mem.alignForward(new_size, len_align);
        return aligned_size;
    }

    fn free(
        self: *ScratchAllocator,
        old_mem: []u8,
        old_align: u29,
        ret_addr: usize,
    ) void {
        _ = self;
        _ = old_mem;
        _ = old_align;
        _ = ret_addr;
    }
};
