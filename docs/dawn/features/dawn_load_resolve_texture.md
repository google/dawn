# Dawn Load Resolve Texture

The `dawn-load-resolve-texture` feature allows a render pass to expand a resolve attachment's pixels into the MSAA attachment's as part of the loading operation.

Additional functionalities:
 - Adds `wgpu::LoadOp::ExpandResolveTexture` enum value to specify that the MSAA attachment will load the pixels from its corresponding resolve texture. This is cheaper than `wgpu::LoadOp::Load` which will load the existing pixels of the MSAA attachment itself.

Example Usage:
```
// Create MSAA texture
wgpu::TextureDescriptor desc = ...;
desc.usage = wgpu::TextureUsage::RenderAttachment;
desc.sampleCount = 4;

auto msaaTexture = device.CreateTexture(&desc);


// Create resolve texture with TextureBinding usage.
wgpu::TextureDescriptor desc = ...;
desc.usage = wgpu::TextureUsage::RenderAttachment | wgpu::TextureUsage::TextureBinding;

auto resolveTexture = device.CreateTexture(&desc);

// Create a render pass which will discard the MSAA texture.
wgpu::RenderPassDescriptor renderPassDesc = ...;
renderPassDesc.colorAttachments[0].view = msaaTexture.CreateView();
renderPassDesc.colorAttachments[0].resolveTarget
 = resolveTexture.CreateView();
renderPassDesc.colorAttachments[0].storeOp
 = wgpu::StoreOp::Discard;

auto renderPassEncoder = encoder.BeginRenderPass(&renderPassDesc);
renderPassEncoder.Draw(3);
renderPassEncoder.End();


// Create another render pass with "ExpandResolveTexture" LoadOp.
// Even though we discard the previous content of the MSAA texture,
// the old pixels of the resolve texture will be reserved across
// render passes.
wgpu::RenderPassDescriptor renderPassDesc2 = ...;
renderPassDesc2.colorAttachments[0].view = msaaTexture.CreateView();
renderPassDesc2.colorAttachments[0].resolveTarget
 = resolveTexture.CreateView();
renderPassDesc2.colorAttachments[0].loadOp
 = wgpu::LoadOp::ExpandResolveTexture;

auto renderPassEncoder2 = encoder.BeginRenderPass(&renderPassDesc2);
renderPassEncoder2.Draw(3);
renderPassEncoder2.End();

```

Notes:
 - If a resolve texture is used in a `wgpu::LoadOp::ExpandResolveTexture` operation, it must have `wgpu::TextureUsage::TextureBinding` usage.
 - Currently only one color attachment is supported and the `ExpandResolveTexture` LoadOp only works on color attachment, this could be changed in future.
 - The texture is not supported if it is not resolvable by WebGPU standard. This means this feature currently doesn't work with integer textures.
