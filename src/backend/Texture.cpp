// Copyright 2017 The NXT Authors
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

#include "backend/Texture.h"

#include "backend/Device.h"
#include "common/Assert.h"

namespace backend {

    uint32_t TextureFormatPixelSize(nxt::TextureFormat format) {
        switch (format) {
            case nxt::TextureFormat::R8G8B8A8Unorm:
            case nxt::TextureFormat::R8G8B8A8Uint:
            case nxt::TextureFormat::B8G8R8A8Unorm:
                return 4;
            case nxt::TextureFormat::D32FloatS8Uint:
                return 8;
            default:
                UNREACHABLE();
        }
    }

    bool TextureFormatHasDepth(nxt::TextureFormat format) {
        switch (format) {
            case nxt::TextureFormat::R8G8B8A8Unorm:
            case nxt::TextureFormat::R8G8B8A8Uint:
            case nxt::TextureFormat::B8G8R8A8Unorm:
                return false;
            case nxt::TextureFormat::D32FloatS8Uint:
                return true;
            default:
                UNREACHABLE();
        }
    }

    bool TextureFormatHasStencil(nxt::TextureFormat format) {
        switch (format) {
            case nxt::TextureFormat::R8G8B8A8Unorm:
            case nxt::TextureFormat::R8G8B8A8Uint:
            case nxt::TextureFormat::B8G8R8A8Unorm:
                return false;
            case nxt::TextureFormat::D32FloatS8Uint:
                return true;
            default:
                UNREACHABLE();
        }
    }

    bool TextureFormatHasDepthOrStencil(nxt::TextureFormat format) {
        switch (format) {
            case nxt::TextureFormat::R8G8B8A8Unorm:
            case nxt::TextureFormat::R8G8B8A8Uint:
            case nxt::TextureFormat::B8G8R8A8Unorm:
                return false;
            case nxt::TextureFormat::D32FloatS8Uint:
                return true;
            default:
                UNREACHABLE();
        }
    }


    // TextureBase

    TextureBase::TextureBase(TextureBuilder* builder)
        : mDevice(builder->mDevice), mDimension(builder->mDimension), mFormat(builder->mFormat), mWidth(builder->mWidth),
        mHeight(builder->mHeight), mDepth(builder->mDepth), mNumMipLevels(builder->mNumMipLevels),
        mAllowedUsage(builder->mAllowedUsage), mCurrentUsage(builder->mCurrentUsage) {
    }

    DeviceBase* TextureBase::GetDevice() {
        return mDevice;
    }

    nxt::TextureDimension TextureBase::GetDimension() const {
        return mDimension;
    }
    nxt::TextureFormat TextureBase::GetFormat() const {
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
    nxt::TextureUsageBit TextureBase::GetAllowedUsage() const {
        return mAllowedUsage;
    }
    nxt::TextureUsageBit TextureBase::GetUsage() const {
        return mCurrentUsage;
    }

    TextureViewBuilder* TextureBase::CreateTextureViewBuilder() {
        return new TextureViewBuilder(mDevice, this);
    }

    bool TextureBase::IsFrozen() const {
        return mIsFrozen;
    }

    bool TextureBase::HasFrozenUsage(nxt::TextureUsageBit usage) const {
        return mIsFrozen && (usage & mAllowedUsage);
    }

    bool TextureBase::IsUsagePossible(nxt::TextureUsageBit allowedUsage, nxt::TextureUsageBit usage) {
        bool allowed = (usage & allowedUsage) == usage;
        bool singleUse = nxt::HasZeroOrOneBits(usage);
        return allowed && singleUse;
    }

    bool TextureBase::IsTransitionPossible(nxt::TextureUsageBit usage) const {
        if (mIsFrozen) {
            return false;
        }
        if (mCurrentUsage == nxt::TextureUsageBit::Present) {
            return false;
        }
        return IsUsagePossible(mAllowedUsage, usage);
    }

    void TextureBase::UpdateUsageInternal(nxt::TextureUsageBit usage) {
        ASSERT(IsTransitionPossible(usage));
        mCurrentUsage = usage;
    }

    void TextureBase::TransitionUsage(nxt::TextureUsageBit usage) {
        if (!IsTransitionPossible(usage)) {
            mDevice->HandleError("Texture frozen or usage not allowed");
            return;
        }
        TransitionUsageImpl(mCurrentUsage, usage);
        UpdateUsageInternal(usage);
    }

    void TextureBase::FreezeUsage(nxt::TextureUsageBit usage) {
        if (!IsTransitionPossible(usage)) {
            mDevice->HandleError("Texture frozen or usage not allowed");
            return;
        }
        mAllowedUsage = usage;
        TransitionUsageImpl(mCurrentUsage, usage);
        UpdateUsageInternal(usage);
        mIsFrozen = true;
    }

    // TextureBuilder

    enum TextureSetProperties {
        TEXTURE_PROPERTY_DIMENSION = 0x1,
        TEXTURE_PROPERTY_EXTENT = 0x2,
        TEXTURE_PROPERTY_FORMAT = 0x4,
        TEXTURE_PROPERTY_MIP_LEVELS = 0x8,
        TEXTURE_PROPERTY_ALLOWED_USAGE = 0x10,
        TEXTURE_PROPERTY_INITIAL_USAGE = 0x20,
    };

    TextureBuilder::TextureBuilder(DeviceBase* device)
        : Builder(device) {
    }

    TextureBase* TextureBuilder::GetResultImpl() {
        constexpr int allProperties = TEXTURE_PROPERTY_DIMENSION | TEXTURE_PROPERTY_EXTENT |
            TEXTURE_PROPERTY_FORMAT | TEXTURE_PROPERTY_MIP_LEVELS | TEXTURE_PROPERTY_ALLOWED_USAGE;
        if ((mPropertiesSet & allProperties) != allProperties) {
            HandleError("Texture missing properties");
            return nullptr;
        }

        if (!TextureBase::IsUsagePossible(mAllowedUsage, mCurrentUsage)) {
            HandleError("Initial texture usage is not allowed");
            return nullptr;
        }

        // TODO(cwallez@chromium.org): check stuff based on the dimension

        return mDevice->CreateTexture(this);
    }

    void TextureBuilder::SetDimension(nxt::TextureDimension dimension) {
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

    void TextureBuilder::SetFormat(nxt::TextureFormat format) {
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

    void TextureBuilder::SetAllowedUsage(nxt::TextureUsageBit usage) {
        if ((mPropertiesSet & TEXTURE_PROPERTY_ALLOWED_USAGE) != 0) {
            HandleError("Texture allowed usage property set multiple times");
            return;
        }

        mPropertiesSet |= TEXTURE_PROPERTY_ALLOWED_USAGE;
        mAllowedUsage = usage;
    }

    void TextureBuilder::SetInitialUsage(nxt::TextureUsageBit usage) {
        if ((mPropertiesSet & TEXTURE_PROPERTY_INITIAL_USAGE) != 0) {
            HandleError("Texture initial usage property set multiple times");
            return;
        }

        mPropertiesSet |= TEXTURE_PROPERTY_INITIAL_USAGE;
        mCurrentUsage = usage;
    }

    // TextureViewBase

    TextureViewBase::TextureViewBase(TextureViewBuilder* builder)
        : mTexture(builder->mTexture) {
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

}
