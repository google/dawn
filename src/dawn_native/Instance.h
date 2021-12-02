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

#ifndef DAWNNATIVE_INSTANCE_H_
#define DAWNNATIVE_INSTANCE_H_

#include "common/RefCounted.h"
#include "common/ityp_bitset.h"
#include "dawn_native/Adapter.h"
#include "dawn_native/BackendConnection.h"
#include "dawn_native/Features.h"
#include "dawn_native/Toggles.h"
#include "dawn_native/dawn_platform.h"

#include <array>
#include <memory>
#include <unordered_map>
#include <vector>

namespace dawn_platform {
    class Platform;
}  // namespace dawn_platform

namespace dawn_native {

    class Surface;
    class XlibXcbFunctions;

    using BackendsBitset = ityp::bitset<wgpu::BackendType, kEnumCount<wgpu::BackendType>>;

    // This is called InstanceBase for consistency across the frontend, even if the backends don't
    // specialize this class.
    class InstanceBase final : public RefCounted {
      public:
        static InstanceBase* Create(const InstanceDescriptor* descriptor = nullptr);

        void DiscoverDefaultAdapters();
        bool DiscoverAdapters(const AdapterDiscoveryOptionsBase* options);

        const std::vector<std::unique_ptr<AdapterBase>>& GetAdapters() const;

        // Used to handle error that happen up to device creation.
        bool ConsumedError(MaybeError maybeError);

        // Used to query the details of a toggle. Return nullptr if toggleName is not a valid name
        // of a toggle supported in Dawn.
        const ToggleInfo* GetToggleInfo(const char* toggleName);
        Toggle ToggleNameToEnum(const char* toggleName);

        // Used to query the details of an feature. Return nullptr if featureName is not a valid
        // name of an feature supported in Dawn.
        const FeatureInfo* GetFeatureInfo(const char* featureName);
        Feature FeatureNameToEnum(const char* featureName);
        FeaturesSet FeatureNamesToFeaturesSet(const std::vector<const char*>& requiredFeatures);

        bool IsBackendValidationEnabled() const;
        void SetBackendValidationLevel(BackendValidationLevel level);
        BackendValidationLevel GetBackendValidationLevel() const;

        void EnableBeginCaptureOnStartup(bool beginCaptureOnStartup);
        bool IsBeginCaptureOnStartupEnabled() const;

        void SetPlatform(dawn_platform::Platform* platform);
        dawn_platform::Platform* GetPlatform();

        // Get backend-independent libraries that need to be loaded dynamically.
        const XlibXcbFunctions* GetOrCreateXlibXcbFunctions();

        // Dawn API
        Surface* APICreateSurface(const SurfaceDescriptor* descriptor);

      private:
        InstanceBase() = default;
        ~InstanceBase() = default;

        InstanceBase(const InstanceBase& other) = delete;
        InstanceBase& operator=(const InstanceBase& other) = delete;

        bool Initialize(const InstanceDescriptor* descriptor);

        // Lazily creates connections to all backends that have been compiled.
        void EnsureBackendConnection(wgpu::BackendType backendType);

        MaybeError DiscoverAdaptersInternal(const AdapterDiscoveryOptionsBase* options);

        BackendsBitset mBackendsConnected;

        bool mDiscoveredDefaultAdapters = false;

        bool mBeginCaptureOnStartup = false;
        BackendValidationLevel mBackendValidationLevel = BackendValidationLevel::Disabled;

        dawn_platform::Platform* mPlatform = nullptr;
        std::unique_ptr<dawn_platform::Platform> mDefaultPlatform;

        std::vector<std::unique_ptr<BackendConnection>> mBackends;
        std::vector<std::unique_ptr<AdapterBase>> mAdapters;

        FeaturesInfo mFeaturesInfo;
        TogglesInfo mTogglesInfo;

#if defined(DAWN_USE_X11)
        std::unique_ptr<XlibXcbFunctions> mXlibXcbFunctions;
#endif  // defined(DAWN_USE_X11)
    };

}  // namespace dawn_native

#endif  // DAWNNATIVE_INSTANCE_H_
