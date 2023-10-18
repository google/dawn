// Copyright 2018 The Dawn & Tint Authors
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

#include "dawn/native/Instance.h"

#include <utility>

#include "dawn/common/Assert.h"
#include "dawn/common/FutureUtils.h"
#include "dawn/common/GPUInfo.h"
#include "dawn/common/Log.h"
#include "dawn/common/SystemUtils.h"
#include "dawn/native/CallbackTaskManager.h"
#include "dawn/native/ChainUtils.h"
#include "dawn/native/Device.h"
#include "dawn/native/ErrorData.h"
#include "dawn/native/Surface.h"
#include "dawn/native/Toggles.h"
#include "dawn/native/ValidationUtils_autogen.h"
#include "dawn/platform/DawnPlatform.h"

// For SwiftShader fallback
#if defined(DAWN_ENABLE_BACKEND_VULKAN)
#include "dawn/native/VulkanBackend.h"
#endif  // defined(DAWN_ENABLE_BACKEND_VULKAN)

#if defined(DAWN_ENABLE_BACKEND_D3D11) || defined(DAWN_ENABLE_BACKEND_D3D12)
#include "dawn/native/D3DBackend.h"
#include "dawn/native/d3d/BackendD3D.h"
#include "dawn/native/d3d/D3DError.h"
#endif  // defined(DAWN_ENABLE_BACKEND_D3D11) || defined(DAWN_ENABLE_BACKEND_D3D12)

#if defined(DAWN_ENABLE_BACKEND_OPENGL)
#include "dawn/native/OpenGLBackend.h"
#endif  // defined(DAWN_ENABLE_BACKEND_OPENGL)

#if defined(DAWN_USE_X11)
#include "dawn/native/X11Functions.h"
#endif  // defined(DAWN_USE_X11)

namespace dawn::native {

// Forward definitions of each backend's "Connect" function that creates new BackendConnection.
// Conditionally compiled declarations are used to avoid using static constructors instead.
#if defined(DAWN_ENABLE_BACKEND_D3D11)
namespace d3d11 {
BackendConnection* Connect(InstanceBase* instance);
}
#endif  // defined(DAWN_ENABLE_BACKEND_D3D11)
#if defined(DAWN_ENABLE_BACKEND_D3D12)
namespace d3d12 {
BackendConnection* Connect(InstanceBase* instance);
}
#endif  // defined(DAWN_ENABLE_BACKEND_D3D12)
#if defined(DAWN_ENABLE_BACKEND_METAL)
namespace metal {
BackendConnection* Connect(InstanceBase* instance);
}
#endif  // defined(DAWN_ENABLE_BACKEND_METAL)
#if defined(DAWN_ENABLE_BACKEND_NULL)
namespace null {
BackendConnection* Connect(InstanceBase* instance);
}
#endif  // defined(DAWN_ENABLE_BACKEND_NULL)
#if defined(DAWN_ENABLE_BACKEND_OPENGL)
namespace opengl {
BackendConnection* Connect(InstanceBase* instance, wgpu::BackendType backendType);
}
#endif  // defined(DAWN_ENABLE_BACKEND_OPENGL)
#if defined(DAWN_ENABLE_BACKEND_VULKAN)
namespace vulkan {
BackendConnection* Connect(InstanceBase* instance);
}
#endif  // defined(DAWN_ENABLE_BACKEND_VULKAN)

namespace {

dawn::platform::CachingInterface* GetCachingInterface(dawn::platform::Platform* platform) {
    if (platform != nullptr) {
        return platform->GetCachingInterface();
    }
    return nullptr;
}

}  // anonymous namespace

wgpu::Bool APIGetInstanceFeatures(InstanceFeatures* features) {
    if (features->nextInChain != nullptr) {
        return false;
    }

    features->timedWaitAnyEnable = true;
    features->timedWaitAnyMaxCount = kTimedWaitAnyMaxCountDefault;
    return true;
}

InstanceBase* APICreateInstance(const InstanceDescriptor* descriptor) {
    return InstanceBase::Create(descriptor).Detach();
}

// InstanceBase

// static
Ref<InstanceBase> InstanceBase::Create(const InstanceDescriptor* descriptor) {
    static constexpr InstanceDescriptor kDefaultDesc = {};
    if (descriptor == nullptr) {
        descriptor = &kDefaultDesc;
    }

    const DawnTogglesDescriptor* instanceTogglesDesc = nullptr;
    FindInChain(descriptor->nextInChain, &instanceTogglesDesc);

    // Set up the instance toggle state from toggles descriptor
    TogglesState instanceToggles =
        TogglesState::CreateFromTogglesDescriptor(instanceTogglesDesc, ToggleStage::Instance);
    // By default disable the AllowUnsafeAPIs instance toggle, it will be inherited to adapters
    // and devices created by this instance if not overriden.
    instanceToggles.Default(Toggle::AllowUnsafeAPIs, false);

    Ref<InstanceBase> instance = AcquireRef(new InstanceBase(instanceToggles));
    if (instance->ConsumedError(instance->Initialize(descriptor))) {
        return nullptr;
    }
    return instance;
}

InstanceBase::InstanceBase(const TogglesState& instanceToggles) : mToggles(instanceToggles) {}

InstanceBase::~InstanceBase() = default;

void InstanceBase::WillDropLastExternalRef() {
    // InstanceBase uses RefCountedWithExternalCount to break refcycles.

    // Stop tracking events. See comment on ShutDown.
    mEventManager.ShutDown();

    // InstanceBase holds backends which hold Refs to PhysicalDeviceBases discovered, which hold
    // Refs back to the InstanceBase.
    // In order to break this cycle and prevent leaks, when the application drops the last external
    // ref and WillDropLastExternalRef is called, the instance clears out any member refs to
    // physical devices that hold back-refs to the instance - thus breaking any reference cycles.
    for (auto& backend : mBackends) {
        if (backend != nullptr) {
            backend->ClearPhysicalDevices();
        }
    }
}

// TODO(crbug.com/dawn/832): make the platform an initialization parameter of the instance.
MaybeError InstanceBase::Initialize(const InstanceDescriptor* descriptor) {
    DAWN_TRY(ValidateSTypes(descriptor->nextInChain, {{wgpu::SType::DawnInstanceDescriptor},
                                                      {wgpu::SType::DawnTogglesDescriptor}}));

    const DawnInstanceDescriptor* dawnDesc = nullptr;
    FindInChain(descriptor->nextInChain, &dawnDesc);
    if (dawnDesc != nullptr) {
        for (uint32_t i = 0; i < dawnDesc->additionalRuntimeSearchPathsCount; ++i) {
            mRuntimeSearchPaths.push_back(dawnDesc->additionalRuntimeSearchPaths[i]);
        }
    }
    // Default paths to search are next to the shared library, next to the executable, and
    // no path (just libvulkan.so).
    if (auto p = GetModuleDirectory()) {
        mRuntimeSearchPaths.push_back(std::move(*p));
    }
    if (auto p = GetExecutableDirectory()) {
        mRuntimeSearchPaths.push_back(std::move(*p));
    }
    mRuntimeSearchPaths.push_back("");

    mCallbackTaskManager = AcquireRef(new CallbackTaskManager());

    // Initialize the platform to the default for now.
    mDefaultPlatform = std::make_unique<dawn::platform::Platform>();
    SetPlatform(dawnDesc != nullptr ? dawnDesc->platform : mDefaultPlatform.get());

    DAWN_TRY(mEventManager.Initialize(descriptor));

    return {};
}

void InstanceBase::APIRequestAdapter(const RequestAdapterOptions* options,
                                     WGPURequestAdapterCallback callback,
                                     void* userdata) {
    static constexpr RequestAdapterOptions kDefaultOptions = {};
    if (options == nullptr) {
        options = &kDefaultOptions;
    }
    auto adapters = EnumerateAdapters(options);
    if (adapters.empty()) {
        callback(WGPURequestAdapterStatus_Unavailable, nullptr, "No supported adapters.", userdata);
    } else {
        callback(WGPURequestAdapterStatus_Success, ToAPI(adapters[0].Detach()), nullptr, userdata);
    }
}

Ref<AdapterBase> InstanceBase::CreateAdapter(Ref<PhysicalDeviceBase> physicalDevice,
                                             FeatureLevel featureLevel,
                                             const DawnTogglesDescriptor* requiredAdapterToggles,
                                             wgpu::PowerPreference powerPreference) const {
    // Set up toggles state for default adapter from given toggles descriptor and inherit from
    // instance toggles.
    TogglesState adapterToggles =
        TogglesState::CreateFromTogglesDescriptor(requiredAdapterToggles, ToggleStage::Adapter);
    adapterToggles.InheritFrom(mToggles);
    // Set up forced and default adapter toggles for selected physical device.
    physicalDevice->SetupBackendAdapterToggles(&adapterToggles);

    return AcquireRef(
        new AdapterBase(std::move(physicalDevice), featureLevel, adapterToggles, powerPreference));
}

const TogglesState& InstanceBase::GetTogglesState() const {
    return mToggles;
}

const ToggleInfo* InstanceBase::GetToggleInfo(const char* toggleName) {
    return mTogglesInfo.GetToggleInfo(toggleName);
}

Toggle InstanceBase::ToggleNameToEnum(const char* toggleName) {
    return mTogglesInfo.ToggleNameToEnum(toggleName);
}

const FeatureInfo* InstanceBase::GetFeatureInfo(wgpu::FeatureName feature) {
    Feature f = FromAPI(feature);
    if (f == Feature::InvalidEnum) {
        return nullptr;
    }
    return &kFeatureNameAndInfoList[f];
}

std::vector<Ref<AdapterBase>> InstanceBase::EnumerateAdapters(
    const RequestAdapterOptions* options) {
    if (options == nullptr) {
        // Default path that returns all WebGPU core adapters on the system with default toggles.
        RequestAdapterOptions defaultOptions = {};
        return EnumerateAdapters(&defaultOptions);
    }

    const DawnTogglesDescriptor* togglesDesc = nullptr;
    FindInChain(options->nextInChain, &togglesDesc);

    FeatureLevel featureLevel =
        options->compatibilityMode ? FeatureLevel::Compatibility : FeatureLevel::Core;
    std::vector<Ref<AdapterBase>> adapters;
    for (const auto& physicalDevice : EnumeratePhysicalDevices(options)) {
        DAWN_ASSERT(physicalDevice->SupportsFeatureLevel(featureLevel));
        adapters.push_back(
            CreateAdapter(physicalDevice, featureLevel, togglesDesc, options->powerPreference));
    }
    return SortAdapters(std::move(adapters), options);
}

size_t InstanceBase::GetPhysicalDeviceCountForTesting() const {
    size_t count = 0;
    for (auto& backend : mBackends) {
        if (backend != nullptr) {
            count += backend->GetPhysicalDeviceCountForTesting();
        }
    }
    return count;
}

BackendConnection* InstanceBase::GetBackendConnection(wgpu::BackendType backendType) {
    if (mBackendsTried[backendType]) {
        return mBackends[backendType].get();
    }

    auto Register = [this](BackendConnection* connection, wgpu::BackendType expectedType) {
        if (connection != nullptr) {
            DAWN_ASSERT(connection->GetType() == expectedType);
            DAWN_ASSERT(connection->GetInstance() == this);
            mBackends[connection->GetType()] = std::unique_ptr<BackendConnection>(connection);
        }
    };

    switch (backendType) {
#if defined(DAWN_ENABLE_BACKEND_NULL)
        case wgpu::BackendType::Null:
            Register(null::Connect(this), wgpu::BackendType::Null);
            break;
#endif  // defined(DAWN_ENABLE_BACKEND_NULL)

#if defined(DAWN_ENABLE_BACKEND_D3D11)
        case wgpu::BackendType::D3D11:
            Register(d3d11::Connect(this), wgpu::BackendType::D3D11);
            break;
#endif  // defined(DAWN_ENABLE_BACKEND_D3D11)

#if defined(DAWN_ENABLE_BACKEND_D3D12)
        case wgpu::BackendType::D3D12:
            Register(d3d12::Connect(this), wgpu::BackendType::D3D12);
            break;
#endif  // defined(DAWN_ENABLE_BACKEND_D3D12)

#if defined(DAWN_ENABLE_BACKEND_METAL)
        case wgpu::BackendType::Metal:
            Register(metal::Connect(this), wgpu::BackendType::Metal);
            break;
#endif  // defined(DAWN_ENABLE_BACKEND_METAL)

#if defined(DAWN_ENABLE_BACKEND_VULKAN)
        case wgpu::BackendType::Vulkan:
            Register(vulkan::Connect(this), wgpu::BackendType::Vulkan);
            break;
#endif  // defined(DAWN_ENABLE_BACKEND_VULKAN)

#if defined(DAWN_ENABLE_BACKEND_DESKTOP_GL)
        case wgpu::BackendType::OpenGL:
            Register(opengl::Connect(this, wgpu::BackendType::OpenGL), wgpu::BackendType::OpenGL);
            break;
#endif  // defined(DAWN_ENABLE_BACKEND_DESKTOP_GL)

#if defined(DAWN_ENABLE_BACKEND_OPENGLES)
        case wgpu::BackendType::OpenGLES:
            Register(opengl::Connect(this, wgpu::BackendType::OpenGLES),
                     wgpu::BackendType::OpenGLES);
            break;
#endif  // defined(DAWN_ENABLE_BACKEND_OPENGLES)

        default:
            break;
    }

    mBackendsTried.set(backendType);
    return mBackends[backendType].get();
}

std::vector<Ref<PhysicalDeviceBase>> InstanceBase::EnumeratePhysicalDevices(
    const RequestAdapterOptions* options) {
    DAWN_ASSERT(options);

    BackendsBitset backendsToFind;
    if (options->backendType != wgpu::BackendType::Undefined) {
        backendsToFind = {};
        if (!ConsumedErrorAndWarnOnce(ValidateBackendType(options->backendType))) {
            backendsToFind.set(options->backendType);
        }
    } else {
        backendsToFind.set();
    }

    std::vector<Ref<PhysicalDeviceBase>> discoveredPhysicalDevices;
    for (wgpu::BackendType b : IterateBitSet(backendsToFind)) {
        BackendConnection* backend = GetBackendConnection(b);

        if (backend != nullptr) {
            std::vector<Ref<PhysicalDeviceBase>> physicalDevices =
                mBackends[b]->DiscoverPhysicalDevices(options);
            discoveredPhysicalDevices.insert(discoveredPhysicalDevices.end(),
                                             physicalDevices.begin(), physicalDevices.end());
        }
    }
    return discoveredPhysicalDevices;
}

bool InstanceBase::ConsumedError(MaybeError maybeError) {
    if (maybeError.IsError()) {
        ConsumeError(maybeError.AcquireError());
        return true;
    }
    return false;
}

bool InstanceBase::ConsumedErrorAndWarnOnce(MaybeError maybeErr) {
    if (!maybeErr.IsError()) {
        return false;
    }
    std::string message = maybeErr.AcquireError()->GetFormattedMessage();
    if (warningMessages.insert(message).second) {
        dawn::WarningLog() << message;
    }
    return true;
}

bool InstanceBase::IsBackendValidationEnabled() const {
    return mBackendValidationLevel != BackendValidationLevel::Disabled;
}

void InstanceBase::SetBackendValidationLevel(BackendValidationLevel level) {
    mBackendValidationLevel = level;
}

BackendValidationLevel InstanceBase::GetBackendValidationLevel() const {
    return mBackendValidationLevel;
}

void InstanceBase::EnableBeginCaptureOnStartup(bool beginCaptureOnStartup) {
    mBeginCaptureOnStartup = beginCaptureOnStartup;
}

bool InstanceBase::IsBeginCaptureOnStartupEnabled() const {
    return mBeginCaptureOnStartup;
}

void InstanceBase::EnableAdapterBlocklist(bool enable) {
    mEnableAdapterBlocklist = enable;
}

bool InstanceBase::IsAdapterBlocklistEnabled() const {
    return mEnableAdapterBlocklist;
}

void InstanceBase::SetPlatform(dawn::platform::Platform* platform) {
    if (platform == nullptr) {
        mPlatform = mDefaultPlatform.get();
    } else {
        mPlatform = platform;
    }
    mBlobCache = std::make_unique<BlobCache>(GetCachingInterface(platform));
}

void InstanceBase::SetPlatformForTesting(dawn::platform::Platform* platform) {
    SetPlatform(platform);
}

dawn::platform::Platform* InstanceBase::GetPlatform() {
    return mPlatform;
}

BlobCache* InstanceBase::GetBlobCache(bool enabled) {
    if (enabled) {
        return mBlobCache.get();
    }
    return &mPassthroughBlobCache;
}

uint64_t InstanceBase::GetDeviceCountForTesting() const {
    std::lock_guard<std::mutex> lg(mDevicesListMutex);
    return mDevicesList.size();
}

void InstanceBase::AddDevice(DeviceBase* device) {
    std::lock_guard<std::mutex> lg(mDevicesListMutex);
    mDevicesList.insert(device);
}

void InstanceBase::RemoveDevice(DeviceBase* device) {
    std::lock_guard<std::mutex> lg(mDevicesListMutex);
    mDevicesList.erase(device);
}

void InstanceBase::APIProcessEvents() {
    std::vector<Ref<DeviceBase>> devices;
    {
        std::lock_guard<std::mutex> lg(mDevicesListMutex);
        for (auto device : mDevicesList) {
            devices.push_back(device);
        }
    }

    for (auto device : devices) {
        device->APITick();
    }

    mCallbackTaskManager->Flush();
    mEventManager.ProcessPollEvents();
}

wgpu::WaitStatus InstanceBase::APIWaitAny(size_t count,
                                          FutureWaitInfo* futures,
                                          uint64_t timeoutNS) {
    return mEventManager.WaitAny(count, futures, Nanoseconds(timeoutNS));
}

const std::vector<std::string>& InstanceBase::GetRuntimeSearchPaths() const {
    return mRuntimeSearchPaths;
}

const Ref<CallbackTaskManager>& InstanceBase::GetCallbackTaskManager() const {
    return mCallbackTaskManager;
}

EventManager* InstanceBase::GetEventManager() {
    return &mEventManager;
}

void InstanceBase::ConsumeError(std::unique_ptr<ErrorData> error) {
    DAWN_ASSERT(error != nullptr);
    dawn::ErrorLog() << error->GetFormattedMessage();
}

const X11Functions* InstanceBase::GetOrLoadX11Functions() {
#if defined(DAWN_USE_X11)
    if (mX11Functions == nullptr) {
        mX11Functions = std::make_unique<X11Functions>();
    }
    return mX11Functions.get();
#else
    DAWN_UNREACHABLE();
#endif  // defined(DAWN_USE_X11)
}

Surface* InstanceBase::APICreateSurface(const SurfaceDescriptor* descriptor) {
    if (ConsumedError(ValidateSurfaceDescriptor(this, descriptor))) {
        return Surface::MakeError(this);
    }

    return new Surface(this, descriptor);
}

}  // namespace dawn::native
