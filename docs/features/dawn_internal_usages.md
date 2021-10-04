# Dawn Internal Usages

The `dawn-internal-usages` feature allows adding additional usage which affects how a texture is allocated, but does not affect frontend validation.

One use case for this is so that Chromium can use an internal copyTextureToTexture command to implement copies from a WebGPU texture-backed canvas to other Web platform primitives when the swapchain texture was not explicitly created with CopySrc usage in Javascript.

```
Usage:

wgpu::DawnTextureInternalUsageDescriptor internalDesc = {};
internalDesc.internalUsage = wgpu::TextureUsage::CopySrc;

wgpu::TextureDescriptor desc = {};
// set properties of desc.
desc.nextInChain = &internalDesc;

device.createTexture(&desc);
```
