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

#include "dawn_native/vulkan/ComputePipelineVk.h"

#include "dawn_native/CreatePipelineAsyncTask.h"
#include "dawn_native/vulkan/DeviceVk.h"
#include "dawn_native/vulkan/FencedDeleter.h"
#include "dawn_native/vulkan/PipelineLayoutVk.h"
#include "dawn_native/vulkan/ShaderModuleVk.h"
#include "dawn_native/vulkan/UtilsVulkan.h"
#include "dawn_native/vulkan/VulkanError.h"

namespace dawn_native { namespace vulkan {

    // static
    Ref<ComputePipeline> ComputePipeline::CreateUninitialized(
        Device* device,
        const ComputePipelineDescriptor* descriptor) {
        return AcquireRef(new ComputePipeline(device, descriptor));
    }

    MaybeError ComputePipeline::Initialize() {
        VkComputePipelineCreateInfo createInfo;
        createInfo.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
        createInfo.pNext = nullptr;
        createInfo.flags = 0;
        createInfo.layout = ToBackend(GetLayout())->GetHandle();
        createInfo.basePipelineHandle = ::VK_NULL_HANDLE;
        createInfo.basePipelineIndex = -1;

        createInfo.stage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        createInfo.stage.pNext = nullptr;
        createInfo.stage.flags = 0;
        createInfo.stage.stage = VK_SHADER_STAGE_COMPUTE_BIT;
        // Generate a new VkShaderModule with BindingRemapper tint transform for each pipeline
        const ProgrammableStage& computeStage = GetStage(SingleShaderStage::Compute);
        DAWN_TRY_ASSIGN(createInfo.stage.module,
                        ToBackend(computeStage.module.Get())
                            ->GetTransformedModuleHandle(computeStage.entryPoint.c_str(),
                                                         ToBackend(GetLayout())));

        createInfo.stage.pName = computeStage.entryPoint.c_str();

        std::vector<OverridableConstantScalar> specializationDataEntries;
        std::vector<VkSpecializationMapEntry> specializationMapEntries;
        VkSpecializationInfo specializationInfo{};
        createInfo.stage.pSpecializationInfo =
            GetVkSpecializationInfo(computeStage, &specializationInfo, &specializationDataEntries,
                                    &specializationMapEntries);

        Device* device = ToBackend(GetDevice());

        PNextChainBuilder stageExtChain(&createInfo.stage);

        VkPipelineShaderStageRequiredSubgroupSizeCreateInfoEXT subgroupSizeInfo = {};
        uint32_t computeSubgroupSize = device->GetComputeSubgroupSize();
        if (computeSubgroupSize != 0u) {
            ASSERT(device->GetDeviceInfo().HasExt(DeviceExt::SubgroupSizeControl));
            subgroupSizeInfo.requiredSubgroupSize = computeSubgroupSize;
            stageExtChain.Add(
                &subgroupSizeInfo,
                VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_REQUIRED_SUBGROUP_SIZE_CREATE_INFO_EXT);
        }

        DAWN_TRY(CheckVkSuccess(
            device->fn.CreateComputePipelines(device->GetVkDevice(), ::VK_NULL_HANDLE, 1,
                                              &createInfo, nullptr, &*mHandle),
            "CreateComputePipeline"));

        SetLabelImpl();

        return {};
    }

    void ComputePipeline::SetLabelImpl() {
        SetDebugName(ToBackend(GetDevice()), VK_OBJECT_TYPE_PIPELINE,
                     reinterpret_cast<uint64_t&>(mHandle), "Dawn_ComputePipeline", GetLabel());
    }

    ComputePipeline::~ComputePipeline() = default;

    void ComputePipeline::DestroyImpl() {
        ComputePipelineBase::DestroyImpl();

        if (mHandle != VK_NULL_HANDLE) {
            ToBackend(GetDevice())->GetFencedDeleter()->DeleteWhenUnused(mHandle);
            mHandle = VK_NULL_HANDLE;
        }
    }

    VkPipeline ComputePipeline::GetHandle() const {
        return mHandle;
    }

    void ComputePipeline::InitializeAsync(Ref<ComputePipelineBase> computePipeline,
                                          WGPUCreateComputePipelineAsyncCallback callback,
                                          void* userdata) {
        std::unique_ptr<CreateComputePipelineAsyncTask> asyncTask =
            std::make_unique<CreateComputePipelineAsyncTask>(std::move(computePipeline), callback,
                                                             userdata);
        CreateComputePipelineAsyncTask::RunAsync(std::move(asyncTask));
    }

}}  // namespace dawn_native::vulkan
