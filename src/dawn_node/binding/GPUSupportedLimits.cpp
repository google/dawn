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

#include "src/dawn_node/binding/GPUSupportedLimits.h"

namespace wgpu { namespace binding {

    ////////////////////////////////////////////////////////////////////////////////
    // wgpu::bindings::GPUSupportedLimits
    ////////////////////////////////////////////////////////////////////////////////

    // Values taken from.
    // https://source.chromium.org/chromium/chromium/src/+/main:third_party/blink/renderer/modules/webgpu/gpu_supported_limits.h
    // TODO(crbug.com/dawn/1131): Actually use limits reported by the device / adapter.

    uint32_t GPUSupportedLimits::getMaxTextureDimension1D(Napi::Env) {
        return 8192;
    }

    uint32_t GPUSupportedLimits::getMaxTextureDimension2D(Napi::Env) {
        return 8192;
    }

    uint32_t GPUSupportedLimits::getMaxTextureDimension3D(Napi::Env) {
        return 2048;
    }

    uint32_t GPUSupportedLimits::getMaxTextureArrayLayers(Napi::Env) {
        return 2048;
    }

    uint32_t GPUSupportedLimits::getMaxBindGroups(Napi::Env) {
        return 4;
    }

    uint32_t GPUSupportedLimits::getMaxDynamicUniformBuffersPerPipelineLayout(Napi::Env) {
        return 8;
    }

    uint32_t GPUSupportedLimits::getMaxDynamicStorageBuffersPerPipelineLayout(Napi::Env) {
        return 4;
    }

    uint32_t GPUSupportedLimits::getMaxSampledTexturesPerShaderStage(Napi::Env) {
        return 16;
    }

    uint32_t GPUSupportedLimits::getMaxSamplersPerShaderStage(Napi::Env) {
        return 16;
    }

    uint32_t GPUSupportedLimits::getMaxStorageBuffersPerShaderStage(Napi::Env) {
        return 4;
    }

    uint32_t GPUSupportedLimits::getMaxStorageTexturesPerShaderStage(Napi::Env) {
        return 4;
    }

    uint32_t GPUSupportedLimits::getMaxUniformBuffersPerShaderStage(Napi::Env) {
        return 12;
    }

    uint64_t GPUSupportedLimits::getMaxUniformBufferBindingSize(Napi::Env) {
        return 16384;
    }

    uint64_t GPUSupportedLimits::getMaxStorageBufferBindingSize(Napi::Env) {
        return 134217728;
    }

    uint32_t GPUSupportedLimits::getMinUniformBufferOffsetAlignment(Napi::Env) {
        return 256;
    }

    uint32_t GPUSupportedLimits::getMinStorageBufferOffsetAlignment(Napi::Env) {
        return 256;
    }

    uint32_t GPUSupportedLimits::getMaxVertexBuffers(Napi::Env) {
        return 8;
    }

    uint32_t GPUSupportedLimits::getMaxVertexAttributes(Napi::Env) {
        return 16;
    }

    uint32_t GPUSupportedLimits::getMaxVertexBufferArrayStride(Napi::Env) {
        return 2048;
    }

    uint32_t GPUSupportedLimits::getMaxInterStageShaderComponents(Napi::Env) {
        return 60;
    }

    uint32_t GPUSupportedLimits::getMaxComputeWorkgroupStorageSize(Napi::Env) {
        return 16352;
    }

    uint32_t GPUSupportedLimits::getMaxComputeInvocationsPerWorkgroup(Napi::Env) {
        return 256;
    }

    uint32_t GPUSupportedLimits::getMaxComputeWorkgroupSizeX(Napi::Env) {
        return 256;
    }

    uint32_t GPUSupportedLimits::getMaxComputeWorkgroupSizeY(Napi::Env) {
        return 256;
    }

    uint32_t GPUSupportedLimits::getMaxComputeWorkgroupSizeZ(Napi::Env) {
        return 64;
    }

    uint32_t GPUSupportedLimits::getMaxComputeWorkgroupsPerDimension(Napi::Env) {
        return 65535;
    }

}}  // namespace wgpu::binding
