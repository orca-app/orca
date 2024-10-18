const std = @import("std");
const builtin = @import("builtin");
const os = std.os;
const mem = std.mem;
const assert = std.debug.assert;

const darwin = struct {
    extern "c" fn madvise(ptr: [*]align(mem.page_size) u8, length: usize, advice: c_int) c_int;
};

pub fn StableArray(comptime T: type) type {
    return StableArrayAligned(T, @alignOf(T));
}

pub fn StableArrayAligned(comptime T: type, comptime alignment: u29) type {
    if (@sizeOf(T) == 0) {
        @compileError("StableArray does not support types of size 0. Use ArrayList instead.");
    }

    return struct {
        const Self = @This();

        pub const Slice = []align(alignment) T;
        pub const VariableSlice = [*]align(alignment) T;

        pub const k_sizeof: usize = if (alignment > @sizeOf(T)) alignment else @sizeOf(T);

        items: Slice,
        capacity: usize,
        max_virtual_alloc_bytes: usize,

        pub fn init(max_virtual_alloc_bytes: usize) Self {
            assert(@mod(max_virtual_alloc_bytes, mem.page_size) == 0); // max_virtual_alloc_bytes must be a multiple of mem.page_size
            return Self{
                .items = &[_]T{},
                .capacity = 0,
                .max_virtual_alloc_bytes = max_virtual_alloc_bytes,
            };
        }

        pub fn initCapacity(max_virtual_alloc_bytes: usize, capacity: usize) !Self {
            var self = Self.init(max_virtual_alloc_bytes);
            try self.ensureTotalCapacity(capacity);
            return self;
        }

        pub fn deinit(self: *Self) void {
            self.clearAndFree();
        }

        pub fn insert(self: *Self, n: usize, item: T) !void {
            try self.ensureUnusedCapacity(1);
            self.items.len += 1;

            mem.copyBackwards(T, self.items[n + 1 .. self.items.len], self.items[n .. self.items.len - 1]);
            self.items[n] = item;
        }

        pub fn insertSlice(self: *Self, i: usize, items: []const T) !void {
            try self.ensureUnusedCapacity(items.len);
            self.items.len += items.len;

            mem.copyBackwards(T, self.items[i + items.len .. self.items.len], self.items[i .. self.items.len - items.len]);
            mem.copy(T, self.items[i .. i + items.len], items);
        }

        pub fn replaceRange(self: *Self, start: usize, len: usize, new_items: []const T) !void {
            const after_range = start + len;
            const range = self.items[start..after_range];

            if (range.len == new_items.len)
                mem.copy(T, range, new_items)
            else if (range.len < new_items.len) {
                const first = new_items[0..range.len];
                const rest = new_items[range.len..];

                mem.copy(T, range, first);
                try self.insertSlice(after_range, rest);
            } else {
                mem.copy(T, range, new_items);
                const after_subrange = start + new_items.len;

                for (self.items[after_range..], 0..) |item, i| {
                    self.items[after_subrange..][i] = item;
                }

                self.items.len -= len - new_items.len;
            }
        }

        pub fn append(self: *Self, item: T) !void {
            const new_item_ptr = try self.addOne();
            new_item_ptr.* = item;
        }

        pub fn appendAssumeCapacity(self: *Self, item: T) void {
            const new_item_ptr = self.addOneAssumeCapacity();
            new_item_ptr.* = item;
        }

        pub fn appendSlice(self: *Self, items: []const T) !void {
            try self.ensureUnusedCapacity(items.len);
            self.appendSliceAssumeCapacity(items);
        }

        pub fn appendSliceAssumeCapacity(self: *Self, items: []const T) void {
            const old_len = self.items.len;
            const new_len = old_len + items.len;
            assert(new_len <= self.capacity);
            self.items.len = new_len;
            mem.copy(T, self.items[old_len..], items);
        }

        pub fn appendNTimes(self: *Self, value: T, n: usize) !void {
            const old_len = self.items.len;
            try self.resize(self.items.len + n);
            @memset(self.items[old_len..self.items.len], value);
        }

        pub fn appendNTimesAssumeCapacity(self: *Self, value: T, n: usize) void {
            const new_len = self.items.len + n;
            assert(new_len <= self.capacity);
            @memset(self.items.ptr[self.items.len..new_len], value);
            self.items.len = new_len;
        }

        pub const Writer = if (T != u8)
            @compileError("The Writer interface is only defined for StableArray(u8) " ++
                "but the given type is StableArray(" ++ @typeName(T) ++ ")")
        else
            std.io.Writer(*Self, error{OutOfMemory}, appendWrite);

        pub fn writer(self: *Self) Writer {
            return .{ .context = self };
        }

        fn appendWrite(self: *Self, m: []const u8) !usize {
            try self.appendSlice(m);
            return m.len;
        }

        pub fn addOne(self: *Self) !*T {
            const newlen = self.items.len + 1;
            try self.ensureTotalCapacity(newlen);
            return self.addOneAssumeCapacity();
        }

        pub fn addOneAssumeCapacity(self: *Self) *T {
            assert(self.items.len < self.capacity);

            self.items.len += 1;
            return &self.items[self.items.len - 1];
        }

        pub fn addManyAsArray(self: *Self, comptime n: usize) !*[n]T {
            const prev_len = self.items.len;
            try self.resize(self.items.len + n);
            return self.items[prev_len..][0..n];
        }

        pub fn addManyAsArrayAssumeCapacity(self: *Self, comptime n: usize) *[n]T {
            assert(self.items.len + n <= self.capacity);
            const prev_len = self.items.len;
            self.items.len += n;
            return self.items[prev_len..][0..n];
        }

        pub fn orderedRemove(self: *Self, i: usize) T {
            const newlen = self.items.len - 1;
            if (newlen == i) return self.pop();

            const old_item = self.items[i];
            for (self.items[i..newlen], 0..) |*b, j| b.* = self.items[i + 1 + j];
            self.items[newlen] = undefined;
            self.items.len = newlen;
            return old_item;
        }

        pub fn swapRemove(self: *Self, i: usize) T {
            if (self.items.len - 1 == i) return self.pop();

            const old_item = self.items[i];
            self.items[i] = self.pop();
            return old_item;
        }

        pub fn resize(self: *Self, new_len: usize) !void {
            try self.ensureTotalCapacity(new_len);
            self.items.len = new_len;
        }

        pub fn shrinkAndFree(self: *Self, new_len: usize) void {
            assert(new_len <= self.items.len);

            const new_capacity_bytes = calcBytesUsedForCapacity(new_len);
            const current_capacity_bytes: usize = calcBytesUsedForCapacity(self.capacity);

            if (new_capacity_bytes < current_capacity_bytes) {
                const bytes_to_free: usize = current_capacity_bytes - new_capacity_bytes;

                if (builtin.os.tag == .windows) {
                    const w = os.windows;
                    const addr: usize = @intFromPtr(self.items.ptr) + new_capacity_bytes;
                    w.VirtualFree(@as(w.PVOID, @ptrFromInt(addr)), bytes_to_free, w.MEM_DECOMMIT);
                } else {
                    var base_addr: usize = @intFromPtr(self.items.ptr);
                    var offset_addr: usize = base_addr + new_capacity_bytes;
                    var addr: [*]align(mem.page_size) u8 = @ptrFromInt(offset_addr);
                    if (comptime builtin.target.isDarwin()) {
                        const MADV_DONTNEED = 4;
                        const err: c_int = darwin.madvise(addr, bytes_to_free, MADV_DONTNEED);
                        switch (@as(os.darwin.E, @enumFromInt(err))) {
                            os.E.INVAL => unreachable,
                            os.E.NOMEM => unreachable,
                            else => {},
                        }
                    } else {
                        os.madvise(addr, bytes_to_free, std.c.MADV.DONTNEED) catch unreachable;
                    }
                }

                self.capacity = new_capacity_bytes / k_sizeof;
            }

            self.items.len = new_len;
        }

        pub fn shrinkRetainingCapacity(self: *Self, new_len: usize) void {
            assert(new_len <= self.items.len);
            self.items.len = new_len;
        }

        pub fn clearRetainingCapacity(self: *Self) void {
            self.items.len = 0;
        }

        pub fn clearAndFree(self: *Self) void {
            if (self.capacity > 0) {
                if (builtin.os.tag == .windows) {
                    const w = os.windows;
                    w.VirtualFree(@as(*anyopaque, @ptrCast(self.items.ptr)), 0, w.MEM_RELEASE);
                } else {
                    var slice: []align(mem.page_size) const u8 = undefined;
                    slice.ptr = @alignCast(@as([*]u8, @ptrCast(self.items.ptr)));
                    slice.len = self.max_virtual_alloc_bytes;
                    os.munmap(slice);
                }
            }

            self.capacity = 0;
            self.items = &[_]T{};
        }

        pub fn ensureTotalCapacity(self: *Self, new_capacity: usize) !void {
            const new_capacity_bytes = calcBytesUsedForCapacity(new_capacity);
            const current_capacity_bytes: usize = calcBytesUsedForCapacity(self.capacity);

            if (current_capacity_bytes < new_capacity_bytes) {
                if (self.capacity == 0) {
                    if (builtin.os.tag == .windows) {
                        const w = os.windows;
                        const addr: w.PVOID = try w.VirtualAlloc(null, self.max_virtual_alloc_bytes, w.MEM_RESERVE, w.PAGE_READWRITE);
                        self.items.ptr = @alignCast(@ptrCast(addr));
                        self.items.len = 0;
                    } else {
                        const prot: u32 = std.c.PROT.READ | std.c.PROT.WRITE;
                        const map: u32 = std.c.MAP.PRIVATE | std.c.MAP.ANONYMOUS;
                        const fd: os.fd_t = -1;
                        const offset: usize = 0;
                        var slice = try os.mmap(null, self.max_virtual_alloc_bytes, prot, map, fd, offset);
                        self.items.ptr = @alignCast(@ptrCast(slice.ptr));
                        self.items.len = 0;
                    }
                } else if (current_capacity_bytes == self.max_virtual_alloc_bytes) {
                    // If you hit this, you likely either didn't reserve enough space up-front, or have a leak that is allocating too many elements
                    return error.OutOfMemory;
                }

                if (builtin.os.tag == .windows) {
                    const w = std.os.windows;
                    _ = try w.VirtualAlloc(@as(w.PVOID, @ptrCast(self.items.ptr)), new_capacity_bytes, w.MEM_COMMIT, w.PAGE_READWRITE);
                }
            }

            self.capacity = new_capacity;
        }

        pub fn ensureUnusedCapacity(self: *Self, additional_count: usize) !void {
            return self.ensureTotalCapacity(self.items.len + additional_count);
        }

        pub fn expandToCapacity(self: *Self) void {
            self.items.len = self.capacity;
        }

        pub fn pop(self: *Self) T {
            const val = self.items[self.items.len - 1];
            self.items.len -= 1;
            return val;
        }

        pub fn popOrNull(self: *Self) ?T {
            if (self.items.len == 0) return null;
            return self.pop();
        }

        pub fn allocatedSlice(self: Self) Slice {
            return self.items.ptr[0..self.capacity];
        }

        // Make sure to update self.items.len if you indend for any writes to this
        // to modify the length of the array.
        pub fn unusedCapacitySlice(self: Self) Slice {
            return self.allocatedSlice()[self.items.len..];
        }

        pub fn calcTotalUsedBytes(self: Self) usize {
            return calcBytesUsedForCapacity(self.capacity);
        }

        fn calcBytesUsedForCapacity(capacity: usize) usize {
            return mem.alignForward(usize, k_sizeof * capacity, mem.page_size);
        }
    };
}

const TEST_VIRTUAL_ALLOC_SIZE = 1024 * 1024 * 2; // 2 MB

test "init" {
    var a = StableArray(u8).init(TEST_VIRTUAL_ALLOC_SIZE);
    assert(a.items.len == 0);
    assert(a.capacity == 0);
    assert(a.max_virtual_alloc_bytes == TEST_VIRTUAL_ALLOC_SIZE);
    a.deinit();

    var b = StableArrayAligned(u8, 16).init(TEST_VIRTUAL_ALLOC_SIZE);
    assert(b.items.len == 0);
    assert(b.capacity == 0);
    assert(b.max_virtual_alloc_bytes == TEST_VIRTUAL_ALLOC_SIZE);
    b.deinit();
}

test "append" {
    var a = StableArray(u8).init(TEST_VIRTUAL_ALLOC_SIZE);
    try a.appendSlice(&[_]u8{ 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 });
    assert(a.calcTotalUsedBytes() == mem.page_size);
    for (a.items, 0..) |v, i| {
        assert(v == i);
    }
    a.deinit();

    var b = StableArrayAligned(u8, mem.page_size).init(TEST_VIRTUAL_ALLOC_SIZE);
    try b.appendSlice(&[_]u8{ 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 });
    assert(b.calcTotalUsedBytes() == mem.page_size * 10);
    for (b.items, 0..) |v, i| {
        assert(v == i);
    }
    b.deinit();
}

test "shrinkAndFree" {
    var a = StableArray(u8).init(TEST_VIRTUAL_ALLOC_SIZE);
    try a.appendSlice(&[_]u8{ 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 });
    a.shrinkAndFree(5);

    assert(a.calcTotalUsedBytes() == mem.page_size);
    assert(a.items.len == 5);
    for (a.items, 0..) |v, i| {
        assert(v == i);
    }
    a.deinit();

    var b = StableArrayAligned(u8, mem.page_size).init(TEST_VIRTUAL_ALLOC_SIZE);
    try b.appendSlice(&[_]u8{ 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 });
    b.shrinkAndFree(5);
    assert(b.calcTotalUsedBytes() == mem.page_size * 5);
    assert(b.items.len == 5);
    for (b.items, 0..) |v, i| {
        assert(v == i);
    }
    b.deinit();

    var c = StableArrayAligned(u8, 2048).init(TEST_VIRTUAL_ALLOC_SIZE);
    try c.appendSlice(&[_]u8{ 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 });
    c.shrinkAndFree(5);
    assert(c.calcTotalUsedBytes() == mem.page_size * 3);
    assert(c.capacity == 6);
    assert(c.items.len == 5);
    for (c.items, 0..) |v, i| {
        assert(v == i);
    }
    c.deinit();
}

test "resize" {
    const max: usize = 1024 * 1024 * 1;
    var a = StableArray(u8).init(max);

    var size: usize = 512;
    while (size <= max) {
        try a.resize(size);
        size *= 2;
    }
}

test "out of memory" {
    var a = StableArrayAligned(u8, mem.page_size).init(TEST_VIRTUAL_ALLOC_SIZE);
    const max_capacity: usize = TEST_VIRTUAL_ALLOC_SIZE / mem.page_size;
    try a.appendNTimes(0xFF, max_capacity);
    for (a.items) |v| {
        assert(v == 0xFF);
    }
    assert(a.max_virtual_alloc_bytes == a.calcTotalUsedBytes());
    assert(a.capacity == max_capacity);
    assert(a.items.len == max_capacity);

    var didCatchError: bool = false;
    a.append(0) catch |err| {
        didCatchError = true;
        assert(err == error.OutOfMemory);
    };
    assert(didCatchError == true);
    a.deinit();
}
