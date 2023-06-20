# Adapter Options (Unstable!)

`wgpu::RequestAdapterOptions` may be passed to `wgpuInstanceRequestAdapter` to request an adapter.

Dawn provides a few chained extension structs on `RequestAdapterOptions` to customize the behavior.
Currently, the `WGPUInstance` doesn't provide a way to query support for these features, so these
features may not be suitable for general use yet. Currently, they are used in Dawn's testing, and
in Chromium-specific integration with Dawn.

`dawn::native::Instance::EnumerateAdapters` is a Dawn native-only API that may be used to synchronously
get a list of adapters according to the RequestAdapterOptions. The members are treated as follows:
 - `RequestAdapterOptions::compatibleSurface` is ignored.
 - `RequestAdapterOptions::powerPreference` adapters are sorted according to powerPreference such that
   preferred adapters are at the front of the list. It is a preference - so if
  wgpu::PowerPreference::LowPower is passed, the list may contain only integrated GPUs, fallback adapters, or a mix of everything. Implementations *should* try to avoid returning any discrete GPUs when low power is requested if at least one integrated GPU is available.
 - `RequestAdapterOptions::compatibilityMode` all returned adapters must match the requested compatibility mode.
 - `RequestAdapterOptions::forceFallbackAdapter` all returned adapters must be fallback adapters.

If no options are passed to EnumerateAdapters, then it is as if the default `RequestAdapterOptions` are passed.

### `RequestAdapterOptionsBackendType`

Filters the adapters that Dawn discovers to only one particular backend.

### `RequestAdapterOptionsGetGLProc`

When discovering adapters on the GL backend, Dawn uses the provided `RequestAdapterOptionsGetGLProc::getProc` method to load GL procs. This extension struct does nothing on other backends.

### `RequestAdapterOptionsLUID`

When discovering adapters on D3D11 and D3D12, Dawn only discovers adapters matching the provided `RequestAdapterOptionsLUID::adapterLUID`. This extension struct does nothing on other backends.
