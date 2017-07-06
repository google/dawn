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

#ifndef BACKEND_TEXTURE_H_
#define BACKEND_TEXTURE_H_

#include "backend/Forward.h"
#include "backend/Builder.h"
#include "backend/RefCounted.h"

#include "nxt/nxtcpp.h"

namespace backend {

    size_t TextureFormatPixelSize(nxt::TextureFormat format);

    class TextureBase : public RefCounted {
        public:
            TextureBase(TextureBuilder* builder);

            nxt::TextureDimension GetDimension() const;
            nxt::TextureFormat GetFormat() const;
            uint32_t GetWidth() const;
            uint32_t GetHeight() const;
            uint32_t GetDepth() const;
            uint32_t GetNumMipLevels() const;
            nxt::TextureUsageBit GetAllowedUsage() const;
            nxt::TextureUsageBit GetUsage() const;
            bool IsFrozen() const;
            bool HasFrozenUsage(nxt::TextureUsageBit usage) const;
            static bool IsUsagePossible(nxt::TextureUsageBit allowedUsage, nxt::TextureUsageBit usage);
            bool IsTransitionPossible(nxt::TextureUsageBit usage) const;
            void UpdateUsageInternal(nxt::TextureUsageBit usage);

            DeviceBase* GetDevice();

            // NXT API
            TextureViewBuilder* CreateTextureViewBuilder();
            void TransitionUsage(nxt::TextureUsageBit usage);
            void FreezeUsage(nxt::TextureUsageBit usage);

        private:
            virtual void TransitionUsageImpl(nxt::TextureUsageBit currentUsage, nxt::TextureUsageBit targetUsage) = 0;

            DeviceBase* device;

            nxt::TextureDimension dimension;
            nxt::TextureFormat format;
            uint32_t width, height, depth;
            uint32_t numMipLevels;
            nxt::TextureUsageBit allowedUsage = nxt::TextureUsageBit::None;
            nxt::TextureUsageBit currentUsage = nxt::TextureUsageBit::None;
            bool frozen = false;
    };

    class TextureBuilder : public Builder<TextureBase> {
        public:
            TextureBuilder(DeviceBase* device);

            // NXT API
            void SetDimension(nxt::TextureDimension dimension);
            void SetExtent(uint32_t width, uint32_t height, uint32_t depth);
            void SetFormat(nxt::TextureFormat format);
            void SetMipLevels(uint32_t numMipLevels);
            void SetAllowedUsage(nxt::TextureUsageBit usage);
            void SetInitialUsage(nxt::TextureUsageBit usage);

        private:
            friend class TextureBase;

            TextureBase* GetResultImpl() override;

            int propertiesSet = 0;

            nxt::TextureDimension dimension;
            uint32_t width, height, depth;
            nxt::TextureFormat format;
            uint32_t numMipLevels;
            nxt::TextureUsageBit allowedUsage = nxt::TextureUsageBit::None;
            nxt::TextureUsageBit currentUsage = nxt::TextureUsageBit::None;
    };

    class TextureViewBase : public RefCounted {
        public:
            TextureViewBase(TextureViewBuilder* builder);

            TextureBase* GetTexture();

        private:
            Ref<TextureBase> texture;
    };

    class TextureViewBuilder : public Builder<TextureViewBase> {
        public:
            TextureViewBuilder(DeviceBase* device, TextureBase* texture);

        private:
            friend class TextureViewBase;

            TextureViewBase* GetResultImpl() override;

            Ref<TextureBase> texture;
    };

}

#endif // BACKEND_TEXTURE_H_
