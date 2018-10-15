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

#ifndef DAWNNATIVE_BINDGROUP_H_
#define DAWNNATIVE_BINDGROUP_H_

#include "common/Constants.h"
#include "dawn_native/BindGroupLayout.h"
#include "dawn_native/Builder.h"
#include "dawn_native/Forward.h"
#include "dawn_native/ObjectBase.h"

#include "dawn_native/dawn_platform.h"

#include <array>
#include <bitset>

namespace dawn_native {

    class BindGroupBase : public ObjectBase {
      public:
        BindGroupBase(BindGroupBuilder* builder);

        const BindGroupLayoutBase* GetLayout() const;
        BufferViewBase* GetBindingAsBufferView(size_t binding);
        SamplerBase* GetBindingAsSampler(size_t binding);
        TextureViewBase* GetBindingAsTextureView(size_t binding);

      private:
        Ref<BindGroupLayoutBase> mLayout;
        std::array<Ref<ObjectBase>, kMaxBindingsPerGroup> mBindings;
    };

    class BindGroupBuilder : public Builder<BindGroupBase> {
      public:
        BindGroupBuilder(DeviceBase* device);

        // Dawn API
        void SetLayout(BindGroupLayoutBase* layout);

        void SetBufferViews(uint32_t start, uint32_t count, BufferViewBase* const* bufferViews);
        void SetSamplers(uint32_t start, uint32_t count, SamplerBase* const* samplers);
        void SetTextureViews(uint32_t start, uint32_t count, TextureViewBase* const* textureViews);

      private:
        friend class BindGroupBase;

        BindGroupBase* GetResultImpl() override;
        void SetBindingsBase(uint32_t start, uint32_t count, ObjectBase* const* objects);
        bool SetBindingsValidationBase(uint32_t start, uint32_t count);

        std::bitset<kMaxBindingsPerGroup> mSetMask;
        int mPropertiesSet = 0;

        Ref<BindGroupLayoutBase> mLayout;
        std::array<Ref<ObjectBase>, kMaxBindingsPerGroup> mBindings;
    };

}  // namespace dawn_native

#endif  // DAWNNATIVE_BINDGROUP_H_
