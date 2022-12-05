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

#include "src/dawn/node/binding/GPUTexture.h"

#include <utility>

#include "src/dawn/node/binding/Converter.h"
#include "src/dawn/node/binding/Errors.h"
#include "src/dawn/node/binding/GPUTextureView.h"
#include "src/dawn/node/utils/Debug.h"

namespace wgpu::binding {

////////////////////////////////////////////////////////////////////////////////
// wgpu::bindings::GPUTexture
////////////////////////////////////////////////////////////////////////////////
GPUTexture::GPUTexture(wgpu::Device device, wgpu::Texture texture)
    : device_(std::move(device)), texture_(std::move(texture)) {}

interop::Interface<interop::GPUTextureView> GPUTexture::createView(
    Napi::Env env,
    interop::GPUTextureViewDescriptor descriptor) {
    if (!texture_) {
        Errors::OperationError(env).ThrowAsJavaScriptException();
        return {};
    }

    wgpu::TextureViewDescriptor desc{};
    Converter conv(env, device_);
    if (!conv(desc.baseMipLevel, descriptor.baseMipLevel) ||        //
        !conv(desc.mipLevelCount, descriptor.mipLevelCount) ||      //
        !conv(desc.baseArrayLayer, descriptor.baseArrayLayer) ||    //
        !conv(desc.arrayLayerCount, descriptor.arrayLayerCount) ||  //
        !conv(desc.format, descriptor.format) ||                    //
        !conv(desc.dimension, descriptor.dimension) ||              //
        !conv(desc.aspect, descriptor.aspect)) {
        return {};
    }
    return interop::GPUTextureView::Create<GPUTextureView>(env, texture_.CreateView(&desc));
}

void GPUTexture::destroy(Napi::Env) {
    texture_.Destroy();
}

interop::GPUIntegerCoordinate GPUTexture::getWidth(Napi::Env) {
    return texture_.GetWidth();
}

interop::GPUIntegerCoordinate GPUTexture::getHeight(Napi::Env) {
    return texture_.GetHeight();
}

interop::GPUIntegerCoordinate GPUTexture::getDepthOrArrayLayers(Napi::Env) {
    return texture_.GetDepthOrArrayLayers();
}

interop::GPUIntegerCoordinate GPUTexture::getMipLevelCount(Napi::Env) {
    return texture_.GetMipLevelCount();
}

interop::GPUSize32 GPUTexture::getSampleCount(Napi::Env) {
    return texture_.GetSampleCount();
}

interop::GPUTextureDimension GPUTexture::getDimension(Napi::Env env) {
    interop::GPUTextureDimension result;

    Converter conv(env);
    if (!conv(result, texture_.GetDimension())) {
        Napi::Error::New(env, "Couldn't convert dimension to a JavaScript value.")
            .ThrowAsJavaScriptException();
        return interop::GPUTextureDimension::k1D;  // Doesn't get used.
    }

    return result;
}

interop::GPUTextureFormat GPUTexture::getFormat(Napi::Env env) {
    interop::GPUTextureFormat result;

    Converter conv(env);
    if (!conv(result, texture_.GetFormat())) {
        Napi::Error::New(env, "Couldn't convert format to a JavaScript value.")
            .ThrowAsJavaScriptException();
        return interop::GPUTextureFormat::kR32Float;  // Doesn't get used.
    }

    return result;
}

interop::GPUTextureUsageFlags GPUTexture::getUsage(Napi::Env env) {
    interop::GPUTextureUsageFlags result;

    Converter conv(env);
    if (!conv(result, texture_.GetUsage())) {
        Napi::Error::New(env, "Couldn't convert usage to a JavaScript value.")
            .ThrowAsJavaScriptException();
        return {0u};  // Doesn't get used.
    }

    return result;
}

std::string GPUTexture::getLabel(Napi::Env) {
    UNIMPLEMENTED();
}

void GPUTexture::setLabel(Napi::Env, std::string value) {
    UNIMPLEMENTED();
}

}  // namespace wgpu::binding
