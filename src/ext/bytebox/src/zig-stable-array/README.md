# zig-stable-array
Address-stable array with a max size that allocates directly from virtual memory. Memory is only committed when actually used, and virtual page table mappings are relatively cheap, so you only pay for the memory that you're actually using. Additionally, since all memory remains inplace, and new memory is committed incrementally at the end of the array, there are no additional recopies of data made when the array is enlarged.

Ideal use cases are for large arrays that potentially grow over time. When reallocating a dynamic array with a high upper bound would be a waste of memory, and depending on dynamic resizing may incur high recopy costs due to the size of the array, consider using this array type. Another good use case is when stable pointers or slices to the array contents are desired; since the memory is never moved, pointers to the contents of the array will not be invalidated when growing. Not recommended for small arrays, since the minimum allocation size is the platform's minimum page size. Also not for use with platforms that don't support virtual memory, such as WASM.

Typical usage is to specify a large size up-front that the array should not encounter, such as 2GB+. Then use the array as usual. If freeing memory is desired, `shrinkAndFree()` will decommit memory at the end of the array. Total memory usage can be calculated with `calcTotalUsedBytes()`. The interface is very similar to ArrayList, except for the allocator semantics. Since typical heap semantics don't apply to this array, the memory is manually managed using mmap/munmap and VirtualAlloc/VirtualFree on nix and Windows platforms, respectively.

Usage:
```    
var array = StableArray(u8).init(TEST_VIRTUAL_ALLOC_SIZE);
try array.appendSlice(&[_]u8{ 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 });
assert(array.calcTotalUsedBytes() == mem.page_size);
for (array.items) |v, i| {
    assert(v == i);
}
array.shrinkAndFree(5);
assert(array.calcTotalUsedBytes() == mem.page_size);
array.deinit();
```
