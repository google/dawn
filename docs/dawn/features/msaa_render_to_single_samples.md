# MSAA Render To Single Sampled/Multisampled Render To Single Sampled

The `msaa-render-to-single-sampled` feature (MSRTSS) allows a render pass to include single-sampled attachments while rendering is done with a specified number of samples. When this feature is used, the client doesn't need to explicitly allocate any multi-sampled color textures.

Additional functionalities:
 - Adds `wgpu::DawnRenderPassSampleCount` as chained struct for `wgpu::RenderPassDescriptor` to specify number of samples to be rendered for the respective single-sampled attachment.

Example Usage:
```
// Create texture with RenderAttachment usage.
wgpu::TextureDescriptor desc = ...;
desc.sampleCount = 1;
desc.usage = wgpu::TextureUsage::RenderAttachment;

auto texture = device.CreateTexture(&desc);

// Create a render pipeline
wgpu::RenderPipelineDescriptor pipelineDesc = ...;
auto pipeline = device.CreateRenderPipeline(&pipelineDesc);

// Create a render pass with "implicit multi-sampled" enabled.
wgpu::DawnRenderPassSampleCount renderPassSampleCount;
renderPassSampleCount.sampleCount = 4;

wgpu::RenderPassDescriptor renderPassDesc = ...;
renderPassDesc.colorAttachments[0].view = texture.CreateView();
renderPassDesc.nextInChain = &renderPassSampleCount;

auto renderPassEncoder = encoder.BeginRenderPass(&renderPassDesc);

renderPassEncoder.SetPipeline(pipeline);
renderPassEncoder.Draw(3);
renderPassEncoder.End();
```

Notes:
 - To trigger MSRTSS load or store behavior, a single-sampled color attachment must be attached to a render pass with an explicit sample count given by the `wgpu::DawnRenderPassSampleCount` chained struct. The `resolveTarget` field of the the color attachment must be `nullptr`.
 - Traditional multi-sampled color or depth/stencil attachments can be mixed with the MSRTSS color attachments. They must have the match the `sampleCount` specified by the `wgpu::DawnRenderPassSampleCount` chained struct and color attachments may have a `resolveTarget` as usual.
 - If a texture is not resolvable by WebGPU standard it cannot be used as an MSRTSS color attachment. This means this feature currently doesn't work with integer textures.
