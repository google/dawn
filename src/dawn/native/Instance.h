// Copyright 2018 The Dawn Authors
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

#ifndef SRC_DAWN_NATIVE_INSTANCE_H_
#define SRC_DAWN_NATIVE_INSTANCE_H_

#include <array>
#include <memory>
#include <mutex>
#include <set>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "dawn/common/RefCounted.h"
#include "dawn/common/ityp_array.h"
#include "dawn/common/ityp_bitset.h"
#include "dawn/native/Adapter.h"
#include "dawn/native/BackendConnection.h"
#include "dawn/native/BlobCache.h"
#include "dawn/native/Features.h"
#include "dawn/native/RefCountedWithExternalCount.h"
#include "dawn/native/Toggles.h"
#include "dawn/native/dawn_platform.h"

namespace dawn::platform {
class Platform;
}  // namespace dawn::platform

namespace dawn::native {

class CallbackTaskManager;
class DeviceBase;
class Surface;
class XlibXcbFunctions;

using BackendsBitset = ityp::bitset<wgpu::BackendType, kEnumCount<wgpu::BackendType>>;
using BackendsArray = ityp::
    array<wgpu::BackendType, std::unique_ptr<BackendConnection>, kEnumCount<wgpu::BackendType>>;

InstanceBase* APICreateInstance(const InstanceDescriptor* descriptor);

// This is called InstanceBase for consistency across the frontend, even if the backends don't
// specialize this class.
class InstanceBase final : public RefCountedWithExternalCount {
  public:
    static Ref<InstanceBase> Create(const InstanceDescriptor* descriptor = nullptr);

    void APIRequestAdapter(const RequestAdapterOptions* options,
                           WGPURequestAdapterCallback callback,
                           void* userdata);

    // Deprecated: Discover physical devices and save them on the instance.
    void DiscoverDefaultPhysicalDevices();
    bool DiscoverPhysicalDevices(const PhysicalDeviceDiscoveryOptionsBase* options);
    // Return adapters created on physical device discovered by the instance.
    std::vector<Ref<AdapterBase>> GetAdapters() const;

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

    // Used to query the details of a toggle. Return nullptr if toggleName is not a valid name
    // of a toggle supported in Dawn.
    const ToggleInfo* GetToggleInfo(const char* toggleName);
    Toggle ToggleNameToEnum(const char* toggleName);

    // Used to query the details of an feature. Return nullptr if featureName is not a valid
    // name of an feature supported in Dawn.
    const FeatureInfo* GetFeatureInfo(wgpu::FeatureName feature);

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
    BlobCache* GetBlobCache(bool enabled = true);

    uint64_t GetDeviceCountForTesting() const;
    void AddDevice(DeviceBase* device);
    void RemoveDevice(DeviceBase* device);

    const std::vector<std::string>& GetRuntimeSearchPaths() const;

    const Ref<CallbackTaskManager>& GetCallbackTaskManager() const;

    // Get backend-independent libraries that need to be loaded dynamically.
    const XlibXcbFunctions* GetOrCreateXlibXcbFunctions();

    // Dawn API
    Surface* APICreateSurface(const SurfaceDescriptor* descriptor);
    bool APIProcessEvents();

  private:
    explicit InstanceBase(const TogglesState& instanceToggles);
    ~InstanceBase() override;

    void WillDropLastExternalRef() override;

    InstanceBase(const InstanceBase& other) = delete;
    InstanceBase& operator=(const InstanceBase& other) = delete;

    MaybeError Initialize(const InstanceDescriptor* descriptor);
    void SetPlatform(dawn::platform::Platform* platform);

    // Lazily creates connections to all backends that have been compiled.
    void EnsureBackendConnection(wgpu::BackendType backendType);

    // Deprecated: Discover physical devices with options, and save them on the instance.
    void DeprecatedDiscoverPhysicalDevices(const RequestAdapterOptions* options);
    // Enumerate physical devices according to options and return them.
    std::vector<Ref<PhysicalDeviceBase>> EnumeratePhysicalDevices(
        const RequestAdapterOptions* options);

    void ConsumeError(std::unique_ptr<ErrorData> error);

    std::unordered_set<std::string> warningMessages;

    std::vector<std::string> mRuntimeSearchPaths;

    BackendsBitset mBackendsConnected;

    bool mBeginCaptureOnStartup = false;
    bool mEnableAdapterBlocklist = false;
    BackendValidationLevel mBackendValidationLevel = BackendValidationLevel::Disabled;

    dawn::platform::Platform* mPlatform = nullptr;
    std::unique_ptr<dawn::platform::Platform> mDefaultPlatform;
    std::unique_ptr<BlobCache> mBlobCache;
    BlobCache mPassthroughBlobCache;

    BackendsArray mBackends;
    std::vector<Ref<PhysicalDeviceBase>> mDeprecatedPhysicalDevices;
    bool mDeprecatedDiscoveredDefaultPhysicalDevices = false;

    TogglesState mToggles;

    FeaturesInfo mFeaturesInfo;
    TogglesInfo mTogglesInfo;

#if defined(DAWN_USE_X11)
    std::unique_ptr<XlibXcbFunctions> mXlibXcbFunctions;
#endif  // defined(DAWN_USE_X11)

    Ref<CallbackTaskManager> mCallbackTaskManager;

    std::set<DeviceBase*> mDevicesList;
    mutable std::mutex mDevicesListMutex;
};

}  // namespace dawn::native

#endif  // SRC_DAWN_NATIVE_INSTANCE_H_
