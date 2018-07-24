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

#ifndef DAWNNATIVE_TEXTURE_H_
#define DAWNNATIVE_TEXTURE_H_

#include "dawn_native/Builder.h"
#include "dawn_native/Forward.h"
#include "dawn_native/RefCounted.h"

#include "dawn/dawncpp.h"

namespace backend {

    uint32_t TextureFormatPixelSize(dawn::TextureFormat format);
    bool TextureFormatHasDepth(dawn::TextureFormat format);
    bool TextureFormatHasStencil(dawn::TextureFormat format);
    bool TextureFormatHasDepthOrStencil(dawn::TextureFormat format);

    static constexpr dawn::TextureUsageBit kReadOnlyTextureUsages =
        dawn::TextureUsageBit::TransferSrc | dawn::TextureUsageBit::Sampled |
        dawn::TextureUsageBit::Present;

    static constexpr dawn::TextureUsageBit kWritableTextureUsages =
        dawn::TextureUsageBit::TransferDst | dawn::TextureUsageBit::Storage |
        dawn::TextureUsageBit::OutputAttachment;

    class TextureBase : public RefCounted {
      public:
        TextureBase(TextureBuilder* builder);

        dawn::TextureDimension GetDimension() const;
        dawn::TextureFormat GetFormat() const;
        uint32_t GetWidth() const;
        uint32_t GetHeight() const;
        uint32_t GetDepth() const;
        uint32_t GetNumMipLevels() const;
        dawn::TextureUsageBit GetAllowedUsage() const;
        DeviceBase* GetDevice() const;

        // Dawn API
        TextureViewBuilder* CreateTextureViewBuilder();

      private:
        DeviceBase* mDevice;

        dawn::TextureDimension mDimension;
        dawn::TextureFormat mFormat;
        uint32_t mWidth, mHeight, mDepth;
        uint32_t mNumMipLevels;
        dawn::TextureUsageBit mAllowedUsage = dawn::TextureUsageBit::None;
    };

    class TextureBuilder : public Builder<TextureBase> {
      public:
        TextureBuilder(DeviceBase* device);

        // Dawn API
        void SetDimension(dawn::TextureDimension dimension);
        void SetExtent(uint32_t width, uint32_t height, uint32_t depth);
        void SetFormat(dawn::TextureFormat format);
        void SetMipLevels(uint32_t numMipLevels);
        void SetAllowedUsage(dawn::TextureUsageBit usage);
        void SetInitialUsage(dawn::TextureUsageBit usage);

      private:
        friend class TextureBase;

        TextureBase* GetResultImpl() override;

        int mPropertiesSet = 0;

        dawn::TextureDimension mDimension;
        uint32_t mWidth, mHeight, mDepth;
        dawn::TextureFormat mFormat;
        uint32_t mNumMipLevels;
        dawn::TextureUsageBit mAllowedUsage = dawn::TextureUsageBit::None;
    };

    class TextureViewBase : public RefCounted {
      public:
        TextureViewBase(TextureViewBuilder* builder);

        const TextureBase* GetTexture() const;
        TextureBase* GetTexture();

      private:
        Ref<TextureBase> mTexture;
    };

    class TextureViewBuilder : public Builder<TextureViewBase> {
      public:
        TextureViewBuilder(DeviceBase* device, TextureBase* texture);

      private:
        friend class TextureViewBase;

        TextureViewBase* GetResultImpl() override;

        Ref<TextureBase> mTexture;
    };

}  // namespace backend

#endif  // DAWNNATIVE_TEXTURE_H_
