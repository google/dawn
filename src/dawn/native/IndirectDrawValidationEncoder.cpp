// Copyright 2021 The Dawn Authors
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

#include "dawn/native/IndirectDrawValidationEncoder.h"

#include <algorithm>
#include <cstdlib>
#include <limits>
#include <memory>
#include <utility>
#include <vector>

#include "dawn/common/Constants.h"
#include "dawn/common/Math.h"
#include "dawn/native/BindGroup.h"
#include "dawn/native/BindGroupLayout.h"
#include "dawn/native/CommandEncoder.h"
#include "dawn/native/ComputePassEncoder.h"
#include "dawn/native/ComputePipeline.h"
#include "dawn/native/Device.h"
#include "dawn/native/InternalPipelineStore.h"
#include "dawn/native/Queue.h"
#include "dawn/native/utils/WGPUHelpers.h"

namespace dawn::native {

namespace {
// NOTE: This must match the workgroup_size attribute on the compute entry point below.
constexpr uint64_t kWorkgroupSize = 64;

// Bitmasks for BatchInfo::flags
constexpr uint32_t kDuplicateBaseVertexInstance = 1;
constexpr uint32_t kIndexedDraw = 2;
constexpr uint32_t kValidationEnabled = 4;
constexpr uint32_t kIndirectFirstInstanceEnabled = 8;

// Equivalent to the BatchInfo struct defined in the shader below.
struct BatchInfo {
    uint64_t numIndexBufferElements;
    uint32_t numDraws;
    uint32_t flags;
};

// TODO(https://crbug.com/dawn/1108): Propagate validation feedback from this shader in
// various failure modes.
static const char sRenderValidationShaderSource[] = R"(

            const kNumDrawIndirectParams = 4u;

            const kIndexCountEntry = 0u;
            const kFirstIndexEntry = 2u;

            // Bitmasks for BatchInfo::flags
            const kDuplicateBaseVertexInstance = 1u;
            const kIndexedDraw = 2u;
            const kValidationEnabled = 4u;
            const kIndirectFirstInstanceEnabled = 8u;

            struct BatchInfo {
                numIndexBufferElementsLow: u32,
                numIndexBufferElementsHigh: u32,
                numDraws: u32,
                flags: u32,
                indirectOffsets: array<u32>,
            }

            struct IndirectParams {
                data: array<u32>,
            }

            @group(0) @binding(0) var<storage, read> batch: BatchInfo;
            @group(0) @binding(1) var<storage, read_write> inputParams: IndirectParams;
            @group(0) @binding(2) var<storage, read_write> outputParams: IndirectParams;

            fn numIndirectParamsPerDrawCallInput() -> u32 {
                var numParams = kNumDrawIndirectParams;
                // Indexed Draw has an extra parameter (firstIndex)
                if (bool(batch.flags & kIndexedDraw)) {
                    numParams = numParams + 1u;
                }
                return numParams;
            }

            fn numIndirectParamsPerDrawCallOutput() -> u32 {
                var numParams = numIndirectParamsPerDrawCallInput();
                // 2 extra parameter for duplicated first/baseVexter and firstInstance
                if (bool(batch.flags & kDuplicateBaseVertexInstance)) {
                    numParams = numParams + 2u;
                }
                return numParams;
            }

            fn fail(drawIndex: u32) {
                let numParams = numIndirectParamsPerDrawCallOutput();
                let index = drawIndex * numParams;
                for(var i = 0u; i < numParams; i = i + 1u) {
                    outputParams.data[index + i] = 0u;
                }
            }

            fn set_pass(drawIndex: u32) {
                let numInputParams = numIndirectParamsPerDrawCallInput();
                var outIndex = drawIndex * numIndirectParamsPerDrawCallOutput();
                let inIndex = batch.indirectOffsets[drawIndex];

                // The first 2 parameter is reserved for the duplicated first/baseVertex and firstInstance

                if (bool(batch.flags & kDuplicateBaseVertexInstance)) {
                    // first/baseVertex and firstInstance are always last two parameters
                    let dupIndex = inIndex + numInputParams - 2u;
                    outputParams.data[outIndex] = inputParams.data[dupIndex];
                    outputParams.data[outIndex + 1u] = inputParams.data[dupIndex + 1u];

                    outIndex = outIndex + 2u;
                }

                for(var i = 0u; i < numInputParams; i = i + 1u) {
                    outputParams.data[outIndex + i] = inputParams.data[inIndex + i];
                }
            }

            @compute @workgroup_size(64, 1, 1)
            fn main(@builtin(global_invocation_id) id : vec3u) {
                if (id.x >= batch.numDraws) {
                    return;
                }

                if(!bool(batch.flags & kValidationEnabled)) {
                    set_pass(id.x);
                    return;
                }

                let inputIndex = batch.indirectOffsets[id.x];
                if(!bool(batch.flags & kIndirectFirstInstanceEnabled)) {
                    // firstInstance is always the last parameter
                    let firstInstance = inputParams.data[inputIndex + numIndirectParamsPerDrawCallInput() - 1u];
                    if (firstInstance != 0u) {
                        fail(id.x);
                        return;
                    }
                }

                if (!bool(batch.flags & kIndexedDraw)) {
                    set_pass(id.x);
                    return;
                }

                if (batch.numIndexBufferElementsHigh >= 2u) {
                    // firstIndex and indexCount are both u32. The maximum possible sum of these
                    // values is 0x1fffffffe, which is less than 0x200000000. Nothing to validate.
                    set_pass(id.x);
                    return;
                }

                let firstIndex = inputParams.data[inputIndex + kFirstIndexEntry];
                if (batch.numIndexBufferElementsHigh == 0u &&
                    batch.numIndexBufferElementsLow < firstIndex) {
                    fail(id.x);
                    return;
                }

                // Note that this subtraction may underflow, but only when
                // numIndexBufferElementsHigh is 1u. The result is still correct in that case.
                let maxIndexCount = batch.numIndexBufferElementsLow - firstIndex;
                let indexCount = inputParams.data[inputIndex + kIndexCountEntry];
                if (indexCount > maxIndexCount) {
                    fail(id.x);
                    return;
                }
                set_pass(id.x);
            }
        )";

ResultOrError<ComputePipelineBase*> GetOrCreateRenderValidationPipeline(DeviceBase* device) {
    InternalPipelineStore* store = device->GetInternalPipelineStore();

    if (store->renderValidationPipeline == nullptr) {
        // Create compute shader module if not cached before.
        if (store->renderValidationShader == nullptr) {
            DAWN_TRY_ASSIGN(store->renderValidationShader,
                            utils::CreateShaderModule(device, sRenderValidationShaderSource));
        }

        Ref<BindGroupLayoutBase> bindGroupLayout;
        DAWN_TRY_ASSIGN(
            bindGroupLayout,
            utils::MakeBindGroupLayout(
                device,
                {
                    {0, wgpu::ShaderStage::Compute, wgpu::BufferBindingType::ReadOnlyStorage},
                    {1, wgpu::ShaderStage::Compute, kInternalStorageBufferBinding},
                    {2, wgpu::ShaderStage::Compute, wgpu::BufferBindingType::Storage},
                },
                /* allowInternalBinding */ true));

        Ref<PipelineLayoutBase> pipelineLayout;
        DAWN_TRY_ASSIGN(pipelineLayout, utils::MakeBasicPipelineLayout(device, bindGroupLayout));

        ComputePipelineDescriptor computePipelineDescriptor = {};
        computePipelineDescriptor.layout = pipelineLayout.Get();
        computePipelineDescriptor.compute.module = store->renderValidationShader.Get();
        computePipelineDescriptor.compute.entryPoint = "main";

        DAWN_TRY_ASSIGN(store->renderValidationPipeline,
                        device->CreateComputePipeline(&computePipelineDescriptor));
    }

    return store->renderValidationPipeline.Get();
}

size_t GetBatchDataSize(uint32_t numDraws) {
    return sizeof(BatchInfo) + numDraws * sizeof(uint32_t);
}

}  // namespace

uint32_t ComputeMaxDrawCallsPerIndirectValidationBatch(const CombinedLimits& limits) {
    const uint64_t batchDrawCallLimitByDispatchSize =
        static_cast<uint64_t>(limits.v1.maxComputeWorkgroupsPerDimension) * kWorkgroupSize;
    const uint64_t batchDrawCallLimitByStorageBindingSize =
        (limits.v1.maxStorageBufferBindingSize - sizeof(BatchInfo)) / sizeof(uint32_t);
    return static_cast<uint32_t>(
        std::min({batchDrawCallLimitByDispatchSize, batchDrawCallLimitByStorageBindingSize,
                  uint64_t(std::numeric_limits<uint32_t>::max())}));
}

MaybeError EncodeIndirectDrawValidationCommands(DeviceBase* device,
                                                CommandEncoder* commandEncoder,
                                                RenderPassResourceUsageTracker* usageTracker,
                                                IndirectDrawMetadata* indirectDrawMetadata) {
    // Since encoding validation commands may create new objects, verify that the device is alive.
    // TODO(dawn:1199): This check is obsolete if device loss causes device.destroy().
    //   - This function only happens within the context of a TryEncode which would catch the
    //     same issue if device loss implied device.destroy().
    DAWN_TRY(device->ValidateIsAlive());

    struct Batch {
        const IndirectDrawMetadata::IndirectValidationBatch* metadata;
        uint64_t numIndexBufferElements;
        uint64_t dataBufferOffset;
        uint64_t dataSize;
        uint64_t inputIndirectOffset;
        uint64_t inputIndirectSize;
        uint64_t outputParamsOffset;
        uint64_t outputParamsSize;
        BatchInfo* batchInfo;
    };

    struct Pass {
        uint32_t flags;
        BufferBase* inputIndirectBuffer;
        uint64_t outputParamsSize = 0;
        uint64_t batchDataSize = 0;
        std::unique_ptr<void, void (*)(void*)> batchData{nullptr, std::free};
        std::vector<Batch> batches;
    };

    // First stage is grouping all batches into passes. We try to pack as many batches into a
    // single pass as possible. Batches can be grouped together as long as they're validating
    // data from the same indirect buffer, but they may still be split into multiple passes if
    // the number of draw calls in a pass would exceed some (very high) upper bound.
    uint64_t outputParamsSize = 0;
    std::vector<Pass> passes;
    IndirectDrawMetadata::IndexedIndirectBufferValidationInfoMap& bufferInfoMap =
        *indirectDrawMetadata->GetIndexedIndirectBufferValidationInfo();
    if (bufferInfoMap.empty()) {
        return {};
    }

    const uint64_t maxStorageBufferBindingSize = device->GetLimits().v1.maxStorageBufferBindingSize;
    const uint32_t minStorageBufferOffsetAlignment =
        device->GetLimits().v1.minStorageBufferOffsetAlignment;

    for (auto& [config, validationInfo] : bufferInfoMap) {
        const uint64_t indirectDrawCommandSize =
            config.drawType == IndirectDrawMetadata::DrawType::Indexed ? kDrawIndexedIndirectSize
                                                                       : kDrawIndirectSize;

        uint64_t outputIndirectSize = indirectDrawCommandSize;
        if (config.duplicateBaseVertexInstance) {
            outputIndirectSize += 2 * sizeof(uint32_t);
        }

        for (const IndirectDrawMetadata::IndirectValidationBatch& batch :
             validationInfo.GetBatches()) {
            const uint64_t minOffsetFromAlignedBoundary =
                batch.minOffset % minStorageBufferOffsetAlignment;
            const uint64_t minOffsetAlignedDown = batch.minOffset - minOffsetFromAlignedBoundary;

            Batch newBatch;
            newBatch.metadata = &batch;
            newBatch.numIndexBufferElements = config.numIndexBufferElements;
            newBatch.dataSize = GetBatchDataSize(batch.draws.size());
            newBatch.inputIndirectOffset = minOffsetAlignedDown;
            newBatch.inputIndirectSize =
                batch.maxOffset + indirectDrawCommandSize - minOffsetAlignedDown;

            newBatch.outputParamsSize = batch.draws.size() * outputIndirectSize;
            newBatch.outputParamsOffset = Align(outputParamsSize, minStorageBufferOffsetAlignment);
            outputParamsSize = newBatch.outputParamsOffset + newBatch.outputParamsSize;
            if (outputParamsSize > maxStorageBufferBindingSize) {
                return DAWN_INTERNAL_ERROR("Too many drawIndexedIndirect calls to validate");
            }

            Pass* currentPass = passes.empty() ? nullptr : &passes.back();
            if (currentPass && currentPass->inputIndirectBuffer == config.inputIndirectBuffer) {
                uint64_t nextBatchDataOffset =
                    Align(currentPass->batchDataSize, minStorageBufferOffsetAlignment);
                uint64_t newPassBatchDataSize = nextBatchDataOffset + newBatch.dataSize;
                if (newPassBatchDataSize <= maxStorageBufferBindingSize) {
                    // We can fit this batch in the current pass.
                    newBatch.dataBufferOffset = nextBatchDataOffset;
                    currentPass->batchDataSize = newPassBatchDataSize;
                    currentPass->batches.push_back(newBatch);
                    continue;
                }
            }

            // We need to start a new pass for this batch.
            newBatch.dataBufferOffset = 0;

            Pass newPass{};
            newPass.inputIndirectBuffer = config.inputIndirectBuffer;
            newPass.batchDataSize = newBatch.dataSize;
            newPass.batches.push_back(newBatch);
            newPass.flags = 0;
            if (config.duplicateBaseVertexInstance) {
                newPass.flags |= kDuplicateBaseVertexInstance;
            }
            if (config.drawType == IndirectDrawMetadata::DrawType::Indexed) {
                newPass.flags |= kIndexedDraw;
            }
            if (device->IsValidationEnabled()) {
                newPass.flags |= kValidationEnabled;
            }
            if (device->HasFeature(Feature::IndirectFirstInstance)) {
                newPass.flags |= kIndirectFirstInstanceEnabled;
            }
            passes.push_back(std::move(newPass));
        }
    }

    auto* const store = device->GetInternalPipelineStore();
    ScratchBuffer& outputParamsBuffer = store->scratchIndirectStorage;
    ScratchBuffer& batchDataBuffer = store->scratchStorage;

    uint64_t requiredBatchDataBufferSize = 0;
    for (const Pass& pass : passes) {
        requiredBatchDataBufferSize = std::max(requiredBatchDataBufferSize, pass.batchDataSize);
    }
    DAWN_TRY(batchDataBuffer.EnsureCapacity(requiredBatchDataBufferSize));
    usageTracker->BufferUsedAs(batchDataBuffer.GetBuffer(), wgpu::BufferUsage::Storage);

    DAWN_TRY(outputParamsBuffer.EnsureCapacity(outputParamsSize));
    usageTracker->BufferUsedAs(outputParamsBuffer.GetBuffer(), wgpu::BufferUsage::Indirect);

    // Now we allocate and populate host-side batch data to be copied to the GPU.
    for (Pass& pass : passes) {
        // We use std::malloc here because it guarantees maximal scalar alignment.
        pass.batchData = {std::malloc(pass.batchDataSize), std::free};
        memset(pass.batchData.get(), 0, pass.batchDataSize);
        uint8_t* batchData = static_cast<uint8_t*>(pass.batchData.get());
        for (Batch& batch : pass.batches) {
            batch.batchInfo = new (&batchData[batch.dataBufferOffset]) BatchInfo();
            batch.batchInfo->numIndexBufferElements = batch.numIndexBufferElements;
            batch.batchInfo->numDraws = static_cast<uint32_t>(batch.metadata->draws.size());
            batch.batchInfo->flags = pass.flags;

            uint32_t* indirectOffsets = reinterpret_cast<uint32_t*>(batch.batchInfo + 1);
            uint64_t outputParamsOffset = batch.outputParamsOffset;
            for (auto& draw : batch.metadata->draws) {
                // The shader uses this to index an array of u32, hence the division by 4 bytes.
                *indirectOffsets++ =
                    static_cast<uint32_t>((draw.inputBufferOffset - batch.inputIndirectOffset) / 4);

                draw.cmd->indirectBuffer = outputParamsBuffer.GetBuffer();
                draw.cmd->indirectOffset = outputParamsOffset;
                if (pass.flags & kIndexedDraw) {
                    outputParamsOffset += kDrawIndexedIndirectSize;
                } else {
                    outputParamsOffset += kDrawIndirectSize;
                }
            }
        }
    }

    ComputePipelineBase* pipeline;
    DAWN_TRY_ASSIGN(pipeline, GetOrCreateRenderValidationPipeline(device));

    Ref<BindGroupLayoutBase> layout;
    DAWN_TRY_ASSIGN(layout, pipeline->GetBindGroupLayout(0));

    BindGroupEntry bindings[3];
    BindGroupEntry& bufferDataBinding = bindings[0];
    bufferDataBinding.binding = 0;
    bufferDataBinding.buffer = batchDataBuffer.GetBuffer();

    BindGroupEntry& inputIndirectBinding = bindings[1];
    inputIndirectBinding.binding = 1;

    BindGroupEntry& outputParamsBinding = bindings[2];
    outputParamsBinding.binding = 2;
    outputParamsBinding.buffer = outputParamsBuffer.GetBuffer();

    BindGroupDescriptor bindGroupDescriptor = {};
    bindGroupDescriptor.layout = layout.Get();
    bindGroupDescriptor.entryCount = 3;
    bindGroupDescriptor.entries = bindings;

    // Finally, we can now encode our validation and duplication passes. Each pass first does a
    // two WriteBuffer to get batch and pass data over to the GPU, followed by a single compute
    // pass. The compute pass encodes a separate SetBindGroup and Dispatch command for each
    // batch.
    for (const Pass& pass : passes) {
        commandEncoder->APIWriteBuffer(batchDataBuffer.GetBuffer(), 0,
                                       static_cast<const uint8_t*>(pass.batchData.get()),
                                       pass.batchDataSize);

        Ref<ComputePassEncoder> passEncoder = commandEncoder->BeginComputePass();
        passEncoder->APISetPipeline(pipeline);

        inputIndirectBinding.buffer = pass.inputIndirectBuffer;

        for (const Batch& batch : pass.batches) {
            bufferDataBinding.offset = batch.dataBufferOffset;
            bufferDataBinding.size = batch.dataSize;
            inputIndirectBinding.offset = batch.inputIndirectOffset;
            inputIndirectBinding.size = batch.inputIndirectSize;
            outputParamsBinding.offset = batch.outputParamsOffset;
            outputParamsBinding.size = batch.outputParamsSize;

            Ref<BindGroupBase> bindGroup;
            DAWN_TRY_ASSIGN(bindGroup, device->CreateBindGroup(&bindGroupDescriptor));

            const uint32_t numDrawsRoundedUp =
                (batch.batchInfo->numDraws + kWorkgroupSize - 1) / kWorkgroupSize;
            passEncoder->APISetBindGroup(0, bindGroup.Get());
            passEncoder->APIDispatchWorkgroups(numDrawsRoundedUp);
        }

        passEncoder->APIEnd();
    }

    return {};
}

}  // namespace dawn::native
