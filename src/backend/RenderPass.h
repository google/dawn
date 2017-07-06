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

#ifndef BACKEND_RENDERPASS_H_
#define BACKEND_RENDERPASS_H_

#include "backend/Builder.h"
#include "backend/Forward.h"
#include "backend/RefCounted.h"
#include "common/Constants.h"

#include "nxt/nxtcpp.h"

#include <array>
#include <bitset>
#include <vector>

namespace backend {

    class RenderPassBase : public RefCounted {
        public:
            RenderPassBase(RenderPassBuilder* builder);

            struct AttachmentInfo {
                nxt::TextureFormat format;
            };

            struct SubpassInfo {
                std::bitset<kMaxColorAttachments> colorAttachmentsSet;
                std::array<uint32_t, kMaxColorAttachments> colorAttachments;
            };

            uint32_t GetAttachmentCount() const;
            const AttachmentInfo& GetAttachmentInfo(uint32_t attachment) const;
            uint32_t GetSubpassCount() const;
            const SubpassInfo& GetSubpassInfo(uint32_t subpass) const;
            bool IsCompatibleWith(const RenderPassBase* other) const;

        private:
            std::vector<AttachmentInfo> attachments;
            std::vector<SubpassInfo> subpasses;
    };

    class RenderPassBuilder : public Builder<RenderPassBase> {
        public:
            RenderPassBuilder(DeviceBase* device);

            bool WasConsumed() const;

            // NXT API
            RenderPassBase* GetResultImpl() override;
            void SetAttachmentCount(uint32_t attachmentCount);
            void AttachmentSetFormat(uint32_t attachmentSlot, nxt::TextureFormat format);
            void SetSubpassCount(uint32_t subpassCount);
            void SubpassSetColorAttachment(uint32_t subpass, uint32_t outputAttachmentLocation, uint32_t attachmentSlot);

        private:
            friend class RenderPassBase;

            enum AttachmentProperty {
                ATTACHMENT_PROPERTY_FORMAT,
                ATTACHMENT_PROPERTY_COUNT
            };

            std::vector<std::bitset<ATTACHMENT_PROPERTY_COUNT>> attachmentProperties;
            std::vector<RenderPassBase::AttachmentInfo> attachments;
            std::vector<RenderPassBase::SubpassInfo> subpasses;
            int propertiesSet = 0;
    };

}

#endif // BACKEND_RENDERPASS_H_
