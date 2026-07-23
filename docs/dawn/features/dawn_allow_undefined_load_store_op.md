# Dawn Allow Undefined Load Store Op
The `dawn-allow-undefined-load-store-op` feature allows `wgpu::LoadOp::Undefined` and
`wgpu::StoreOp::Undefined` to be passed for render pass attachments that would otherwise require
an explicit load/store op:
- `wgpu::LoadOp::Undefined`
  - This will be either Load or Clear depending on what the driver sees as the best performing
    option.
- `wgpu::StoreOp::Undefined`
  - This will be either Store or Discard depending on what the driver sees as the best performing
    option.
