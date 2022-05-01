// Copyright 2021 The Dawn Authors
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

#include "dawn/native/InternalPipelineStore.h"

#include <unordered_map>

#include "dawn/native/ComputePipeline.h"
#include "dawn/native/Device.h"
#include "dawn/native/RenderPipeline.h"
#include "dawn/native/ShaderModule.h"

namespace dawn::native {

class RenderPipelineBase;
class ShaderModuleBase;

InternalPipelineStore::InternalPipelineStore(DeviceBase* device)
    : scratchStorage(device, wgpu::BufferUsage::CopyDst | wgpu::BufferUsage::Storage),
      scratchIndirectStorage(
          device,
          wgpu::BufferUsage::CopyDst | wgpu::BufferUsage::Indirect | wgpu::BufferUsage::Storage) {}

InternalPipelineStore::~InternalPipelineStore() = default;

}  // namespace dawn::native
