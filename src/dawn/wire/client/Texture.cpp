// Copyright 2022 The Dawn & Tint Authors
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// 1. Redistributions of source code must retain the above copyright notice, this
//    list of conditions and the following disclaimer.
//
// 2. Redistributions in binary form must reproduce the above copyright notice,
//    this list of conditions and the following disclaimer in the documentation
//    and/or other materials provided with the distribution.
//
// 3. Neither the name of the copyright holder nor the names of its
//    contributors may be used to endorse or promote products derived from
//    this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
// FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
// DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
// CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
// OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#include "src/dawn/wire/client/Texture.h"

#include <utility>

#include "src/dawn/wire/client/Client.h"
#include "src/dawn/wire/client/Device.h"

namespace dawn::wire::client {

// static
WGPUTexture Texture::Create(Device* device, const WGPUTextureDescriptor* descriptor) {
    Client* wireClient = device->GetClient();

    DeviceCreateTextureCmd cmd;
    cmd.self = ToAPI(device);
    cmd.descriptor = descriptor;

    Ref<Texture> texture = wireClient->Make<Texture>(device, descriptor);
    cmd.result = texture->GetWireHandle(wireClient);

    wireClient->SerializeCommand(cmd);

    return ReturnToAPI(std::move(texture));
}

// static
WGPUTexture Texture::CreateError(Device* device, const WGPUTextureDescriptor* descriptor) {
    Client* client = device->GetClient();
    Ref<Texture> texture = client->Make<Texture>(device, descriptor);

    DeviceCreateErrorTextureCmd cmd;
    cmd.self = ToAPI(device);
    cmd.descriptor = descriptor;
    cmd.result = texture->GetWireHandle(client);
    client->SerializeCommand(cmd);

    return ReturnToAPI(std::move(texture));
}

Texture::Texture(const ObjectBaseParams& params,
                 const Device* device,
                 const WGPUTextureDescriptor* descriptor)
    : ObjectBase(params),
      mSize(descriptor->size),
      mMipLevelCount(descriptor->mipLevelCount),
      mSampleCount(descriptor->sampleCount),
      mDimension(FromAPI(descriptor->dimension) == wgpu::TextureDimension::Undefined
                     ? wgpu::TextureDimension::e2D
                     : FromAPI(descriptor->dimension)),
      mFormat(FromAPI(descriptor->format)),
      mUsage(static_cast<wgpu::TextureUsage>(descriptor->usage)),
      mTextureBindingViewDimension(wgpu::TextureViewDimension::Undefined) {
    // We only set mTextureBindingViewDimension in compatibility mode
    // and if it's undefined we need to set it to the default.
    if (!device->APIHasFeature(WGPUFeatureName_CoreFeaturesAndLimits)) {
        for (auto* chain = descriptor->nextInChain; chain; chain = chain->next) {
            switch (chain->sType) {
                case WGPUSType_TextureBindingViewDimension:
                    if (!device->APIHasFeature(WGPUFeatureName_CoreFeaturesAndLimits)) {
                        WGPUTextureViewDimension viewDimension =
                            reinterpret_cast<const WGPUTextureBindingViewDimension*>(chain)
                                ->textureBindingViewDimension;
                        mTextureBindingViewDimension = FromAPI(viewDimension);
                    }
                    break;
                default:
                    break;
            }
        }
        if (mTextureBindingViewDimension == wgpu::TextureViewDimension::Undefined) {
            switch (mDimension) {
                case wgpu::TextureDimension::e1D:
                    mTextureBindingViewDimension = wgpu::TextureViewDimension::e1D;
                    break;
                case wgpu::TextureDimension::e2D:
                    mTextureBindingViewDimension = mSize.depthOrArrayLayers == 1
                                                       ? wgpu::TextureViewDimension::e2D
                                                       : wgpu::TextureViewDimension::e2DArray;
                    break;
                case wgpu::TextureDimension::e3D:
                    mTextureBindingViewDimension = wgpu::TextureViewDimension::e3D;
                    break;
                default:
                    // We could get here if the texture descriptor is invalid
                    break;
            }
        }
    }
}

Texture::~Texture() = default;

ObjectType Texture::GetObjectType() const {
    return ObjectType::Texture;
}

uint32_t Texture::APIGetWidth() const {
    return mSize.width;
}

uint32_t Texture::APIGetHeight() const {
    return mSize.height;
}

uint32_t Texture::APIGetDepthOrArrayLayers() const {
    return mSize.depthOrArrayLayers;
}

uint32_t Texture::APIGetMipLevelCount() const {
    return mMipLevelCount;
}

uint32_t Texture::APIGetSampleCount() const {
    return mSampleCount;
}

wgpu::TextureDimension Texture::APIGetDimension() const {
    return mDimension;
}

wgpu::TextureFormat Texture::APIGetFormat() const {
    return mFormat;
}

wgpu::TextureUsage Texture::APIGetUsage() const {
    return mUsage;
}

wgpu::TextureViewDimension Texture::APIGetTextureBindingViewDimension() const {
    return mTextureBindingViewDimension;
}

}  // namespace dawn::wire::client
