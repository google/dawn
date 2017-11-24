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

#ifndef BACKEND_D3D12_COMPUTEPIPELINED3D12_H_
#define BACKEND_D3D12_COMPUTEPIPELINED3D12_H_

#include "backend/ComputePipeline.h"

#include "backend/d3d12/d3d12_platform.h"

namespace backend { namespace d3d12 {

    class ComputePipeline : public ComputePipelineBase {
      public:
        ComputePipeline(ComputePipelineBuilder* builder);

        ComPtr<ID3D12PipelineState> GetPipelineState();

      private:
        ComPtr<ID3D12PipelineState> mPipelineState;
    };

}}  // namespace backend::d3d12

#endif  // BACKEND_D3D12_COMPUTEPIPELINED3D12_H_
