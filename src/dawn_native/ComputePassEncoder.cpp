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

#include "dawn_native/CommandBuffer.h"
#include "dawn_native/Commands.h"
#include "dawn_native/ComputePipeline.h"

namespace dawn_native {

    ComputePassEncoderBase::ComputePassEncoderBase(DeviceBase* device,
                                                   CommandBufferBuilder* topLevelBuilder,
                                                   CommandAllocator* allocator)
        : ProgrammablePassEncoder(device, topLevelBuilder, allocator) {
    }

    void ComputePassEncoderBase::Dispatch(uint32_t x, uint32_t y, uint32_t z) {
        if (mTopLevelBuilder->ConsumedError(ValidateCanRecordCommands())) {
            return;
        }

        DispatchCmd* dispatch = mAllocator->Allocate<DispatchCmd>(Command::Dispatch);
        new (dispatch) DispatchCmd;
        dispatch->x = x;
        dispatch->y = y;
        dispatch->z = z;
    }

    void ComputePassEncoderBase::SetComputePipeline(ComputePipelineBase* pipeline) {
        if (mTopLevelBuilder->ConsumedError(ValidateCanRecordCommands())) {
            return;
        }

        if (pipeline == nullptr) {
            mTopLevelBuilder->HandleError("Pipeline cannot be null");
            return;
        }

        SetComputePipelineCmd* cmd =
            mAllocator->Allocate<SetComputePipelineCmd>(Command::SetComputePipeline);
        new (cmd) SetComputePipelineCmd;
        cmd->pipeline = pipeline;
    }

}  // namespace dawn_native
