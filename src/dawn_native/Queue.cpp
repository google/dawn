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
#include "dawn_native/ErrorScope.h"
#include "dawn_native/ErrorScopeTracker.h"
#include "dawn_native/Fence.h"
#include "dawn_native/FenceSignalTracker.h"
#include "dawn_native/Texture.h"
#include "dawn_platform/DawnPlatform.h"
#include "dawn_platform/tracing/TraceEvent.h"

namespace dawn_native {

    // QueueBase

    QueueBase::QueueBase(DeviceBase* device) : ObjectBase(device) {
    }

    QueueBase::QueueBase(DeviceBase* device, ObjectBase::ErrorTag tag) : ObjectBase(device, tag) {
    }

    // static
    QueueBase* QueueBase::MakeError(DeviceBase* device) {
        return new QueueBase(device, ObjectBase::kError);
    }

    MaybeError QueueBase::SubmitImpl(uint32_t commandCount, CommandBufferBase* const* commands) {
        UNREACHABLE();
        return {};
    }

    void QueueBase::Submit(uint32_t commandCount, CommandBufferBase* const* commands) {
        DeviceBase* device = GetDevice();
        if (device->ConsumedError(device->ValidateIsAlive())) {
            // If device is lost, don't let any commands be submitted
            return;
        }

        TRACE_EVENT0(device->GetPlatform(), General, "Queue::Submit");
        if (device->IsValidationEnabled() &&
            device->ConsumedError(ValidateSubmit(commandCount, commands))) {
            return;
        }
        ASSERT(!IsError());

        if (device->ConsumedError(SubmitImpl(commandCount, commands))) {
            return;
        }
        device->GetErrorScopeTracker()->TrackUntilLastSubmitComplete(
            device->GetCurrentErrorScope());
    }

    void QueueBase::Signal(Fence* fence, uint64_t signalValue) {
        DeviceBase* device = GetDevice();
        if (device->ConsumedError(ValidateSignal(fence, signalValue))) {
            return;
        }
        ASSERT(!IsError());

        fence->SetSignaledValue(signalValue);
        device->GetFenceSignalTracker()->UpdateFenceOnComplete(fence, signalValue);
        device->GetErrorScopeTracker()->TrackUntilLastSubmitComplete(
            device->GetCurrentErrorScope());
    }

    Fence* QueueBase::CreateFence(const FenceDescriptor* descriptor) {
        if (GetDevice()->ConsumedError(ValidateCreateFence(descriptor))) {
            return Fence::MakeError(GetDevice());
        }

        return new Fence(this, descriptor);
    }

    MaybeError QueueBase::ValidateSubmit(uint32_t commandCount,
                                         CommandBufferBase* const* commands) {
        TRACE_EVENT0(GetDevice()->GetPlatform(), Validation, "Queue::ValidateSubmit");
        DAWN_TRY(GetDevice()->ValidateObject(this));

        for (uint32_t i = 0; i < commandCount; ++i) {
            DAWN_TRY(GetDevice()->ValidateObject(commands[i]));

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

    MaybeError QueueBase::ValidateSignal(const Fence* fence, uint64_t signalValue) {
        DAWN_TRY(GetDevice()->ValidateIsAlive());
        DAWN_TRY(GetDevice()->ValidateObject(this));
        DAWN_TRY(GetDevice()->ValidateObject(fence));

        if (fence->GetQueue() != this) {
            return DAWN_VALIDATION_ERROR(
                "Fence must be signaled on the queue on which it was created.");
        }
        if (signalValue <= fence->GetSignaledValue()) {
            return DAWN_VALIDATION_ERROR("Signal value less than or equal to fence signaled value");
        }
        return {};
    }

    MaybeError QueueBase::ValidateCreateFence(const FenceDescriptor* descriptor) {
        DAWN_TRY(GetDevice()->ValidateIsAlive());
        DAWN_TRY(GetDevice()->ValidateObject(this));
        DAWN_TRY(ValidateFenceDescriptor(descriptor));

        return {};
    }

}  // namespace dawn_native
