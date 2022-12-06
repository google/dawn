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

#ifndef SRC_DAWN_COMMON_CONSTANTS_H_
#define SRC_DAWN_COMMON_CONSTANTS_H_

#include <cstddef>
#include <cstdint>

static constexpr uint32_t kMaxBindGroups = 4u;
static constexpr uint32_t kMaxBindingsPerBindGroup = 640u;
static constexpr uint8_t kMaxVertexAttributes = 16u;
static constexpr uint8_t kMaxVertexBuffers = 8u;
static constexpr uint32_t kMaxVertexBufferArrayStride = 2048u;
static constexpr uint32_t kNumStages = 3;
static constexpr uint8_t kMaxColorAttachments = 8u;
static constexpr uint32_t kTextureBytesPerRowAlignment = 256u;
static constexpr uint32_t kMaxInterStageShaderComponents = 60u;
static constexpr uint32_t kMaxInterStageShaderVariables = 16u;
static constexpr uint64_t kAssumedMaxBufferSize =
    0x80000000u;  // Use 2 GB when the limit is unavailable

// Per stage limits
static constexpr uint32_t kMaxSampledTexturesPerShaderStage = 16;
static constexpr uint32_t kMaxSamplersPerShaderStage = 16;
static constexpr uint32_t kMaxStorageBuffersPerShaderStage = 8;
static constexpr uint32_t kMaxStorageTexturesPerShaderStage = 4;
static constexpr uint32_t kMaxUniformBuffersPerShaderStage = 12;

// Per pipeline layout limits
static constexpr uint32_t kMaxDynamicUniformBuffersPerPipelineLayout = 8u;
static constexpr uint32_t kMaxDynamicStorageBuffersPerPipelineLayout = 4u;

// Indirect command sizes
static constexpr uint64_t kDispatchIndirectSize = 3 * sizeof(uint32_t);
static constexpr uint64_t kDrawIndirectSize = 4 * sizeof(uint32_t);
static constexpr uint64_t kDrawIndexedIndirectSize = 5 * sizeof(uint32_t);

// Non spec defined constants.
static constexpr float kLodMin = 0.0;
static constexpr float kLodMax = 1000.0;

// Offset alignment for CopyB2B. Strictly speaking this alignment is required only
// on macOS, but we decide to do it on all platforms.
static constexpr uint64_t kCopyBufferToBufferOffsetAlignment = 4u;

// Metal has a maximum size of 32Kb for a counter set buffer. Each query is 8 bytes.
// So, the maximum nymber of queries is 32Kb / 8.
static constexpr uint32_t kMaxQueryCount = 4096;

// An external texture occupies multiple binding slots. These are the per-external-texture bindings
// needed.
static constexpr uint8_t kSampledTexturesPerExternalTexture = 4u;
static constexpr uint8_t kSamplersPerExternalTexture = 1u;
static constexpr uint8_t kUniformsPerExternalTexture = 1u;

// Wire buffer alignments.
static constexpr size_t kWireBufferAlignment = 8u;

#endif  // SRC_DAWN_COMMON_CONSTANTS_H_
