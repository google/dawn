# Chromium Experimental Texel Buffers

The `texel_buffers` language feature allows to store WebGPU plain
color format data with a `GPUBuffer`, supporting format conversion on load and store
operations. Texel buffers roughly translate to buffer textures in Vulkan
(`Dim = Buffer`), `texture_buffer<format>` in Metal Shading Language, and typed
UAV/SRV buffer views in HLSL/D3D12.

## Status

The feature is actively under review for adoption in WGSL. Although a [proposal]
(https://github.com/gpuweb/gpuweb/blob/main/proposals/texel-buffers.md) has been
accepted, implementations must not expose the feature
without an explicit draft-enable flag. Discussion is ongoing in a
[Texel Buffers Issue] (https://github.com/gpuweb/gpuweb/issues/162).

## Syntax

A new `texel_buffer` type, parameterized by the color format and the access mode:
`texel_buffer<Format, Access>`.

*   `Format` must be a WebGPU plain color format.
*   `Access` is either read or read_write.

Texel buffers overload `textureDimensions`, `textureLoad` and `textureStore`
built-in functions.

## Example usage

```wgsl
@group(0) @binding(0) var myBuffer : texel_buffer<rgba8unorm, read_write>;

@compute @workgroup_size(32)
fn computeMain(@builtin(global_invocation_id) threadId : vec3<u32>) {
    let length = textureDimensions(myBuffer);
    if (threadId.x >= length) {
        return;
    }

    var value = textureLoad(myBuffer, threadId.x);
    value = value * value;
    textureStore(myBuffer, threadId.x, value);
}
```


