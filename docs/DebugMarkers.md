# Debug Markers

Dawn provides debug tooling integration for each backend.

Debugging markers are exposed through this API:
```
partial GPUProgrammablePassEncoder {
    void pushDebugGroup(const char * markerLabel);
    void popDebugGroup();
    void insertDebugMarker(const char * markerLabel);
};
```

These APIs will result in silent no-ops if they are used without setting up
the execution environment properly. Each backend has a specific process
for setting up this environment.

## D3D12

Debug markers are currently unimplemented on D3D12 pending resolution of a licensing issue.

## Vulkan

Debug markers on Vulkan are implemented with [VK_EXT_debug_marker](https://www.khronos.org/registry/vulkan/specs/1.1-extensions/html/vkspec.html#VK_EXT_debug_marker).

To enable marker functionality, you must launch your application from your debugging tool. Attaching to an already running application is not supported.

Vulkan markers have been tested with [RenderDoc](https://renderdoc.org/).

## Metal

Debug markers on Metal are used with the XCode debugger.

To enable marker functionality, you must launch your application from XCode and use [GPU Frame Capture](https://developer.apple.com/documentation/metal/tools_profiling_and_debugging/metal_gpu_capture).

## OpenGL

Debug markers on OpenGL are not implemented and will result in a silent no-op. This is due to low adoption of the GL_EXT_debug_marker extension in Linux device drivers.