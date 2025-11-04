# Debugging Dawn

## Toggles
There are various debug-related Toggles that can help diagnose issues. Useful debug toggles:
 - `dump_shaders`: Log input WGSL shaders and translated backend shaders (MSL/ HLSL/DXBC/DXIL / SPIR-V).
 - `disable_symbol_renaming`: As much as possible, disable renaming of symbols (variables, function names, etc.). This can make dumped shaders more readable.
 - `emit_hlsl_debug_symbols`: Sets the D3DCOMPILE_SKIP_OPTIMIZATION and D3DCOMPILE_DEBUG compilation flags when compiling HLSL code.
 - `use_user_defined_labels_in_backend`: Forward object labels to the backend so that they can be seen in native debugging tools like RenderDoc, PIX, or Mac Instruments.

Toggles may be enabled/disabled in different ways.

- **In code:**

  Use extension struct `DawnTogglesDescriptor` chained on `DeviceDescriptor`.

  For example:
  ```c++
  const char* const enabledToggles[] = {"dump_shaders", "disable_symbol_renaming"};

  wgpu::DawnTogglesDescriptor deviceTogglesDesc;
  deviceTogglesDesc.enabledToggles = enabledToggles;
  deviceTogglesDesc.enabledTogglesCount = 2;

  wgpu::DeviceDescriptor deviceDescriptor;
  deviceDescriptor.nextInChain = &deviceTogglesDesc;
  ```

- **Command-line for Chrome**

  Run Chrome with command line flags`--enable-dawn-features` and/or `--disable-dawn-features` to force enable/disable toggles. Toggles should be comma-delimited.

  For example:
  `--enable-dawn-features=dump_shaders,disable_symbol_renaming`

- **Command-line for dawn_end2end_tests/dawn_unittests**

  Run Dawn test binaries with command line flags`--enable-toggles` and/or `--disable-toggles` to force enable/disable toggles. Toggles should be comma-delimited.

  For example:
  `dawn_end2end_tests --enable-toggles=dump_shaders,disable_symbol_renaming`

## Environment Variables

 - `DAWN_DEBUG_BREAK_ON_ERROR`

    Errors in WebGPU are reported asynchronously which may make debugging difficult because at the time an error is reported, you can't easily create a breakpoint to inspect the callstack in your application.

    Setting `DAWN_DEBUG_BREAK_ON_ERROR` to a non-empty, non-zero value will execute a debug breakpoint
    instruction ([`dawn::Breakpoint()`](https://source.chromium.org/chromium/chromium/src/+/main:third_party/dawn/src/dawn/common/Assert.cpp?q=dawn::Breakpoint)) as soon as any type of error is generated.

## Tracing Native GPU API usage

Setting the environment variable `DAWN_TRACE_FILE_BASE` to some filename
tells dawn to save a GPU trace. Setting `DAWN_TRACE_DEVICE_FILTER` to some
string lets you filter which devices are traced based on the device's label.
If `DAWN_TRACE_DEVICE_FILTER` is set then, if the label contains that substring
the device will be traced. Otherwise all devices are traced.

Tracing happens from device creation until device destruction.

Note that tracing is not currently implemented on all backends.

* DirectX11 - not yet implemented
* DirectX12 - not yet implemented
* OpenGL - not yet implemented
* Metal - implemented.

  Saves a .gputrace file that can be loaded into XCode's Metal Debugger.

* Vulkan - not yet implemented

## Capturing with RenderDoc in Chrome

Currently only supported on Windows using the D3D12 backend.

Launch Chrome with the following flags:
```bash
set RENDERDOC_HOOK_EGL=0 && path\to\chrome.exe --no-sandbox --disable-gpu-sandbox --disable-direct-composition --gpu-startup-dialog --enable-dawn-features=enable_renderdoc_process_injection <path\to\chrome.exe>
```

This will start Chrome with a dialog that says "Gpu starting with pid: <pid>".

In RenderDoc, in Tools -> Settings, check "Enable process injection" and restart RenderDoc.

Go to File -> Inject into Process, search for "gpu" in the list of processes and double-click the chrome.exe one with the title "Google Chrome GPU". This is the GPU process.

Go back to the Chrome GPU dialog and click OK to close it. The GPU process will resume.

Now open a WebGPU application in a tab, and as soon as it starts rendering, each frame will be automatically captured and sent to RenderDoc. To stop the captures, switch tabs, close the tab, or exit Chrome.

Note that you can use the "Refresh" button in RenderDoc's "Inject into Process" tab the next time Chrome is launched to select the GPU process again.
