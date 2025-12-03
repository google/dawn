# Dawn Native

The `dawn-native` feature enables additional functionality that is supported only
when the WebGPU implementation is `dawn_native`.

Additional functionality:

 - `dawn::native::DawnInstanceDescriptor` may be chained on `wgpu::InstanceDescriptor` on instance creation to pass in the platform and additional search paths, set backend validation level, and receive instance level errors.

 - `wgpu::DawnWGSLBlocklist` may be chained on `wgpu::InstanceDescriptor` on instance creation to block some WGSL features.

 - `wgpu::DawnCacheDeviceDescriptor` may be chained on `wgpu::DeviceDescriptor` on device creation to enable cache options such as isolation keys and pass in load and store functions.

 - Synchronous `adapter.CreateDevice(const wgpu::DeviceDescriptor*)` may be called.

Notes:
 - Enabling this feature in the `wgpu::DeviceDescriptor` does nothing, but
its presence in the Adapter's set of supported features means that the additional functionality is supported.
