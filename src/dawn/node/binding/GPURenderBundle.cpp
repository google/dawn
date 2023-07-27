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

#include "src/dawn/node/binding/GPURenderBundle.h"

#include <utility>

#include "src/dawn/node/binding/Converter.h"
#include "src/dawn/node/binding/GPUBuffer.h"
#include "src/dawn/node/binding/GPURenderPipeline.h"
#include "src/dawn/node/utils/Debug.h"

namespace wgpu::binding {

////////////////////////////////////////////////////////////////////////////////
// wgpu::bindings::GPURenderBundle
////////////////////////////////////////////////////////////////////////////////
GPURenderBundle::GPURenderBundle(const wgpu::RenderBundleDescriptor& desc,
                                 wgpu::RenderBundle bundle)
    : bundle_(std::move(bundle)), label_(desc.label ? desc.label : "") {}

std::string GPURenderBundle::getLabel(Napi::Env) {
    return label_;
}

void GPURenderBundle::setLabel(Napi::Env, std::string value) {
    bundle_.SetLabel(value.c_str());
    label_ = value;
}

}  // namespace wgpu::binding
