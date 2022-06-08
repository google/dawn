// Copyright 2020 The Dawn Authors
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#ifndef SRC_DAWN_NATIVE_SURFACE_H_
#define SRC_DAWN_NATIVE_SURFACE_H_

#include "dawn/native/Error.h"
#include "dawn/native/Forward.h"
#include "dawn/native/ObjectBase.h"

#include "dawn/native/dawn_platform.h"

#include "dawn/common/Platform.h"

#if DAWN_PLATFORM_IS(WINDOWS)
#include "dawn/native/d3d12/d3d12_platform.h"
#endif  // DAWN_PLATFORM_IS(WINDOWS)

// Forward declare IUnknown
// GetCoreWindow needs to return an IUnknown pointer
// non-windows platforms don't have this type
struct IUnknown;

namespace dawn::native {

MaybeError ValidateSurfaceDescriptor(const InstanceBase* instance,
                                     const SurfaceDescriptor* descriptor);

// A surface is a sum types of all the kind of windows Dawn supports. The OS-specific types
// aren't used because they would cause compilation errors on other OSes (or require
// ObjectiveC).
// The surface is also used to store the current swapchain so that we can detach it when it is
// replaced.
class Surface final : public ErrorMonad {
  public:
    static Surface* MakeError(InstanceBase* instance);

    Surface(InstanceBase* instance, const SurfaceDescriptor* descriptor);

    void SetAttachedSwapChain(NewSwapChainBase* swapChain);
    NewSwapChainBase* GetAttachedSwapChain();

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
    Ref<NewSwapChainBase> mSwapChain;

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

#if DAWN_PLATFORM_IS(WINDOWS)
    // WindowsCoreWindow
    ComPtr<IUnknown> mCoreWindow;

    // WindowsSwapChainPanel
    ComPtr<IUnknown> mSwapChainPanel;
#endif  // DAWN_PLATFORM_IS(WINDOWS)

    // Xlib
    void* mXDisplay = nullptr;
    uint32_t mXWindow = 0;
};

// Not defined in webgpu_absl_format.h/cpp because you can't forward-declare a nested type.
absl::FormatConvertResult<absl::FormatConversionCharSet::kString>
AbslFormatConvert(Surface::Type value, const absl::FormatConversionSpec& spec, absl::FormatSink* s);

}  // namespace dawn::native

#endif  // SRC_DAWN_NATIVE_SURFACE_H_
