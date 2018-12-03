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

#include "dawn_native/Queue.h"

#include "dawn_native/Buffer.h"
#include "dawn_native/CommandBuffer.h"
#include "dawn_native/Device.h"
#include "dawn_native/Fence.h"
#include "dawn_native/FenceSignalTracker.h"
#include "dawn_native/Texture.h"

namespace dawn_native {

    // QueueBase

    QueueBase::QueueBase(DeviceBase* device) : ObjectBase(device) {
    }

    void QueueBase::Submit(uint32_t numCommands, CommandBufferBase* const* commands) {
        if (GetDevice()->ConsumedError(ValidateSubmit(numCommands, commands))) {
            return;
        }

        SubmitImpl(numCommands, commands);
    }

    void QueueBase::Signal(FenceBase* fence, uint64_t signalValue) {
        if (GetDevice()->ConsumedError(ValidateSignal(fence, signalValue))) {
            return;
        }

        fence->SetSignaledValue(signalValue);
        GetDevice()->GetFenceSignalTracker()->UpdateFenceOnComplete(fence, signalValue);
    }

    MaybeError QueueBase::ValidateSubmit(uint32_t numCommands, CommandBufferBase* const* commands) {
        for (uint32_t i = 0; i < numCommands; ++i) {
            const CommandBufferResourceUsage& usages = commands[i]->GetResourceUsages();

            for (const PassResourceUsage& passUsages : usages.perPass) {
                for (const BufferBase* buffer : passUsages.buffers) {
                    DAWN_TRY(buffer->ValidateCanUseInSubmitNow());
                }
                for (const TextureBase* texture : passUsages.textures) {
                    DAWN_TRY(texture->ValidateCanUseInSubmitNow());
                }
            }

            for (const BufferBase* buffer : usages.topLevelBuffers) {
                DAWN_TRY(buffer->ValidateCanUseInSubmitNow());
            }
            for (const TextureBase* texture : usages.topLevelTextures) {
                DAWN_TRY(texture->ValidateCanUseInSubmitNow());
            }
        }

        return {};
    }

    MaybeError QueueBase::ValidateSignal(const FenceBase* fence, uint64_t signalValue) {
        if (signalValue <= fence->GetSignaledValue()) {
            return DAWN_VALIDATION_ERROR("Signal value less than or equal to fence signaled value");
        }
        return {};
    }

}  // namespace dawn_native
