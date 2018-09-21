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

#include "dawn_native/ProgrammablePassEncoder.h"

#include "dawn_native/BindGroup.h"
#include "dawn_native/CommandBuffer.h"
#include "dawn_native/Commands.h"

namespace dawn_native {

    ProgrammablePassEncoder::ProgrammablePassEncoder(DeviceBase* device,
                                                     CommandBufferBuilder* topLevelBuilder,
                                                     CommandAllocator* allocator)
        : mDevice(device), mTopLevelBuilder(topLevelBuilder), mAllocator(allocator) {
    }

    void ProgrammablePassEncoder::EndPass() {
        mTopLevelBuilder->PassEnded();
        mAllocator = nullptr;
    }

    void ProgrammablePassEncoder::SetBindGroup(uint32_t groupIndex, BindGroupBase* group) {
        if (mTopLevelBuilder->ConsumedError(ValidateCanRecordCommands())) {
            return;
        }

        if (groupIndex >= kMaxBindGroups) {
            mTopLevelBuilder->HandleError("Setting bind group over the max");
            return;
        }

        SetBindGroupCmd* cmd = mAllocator->Allocate<SetBindGroupCmd>(Command::SetBindGroup);
        new (cmd) SetBindGroupCmd;
        cmd->index = groupIndex;
        cmd->group = group;
    }

    void ProgrammablePassEncoder::SetPushConstants(dawn::ShaderStageBit stages,
                                                   uint32_t offset,
                                                   uint32_t count,
                                                   const void* data) {
        if (mTopLevelBuilder->ConsumedError(ValidateCanRecordCommands())) {
            return;
        }

        // TODO(cwallez@chromium.org): check for overflows
        if (offset + count > kMaxPushConstants) {
            mTopLevelBuilder->HandleError("Setting too many push constants");
            return;
        }

        SetPushConstantsCmd* cmd =
            mAllocator->Allocate<SetPushConstantsCmd>(Command::SetPushConstants);
        new (cmd) SetPushConstantsCmd;
        cmd->stages = stages;
        cmd->offset = offset;
        cmd->count = count;

        uint32_t* values = mAllocator->AllocateData<uint32_t>(count);
        memcpy(values, data, count * sizeof(uint32_t));
    }

    DeviceBase* ProgrammablePassEncoder::GetDevice() const {
        return mDevice;
    }

    MaybeError ProgrammablePassEncoder::ValidateCanRecordCommands() const {
        if (mAllocator == nullptr) {
            return DAWN_VALIDATION_ERROR("Recording in an already ended computePassEncoder");
        }

        return nullptr;
    }

}  // namespace dawn_native
