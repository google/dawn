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

#ifndef SRC_DAWN_NATIVE_RENDERBUNDLE_H_
#define SRC_DAWN_NATIVE_RENDERBUNDLE_H_

#include <bitset>

#include "dawn/common/Constants.h"
#include "dawn/native/AttachmentState.h"
#include "dawn/native/CommandAllocator.h"
#include "dawn/native/Error.h"
#include "dawn/native/Forward.h"
#include "dawn/native/IndirectDrawMetadata.h"
#include "dawn/native/ObjectBase.h"
#include "dawn/native/PassResourceUsage.h"

#include "dawn/native/dawn_platform.h"

namespace dawn::native {

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
    uint64_t GetDrawCount() const;
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
    uint64_t mDrawCount;
    RenderPassResourceUsage mResourceUsage;
};

}  // namespace dawn::native

#endif  // SRC_DAWN_NATIVE_RENDERBUNDLE_H_
