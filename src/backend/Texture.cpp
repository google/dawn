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

namespace backend {

    size_t TextureFormatPixelSize(nxt::TextureFormat format) {
        switch (format) {
            case nxt::TextureFormat::R8G8B8A8Unorm:
                return 4;
        }
    }

    // TextureBase

    TextureBase::TextureBase(TextureBuilder* builder)
        : device(builder->device), dimension(builder->dimension), format(builder->format), width(builder->width),
        height(builder->height), depth(builder->depth), numMipLevels(builder->numMipLevels),
        allowedUsage(builder->allowedUsage), currentUsage(builder->currentUsage) {
    }

    DeviceBase* TextureBase::GetDevice() {
        return device;
    }

    nxt::TextureDimension TextureBase::GetDimension() const {
        return dimension;
    }
    nxt::TextureFormat TextureBase::GetFormat() const {
        return format;
    }
    uint32_t TextureBase::GetWidth() const {
        return width;
    }
    uint32_t TextureBase::GetHeight() const {
        return height;
    }
    uint32_t TextureBase::GetDepth() const {
        return depth;
    }
    uint32_t TextureBase::GetNumMipLevels() const {
        return numMipLevels;
    }
    nxt::TextureUsageBit TextureBase::GetAllowedUsage() const {
        return allowedUsage;
    }
    nxt::TextureUsageBit TextureBase::GetUsage() const {
        return currentUsage;
    }

    TextureViewBuilder* TextureBase::CreateTextureViewBuilder() {
        return new TextureViewBuilder(device, this);
    }

    bool TextureBase::IsFrozen() const {
        return frozen;
    }

    bool TextureBase::HasFrozenUsage(nxt::TextureUsageBit usage) const {
        return frozen && (usage & allowedUsage);
    }

    bool TextureBase::IsUsagePossible(nxt::TextureUsageBit allowedUsage, nxt::TextureUsageBit usage) {
        bool allowed = (usage & allowedUsage) == usage;
        bool singleUse = nxt::HasZeroOrOneBits(usage);
        return allowed && singleUse;
    }

    bool TextureBase::IsTransitionPossible(nxt::TextureUsageBit usage) const {
        if (frozen) {
            return false;
        }
        return IsUsagePossible(allowedUsage, usage);
    }

    void TextureBase::UpdateUsageInternal(nxt::TextureUsageBit usage) {
        assert(IsTransitionPossible(usage));
        currentUsage = usage;
    }

    void TextureBase::TransitionUsage(nxt::TextureUsageBit usage) {
        if (!IsTransitionPossible(usage)) {
            device->HandleError("Texture frozen or usage not allowed");
            return;
        }
        TransitionUsageImpl(currentUsage, usage);
        currentUsage = usage;
    }

    void TextureBase::FreezeUsage(nxt::TextureUsageBit usage) {
        if (!IsTransitionPossible(usage)) {
            device->HandleError("Texture frozen or usage not allowed");
            return;
        }
        allowedUsage = usage;
        TransitionUsageImpl(currentUsage, usage);
        currentUsage = usage;
        frozen = true;
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
        if ((propertiesSet & allProperties) != allProperties) {
            HandleError("Texture missing properties");
            return nullptr;
        }

        if (!TextureBase::IsUsagePossible(allowedUsage, currentUsage)) {
            HandleError("Initial texture usage is not allowed");
            return nullptr;
        }

        // TODO(cwallez@chromium.org): check stuff based on the dimension

        return device->CreateTexture(this);
    }

    void TextureBuilder::SetDimension(nxt::TextureDimension dimension) {
        if ((propertiesSet & TEXTURE_PROPERTY_DIMENSION) != 0) {
            HandleError("Texture dimension property set multiple times");
            return;
        }

        propertiesSet |= TEXTURE_PROPERTY_DIMENSION;
        this->dimension = dimension;
    }

    void TextureBuilder::SetExtent(uint32_t width, uint32_t height, uint32_t depth) {
        if ((propertiesSet & TEXTURE_PROPERTY_EXTENT) != 0) {
            HandleError("Texture extent property set multiple times");
            return;
        }

        if (width == 0 || height == 0 || depth == 0) {
            HandleError("Cannot create an empty texture");
            return;
        }

        propertiesSet |= TEXTURE_PROPERTY_EXTENT;
        this->width = width;
        this->height = height;
        this->depth = depth;
    }

    void TextureBuilder::SetFormat(nxt::TextureFormat format) {
        if ((propertiesSet & TEXTURE_PROPERTY_FORMAT) != 0) {
            HandleError("Texture format property set multiple times");
            return;
        }

        propertiesSet |= TEXTURE_PROPERTY_FORMAT;
        this->format = format;
    }

    void TextureBuilder::SetMipLevels(uint32_t numMipLevels) {
        if ((propertiesSet & TEXTURE_PROPERTY_MIP_LEVELS) != 0) {
            HandleError("Texture mip levels property set multiple times");
            return;
        }

        propertiesSet |= TEXTURE_PROPERTY_MIP_LEVELS;
        this->numMipLevels = numMipLevels;
    }

    void TextureBuilder::SetAllowedUsage(nxt::TextureUsageBit usage) {
        if ((propertiesSet & TEXTURE_PROPERTY_ALLOWED_USAGE) != 0) {
            HandleError("Texture allowed usage property set multiple times");
            return;
        }

        propertiesSet |= TEXTURE_PROPERTY_ALLOWED_USAGE;
        this->allowedUsage = usage;
    }

    void TextureBuilder::SetInitialUsage(nxt::TextureUsageBit usage) {
        if ((propertiesSet & TEXTURE_PROPERTY_INITIAL_USAGE) != 0) {
            HandleError("Texture initial usage property set multiple times");
            return;
        }

        propertiesSet |= TEXTURE_PROPERTY_INITIAL_USAGE;
        this->currentUsage = usage;
    }

    // TextureViewBase

    TextureViewBase::TextureViewBase(TextureViewBuilder* builder)
        : texture(builder->texture) {
    }

    TextureBase* TextureViewBase::GetTexture() {
        return texture.Get();
    }

    // TextureViewBuilder

    TextureViewBuilder::TextureViewBuilder(DeviceBase* device, TextureBase* texture)
        : Builder(device), texture(texture) {
    }

    TextureViewBase* TextureViewBuilder::GetResultImpl() {
        return device->CreateTextureView(this);
    }

}
