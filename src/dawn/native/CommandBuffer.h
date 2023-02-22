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

#ifndef SRC_DAWN_NATIVE_COMMANDBUFFER_H_
#define SRC_DAWN_NATIVE_COMMANDBUFFER_H_

#include "dawn/native/dawn_platform.h"

#include "dawn/native/CommandAllocator.h"
#include "dawn/native/Error.h"
#include "dawn/native/Forward.h"
#include "dawn/native/ObjectBase.h"
#include "dawn/native/PassResourceUsage.h"
#include "dawn/native/Texture.h"

namespace dawn::native {

struct BeginRenderPassCmd;
struct CopyTextureToBufferCmd;
struct TextureCopy;

class CommandBufferBase : public ApiObjectBase {
  public:
    CommandBufferBase(CommandEncoder* encoder, const CommandBufferDescriptor* descriptor);

    static CommandBufferBase* MakeError(DeviceBase* device);

    ObjectType GetType() const override;

    MaybeError ValidateCanUseInSubmitNow() const;

    const CommandBufferResourceUsage& GetResourceUsages() const;

    CommandIterator* GetCommandIteratorForTesting();

  protected:
    void DestroyImpl() override;

    CommandIterator mCommands;

  private:
    CommandBufferBase(DeviceBase* device, ObjectBase::ErrorTag tag);

    CommandBufferResourceUsage mResourceUsages;
};

bool IsCompleteSubresourceCopiedTo(const TextureBase* texture,
                                   const Extent3D copySize,
                                   const uint32_t mipLevel);
SubresourceRange GetSubresourcesAffectedByCopy(const TextureCopy& copy, const Extent3D& copySize);

void LazyClearRenderPassAttachments(BeginRenderPassCmd* renderPass);

bool IsFullBufferOverwrittenInTextureToBufferCopy(const CopyTextureToBufferCmd* copy);

std::array<float, 4> ConvertToFloatColor(dawn::native::Color color);
std::array<int32_t, 4> ConvertToSignedIntegerColor(dawn::native::Color color);
std::array<uint32_t, 4> ConvertToUnsignedIntegerColor(dawn::native::Color color);

}  // namespace dawn::native

#endif  // SRC_DAWN_NATIVE_COMMANDBUFFER_H_
