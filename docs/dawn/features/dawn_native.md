# Dawn Native

The `dawn-native` feature enables additional functionality that is supported only
when the WebGPU implementation is `dawn_native`.

Additional functionality:
 - `wgpu::DawnTogglesDeviceDescriptor` may be chained on `wgpu::DeviceDescriptor` on device creation to enable Dawn-specific toggles on the device.

 - Synchronous `adapter.CreateDevice(const wgpu::DeviceDescriptor*)` may be called.

Notes:
 - Enabling this feature in the `wgpu::DeviceDescriptor` does nothing, but
its presence in the Adapter's set of supported features means that the additional functionality is supported.
