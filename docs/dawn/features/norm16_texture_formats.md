# Norm16 texture formats

Adds support for the following norm16 formats with `CopySrc|CopyDest|RenderAttachment|TextureBinding` usages, multisampling and resolving capabilities:

 - `wgpu::TextureFormat::R16Unorm`
 - `wgpu::TextureFormat::RG16Unorm`
 - `wgpu::TextureFormat::RGBA16Unorm`

The initial tracking bug was https://crbug.com/dawn/1982.
