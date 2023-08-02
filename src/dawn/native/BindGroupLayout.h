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

#ifndef SRC_DAWN_NATIVE_BINDGROUPLAYOUT_H_
#define SRC_DAWN_NATIVE_BINDGROUPLAYOUT_H_

#include <string>

#include "dawn/native/BindGroupLayoutInternal.h"
#include "dawn/native/IntegerTypes.h"
#include "dawn/native/ObjectBase.h"

namespace dawn::native {

// Wrapper passthrough frontend object that is essentially just a Ref to a backing
// BindGroupLayoutInternalBase and a pipeline compatibility token.
// TODO(lokokung) Could maybe make this a final class if we update the mock objects.
class BindGroupLayoutBase : public ApiObjectBase {
  public:
    BindGroupLayoutBase(DeviceBase* device,
                        const char* label,
                        Ref<BindGroupLayoutInternalBase> internal,
                        PipelineCompatibilityToken pipelineCompatibilityToken);

    static BindGroupLayoutBase* MakeError(DeviceBase* device, const char* label = nullptr);

    ObjectType GetType() const override;

    // Proxy functions that just call their respective counterparts in the internal layout.
    const BindingInfo& GetBindingInfo(BindingIndex bindingIndex) const {
        return mInternalLayout->GetBindingInfo(bindingIndex);
    }
    const BindGroupLayoutInternalBase::BindingMap& GetBindingMap() const {
        return mInternalLayout->GetBindingMap();
    }
    bool HasBinding(BindingNumber bindingNumber) const {
        return mInternalLayout->HasBinding(bindingNumber);
    }
    BindingIndex GetBindingIndex(BindingNumber bindingNumber) const {
        return mInternalLayout->GetBindingIndex(bindingNumber);
    }
    BindingIndex GetBindingCount() const { return mInternalLayout->GetBindingCount(); }
    BindingIndex GetBufferCount() const { return mInternalLayout->GetBufferCount(); }
    BindingIndex GetDynamicBufferCount() const { return mInternalLayout->GetDynamicBufferCount(); }
    uint32_t GetUnverifiedBufferCount() const {
        return mInternalLayout->GetUnverifiedBufferCount();
    }
    const BindingCounts& GetBindingCountInfo() const {
        return mInternalLayout->GetBindingCountInfo();
    }
    uint32_t GetExternalTextureBindingCount() const {
        return mInternalLayout->GetExternalTextureBindingCount();
    }
    const ExternalTextureBindingExpansionMap& GetExternalTextureBindingExpansionMap() const {
        return mInternalLayout->GetExternalTextureBindingExpansionMap();
    }
    uint32_t GetUnexpandedBindingCount() const {
        return mInternalLayout->GetUnexpandedBindingCount();
    }
    size_t GetBindingDataSize() const { return mInternalLayout->GetBindingDataSize(); }
    static constexpr size_t GetBindingDataAlignment() {
        return BindGroupLayoutInternalBase::GetBindingDataAlignment();
    }
    BindGroupLayoutInternalBase::BindingDataPointers ComputeBindingDataPointers(
        void* dataStart) const {
        return mInternalLayout->ComputeBindingDataPointers(dataStart);
    }
    bool IsStorageBufferBinding(BindingIndex bindingIndex) const {
        return mInternalLayout->IsStorageBufferBinding(bindingIndex);
    }
    std::string EntriesToString() const { return mInternalLayout->EntriesToString(); }

    // Non-proxy functions that are specific to the realized frontend object.
    BindGroupLayoutInternalBase* GetInternalBindGroupLayout() const;
    bool IsLayoutEqual(const BindGroupLayoutBase* other,
                       bool excludePipelineCompatibiltyToken = false) const;
    PipelineCompatibilityToken GetPipelineCompatibilityToken() const {
        return mPipelineCompatibilityToken;
    }

  protected:
    void DestroyImpl() override;

  private:
    BindGroupLayoutBase(DeviceBase* device, ObjectBase::ErrorTag tag, const char* label);

    const Ref<BindGroupLayoutInternalBase> mInternalLayout;

    // Non-0 if this BindGroupLayout was created as part of a default PipelineLayout.
    const PipelineCompatibilityToken mPipelineCompatibilityToken = PipelineCompatibilityToken(0);
};

}  // namespace dawn::native

#endif  // SRC_DAWN_NATIVE_BINDGROUPLAYOUT_H_
