# Platform and API support for Dawn/Tint

## Khronos's Vulkan

Vulkan is supported with minimal features, limits and extensions being required (what's required has been measured to be wildly available).
Vulkan is the preferred target API on platforms that don't have other "modern" GPU APIs.
Vulkan is supported as best effort on other platforms (e.g. Windows and macOS).

**Required version**: Vulkan 1.0 is supported with some required extensions (`VK_KHR_maintenance1`), or Vulkan 1.1 and above without extensions.

**Required features**: `depthBiasClamp`, `fragmentStoresAndAtomics`, `fullDrawIndexUint32`, `imageCubeArray`, `independentBlend`, `sampleRateShading`, and either `textureCompressionBC` or both of `textureCompressionETC` and `textureCompressionASTC_LDR`.

**Required limits**: they are too detailed to describe here, but in general should be wildly supported.
See the [WebGPU limits](https://gpuweb.github.io/gpuweb/#limits) that mostly correspond to Vulkan limits.

**Operating system support**:

 - Linux: Supported.
 - ChromeOS: Supported.
 - Android: Work in progress.
 - Fuchsia: Work in progress.

## Microsoft's D3D12

D3D12 is supported with feature level 11.1, or feature level 11.0 with Resource Binding Tier 2.
This is the vast majority of D3D12 devices.
Supported shader models are 5.1 and above. It is the preferred target API when available.

**Windows flavor support**:

 - Win32: Supported.
 - UWP: Supported, best effort.
 - Xbox: Not supported, contributions welcome.

## Apple's Metal

Metal is supported and is the preferred target API when available.

**Apple OS support:**

 - macOS: supported.
 - iOS: supported, best effort.
 - tvOS/ipadOS/...: Not supported, contributions welcome.

## Khronos's OpenGL family

Support for OpenGL is in progress with the aim to make OpenGL ES 3.1 (with extensions and limits) supported through EGL.
Other flavors of OpenGL (desktop OpenGL) or binding APIs (GLX, WGL, EAGL, CGL) are supported as best effort with contributions welcome.


## Microsoft's D3D11

Dawn doesn't have a D3D11 backend at the moment, but D3D11 support can be achieved with the OpenGL ES backend through ANGLE's OpenGL ES to D3D11 translation.
There might be a D3D11 backend in the future.
