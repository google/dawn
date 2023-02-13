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
#include <string>
#include <unordered_map>
#include <vector>

#include "dawn/common/RefCounted.h"
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

class Surface;
class XlibXcbFunctions;

using BackendsBitset = ityp::bitset<wgpu::BackendType, kEnumCount<wgpu::BackendType>>;

InstanceBase* APICreateInstance(const InstanceDescriptor* descriptor);

// This is called InstanceBase for consistency across the frontend, even if the backends don't
// specialize this class.
class InstanceBase final : public RefCountedWithExternalCount {
  public:
    static Ref<InstanceBase> Create(const InstanceDescriptor* descriptor = nullptr);

    void APIRequestAdapter(const RequestAdapterOptions* options,
                           WGPURequestAdapterCallback callback,
                           void* userdata);

    void DiscoverDefaultAdapters();
    bool DiscoverAdapters(const AdapterDiscoveryOptionsBase* options);

    const std::vector<Ref<AdapterBase>>& GetAdapters() const;

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

    // TODO(dawn:1374): SetPlatform should become a private helper, and SetPlatformForTesting
    // will become the NOT thread-safe testing version exposed for special testing cases.
    void SetPlatform(dawn::platform::Platform* platform);
    void SetPlatformForTesting(dawn::platform::Platform* platform);
    dawn::platform::Platform* GetPlatform();
    BlobCache* GetBlobCache(bool enabled = true);

    uint64_t GetDeviceCountForTesting() const;
    void IncrementDeviceCountForTesting();
    void DecrementDeviceCountForTesting();

    const std::vector<std::string>& GetRuntimeSearchPaths() const;

    // Get backend-independent libraries that need to be loaded dynamically.
    const XlibXcbFunctions* GetOrCreateXlibXcbFunctions();

    // Dawn API
    Surface* APICreateSurface(const SurfaceDescriptor* descriptor);

  private:
    InstanceBase();
    ~InstanceBase() override;

    void WillDropLastExternalRef() override;

    InstanceBase(const InstanceBase& other) = delete;
    InstanceBase& operator=(const InstanceBase& other) = delete;

    MaybeError Initialize(const InstanceDescriptor* descriptor);

    // Lazily creates connections to all backends that have been compiled.
    void EnsureBackendConnection(wgpu::BackendType backendType);

    MaybeError DiscoverAdaptersInternal(const AdapterDiscoveryOptionsBase* options);

    ResultOrError<Ref<AdapterBase>> RequestAdapterInternal(const RequestAdapterOptions* options);

    void ConsumeError(std::unique_ptr<ErrorData> error);

    std::vector<std::string> mRuntimeSearchPaths;

    BackendsBitset mBackendsConnected;

    bool mDiscoveredDefaultAdapters = false;

    bool mBeginCaptureOnStartup = false;
    bool mEnableAdapterBlocklist = false;
    BackendValidationLevel mBackendValidationLevel = BackendValidationLevel::Disabled;

    dawn::platform::Platform* mPlatform = nullptr;
    std::unique_ptr<dawn::platform::Platform> mDefaultPlatform;
    std::unique_ptr<BlobCache> mBlobCache;
    BlobCache mPassthroughBlobCache;

    std::vector<std::unique_ptr<BackendConnection>> mBackends;
    std::vector<Ref<AdapterBase>> mAdapters;

    FeaturesInfo mFeaturesInfo;
    TogglesInfo mTogglesInfo;

#if defined(DAWN_USE_X11)
    std::unique_ptr<XlibXcbFunctions> mXlibXcbFunctions;
#endif  // defined(DAWN_USE_X11)

    std::atomic_uint64_t mDeviceCountForTesting{0};
};

}  // namespace dawn::native

#endif  // SRC_DAWN_NATIVE_INSTANCE_H_
