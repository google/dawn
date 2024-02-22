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

#ifndef SRC_DAWN_NATIVE_INSTANCE_H_
#define SRC_DAWN_NATIVE_INSTANCE_H_

#include <array>
#include <memory>
#include <mutex>
#include <string>
#include <unordered_set>
#include <vector>

#include "absl/container/flat_hash_set.h"
#include "dawn/common/MutexProtected.h"
#include "dawn/common/Ref.h"
#include "dawn/common/ityp_array.h"
#include "dawn/common/ityp_bitset.h"
#include "dawn/native/Adapter.h"
#include "dawn/native/BackendConnection.h"
#include "dawn/native/EventManager.h"
#include "dawn/native/Features.h"
#include "dawn/native/Forward.h"
#include "dawn/native/RefCountedWithExternalCount.h"
#include "dawn/native/Toggles.h"
#include "dawn/native/dawn_platform.h"
#include "partition_alloc/pointers/raw_ptr.h"
#include "tint/lang/wgsl/features/language_feature.h"

namespace dawn::platform {
class Platform;
}  // namespace dawn::platform

namespace dawn::native {

class AHBFunctions;
class CallbackTaskManager;
class DeviceBase;
class Surface;
class X11Functions;

using BackendsBitset = ityp::bitset<wgpu::BackendType, kEnumCount<wgpu::BackendType>>;
using BackendsArray = ityp::
    array<wgpu::BackendType, std::unique_ptr<BackendConnection>, kEnumCount<wgpu::BackendType>>;

wgpu::Bool APIGetInstanceFeatures(InstanceFeatures* features);
InstanceBase* APICreateInstance(const InstanceDescriptor* descriptor);

// This is called InstanceBase for consistency across the frontend, even if the backends don't
// specialize this class.
class InstanceBase final : public RefCountedWithExternalCount {
  public:
    static ResultOrError<Ref<InstanceBase>> Create(const InstanceDescriptor* descriptor = nullptr);

    void APIRequestAdapter(const RequestAdapterOptions* options,
                           WGPURequestAdapterCallback callback,
                           void* userdata);
    Future APIRequestAdapterF(const RequestAdapterOptions* options,
                              const RequestAdapterCallbackInfo& callbackInfo);

    // Discovers and returns a vector of adapters.
    // All systems adapters that can be found are returned if no options are passed.
    // Otherwise, returns adapters based on the `options`.
    std::vector<Ref<AdapterBase>> EnumerateAdapters(const RequestAdapterOptions* options = nullptr);

    size_t GetPhysicalDeviceCountForTesting() const;

    // Used to handle error that happen up to device creation.
    bool ConsumedError(MaybeError maybeError);

    template <typename T>
    bool ConsumedError(ResultOrError<T> resultOrError, T* result) {
        if (resultOrError.IsError()) {
            ConsumeError(resultOrError.AcquireError());
            return true;
        }
        *result = resultOrError.AcquireSuccess();
        return false;
    }

    // Consume an error and log its warning at most once. This is useful for
    // physical device creation errors that happen because the backend is not
    // supported or doesn't meet the required capabilities.
    bool ConsumedErrorAndWarnOnce(MaybeError maybeError);

    template <typename T>
    [[nodiscard]] bool ConsumedErrorAndWarnOnce(ResultOrError<T> resultOrError, T* result) {
        if (DAWN_UNLIKELY(resultOrError.IsError())) {
            return ConsumedErrorAndWarnOnce(resultOrError.AcquireError());
        }
        *result = resultOrError.AcquireSuccess();
        return false;
    }

    const TogglesState& GetTogglesState() const;
    const std::unordered_set<tint::wgsl::LanguageFeature>& GetAllowedWGSLLanguageFeatures() const;

    // Used to query the details of a toggle. Return nullptr if toggleName is not a valid name
    // of a toggle supported in Dawn.
    const ToggleInfo* GetToggleInfo(const char* toggleName);
    Toggle ToggleNameToEnum(const char* toggleName);

    // Used to query the details of an feature. Return nullptr if featureName is not a valid
    // name of an feature supported in Dawn.
    const FeatureInfo* GetFeatureInfo(wgpu::FeatureName feature);

    // TODO(dawn:2166): Move this method to PhysicalDevice to better detect that the backend
    // validation is actually enabled or not when a physical device is created. Sometimes it is
    // enabled externally via command line or environment variables.
    bool IsBackendValidationEnabled() const;
    void SetBackendValidationLevel(BackendValidationLevel level);
    BackendValidationLevel GetBackendValidationLevel() const;

    void EnableBeginCaptureOnStartup(bool beginCaptureOnStartup);
    bool IsBeginCaptureOnStartupEnabled() const;

    // TODO(crbug.com/dawn/1495): Move this to a Toggle, perhaps on RequestAdapterOptions
    // after Toggle refactor is complete.
    void EnableAdapterBlocklist(bool enable);
    bool IsAdapterBlocklistEnabled() const;

    // Testing only API that is NOT thread-safe.
    void SetPlatformForTesting(dawn::platform::Platform* platform);
    dawn::platform::Platform* GetPlatform();

    uint64_t GetDeviceCountForTesting() const;
    void AddDevice(DeviceBase* device);
    void RemoveDevice(DeviceBase* device);

    const std::vector<std::string>& GetRuntimeSearchPaths() const;

    const Ref<CallbackTaskManager>& GetCallbackTaskManager() const;
    EventManager* GetEventManager();

    // Get backend-independent libraries that need to be loaded dynamically.
    const X11Functions* GetOrLoadX11Functions();
    const AHBFunctions* GetOrLoadAHBFunctions();

    // TODO(dawn:752) Standardize webgpu.h to decide if we should return bool.
    //   Currently this is a backdoor for Chromium's process event loop.
    bool ProcessEvents();

    // Dawn API
    Surface* APICreateSurface(const SurfaceDescriptor* descriptor);
    void APIProcessEvents();
    [[nodiscard]] wgpu::WaitStatus APIWaitAny(size_t count,
                                              FutureWaitInfo* futures,
                                              uint64_t timeoutNS);
    bool APIHasWGSLLanguageFeature(wgpu::WGSLFeatureName feature) const;
    // Always writes the full list when features is not nullptr.
    // TODO(https://github.com/webgpu-native/webgpu-headers/issues/252): Add a count argument.
    size_t APIEnumerateWGSLLanguageFeatures(wgpu::WGSLFeatureName* features) const;

  private:
    explicit InstanceBase(const TogglesState& instanceToggles);
    ~InstanceBase() override;

    void DeleteThis() override;
    void WillDropLastExternalRef() override;

    InstanceBase(const InstanceBase& other) = delete;
    InstanceBase& operator=(const InstanceBase& other) = delete;

    MaybeError Initialize(const UnpackedPtr<InstanceDescriptor>& descriptor);
    void SetPlatform(dawn::platform::Platform* platform);

    // Lazily creates connections to all backends that have been compiled, may return null even for
    // compiled in backends.
    BackendConnection* GetBackendConnection(wgpu::BackendType backendType);

    // Enumerate physical devices according to options and return them.
    std::vector<Ref<PhysicalDeviceBase>> EnumeratePhysicalDevices(
        const UnpackedPtr<RequestAdapterOptions>& options);

    // Helper function that create adapter on given physical device handling required adapter
    // toggles descriptor.
    Ref<AdapterBase> CreateAdapter(Ref<PhysicalDeviceBase> physicalDevice,
                                   FeatureLevel featureLevel,
                                   const DawnTogglesDescriptor* requiredAdapterToggles,
                                   wgpu::PowerPreference powerPreference) const;

    void GatherWGSLFeatures(const DawnWGSLBlocklist* wgslBlocklist);
    void ConsumeError(std::unique_ptr<ErrorData> error);

    absl::flat_hash_set<std::string> mWarningMessages;

    std::vector<std::string> mRuntimeSearchPaths;

    bool mBeginCaptureOnStartup = false;
    bool mEnableAdapterBlocklist = false;
    BackendValidationLevel mBackendValidationLevel = BackendValidationLevel::Disabled;

    wgpu::LoggingCallback mLoggingCallback = nullptr;
    raw_ptr<void> mLoggingCallbackUserdata = nullptr;

    // TODO(https://crbug.com/dawn/2349): Investigate DanglingUntriaged in dawn/native.
    raw_ptr<dawn::platform::Platform, DanglingUntriaged> mPlatform = nullptr;
    std::unique_ptr<dawn::platform::Platform> mDefaultPlatform;

    BackendsArray mBackends;
    BackendsBitset mBackendsTried;

    TogglesState mToggles;
    TogglesInfo mTogglesInfo;

    absl::flat_hash_set<wgpu::WGSLFeatureName> mWGSLFeatures;
    // TODO(dawn:1513): Use absl::flat_hash_set after it is supported in Tint.
    std::unordered_set<tint::wgsl::LanguageFeature> mTintLanguageFeatures;

#if defined(DAWN_USE_X11)
    std::unique_ptr<X11Functions> mX11Functions;
#endif  // defined(DAWN_USE_X11)
#if DAWN_PLATFORM_IS(ANDROID)
    std::unique_ptr<AHBFunctions> mAHBFunctions;
#endif  // DAWN_PLATFORM_IS(ANDROID)

    Ref<CallbackTaskManager> mCallbackTaskManager;
    EventManager mEventManager;

    MutexProtected<absl::flat_hash_set<DeviceBase*>> mDevicesList;
};

}  // namespace dawn::native

#endif  // SRC_DAWN_NATIVE_INSTANCE_H_
