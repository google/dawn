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

#include <memory>
#include <string>
#include <utility>

#include "dawn/common/Platform.h"
#include "dawn/native/ChainUtils.h"
#include "dawn/native/Device.h"
#include "dawn/native/Instance.h"
#include "dawn/native/SwapChain.h"
#include "dawn/native/ValidationUtils_autogen.h"
#include "dawn/native/utils/WGPUHelpers.h"

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

ResultOrError<UnpackedPtr<SurfaceDescriptor>> ValidateSurfaceDescriptor(
    InstanceBase* instance,
    const SurfaceDescriptor* rawDescriptor) {
    DAWN_INVALID_IF(rawDescriptor->nextInChain == nullptr,
                    "Surface cannot be created with %s. nextInChain is not specified.",
                    rawDescriptor);
    UnpackedPtr<SurfaceDescriptor> descriptor;
    DAWN_TRY_ASSIGN(descriptor, ValidateAndUnpack(rawDescriptor));

    wgpu::SType type;
    DAWN_TRY_ASSIGN(type,
                    (descriptor.ValidateBranches<Branch<SurfaceDescriptorFromAndroidNativeWindow>,
                                                 Branch<SurfaceDescriptorFromMetalLayer>,
                                                 Branch<SurfaceDescriptorFromWindowsHWND>,
                                                 Branch<SurfaceDescriptorFromWindowsCoreWindow>,
                                                 Branch<SurfaceDescriptorFromWindowsSwapChainPanel>,
                                                 Branch<SurfaceDescriptorFromXlibWindow>,
                                                 Branch<SurfaceDescriptorFromWaylandSurface>>()));
    switch (type) {
#if DAWN_PLATFORM_IS(ANDROID)
        case wgpu::SType::SurfaceDescriptorFromAndroidNativeWindow: {
            auto* subDesc = descriptor.Get<SurfaceDescriptorFromAndroidNativeWindow>();
            DAWN_ASSERT(subDesc != nullptr);
            DAWN_INVALID_IF(subDesc->window == nullptr, "Android window is not set.");
            return descriptor;
        }
#endif  // DAWN_PLATFORM_IS(ANDROID)
#if defined(DAWN_ENABLE_BACKEND_METAL)
        case wgpu::SType::SurfaceDescriptorFromMetalLayer: {
            auto* subDesc = descriptor.Get<SurfaceDescriptorFromMetalLayer>();
            DAWN_ASSERT(subDesc != nullptr);
            // Check that the layer is a CAMetalLayer (or a derived class).
            DAWN_INVALID_IF(!InheritsFromCAMetalLayer(subDesc->layer),
                            "Layer must be a CAMetalLayer");
            return descriptor;
        }
#endif  // defined(DAWN_ENABLE_BACKEND_METAL)
#if DAWN_PLATFORM_IS(WIN32)
        case wgpu::SType::SurfaceDescriptorFromWindowsHWND: {
            auto* subDesc = descriptor.Get<SurfaceDescriptorFromWindowsHWND>();
            DAWN_ASSERT(subDesc != nullptr);
            DAWN_INVALID_IF(IsWindow(static_cast<HWND>(subDesc->hwnd)) == 0, "Invalid HWND");
            return descriptor;
        }
#endif  // DAWN_PLATFORM_IS(WIN32)
#if defined(DAWN_USE_WINDOWS_UI)
        case wgpu::SType::SurfaceDescriptorFromWindowsCoreWindow: {
            auto* subDesc = descriptor.Get<SurfaceDescriptorFromWindowsCoreWindow>();
            DAWN_ASSERT(subDesc != nullptr);
            // Validate the coreWindow by query for ICoreWindow interface
            ComPtr<ABI::Windows::UI::Core::ICoreWindow> coreWindow;
            DAWN_INVALID_IF(subDesc->coreWindow == nullptr ||
                                FAILED(static_cast<IUnknown*>(subDesc->coreWindow)
                                           ->QueryInterface(IID_PPV_ARGS(&coreWindow))),
                            "Invalid CoreWindow");
            return descriptor;
        }
        case wgpu::SType::SurfaceDescriptorFromWindowsSwapChainPanel: {
            auto* subDesc = descriptor.Get<SurfaceDescriptorFromWindowsSwapChainPanel>();
            DAWN_ASSERT(subDesc != nullptr);
            // Validate the swapChainPanel by querying for ISwapChainPanel interface
            ComPtr<ABI::Windows::UI::Xaml::Controls::ISwapChainPanel> swapChainPanel;
            DAWN_INVALID_IF(subDesc->swapChainPanel == nullptr ||
                                FAILED(static_cast<IUnknown*>(subDesc->swapChainPanel)
                                           ->QueryInterface(IID_PPV_ARGS(&swapChainPanel))),
                            "Invalid SwapChainPanel");
            return descriptor;
        }
#endif  // defined(DAWN_USE_WINDOWS_UI)
#if defined(DAWN_USE_WAYLAND)
        case wgpu::SType::SurfaceDescriptorFromWaylandSurface: {
            auto* subDesc = descriptor.Get<SurfaceDescriptorFromWaylandSurface>();
            DAWN_ASSERT(subDesc != nullptr);
            // Unfortunately we can't check the validity of wayland objects. Only that they
            // aren't nullptr.
            DAWN_INVALID_IF(subDesc->display == nullptr, "Wayland display is nullptr.");
            DAWN_INVALID_IF(subDesc->surface == nullptr, "Wayland surface is nullptr.");
            return descriptor;
        }
#endif  // defined(DAWN_USE_WAYLAND)
#if defined(DAWN_USE_X11)
        case wgpu::SType::SurfaceDescriptorFromXlibWindow: {
            auto* subDesc = descriptor.Get<SurfaceDescriptorFromXlibWindow>();
            DAWN_ASSERT(subDesc != nullptr);
            // Check the validity of the window by calling a getter function on the window that
            // returns a status code. If the window is bad the call return a status of zero. We
            // need to set a temporary X11 error handler while doing this because the default
            // X11 error handler exits the program on any error.
            const X11Functions* x11 = instance->GetOrLoadX11Functions();
            DAWN_INVALID_IF(!x11->IsX11Loaded(), "Couldn't load libX11.");

            XErrorHandler oldErrorHandler =
                x11->xSetErrorHandler([](Display*, XErrorEvent*) { return 0; });
            XWindowAttributes attributes;
            int status =
                x11->xGetWindowAttributes(reinterpret_cast<Display*>(subDesc->display),
                                          static_cast<Window>(subDesc->window), &attributes);
            x11->xSetErrorHandler(oldErrorHandler);

            DAWN_INVALID_IF(status == 0, "Invalid X Window");
            return descriptor;
        }
#endif  // defined(DAWN_USE_X11)
        default:
            return DAWN_VALIDATION_ERROR("Unsupported sType (%s)", type);
    }
}

MaybeError ValidateSurfaceConfiguration(DeviceBase* device,
                                        const PhysicalDeviceSurfaceCapabilities& capabilities,
                                        const SurfaceConfiguration* config,
                                        const Surface* surface) {
    UnpackedPtr<SurfaceConfiguration> unpacked;
    DAWN_TRY_ASSIGN(unpacked, ValidateAndUnpack(config));

    DAWN_TRY(config->device->ValidateIsAlive());

    const Format* format = nullptr;
    DAWN_TRY_ASSIGN(format, device->GetInternalFormat(config->format));
    DAWN_ASSERT(format != nullptr);

    // TODO(crbug.com/dawn/160): Lift this restriction once
    // wgpu::Instance::GetPreferredSurfaceFormat is implemented.
    // TODO(dawn:286):
#if DAWN_PLATFORM_IS(ANDROID)
    constexpr wgpu::TextureFormat kRequireSwapChainFormat = wgpu::TextureFormat::RGBA8Unorm;
#else
    constexpr wgpu::TextureFormat kRequireSwapChainFormat = wgpu::TextureFormat::BGRA8Unorm;
#endif  // !DAWN_PLATFORM_IS(ANDROID)
    DAWN_INVALID_IF(config->format != kRequireSwapChainFormat,
                    "Format (%s) is not %s, which is (currently) the only accepted format.",
                    config->format, kRequireSwapChainFormat);

    if (device->HasFeature(Feature::SurfaceCapabilities)) {
        wgpu::TextureUsage validUsage;
        DAWN_TRY_ASSIGN(validUsage, device->GetSupportedSurfaceUsage(surface));
        DAWN_INVALID_IF(
            !IsSubset(config->usage, validUsage),
            "Usage (%s) is not supported, %s are (currently) the only accepted usage flags.",
            config->usage, validUsage);
    } else {
        DAWN_INVALID_IF(config->usage != wgpu::TextureUsage::RenderAttachment,
                        "Usage (%s) is not %s, which is (currently) the only accepted usage. Other "
                        "usage flags require enabling %s",
                        config->usage, wgpu::TextureUsage::RenderAttachment,
                        wgpu::FeatureName::SurfaceCapabilities);
    }

    for (size_t i = 0; i < config->viewFormatCount; ++i) {
        const wgpu::TextureFormat apiViewFormat = config->viewFormats[i];
        const Format* viewFormat = nullptr;
        DAWN_TRY_ASSIGN(viewFormat, device->GetInternalFormat(apiViewFormat));
        DAWN_ASSERT(viewFormat != nullptr);

        DAWN_INVALID_IF(std::find(capabilities.formats.begin(), capabilities.formats.end(),
                                  apiViewFormat) == capabilities.formats.end(),
                        "View format (%s) is not supported by the adapter (%s) for this surface.",
                        apiViewFormat, device->GetAdapter());
    }

    DAWN_TRY(ValidatePresentMode(config->presentMode));

    // Check that config matches capabilities
    auto formatIt =
        std::find(capabilities.formats.begin(), capabilities.formats.end(), config->format);
    DAWN_INVALID_IF(formatIt == capabilities.formats.end(),
                    "Format (%s) is not supported by the adapter (%s) for this surface.",
                    config->format, config->device->GetAdapter());

    auto presentModeIt = std::find(capabilities.presentModes.begin(),
                                   capabilities.presentModes.end(), config->presentMode);
    DAWN_INVALID_IF(presentModeIt == capabilities.presentModes.end(),
                    "Present mode (%s) is not supported by the adapter (%s) for this surface.",
                    config->format, config->device->GetAdapter());

    auto alphaModeIt = std::find(capabilities.alphaModes.begin(), capabilities.alphaModes.end(),
                                 config->alphaMode);
    DAWN_INVALID_IF(alphaModeIt == capabilities.alphaModes.end(),
                    "Alpha mode (%s) is not supported by the adapter (%s) for this surface.",
                    config->format, config->device->GetAdapter());

    DAWN_INVALID_IF(config->width == 0 || config->height == 0,
                    "Surface configuration size (width: %u, height: %u) is empty.", config->width,
                    config->height);

    DAWN_INVALID_IF(
        config->width > device->GetLimits().v1.maxTextureDimension2D ||
            config->height > device->GetLimits().v1.maxTextureDimension2D,
        "Surface configuration size (width: %u, height: %u) is greater than the maximum 2D texture "
        "size (width: %u, height: %u).",
        config->width, config->height, device->GetLimits().v1.maxTextureDimension2D,
        device->GetLimits().v1.maxTextureDimension2D);

    return {};
}

class AdapterSurfaceCapCache {
  public:
    template <typename F>
    MaybeError WithAdapterCapabilities(AdapterBase* adapter, const Surface* surface, F f) {
        if (mCachedCapabilitiesAdapter.Promote().Get() != adapter) {
            const PhysicalDeviceBase* physicalDevice = adapter->GetPhysicalDevice();
            DAWN_TRY_ASSIGN(mCachedCapabilities, physicalDevice->GetSurfaceCapabilities(surface));
            mCachedCapabilitiesAdapter = GetWeakRef(adapter);
        }
        return f(mCachedCapabilities);
    }

  private:
    WeakRef<AdapterBase> mCachedCapabilitiesAdapter = nullptr;
    PhysicalDeviceSurfaceCapabilities mCachedCapabilities;
};

// static
Ref<Surface> Surface::MakeError(InstanceBase* instance) {
    return AcquireRef(new Surface(instance, ErrorMonad::kError));
}

Surface::Surface(InstanceBase* instance, ErrorTag tag) : ErrorMonad(tag), mInstance(instance) {}

Surface::Surface(InstanceBase* instance, const UnpackedPtr<SurfaceDescriptor>& descriptor)
    : ErrorMonad(),
      mInstance(instance),
      mCapabilityCache(std::make_unique<AdapterSurfaceCapCache>()) {
    if (descriptor->label != nullptr && strlen(descriptor->label) != 0) {
        mLabel = descriptor->label;
    }

    // Type is validated in validation, otherwise this may crash with an assert failure.
    wgpu::SType type = descriptor
                           .ValidateBranches<Branch<SurfaceDescriptorFromAndroidNativeWindow>,
                                             Branch<SurfaceDescriptorFromMetalLayer>,
                                             Branch<SurfaceDescriptorFromWindowsHWND>,
                                             Branch<SurfaceDescriptorFromWindowsCoreWindow>,
                                             Branch<SurfaceDescriptorFromWindowsSwapChainPanel>,
                                             Branch<SurfaceDescriptorFromXlibWindow>,
                                             Branch<SurfaceDescriptorFromWaylandSurface>>()
                           .AcquireSuccess();
    switch (type) {
        case wgpu::SType::SurfaceDescriptorFromAndroidNativeWindow: {
            auto* subDesc = descriptor.Get<SurfaceDescriptorFromAndroidNativeWindow>();
            mType = Type::AndroidWindow;
            mAndroidNativeWindow = subDesc->window;
            break;
        }
        case wgpu::SType::SurfaceDescriptorFromMetalLayer: {
            auto* subDesc = descriptor.Get<SurfaceDescriptorFromMetalLayer>();
            mType = Type::MetalLayer;
            mMetalLayer = subDesc->layer;
            break;
        }
        case wgpu::SType::SurfaceDescriptorFromWindowsHWND: {
            auto* subDesc = descriptor.Get<SurfaceDescriptorFromWindowsHWND>();
            mType = Type::WindowsHWND;
            mHInstance = subDesc->hinstance;
            mHWND = subDesc->hwnd;
            break;
        }
#if defined(DAWN_USE_WINDOWS_UI)
        case wgpu::SType::SurfaceDescriptorFromWindowsCoreWindow: {
            auto* subDesc = descriptor.Get<SurfaceDescriptorFromWindowsCoreWindow>();
            mType = Type::WindowsCoreWindow;
            mCoreWindow = static_cast<IUnknown*>(subDesc->coreWindow);
            break;
        }
        case wgpu::SType::SurfaceDescriptorFromWindowsSwapChainPanel: {
            auto* subDesc = descriptor.Get<SurfaceDescriptorFromWindowsSwapChainPanel>();
            mType = Type::WindowsSwapChainPanel;
            mSwapChainPanel = static_cast<IUnknown*>(subDesc->swapChainPanel);
            break;
        }
#endif  // defined(DAWN_USE_WINDOWS_UI)
        case wgpu::SType::SurfaceDescriptorFromWaylandSurface: {
            auto* subDesc = descriptor.Get<SurfaceDescriptorFromWaylandSurface>();
            mType = Type::WaylandSurface;
            mWaylandDisplay = subDesc->display;
            mWaylandSurface = subDesc->surface;
            break;
        }
        case wgpu::SType::SurfaceDescriptorFromXlibWindow: {
            auto* subDesc = descriptor.Get<SurfaceDescriptorFromXlibWindow>();
            mType = Type::XlibWindow;
            mXDisplay = subDesc->display;
            mXWindow = subDesc->window;
            break;
        }
        default:
            DAWN_UNREACHABLE();
    }
}

Surface::~Surface() {
    if (mSwapChain != nullptr) {
        DAWN_UNUSED(mInstance->ConsumedError(Unconfigure()));
    }

    if (mRecycledSwapChain != nullptr) {
        mRecycledSwapChain->DetachFromSurface();
        mRecycledSwapChain->APIRelease();
        mRecycledSwapChain = nullptr;
    }
}

SwapChainBase* Surface::GetAttachedSwapChain() {
    DAWN_ASSERT(!IsError());
    DAWN_ASSERT(mIsSwapChainManagedBySurface == ManagesSwapChain::Unknown ||
                mIsSwapChainManagedBySurface == ManagesSwapChain::No);
    mIsSwapChainManagedBySurface = ManagesSwapChain::No;

    return mSwapChain.Get();
}
void Surface::SetAttachedSwapChain(SwapChainBase* swapChain) {
    DAWN_ASSERT(!IsError());
    DAWN_ASSERT(mIsSwapChainManagedBySurface == ManagesSwapChain::Unknown ||
                mIsSwapChainManagedBySurface == ManagesSwapChain::No);
    mIsSwapChainManagedBySurface = ManagesSwapChain::No;

    mSwapChain = swapChain;
}

InstanceBase* Surface::GetInstance() const {
    return mInstance.Get();
}

DeviceBase* Surface::GetCurrentDevice() const {
    return mCurrentDevice.Get();
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

MaybeError Surface::Configure(const SurfaceConfiguration* config) {
    DAWN_INVALID_IF(IsError(), "%s is invalid.", this);
    mCurrentDevice = config->device;  // next errors are routed to the new device
    DAWN_INVALID_IF(mIsSwapChainManagedBySurface == ManagesSwapChain::No,
                    "%s cannot be configured because it is used by legacy swapchain %s.", this,
                    mSwapChain.Get());

    mIsSwapChainManagedBySurface = ManagesSwapChain::Yes;

    DAWN_TRY(mCapabilityCache->WithAdapterCapabilities(
        GetCurrentDevice()->GetAdapter(), this,
        [&](const PhysicalDeviceSurfaceCapabilities& caps) -> MaybeError {
            return ValidateSurfaceConfiguration(GetCurrentDevice(), caps, config, this);
        }));

    Ref<SwapChainBase> previousSwapChain = mSwapChain;
    if (previousSwapChain == nullptr && mRecycledSwapChain != nullptr &&
        mRecycledSwapChain->GetDevice() == config->device) {
        std::swap(previousSwapChain, mRecycledSwapChain);
    }

    // Don't keep swap chains older than 1 call to Unconfigure
    if (mRecycledSwapChain != nullptr) {
        mRecycledSwapChain->DetachFromSurface();
        mRecycledSwapChain->APIRelease();
        mRecycledSwapChain = nullptr;
    }

    DAWN_TRY_ASSIGN(mSwapChain,
                    GetCurrentDevice()->CreateSwapChain(this, previousSwapChain.Get(), config));

    // NB: There must be no DAWN_TRY beyond this point, to ensure that if the
    // configuration failed, the previous swap chain remains valid.

    if (previousSwapChain != nullptr) {
        previousSwapChain->DetachFromSurface();
        previousSwapChain->APIRelease();
    }

    mSwapChain->SetIsAttached();

    return {};
}

MaybeError Surface::Unconfigure() {
    DAWN_INVALID_IF(IsError(), "%s is invalid.", this);
    DAWN_INVALID_IF(!mSwapChain.Get(), "%s is not configured.", this);

    if (mSwapChain != nullptr) {
        if (mIsSwapChainManagedBySurface == ManagesSwapChain::Yes) {
            if (mRecycledSwapChain != nullptr) {
                mRecycledSwapChain->DetachFromSurface();
                mRecycledSwapChain->APIRelease();
                mRecycledSwapChain = nullptr;
            }
            mRecycledSwapChain = mSwapChain;
        } else {
            mSwapChain->DetachFromSurface();
        }
        mSwapChain = nullptr;
    }

    return {};
}

MaybeError Surface::GetCapabilities(AdapterBase* adapter, SurfaceCapabilities* capabilities) const {
    DAWN_INVALID_IF(IsError(), "%s is invalid.", this);

    DAWN_TRY(mCapabilityCache->WithAdapterCapabilities(
        adapter, this,
        [&capabilities](const PhysicalDeviceSurfaceCapabilities& caps) -> MaybeError {
            capabilities->nextInChain = nullptr;
            DAWN_TRY(utils::AllocateApiSeqFromStdVector(capabilities->formats,
                                                        capabilities->formatCount, caps.formats));
            DAWN_TRY(utils::AllocateApiSeqFromStdVector(
                capabilities->presentModes, capabilities->presentModeCount, caps.presentModes));
            DAWN_TRY(utils::AllocateApiSeqFromStdVector(
                capabilities->alphaModes, capabilities->alphaModeCount, caps.alphaModes));
            return {};
        }));

    return {};
}

void APISurfaceCapabilitiesFreeMembers(WGPUSurfaceCapabilities capabilities) {
    utils::FreeApiSeq(capabilities.formats, capabilities.formatCount);
    utils::FreeApiSeq(capabilities.presentModes, capabilities.presentModeCount);
    utils::FreeApiSeq(capabilities.alphaModes, capabilities.alphaModeCount);
}

MaybeError Surface::GetCurrentTexture(SurfaceTexture* surfaceTexture) const {
    DAWN_INVALID_IF(IsError(), "%s is invalid.", this);
    DAWN_INVALID_IF(!mSwapChain.Get(), "%s is not configured.", this);

    DAWN_TRY_ASSIGN(*surfaceTexture, mSwapChain->GetCurrentTexture());

    return {};
}

ResultOrError<wgpu::TextureFormat> Surface::GetPreferredFormat(AdapterBase* adapter) const {
    wgpu::TextureFormat format = wgpu::TextureFormat::Undefined;

    DAWN_TRY(mCapabilityCache->WithAdapterCapabilities(
        adapter, this, [&](const PhysicalDeviceSurfaceCapabilities& caps) -> MaybeError {
            DAWN_INVALID_IF(caps.formats.empty(), "No format is supported by %s for %s.", adapter,
                            this);
            format = caps.formats.front();
            return {};
        }));

    return format;
}

MaybeError Surface::Present() {
    DAWN_INVALID_IF(IsError(), "%s is invalid.", this);
    DAWN_INVALID_IF(!mSwapChain.Get(), "%s is not configured.", this);
    mSwapChain->APIPresent();
    return {};
}

const std::string& Surface::GetLabel() const {
    return mLabel;
}

void Surface::APIConfigure(const SurfaceConfiguration* config) {
    MaybeError maybeError = Configure(config);
    if (!GetCurrentDevice()) {
        mInstance->ConsumedError(std::move(maybeError));
    } else {
        DAWN_UNUSED(GetCurrentDevice()->ConsumedError(std::move(maybeError),
                                                      "calling %s.Configure().", this));
    }
}

void Surface::APIGetCapabilities(AdapterBase* adapter, SurfaceCapabilities* capabilities) const {
    MaybeError maybeError = GetCapabilities(adapter, capabilities);
    if (!GetCurrentDevice()) {
        mInstance->ConsumedError(std::move(maybeError));
    } else {
        DAWN_UNUSED(GetCurrentDevice()->ConsumedError(std::move(maybeError),
                                                      "calling %s.Configure().", this));
    }
}

void Surface::APIGetCurrentTexture(SurfaceTexture* surfaceTexture) const {
    MaybeError maybeError = GetCurrentTexture(surfaceTexture);
    if (!GetCurrentDevice()) {
        if (mInstance->ConsumedError(std::move(maybeError))) {
            // TODO(dawn:2320) This is the closest status to "surface was not configured so there is
            // no associated device" but SurfaceTexture may change soon upstream.
            surfaceTexture->status = wgpu::SurfaceGetCurrentTextureStatus::DeviceLost;
            surfaceTexture->suboptimal = true;
            surfaceTexture->texture = nullptr;
        }
    } else {
        DAWN_UNUSED(GetCurrentDevice()->ConsumedError(std::move(maybeError)));
    }
}

wgpu::TextureFormat Surface::APIGetPreferredFormat(AdapterBase* adapter) const {
    ResultOrError<wgpu::TextureFormat> resultOrError = GetPreferredFormat(adapter);
    wgpu::TextureFormat format;
    if (!GetCurrentDevice()) {
        if (mInstance->ConsumedError(std::move(resultOrError), &format)) {
            return wgpu::TextureFormat::Undefined;
        }
    } else if (GetCurrentDevice()->ConsumedError(std::move(resultOrError), &format,
                                                 "calling %s.GetPreferredFormat(%s).", this,
                                                 adapter)) {
        return wgpu::TextureFormat::Undefined;
    }
    return format;
}

void Surface::APIPresent() {
    MaybeError maybeError = Present();
    if (!GetCurrentDevice()) {
        mInstance->ConsumedError(std::move(maybeError));
    } else {
        DAWN_UNUSED(GetCurrentDevice()->ConsumedError(std::move(maybeError)));
    }
}

void Surface::APIUnconfigure() {
    MaybeError maybeError = Unconfigure();
    if (!GetCurrentDevice()) {
        mInstance->ConsumedError(std::move(maybeError));
    } else {
        DAWN_UNUSED(GetCurrentDevice()->ConsumedError(std::move(maybeError)));
    }
}

void Surface::APISetLabel(const char* label) {
    mLabel = label;
}

}  // namespace dawn::native
