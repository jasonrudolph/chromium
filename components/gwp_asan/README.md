GWP-ASan is a sampling debug allocator, similar to ElectricFence or Page Heap,
intended to detect heap memory errors in the wild. When crashes in a GWP-ASan
allocation are detected in the crash handler, additional debug information about
the allocation/deallocation context is added to the crash minidump. See
[GWP-ASan documentation](/docs/gwp_asan.md) for more information.
