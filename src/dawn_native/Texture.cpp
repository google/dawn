// Copyright 2017 The Dawn Authors
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

#include "dawn_native/Texture.h"

#include "common/Assert.h"
#include "dawn_native/Device.h"

namespace dawn_native {

    uint32_t TextureFormatPixelSize(dawn::TextureFormat format) {
        switch (format) {
            case dawn::TextureFormat::R8Unorm:
            case dawn::TextureFormat::R8Uint:
                return 1;
            case dawn::TextureFormat::R8G8Unorm:
            case dawn::TextureFormat::R8G8Uint:
                return 2;
            case dawn::TextureFormat::R8G8B8A8Unorm:
            case dawn::TextureFormat::R8G8B8A8Uint:
            case dawn::TextureFormat::B8G8R8A8Unorm:
                return 4;
            case dawn::TextureFormat::D32FloatS8Uint:
                return 8;
            default:
                UNREACHABLE();
        }
    }

    bool TextureFormatHasDepth(dawn::TextureFormat format) {
        switch (format) {
            case dawn::TextureFormat::D32FloatS8Uint:
                return true;
            default:
                return false;
        }
    }

    bool TextureFormatHasStencil(dawn::TextureFormat format) {
        switch (format) {
            case dawn::TextureFormat::D32FloatS8Uint:
                return true;
            default:
                return false;
        }
    }

    bool TextureFormatHasDepthOrStencil(dawn::TextureFormat format) {
        switch (format) {
            case dawn::TextureFormat::D32FloatS8Uint:
                return true;
            default:
                return false;
        }
    }

    // TextureBase

    TextureBase::TextureBase(TextureBuilder* builder)
        : mDevice(builder->mDevice),
          mDimension(builder->mDimension),
          mFormat(builder->mFormat),
          mWidth(builder->mWidth),
          mHeight(builder->mHeight),
          mDepth(builder->mDepth),
          mNumMipLevels(builder->mNumMipLevels),
          mUsage(builder->mAllowedUsage) {
    }

    DeviceBase* TextureBase::GetDevice() const {
        return mDevice;
    }

    dawn::TextureDimension TextureBase::GetDimension() const {
        return mDimension;
    }
    dawn::TextureFormat TextureBase::GetFormat() const {
        return mFormat;
    }
    uint32_t TextureBase::GetWidth() const {
        return mWidth;
    }
    uint32_t TextureBase::GetHeight() const {
        return mHeight;
    }
    uint32_t TextureBase::GetDepth() const {
        return mDepth;
    }
    uint32_t TextureBase::GetNumMipLevels() const {
        return mNumMipLevels;
    }
    dawn::TextureUsageBit TextureBase::GetUsage() const {
        return mUsage;
    }

    TextureViewBuilder* TextureBase::CreateTextureViewBuilder() {
        return new TextureViewBuilder(mDevice, this);
    }

    // TextureBuilder

    enum TextureSetProperties {
        TEXTURE_PROPERTY_DIMENSION = 0x1,
        TEXTURE_PROPERTY_EXTENT = 0x2,
        TEXTURE_PROPERTY_FORMAT = 0x4,
        TEXTURE_PROPERTY_MIP_LEVELS = 0x8,
        TEXTURE_PROPERTY_ALLOWED_USAGE = 0x10,
    };

    TextureBuilder::TextureBuilder(DeviceBase* device) : Builder(device) {
    }

    TextureBase* TextureBuilder::GetResultImpl() {
        constexpr int allProperties = TEXTURE_PROPERTY_DIMENSION | TEXTURE_PROPERTY_EXTENT |
                                      TEXTURE_PROPERTY_FORMAT | TEXTURE_PROPERTY_MIP_LEVELS |
                                      TEXTURE_PROPERTY_ALLOWED_USAGE;
        if ((mPropertiesSet & allProperties) != allProperties) {
            HandleError("Texture missing properties");
            return nullptr;
        }

        // TODO(cwallez@chromium.org): check stuff based on the dimension

        return mDevice->CreateTexture(this);
    }

    void TextureBuilder::SetDimension(dawn::TextureDimension dimension) {
        if ((mPropertiesSet & TEXTURE_PROPERTY_DIMENSION) != 0) {
            HandleError("Texture dimension property set multiple times");
            return;
        }

        mPropertiesSet |= TEXTURE_PROPERTY_DIMENSION;
        mDimension = dimension;
    }

    void TextureBuilder::SetExtent(uint32_t width, uint32_t height, uint32_t depth) {
        if ((mPropertiesSet & TEXTURE_PROPERTY_EXTENT) != 0) {
            HandleError("Texture extent property set multiple times");
            return;
        }

        if (width == 0 || height == 0 || depth == 0) {
            HandleError("Cannot create an empty texture");
            return;
        }

        mPropertiesSet |= TEXTURE_PROPERTY_EXTENT;
        mWidth = width;
        mHeight = height;
        mDepth = depth;
    }

    void TextureBuilder::SetFormat(dawn::TextureFormat format) {
        if ((mPropertiesSet & TEXTURE_PROPERTY_FORMAT) != 0) {
            HandleError("Texture format property set multiple times");
            return;
        }

        mPropertiesSet |= TEXTURE_PROPERTY_FORMAT;
        mFormat = format;
    }

    void TextureBuilder::SetMipLevels(uint32_t numMipLevels) {
        if ((mPropertiesSet & TEXTURE_PROPERTY_MIP_LEVELS) != 0) {
            HandleError("Texture mip levels property set multiple times");
            return;
        }

        mPropertiesSet |= TEXTURE_PROPERTY_MIP_LEVELS;
        mNumMipLevels = numMipLevels;
    }

    void TextureBuilder::SetAllowedUsage(dawn::TextureUsageBit usage) {
        if ((mPropertiesSet & TEXTURE_PROPERTY_ALLOWED_USAGE) != 0) {
            HandleError("Texture allowed usage property set multiple times");
            return;
        }

        mPropertiesSet |= TEXTURE_PROPERTY_ALLOWED_USAGE;
        mAllowedUsage = usage;
    }

    // TextureViewBase

    TextureViewBase::TextureViewBase(TextureViewBuilder* builder) : mTexture(builder->mTexture) {
    }

    const TextureBase* TextureViewBase::GetTexture() const {
        return mTexture.Get();
    }

    TextureBase* TextureViewBase::GetTexture() {
        return mTexture.Get();
    }

    // TextureViewBuilder

    TextureViewBuilder::TextureViewBuilder(DeviceBase* device, TextureBase* texture)
        : Builder(device), mTexture(texture) {
    }

    TextureViewBase* TextureViewBuilder::GetResultImpl() {
        return mDevice->CreateTextureView(this);
    }

}  // namespace dawn_native
