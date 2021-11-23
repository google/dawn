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

#ifndef DAWNNATIVE_ADAPTER_H_
#define DAWNNATIVE_ADAPTER_H_

#include "dawn_native/DawnNative.h"

#include "dawn_native/Error.h"
#include "dawn_native/Features.h"
#include "dawn_native/Limits.h"
#include "dawn_native/dawn_platform.h"

#include <string>

namespace dawn_native {

    class DeviceBase;

    class AdapterBase {
      public:
        AdapterBase(InstanceBase* instance, wgpu::BackendType backend);
        virtual ~AdapterBase() = default;

        MaybeError Initialize();

        wgpu::BackendType GetBackendType() const;
        wgpu::AdapterType GetAdapterType() const;
        const std::string& GetDriverDescription() const;
        const PCIInfo& GetPCIInfo() const;
        InstanceBase* GetInstance() const;

        DeviceBase* CreateDevice(const DawnDeviceDescriptor* descriptor = nullptr);

        void RequestDevice(const DawnDeviceDescriptor* descriptor,
                           WGPURequestDeviceCallback callback,
                           void* userdata);

        void ResetInternalDeviceForTesting();

        FeaturesSet GetSupportedFeatures() const;
        bool SupportsAllRequestedFeatures(const std::vector<const char*>& requestedFeatures) const;
        WGPUDeviceProperties GetAdapterProperties() const;

        bool GetLimits(SupportedLimits* limits) const;

        void SetUseTieredLimits(bool useTieredLimits);

        virtual bool SupportsExternalImages() const = 0;

      protected:
        PCIInfo mPCIInfo = {};
        wgpu::AdapterType mAdapterType = wgpu::AdapterType::Unknown;
        std::string mDriverDescription;
        FeaturesSet mSupportedFeatures;

      private:
        virtual ResultOrError<DeviceBase*> CreateDeviceImpl(
            const DawnDeviceDescriptor* descriptor) = 0;

        virtual MaybeError InitializeImpl() = 0;

        // Check base WebGPU features and discover supported featurees.
        virtual MaybeError InitializeSupportedFeaturesImpl() = 0;

        // Check base WebGPU limits and populate supported limits.
        virtual MaybeError InitializeSupportedLimitsImpl(CombinedLimits* limits) = 0;

        MaybeError CreateDeviceInternal(DeviceBase** result,
                                        const DawnDeviceDescriptor* descriptor);

        virtual MaybeError ResetInternalDeviceForTestingImpl();
        InstanceBase* mInstance = nullptr;
        wgpu::BackendType mBackend;
        CombinedLimits mLimits;
        bool mUseTieredLimits = false;
    };

}  // namespace dawn_native

#endif  // DAWNNATIVE_ADAPTER_H_
