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

#ifndef SRC_DAWN_NODE_BINDING_GPUTEXTURE_H_
#define SRC_DAWN_NODE_BINDING_GPUTEXTURE_H_

#include <string>

#include "dawn/native/DawnNative.h"
#include "dawn/webgpu_cpp.h"
#include "src/dawn/node/interop/Napi.h"
#include "src/dawn/node/interop/WebGPU.h"

namespace wgpu::binding {

// GPUTexture is an implementation of interop::GPUTexture that wraps a wgpu::Texture.
class GPUTexture final : public interop::GPUTexture {
  public:
    explicit GPUTexture(wgpu::Device device, wgpu::Texture texture);

    // Implicit cast operator to Dawn GPU object
    inline operator const wgpu::Texture&() const { return texture_; }

    // interop::GPUTexture interface compliance
    interop::Interface<interop::GPUTextureView> createView(
        Napi::Env,
        interop::GPUTextureViewDescriptor descriptor) override;
    void destroy(Napi::Env) override;
    interop::GPUIntegerCoordinate getWidth(Napi::Env) override;
    interop::GPUIntegerCoordinate getHeight(Napi::Env) override;
    interop::GPUIntegerCoordinate getDepthOrArrayLayers(Napi::Env) override;
    interop::GPUIntegerCoordinate getMipLevelCount(Napi::Env) override;
    interop::GPUSize32 getSampleCount(Napi::Env) override;
    interop::GPUTextureDimension getDimension(Napi::Env) override;
    interop::GPUTextureFormat getFormat(Napi::Env) override;
    interop::GPUTextureUsageFlags getUsage(Napi::Env) override;
    std::string getLabel(Napi::Env) override;
    void setLabel(Napi::Env, std::string value) override;

  private:
    wgpu::Device device_;
    wgpu::Texture texture_;
};

}  // namespace wgpu::binding

#endif  // SRC_DAWN_NODE_BINDING_GPUTEXTURE_H_
