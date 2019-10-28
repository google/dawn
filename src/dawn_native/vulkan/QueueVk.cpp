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

#include "dawn_native/vulkan/QueueVk.h"

#include "dawn_native/vulkan/CommandBufferVk.h"
#include "dawn_native/vulkan/CommandRecordingContext.h"
#include "dawn_native/vulkan/DeviceVk.h"
#include "dawn_platform/DawnPlatform.h"
#include "dawn_platform/tracing/TraceEvent.h"

namespace dawn_native { namespace vulkan {

    // static
    Queue* Queue::Create(Device* device) {
        return new Queue(device);
    }

    Queue::~Queue() {
    }

    MaybeError Queue::SubmitImpl(uint32_t commandCount, CommandBufferBase* const* commands) {
        Device* device = ToBackend(GetDevice());

        device->Tick();

        TRACE_EVENT_BEGIN0(GetDevice()->GetPlatform(), Recording,
                           "CommandBufferVk::RecordCommands");
        CommandRecordingContext* recordingContext = device->GetPendingRecordingContext();
        for (uint32_t i = 0; i < commandCount; ++i) {
            DAWN_TRY(ToBackend(commands[i])->RecordCommands(recordingContext));
        }
        TRACE_EVENT_END0(GetDevice()->GetPlatform(), Recording, "CommandBufferVk::RecordCommands");

        DAWN_TRY(device->SubmitPendingCommands());

        return {};
    }

}}  // namespace dawn_native::vulkan
