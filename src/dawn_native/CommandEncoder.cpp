// Copyright 2019 The Dawn Authors
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

#include "dawn_native/CommandEncoder.h"

#include "dawn_native/CommandBuffer.h"
#include "dawn_native/Device.h"

namespace dawn_native {

    CommandEncoderBase::CommandEncoderBase(DeviceBase* device) : ObjectBase(device) {
        // Create a builder with an external reference count of 1. We don't use Ref<> because we
        // want to release the external reference when the encoder is destroyed.
        mBuilder = GetDevice()->CreateCommandBufferBuilder();
        mBuilder->SetErrorCallback(
            HandleBuilderError,
            static_cast<dawnCallbackUserdata>(reinterpret_cast<uintptr_t>(this)), 0);
    }

    CommandEncoderBase::~CommandEncoderBase() {
        // Release the single external reference of the builder
        mBuilder->Release();
        mBuilder = nullptr;
    }

    ComputePassEncoderBase* CommandEncoderBase::BeginComputePass() {
        return mBuilder->BeginComputePass();
    }

    RenderPassEncoderBase* CommandEncoderBase::BeginRenderPass(RenderPassDescriptorBase* info) {
        return mBuilder->BeginRenderPass(info);
    }

    void CommandEncoderBase::CopyBufferToBuffer(BufferBase* source,
                                                uint32_t sourceOffset,
                                                BufferBase* destination,
                                                uint32_t destinationOffset,
                                                uint32_t size) {
        return mBuilder->CopyBufferToBuffer(source, sourceOffset, destination, destinationOffset,
                                            size);
    }

    void CommandEncoderBase::CopyBufferToTexture(const BufferCopyView* source,
                                                 const TextureCopyView* destination,
                                                 const Extent3D* copySize) {
        return mBuilder->CopyBufferToTexture(source, destination, copySize);
    }

    void CommandEncoderBase::CopyTextureToBuffer(const TextureCopyView* source,
                                                 const BufferCopyView* destination,
                                                 const Extent3D* copySize) {
        return mBuilder->CopyTextureToBuffer(source, destination, copySize);
    }

    CommandBufferBase* CommandEncoderBase::Finish() {
        if (GetDevice()->ConsumedError(ValidateFinish())) {
            return CommandBufferBase::MakeError(GetDevice());
        }
        ASSERT(!IsError());

        CommandBufferBase* result = mBuilder->GetResult();
        if (result == nullptr) {
            ASSERT(mGotError);
            GetDevice()->ConsumedError(DAWN_VALIDATION_ERROR(mErrorMessage));
            return CommandBufferBase::MakeError(GetDevice());
        }

        ASSERT(result != nullptr);
        return result;
    }

    MaybeError CommandEncoderBase::ValidateFinish() {
        DAWN_TRY(GetDevice()->ValidateObject(this));
        DAWN_TRY(mBuilder->ValidateGetResult());
        return {};
    }

    // static
    void CommandEncoderBase::HandleBuilderError(dawnBuilderErrorStatus status,
                                                const char* message,
                                                dawnCallbackUserdata userdata1,
                                                dawnCallbackUserdata) {
        CommandEncoderBase* self =
            reinterpret_cast<CommandEncoderBase*>(static_cast<uintptr_t>(userdata1));

        if (status != DAWN_BUILDER_ERROR_STATUS_SUCCESS && !self->mGotError) {
            self->mGotError = true;
            self->mErrorMessage = message;
        }
    }

}  // namespace dawn_native
