// Copyright 2023 The Dawn Authors
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

#ifndef SRC_DAWN_NATIVE_D3D11_BINDGROUPTRACKERD3D11_H_
#define SRC_DAWN_NATIVE_D3D11_BINDGROUPTRACKERD3D11_H_

#include "dawn/native/BindGroupTracker.h"
#include "dawn/native/d3d/d3d_platform.h"

namespace dawn::native::d3d11 {

class CommandRecordingContext;

// We need convert WebGPU bind slot to d3d11 bind slot according a map in PipelineLayout, so we
// cannot inherit BindGroupTrackerGroups.
class BindGroupTracker : public BindGroupTrackerBase</*CanInheritBindGroups=*/true, uint64_t> {
  public:
    BindGroupTracker(CommandRecordingContext* commandContext, bool isRenderPass);
    ~BindGroupTracker();
    MaybeError Apply();

  private:
    MaybeError ApplyBindGroup(BindGroupIndex index);
    void UnApplyBindGroup(BindGroupIndex index);

    CommandRecordingContext* const mCommandContext;
    const bool mIsRenderPass;
    const wgpu::ShaderStage mVisibleStages;
};

}  // namespace dawn::native::d3d11

#endif  // SRC_DAWN_NATIVE_D3D11_BINDGROUPTRACKERD3D11_H_
