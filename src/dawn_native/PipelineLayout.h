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

#ifndef BACKEND_PIPELINELAYOUT_H_
#define BACKEND_PIPELINELAYOUT_H_

#include "common/Constants.h"
#include "dawn_native/Error.h"
#include "dawn_native/Forward.h"
#include "dawn_native/RefCounted.h"

#include "dawn/dawncpp.h"

#include <array>
#include <bitset>

namespace backend {

    MaybeError ValidatePipelineLayoutDescriptor(DeviceBase*,
                                                const dawn::PipelineLayoutDescriptor* descriptor);

    using BindGroupLayoutArray = std::array<Ref<BindGroupLayoutBase>, kMaxBindGroups>;

    class PipelineLayoutBase : public RefCounted {
      public:
        PipelineLayoutBase(DeviceBase* device, const dawn::PipelineLayoutDescriptor* descriptor);

        const BindGroupLayoutBase* GetBindGroupLayout(size_t group) const;
        const std::bitset<kMaxBindGroups> GetBindGroupLayoutsMask() const;

        // Utility functions to compute inherited bind groups.
        // Returns the inherited bind groups as a mask.
        std::bitset<kMaxBindGroups> InheritedGroupsMask(const PipelineLayoutBase* other) const;

        // Returns the index of the first incompatible bind group in the range
        // [1, kMaxBindGroups + 1]
        uint32_t GroupsInheritUpTo(const PipelineLayoutBase* other) const;

        DeviceBase* GetDevice() const;

      protected:
        DeviceBase* mDevice;
        BindGroupLayoutArray mBindGroupLayouts;
        std::bitset<kMaxBindGroups> mMask;
    };

}  // namespace backend

#endif  // BACKEND_PIPELINELAYOUT_H_
