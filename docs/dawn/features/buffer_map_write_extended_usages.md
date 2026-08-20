# Buffer Map Write Extended Usages

## Background

By default WebGPU only allows `wgpu::BufferUsage::MapWrite` to be combined with
`wgpu::BufferUsage::CopySrc`. Uploading data that the GPU consumes as a uniform, vertex, index,
storage or indirect buffer therefore requires a separate staging buffer and an explicit copy on
every update.

The `wgpu::FeatureName::BufferMapWriteExtendedUsages` feature removes this restriction: it allows
creating a buffer with `wgpu::BufferUsage::MapWrite` combined with any other `wgpu::BufferUsage`,
**except** `wgpu::BufferUsage::MapRead`. This makes it possible to write directly into a mappable
buffer that is also GPU-usable, eliminating the staging buffer and the extra copy.

The feature works on the GPUs with either **cache-coherent** or **non-cache-coherent** Unified
Memory Architecture (UMA), where we can effectively upload data from CPU to GPU through mapped
pointer, and directly access the data on GPU without any extra copies.

This is a subset of the more permissive
[`BufferMapExtendedUsages`](./buffer_map_extended_usages.md) feature, which additionally allows
`wgpu::BufferUsage::MapRead` to be combined with arbitrary usages but is only available on a
narrower set of hardware (cache-coherent UMA). Because it is a superset, enabling
`BufferMapExtendedUsages` on a device implicitly enables `BufferMapWriteExtendedUsages` as well.

## Target API support

The table below summarizes the requirement to enable this feature on D3D12, Metal and Vulkan:

| Backend | Requirement to enable | Backing memory for write-only mapping |
| ------- | --------------------- | ------------------------------------- |
| Direct3D 12 | `D3D12_FEATURE_DATA_ARCHITECTURE.UMA == TRUE` | Custom heap (`D3D12_HEAP_TYPE_CUSTOM`) in the L0 system memory pool (`D3D12_MEMORY_POOL_L0`), with CPU page property `D3D12_CPU_PAGE_PROPERTY_WRITE_BACK` on cache-coherent UMA or `D3D12_CPU_PAGE_PROPERTY_WRITE_COMBINE` on non-cache-coherent UMA. |
| Metal | `[MTLDevice hasUnifiedMemory] == YES` | `MTLResourceStorageModeShared` allocation with `MTLResourceCPUCacheModeWriteCombined`; no `didModifyRange:` / synchronization needed. |
| Vulkan | A memory type exposing `VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT \| VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT \| VK_MEMORY_PROPERTY_HOST_COHERENT_BIT`. | A `HOST_COHERENT` (write-combined) memory type; `VK_MEMORY_PROPERTY_HOST_CACHED_BIT` is not required, so non-cache-coherent UMA GPUs are supported. |

## Proposal

### API

New feature name `wgpu::FeatureName::BufferMapWriteExtendedUsages`. It is currently experimental
(see [crbug.com/386255678](https://crbug.com/386255678)).

Once enabled, `wgpu::BufferUsage::MapWrite` can be combined with any other `wgpu::BufferUsage`
except `wgpu::BufferUsage::MapRead`.

#### Example Usage

```c++
wgpu::BufferDescriptor descriptor;
descriptor.size = size;
// MapWrite can now be combined with any usage other than MapRead.
descriptor.usage = wgpu::BufferUsage::MapWrite | wgpu::BufferUsage::Uniform;
wgpu::Buffer uniformBuffer = device.CreateBuffer(&descriptor);

uniformBuffer.MapAsync(wgpu::MapMode::Write, 0, size,
    wgpu::CallbackMode::AllowSpontaneous,
    [uniformBuffer, size](wgpu::MapAsyncStatus status, wgpu::StringView) {
        if (status == wgpu::MapAsyncStatus::Success) {
            memcpy(uniformBuffer.GetMappedRange(), data, size);
            uniformBuffer.Unmap();
        }
    });
```

#### Validation

- The feature must be enabled on the device before creating a buffer
  that combines `wgpu::BufferUsage::MapWrite` with non-`wgpu::BufferUsage::CopySrc` usages.
- `wgpu::BufferUsage::MapRead` cannot be combined with other GPU usages under this feature.

#### Notes and Limitations

- Reading back from a buffer created with this feature on CPU (for example via `memcpy` out of the
  mapped range) may be extremely slow because the underlying memory is typically uncached on CPU.

