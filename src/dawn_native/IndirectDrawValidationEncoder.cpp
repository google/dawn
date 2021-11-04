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

#include "dawn_native/IndirectDrawValidationEncoder.h"

#include "common/Constants.h"
#include "common/Math.h"
#include "dawn_native/BindGroup.h"
#include "dawn_native/BindGroupLayout.h"
#include "dawn_native/CommandEncoder.h"
#include "dawn_native/ComputePassEncoder.h"
#include "dawn_native/ComputePipeline.h"
#include "dawn_native/Device.h"
#include "dawn_native/InternalPipelineStore.h"
#include "dawn_native/Queue.h"
#include "dawn_native/utils/WGPUHelpers.h"

#include <cstdlib>
#include <limits>

namespace dawn_native {

    namespace {
        // NOTE: This must match the workgroup_size attribute on the compute entry point below.
        constexpr uint64_t kWorkgroupSize = 64;

        // Equivalent to the BatchInfo struct defined in the shader below.
        struct BatchInfo {
            uint64_t numIndexBufferElements;
            uint32_t numDraws;
            uint32_t padding;
        };

        // TODO(https://crbug.com/dawn/1108): Propagate validation feedback from this shader in
        // various failure modes.
        static const char sRenderValidationShaderSource[] = R"(
            let kNumIndirectParamsPerDrawCall = 5u;

            let kIndexCountEntry = 0u;
            let kInstanceCountEntry = 1u;
            let kFirstIndexEntry = 2u;
            let kBaseVertexEntry = 3u;
            let kFirstInstanceEntry = 4u;

            [[block]] struct BatchInfo {
                numIndexBufferElementsLow: u32;
                numIndexBufferElementsHigh: u32;
                numDraws: u32;
                padding: u32;
                indirectOffsets: array<u32>;
            };

            [[block]] struct IndirectParams {
                data: array<u32>;
            };

            [[group(0), binding(0)]] var<storage, read> batch: BatchInfo;
            [[group(0), binding(1)]] var<storage, read_write> clientParams: IndirectParams;
            [[group(0), binding(2)]] var<storage, write> validatedParams: IndirectParams;

            fn fail(drawIndex: u32) {
                let index = drawIndex * kNumIndirectParamsPerDrawCall;
                validatedParams.data[index + kIndexCountEntry] = 0u;
                validatedParams.data[index + kInstanceCountEntry] = 0u;
                validatedParams.data[index + kFirstIndexEntry] = 0u;
                validatedParams.data[index + kBaseVertexEntry] = 0u;
                validatedParams.data[index + kFirstInstanceEntry] = 0u;
            }

            fn pass(drawIndex: u32) {
                let vIndex = drawIndex * kNumIndirectParamsPerDrawCall;
                let cIndex = batch.indirectOffsets[drawIndex];
                validatedParams.data[vIndex + kIndexCountEntry] =
                    clientParams.data[cIndex + kIndexCountEntry];
                validatedParams.data[vIndex + kInstanceCountEntry] =
                    clientParams.data[cIndex + kInstanceCountEntry];
                validatedParams.data[vIndex + kFirstIndexEntry] =
                    clientParams.data[cIndex + kFirstIndexEntry];
                validatedParams.data[vIndex + kBaseVertexEntry] =
                    clientParams.data[cIndex + kBaseVertexEntry];
                validatedParams.data[vIndex + kFirstInstanceEntry] =
                    clientParams.data[cIndex + kFirstInstanceEntry];
            }

            [[stage(compute), workgroup_size(64, 1, 1)]]
            fn main([[builtin(global_invocation_id)]] id : vec3<u32>) {
                if (id.x >= batch.numDraws) {
                    return;
                }

                let clientIndex = batch.indirectOffsets[id.x];
                let firstInstance = clientParams.data[clientIndex + kFirstInstanceEntry];
                if (firstInstance != 0u) {
                    fail(id.x);
                    return;
                }

                if (batch.numIndexBufferElementsHigh >= 2u) {
                    // firstIndex and indexCount are both u32. The maximum possible sum of these
                    // values is 0x1fffffffe, which is less than 0x200000000. Nothing to validate.
                    pass(id.x);
                    return;
                }

                let firstIndex = clientParams.data[clientIndex + kFirstIndexEntry];
                if (batch.numIndexBufferElementsHigh == 0u &&
                    batch.numIndexBufferElementsLow < firstIndex) {
                    fail(id.x);
                    return;
                }

                // Note that this subtraction may underflow, but only when
                // numIndexBufferElementsHigh is 1u. The result is still correct in that case.
                let maxIndexCount = batch.numIndexBufferElementsLow - firstIndex;
                let indexCount = clientParams.data[clientIndex + kIndexCountEntry];
                if (indexCount > maxIndexCount) {
                    fail(id.x);
                    return;
                }
                pass(id.x);
            }
        )";

        ResultOrError<ComputePipelineBase*> GetOrCreateRenderValidationPipeline(
            DeviceBase* device) {
            InternalPipelineStore* store = device->GetInternalPipelineStore();

            if (store->renderValidationPipeline == nullptr) {
                // Create compute shader module if not cached before.
                if (store->renderValidationShader == nullptr) {
                    DAWN_TRY_ASSIGN(
                        store->renderValidationShader,
                        utils::CreateShaderModule(device, sRenderValidationShaderSource));
                }

                Ref<BindGroupLayoutBase> bindGroupLayout;
                DAWN_TRY_ASSIGN(
                    bindGroupLayout,
                    utils::MakeBindGroupLayout(
                        device,
                        {
                            {0, wgpu::ShaderStage::Compute,
                             wgpu::BufferBindingType::ReadOnlyStorage},
                            {1, wgpu::ShaderStage::Compute, kInternalStorageBufferBinding},
                            {2, wgpu::ShaderStage::Compute, wgpu::BufferBindingType::Storage},
                        },
                        /* allowInternalBinding */ true));

                Ref<PipelineLayoutBase> pipelineLayout;
                DAWN_TRY_ASSIGN(pipelineLayout,
                                utils::MakeBasicPipelineLayout(device, bindGroupLayout));

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
        struct Batch {
            const IndirectDrawMetadata::IndexedIndirectValidationBatch* metadata;
            uint64_t numIndexBufferElements;
            uint64_t dataBufferOffset;
            uint64_t dataSize;
            uint64_t clientIndirectOffset;
            uint64_t clientIndirectSize;
            uint64_t validatedParamsOffset;
            uint64_t validatedParamsSize;
            BatchInfo* batchInfo;
        };

        struct Pass {
            BufferBase* clientIndirectBuffer;
            uint64_t validatedParamsSize = 0;
            uint64_t batchDataSize = 0;
            std::unique_ptr<void, void (*)(void*)> batchData{nullptr, std::free};
            std::vector<Batch> batches;
        };

        // First stage is grouping all batches into passes. We try to pack as many batches into a
        // single pass as possible. Batches can be grouped together as long as they're validating
        // data from the same indirect buffer, but they may still be split into multiple passes if
        // the number of draw calls in a pass would exceed some (very high) upper bound.
        size_t validatedParamsSize = 0;
        std::vector<Pass> passes;
        IndirectDrawMetadata::IndexedIndirectBufferValidationInfoMap& bufferInfoMap =
            *indirectDrawMetadata->GetIndexedIndirectBufferValidationInfo();
        if (bufferInfoMap.empty()) {
            return {};
        }

        const uint32_t maxStorageBufferBindingSize =
            device->GetLimits().v1.maxStorageBufferBindingSize;
        const uint32_t minStorageBufferOffsetAlignment =
            device->GetLimits().v1.minStorageBufferOffsetAlignment;

        for (auto& entry : bufferInfoMap) {
            const IndirectDrawMetadata::IndexedIndirectConfig& config = entry.first;
            BufferBase* clientIndirectBuffer = config.first;
            for (const IndirectDrawMetadata::IndexedIndirectValidationBatch& batch :
                 entry.second.GetBatches()) {
                const uint64_t minOffsetFromAlignedBoundary =
                    batch.minOffset % minStorageBufferOffsetAlignment;
                const uint64_t minOffsetAlignedDown =
                    batch.minOffset - minOffsetFromAlignedBoundary;

                Batch newBatch;
                newBatch.metadata = &batch;
                newBatch.numIndexBufferElements = config.second;
                newBatch.dataSize = GetBatchDataSize(batch.draws.size());
                newBatch.clientIndirectOffset = minOffsetAlignedDown;
                newBatch.clientIndirectSize =
                    batch.maxOffset + kDrawIndexedIndirectSize - minOffsetAlignedDown;

                newBatch.validatedParamsSize = batch.draws.size() * kDrawIndexedIndirectSize;
                newBatch.validatedParamsOffset =
                    Align(validatedParamsSize, minStorageBufferOffsetAlignment);
                validatedParamsSize = newBatch.validatedParamsOffset + newBatch.validatedParamsSize;
                if (validatedParamsSize > maxStorageBufferBindingSize) {
                    return DAWN_INTERNAL_ERROR("Too many drawIndexedIndirect calls to validate");
                }

                Pass* currentPass = passes.empty() ? nullptr : &passes.back();
                if (currentPass && currentPass->clientIndirectBuffer == clientIndirectBuffer) {
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

                Pass newPass;
                newPass.clientIndirectBuffer = clientIndirectBuffer;
                newPass.batchDataSize = newBatch.dataSize;
                newPass.batches.push_back(newBatch);
                passes.push_back(std::move(newPass));
            }
        }

        auto* const store = device->GetInternalPipelineStore();
        ScratchBuffer& validatedParamsBuffer = store->scratchIndirectStorage;
        ScratchBuffer& batchDataBuffer = store->scratchStorage;

        uint64_t requiredBatchDataBufferSize = 0;
        for (const Pass& pass : passes) {
            requiredBatchDataBufferSize = std::max(requiredBatchDataBufferSize, pass.batchDataSize);
        }
        DAWN_TRY(batchDataBuffer.EnsureCapacity(requiredBatchDataBufferSize));
        usageTracker->BufferUsedAs(batchDataBuffer.GetBuffer(), wgpu::BufferUsage::Storage);

        DAWN_TRY(validatedParamsBuffer.EnsureCapacity(validatedParamsSize));
        usageTracker->BufferUsedAs(validatedParamsBuffer.GetBuffer(), wgpu::BufferUsage::Indirect);

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

                uint32_t* indirectOffsets = reinterpret_cast<uint32_t*>(batch.batchInfo + 1);
                uint64_t validatedParamsOffset = batch.validatedParamsOffset;
                for (auto& draw : batch.metadata->draws) {
                    // The shader uses this to index an array of u32, hence the division by 4 bytes.
                    *indirectOffsets++ = static_cast<uint32_t>(
                        (draw.clientBufferOffset - batch.clientIndirectOffset) / 4);

                    draw.cmd->indirectBuffer = validatedParamsBuffer.GetBuffer();
                    draw.cmd->indirectOffset = validatedParamsOffset;

                    validatedParamsOffset += kDrawIndexedIndirectSize;
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

        BindGroupEntry& clientIndirectBinding = bindings[1];
        clientIndirectBinding.binding = 1;

        BindGroupEntry& validatedParamsBinding = bindings[2];
        validatedParamsBinding.binding = 2;
        validatedParamsBinding.buffer = validatedParamsBuffer.GetBuffer();

        BindGroupDescriptor bindGroupDescriptor = {};
        bindGroupDescriptor.layout = layout.Get();
        bindGroupDescriptor.entryCount = 3;
        bindGroupDescriptor.entries = bindings;

        // Finally, we can now encode our validation passes. Each pass first does a single
        // WriteBuffer to get batch data over to the GPU, followed by a single compute pass. The
        // compute pass encodes a separate SetBindGroup and Dispatch command for each batch.
        for (const Pass& pass : passes) {
            commandEncoder->APIWriteBuffer(batchDataBuffer.GetBuffer(), 0,
                                           static_cast<const uint8_t*>(pass.batchData.get()),
                                           pass.batchDataSize);

            // TODO(dawn:723): change to not use AcquireRef for reentrant object creation.
            ComputePassDescriptor descriptor = {};
            Ref<ComputePassEncoder> passEncoder =
                AcquireRef(commandEncoder->APIBeginComputePass(&descriptor));
            passEncoder->APISetPipeline(pipeline);

            clientIndirectBinding.buffer = pass.clientIndirectBuffer;

            for (const Batch& batch : pass.batches) {
                bufferDataBinding.offset = batch.dataBufferOffset;
                bufferDataBinding.size = batch.dataSize;
                clientIndirectBinding.offset = batch.clientIndirectOffset;
                clientIndirectBinding.size = batch.clientIndirectSize;
                validatedParamsBinding.offset = batch.validatedParamsOffset;
                validatedParamsBinding.size = batch.validatedParamsSize;

                Ref<BindGroupBase> bindGroup;
                DAWN_TRY_ASSIGN(bindGroup, device->CreateBindGroup(&bindGroupDescriptor));

                const uint32_t numDrawsRoundedUp =
                    (batch.batchInfo->numDraws + kWorkgroupSize - 1) / kWorkgroupSize;
                passEncoder->APISetBindGroup(0, bindGroup.Get());
                passEncoder->APIDispatch(numDrawsRoundedUp);
            }

            passEncoder->APIEndPass();
        }

        return {};
    }

}  // namespace dawn_native
