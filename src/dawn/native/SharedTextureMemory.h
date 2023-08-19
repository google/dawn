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

#include <map>
#include <stack>

#include "dawn/common/StackContainer.h"
#include "dawn/common/WeakRefSupport.h"
#include "dawn/native/Error.h"
#include "dawn/native/IntegerTypes.h"
#include "dawn/native/ObjectBase.h"
#include "dawn/native/SharedFence.h"
#include "dawn/native/dawn_platform.h"

namespace dawn::native {

struct SharedTextureMemoryDescriptor;
struct SharedTextureMemoryBeginAccessDescriptor;
struct SharedTextureMemoryEndAccessState;
struct SharedTextureMemoryProperties;
struct TextureDescriptor;

class SharedTextureMemoryBase : public ApiObjectBase,
                                public WeakRefSupport<SharedTextureMemoryBase> {
  public:
    using BeginAccessDescriptor = SharedTextureMemoryBeginAccessDescriptor;
    using EndAccessState = SharedTextureMemoryEndAccessState;
    using PendingFenceList = StackVector<FenceAndSignalValue, 1>;

    static SharedTextureMemoryBase* MakeError(DeviceBase* device,
                                              const SharedTextureMemoryDescriptor* descriptor);

    void APIGetProperties(SharedTextureMemoryProperties* properties) const;
    TextureBase* APICreateTexture(const TextureDescriptor* descriptor);
    void APIBeginAccess(TextureBase* texture, const BeginAccessDescriptor* descriptor);
    void APIEndAccess(TextureBase* texture, EndAccessState* state);

    ObjectType GetType() const override;

    // Acquire the begin fences for the current access scope on `texture`.
    void AcquireBeginFences(TextureBase* texture, PendingFenceList* fences);

    // Set the last usage serial. This indicates when the SharedFence exported
    // from APIEndAccess will complete.
    void SetLastUsageSerial(ExecutionSerial lastUsageSerial);
    ExecutionSerial GetLastUsageSerial() const;

  protected:
    SharedTextureMemoryBase(DeviceBase* device,
                            const char* label,
                            const SharedTextureMemoryProperties& properties);
    SharedTextureMemoryBase(DeviceBase* device,
                            const SharedTextureMemoryDescriptor* descriptor,
                            ObjectBase::ErrorTag tag);

    void DestroyImpl() override;

    const SharedTextureMemoryProperties mProperties;

    Ref<TextureBase> mCurrentAccess;

  private:
    ResultOrError<Ref<TextureBase>> CreateTexture(const TextureDescriptor* descriptor);
    MaybeError BeginAccess(TextureBase* texture, const BeginAccessDescriptor* descriptor);
    MaybeError EndAccess(TextureBase* texture, EndAccessState* state);
    ResultOrError<FenceAndSignalValue> EndAccessInternal(TextureBase* texture,
                                                         EndAccessState* state);

    virtual ResultOrError<Ref<TextureBase>> CreateTextureImpl(
        const TextureDescriptor* descriptor) = 0;

    // BeginAccessImpl validates the operation is valid on the backend, and performs any
    // backend specific operations. It does NOT need to acquire begin fences; that is done in the
    // frontend in BeginAccess.
    virtual MaybeError BeginAccessImpl(TextureBase* texture,
                                       const BeginAccessDescriptor* descriptor) = 0;
    // EndAccessImpl validates the operation is valid on the backend, and returns the end fence.
    virtual ResultOrError<FenceAndSignalValue> EndAccessImpl(TextureBase* texture) = 0;

    // Begin an access scope on `texture`. Passing a list of fences that should be waited on before
    // use.
    void PushAccessFences(TextureBase* texture,
                          const SharedTextureMemoryBeginAccessDescriptor* descriptor);
    // End an access scope on `texture`, writing out any fences that have not yet been acquired.
    void PopAccessFences(TextureBase* texture, PendingFenceList* fences);

    // Map of texture -> stack of PendingFenceList.
    std::map<Ref<TextureBase>, std::stack<PendingFenceList>> mAccessScopes;
    ExecutionSerial mLastUsageSerial{0};
};

}  // namespace dawn::native

#endif  // SRC_DAWN_NATIVE_SHAREDTEXTUREMEMORY_H_
