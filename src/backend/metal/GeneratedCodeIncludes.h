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

#include "backend/metal/BlendStateMTL.h"
#include "backend/metal/BufferMTL.h"
#include "backend/metal/CommandBufferMTL.h"
#include "backend/metal/ComputePipelineMTL.h"
#include "backend/metal/DepthStencilStateMTL.h"
#include "backend/metal/InputStateMTL.h"
#include "backend/metal/MetalBackend.h"
#include "backend/metal/PipelineLayoutMTL.h"
#include "backend/metal/RenderPipelineMTL.h"
#include "backend/metal/SamplerMTL.h"
#include "backend/metal/ShaderModuleMTL.h"
#include "backend/metal/SwapChainMTL.h"
#include "backend/metal/TextureMTL.h"
