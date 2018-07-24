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

#include "dawn_native/vulkan/BindGroupLayoutVk.h"
#include "dawn_native/vulkan/BindGroupVk.h"
#include "dawn_native/vulkan/BlendStateVk.h"
#include "dawn_native/vulkan/BufferVk.h"
#include "dawn_native/vulkan/CommandBufferVk.h"
#include "dawn_native/vulkan/ComputePipelineVk.h"
#include "dawn_native/vulkan/DepthStencilStateVk.h"
#include "dawn_native/vulkan/DeviceVk.h"
#include "dawn_native/vulkan/InputStateVk.h"
#include "dawn_native/vulkan/PipelineLayoutVk.h"
#include "dawn_native/vulkan/QueueVk.h"
#include "dawn_native/vulkan/RenderPassDescriptorVk.h"
#include "dawn_native/vulkan/RenderPipelineVk.h"
#include "dawn_native/vulkan/SamplerVk.h"
#include "dawn_native/vulkan/ShaderModuleVk.h"
#include "dawn_native/vulkan/SwapChainVk.h"
#include "dawn_native/vulkan/TextureVk.h"
