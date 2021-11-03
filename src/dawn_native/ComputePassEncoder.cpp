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

#include "dawn_native/ComputePassEncoder.h"

#include "dawn_native/BindGroup.h"
#include "dawn_native/BindGroupLayout.h"
#include "dawn_native/Buffer.h"
#include "dawn_native/CommandEncoder.h"
#include "dawn_native/CommandValidation.h"
#include "dawn_native/Commands.h"
#include "dawn_native/ComputePipeline.h"
#include "dawn_native/Device.h"
#include "dawn_native/InternalPipelineStore.h"
#include "dawn_native/ObjectType_autogen.h"
#include "dawn_native/PassResourceUsageTracker.h"
#include "dawn_native/QuerySet.h"

namespace dawn_native {

    namespace {

        ResultOrError<ComputePipelineBase*> GetOrCreateIndirectDispatchValidationPipeline(
            DeviceBase* device) {
            InternalPipelineStore* store = device->GetInternalPipelineStore();

            if (store->dispatchIndirectValidationPipeline != nullptr) {
                return store->dispatchIndirectValidationPipeline.Get();
            }

            ShaderModuleDescriptor descriptor;
            ShaderModuleWGSLDescriptor wgslDesc;
            descriptor.nextInChain = reinterpret_cast<ChainedStruct*>(&wgslDesc);

            // TODO(https://crbug.com/dawn/1108): Propagate validation feedback from this
            // shader in various failure modes.
            wgslDesc.source = R"(
                [[block]] struct UniformParams {
                    maxComputeWorkgroupsPerDimension: u32;
                    clientOffsetInU32: u32;
                };

                [[block]] struct IndirectParams {
                    data: array<u32>;
                };

                [[block]] struct ValidatedParams {
                    data: array<u32, 3>;
                };

                [[group(0), binding(0)]] var<uniform> uniformParams: UniformParams;
                [[group(0), binding(1)]] var<storage, read_write> clientParams: IndirectParams;
                [[group(0), binding(2)]] var<storage, write> validatedParams: ValidatedParams;

                [[stage(compute), workgroup_size(1, 1, 1)]]
                fn main() {
                    for (var i = 0u; i < 3u; i = i + 1u) {
                        var numWorkgroups = clientParams.data[uniformParams.clientOffsetInU32 + i];
                        if (numWorkgroups > uniformParams.maxComputeWorkgroupsPerDimension) {
                            numWorkgroups = 0u;
                        }
                        validatedParams.data[i] = numWorkgroups;
                    }
                }
            )";

            Ref<ShaderModuleBase> shaderModule;
            DAWN_TRY_ASSIGN(shaderModule, device->CreateShaderModule(&descriptor));

            std::array<BindGroupLayoutEntry, 3> entries;
            entries[0].binding = 0;
            entries[0].visibility = wgpu::ShaderStage::Compute;
            entries[0].buffer.type = wgpu::BufferBindingType::Uniform;
            entries[1].binding = 1;
            entries[1].visibility = wgpu::ShaderStage::Compute;
            entries[1].buffer.type = kInternalStorageBufferBinding;
            entries[2].binding = 2;
            entries[2].visibility = wgpu::ShaderStage::Compute;
            entries[2].buffer.type = wgpu::BufferBindingType::Storage;

            BindGroupLayoutDescriptor bindGroupLayoutDescriptor;
            bindGroupLayoutDescriptor.entryCount = entries.size();
            bindGroupLayoutDescriptor.entries = entries.data();
            Ref<BindGroupLayoutBase> bindGroupLayout;
            DAWN_TRY_ASSIGN(bindGroupLayout,
                            device->CreateBindGroupLayout(&bindGroupLayoutDescriptor, true));

            PipelineLayoutDescriptor pipelineDescriptor;
            pipelineDescriptor.bindGroupLayoutCount = 1;
            pipelineDescriptor.bindGroupLayouts = &bindGroupLayout.Get();
            Ref<PipelineLayoutBase> pipelineLayout;
            DAWN_TRY_ASSIGN(pipelineLayout, device->CreatePipelineLayout(&pipelineDescriptor));

            ComputePipelineDescriptor computePipelineDescriptor = {};
            computePipelineDescriptor.layout = pipelineLayout.Get();
            computePipelineDescriptor.compute.module = shaderModule.Get();
            computePipelineDescriptor.compute.entryPoint = "main";

            DAWN_TRY_ASSIGN(store->dispatchIndirectValidationPipeline,
                            device->CreateComputePipeline(&computePipelineDescriptor));

            return store->dispatchIndirectValidationPipeline.Get();
        }

    }  // namespace

    ComputePassEncoder::ComputePassEncoder(DeviceBase* device,
                                           CommandEncoder* commandEncoder,
                                           EncodingContext* encodingContext)
        : ProgrammablePassEncoder(device, encodingContext), mCommandEncoder(commandEncoder) {
    }

    ComputePassEncoder::ComputePassEncoder(DeviceBase* device,
                                           CommandEncoder* commandEncoder,
                                           EncodingContext* encodingContext,
                                           ErrorTag errorTag)
        : ProgrammablePassEncoder(device, encodingContext, errorTag),
          mCommandEncoder(commandEncoder) {
    }

    ComputePassEncoder* ComputePassEncoder::MakeError(DeviceBase* device,
                                                      CommandEncoder* commandEncoder,
                                                      EncodingContext* encodingContext) {
        return new ComputePassEncoder(device, commandEncoder, encodingContext, ObjectBase::kError);
    }

    ObjectType ComputePassEncoder::GetType() const {
        return ObjectType::ComputePassEncoder;
    }

    void ComputePassEncoder::APIEndPass() {
        if (mEncodingContext->TryEncode(
                this,
                [&](CommandAllocator* allocator) -> MaybeError {
                    if (IsValidationEnabled()) {
                        DAWN_TRY(ValidateProgrammableEncoderEnd());
                    }

                    allocator->Allocate<EndComputePassCmd>(Command::EndComputePass);

                    return {};
                },
                "encoding %s.EndPass().", this)) {
            mEncodingContext->ExitComputePass(this, mUsageTracker.AcquireResourceUsage());
        }
    }

    void ComputePassEncoder::APIDispatch(uint32_t x, uint32_t y, uint32_t z) {
        mEncodingContext->TryEncode(
            this,
            [&](CommandAllocator* allocator) -> MaybeError {
                if (IsValidationEnabled()) {
                    DAWN_TRY(mCommandBufferState.ValidateCanDispatch());

                    uint32_t workgroupsPerDimension =
                        GetDevice()->GetLimits().v1.maxComputeWorkgroupsPerDimension;

                    DAWN_INVALID_IF(
                        x > workgroupsPerDimension,
                        "Dispatch size X (%u) exceeds max compute workgroups per dimension (%u).",
                        x, workgroupsPerDimension);

                    DAWN_INVALID_IF(
                        y > workgroupsPerDimension,
                        "Dispatch size Y (%u) exceeds max compute workgroups per dimension (%u).",
                        y, workgroupsPerDimension);

                    DAWN_INVALID_IF(
                        z > workgroupsPerDimension,
                        "Dispatch size Z (%u) exceeds max compute workgroups per dimension (%u).",
                        z, workgroupsPerDimension);
                }

                // Record the synchronization scope for Dispatch, which is just the current
                // bindgroups.
                AddDispatchSyncScope();

                DispatchCmd* dispatch = allocator->Allocate<DispatchCmd>(Command::Dispatch);
                dispatch->x = x;
                dispatch->y = y;
                dispatch->z = z;

                return {};
            },
            "encoding %s.Dispatch(%u, %u, %u).", this, x, y, z);
    }

    ResultOrError<std::pair<Ref<BufferBase>, uint64_t>>
    ComputePassEncoder::ValidateIndirectDispatch(BufferBase* indirectBuffer,
                                                 uint64_t indirectOffset) {
        DeviceBase* device = GetDevice();
        auto* const store = device->GetInternalPipelineStore();

        Ref<ComputePipelineBase> validationPipeline;
        DAWN_TRY_ASSIGN(validationPipeline, GetOrCreateIndirectDispatchValidationPipeline(device));

        Ref<BindGroupLayoutBase> layout;
        DAWN_TRY_ASSIGN(layout, validationPipeline->GetBindGroupLayout(0));

        uint32_t storageBufferOffsetAlignment =
            device->GetLimits().v1.minStorageBufferOffsetAlignment;

        std::array<BindGroupEntry, 3> bindings;

        // Storage binding holding the client's indirect buffer.
        BindGroupEntry& clientIndirectBinding = bindings[0];
        clientIndirectBinding.binding = 1;
        clientIndirectBinding.buffer = indirectBuffer;

        // Let the offset be the indirectOffset, aligned down to |storageBufferOffsetAlignment|.
        const uint32_t clientOffsetFromAlignedBoundary =
            indirectOffset % storageBufferOffsetAlignment;
        const uint64_t clientOffsetAlignedDown = indirectOffset - clientOffsetFromAlignedBoundary;
        clientIndirectBinding.offset = clientOffsetAlignedDown;

        // Let the size of the binding be the additional offset, plus the size.
        clientIndirectBinding.size = kDispatchIndirectSize + clientOffsetFromAlignedBoundary;

        struct UniformParams {
            uint32_t maxComputeWorkgroupsPerDimension;
            uint32_t clientOffsetInU32;
        };

        // Create a uniform buffer to hold parameters for the shader.
        Ref<BufferBase> uniformBuffer;
        {
            BufferDescriptor uniformDesc = {};
            uniformDesc.size = sizeof(UniformParams);
            uniformDesc.usage = wgpu::BufferUsage::Uniform | wgpu::BufferUsage::CopyDst;
            uniformDesc.mappedAtCreation = true;
            DAWN_TRY_ASSIGN(uniformBuffer, device->CreateBuffer(&uniformDesc));

            UniformParams* params = static_cast<UniformParams*>(
                uniformBuffer->GetMappedRange(0, sizeof(UniformParams)));
            params->maxComputeWorkgroupsPerDimension =
                device->GetLimits().v1.maxComputeWorkgroupsPerDimension;
            params->clientOffsetInU32 = clientOffsetFromAlignedBoundary / sizeof(uint32_t);
            uniformBuffer->Unmap();
        }

        // Uniform buffer binding pointing to the uniform parameters.
        BindGroupEntry& uniformBinding = bindings[1];
        uniformBinding.binding = 0;
        uniformBinding.buffer = uniformBuffer.Get();
        uniformBinding.offset = 0;
        uniformBinding.size = sizeof(UniformParams);

        // Reserve space in the scratch buffer to hold the validated indirect params.
        ScratchBuffer& scratchBuffer = store->scratchIndirectStorage;
        DAWN_TRY(scratchBuffer.EnsureCapacity(kDispatchIndirectSize));
        Ref<BufferBase> validatedIndirectBuffer = scratchBuffer.GetBuffer();

        // Binding for the validated indirect params.
        BindGroupEntry& validatedParamsBinding = bindings[2];
        validatedParamsBinding.binding = 2;
        validatedParamsBinding.buffer = validatedIndirectBuffer.Get();
        validatedParamsBinding.offset = 0;
        validatedParamsBinding.size = kDispatchIndirectSize;

        BindGroupDescriptor bindGroupDescriptor = {};
        bindGroupDescriptor.layout = layout.Get();
        bindGroupDescriptor.entryCount = bindings.size();
        bindGroupDescriptor.entries = bindings.data();

        Ref<BindGroupBase> validationBindGroup;
        DAWN_TRY_ASSIGN(validationBindGroup, device->CreateBindGroup(&bindGroupDescriptor));

        // Issue commands to validate the indirect buffer.
        APISetPipeline(validationPipeline.Get());
        APISetBindGroup(0, validationBindGroup.Get());
        APIDispatch(1);

        // Return the new indirect buffer and indirect buffer offset.
        return std::make_pair(std::move(validatedIndirectBuffer), uint64_t(0));
    }

    void ComputePassEncoder::APIDispatchIndirect(BufferBase* indirectBuffer,
                                                 uint64_t indirectOffset) {
        mEncodingContext->TryEncode(
            this,
            [&](CommandAllocator* allocator) -> MaybeError {
                if (IsValidationEnabled()) {
                    DAWN_TRY(GetDevice()->ValidateObject(indirectBuffer));
                    DAWN_TRY(ValidateCanUseAs(indirectBuffer, wgpu::BufferUsage::Indirect));
                    DAWN_TRY(mCommandBufferState.ValidateCanDispatch());

                    DAWN_INVALID_IF(indirectOffset % 4 != 0,
                                    "Indirect offset (%u) is not a multiple of 4.", indirectOffset);

                    DAWN_INVALID_IF(
                        indirectOffset >= indirectBuffer->GetSize() ||
                            indirectOffset + kDispatchIndirectSize > indirectBuffer->GetSize(),
                        "Indirect offset (%u) and dispatch size (%u) exceeds the indirect buffer "
                        "size (%u).",
                        indirectOffset, kDispatchIndirectSize, indirectBuffer->GetSize());
                }

                SyncScopeUsageTracker scope;
                scope.BufferUsedAs(indirectBuffer, wgpu::BufferUsage::Indirect);
                mUsageTracker.AddReferencedBuffer(indirectBuffer);
                // TODO(crbug.com/dawn/1166): If validation is enabled, adding |indirectBuffer|
                // is needed for correct usage validation even though it will only be bound for
                // storage. This will unecessarily transition the |indirectBuffer| in
                // the backend.

                Ref<BufferBase> indirectBufferRef = indirectBuffer;
                if (IsValidationEnabled()) {
                    // Save the previous command buffer state so it can be restored after the
                    // validation inserts additional commands.
                    CommandBufferStateTracker previousState = mCommandBufferState;

                    // Validate each indirect dispatch with a single dispatch to copy the indirect
                    // buffer params into a scratch buffer if they're valid, and otherwise zero them
                    // out. We could consider moving the validation earlier in the pass after the
                    // last point the indirect buffer was used with writable usage, as well as batch
                    // validation for multiple dispatches into one, but inserting commands at
                    // arbitrary points in the past is not possible right now.
                    DAWN_TRY_ASSIGN(
                        std::tie(indirectBufferRef, indirectOffset),
                        ValidateIndirectDispatch(indirectBufferRef.Get(), indirectOffset));

                    // Restore the state.
                    RestoreCommandBufferState(std::move(previousState));

                    // |indirectBufferRef| was replaced with a scratch buffer. Add it to the
                    // synchronization scope.
                    ASSERT(indirectBufferRef.Get() != indirectBuffer);
                    scope.BufferUsedAs(indirectBufferRef.Get(), wgpu::BufferUsage::Indirect);
                    mUsageTracker.AddReferencedBuffer(indirectBufferRef.Get());
                }

                AddDispatchSyncScope(std::move(scope));

                DispatchIndirectCmd* dispatch =
                    allocator->Allocate<DispatchIndirectCmd>(Command::DispatchIndirect);
                dispatch->indirectBuffer = std::move(indirectBufferRef);
                dispatch->indirectOffset = indirectOffset;
                return {};
            },
            "encoding %s.DispatchIndirect(%s, %u).", this, indirectBuffer, indirectOffset);
    }

    void ComputePassEncoder::APISetPipeline(ComputePipelineBase* pipeline) {
        mEncodingContext->TryEncode(
            this,
            [&](CommandAllocator* allocator) -> MaybeError {
                if (IsValidationEnabled()) {
                    DAWN_TRY(GetDevice()->ValidateObject(pipeline));
                }

                mCommandBufferState.SetComputePipeline(pipeline);

                SetComputePipelineCmd* cmd =
                    allocator->Allocate<SetComputePipelineCmd>(Command::SetComputePipeline);
                cmd->pipeline = pipeline;

                return {};
            },
            "encoding %s.SetPipeline(%s).", this, pipeline);
    }

    void ComputePassEncoder::APISetBindGroup(uint32_t groupIndexIn,
                                             BindGroupBase* group,
                                             uint32_t dynamicOffsetCount,
                                             const uint32_t* dynamicOffsets) {
        mEncodingContext->TryEncode(
            this,
            [&](CommandAllocator* allocator) -> MaybeError {
                BindGroupIndex groupIndex(groupIndexIn);

                if (IsValidationEnabled()) {
                    DAWN_TRY(ValidateSetBindGroup(groupIndex, group, dynamicOffsetCount,
                                                  dynamicOffsets));
                }

                mUsageTracker.AddResourcesReferencedByBindGroup(group);
                RecordSetBindGroup(allocator, groupIndex, group, dynamicOffsetCount,
                                   dynamicOffsets);
                mCommandBufferState.SetBindGroup(groupIndex, group, dynamicOffsetCount,
                                                 dynamicOffsets);

                return {};
            },
            "encoding %s.SetBindGroup(%u, %s, %u, ...).", this, groupIndexIn, group,
            dynamicOffsetCount);
    }

    void ComputePassEncoder::APIWriteTimestamp(QuerySetBase* querySet, uint32_t queryIndex) {
        mEncodingContext->TryEncode(
            this,
            [&](CommandAllocator* allocator) -> MaybeError {
                if (IsValidationEnabled()) {
                    DAWN_TRY(GetDevice()->ValidateObject(querySet));
                    DAWN_TRY(ValidateTimestampQuery(querySet, queryIndex));
                }

                mCommandEncoder->TrackQueryAvailability(querySet, queryIndex);

                WriteTimestampCmd* cmd =
                    allocator->Allocate<WriteTimestampCmd>(Command::WriteTimestamp);
                cmd->querySet = querySet;
                cmd->queryIndex = queryIndex;

                return {};
            },
            "encoding %s.WriteTimestamp(%s, %u).", this, querySet, queryIndex);
    }

    void ComputePassEncoder::AddDispatchSyncScope(SyncScopeUsageTracker scope) {
        PipelineLayoutBase* layout = mCommandBufferState.GetPipelineLayout();
        for (BindGroupIndex i : IterateBitSet(layout->GetBindGroupLayoutsMask())) {
            scope.AddBindGroup(mCommandBufferState.GetBindGroup(i));
        }
        mUsageTracker.AddDispatch(scope.AcquireSyncScopeUsage());
    }

    void ComputePassEncoder::RestoreCommandBufferState(CommandBufferStateTracker state) {
        // Encode commands for the backend to restore the pipeline and bind groups.
        if (state.HasPipeline()) {
            APISetPipeline(state.GetComputePipeline());
        }
        for (BindGroupIndex i(0); i < kMaxBindGroupsTyped; ++i) {
            BindGroupBase* bg = state.GetBindGroup(i);
            if (bg != nullptr) {
                const std::vector<uint32_t>& offsets = state.GetDynamicOffsets(i);
                if (offsets.empty()) {
                    APISetBindGroup(static_cast<uint32_t>(i), bg);
                } else {
                    APISetBindGroup(static_cast<uint32_t>(i), bg, offsets.size(), offsets.data());
                }
            }
        }

        // Restore the frontend state tracking information.
        mCommandBufferState = std::move(state);
    }

    CommandBufferStateTracker* ComputePassEncoder::GetCommandBufferStateTrackerForTesting() {
        return &mCommandBufferState;
    }

}  // namespace dawn_native
