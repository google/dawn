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
#include "dawn/common/WeakRef.h"
#include "dawn/common/WeakRefSupport.h"
#include "dawn/native/Error.h"
#include "dawn/native/IntegerTypes.h"
#include "dawn/native/ObjectBase.h"
#include "dawn/native/SharedFence.h"
#include "dawn/native/dawn_platform.h"

namespace dawn::native {

class SharedTextureMemoryState;
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
    // Returns true if access was acquired. If it returns true, then APIEndAccess must
    // be called to release access. Other errors may occur even if `true` is returned.
    // Use an error scope to catch them.
    bool APIBeginAccess(TextureBase* texture, const BeginAccessDescriptor* descriptor);
    // Returns true if access was released.
    bool APIEndAccess(TextureBase* texture, EndAccessState* state);

    ObjectType GetType() const override;

    SharedTextureMemoryState* GetState() const;

    // Validate that the texture was created from this SharedTextureMemory.
    MaybeError ValidateTextureCreatedFromSelf(TextureBase* texture);

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

    Ref<SharedTextureMemoryState> mState;
};

// SharedTextureMemoryState is a separate object because it needs to live as long as
// the SharedTextureMemory or any textures created from the SharedTextureMemory. This
// allows state needed by the texture to persist after the SharedTextureMemory itself
// has been dropped.
class SharedTextureMemoryState : public RefCounted {
  public:
    using PendingFenceList = SharedTextureMemoryBase::PendingFenceList;

    explicit SharedTextureMemoryState(WeakRef<SharedTextureMemoryBase> sharedTextureMemory);

    void AcquirePendingFences(PendingFenceList* fences);

    // Set the last usage serial. This indicates when the SharedFence exported
    // from APIEndAccess will complete.
    void SetLastUsageSerial(ExecutionSerial lastUsageSerial);
    ExecutionSerial GetLastUsageSerial() const;

    const WeakRef<SharedTextureMemoryBase>& GetSharedTextureMemory() const;

  private:
    friend class SharedTextureMemoryBase;

    PendingFenceList mPendingFences;
    ExecutionSerial mLastUsageSerial{0};

    WeakRef<SharedTextureMemoryBase> mSharedTextureMemory;
};

}  // namespace dawn::native

#endif  // SRC_DAWN_NATIVE_SHAREDTEXTUREMEMORY_H_
