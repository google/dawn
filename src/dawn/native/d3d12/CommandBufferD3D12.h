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

#ifndef SRC_DAWN_NATIVE_D3D12_COMMANDBUFFERD3D12_H_
#define SRC_DAWN_NATIVE_D3D12_COMMANDBUFFERD3D12_H_

#include "dawn/native/CommandBuffer.h"
#include "dawn/native/Error.h"

namespace dawn::native {
struct BeginComputePassCmd;
struct BeginRenderPassCmd;
}  // namespace dawn::native

namespace dawn::native::d3d12 {

class BindGroupStateTracker;
class CommandRecordingContext;
class RenderPassBuilder;

class CommandBuffer final : public CommandBufferBase {
  public:
    static Ref<CommandBuffer> Create(CommandEncoder* encoder,
                                     const CommandBufferDescriptor* descriptor);

    MaybeError RecordCommands(CommandRecordingContext* commandContext);

  private:
    CommandBuffer(CommandEncoder* encoder, const CommandBufferDescriptor* descriptor);

    MaybeError RecordComputePass(CommandRecordingContext* commandContext,
                                 BindGroupStateTracker* bindingTracker,
                                 BeginComputePassCmd* computePass,
                                 const ComputePassResourceUsage& resourceUsages);
    MaybeError RecordRenderPass(CommandRecordingContext* commandContext,
                                BindGroupStateTracker* bindingTracker,
                                BeginRenderPassCmd* renderPass,
                                bool passHasUAV);
    MaybeError SetupRenderPass(CommandRecordingContext* commandContext,
                               BeginRenderPassCmd* renderPass,
                               RenderPassBuilder* renderPassBuilder);
    void EmulateBeginRenderPass(CommandRecordingContext* commandContext,
                                const RenderPassBuilder* renderPassBuilder) const;
};

}  // namespace dawn::native::d3d12

#endif  // SRC_DAWN_NATIVE_D3D12_COMMANDBUFFERD3D12_H_
