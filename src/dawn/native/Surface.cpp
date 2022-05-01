// Copyright 2020 the Dawn Authors
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

#include "dawn/native/Surface.h"

#include "dawn/common/Platform.h"
#include "dawn/native/ChainUtils_autogen.h"
#include "dawn/native/Instance.h"
#include "dawn/native/SwapChain.h"

#if defined(DAWN_PLATFORM_WINDOWS)
#include <windows.ui.core.h>
#include <windows.ui.xaml.controls.h>
#endif  // defined(DAWN_PLATFORM_WINDOWS)

#if defined(DAWN_USE_X11)
#include "dawn/common/xlib_with_undefs.h"
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

MaybeError ValidateSurfaceDescriptor(const InstanceBase* instance,
                                     const SurfaceDescriptor* descriptor) {
    DAWN_INVALID_IF(descriptor->nextInChain == nullptr,
                    "Surface cannot be created with %s. nextInChain is not specified.", descriptor);

    DAWN_TRY(ValidateSingleSType(
        descriptor->nextInChain, wgpu::SType::SurfaceDescriptorFromAndroidNativeWindow,
        wgpu::SType::SurfaceDescriptorFromMetalLayer, wgpu::SType::SurfaceDescriptorFromWindowsHWND,
        wgpu::SType::SurfaceDescriptorFromWindowsCoreWindow,
        wgpu::SType::SurfaceDescriptorFromWindowsSwapChainPanel,
        wgpu::SType::SurfaceDescriptorFromXlibWindow));

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

#if defined(DAWN_PLATFORM_ANDROID)
    const SurfaceDescriptorFromAndroidNativeWindow* androidDesc = nullptr;
    FindInChain(descriptor->nextInChain, &androidDesc);
    // Currently the best validation we can do since it's not possible to check if the pointer
    // to a ANativeWindow is valid.
    if (androidDesc) {
        DAWN_INVALID_IF(androidDesc->window == nullptr, "Android window is not set.");
        return {};
    }
#endif  // defined(DAWN_PLATFORM_ANDROID)

#if defined(DAWN_PLATFORM_WINDOWS)
#if defined(DAWN_PLATFORM_WIN32)
    const SurfaceDescriptorFromWindowsHWND* hwndDesc = nullptr;
    FindInChain(descriptor->nextInChain, &hwndDesc);
    if (hwndDesc) {
        DAWN_INVALID_IF(IsWindow(static_cast<HWND>(hwndDesc->hwnd)) == 0, "Invalid HWND");
        return {};
    }
#endif  // defined(DAWN_PLATFORM_WIN32)
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
#endif  // defined(DAWN_PLATFORM_WINDOWS)

#if defined(DAWN_USE_X11)
    const SurfaceDescriptorFromXlibWindow* xDesc = nullptr;
    FindInChain(descriptor->nextInChain, &xDesc);
    if (xDesc) {
        // Check the validity of the window by calling a getter function on the window that
        // returns a status code. If the window is bad the call return a status of zero. We
        // need to set a temporary X11 error handler while doing this because the default
        // X11 error handler exits the program on any error.
        XErrorHandler oldErrorHandler = XSetErrorHandler([](Display*, XErrorEvent*) { return 0; });
        XWindowAttributes attributes;
        int status = XGetWindowAttributes(reinterpret_cast<Display*>(xDesc->display), xDesc->window,
                                          &attributes);
        XSetErrorHandler(oldErrorHandler);

        DAWN_INVALID_IF(status == 0, "Invalid X Window");
        return {};
    }
#endif  // defined(DAWN_USE_X11)

    return DAWN_FORMAT_VALIDATION_ERROR("Unsupported sType (%s)", descriptor->nextInChain->sType);
}

Surface::Surface(InstanceBase* instance, const SurfaceDescriptor* descriptor)
    : mInstance(instance) {
    ASSERT(descriptor->nextInChain != nullptr);
    const SurfaceDescriptorFromAndroidNativeWindow* androidDesc = nullptr;
    const SurfaceDescriptorFromMetalLayer* metalDesc = nullptr;
    const SurfaceDescriptorFromWindowsHWND* hwndDesc = nullptr;
    const SurfaceDescriptorFromWindowsCoreWindow* coreWindowDesc = nullptr;
    const SurfaceDescriptorFromWindowsSwapChainPanel* swapChainPanelDesc = nullptr;
    const SurfaceDescriptorFromXlibWindow* xDesc = nullptr;
    FindInChain(descriptor->nextInChain, &androidDesc);
    FindInChain(descriptor->nextInChain, &metalDesc);
    FindInChain(descriptor->nextInChain, &hwndDesc);
    FindInChain(descriptor->nextInChain, &coreWindowDesc);
    FindInChain(descriptor->nextInChain, &swapChainPanelDesc);
    FindInChain(descriptor->nextInChain, &xDesc);
    if (metalDesc) {
        mType = Type::MetalLayer;
        mMetalLayer = metalDesc->layer;
    } else if (androidDesc) {
        mType = Type::AndroidWindow;
        mAndroidNativeWindow = androidDesc->window;
    } else if (hwndDesc) {
        mType = Type::WindowsHWND;
        mHInstance = hwndDesc->hinstance;
        mHWND = hwndDesc->hwnd;
    } else if (coreWindowDesc) {
#if defined(DAWN_PLATFORM_WINDOWS)
        mType = Type::WindowsCoreWindow;
        mCoreWindow = static_cast<IUnknown*>(coreWindowDesc->coreWindow);
#endif  // defined(DAWN_PLATFORM_WINDOWS)
    } else if (swapChainPanelDesc) {
#if defined(DAWN_PLATFORM_WINDOWS)
        mType = Type::WindowsSwapChainPanel;
        mSwapChainPanel = static_cast<IUnknown*>(swapChainPanelDesc->swapChainPanel);
#endif  // defined(DAWN_PLATFORM_WINDOWS)
    } else if (xDesc) {
        mType = Type::XlibWindow;
        mXDisplay = xDesc->display;
        mXWindow = xDesc->window;
    } else {
        UNREACHABLE();
    }
}

Surface::~Surface() {
    if (mSwapChain != nullptr) {
        mSwapChain->DetachFromSurface();
        mSwapChain = nullptr;
    }
}

NewSwapChainBase* Surface::GetAttachedSwapChain() {
    return mSwapChain.Get();
}

void Surface::SetAttachedSwapChain(NewSwapChainBase* swapChain) {
    mSwapChain = swapChain;
}

InstanceBase* Surface::GetInstance() {
    return mInstance.Get();
}

Surface::Type Surface::GetType() const {
    return mType;
}

void* Surface::GetAndroidNativeWindow() const {
    ASSERT(mType == Type::AndroidWindow);
    return mAndroidNativeWindow;
}

void* Surface::GetMetalLayer() const {
    ASSERT(mType == Type::MetalLayer);
    return mMetalLayer;
}

void* Surface::GetHInstance() const {
    ASSERT(mType == Type::WindowsHWND);
    return mHInstance;
}
void* Surface::GetHWND() const {
    ASSERT(mType == Type::WindowsHWND);
    return mHWND;
}

IUnknown* Surface::GetCoreWindow() const {
    ASSERT(mType == Type::WindowsCoreWindow);
#if defined(DAWN_PLATFORM_WINDOWS)
    return mCoreWindow.Get();
#else
    return nullptr;
#endif
}

IUnknown* Surface::GetSwapChainPanel() const {
    ASSERT(mType == Type::WindowsSwapChainPanel);
#if defined(DAWN_PLATFORM_WINDOWS)
    return mSwapChainPanel.Get();
#else
    return nullptr;
#endif
}

void* Surface::GetXDisplay() const {
    ASSERT(mType == Type::XlibWindow);
    return mXDisplay;
}
uint32_t Surface::GetXWindow() const {
    ASSERT(mType == Type::XlibWindow);
    return mXWindow;
}

}  // namespace dawn::native
