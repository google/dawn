// Copyright 2017 The Dawn & Tint Authors
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

#include "dawn/native/metal/PipelineLayoutMTL.h"

#include "dawn/common/BitSetIterator.h"
#include "dawn/native/BindGroupLayoutInternal.h"
#include "dawn/native/metal/DeviceMTL.h"

namespace dawn::native::metal {

// static
Ref<PipelineLayout> PipelineLayout::Create(Device* device,
                                           const PipelineLayoutDescriptor* descriptor) {
    return AcquireRef(new PipelineLayout(device, descriptor));
}

PipelineLayout::PipelineLayout(Device* device, const PipelineLayoutDescriptor* descriptor)
    : PipelineLayoutBase(device, descriptor) {
    // Each stage has its own numbering namespace in CompilerMSL.
    for (auto stage : IterateStages(kAllStages)) {
        uint32_t bufferIndex = 0;
        uint32_t samplerIndex = 0;
        uint32_t textureIndex = 0;

        for (BindGroupIndex group : IterateBitSet(GetBindGroupLayoutsMask())) {
            mIndexInfo[stage][group].resize(GetBindGroupLayout(group)->GetBindingCount());

            for (BindingIndex bindingIndex{0};
                 bindingIndex < GetBindGroupLayout(group)->GetBindingCount(); ++bindingIndex) {
                const BindingInfo& bindingInfo =
                    GetBindGroupLayout(group)->GetBindingInfo(bindingIndex);
                if (!(bindingInfo.visibility & StageBit(stage))) {
                    continue;
                }

                switch (bindingInfo.bindingType) {
                    case BindingInfoType::Buffer:
                        mIndexInfo[stage][group][bindingIndex] = bufferIndex;
                        bufferIndex++;
                        break;

                    case BindingInfoType::Sampler:
                        mIndexInfo[stage][group][bindingIndex] = samplerIndex;
                        samplerIndex++;
                        break;

                    case BindingInfoType::Texture:
                    case BindingInfoType::StorageTexture:
                    case BindingInfoType::ExternalTexture:
                        mIndexInfo[stage][group][bindingIndex] = textureIndex;
                        textureIndex++;
                        break;
                }
            }
        }

        mBufferBindingCount[stage] = bufferIndex;
    }
}

PipelineLayout::~PipelineLayout() = default;

const PipelineLayout::BindingIndexInfo& PipelineLayout::GetBindingIndexInfo(
    SingleShaderStage stage) const {
    return mIndexInfo[stage];
}

uint32_t PipelineLayout::GetBufferBindingCount(SingleShaderStage stage) const {
    return mBufferBindingCount[stage];
}

}  // namespace dawn::native::metal
