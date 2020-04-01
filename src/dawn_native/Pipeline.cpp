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

#include "dawn_native/Pipeline.h"

#include "dawn_native/BindGroupLayout.h"
#include "dawn_native/Device.h"
#include "dawn_native/PipelineLayout.h"
#include "dawn_native/ShaderModule.h"

namespace dawn_native {

    MaybeError ValidateProgrammableStageDescriptor(const DeviceBase* device,
                                                   const ProgrammableStageDescriptor* descriptor,
                                                   const PipelineLayoutBase* layout,
                                                   SingleShaderStage stage) {
        DAWN_TRY(device->ValidateObject(descriptor->module));

        if (descriptor->entryPoint != std::string("main")) {
            return DAWN_VALIDATION_ERROR("Entry point must be \"main\"");
        }
        if (descriptor->module->GetExecutionModel() != stage) {
            return DAWN_VALIDATION_ERROR("Setting module with wrong stages");
        }
        if (layout != nullptr && !descriptor->module->IsCompatibleWithPipelineLayout(layout)) {
            return DAWN_VALIDATION_ERROR("Stage not compatible with layout");
        }
        return {};
    }

    // PipelineBase

    PipelineBase::PipelineBase(DeviceBase* device,
                               PipelineLayoutBase* layout,
                               wgpu::ShaderStage stages)
        : CachedObject(device), mStageMask(stages), mLayout(layout) {
    }

    PipelineBase::PipelineBase(DeviceBase* device, ObjectBase::ErrorTag tag)
        : CachedObject(device, tag) {
    }

    wgpu::ShaderStage PipelineBase::GetStageMask() const {
        ASSERT(!IsError());
        return mStageMask;
    }

    PipelineLayoutBase* PipelineBase::GetLayout() {
        ASSERT(!IsError());
        return mLayout.Get();
    }

    const PipelineLayoutBase* PipelineBase::GetLayout() const {
        ASSERT(!IsError());
        return mLayout.Get();
    }

    MaybeError PipelineBase::ValidateGetBindGroupLayout(uint32_t groupIndex) {
        DAWN_TRY(GetDevice()->ValidateIsAlive());
        DAWN_TRY(GetDevice()->ValidateObject(this));
        DAWN_TRY(GetDevice()->ValidateObject(mLayout.Get()));
        if (groupIndex >= kMaxBindGroups) {
            return DAWN_VALIDATION_ERROR("Bind group layout index out of bounds");
        }
        return {};
    }

    BindGroupLayoutBase* PipelineBase::GetBindGroupLayout(uint32_t groupIndex) {
        if (GetDevice()->ConsumedError(ValidateGetBindGroupLayout(groupIndex))) {
            return BindGroupLayoutBase::MakeError(GetDevice());
        }

        if (!mLayout->GetBindGroupLayoutsMask()[groupIndex]) {
            // Get or create an empty bind group layout.
            // TODO(enga): Consider caching this object on the Device and reusing it.
            // Today, this can't be done correctly because of the order of Device destruction.
            // For example, vulkan::~Device will be called before ~DeviceBase. If DeviceBase owns
            // a Ref<BindGroupLayoutBase>, then the VkDevice will be destroyed before the
            // VkDescriptorSetLayout.
            BindGroupLayoutDescriptor desc = {};
            desc.bindingCount = 0;
            desc.bindings = nullptr;

            BindGroupLayoutBase* bgl = nullptr;
            if (GetDevice()->ConsumedError(GetDevice()->GetOrCreateBindGroupLayout(&desc), &bgl)) {
                return BindGroupLayoutBase::MakeError(GetDevice());
            }
            return bgl;
        }

        BindGroupLayoutBase* bgl = mLayout->GetBindGroupLayout(groupIndex);
        bgl->Reference();
        return bgl;
    }

}  // namespace dawn_native
