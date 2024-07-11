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
#include "dawn/common/WGSLFeatureMapping.h"
#include "dawn/native/CallbackTaskManager.h"
#include "dawn/native/ChainUtils.h"
#include "dawn/native/Device.h"
#include "dawn/native/ErrorData.h"
#include "dawn/native/Surface.h"
#include "dawn/native/Toggles.h"
#include "dawn/native/ValidationUtils_autogen.h"
#include "dawn/platform/DawnPlatform.h"
#include "partition_alloc/pointers/raw_ptr.h"
#include "tint/lang/wgsl/features/status.h"

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

wgpu::WGSLFeatureName ToWGPUFeature(tint::wgsl::LanguageFeature f) {
    switch (f) {
#define CASE(WgslName, WgpuName)                \
    case tint::wgsl::LanguageFeature::WgslName: \
        return wgpu::WGSLFeatureName::WgpuName;
        DAWN_FOREACH_WGSL_FEATURE(CASE)
#undef CASE
        case tint::wgsl::LanguageFeature::kUndefined:
            DAWN_UNREACHABLE();
    }
    DAWN_UNREACHABLE();
}

}  // anonymous namespace

wgpu::Status APIGetInstanceFeatures(InstanceFeatures* features) {
    if (features->nextInChain != nullptr) {
        return wgpu::Status::Error;
    }

    features->timedWaitAnyEnable = true;
    features->timedWaitAnyMaxCount = kTimedWaitAnyMaxCountDefault;
    return wgpu::Status::Success;
}

InstanceBase* APICreateInstance(const InstanceDescriptor* descriptor) {
    auto result = InstanceBase::Create(descriptor);
    if (result.IsError()) {
        dawn::ErrorLog() << result.AcquireError()->GetFormattedMessage();
        return nullptr;
    }
    return ReturnToAPI(result.AcquireSuccess());
}

// InstanceBase

struct InstanceBase::DeprecationWarnings {
    absl::flat_hash_set<std::string> emitted;
    uint64_t count = 0;
};

// static
ResultOrError<Ref<InstanceBase>> InstanceBase::Create(const InstanceDescriptor* descriptor) {
    static constexpr InstanceDescriptor kDefaultDesc = {};
    if (descriptor == nullptr) {
        descriptor = &kDefaultDesc;
    }

    UnpackedPtr<InstanceDescriptor> unpacked;
    DAWN_TRY_ASSIGN(unpacked, ValidateAndUnpack(descriptor));

    const DawnTogglesDescriptor* instanceTogglesDesc = unpacked.Get<DawnTogglesDescriptor>();

    // Set up the instance toggle state from toggles descriptor
    TogglesState instanceToggles =
        TogglesState::CreateFromTogglesDescriptor(instanceTogglesDesc, ToggleStage::Instance);
    // By default disable the AllowUnsafeAPIs instance toggle, it will be inherited to adapters
    // and devices created by this instance if not overriden.
    instanceToggles.Default(Toggle::AllowUnsafeAPIs, false);

    Ref<InstanceBase> instance = AcquireRef(new InstanceBase(instanceToggles));
    DAWN_TRY(instance->Initialize(unpacked));
    return instance;
}

InstanceBase::InstanceBase(const TogglesState& instanceToggles)
    : mToggles(instanceToggles), mDeprecationWarnings(std::make_unique<DeprecationWarnings>()) {}

InstanceBase::~InstanceBase() = default;

void InstanceBase::DeleteThis() {
    // Flush all remaining callback tasks on all devices and on the instance.
    absl::flat_hash_set<DeviceBase*> devices;
    do {
        devices.clear();
        mDevicesList.Use([&](auto deviceList) { devices.swap(*deviceList); });
        for (auto device : devices) {
            device->GetCallbackTaskManager()->HandleShutDown();
            do {
                device->GetCallbackTaskManager()->Flush();
            } while (!device->GetCallbackTaskManager()->IsEmpty());
        }
    } while (!devices.empty());

    mCallbackTaskManager->HandleShutDown();
    do {
        mCallbackTaskManager->Flush();
    } while (!mCallbackTaskManager->IsEmpty());

    RefCountedWithExternalCount::DeleteThis();
}

void InstanceBase::DisconnectDawnPlatform() {
    SetPlatform(nullptr);
}

void InstanceBase::WillDropLastExternalRef() {
    // Stop tracking events. See comment on ShutDown.
    mEventManager.ShutDown();
    mLoggingCallback = nullptr;
    mLoggingCallbackUserdata = nullptr;
}

// TODO(crbug.com/dawn/832): make the platform an initialization parameter of the instance.
MaybeError InstanceBase::Initialize(const UnpackedPtr<InstanceDescriptor>& descriptor) {
    // Initialize the platform to the default for now.
    mDefaultPlatform = std::make_unique<dawn::platform::Platform>();
    SetPlatform(mDefaultPlatform.get());

    // Process DawnInstanceDescriptor
    if (const auto* dawnDesc = descriptor.Get<DawnInstanceDescriptor>()) {
        for (uint32_t i = 0; i < dawnDesc->additionalRuntimeSearchPathsCount; ++i) {
            mRuntimeSearchPaths.push_back(dawnDesc->additionalRuntimeSearchPaths[i]);
        }
        SetPlatform(dawnDesc->platform);

        mBackendValidationLevel = dawnDesc->backendValidationLevel;
        mBeginCaptureOnStartup = dawnDesc->beginCaptureOnStartup;

        mLoggingCallback = dawnDesc->loggingCallback;
        mLoggingCallbackUserdata = dawnDesc->loggingCallbackUserdata;
    }

    if (!mLoggingCallback) {
        mLoggingCallback = [](WGPULoggingType type, char const* message, void*) {
            switch (static_cast<wgpu::LoggingType>(type)) {
                case wgpu::LoggingType::Verbose:
                    dawn::DebugLog() << message;
                    break;
                case wgpu::LoggingType::Info:
                    dawn::InfoLog() << message;
                    break;
                case wgpu::LoggingType::Warning:
                    dawn::WarningLog() << message;
                    break;
                case wgpu::LoggingType::Error:
                    dawn::ErrorLog() << message;
                    break;
            }
        };
        mLoggingCallbackUserdata = nullptr;
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
    DAWN_TRY(mEventManager.Initialize(descriptor));
    GatherWGSLFeatures(descriptor.Get<DawnWGSLBlocklist>());

    return {};
}

void InstanceBase::APIRequestAdapter(const RequestAdapterOptions* options,
                                     WGPURequestAdapterCallback callback,
                                     void* userdata) {
    APIRequestAdapterF(
        options, RequestAdapterCallbackInfo{nullptr, wgpu::CallbackMode::AllowSpontaneous, callback,
                                            userdata});
}

Future InstanceBase::APIRequestAdapterF(const RequestAdapterOptions* options,
                                        const RequestAdapterCallbackInfo& callbackInfo) {
    return APIRequestAdapter2(
        options, {ToAPI(callbackInfo.nextInChain), ToAPI(callbackInfo.mode),
                  [](WGPURequestAdapterStatus status, WGPUAdapter adapter, char const* message,
                     void* callback, void* userdata) {
                      auto cb = reinterpret_cast<WGPURequestAdapterCallback>(callback);
                      cb(status, adapter, message, userdata);
                  },
                  reinterpret_cast<void*>(callbackInfo.callback), callbackInfo.userdata});
}

Future InstanceBase::APIRequestAdapter2(const RequestAdapterOptions* options,
                                        const WGPURequestAdapterCallbackInfo2& callbackInfo) {
    struct RequestAdapterEvent final : public EventManager::TrackedEvent {
        WGPURequestAdapterCallback2 mCallback;
        raw_ptr<void> mUserdata1;
        raw_ptr<void> mUserdata2;
        Ref<AdapterBase> mAdapter;

        RequestAdapterEvent(const WGPURequestAdapterCallbackInfo2& callbackInfo,
                            Ref<AdapterBase> adapter)
            : TrackedEvent(static_cast<wgpu::CallbackMode>(callbackInfo.mode),
                           TrackedEvent::Completed{}),
              mCallback(callbackInfo.callback),
              mUserdata1(callbackInfo.userdata1),
              mUserdata2(callbackInfo.userdata2),
              mAdapter(std::move(adapter)) {}

        ~RequestAdapterEvent() override { EnsureComplete(EventCompletionType::Shutdown); }

        void Complete(EventCompletionType completionType) override {
            if (completionType == EventCompletionType::Shutdown) {
                mCallback(WGPURequestAdapterStatus_InstanceDropped, nullptr, nullptr,
                          mUserdata1.ExtractAsDangling(), mUserdata2.ExtractAsDangling());
                return;
            }

            WGPUAdapter adapter = ToAPI(ReturnToAPI(std::move(mAdapter)));
            if (adapter == nullptr) {
                mCallback(WGPURequestAdapterStatus_Unavailable, nullptr, "No supported adapters",
                          mUserdata1.ExtractAsDangling(), mUserdata2.ExtractAsDangling());
            } else {
                mCallback(WGPURequestAdapterStatus_Success, adapter, nullptr,
                          mUserdata1.ExtractAsDangling(), mUserdata2.ExtractAsDangling());
            }
        }
    };

    static constexpr RequestAdapterOptions kDefaultOptions = {};
    if (options == nullptr) {
        options = &kDefaultOptions;
    }
    auto adapters = EnumerateAdapters(options);

    FutureID futureID = GetEventManager()->TrackEvent(AcquireRef(new RequestAdapterEvent(
        callbackInfo, adapters.empty() ? nullptr : std::move(adapters[0]))));
    return {futureID};
}

Ref<AdapterBase> InstanceBase::CreateAdapter(Ref<PhysicalDeviceBase> physicalDevice,
                                             FeatureLevel featureLevel,
                                             const DawnTogglesDescriptor* requiredAdapterToggles,
                                             wgpu::PowerPreference powerPreference) {
    // Set up toggles state for default adapter from given toggles descriptor and inherit from
    // instance toggles.
    TogglesState adapterToggles =
        TogglesState::CreateFromTogglesDescriptor(requiredAdapterToggles, ToggleStage::Adapter);
    adapterToggles.InheritFrom(mToggles);
    // Set up forced and default adapter toggles for selected physical device.
    physicalDevice->SetupBackendAdapterToggles(GetPlatform(), &adapterToggles);

    return AcquireRef(new AdapterBase(this, std::move(physicalDevice), featureLevel, adapterToggles,
                                      powerPreference));
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

std::vector<Ref<AdapterBase>> InstanceBase::EnumerateAdapters(
    const RequestAdapterOptions* options) {
    static constexpr RequestAdapterOptions kDefaultOptions = {};
    if (options == nullptr) {
        // Default path that returns all WebGPU core adapters on the system with default
        // toggles.
        return EnumerateAdapters(&kDefaultOptions);
    }

    UnpackedPtr<RequestAdapterOptions> unpacked = Unpack(options);
    auto* togglesDesc = unpacked.Get<DawnTogglesDescriptor>();

    FeatureLevel featureLevel =
        options->compatibilityMode ? FeatureLevel::Compatibility : FeatureLevel::Core;
    std::vector<Ref<AdapterBase>> adapters;
    for (const auto& physicalDevice : EnumeratePhysicalDevices(unpacked)) {
        DAWN_ASSERT(physicalDevice->SupportsFeatureLevel(featureLevel));
        adapters.push_back(
            CreateAdapter(physicalDevice, featureLevel, togglesDesc, unpacked->powerPreference));
    }

    if (options->backendType == wgpu::BackendType::D3D11 ||
        options->backendType == wgpu::BackendType::D3D12) {
        // If a D3D backend was requested, the order of the adapters returned by DXGI should be
        // preserved instead of sorting by whether they are integrated vs. discrete. DXGI
        // returns the correct order based on system settings and configuration.
        return adapters;
    }
    return SortAdapters(std::move(adapters), options);
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
    const UnpackedPtr<RequestAdapterOptions>& options) {
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

bool InstanceBase::ConsumedErrorAndWarnOnce(MaybeError maybeErr) {
    if (!maybeErr.IsError()) {
        return false;
    }
    std::string message = maybeErr.AcquireError()->GetFormattedMessage();
    if (mWarningMessages.insert(message).second && mLoggingCallback) {
        mLoggingCallback(WGPULoggingType_Warning, message.c_str(), mLoggingCallbackUserdata);
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

bool InstanceBase::IsBeginCaptureOnStartupEnabled() const {
    return mBeginCaptureOnStartup;
}

void InstanceBase::SetPlatform(dawn::platform::Platform* platform) {
    if (platform == nullptr) {
        mPlatform = mDefaultPlatform.get();
    } else {
        mPlatform = platform;
    }
}

void InstanceBase::SetPlatformForTesting(dawn::platform::Platform* platform) {
    SetPlatform(platform);
}

dawn::platform::Platform* InstanceBase::GetPlatform() {
    return mPlatform;
}

uint64_t InstanceBase::GetDeprecationWarningCountForTesting() {
    return mDeprecationWarnings->count;
}

void InstanceBase::EmitDeprecationWarning(const std::string& message) {
    mDeprecationWarnings->count++;
    if (mDeprecationWarnings->emitted.insert(message).second) {
        dawn::WarningLog() << message;
    }
}

uint64_t InstanceBase::GetDeviceCountForTesting() const {
    return mDevicesList.Use([](auto deviceList) { return deviceList->size(); });
}

void InstanceBase::AddDevice(DeviceBase* device) {
    mDevicesList.Use([&](auto deviceList) { deviceList->insert(device); });
}

void InstanceBase::RemoveDevice(DeviceBase* device) {
    mDevicesList.Use([&](auto deviceList) { deviceList->erase(device); });
}

bool InstanceBase::ProcessEvents() {
    std::vector<Ref<DeviceBase>> devices;
    mDevicesList.Use([&](auto deviceList) {
        for (auto device : *deviceList) {
            devices.push_back(device);
        }
    });

    bool processedEvents = false;
    for (auto device : devices) {
        processedEvents |= device->APITick();
    }

    mCallbackTaskManager->Flush();
    processedEvents |= mEventManager.ProcessPollEvents();
    return processedEvents;
}

void InstanceBase::APIProcessEvents() {
    ProcessEvents();
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

void InstanceBase::ConsumeError(std::unique_ptr<ErrorData> error,
                                InternalErrorType additionalAllowedErrors) {
    // Note: `additionalAllowedErrors` is ignored. The instance considers every type of error to be
    // an error that is logged.
    DAWN_ASSERT(error != nullptr);
    if (mLoggingCallback) {
        std::string messageStr = error->GetFormattedMessage();
        mLoggingCallback(WGPULoggingType_Error, messageStr.c_str(), mLoggingCallbackUserdata);
    }
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
    UnpackedPtr<SurfaceDescriptor> unpacked;
    if (ConsumedError(ValidateSurfaceDescriptor(this, descriptor), &unpacked)) {
        return ReturnToAPI(Surface::MakeError(this));
    }

    return ReturnToAPI(AcquireRef(new Surface(this, unpacked)));
}

const absl::flat_hash_set<tint::wgsl::LanguageFeature>&
InstanceBase::GetAllowedWGSLLanguageFeatures() const {
    return mTintLanguageFeatures;
}

void InstanceBase::GatherWGSLFeatures(const DawnWGSLBlocklist* wgslBlocklist) {
    for (auto wgslFeature : tint::wgsl::kAllLanguageFeatures) {
        // Skip over testing features if we don't have the toggle to expose them.
        if (!mToggles.IsEnabled(Toggle::ExposeWGSLTestingFeatures)) {
            switch (wgslFeature) {
                case tint::wgsl::LanguageFeature::kChromiumTestingUnimplemented:
                case tint::wgsl::LanguageFeature::kChromiumTestingUnsafeExperimental:
                case tint::wgsl::LanguageFeature::kChromiumTestingExperimental:
                case tint::wgsl::LanguageFeature::kChromiumTestingShippedWithKillswitch:
                case tint::wgsl::LanguageFeature::kChromiumTestingShipped:
                    continue;
                default:
                    break;
            }
        }

        // Expose the feature depending on its status and allow_unsafe_apis.
        bool enable = false;
        switch (tint::wgsl::GetLanguageFeatureStatus(wgslFeature)) {
            case tint::wgsl::FeatureStatus::kUnknown:
            case tint::wgsl::FeatureStatus::kUnimplemented:
                enable = false;
                break;

            case tint::wgsl::FeatureStatus::kUnsafeExperimental:
                enable = mToggles.IsEnabled(Toggle::AllowUnsafeAPIs);
                break;
            case tint::wgsl::FeatureStatus::kExperimental:
                enable = mToggles.IsEnabled(Toggle::AllowUnsafeAPIs) ||
                         mToggles.IsEnabled(Toggle::ExposeWGSLExperimentalFeatures);
                break;

            case tint::wgsl::FeatureStatus::kShippedWithKillswitch:
            case tint::wgsl::FeatureStatus::kShipped:
                enable = true;
                break;
        }

        if (enable && wgslFeature != tint::wgsl::LanguageFeature::kUndefined) {
            mWGSLFeatures.emplace(ToWGPUFeature(wgslFeature));
            mTintLanguageFeatures.emplace(wgslFeature);
        }
    }

    // Remove blocklisted features.
    if (wgslBlocklist != nullptr) {
        for (size_t i = 0; i < wgslBlocklist->blocklistedFeatureCount; i++) {
            const char* name = wgslBlocklist->blocklistedFeatures[i];
            tint::wgsl::LanguageFeature tintFeature = tint::wgsl::ParseLanguageFeature(name);
            if (tintFeature == tint::wgsl::LanguageFeature::kUndefined) {
                // Ignore unknown features in the blocklist.
                continue;
            }
            mTintLanguageFeatures.erase(tintFeature);
            mWGSLFeatures.erase(ToWGPUFeature(tintFeature));
        }
    }
}

bool InstanceBase::APIHasWGSLLanguageFeature(wgpu::WGSLFeatureName feature) const {
    return mWGSLFeatures.contains(feature);
}

size_t InstanceBase::APIEnumerateWGSLLanguageFeatures(wgpu::WGSLFeatureName* features) const {
    if (features != nullptr) {
        for (wgpu::WGSLFeatureName f : mWGSLFeatures) {
            *features = f;
            ++features;
        }
    }
    return mWGSLFeatures.size();
}

}  // namespace dawn::native
