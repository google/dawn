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

#ifndef SRC_DAWN_NATIVE_FORWARD_H_
#define SRC_DAWN_NATIVE_FORWARD_H_

#include <cstdint>

namespace dawn {
template <typename T>
class Ref;
}  // namespace dawn

namespace dawn::native {

enum class ObjectType : uint32_t;

class AdapterBase;
class BindGroupBase;
class BindGroupLayoutBase;
class BindGroupLayoutInternalBase;
class BufferBase;
class ComputePipelineBase;
class CommandBufferBase;
class CommandEncoder;
class ComputePassEncoder;
class ExternalTextureBase;
class SharedTextureMemoryBase;
class InstanceBase;
class PhysicalDeviceBase;
class PipelineBase;
class PipelineCacheBase;
class PipelineLayoutBase;
class QuerySetBase;
class QueueBase;
class RenderBundleBase;
class RenderBundleEncoder;
class RenderPassEncoder;
class RenderPipelineBase;
class ResourceHeapBase;
class SamplerBase;
class SharedFenceBase;
class Surface;
class ShaderModuleBase;
class SwapChainBase;
class TextureBase;
class TextureViewBase;

class DeviceBase;

template <typename T>
class PerStage;

struct Format;

// Aliases for frontend-only types.
using CommandEncoderBase = CommandEncoder;
using ComputePassEncoderBase = ComputePassEncoder;
using RenderBundleEncoderBase = RenderBundleEncoder;
using RenderPassEncoderBase = RenderPassEncoder;
using SurfaceBase = Surface;

}  // namespace dawn::native

#endif  // SRC_DAWN_NATIVE_FORWARD_H_
