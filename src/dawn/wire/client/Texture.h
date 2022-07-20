// Copyright 2022 The Dawn Authors
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

#ifndef SRC_DAWN_WIRE_CLIENT_TEXTURE_H_
#define SRC_DAWN_WIRE_CLIENT_TEXTURE_H_

#include "dawn/webgpu.h"

#include "dawn/wire/client/ObjectBase.h"

namespace dawn::wire::client {

class Device;

class Texture final : public ObjectBase {
  public:
    static WGPUTexture Create(Device* device, const WGPUTextureDescriptor* descriptor);
    static WGPUTexture CreateError(Device* device, const WGPUTextureDescriptor* descriptor);

    Texture(const ObjectBaseParams& params, const WGPUTextureDescriptor* descriptor);
    ~Texture() override;

    // Note that these values can be arbitrary since they aren't validated in the wire client.
    uint32_t GetWidth() const;
    uint32_t GetHeight() const;
    uint32_t GetDepthOrArrayLayers() const;
    uint32_t GetMipLevelCount() const;
    uint32_t GetSampleCount() const;
    WGPUTextureDimension GetDimension() const;
    WGPUTextureFormat GetFormat() const;
    WGPUTextureUsage GetUsage() const;

  private:
    WGPUExtent3D mSize;
    uint32_t mMipLevelCount;
    uint32_t mSampleCount;
    WGPUTextureDimension mDimension;
    WGPUTextureFormat mFormat;
    WGPUTextureUsage mUsage;
};

}  // namespace dawn::wire::client

#endif  // SRC_DAWN_WIRE_CLIENT_TEXTURE_H_
