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

#include "dawn/native/d3d12/QueueD3D12.h"

#include "dawn/common/Math.h"
#include "dawn/native/CommandValidation.h"
#include "dawn/native/Commands.h"
#include "dawn/native/DynamicUploader.h"
#include "dawn/native/d3d/D3DError.h"
#include "dawn/native/d3d12/CommandBufferD3D12.h"
#include "dawn/native/d3d12/DeviceD3D12.h"
#include "dawn/native/d3d12/UtilsD3D12.h"
#include "dawn/platform/DawnPlatform.h"
#include "dawn/platform/tracing/TraceEvent.h"

namespace dawn::native::d3d12 {

// static
Ref<Queue> Queue::Create(Device* device, const QueueDescriptor* descriptor) {
    Ref<Queue> queue = AcquireRef(new Queue(device, descriptor));
    queue->Initialize();
    return queue;
}

Queue::Queue(Device* device, const QueueDescriptor* descriptor) : QueueBase(device, descriptor) {}

void Queue::Initialize() {
    SetLabelImpl();
}

MaybeError Queue::SubmitImpl(uint32_t commandCount, CommandBufferBase* const* commands) {
    Device* device = ToBackend(GetDevice());

    CommandRecordingContext* commandContext;
    DAWN_TRY_ASSIGN(commandContext, device->GetPendingCommandContext());

    TRACE_EVENT_BEGIN1(GetDevice()->GetPlatform(), Recording, "CommandBufferD3D12::RecordCommands",
                       "serial", uint64_t(GetDevice()->GetPendingCommandSerial()));
    for (uint32_t i = 0; i < commandCount; ++i) {
        DAWN_TRY(ToBackend(commands[i])->RecordCommands(commandContext));
    }
    TRACE_EVENT_END1(GetDevice()->GetPlatform(), Recording, "CommandBufferD3D12::RecordCommands",
                     "serial", uint64_t(GetDevice()->GetPendingCommandSerial()));

    DAWN_TRY(device->ExecutePendingCommandContext());

    DAWN_TRY(device->NextSerial());

    return {};
}

bool Queue::HasPendingCommands() const {
    return ToBackend(GetDevice())->HasPendingCommands();
}

ResultOrError<ExecutionSerial> Queue::CheckAndUpdateCompletedSerials() {
    return ToBackend(GetDevice())->CheckAndUpdateCompletedSerials();
}

void Queue::ForceEventualFlushOfCommands() {
    return ToBackend(GetDevice())->ForceEventualFlushOfCommands();
}

MaybeError Queue::WaitForIdleForDestruction() {
    return ToBackend(GetDevice())->WaitForIdleForDestruction();
}

void Queue::SetLabelImpl() {
    Device* device = ToBackend(GetDevice());
    // TODO(crbug.com/dawn/1344): When we start using multiple queues this needs to be adjusted
    // so it doesn't always change the default queue's label.
    SetDebugName(device, device->GetCommandQueue().Get(), "Dawn_Queue", GetLabel());
}

}  // namespace dawn::native::d3d12
