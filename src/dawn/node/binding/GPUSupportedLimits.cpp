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

#include "src/dawn/node/binding/GPUSupportedLimits.h"

#include <utility>

namespace wgpu::binding {

////////////////////////////////////////////////////////////////////////////////
// wgpu::bindings::GPUSupportedLimits
////////////////////////////////////////////////////////////////////////////////

GPUSupportedLimits::GPUSupportedLimits(wgpu::SupportedLimits limits) : limits_(std::move(limits)) {}

uint32_t GPUSupportedLimits::getMaxTextureDimension1D(Napi::Env) {
    return limits_.limits.maxTextureDimension1D;
}

uint32_t GPUSupportedLimits::getMaxTextureDimension2D(Napi::Env) {
    return limits_.limits.maxTextureDimension2D;
}

uint32_t GPUSupportedLimits::getMaxTextureDimension3D(Napi::Env) {
    return limits_.limits.maxTextureDimension3D;
}

uint32_t GPUSupportedLimits::getMaxTextureArrayLayers(Napi::Env) {
    return limits_.limits.maxTextureArrayLayers;
}

uint32_t GPUSupportedLimits::getMaxBindGroups(Napi::Env) {
    return limits_.limits.maxBindGroups;
}

uint32_t GPUSupportedLimits::getMaxBindingsPerBindGroup(Napi::Env) {
    return limits_.limits.maxBindingsPerBindGroup;
}

uint32_t GPUSupportedLimits::getMaxDynamicUniformBuffersPerPipelineLayout(Napi::Env) {
    return limits_.limits.maxDynamicUniformBuffersPerPipelineLayout;
}

uint32_t GPUSupportedLimits::getMaxDynamicStorageBuffersPerPipelineLayout(Napi::Env) {
    return limits_.limits.maxDynamicStorageBuffersPerPipelineLayout;
}

uint32_t GPUSupportedLimits::getMaxSampledTexturesPerShaderStage(Napi::Env) {
    return limits_.limits.maxSampledTexturesPerShaderStage;
}

uint32_t GPUSupportedLimits::getMaxSamplersPerShaderStage(Napi::Env) {
    return limits_.limits.maxSamplersPerShaderStage;
}

uint32_t GPUSupportedLimits::getMaxStorageBuffersPerShaderStage(Napi::Env) {
    return limits_.limits.maxStorageBuffersPerShaderStage;
}

uint32_t GPUSupportedLimits::getMaxStorageTexturesPerShaderStage(Napi::Env) {
    return limits_.limits.maxStorageTexturesPerShaderStage;
}

uint32_t GPUSupportedLimits::getMaxUniformBuffersPerShaderStage(Napi::Env) {
    return limits_.limits.maxUniformBuffersPerShaderStage;
}

uint64_t GPUSupportedLimits::getMaxUniformBufferBindingSize(Napi::Env) {
    return limits_.limits.maxUniformBufferBindingSize;
}

uint64_t GPUSupportedLimits::getMaxStorageBufferBindingSize(Napi::Env) {
    return limits_.limits.maxStorageBufferBindingSize;
}

uint32_t GPUSupportedLimits::getMinUniformBufferOffsetAlignment(Napi::Env) {
    return limits_.limits.minUniformBufferOffsetAlignment;
}

uint32_t GPUSupportedLimits::getMinStorageBufferOffsetAlignment(Napi::Env) {
    return limits_.limits.minStorageBufferOffsetAlignment;
}

uint32_t GPUSupportedLimits::getMaxVertexBuffers(Napi::Env) {
    return limits_.limits.maxVertexBuffers;
}

uint64_t GPUSupportedLimits::getMaxBufferSize(Napi::Env) {
    return limits_.limits.maxBufferSize;
}

uint32_t GPUSupportedLimits::getMaxVertexAttributes(Napi::Env) {
    return limits_.limits.maxVertexAttributes;
}

uint32_t GPUSupportedLimits::getMaxVertexBufferArrayStride(Napi::Env) {
    return limits_.limits.maxVertexBufferArrayStride;
}

uint32_t GPUSupportedLimits::getMaxInterStageShaderComponents(Napi::Env) {
    return limits_.limits.maxInterStageShaderComponents;
}

uint32_t GPUSupportedLimits::getMaxInterStageShaderVariables(Napi::Env) {
    return limits_.limits.maxInterStageShaderVariables;
}

uint32_t GPUSupportedLimits::getMaxColorAttachments(Napi::Env) {
    return limits_.limits.maxColorAttachments;
}

uint32_t GPUSupportedLimits::getMaxColorAttachmentBytesPerSample(Napi::Env) {
    return limits_.limits.maxColorAttachmentBytesPerSample;
}

uint32_t GPUSupportedLimits::getMaxComputeWorkgroupStorageSize(Napi::Env) {
    return limits_.limits.maxComputeWorkgroupStorageSize;
}

uint32_t GPUSupportedLimits::getMaxComputeInvocationsPerWorkgroup(Napi::Env) {
    return limits_.limits.maxComputeInvocationsPerWorkgroup;
}

uint32_t GPUSupportedLimits::getMaxComputeWorkgroupSizeX(Napi::Env) {
    return limits_.limits.maxComputeWorkgroupSizeX;
}

uint32_t GPUSupportedLimits::getMaxComputeWorkgroupSizeY(Napi::Env) {
    return limits_.limits.maxComputeWorkgroupSizeY;
}

uint32_t GPUSupportedLimits::getMaxComputeWorkgroupSizeZ(Napi::Env) {
    return limits_.limits.maxComputeWorkgroupSizeZ;
}

uint32_t GPUSupportedLimits::getMaxComputeWorkgroupsPerDimension(Napi::Env) {
    return limits_.limits.maxComputeWorkgroupsPerDimension;
}

}  // namespace wgpu::binding
