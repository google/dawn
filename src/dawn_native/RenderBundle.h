// Copyright 2019 The Dawn Authors
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

#ifndef DAWNNATIVE_RENDERBUNDLE_H_
#define DAWNNATIVE_RENDERBUNDLE_H_

#include "common/Constants.h"
#include "dawn_native/AttachmentState.h"
#include "dawn_native/CommandAllocator.h"
#include "dawn_native/Error.h"
#include "dawn_native/Forward.h"
#include "dawn_native/IndirectDrawMetadata.h"
#include "dawn_native/ObjectBase.h"
#include "dawn_native/PassResourceUsage.h"

#include "dawn_native/dawn_platform.h"

#include <bitset>

namespace dawn_native {

    struct RenderBundleDescriptor;
    class RenderBundleEncoder;

    class RenderBundleBase final : public ApiObjectBase {
      public:
        RenderBundleBase(RenderBundleEncoder* encoder,
                         const RenderBundleDescriptor* descriptor,
                         Ref<AttachmentState> attachmentState,
                         bool depthReadOnly,
                         bool stencilReadOnly,
                         RenderPassResourceUsage resourceUsage,
                         IndirectDrawMetadata indirectDrawMetadata);

        static RenderBundleBase* MakeError(DeviceBase* device);

        ObjectType GetType() const override;

        CommandIterator* GetCommands();

        const AttachmentState* GetAttachmentState() const;
        bool IsDepthReadOnly() const;
        bool IsStencilReadOnly() const;
        const RenderPassResourceUsage& GetResourceUsage() const;
        const IndirectDrawMetadata& GetIndirectDrawMetadata();

      private:
        RenderBundleBase(DeviceBase* device, ErrorTag errorTag);

        void DestroyImpl() override;

        CommandIterator mCommands;
        IndirectDrawMetadata mIndirectDrawMetadata;
        Ref<AttachmentState> mAttachmentState;
        bool mDepthReadOnly;
        bool mStencilReadOnly;
        RenderPassResourceUsage mResourceUsage;
    };

}  // namespace dawn_native

#endif  // DAWNNATIVE_RENDERBUNDLE_H_
