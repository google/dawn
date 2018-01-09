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

#ifndef BACKEND_FRAMEBUFFER_H_
#define BACKEND_FRAMEBUFFER_H_

#include "backend/Builder.h"
#include "backend/Forward.h"
#include "backend/RefCounted.h"
#include "backend/Texture.h"

#include "nxt/nxtcpp.h"

#include <type_traits>
#include <vector>

namespace backend {

    class FramebufferBase : public RefCounted {
      public:
        struct ClearColor {
            float color[4] = {};
        };

        struct ClearDepthStencil {
            float depth = 1.0f;
            uint32_t stencil = 0;
        };

        FramebufferBase(FramebufferBuilder* builder);

        DeviceBase* GetDevice();
        RenderPassBase* GetRenderPass();
        TextureViewBase* GetTextureView(uint32_t attachmentSlot);
        ClearColor GetClearColor(uint32_t attachmentSlot);
        ClearDepthStencil GetClearDepthStencil(uint32_t attachmentSlot);
        uint32_t GetWidth() const;
        uint32_t GetHeight() const;

        // NXT API
        void AttachmentSetClearColor(uint32_t attachmentSlot,
                                     float clearR,
                                     float clearG,
                                     float clearB,
                                     float clearA);
        void AttachmentSetClearDepthStencil(uint32_t attachmentSlot,
                                            float clearDepth,
                                            uint32_t clearStencil);

      private:
        DeviceBase* mDevice;
        Ref<RenderPassBase> mRenderPass;
        uint32_t mWidth = 0;
        uint32_t mHeight = 0;
        std::vector<Ref<TextureViewBase>> mTextureViews;
        std::vector<ClearColor> mClearColors;
        std::vector<ClearDepthStencil> mClearDepthStencils;
    };

    class FramebufferBuilder : public Builder<FramebufferBase> {
      public:
        FramebufferBuilder(DeviceBase* device);

        // NXT API
        FramebufferBase* GetResultImpl() override;
        void SetRenderPass(RenderPassBase* renderPass);
        void SetDimensions(uint32_t width, uint32_t height);
        void SetAttachment(uint32_t attachmentSlot, TextureViewBase* textureView);

      private:
        friend class FramebufferBase;

        Ref<RenderPassBase> mRenderPass;
        uint32_t mWidth = 0;
        uint32_t mHeight = 0;
        std::vector<Ref<TextureViewBase>> mTextureViews;
        int mPropertiesSet = 0;
    };

}  // namespace backend

#endif  // BACKEND_FRAMEBUFFER_H_
