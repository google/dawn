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

#ifndef BACKEND_COMMON_FRAMEBUFFER_H_
#define BACKEND_COMMON_FRAMEBUFFER_H_

#include "Builder.h"
#include "Forward.h"
#include "RefCounted.h"

#include "nxt/nxtcpp.h"

#include <type_traits>
#include <vector>

namespace backend {

    class FramebufferBase : public RefCounted {
        public:
            FramebufferBase(FramebufferBuilder* builder);

            RenderPassBase* GetRenderPass();
            TextureViewBase* GetTextureView(uint32_t index);
            uint32_t GetWidth() const;
            uint32_t GetHeight() const;

        private:
            Ref<RenderPassBase> renderPass;
            uint32_t width = 0;
            uint32_t height = 0;
            std::vector<Ref<TextureViewBase>> textureViews;
    };

    class FramebufferBuilder : public Builder<FramebufferBase> {
        public:
            FramebufferBuilder(DeviceBase* device);

            bool WasConsumed() const;

            // NXT API
            FramebufferBase* GetResultImpl() override;
            void SetRenderPass(RenderPassBase* renderPass);
            void SetDimensions(uint32_t width, uint32_t height);
            void SetAttachment(uint32_t attachmentSlot, TextureViewBase* textureView);

        private:
            friend class FramebufferBase;

            Ref<RenderPassBase> renderPass;
            uint32_t width = 0;
            uint32_t height = 0;
            std::vector<Ref<TextureViewBase>> textureViews;
            int propertiesSet = 0;
    };

}

#endif // BACKEND_COMMON_FRAMEBUFFER_H_
