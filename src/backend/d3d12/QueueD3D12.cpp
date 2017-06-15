// Copyright 2017 The NXT Authors
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

#include "QueueD3D12.h"

#include "D3D12Backend.h"
#include "CommandBufferD3D12.h"

namespace backend {
namespace d3d12 {

    Queue::Queue(Device* device, QueueBuilder* builder)
        : QueueBase(builder), device(device) {

        // TODO(enga@google.com): We don't need this allocator, but it's needed for command list initialization. Is there a better way to do this?
        // Is CommandList creation expensive or can it be done every Queue::Submit?
        ComPtr<ID3D12CommandAllocator> temporaryCommandAllocator;
        ASSERT_SUCCESS(device->GetD3D12Device()->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&temporaryCommandAllocator)));
        ASSERT_SUCCESS(device->GetD3D12Device()->CreateCommandList(
            0,
            D3D12_COMMAND_LIST_TYPE_DIRECT,
            temporaryCommandAllocator.Get(),
            nullptr,
            IID_PPV_ARGS(&commandList)
        ));
        ASSERT_SUCCESS(commandList->Close());
    }

    void Queue::Submit(uint32_t numCommands, CommandBuffer* const * commands) {
        device->Tick();

        ASSERT_SUCCESS(commandList->Reset(device->GetCommandAllocatorManager()->ReserveCommandAllocator().Get(), nullptr));
        for (uint32_t i = 0; i < numCommands; ++i) {
            commands[i]->FillCommands(commandList);
        }
        ASSERT_SUCCESS(commandList->Close());

        device->ExecuteCommandLists({ commandList.Get() });

        device->NextSerial();
    }

}
}
