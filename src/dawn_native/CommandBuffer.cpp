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

#include "dawn_native/CommandBuffer.h"

#include "dawn_native/BindGroup.h"
#include "dawn_native/Buffer.h"
#include "dawn_native/CommandBufferStateTracker.h"
#include "dawn_native/Commands.h"
#include "dawn_native/ComputePassEncoder.h"
#include "dawn_native/ComputePipeline.h"
#include "dawn_native/Device.h"
#include "dawn_native/ErrorData.h"
#include "dawn_native/InputState.h"
#include "dawn_native/PipelineLayout.h"
#include "dawn_native/RenderPassEncoder.h"
#include "dawn_native/RenderPipeline.h"
#include "dawn_native/Texture.h"

#include <cstring>
#include <map>

namespace dawn_native {

    namespace {

        MaybeError ValidateCopySizeFitsInTexture(const TextureCopy& textureCopy,
                                                 const Extent3D& copySize) {
            const TextureBase* texture = textureCopy.texture.Get();
            if (textureCopy.level >= texture->GetNumMipLevels()) {
                return DAWN_VALIDATION_ERROR("Copy mip-level out of range");
            }

            if (textureCopy.slice >= texture->GetArrayLayers()) {
                return DAWN_VALIDATION_ERROR("Copy array-layer out of range");
            }

            // All texture dimensions are in uint32_t so by doing checks in uint64_t we avoid
            // overflows.
            uint64_t level = textureCopy.level;
            if (uint64_t(textureCopy.origin.x) + uint64_t(copySize.width) >
                    (static_cast<uint64_t>(texture->GetSize().width) >> level) ||
                uint64_t(textureCopy.origin.y) + uint64_t(copySize.height) >
                    (static_cast<uint64_t>(texture->GetSize().height) >> level)) {
                return DAWN_VALIDATION_ERROR("Copy would touch outside of the texture");
            }

            // TODO(cwallez@chromium.org): Check the depth bound differently for 2D arrays and 3D
            // textures
            if (textureCopy.origin.z != 0 || copySize.depth != 1) {
                return DAWN_VALIDATION_ERROR("No support for z != 0 and depth != 1 for now");
            }

            return {};
        }

        bool FitsInBuffer(const BufferBase* buffer, uint32_t offset, uint32_t size) {
            uint32_t bufferSize = buffer->GetSize();
            return offset <= bufferSize && (size <= (bufferSize - offset));
        }

        MaybeError ValidateCopySizeFitsInBuffer(const BufferCopy& bufferCopy, uint32_t dataSize) {
            if (!FitsInBuffer(bufferCopy.buffer.Get(), bufferCopy.offset, dataSize)) {
                return DAWN_VALIDATION_ERROR("Copy would overflow the buffer");
            }

            return {};
        }

        MaybeError ValidateTexelBufferOffset(TextureBase* texture, const BufferCopy& bufferCopy) {
            uint32_t texelSize =
                static_cast<uint32_t>(TextureFormatPixelSize(texture->GetFormat()));
            if (bufferCopy.offset % texelSize != 0) {
                return DAWN_VALIDATION_ERROR("Buffer offset must be a multiple of the texel size");
            }

            return {};
        }

        MaybeError ValidateImageHeight(uint32_t imageHeight, uint32_t copyHeight) {
            if (imageHeight < copyHeight) {
                return DAWN_VALIDATION_ERROR("Image height must not be less than the copy height.");
            }

            return {};
        }

        MaybeError ComputeTextureCopyBufferSize(const Extent3D& copySize,
                                                uint32_t rowPitch,
                                                uint32_t imageHeight,
                                                uint32_t* bufferSize) {
            DAWN_TRY(ValidateImageHeight(imageHeight, copySize.height));

            // TODO(cwallez@chromium.org): check for overflows
            uint32_t slicePitch = rowPitch * imageHeight;
            uint32_t sliceSize = rowPitch * (copySize.height - 1) + copySize.width;
            *bufferSize = (slicePitch * (copySize.depth - 1)) + sliceSize;

            return {};
        }

        uint32_t ComputeDefaultRowPitch(TextureBase* texture, uint32_t width) {
            uint32_t texelSize = TextureFormatPixelSize(texture->GetFormat());
            return texelSize * width;
        }

        MaybeError ValidateRowPitch(dawn::TextureFormat format,
                                    const Extent3D& copySize,
                                    uint32_t rowPitch) {
            if (rowPitch % kTextureRowPitchAlignment != 0) {
                return DAWN_VALIDATION_ERROR("Row pitch must be a multiple of 256");
            }

            uint32_t texelSize = TextureFormatPixelSize(format);
            if (rowPitch < copySize.width * texelSize) {
                return DAWN_VALIDATION_ERROR(
                    "Row pitch must not be less than the number of bytes per row");
            }

            return {};
        }

        MaybeError ValidateCanUseAs(BufferBase* buffer, dawn::BufferUsageBit usage) {
            ASSERT(HasZeroOrOneBits(usage));
            if (!(buffer->GetUsage() & usage)) {
                return DAWN_VALIDATION_ERROR("buffer doesn't have the required usage.");
            }

            return {};
        }

        MaybeError ValidateCanUseAs(TextureBase* texture, dawn::TextureUsageBit usage) {
            ASSERT(HasZeroOrOneBits(usage));
            if (!(texture->GetUsage() & usage)) {
                return DAWN_VALIDATION_ERROR("texture doesn't have the required usage.");
            }

            return {};
        }

        enum class PassType {
            Render,
            Compute,
        };

        // Helper class to encapsulate the logic of tracking per-resource usage during the
        // validation of command buffer passes. It is used both to know if there are validation
        // errors, and to get a list of resources used per pass for backends that need the
        // information.
        class PassResourceUsageTracker {
          public:
            void BufferUsedAs(BufferBase* buffer, dawn::BufferUsageBit usage) {
                // std::map's operator[] will create the key and return 0 if the key didn't exist
                // before.
                dawn::BufferUsageBit& storedUsage = mBufferUsages[buffer];

                if (usage == dawn::BufferUsageBit::Storage &&
                    storedUsage & dawn::BufferUsageBit::Storage) {
                    mStorageUsedMultipleTimes = true;
                }

                storedUsage |= usage;
            }

            void TextureUsedAs(TextureBase* texture, dawn::TextureUsageBit usage) {
                // std::map's operator[] will create the key and return 0 if the key didn't exist
                // before.
                dawn::TextureUsageBit& storedUsage = mTextureUsages[texture];

                if (usage == dawn::TextureUsageBit::Storage &&
                    storedUsage & dawn::TextureUsageBit::Storage) {
                    mStorageUsedMultipleTimes = true;
                }

                storedUsage |= usage;
            }

            // Performs the per-pass usage validation checks
            MaybeError ValidateUsages(PassType pass) const {
                // Storage resources cannot be used twice in the same compute pass
                if (pass == PassType::Compute && mStorageUsedMultipleTimes) {
                    return DAWN_VALIDATION_ERROR(
                        "Storage resource used multiple times in compute pass");
                }

                // Buffers can only be used as single-write or multiple read.
                for (auto& it : mBufferUsages) {
                    BufferBase* buffer = it.first;
                    dawn::BufferUsageBit usage = it.second;

                    if (usage & ~buffer->GetUsage()) {
                        return DAWN_VALIDATION_ERROR("Buffer missing usage for the pass");
                    }

                    bool readOnly = (usage & kReadOnlyBufferUsages) == usage;
                    bool singleUse = dawn::HasZeroOrOneBits(usage);

                    if (!readOnly && !singleUse) {
                        return DAWN_VALIDATION_ERROR(
                            "Buffer used as writeable usage and another usage in pass");
                    }
                }

                // Textures can only be used as single-write or multiple read.
                // TODO(cwallez@chromium.org): implement per-subresource tracking
                for (auto& it : mTextureUsages) {
                    TextureBase* texture = it.first;
                    dawn::TextureUsageBit usage = it.second;

                    if (usage & ~texture->GetUsage()) {
                        return DAWN_VALIDATION_ERROR("Texture missing usage for the pass");
                    }

                    // For textures the only read-only usage in a pass is Sampled, so checking the
                    // usage constraint simplifies to checking a single usage bit is set.
                    if (!dawn::HasZeroOrOneBits(it.second)) {
                        return DAWN_VALIDATION_ERROR(
                            "Texture used with more than one usage in pass");
                    }
                }

                return {};
            }

            // Returns the per-pass usage for use by backends for APIs with explicit barriers.
            PassResourceUsage AcquireResourceUsage() {
                PassResourceUsage result;
                result.buffers.reserve(mBufferUsages.size());
                result.bufferUsages.reserve(mBufferUsages.size());
                result.textures.reserve(mTextureUsages.size());
                result.textureUsages.reserve(mTextureUsages.size());

                for (auto& it : mBufferUsages) {
                    result.buffers.push_back(it.first);
                    result.bufferUsages.push_back(it.second);
                }

                for (auto& it : mTextureUsages) {
                    result.textures.push_back(it.first);
                    result.textureUsages.push_back(it.second);
                }

                return result;
            }

          private:
            std::map<BufferBase*, dawn::BufferUsageBit> mBufferUsages;
            std::map<TextureBase*, dawn::TextureUsageBit> mTextureUsages;
            bool mStorageUsedMultipleTimes = false;
        };

        void TrackBindGroupResourceUsage(BindGroupBase* group, PassResourceUsageTracker* tracker) {
            const auto& layoutInfo = group->GetLayout()->GetBindingInfo();

            for (uint32_t i : IterateBitSet(layoutInfo.mask)) {
                dawn::BindingType type = layoutInfo.types[i];

                switch (type) {
                    case dawn::BindingType::UniformBuffer: {
                        BufferBase* buffer = group->GetBindingAsBufferBinding(i).buffer;
                        tracker->BufferUsedAs(buffer, dawn::BufferUsageBit::Uniform);
                    } break;

                    case dawn::BindingType::StorageBuffer: {
                        BufferBase* buffer = group->GetBindingAsBufferBinding(i).buffer;
                        tracker->BufferUsedAs(buffer, dawn::BufferUsageBit::Storage);
                    } break;

                    case dawn::BindingType::SampledTexture: {
                        TextureBase* texture = group->GetBindingAsTextureView(i)->GetTexture();
                        tracker->TextureUsedAs(texture, dawn::TextureUsageBit::Sampled);
                    } break;

                    case dawn::BindingType::Sampler:
                        break;
                }
            }
        }

    }  // namespace

    enum class CommandBufferBuilder::EncodingState : uint8_t {
        TopLevel,
        ComputePass,
        RenderPass,
        Finished
    };

    // CommandBuffer

    CommandBufferBase::CommandBufferBase(CommandBufferBuilder* builder)
        : ObjectBase(builder->GetDevice()), mResourceUsages(builder->AcquireResourceUsages()) {
    }

    const CommandBufferResourceUsage& CommandBufferBase::GetResourceUsages() const {
        return mResourceUsages;
    }

    // CommandBufferBuilder

    CommandBufferBuilder::CommandBufferBuilder(DeviceBase* device)
        : Builder(device), mEncodingState(EncodingState::TopLevel) {
    }

    CommandBufferBuilder::~CommandBufferBuilder() {
        if (!mWereCommandsAcquired) {
            MoveToIterator();
            FreeCommands(&mIterator);
        }
    }

    CommandIterator CommandBufferBuilder::AcquireCommands() {
        ASSERT(!mWereCommandsAcquired);
        mWereCommandsAcquired = true;
        return std::move(mIterator);
    }

    CommandBufferResourceUsage CommandBufferBuilder::AcquireResourceUsages() {
        ASSERT(!mWereResourceUsagesAcquired);
        mWereResourceUsagesAcquired = true;
        return std::move(mResourceUsages);
    }

    CommandBufferBase* CommandBufferBuilder::GetResultImpl() {
        mEncodingState = EncodingState::Finished;

        MoveToIterator();
        return GetDevice()->CreateCommandBuffer(this);
    }

    void CommandBufferBuilder::MoveToIterator() {
        if (!mWasMovedToIterator) {
            mIterator = std::move(mAllocator);
            mWasMovedToIterator = true;
        }
    }

    void CommandBufferBuilder::PassEnded() {
        if (mEncodingState == EncodingState::ComputePass) {
            mAllocator.Allocate<EndComputePassCmd>(Command::EndComputePass);
        } else {
            ASSERT(mEncodingState == EncodingState::RenderPass);
            mAllocator.Allocate<EndRenderPassCmd>(Command::EndRenderPass);
        }
        mEncodingState = EncodingState::TopLevel;
    }

    void CommandBufferBuilder::ConsumeError(ErrorData* error) {
        HandleError(error->GetMessage().c_str());
        delete error;
    }

    // Implementation of the command buffer validation that can be precomputed before submit

    MaybeError CommandBufferBuilder::ValidateGetResult() {
        if (mEncodingState != EncodingState::TopLevel) {
            return DAWN_VALIDATION_ERROR("Command buffer recording ended mid-pass");
        }

        MoveToIterator();
        mIterator.Reset();

        Command type;
        while (mIterator.NextCommandId(&type)) {
            switch (type) {
                case Command::BeginComputePass: {
                    mIterator.NextCommand<BeginComputePassCmd>();
                    DAWN_TRY(ValidateComputePass());
                } break;

                case Command::BeginRenderPass: {
                    BeginRenderPassCmd* cmd = mIterator.NextCommand<BeginRenderPassCmd>();
                    DAWN_TRY(ValidateRenderPass(cmd->info.Get()));
                } break;

                case Command::CopyBufferToBuffer: {
                    CopyBufferToBufferCmd* copy = mIterator.NextCommand<CopyBufferToBufferCmd>();

                    DAWN_TRY(ValidateCopySizeFitsInBuffer(copy->source, copy->size));
                    DAWN_TRY(ValidateCopySizeFitsInBuffer(copy->destination, copy->size));

                    DAWN_TRY(ValidateCanUseAs(copy->source.buffer.Get(),
                                              dawn::BufferUsageBit::TransferSrc));
                    DAWN_TRY(ValidateCanUseAs(copy->destination.buffer.Get(),
                                              dawn::BufferUsageBit::TransferDst));

                    mResourceUsages.topLevelBuffers.insert(copy->source.buffer.Get());
                    mResourceUsages.topLevelBuffers.insert(copy->destination.buffer.Get());
                } break;

                case Command::CopyBufferToTexture: {
                    CopyBufferToTextureCmd* copy = mIterator.NextCommand<CopyBufferToTextureCmd>();

                    uint32_t bufferCopySize = 0;
                    DAWN_TRY(ValidateRowPitch(copy->destination.texture->GetFormat(),
                                              copy->copySize, copy->source.rowPitch));

                    DAWN_TRY(ComputeTextureCopyBufferSize(copy->copySize, copy->source.rowPitch,
                                                          copy->source.imageHeight,
                                                          &bufferCopySize));

                    DAWN_TRY(ValidateCopySizeFitsInTexture(copy->destination, copy->copySize));
                    DAWN_TRY(ValidateCopySizeFitsInBuffer(copy->source, bufferCopySize));
                    DAWN_TRY(
                        ValidateTexelBufferOffset(copy->destination.texture.Get(), copy->source));

                    DAWN_TRY(ValidateCanUseAs(copy->source.buffer.Get(),
                                              dawn::BufferUsageBit::TransferSrc));
                    DAWN_TRY(ValidateCanUseAs(copy->destination.texture.Get(),
                                              dawn::TextureUsageBit::TransferDst));

                    mResourceUsages.topLevelBuffers.insert(copy->source.buffer.Get());
                    mResourceUsages.topLevelTextures.insert(copy->destination.texture.Get());
                } break;

                case Command::CopyTextureToBuffer: {
                    CopyTextureToBufferCmd* copy = mIterator.NextCommand<CopyTextureToBufferCmd>();

                    uint32_t bufferCopySize = 0;
                    DAWN_TRY(ValidateRowPitch(copy->source.texture->GetFormat(), copy->copySize,
                                              copy->destination.rowPitch));
                    DAWN_TRY(ComputeTextureCopyBufferSize(
                        copy->copySize, copy->destination.rowPitch, copy->destination.imageHeight,
                        &bufferCopySize));

                    DAWN_TRY(ValidateCopySizeFitsInTexture(copy->source, copy->copySize));
                    DAWN_TRY(ValidateCopySizeFitsInBuffer(copy->destination, bufferCopySize));
                    DAWN_TRY(
                        ValidateTexelBufferOffset(copy->source.texture.Get(), copy->destination));

                    DAWN_TRY(ValidateCanUseAs(copy->source.texture.Get(),
                                              dawn::TextureUsageBit::TransferSrc));
                    DAWN_TRY(ValidateCanUseAs(copy->destination.buffer.Get(),
                                              dawn::BufferUsageBit::TransferDst));

                    mResourceUsages.topLevelTextures.insert(copy->source.texture.Get());
                    mResourceUsages.topLevelBuffers.insert(copy->destination.buffer.Get());
                } break;

                default:
                    return DAWN_VALIDATION_ERROR("Command disallowed outside of a pass");
            }
        }

        return {};
    }

    MaybeError CommandBufferBuilder::ValidateComputePass() {
        PassResourceUsageTracker usageTracker;
        CommandBufferStateTracker persistentState;

        Command type;
        while (mIterator.NextCommandId(&type)) {
            switch (type) {
                case Command::EndComputePass: {
                    mIterator.NextCommand<EndComputePassCmd>();

                    DAWN_TRY(usageTracker.ValidateUsages(PassType::Compute));
                    mResourceUsages.perPass.push_back(usageTracker.AcquireResourceUsage());
                    return {};
                } break;

                case Command::Dispatch: {
                    mIterator.NextCommand<DispatchCmd>();
                    DAWN_TRY(persistentState.ValidateCanDispatch());
                } break;

                case Command::SetComputePipeline: {
                    SetComputePipelineCmd* cmd = mIterator.NextCommand<SetComputePipelineCmd>();
                    ComputePipelineBase* pipeline = cmd->pipeline.Get();
                    persistentState.SetComputePipeline(pipeline);
                } break;

                case Command::SetPushConstants: {
                    SetPushConstantsCmd* cmd = mIterator.NextCommand<SetPushConstantsCmd>();
                    mIterator.NextData<uint32_t>(cmd->count);
                    // Validation of count and offset has already been done when the command was
                    // recorded because it impacts the size of an allocation in the
                    // CommandAllocator.
                    if (cmd->stages & ~dawn::ShaderStageBit::Compute) {
                        return DAWN_VALIDATION_ERROR(
                            "SetPushConstants stage must be compute or 0 in compute passes");
                    }
                } break;

                case Command::SetBindGroup: {
                    SetBindGroupCmd* cmd = mIterator.NextCommand<SetBindGroupCmd>();

                    TrackBindGroupResourceUsage(cmd->group.Get(), &usageTracker);
                    persistentState.SetBindGroup(cmd->index, cmd->group.Get());
                } break;

                default:
                    return DAWN_VALIDATION_ERROR("Command disallowed inside a compute pass");
            }
        }

        UNREACHABLE();
        return DAWN_VALIDATION_ERROR("Unfinished compute pass");
    }

    MaybeError CommandBufferBuilder::ValidateRenderPass(RenderPassDescriptorBase* renderPass) {
        PassResourceUsageTracker usageTracker;
        CommandBufferStateTracker persistentState;

        // Track usage of the render pass attachments
        for (uint32_t i : IterateBitSet(renderPass->GetColorAttachmentMask())) {
            TextureBase* texture = renderPass->GetColorAttachment(i).view->GetTexture();
            usageTracker.TextureUsedAs(texture, dawn::TextureUsageBit::OutputAttachment);
        }

        if (renderPass->HasDepthStencilAttachment()) {
            TextureBase* texture = renderPass->GetDepthStencilAttachment().view->GetTexture();
            usageTracker.TextureUsedAs(texture, dawn::TextureUsageBit::OutputAttachment);
        }

        Command type;
        while (mIterator.NextCommandId(&type)) {
            switch (type) {
                case Command::EndRenderPass: {
                    mIterator.NextCommand<EndRenderPassCmd>();

                    DAWN_TRY(usageTracker.ValidateUsages(PassType::Render));
                    mResourceUsages.perPass.push_back(usageTracker.AcquireResourceUsage());
                    return {};
                } break;

                case Command::Draw: {
                    mIterator.NextCommand<DrawCmd>();
                    DAWN_TRY(persistentState.ValidateCanDraw());
                } break;

                case Command::DrawIndexed: {
                    mIterator.NextCommand<DrawIndexedCmd>();
                    DAWN_TRY(persistentState.ValidateCanDrawIndexed());
                } break;

                case Command::SetRenderPipeline: {
                    SetRenderPipelineCmd* cmd = mIterator.NextCommand<SetRenderPipelineCmd>();
                    RenderPipelineBase* pipeline = cmd->pipeline.Get();

                    if (!pipeline->IsCompatibleWith(renderPass)) {
                        return DAWN_VALIDATION_ERROR(
                            "Pipeline is incompatible with this render pass");
                    }

                    persistentState.SetRenderPipeline(pipeline);
                } break;

                case Command::SetPushConstants: {
                    SetPushConstantsCmd* cmd = mIterator.NextCommand<SetPushConstantsCmd>();
                    mIterator.NextData<uint32_t>(cmd->count);
                    // Validation of count and offset has already been done when the command was
                    // recorded because it impacts the size of an allocation in the
                    // CommandAllocator.
                    if (cmd->stages &
                        ~(dawn::ShaderStageBit::Vertex | dawn::ShaderStageBit::Fragment)) {
                        return DAWN_VALIDATION_ERROR(
                            "SetPushConstants stage must be a subset of (vertex|fragment) in "
                            "render passes");
                    }
                } break;

                case Command::SetStencilReference: {
                    mIterator.NextCommand<SetStencilReferenceCmd>();
                } break;

                case Command::SetBlendColor: {
                    mIterator.NextCommand<SetBlendColorCmd>();
                } break;

                case Command::SetScissorRect: {
                    mIterator.NextCommand<SetScissorRectCmd>();
                } break;

                case Command::SetBindGroup: {
                    SetBindGroupCmd* cmd = mIterator.NextCommand<SetBindGroupCmd>();

                    TrackBindGroupResourceUsage(cmd->group.Get(), &usageTracker);
                    persistentState.SetBindGroup(cmd->index, cmd->group.Get());
                } break;

                case Command::SetIndexBuffer: {
                    SetIndexBufferCmd* cmd = mIterator.NextCommand<SetIndexBufferCmd>();

                    usageTracker.BufferUsedAs(cmd->buffer.Get(), dawn::BufferUsageBit::Index);
                    persistentState.SetIndexBuffer();
                } break;

                case Command::SetVertexBuffers: {
                    SetVertexBuffersCmd* cmd = mIterator.NextCommand<SetVertexBuffersCmd>();
                    auto buffers = mIterator.NextData<Ref<BufferBase>>(cmd->count);
                    mIterator.NextData<uint32_t>(cmd->count);

                    for (uint32_t i = 0; i < cmd->count; ++i) {
                        usageTracker.BufferUsedAs(buffers[i].Get(), dawn::BufferUsageBit::Vertex);
                    }
                    persistentState.SetVertexBuffer(cmd->startSlot, cmd->count);
                } break;

                default:
                    return DAWN_VALIDATION_ERROR("Command disallowed inside a render pass");
            }
        }

        UNREACHABLE();
        return DAWN_VALIDATION_ERROR("Unfinished render pass");
    }

    MaybeError CommandBufferBuilder::ValidateCanRecordTopLevelCommands() const {
        if (mEncodingState != EncodingState::TopLevel) {
            return DAWN_VALIDATION_ERROR("Command cannot be recorded inside a pass");
        }
        return {};
    }

    // Implementation of the API's command recording methods

    ComputePassEncoderBase* CommandBufferBuilder::BeginComputePass() {
        if (ConsumedError(ValidateCanRecordTopLevelCommands())) {
            return nullptr;
        }

        mAllocator.Allocate<BeginComputePassCmd>(Command::BeginComputePass);

        mEncodingState = EncodingState::ComputePass;
        return new ComputePassEncoderBase(GetDevice(), this, &mAllocator);
    }

    RenderPassEncoderBase* CommandBufferBuilder::BeginRenderPass(RenderPassDescriptorBase* info) {
        if (ConsumedError(ValidateCanRecordTopLevelCommands())) {
            return nullptr;
        }

        if (info == nullptr) {
            HandleError("RenderPassDescriptor cannot be null");
            return nullptr;
        }

        BeginRenderPassCmd* cmd = mAllocator.Allocate<BeginRenderPassCmd>(Command::BeginRenderPass);
        new (cmd) BeginRenderPassCmd;
        cmd->info = info;

        mEncodingState = EncodingState::RenderPass;
        return new RenderPassEncoderBase(GetDevice(), this, &mAllocator);
    }

    void CommandBufferBuilder::CopyBufferToBuffer(BufferBase* source,
                                                  uint32_t sourceOffset,
                                                  BufferBase* destination,
                                                  uint32_t destinationOffset,
                                                  uint32_t size) {
        if (ConsumedError(ValidateCanRecordTopLevelCommands())) {
            return;
        }

        if (source == nullptr) {
            HandleError("Source cannot be null");
            return;
        }

        if (destination == nullptr) {
            HandleError("Destination cannot be null");
            return;
        }

        CopyBufferToBufferCmd* copy =
            mAllocator.Allocate<CopyBufferToBufferCmd>(Command::CopyBufferToBuffer);
        new (copy) CopyBufferToBufferCmd;
        copy->source.buffer = source;
        copy->source.offset = sourceOffset;
        copy->destination.buffer = destination;
        copy->destination.offset = destinationOffset;
        copy->size = size;
    }

    void CommandBufferBuilder::CopyBufferToTexture(const BufferCopyView* source,
                                                   const TextureCopyView* destination,
                                                   const Extent3D* copySize) {
        if (ConsumedError(ValidateCanRecordTopLevelCommands())) {
            return;
        }

        if (source->buffer == nullptr) {
            HandleError("Buffer cannot be null");
            return;
        }

        if (destination->texture == nullptr) {
            HandleError("Texture cannot be null");
            return;
        }

        CopyBufferToTextureCmd* copy =
            mAllocator.Allocate<CopyBufferToTextureCmd>(Command::CopyBufferToTexture);
        new (copy) CopyBufferToTextureCmd;
        copy->source.buffer = source->buffer;
        copy->source.offset = source->offset;
        copy->destination.texture = destination->texture;
        copy->destination.origin = destination->origin;
        copy->copySize = *copySize;
        copy->destination.level = destination->level;
        copy->destination.slice = destination->slice;
        if (source->rowPitch == 0) {
            copy->source.rowPitch = ComputeDefaultRowPitch(destination->texture, copySize->width);
        } else {
            copy->source.rowPitch = source->rowPitch;
        }
        if (source->imageHeight == 0) {
            copy->source.imageHeight = copySize->height;
        } else {
            copy->source.imageHeight = source->imageHeight;
        }
    }

    void CommandBufferBuilder::CopyTextureToBuffer(const TextureCopyView* source,
                                                   const BufferCopyView* destination,
                                                   const Extent3D* copySize) {
        if (ConsumedError(ValidateCanRecordTopLevelCommands())) {
            return;
        }

        if (source->texture == nullptr) {
            HandleError("Texture cannot be null");
            return;
        }

        if (destination->buffer == nullptr) {
            HandleError("Buffer cannot be null");
            return;
        }

        CopyTextureToBufferCmd* copy =
            mAllocator.Allocate<CopyTextureToBufferCmd>(Command::CopyTextureToBuffer);
        new (copy) CopyTextureToBufferCmd;
        copy->source.texture = source->texture;
        copy->source.origin = source->origin;
        copy->copySize = *copySize;
        copy->source.level = source->level;
        copy->source.slice = source->slice;
        copy->destination.buffer = destination->buffer;
        copy->destination.offset = destination->offset;
        if (destination->rowPitch == 0) {
            copy->destination.rowPitch = ComputeDefaultRowPitch(source->texture, copySize->width);
        } else {
            copy->destination.rowPitch = destination->rowPitch;
        }
        if (destination->imageHeight == 0) {
            copy->destination.imageHeight = copySize->height;
        } else {
            copy->destination.imageHeight = destination->imageHeight;
        }
    }

}  // namespace dawn_native
