// Copyright 2025 The Dawn & Tint Authors
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

#ifndef SRC_DAWN_NATIVE_DYNAMICARRAYSTATE_H_
#define SRC_DAWN_NATIVE_DYNAMICARRAYSTATE_H_

#include <vector>

#include "dawn/common/NonMovable.h"
#include "dawn/common/Ref.h"
#include "dawn/common/RefCounted.h"
#include "dawn/common/WeakRefSupport.h"
#include "dawn/common/ityp_span.h"
#include "dawn/common/ityp_vector.h"
#include "dawn/native/Error.h"
#include "dawn/native/Forward.h"
#include "dawn/native/IntegerTypes.h"
#include "dawn/native/dawn_platform.h"
#include "partition_alloc/pointers/raw_ptr.h"

namespace tint {
enum class ResourceType : uint32_t;
}  // namespace tint

namespace dawn::native {

// Returns the order in which we will put the default bindings at the end of the dynamic binding
// array.
ityp::span<BindingIndex, const tint::ResourceType> GetDefaultBindingOrder(
    wgpu::DynamicBindingKind kind);
BindingIndex GetDefaultBindingCount(wgpu::DynamicBindingKind kind);

// An optional component of a BindGroup that's used to track the resources that are in the dynamic
// binding array part. It helps maintain the metadata buffer that's used in shaders to know if it is
// valid to access an entry of the dynamic binding array with a given type (note that the writing of
// the updates to the buffer are done by the backends).
//
// DynamicArrayState has a single strong reference owned by the BindGroup that created it, however
// all resources contained in the dynamic array need WeakRefs to update it on Pin/Unpin. (They use
// WeakRef to avoid a reference cycle between the dynamic array and its bindings).
class DynamicArrayState : public RefCounted, public WeakRefSupport<DynamicArrayState> {
  public:
    DynamicArrayState(DeviceBase* device, BindingIndex size, wgpu::DynamicBindingKind kind);
    ~DynamicArrayState() override;

    MaybeError Initialize();
    void Destroy();

    wgpu::DynamicBindingKind GetKind() const;
    BindingIndex GetAPISize() const;
    ityp::span<BindingIndex, const Ref<TextureViewBase>> GetBindings() const;
    BufferBase* GetMetadataBuffer() const;
    bool IsDestroyed() const;
    bool CanBeUpdated(BindingIndex slot) const;
    std::optional<BindingIndex> GetFreeSlot() const;

    // Methods that mutate the state of bindings in the dynamic array. They keep track of the
    // necessary metadata buffer updates required for dynamic type checks in the shader to match
    // what's in the binding array.
    // `contents` can contain no resources, this is useful to mark the slot used even when an error
    // happens, to match what client-side validation would do.
    void Update(BindingIndex slot, const BindGroupEntryContents& contents);
    void Remove(BindingIndex slot);
    void OnPinned(BindingIndex slot, TextureBase* texture);
    void OnUnpinned(BindingIndex slot, TextureBase* texture);

    // Returns the various type ids that need to be updated in the metadata buffer before the next
    // use of the binding array.
    struct MetadataUpdate {
        uint32_t offset;
        uint32_t data;
    };
    struct ResourceUpdate {
        BindingIndex slot;
        TextureViewBase* textureView = nullptr;
    };
    struct BindingUpdates {
        std::vector<MetadataUpdate> metadataUpdates;
        std::vector<ResourceUpdate> resourceUpdates;
    };
    BindingUpdates AcquireDirtyBindingUpdates();

  private:
    bool mDestroyed = false;
    wgpu::DynamicBindingKind mKind;
    BindingIndex mAPISize;
    raw_ptr<DeviceBase> mDevice;

    ityp::vector<BindingIndex, Ref<TextureViewBase>> mBindings;
    // Buffer that contains a WGSL metadata struct of the following shape:
    //
    // struct Metadata {
    //     arrayLength: u32,  //  Doesn't include the default bindings
    //     bindings: array<u32>,  // One entry per binding, including default bindings
    // }
    Ref<BufferBase> mMetadataBuffer;

    struct BindingState {
        // Matches the value of the Tint enum for type IDs but kept as u32 to keep usage of Tint
        // headers local.
        tint::ResourceType typeId = tint::ResourceType(0);
        ExecutionSerial availableAfter = kBeginningOfGPUTime;
        bool dirty = false;
        bool resourceDirty = false;  // resourceDirty implies dirty.
        bool pinned = false;
    };
    ityp::vector<BindingIndex, BindingState> mBindingState;

    // The list of bindings that need to be updated before the next use of the dynamic array.
    std::vector<BindingIndex> mDirtyBindings;

    // Helper method that does the bulk of the shared work between Update and RemoveBinding.
    void SetEntry(BindingIndex slot, const BindGroupEntryContents& contents);

    void MarkStateDirty(BindingIndex slot);
    void SetMetadata(BindingIndex slot, tint::ResourceType typeId, bool pinned);
};

class DynamicArrayDefaultBindings : public NonMovable {
  public:
    ResultOrError<ityp::span<BindingIndex, Ref<TextureViewBase>>> GetOrCreateSampledTextureDefaults(
        DeviceBase* device);

  private:
    ityp::vector<BindingIndex, Ref<TextureViewBase>> mSampledTextureDefaults;
};

}  // namespace dawn::native

#endif  // SRC_DAWN_NATIVE_DYNAMICARRAYSTATE_H_
