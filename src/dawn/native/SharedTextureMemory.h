// Copyright 2023 The Dawn & Tint Authors
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// 1. Redistributions of source code must retain the above copyright notice, this
//    list of conditions and the following disclaimer.
//
// 2. Redistributions in binary form must reproduce the above copyright notice,
//    this list of conditions and the following disclaimer in the documentation
//    and/or other materials provided with the distribution.
//
// 3. Neither the name of the copyright holder nor the names of its
//    contributors may be used to endorse or promote products derived from
//    this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
// FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
// DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
// CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
// OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

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

class SharedTextureMemoryContents;
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

    void Initialize();

    void APIGetProperties(SharedTextureMemoryProperties* properties) const;
    TextureBase* APICreateTexture(const TextureDescriptor* descriptor);
    // Returns true if access was acquired. If it returns true, then APIEndAccess must
    // be called to release access. Other errors may occur even if `true` is returned.
    // Use an error scope to catch them.
    bool APIBeginAccess(TextureBase* texture, const BeginAccessDescriptor* descriptor);
    // Returns true if access was released.
    bool APIEndAccess(TextureBase* texture, EndAccessState* state);

    ObjectType GetType() const override;

    SharedTextureMemoryContents* GetContents() const;

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
    virtual Ref<SharedTextureMemoryContents> CreateContents();

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

    Ref<SharedTextureMemoryContents> mContents;
};

// SharedTextureMemoryContents is a separate object because it needs to live as long as
// the SharedTextureMemory or any textures created from the SharedTextureMemory. This
// allows state and objects needed by the texture to persist after the
// SharedTextureMemory itself has been dropped.
class SharedTextureMemoryContents : public RefCounted {
  public:
    using PendingFenceList = SharedTextureMemoryBase::PendingFenceList;

    explicit SharedTextureMemoryContents(WeakRef<SharedTextureMemoryBase> sharedTextureMemory);

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
