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

#ifndef SRC_DAWN_NATIVE_BINDGROUP_H_
#define SRC_DAWN_NATIVE_BINDGROUP_H_

#include <array>
#include <vector>

#include "dawn/common/Constants.h"
#include "dawn/common/Math.h"
#include "dawn/native/BindGroupLayout.h"
#include "dawn/native/Error.h"
#include "dawn/native/Forward.h"
#include "dawn/native/ObjectBase.h"
#include "dawn/native/UsageValidationMode.h"

#include "dawn/native/dawn_platform.h"

namespace dawn::native {

class DeviceBase;

MaybeError ValidateBindGroupDescriptor(DeviceBase* device,
                                       const BindGroupDescriptor* descriptor,
                                       UsageValidationMode mode);

struct BufferBinding {
    BufferBase* buffer;
    uint64_t offset;
    uint64_t size;
};

class BindGroupBase : public ApiObjectBase {
  public:
    static BindGroupBase* MakeError(DeviceBase* device);

    ObjectType GetType() const override;

    BindGroupLayoutBase* GetLayout();
    const BindGroupLayoutBase* GetLayout() const;
    BufferBinding GetBindingAsBufferBinding(BindingIndex bindingIndex);
    SamplerBase* GetBindingAsSampler(BindingIndex bindingIndex) const;
    TextureViewBase* GetBindingAsTextureView(BindingIndex bindingIndex);
    const ityp::span<uint32_t, uint64_t>& GetUnverifiedBufferSizes() const;
    const std::vector<Ref<ExternalTextureBase>>& GetBoundExternalTextures() const;

  protected:
    // To save memory, the size of a bind group is dynamically determined and the bind group is
    // placement-allocated into memory big enough to hold the bind group with its
    // dynamically-sized bindings after it. The pointer of the memory of the beginning of the
    // binding data should be passed as |bindingDataStart|.
    BindGroupBase(DeviceBase* device,
                  const BindGroupDescriptor* descriptor,
                  void* bindingDataStart);

    // Helper to instantiate BindGroupBase. We pass in |derived| because BindGroupBase may not
    // be first in the allocation. The binding data is stored after the Derived class.
    template <typename Derived>
    BindGroupBase(Derived* derived, DeviceBase* device, const BindGroupDescriptor* descriptor)
        : BindGroupBase(device,
                        descriptor,
                        AlignPtr(reinterpret_cast<char*>(derived) + sizeof(Derived),
                                 descriptor->layout->GetBindingDataAlignment())) {
        static_assert(std::is_base_of<BindGroupBase, Derived>::value);
    }

    void DestroyImpl() override;

    ~BindGroupBase() override;

  private:
    BindGroupBase(DeviceBase* device, ObjectBase::ErrorTag tag);
    void DeleteThis() override;

    Ref<BindGroupLayoutBase> mLayout;
    BindGroupLayoutBase::BindingDataPointers mBindingData;

    // TODO(dawn:1293): Store external textures in
    // BindGroupLayoutBase::BindingDataPointers::bindings
    std::vector<Ref<ExternalTextureBase>> mBoundExternalTextures;
};

}  // namespace dawn::native

#endif  // SRC_DAWN_NATIVE_BINDGROUP_H_
