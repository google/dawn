# Dawn partial Load Resolve Texture

The `dawn-partial-load-resolve-texture` feature is an extension to `dawn-load-resolve-texture`, which in addition allows to specify a rect sub-region of texture, where load and resolve will take effect only. The feature can't be available unless `dawn-load-resolve-texture` is available.

Additional functionalities:
 - Adds `wgpu::RenderPassDescriptorResolveRect` as chained struct for `wgpu::RenderPassDescriptor`. It defines an expanding  rect of {`colorOffsetX`, `colorOffsetY`, `width`, `height`} and a resolving rect {`resolveOffsetX`, `resolveOffsetY`, `width`, `height`} to indicate that expanding and resolving are only performed partially on the texels within this rect region of texture. The texels outside of the rect are not impacted.

Example Usage 1, ExpandResolveTexture and RenderPassDescriptorResolveRect:

When the load operation is set to wgpu::LoadOp::ExpandResolveTexture:
1. Expand the resolve texture's region {resolveOffsetX, resolveOffsetY, width, height} into the MSAA texture's region {colorOffsetX, colorOffsetY, width, height}.
2. Optional user draws to the MSAA texture.
3. Resolve the MSAA texture's region {colorOffsetX, colorOffsetY, width, height} into the resolve texture's region {resolveOffsetX, resolveOffsetY, width, height}.

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

// Create a render pipeline with wgpu::ColorTargetStateExpandResolveTextureDawn.
wgpu::ColorTargetStateExpandResolveTextureDawn pipelineExpandResolveTextureState;
pipelineExpandResolveTextureState.enabled = true;

wgpu::RenderPipelineDescriptor pipelineDesc = ...;
pipelineDesc.multisample.count = 4;

pipelineDesc->fragment->targets[0].nextInChain = &pipelineExpandResolveTextureState;

auto pipeline = device.CreateRenderPipeline(&pipelineDesc);

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

// If there is no need to expand and resolve the whole texture,
// RenderPassDescriptorResolveRect can be used to specify a
// subregion of texture to be updated only.
wgpu::RenderPassDescriptorResolveRect rect{};
rect.colorOffsetX = rect.colorOffsetY = 1;
rect.resolveOffsetX = rect.resolveOffsetY = 1;
rect.width = rect.height = 32;
renderPassDesc2.nextInChain = &rect;

auto renderPassEncoder2 = encoder.BeginRenderPass(&renderPassDesc2);
renderPassEncoder2.SetPipeline(pipeline);
renderPassEncoder2.Draw(3);
renderPassEncoder2.End();
```

Example Usage 2, Clear and RenderPassDescriptorResolveRect:

Similar to the above example, but using wgpu::LoadOp::Clear:
1. Clear the entire MSAA texture using the clearValue.
2. Optional user draws to the MSAA texture.
3. Resolve the MSAA texture's region {colorOffsetX, colorOffsetY, width, height} into the resolve texture's region {resolveOffsetX, resolveOffsetY, width, height}.

```
// Create another render pass with "Clear" LoadOp.
wgpu::RenderPassDescriptor renderPassDesc2 = ...;
renderPassDesc2.colorAttachments[0].view = msaaTexture.CreateView();
renderPassDesc2.colorAttachments[0].resolveTarget
 = resolveTexture.CreateView();
renderPassDesc2.colorAttachments[0].loadOp = wgpu::LoadOp::Clear;
renderPassDesc2.colorAttachments[0].clearValue = clearValue;

wgpu::RenderPassDescriptorResolveRect rect{};
rect.colorOffsetX = rect.colorOffsetY = 1;
rect.resolveOffsetX = rect.resolveOffsetY = 1;
rect.width = rect.height = 32;
renderPassDesc2.nextInChain = &rect;
```

Example Usage 3, Load and RenderPassDescriptorResolveRect:

Similar to the first example, but using wgpu::LoadOp::Load:
1. Load the MSAA texture.
2. Optional user draws to the MSAA texture.
3. Resolve the MSAA texture's region {colorOffsetX, colorOffsetY, width, height} into the resolve texture's region {resolveOffsetX, resolveOffsetY, width, height}.

```
// Create another render pass with "Load" LoadOp.
wgpu::RenderPassDescriptor renderPassDesc2 = ...;
renderPassDesc2.colorAttachments[0].view = msaaTexture.CreateView();
renderPassDesc2.colorAttachments[0].resolveTarget
 = resolveTexture.CreateView();
renderPassDesc2.colorAttachments[0].loadOp = wgpu::LoadOp::Load;

wgpu::RenderPassDescriptorResolveRect rect{};
rect.colorOffsetX = rect.colorOffsetY = 1;
rect.resolveOffsetX = rect.resolveOffsetY = 1;
rect.width = rect.height = 32;
renderPassDesc2.nextInChain = &rect;
```

Notes:
 - In case that the target size of a render pass is very large, the cost of using `wgpu::LoadOp::ExpandResolveTexture` can be rather expensive, as it always assumes full-size expand and resolve. More commonly in reality, each frame we only need to re-draw a small damage region, of which UI frameworks usually have the knowledge, instead of the full window, or webpage. This feature aims to eliminate the waste by doing partial expand and resolve with the hint of `wgpu::RenderPassDescriptorResolveRect`, the actual damage region.
 - This feature is also useful to some scenarios where the applications want to use a smaller MSAA texture to render to a larger single sampled texture.
 - If the color attachment's loadOp is Load or Clear, then RenderPassDescriptorResolveRect will be ignored in the loading step. It will still be used in the resolving step to do a partial resolve.
 - There is also an existing rect RenderPassDescriptorExpandResolveRect which only defines the same offset for both color & resolve textures. This rect is deprecated and should not be used in future applications. It will be removed at some point.
 - The feature currently is only available on dawn d3d11 backend. Internally, both expand and resolve are implemented with a dedicated `wgpu::RenderPipeline`. `wgpu::RenderPassEncoder::APISetScissorRect` is used to set the scissor rect to `wgpu::RenderPassDescriptorResolveRect`, when using the pipeline. The major difference is that expand lives in the original render pass, while resolve requires a separate one.