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
#include "dawn_native/Error.h"
#include "dawn_native/Forward.h"
#include "dawn_native/ObjectBase.h"

#include "dawn_native/dawn_platform.h"

namespace dawn_native {
    MaybeError ValidateTextureDescriptor(DeviceBase* device, const TextureDescriptor* descriptor);
    MaybeError ValidateTextureViewDescriptor(const DeviceBase* device,
                                             const TextureBase* texture,
                                             const TextureViewDescriptor* descriptor);

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

    class TextureBase : public ObjectBase {
      public:
        TextureBase(DeviceBase* device, const TextureDescriptor* descriptor);

        dawn::TextureDimension GetDimension() const;
        dawn::TextureFormat GetFormat() const;
        const Extent3D& GetSize() const;
        uint32_t GetArrayLayers() const;
        uint32_t GetNumMipLevels() const;
        dawn::TextureUsageBit GetUsage() const;

        // Dawn API
        TextureViewBase* CreateDefaultTextureView();
        TextureViewBase* CreateTextureView(const TextureViewDescriptor* descriptor);

      private:
        dawn::TextureDimension mDimension;
        dawn::TextureFormat mFormat;
        Extent3D mSize;
        uint32_t mArrayLayers;
        uint32_t mNumMipLevels;
        dawn::TextureUsageBit mUsage = dawn::TextureUsageBit::None;
    };

    class TextureViewBase : public ObjectBase {
      public:
        TextureViewBase(TextureBase* texture, const TextureViewDescriptor* descriptor);

        const TextureBase* GetTexture() const;
        TextureBase* GetTexture();

      private:
        Ref<TextureBase> mTexture;
    };

}  // namespace dawn_native

#endif  // DAWNNATIVE_TEXTURE_H_
