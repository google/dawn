// Copyright 2024 The Dawn & Tint Authors
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

#include "dawn/wire/client/Surface.h"

#include "dawn/common/Log.h"
#include "dawn/common/Platform.h"
#include "dawn/wire/client/Client.h"
#include "dawn/wire/client/Device.h"
#include "dawn/wire/client/Texture.h"

namespace dawn::wire::client {

Surface::Surface(const ObjectBaseParams& params) : ObjectBase(params) {}

Surface::~Surface() = default;

ObjectType Surface::GetObjectType() const {
    return ObjectType::Surface;
}

void Surface::Configure(WGPUSurfaceConfiguration const* config) {
    mTextureDescriptor = {};
    mTextureDescriptor.size = {config->width, config->height, 1};
    mTextureDescriptor.format = config->format;
    mTextureDescriptor.usage = config->usage;
    mTextureDescriptor.dimension = WGPUTextureDimension_2D;
    mTextureDescriptor.mipLevelCount = 1;
    mTextureDescriptor.sampleCount = 1;

    SurfaceConfigureCmd cmd;
    cmd.self = ToAPI(this);
    cmd.config = config;
    GetClient()->SerializeCommand(cmd);
}

WGPUTextureFormat Surface::GetPreferredFormat([[maybe_unused]] WGPUAdapter adapter) const {
    // TODO(dawn:2320) Use the result of GetCapabilities
    // This is the only supported format in native mode (see crbug.com/dawn/160).
    return WGPUTextureFormat_BGRA8Unorm;
}

void Surface::GetCapabilities(WGPUAdapter adapter, WGPUSurfaceCapabilities* capabilities) const {
    // TODO(dawn:2320) Implement this
    dawn::ErrorLog() << "surface.GetCapabilities not supported yet with dawn_wire.";
}

void Surface::GetCurrentTexture(WGPUSurfaceTexture* surfaceTexture) {
    // TODO(dawn:2320) Implement this
    dawn::ErrorLog() << "surface.GetCurrentTexture not supported yet with dawn_wire.";

    Client* wireClient = GetClient();
    Texture* texture = wireClient->Make<Texture>(&mTextureDescriptor);
    surfaceTexture->texture = ToAPI(texture);

    SurfaceGetCurrentTextureCmd cmd;
    cmd.self = ToAPI(this);
    cmd.selfId = GetWireId();
    // cmd.result = texture->GetWireHandle(); // TODO(dawn:2320) Feed surfaceTexture to cmd
    wireClient->SerializeCommand(cmd);
}

void ClientSurfaceCapabilitiesFreeMembers(WGPUSurfaceCapabilities capabilities) {
    // TODO(dawn:2320) Implement this
    dawn::ErrorLog() << "surfaceCapabilities.FreeMembers not supported yet with dawn_wire.";
}

}  // namespace dawn::wire::client
