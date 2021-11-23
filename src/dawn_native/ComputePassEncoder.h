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

#ifndef DAWNNATIVE_COMPUTEPASSENCODER_H_
#define DAWNNATIVE_COMPUTEPASSENCODER_H_

#include "dawn_native/CommandBufferStateTracker.h"
#include "dawn_native/Error.h"
#include "dawn_native/Forward.h"
#include "dawn_native/PassResourceUsageTracker.h"
#include "dawn_native/ProgrammableEncoder.h"

namespace dawn_native {

    class SyncScopeUsageTracker;

    class ComputePassEncoder final : public ProgrammableEncoder {
      public:
        ComputePassEncoder(DeviceBase* device,
                           const ComputePassDescriptor* descriptor,
                           CommandEncoder* commandEncoder,
                           EncodingContext* encodingContext);

        static ComputePassEncoder* MakeError(DeviceBase* device,
                                             CommandEncoder* commandEncoder,
                                             EncodingContext* encodingContext);

        ObjectType GetType() const override;

        void APIEndPass();

        void APIDispatch(uint32_t x, uint32_t y = 1, uint32_t z = 1);
        void APIDispatchIndirect(BufferBase* indirectBuffer, uint64_t indirectOffset);
        void APISetPipeline(ComputePipelineBase* pipeline);

        void APISetBindGroup(uint32_t groupIndex,
                             BindGroupBase* group,
                             uint32_t dynamicOffsetCount = 0,
                             const uint32_t* dynamicOffsets = nullptr);

        void APIWriteTimestamp(QuerySetBase* querySet, uint32_t queryIndex);

        CommandBufferStateTracker* GetCommandBufferStateTrackerForTesting();
        void RestoreCommandBufferStateForTesting(CommandBufferStateTracker state) {
            RestoreCommandBufferState(std::move(state));
        }

      protected:
        ComputePassEncoder(DeviceBase* device,
                           CommandEncoder* commandEncoder,
                           EncodingContext* encodingContext,
                           ErrorTag errorTag);

      private:
        void DestroyImpl() override;

        ResultOrError<std::pair<Ref<BufferBase>, uint64_t>> TransformIndirectDispatchBuffer(
            Ref<BufferBase> indirectBuffer,
            uint64_t indirectOffset);

        void RestoreCommandBufferState(CommandBufferStateTracker state);

        CommandBufferStateTracker mCommandBufferState;

        // Adds the bindgroups used for the current dispatch to the SyncScopeResourceUsage and
        // records it in mUsageTracker.
        void AddDispatchSyncScope(SyncScopeUsageTracker scope = {});
        ComputePassResourceUsageTracker mUsageTracker;

        // For render and compute passes, the encoding context is borrowed from the command encoder.
        // Keep a reference to the encoder to make sure the context isn't freed.
        Ref<CommandEncoder> mCommandEncoder;
    };

}  // namespace dawn_native

#endif  // DAWNNATIVE_COMPUTEPASSENCODER_H_
