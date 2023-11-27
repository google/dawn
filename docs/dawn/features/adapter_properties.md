# Adapter Properties

## Memory Heaps

`wgpu::FeatureName::AdapterPropertiesMemoryHeaps` allows querying memory heap information from the adapter.

`wgpu::AdapterPropertiesMemoryHeaps` may be chained on `wgpu::AdapterProperties` in a call to `wgpu::Adapter::GetProperties` in order to query information about the memory heaps on that adapter.
The implementation will write out the number of memory heaps and information about each heap.

If `wgpu::FeatureName::AdapterPropertiesMemoryHeaps` is not available, the struct will not be populated.

Adds `wgpu::HeapProperty` which is a bitmask describing the type of memory a heap is. Valid bits:
- DeviceLocal
- HostVisible
- HostCoherent
- HostUncached
- HostCached

Note that both HostUncached and HostCached may be set if a heap can allocate pages with either cache property.

Adds `wgpu::MemoryHeapInfo` which is a struct describing a memory heap.
```
struct MemoryHeapInfo {
    HeapProperty properties;
    uint64_t size;
};
```

`wgpu::MemoryHeapInfo::size` is the size that should be allocated out of this heap. Allocating more than this may result in poor performance or may deterministically run out of memory.
