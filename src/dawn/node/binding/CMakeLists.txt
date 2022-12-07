# Copyright 2021 The Dawn Authors
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

add_library(dawn_node_binding STATIC
    "AsyncRunner.cpp"
    "AsyncRunner.h"
    "Converter.cpp"
    "Converter.h"
    "Errors.cpp"
    "Errors.h"
    "Flags.cpp"
    "Flags.h"
    "GPU.cpp"
    "GPU.h"
    "GPUAdapter.cpp"
    "GPUAdapter.h"
    "GPUBindGroup.cpp"
    "GPUBindGroup.h"
    "GPUBindGroupLayout.cpp"
    "GPUBindGroupLayout.h"
    "GPUBuffer.cpp"
    "GPUBuffer.h"
    "GPUCommandBuffer.cpp"
    "GPUCommandBuffer.h"
    "GPUCommandEncoder.cpp"
    "GPUCommandEncoder.h"
    "GPUComputePassEncoder.cpp"
    "GPUComputePassEncoder.h"
    "GPUComputePipeline.cpp"
    "GPUComputePipeline.h"
    "GPUDevice.cpp"
    "GPUDevice.h"
    "GPUPipelineLayout.cpp"
    "GPUPipelineLayout.h"
    "GPUQuerySet.cpp"
    "GPUQuerySet.h"
    "GPUQueue.cpp"
    "GPUQueue.h"
    "GPURenderBundle.cpp"
    "GPURenderBundle.h"
    "GPURenderBundleEncoder.cpp"
    "GPURenderBundleEncoder.h"
    "GPURenderPassEncoder.cpp"
    "GPURenderPassEncoder.h"
    "GPURenderPipeline.cpp"
    "GPURenderPipeline.h"
    "GPUSampler.cpp"
    "GPUSampler.h"
    "GPUShaderModule.cpp"
    "GPUShaderModule.h"
    "GPUSupportedFeatures.cpp"
    "GPUSupportedFeatures.h"
    "GPUSupportedLimits.cpp"
    "GPUSupportedLimits.h"
    "GPUTexture.cpp"
    "GPUTexture.h"
    "GPUTextureView.cpp"
    "GPUTextureView.h"
)
common_compile_options(dawn_node_binding)

target_include_directories(dawn_node_binding
    PRIVATE
        "${PROJECT_SOURCE_DIR}"
        "${DAWN_THIRD_PARTY_DIR}"
        "${NODE_API_HEADERS_DIR}/include"
        "${DAWN_NODE_GEN_DIR}"
)

target_link_libraries(dawn_node_binding
    PRIVATE
        dawncpp
        dawn_node_interop
)
