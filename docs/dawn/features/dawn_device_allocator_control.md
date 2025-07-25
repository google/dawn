# Dawn Device Allocator Control (experimental)

This feature allows `wgpu::DawnDeviceAllocatorControl` to be chained on `wgpu::DeviceDescriptor`. If
`wgpu::DawnDeviceAllocatorControl` is chained and `wgpu::FeatureName::DawnDeviceAllocatorControl` is
not in `requiredFeatures` then creating the device will fail with a validation error.

It is available in the Vulkan backend.
