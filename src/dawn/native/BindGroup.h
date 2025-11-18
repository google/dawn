// Copyright 2017 The Dawn & Tint Authors
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

#ifndef SRC_DAWN_NATIVE_BINDGROUP_H_
#define SRC_DAWN_NATIVE_BINDGROUP_H_

#include <array>
#include <vector>

#include "dawn/common/Constants.h"
#include "dawn/common/Math.h"
#include "dawn/common/ityp_span.h"
#include "dawn/native/BindGroupLayout.h"
#include "dawn/native/ChainUtils.h"
#include "dawn/native/Error.h"
#include "dawn/native/Forward.h"
#include "dawn/native/ObjectBase.h"
#include "dawn/native/UsageValidationMode.h"

#include "dawn/native/dawn_platform.h"

namespace dawn::native {

class DeviceBase;
class DynamicArrayState;

ResultOrError<UnpackedPtr<BindGroupDescriptor>> ValidateBindGroupDescriptor(
    DeviceBase* device,
    const BindGroupDescriptor* descriptor,
    UsageValidationMode mode);

struct BufferBinding {
    BufferBase* buffer;
    uint64_t offset;
    uint64_t size;
};

class BindGroupBase : public ApiObjectBase {
  public:
    static Ref<BindGroupBase> MakeError(DeviceBase* device, StringView label);

    MaybeError Initialize(const UnpackedPtr<BindGroupDescriptor>& descriptor);

    ObjectType GetType() const override;

    // Dawn API
    void APIDestroy();
    wgpu::Status APIUpdate(const BindGroupEntry* entry);
    uint32_t APIInsertBinding(const BindGroupEntryContents* contents);
    void APIRemoveBinding(uint32_t binding);

    BindGroupLayoutBase* GetFrontendLayout();
    const BindGroupLayoutBase* GetFrontendLayout() const;
    BindGroupLayoutInternalBase* GetLayout();
    const BindGroupLayoutInternalBase* GetLayout() const;

    // Getters for static bindings part.
    BufferBase* GetBindingAsBuffer(BindingIndex bindingIndex);
    SamplerBase* GetBindingAsSampler(BindingIndex bindingIndex) const;
    TextureViewBase* GetBindingAsTextureView(BindingIndex bindingIndex);
    BufferBinding GetBindingAsBufferBinding(BindingIndex bindingIndex);
    TexelBufferViewBase* GetBindingAsTexelBufferView(BindingIndex bindingIndex);
    const ityp::span<uint32_t, uint64_t>& GetUnverifiedBufferSizes() const;
    const std::vector<Ref<ExternalTextureBase>>& GetBoundExternalTextures() const;

    void ForEachUnverifiedBufferBindingIndex(std::function<void(BindingIndex, uint32_t)> fn) const;

    // Getters and operations on the dynamic array part for code that doesn't need to directly
    // modify the state.
    bool HasDynamicArray() const;
    ityp::span<BindingIndex, const Ref<TextureViewBase>> GetDynamicArrayBindings() const;
    MaybeError ValidateCanUseOnQueueNow() const;
    DynamicArrayState* GetDynamicArray() const;

  protected:
    // To save memory, the size of a bind group is dynamically determined and the bind group is
    // placement-allocated into memory big enough to hold the bind group with its
    // dynamically-sized bindings after it. The pointer of the memory of the beginning of the
    // binding data should be passed as |bindingDataStart|.
    BindGroupBase(DeviceBase* device,
                  const UnpackedPtr<BindGroupDescriptor>& descriptor,
                  void* bindingDataStart);

    // Helper to instantiate BindGroupBase. We pass in |derived| because BindGroupBase may not
    // be first in the allocation. The binding data is stored after the Derived class.
    template <typename Derived>
    BindGroupBase(Derived* derived,
                  DeviceBase* device,
                  const UnpackedPtr<BindGroupDescriptor>& descriptor)
        : BindGroupBase(
              device,
              descriptor,
              AlignPtr(
                  reinterpret_cast<char*>(derived) + sizeof(Derived),
                  descriptor->layout->GetInternalBindGroupLayout()->GetBindingDataAlignment())) {
        static_assert(std::is_base_of<BindGroupBase, Derived>::value);
    }

    virtual MaybeError InitializeImpl() = 0;

    void DestroyImpl() override;

    ~BindGroupBase() override;

  private:
    BindGroupBase(DeviceBase* device, ObjectBase::ErrorTag tag, StringView label);

    MaybeError ValidateDestroy() const;

    Ref<BindGroupLayoutBase> mLayout;
    BindGroupLayoutInternalBase::BindingDataPointers mBindingData;
    std::vector<Ref<ExternalTextureBase>> mBoundExternalTextures;

    // The dynamic array is separate so as to not bloat the size and destructor of bind groups
    // without them. Note that this is the only persistent owning Ref. DynamicArray is a RefCounted
    // only so WeakRef to it can be created.
    Ref<DynamicArrayState> mDynamicArray;
};

}  // namespace dawn::native

#endif  // SRC_DAWN_NATIVE_BINDGROUP_H_
