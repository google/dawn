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

#ifndef SRC_DAWN_NATIVE_RESOURCETABLE_H_
#define SRC_DAWN_NATIVE_RESOURCETABLE_H_

#include "dawn/native/DynamicArrayState.h"
#include "dawn/native/ObjectBase.h"

#include "dawn/native/dawn_platform.h"

namespace dawn::native {

MaybeError ValidateResourceTableDescriptor(const DeviceBase* device,
                                           const ResourceTableDescriptor* descriptor);

// TODO(https://issues.chromium.org/463925499): Once bindless bindgroup support is removed, make
// this into a new typed integer type instead of an alias to BindingIndex.
using ResourceTableSlot = BindingIndex;

class ResourceTableBase : public ApiObjectBase {
  public:
    static Ref<ResourceTableBase> MakeError(DeviceBase* device,
                                            const ResourceTableDescriptor* descriptor);

    ObjectType GetType() const override;

    // Dawn API
    void APIDestroy();
    wgpu::Status APIUpdate(uint32_t slot, const BindingResource* resource);
    uint32_t APIInsertBinding(const BindingResource* resource);
    wgpu::Status APIRemoveBinding(uint32_t slot);

    MaybeError ValidateCanUseInSubmitNow() const;

  protected:
    ResourceTableBase(DeviceBase* device, const ResourceTableDescriptor* descriptor);

    MaybeError InitializeBase();
    void DestroyImpl() override;

    DynamicArrayState* GetDynamicArrayState();

  private:
    ResourceTableBase(DeviceBase* device,
                      const ResourceTableDescriptor* descriptor,
                      ObjectBase::ErrorTag tag);

    bool IsValidSlot(ResourceTableSlot slot) const;

    // TODO(https://issues.chromium.org/463925499): Inline the functionality of DynamicArrayState in
    // ResourceTable once bindless bindgroup support is removed.
    Ref<DynamicArrayState> mDynamicArray;
};

}  // namespace dawn::native

#endif  // SRC_DAWN_NATIVE_RESOURCETABLE_H_
