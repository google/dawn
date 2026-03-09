# Render Pass Render Area

The `render-pass-render-area` feature allows specifying a sub-region of the attachments that the
render pass will use. The "render area" defines the rectangle within which rendering operations,
including load and store operations, must be performed. Rendering operations may still modify texels
outside of the render area.

Additional functionalities:
 - Adds `wgpu::RenderPassRenderAreaRect` as chained struct for `wgpu::RenderPassDescriptor`. It
defines a rect of {`x`, `y`, `width`, `height`} to indicate that rendering operations are only
performed on the texels within this rect region of the attachments.

Example Usage:
```cpp
wgpu::RenderPassRenderAreaRect rect;
rect.x = 10;
rect.y = 10;
rect.width = 100;
rect.height = 100;

wgpu::RenderPassDescriptor renderPassDesc = ...;
renderPassDesc.nextInChain = &rect;

auto renderPassEncoder = encoder.BeginRenderPass(&renderPassDesc);
// Draw commands here...
renderPassEncoder.End();
```

Notes:
 - The render area must be contained within the dimensions of the render pass attachments.
 - The render area cannot be empty.
 - Any scissor rect set with `wgpu::RenderPassEncoder::SetScissorRect()` must be contained by the
render area rect specified in `wgpu::RenderPassRenderAreaRect`.
 - The default scissor rect is the render area.
 - The default viewport is unchanged.
 - The actual render area used by the hardware may be slightly larger than the requested area due to
tile alignment or granularity requirements. Dawn handles this alignment internally by expanding
the render area to meet the backend's granularity requirements while still encompassing the
requested region.
