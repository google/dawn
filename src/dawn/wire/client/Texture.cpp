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

#include "dawn/wire/client/Texture.h"

#include "dawn/wire/client/Client.h"
#include "dawn/wire/client/Device.h"

namespace dawn::wire::client {

// static
WGPUTexture Texture::Create(Device* device, const WGPUTextureDescriptor* descriptor) {
    Client* wireClient = device->GetClient();
    Texture* texture = wireClient->Make<Texture>(descriptor);

    // Send the Device::CreateTexture command without modifications.
    DeviceCreateTextureCmd cmd;
    cmd.self = ToAPI(device);
    cmd.selfId = device->GetWireId();
    cmd.descriptor = descriptor;
    cmd.result = texture->GetWireHandle();
    wireClient->SerializeCommand(cmd);

    return ToAPI(texture);
}

// static
WGPUTexture Texture::CreateError(Device* device, const WGPUTextureDescriptor* descriptor) {
    Client* wireClient = device->GetClient();
    Texture* texture = wireClient->Make<Texture>(descriptor);

    // Send the Device::CreateErrorTexture command without modifications.
    DeviceCreateErrorTextureCmd cmd;
    cmd.self = ToAPI(device);
    cmd.selfId = device->GetWireId();
    cmd.descriptor = descriptor;
    cmd.result = texture->GetWireHandle();
    wireClient->SerializeCommand(cmd);

    return ToAPI(texture);
}

Texture::Texture(const ObjectBaseParams& params, const WGPUTextureDescriptor* descriptor)
    : ObjectBase(params),
      mSize(descriptor->size),
      mMipLevelCount(descriptor->mipLevelCount),
      mSampleCount(descriptor->sampleCount),
      mDimension(descriptor->dimension),
      mFormat(descriptor->format),
      mUsage(static_cast<WGPUTextureUsage>(descriptor->usage)) {}

Texture::~Texture() = default;

uint32_t Texture::GetWidth() const {
    return mSize.width;
}

uint32_t Texture::GetHeight() const {
    return mSize.height;
}

uint32_t Texture::GetDepthOrArrayLayers() const {
    return mSize.depthOrArrayLayers;
}

uint32_t Texture::GetMipLevelCount() const {
    return mMipLevelCount;
}

uint32_t Texture::GetSampleCount() const {
    return mSampleCount;
}

WGPUTextureDimension Texture::GetDimension() const {
    return mDimension;
}

WGPUTextureFormat Texture::GetFormat() const {
    return mFormat;
}

WGPUTextureUsage Texture::GetUsage() const {
    return mUsage;
}

}  // namespace dawn::wire::client
