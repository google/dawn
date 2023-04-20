# Implicit Device Synchronization

`implicit-device-sync` is an experimental feature that offers multithreading support for `dawn_native`.

Additional functionality:
 - `wgpu::Device` and most of its child objects are safe to be used on multiple threads, including:
   - `wgpu::Queue`.
   - `wgpu::BindGroup`.
   - `wgpu::BindGroupLayout`.
   - `wgpu::Buffer`. See Notes.
   - `wgpu::Texture`.
   - `wgpu::TextureView`.
   - `wgpu::ComputePipeline`.
   - `wgpu::RenderPipeline`.
   - `wgpu::PipelineLayout`.
   - `wgpu::Sampler`.
   - `wgpu::ShaderModule`.
   - `wgpu::SwapChain`.
 - These objects are *not* safe to be used concurrently:
   - `wgpu:CommandEncoder`.
   - `wgpu:ComputePassEncoder`.
   - `wgpu:RenderPassEncoder`.
   - `wgpu:RenderBundleEncoder`.
   - Except that the creation, referencing, releasing and destruction of these objects are guaranteed to be thread safe.


Notes:
 - This feature is experimental, meaning currently it has some limitations:
   - For `wgpu::Buffer` to be safe on multiple threads. Proper manual synchronization must be done by users to ensure that the buffer is not currently being mapped and being used by any render/compute pass encoder at the same time on different threads.
   - Enabling this feature will disable the compatibility between `wgpu::BindGroupLayout`. That means the two different `wgpu::BindGroupLayout` objects created with equivalent `wgpu::BindGroupLayoutDescriptor` won't be considered "group-equivalent" anymore. You can only use a `wgpu::BindGroup` with a `wgpu::RenderPipeline`/`wgpu::ComputePipeline` if they are both created with the same `wgpu::BindGroupLayout`.
     - Consider using `wgpu::RenderPipeline::GetBindGroupLayout()`/`wgpu::ComputePipeline::GetBindGroupLayout()` when creating a `wgpu::BindGroup`.
