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

#ifndef DAWN_NODE_BINDING_GPUBINDGROUPLAYOUT_H_
#define DAWN_NODE_BINDING_GPUBINDGROUPLAYOUT_H_

#include "dawn/webgpu_cpp.h"
#include "dawn_native/DawnNative.h"
#include "napi.h"
#include "src/dawn_node/interop/WebGPU.h"

namespace wgpu { namespace binding {

    // GPUBindGroupLayout is an implementation of interop::GPUBindGroupLayout that wraps a
    // wgpu::BindGroupLayout.
    class GPUBindGroupLayout final : public interop::GPUBindGroupLayout {
      public:
        GPUBindGroupLayout(wgpu::BindGroupLayout layout);

        // Implicit cast operator to Dawn GPU object
        inline operator const wgpu::BindGroupLayout &() const {
            return layout_;
        }

        // interop::GPUBindGroupLayout interface compliance
        std::optional<std::string> getLabel(Napi::Env) override;
        void setLabel(Napi::Env, std::optional<std::string> value) override;

      private:
        wgpu::BindGroupLayout layout_;
    };

}}  // namespace wgpu::binding

#endif  // DAWN_NODE_BINDING_GPUBINDGROUPLAYOUT_H_
