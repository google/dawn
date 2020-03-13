// Copyright 2017 The Dawn Authors
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

#ifndef DAWNNATIVE_D3D12_BINDGROUPD3D12_H_
#define DAWNNATIVE_D3D12_BINDGROUPD3D12_H_

#include "common/PlacementAllocated.h"
#include "common/Serial.h"
#include "dawn_native/BindGroup.h"
#include "dawn_native/d3d12/d3d12_platform.h"

namespace dawn_native { namespace d3d12 {

    class Device;
    class ShaderVisibleDescriptorAllocator;

    class BindGroup : public BindGroupBase, public PlacementAllocated {
      public:
        static BindGroup* Create(Device* device, const BindGroupDescriptor* descriptor);

        BindGroup(Device* device, const BindGroupDescriptor* descriptor);
        ~BindGroup() override;

        // Returns true if the BindGroup was successfully populated.
        ResultOrError<bool> Populate(ShaderVisibleDescriptorAllocator* allocator);

        D3D12_GPU_DESCRIPTOR_HANDLE GetBaseCbvUavSrvDescriptor() const;
        D3D12_GPU_DESCRIPTOR_HANDLE GetBaseSamplerDescriptor() const;

      private:
        Serial mLastUsageSerial = 0;
        Serial mHeapSerial = 0;

        D3D12_GPU_DESCRIPTOR_HANDLE mBaseCbvSrvUavDescriptor = {0};
        D3D12_GPU_DESCRIPTOR_HANDLE mBaseSamplerDescriptor = {0};
    };
}}  // namespace dawn_native::d3d12

#endif  // DAWNNATIVE_D3D12_BINDGROUPD3D12_H_
