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

#include "backend/d3d12/D3D12Backend.h"
#include "backend/d3d12/BindGroupD3D12.h"
#include "backend/d3d12/BindGroupLayoutD3D12.h"
#include "backend/d3d12/BufferD3D12.h"
#include "backend/d3d12/CommandBufferD3D12.h"
#include "backend/d3d12/ComputePipelineD3D12.h"
#include "backend/d3d12/DepthStencilStateD3D12.h"
#include "backend/d3d12/FramebufferD3D12.h"
#include "backend/d3d12/InputStateD3D12.h"
#include "backend/d3d12/PipelineLayoutD3D12.h"
#include "backend/d3d12/QueueD3D12.h"
#include "backend/d3d12/RenderPipelineD3D12.h"
#include "backend/d3d12/SamplerD3D12.h"
#include "backend/d3d12/ShaderModuleD3D12.h"
#include "backend/d3d12/SwapChainD3D12.h"
#include "backend/d3d12/TextureD3D12.h"
