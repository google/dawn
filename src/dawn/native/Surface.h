// Copyright 2020 The Dawn & Tint Authors
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// 1. Redistributions of source code must retain the above copyright notice, this
//    list of conditions and the following disclaimer.
//
// 2. Redistributions in binary form must reproduce the above copyright notice,
//    this list of conditions and the following disclaimer in the documentation
//    and/or other materials provided with the distribution.
//
// 3. Neither the name of the copyright holder nor the names of its
//    contributors may be used to endorse or promote products derived from
//    this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
// FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
// DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
// CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
// OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#ifndef SRC_DAWN_NATIVE_SURFACE_H_
#define SRC_DAWN_NATIVE_SURFACE_H_

#include "dawn/native/Error.h"
#include "dawn/native/Forward.h"
#include "dawn/native/ObjectBase.h"

#include "dawn/native/dawn_platform.h"

#include "dawn/common/Platform.h"

#if DAWN_PLATFORM_IS(WINDOWS)
#include "dawn/native/d3d/d3d_platform.h"
#endif  // DAWN_PLATFORM_IS(WINDOWS)

// Forward declare IUnknown
// GetCoreWindow needs to return an IUnknown pointer
// non-windows platforms don't have this type
struct IUnknown;

namespace dawn::native {

MaybeError ValidateSurfaceDescriptor(InstanceBase* instance, const SurfaceDescriptor* descriptor);

// A surface is a sum types of all the kind of windows Dawn supports. The OS-specific types
// aren't used because they would cause compilation errors on other OSes (or require
// ObjectiveC).
// The surface is also used to store the current swapchain so that we can detach it when it is
// replaced.
class Surface final : public ErrorMonad {
  public:
    static Surface* MakeError(InstanceBase* instance);

    Surface(InstanceBase* instance, const SurfaceDescriptor* descriptor);

    void SetAttachedSwapChain(SwapChainBase* swapChain);
    SwapChainBase* GetAttachedSwapChain();

    // These are valid to call on all Surfaces.
    enum class Type {
        AndroidWindow,
        MetalLayer,
        WaylandSurface,
        WindowsHWND,
        WindowsCoreWindow,
        WindowsSwapChainPanel,
        XlibWindow,
    };
    Type GetType() const;
    InstanceBase* GetInstance() const;

    // Valid to call if the type is MetalLayer
    void* GetMetalLayer() const;

    // Valid to call if the type is Android
    void* GetAndroidNativeWindow() const;

    // Valid to call if the type is WaylandSurface
    void* GetWaylandDisplay() const;
    void* GetWaylandSurface() const;

    // Valid to call if the type is WindowsHWND
    void* GetHInstance() const;
    void* GetHWND() const;

    // Valid to call if the type is WindowsCoreWindow
    IUnknown* GetCoreWindow() const;

    // Valid to call if the type is WindowsSwapChainPanel
    IUnknown* GetSwapChainPanel() const;

    // Valid to call if the type is WindowsXlib
    void* GetXDisplay() const;
    uint32_t GetXWindow() const;

  private:
    Surface(InstanceBase* instance, ErrorMonad::ErrorTag tag);
    ~Surface() override;

    Ref<InstanceBase> mInstance;
    Type mType;

    // The swapchain will set this to null when it is destroyed.
    Ref<SwapChainBase> mSwapChain;

    // MetalLayer
    void* mMetalLayer = nullptr;

    // ANativeWindow
    void* mAndroidNativeWindow = nullptr;

    // Wayland
    void* mWaylandDisplay = nullptr;
    void* mWaylandSurface = nullptr;

    // WindowsHwnd
    void* mHInstance = nullptr;
    void* mHWND = nullptr;

#if defined(DAWN_USE_WINDOWS_UI)
    // WindowsCoreWindow
    ComPtr<IUnknown> mCoreWindow;

    // WindowsSwapChainPanel
    ComPtr<IUnknown> mSwapChainPanel;
#endif  // defined(DAWN_USE_WINDOWS_UI)

    // Xlib
    void* mXDisplay = nullptr;
    uint32_t mXWindow = 0;
};

// Not defined in webgpu_absl_format.h/cpp because you can't forward-declare a nested type.
absl::FormatConvertResult<absl::FormatConversionCharSet::kString>
AbslFormatConvert(Surface::Type value, const absl::FormatConversionSpec& spec, absl::FormatSink* s);

}  // namespace dawn::native

#endif  // SRC_DAWN_NATIVE_SURFACE_H_
