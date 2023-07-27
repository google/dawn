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

#include "src/dawn/node/binding/GPUComputePipeline.h"

#include <utility>

#include "src/dawn/node/binding/GPUBindGroupLayout.h"
#include "src/dawn/node/binding/GPUBuffer.h"
#include "src/dawn/node/utils/Debug.h"

namespace wgpu::binding {

////////////////////////////////////////////////////////////////////////////////
// wgpu::bindings::GPUComputePipeline
////////////////////////////////////////////////////////////////////////////////
GPUComputePipeline::GPUComputePipeline(const wgpu::ComputePipelineDescriptor& desc,
                                       wgpu::ComputePipeline pipeline)
    : pipeline_(std::move(pipeline)), label_(desc.label ? desc.label : "") {}

GPUComputePipeline::GPUComputePipeline(wgpu::ComputePipeline pipeline, std::string label)
    : pipeline_(std::move(pipeline)), label_(label) {}

interop::Interface<interop::GPUBindGroupLayout> GPUComputePipeline::getBindGroupLayout(
    Napi::Env env,
    uint32_t index) {
    wgpu::BindGroupLayoutDescriptor desc{};
    return interop::GPUBindGroupLayout::Create<GPUBindGroupLayout>(
        env, desc, pipeline_.GetBindGroupLayout(index));
}

std::string GPUComputePipeline::getLabel(Napi::Env) {
    return label_;
}

void GPUComputePipeline::setLabel(Napi::Env, std::string value) {
    pipeline_.SetLabel(value.c_str());
    label_ = value;
}

}  // namespace wgpu::binding
