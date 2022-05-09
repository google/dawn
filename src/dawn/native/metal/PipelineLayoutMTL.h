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

#ifndef SRC_DAWN_NATIVE_METAL_PIPELINELAYOUTMTL_H_
#define SRC_DAWN_NATIVE_METAL_PIPELINELAYOUTMTL_H_

#include "dawn/common/ityp_stack_vec.h"
#include "dawn/native/BindingInfo.h"
#include "dawn/native/PipelineLayout.h"

#include "dawn/native/PerStage.h"

#import <Metal/Metal.h>

namespace dawn::native::metal {

class Device;

// The number of Metal buffers usable by applications in general
static constexpr size_t kMetalBufferTableSize = 31;
// The Metal buffer slot that Dawn reserves for its own use to pass more data to shaders
static constexpr size_t kBufferLengthBufferSlot = kMetalBufferTableSize - 1;
// The number of Metal buffers Dawn can use in a generic way (i.e. that aren't reserved)
static constexpr size_t kGenericMetalBufferSlots = kMetalBufferTableSize - 1;

static constexpr BindGroupIndex kPullingBufferBindingSet = BindGroupIndex(kMaxBindGroups);

class PipelineLayout final : public PipelineLayoutBase {
  public:
    static Ref<PipelineLayout> Create(Device* device, const PipelineLayoutDescriptor* descriptor);

    using BindingIndexInfo =
        ityp::array<BindGroupIndex,
                    ityp::stack_vec<BindingIndex, uint32_t, kMaxOptimalBindingsPerGroup>,
                    kMaxBindGroups>;
    const BindingIndexInfo& GetBindingIndexInfo(SingleShaderStage stage) const;

    // The number of Metal vertex stage buffers used for the whole pipeline layout.
    uint32_t GetBufferBindingCount(SingleShaderStage stage);

  private:
    PipelineLayout(Device* device, const PipelineLayoutDescriptor* descriptor);
    ~PipelineLayout() override;
    PerStage<BindingIndexInfo> mIndexInfo;
    PerStage<uint32_t> mBufferBindingCount;
};

}  // namespace dawn::native::metal

#endif  // SRC_DAWN_NATIVE_METAL_PIPELINELAYOUTMTL_H_
