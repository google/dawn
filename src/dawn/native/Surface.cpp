// Copyright 2020 the Dawn & Tint Authors
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

#include "dawn/native/Surface.h"

#include "dawn/common/Platform.h"
#include "dawn/native/ChainUtils.h"
#include "dawn/native/Instance.h"
#include "dawn/native/SwapChain.h"

#if defined(DAWN_USE_WINDOWS_UI)
#include <windows.ui.core.h>
#include <windows.ui.xaml.controls.h>
#endif  // defined(DAWN_USE_WINDOWS_UI)

#if defined(DAWN_USE_X11)
#include "dawn/common/xlib_with_undefs.h"
#include "dawn/native/X11Functions.h"
#endif  // defined(DAWN_USE_X11)

namespace dawn::native {

absl::FormatConvertResult<absl::FormatConversionCharSet::kString> AbslFormatConvert(
    Surface::Type value,
    const absl::FormatConversionSpec& spec,
    absl::FormatSink* s) {
    switch (value) {
        case Surface::Type::AndroidWindow:
            s->Append("AndroidWindow");
            break;
        case Surface::Type::MetalLayer:
            s->Append("MetalLayer");
            break;
        case Surface::Type::WaylandSurface:
            s->Append("WaylandSurface");
            break;
        case Surface::Type::WindowsHWND:
            s->Append("WindowsHWND");
            break;
        case Surface::Type::WindowsCoreWindow:
            s->Append("WindowsCoreWindow");
            break;
        case Surface::Type::WindowsSwapChainPanel:
            s->Append("WindowsSwapChainPanel");
            break;
        case Surface::Type::XlibWindow:
            s->Append("XlibWindow");
            break;
    }
    return {true};
}

#if defined(DAWN_ENABLE_BACKEND_METAL)
bool InheritsFromCAMetalLayer(void* obj);
#endif  // defined(DAWN_ENABLE_BACKEND_METAL)

MaybeError ValidateSurfaceDescriptor(InstanceBase* instance, const SurfaceDescriptor* descriptor) {
    DAWN_INVALID_IF(descriptor->nextInChain == nullptr,
                    "Surface cannot be created with %s. nextInChain is not specified.", descriptor);

    DAWN_TRY(ValidateSingleSType(
        descriptor->nextInChain, wgpu::SType::SurfaceDescriptorFromAndroidNativeWindow,
        wgpu::SType::SurfaceDescriptorFromMetalLayer, wgpu::SType::SurfaceDescriptorFromWindowsHWND,
        wgpu::SType::SurfaceDescriptorFromWindowsCoreWindow,
        wgpu::SType::SurfaceDescriptorFromWindowsSwapChainPanel,
        wgpu::SType::SurfaceDescriptorFromXlibWindow,
        wgpu::SType::SurfaceDescriptorFromWaylandSurface));

#if defined(DAWN_ENABLE_BACKEND_METAL)
    const SurfaceDescriptorFromMetalLayer* metalDesc = nullptr;
    FindInChain(descriptor->nextInChain, &metalDesc);
    if (metalDesc) {
        // Check that the layer is a CAMetalLayer (or a derived class).
        DAWN_INVALID_IF(!InheritsFromCAMetalLayer(metalDesc->layer),
                        "Layer must be a CAMetalLayer");
        return {};
    }
#endif  // defined(DAWN_ENABLE_BACKEND_METAL)

#if DAWN_PLATFORM_IS(ANDROID)
    const SurfaceDescriptorFromAndroidNativeWindow* androidDesc = nullptr;
    FindInChain(descriptor->nextInChain, &androidDesc);
    // Currently the best validation we can do since it's not possible to check if the pointer
    // to a ANativeWindow is valid.
    if (androidDesc) {
        DAWN_INVALID_IF(androidDesc->window == nullptr, "Android window is not set.");
        return {};
    }
#endif  // DAWN_PLATFORM_IS(ANDROID)

#if DAWN_PLATFORM_IS(WIN32)
    const SurfaceDescriptorFromWindowsHWND* hwndDesc = nullptr;
    FindInChain(descriptor->nextInChain, &hwndDesc);
    if (hwndDesc) {
        DAWN_INVALID_IF(IsWindow(static_cast<HWND>(hwndDesc->hwnd)) == 0, "Invalid HWND");
        return {};
    }
#endif  // DAWN_PLATFORM_IS(WIN32)
#if defined(DAWN_USE_WINDOWS_UI)
    const SurfaceDescriptorFromWindowsCoreWindow* coreWindowDesc = nullptr;
    FindInChain(descriptor->nextInChain, &coreWindowDesc);
    if (coreWindowDesc) {
        // Validate the coreWindow by query for ICoreWindow interface
        ComPtr<ABI::Windows::UI::Core::ICoreWindow> coreWindow;
        DAWN_INVALID_IF(coreWindowDesc->coreWindow == nullptr ||
                            FAILED(static_cast<IUnknown*>(coreWindowDesc->coreWindow)
                                       ->QueryInterface(IID_PPV_ARGS(&coreWindow))),
                        "Invalid CoreWindow");
        return {};
    }
    const SurfaceDescriptorFromWindowsSwapChainPanel* swapChainPanelDesc = nullptr;
    FindInChain(descriptor->nextInChain, &swapChainPanelDesc);
    if (swapChainPanelDesc) {
        // Validate the swapChainPanel by querying for ISwapChainPanel interface
        ComPtr<ABI::Windows::UI::Xaml::Controls::ISwapChainPanel> swapChainPanel;
        DAWN_INVALID_IF(swapChainPanelDesc->swapChainPanel == nullptr ||
                            FAILED(static_cast<IUnknown*>(swapChainPanelDesc->swapChainPanel)
                                       ->QueryInterface(IID_PPV_ARGS(&swapChainPanel))),
                        "Invalid SwapChainPanel");
        return {};
    }
#endif  // defined(DAWN_USE_WINDOWS_UI)

#if defined(DAWN_USE_WAYLAND)
    const SurfaceDescriptorFromWaylandSurface* waylandDesc = nullptr;
    FindInChain(descriptor->nextInChain, &waylandDesc);
    if (waylandDesc) {
        // Unfortunately we can't check the validity of wayland objects. Only that they
        // aren't nullptr.
        DAWN_INVALID_IF(waylandDesc->display == nullptr, "Wayland display is nullptr.");
        DAWN_INVALID_IF(waylandDesc->surface == nullptr, "Wayland surface is nullptr.");
        return {};
    }
#endif  // defined(DAWN_USE_X11)

#if defined(DAWN_USE_X11)
    const SurfaceDescriptorFromXlibWindow* xDesc = nullptr;
    FindInChain(descriptor->nextInChain, &xDesc);
    if (xDesc) {
        // Check the validity of the window by calling a getter function on the window that
        // returns a status code. If the window is bad the call return a status of zero. We
        // need to set a temporary X11 error handler while doing this because the default
        // X11 error handler exits the program on any error.
        const X11Functions* x11 = instance->GetOrLoadX11Functions();
        DAWN_INVALID_IF(!x11->IsX11Loaded(), "Couldn't load libX11.");

        XErrorHandler oldErrorHandler =
            x11->xSetErrorHandler([](Display*, XErrorEvent*) { return 0; });
        XWindowAttributes attributes;
        int status = x11->xGetWindowAttributes(reinterpret_cast<Display*>(xDesc->display),
                                               xDesc->window, &attributes);
        x11->xSetErrorHandler(oldErrorHandler);

        DAWN_INVALID_IF(status == 0, "Invalid X Window");
        return {};
    }
#endif  // defined(DAWN_USE_X11)

    return DAWN_VALIDATION_ERROR("Unsupported sType (%s)", descriptor->nextInChain->sType);
}

// static
Surface* Surface::MakeError(InstanceBase* instance) {
    return new Surface(instance, ErrorMonad::kError);
}

Surface::Surface(InstanceBase* instance, ErrorTag tag) : ErrorMonad(tag), mInstance(instance) {}

Surface::Surface(InstanceBase* instance, const SurfaceDescriptor* descriptor)
    : ErrorMonad(), mInstance(instance) {
    DAWN_ASSERT(descriptor->nextInChain != nullptr);
    const SurfaceDescriptorFromAndroidNativeWindow* androidDesc = nullptr;
    const SurfaceDescriptorFromMetalLayer* metalDesc = nullptr;
    const SurfaceDescriptorFromWindowsHWND* hwndDesc = nullptr;
    const SurfaceDescriptorFromWindowsCoreWindow* coreWindowDesc = nullptr;
    const SurfaceDescriptorFromWindowsSwapChainPanel* swapChainPanelDesc = nullptr;
    const SurfaceDescriptorFromWaylandSurface* waylandDesc = nullptr;
    const SurfaceDescriptorFromXlibWindow* xDesc = nullptr;
    FindInChain(descriptor->nextInChain, &androidDesc);
    FindInChain(descriptor->nextInChain, &metalDesc);
    FindInChain(descriptor->nextInChain, &hwndDesc);
    FindInChain(descriptor->nextInChain, &coreWindowDesc);
    FindInChain(descriptor->nextInChain, &swapChainPanelDesc);
    FindInChain(descriptor->nextInChain, &xDesc);
    FindInChain(descriptor->nextInChain, &waylandDesc);
    if (metalDesc) {
        mType = Type::MetalLayer;
        mMetalLayer = metalDesc->layer;
    } else if (androidDesc) {
        mType = Type::AndroidWindow;
        mAndroidNativeWindow = androidDesc->window;
    } else if (waylandDesc) {
        mType = Type::WaylandSurface;
        mWaylandDisplay = waylandDesc->display;
        mWaylandSurface = waylandDesc->surface;
    } else if (hwndDesc) {
        mType = Type::WindowsHWND;
        mHInstance = hwndDesc->hinstance;
        mHWND = hwndDesc->hwnd;
    } else if (coreWindowDesc) {
#if defined(DAWN_USE_WINDOWS_UI)
        mType = Type::WindowsCoreWindow;
        mCoreWindow = static_cast<IUnknown*>(coreWindowDesc->coreWindow);
#endif  // defined(DAWN_USE_WINDOWS_UI)
    } else if (swapChainPanelDesc) {
#if defined(DAWN_USE_WINDOWS_UI)
        mType = Type::WindowsSwapChainPanel;
        mSwapChainPanel = static_cast<IUnknown*>(swapChainPanelDesc->swapChainPanel);
#endif  // defined(DAWN_USE_WINDOWS_UI)
    } else if (xDesc) {
        mType = Type::XlibWindow;
        mXDisplay = xDesc->display;
        mXWindow = xDesc->window;
    } else {
        DAWN_UNREACHABLE();
    }
}

Surface::~Surface() {
    if (mSwapChain != nullptr) {
        mSwapChain->DetachFromSurface();
        mSwapChain = nullptr;
    }
}

SwapChainBase* Surface::GetAttachedSwapChain() {
    DAWN_ASSERT(!IsError());
    return mSwapChain.Get();
}

void Surface::SetAttachedSwapChain(SwapChainBase* swapChain) {
    DAWN_ASSERT(!IsError());
    mSwapChain = swapChain;
}

InstanceBase* Surface::GetInstance() const {
    return mInstance.Get();
}

Surface::Type Surface::GetType() const {
    DAWN_ASSERT(!IsError());
    return mType;
}

void* Surface::GetAndroidNativeWindow() const {
    DAWN_ASSERT(!IsError());
    DAWN_ASSERT(mType == Type::AndroidWindow);
    return mAndroidNativeWindow;
}

void* Surface::GetMetalLayer() const {
    DAWN_ASSERT(!IsError());
    DAWN_ASSERT(mType == Type::MetalLayer);
    return mMetalLayer;
}

void* Surface::GetWaylandDisplay() const {
    DAWN_ASSERT(mType == Type::WaylandSurface);
    return mWaylandDisplay;
}

void* Surface::GetWaylandSurface() const {
    DAWN_ASSERT(mType == Type::WaylandSurface);
    return mWaylandSurface;
}

void* Surface::GetHInstance() const {
    DAWN_ASSERT(!IsError());
    DAWN_ASSERT(mType == Type::WindowsHWND);
    return mHInstance;
}
void* Surface::GetHWND() const {
    DAWN_ASSERT(!IsError());
    DAWN_ASSERT(mType == Type::WindowsHWND);
    return mHWND;
}

IUnknown* Surface::GetCoreWindow() const {
    DAWN_ASSERT(!IsError());
    DAWN_ASSERT(mType == Type::WindowsCoreWindow);
#if defined(DAWN_USE_WINDOWS_UI)
    return mCoreWindow.Get();
#else
    return nullptr;
#endif
}

IUnknown* Surface::GetSwapChainPanel() const {
    DAWN_ASSERT(!IsError());
    DAWN_ASSERT(mType == Type::WindowsSwapChainPanel);
#if defined(DAWN_USE_WINDOWS_UI)
    return mSwapChainPanel.Get();
#else
    return nullptr;
#endif
}

void* Surface::GetXDisplay() const {
    DAWN_ASSERT(!IsError());
    DAWN_ASSERT(mType == Type::XlibWindow);
    return mXDisplay;
}
uint32_t Surface::GetXWindow() const {
    DAWN_ASSERT(!IsError());
    DAWN_ASSERT(mType == Type::XlibWindow);
    return mXWindow;
}

}  // namespace dawn::native
