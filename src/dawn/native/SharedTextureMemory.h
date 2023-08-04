// Copyright 2023 The Dawn Authors
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

#ifndef SRC_DAWN_NATIVE_SHAREDTEXTUREMEMORY_H_
#define SRC_DAWN_NATIVE_SHAREDTEXTUREMEMORY_H_

#include "dawn/native/ObjectBase.h"

namespace dawn::native {

struct SharedTextureMemoryDescriptor;
struct SharedTextureMemoryBeginAccessDescriptor;
struct SharedTextureMemoryEndAccessState;
struct SharedTextureMemoryProperties;
struct TextureDescriptor;

class SharedTextureMemoryBase : public ApiObjectBase {
  public:
    using BeginAccessDescriptor = SharedTextureMemoryBeginAccessDescriptor;
    using EndAccessState = SharedTextureMemoryEndAccessState;

    static SharedTextureMemoryBase* MakeError(DeviceBase* device,
                                              const SharedTextureMemoryDescriptor* descriptor);

    void APIGetProperties(SharedTextureMemoryProperties* properties) const;
    TextureBase* APICreateTexture(const TextureDescriptor* descriptor);
    void APIBeginAccess(TextureBase* texture, const BeginAccessDescriptor* descriptor);
    void APIEndAccess(TextureBase* texture, EndAccessState* state);

    ObjectType GetType() const override;

  private:
    void DestroyImpl() override;

    SharedTextureMemoryBase(DeviceBase* device,
                            const SharedTextureMemoryDescriptor* descriptor,
                            ObjectBase::ErrorTag tag);
};

}  // namespace dawn::native

#endif  // SRC_DAWN_NATIVE_SHAREDTEXTUREMEMORY_H_
