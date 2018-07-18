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

#ifndef BACKEND_BINDGROUP_H_
#define BACKEND_BINDGROUP_H_

#include "backend/BindGroupLayout.h"
#include "backend/Builder.h"
#include "backend/Forward.h"
#include "backend/RefCounted.h"
#include "common/Constants.h"

#include "dawn/dawncpp.h"

#include <array>
#include <bitset>
#include <type_traits>

namespace backend {

    class BindGroupBase : public RefCounted {
      public:
        BindGroupBase(BindGroupBuilder* builder);

        const BindGroupLayoutBase* GetLayout() const;
        dawn::BindGroupUsage GetUsage() const;
        BufferViewBase* GetBindingAsBufferView(size_t binding);
        SamplerBase* GetBindingAsSampler(size_t binding);
        TextureViewBase* GetBindingAsTextureView(size_t binding);

        DeviceBase* GetDevice() const;

      private:
        Ref<BindGroupLayoutBase> mLayout;
        dawn::BindGroupUsage mUsage;
        std::array<Ref<RefCounted>, kMaxBindingsPerGroup> mBindings;
    };

    class BindGroupBuilder : public Builder<BindGroupBase> {
      public:
        BindGroupBuilder(DeviceBase* device);

        // NXT API
        void SetLayout(BindGroupLayoutBase* layout);
        void SetUsage(dawn::BindGroupUsage usage);

        template <typename T>
        void SetBufferViews(uint32_t start, uint32_t count, T* const* bufferViews) {
            static_assert(std::is_base_of<BufferViewBase, T>::value, "");
            SetBufferViews(start, count, reinterpret_cast<BufferViewBase* const*>(bufferViews));
        }
        void SetBufferViews(uint32_t start, uint32_t count, BufferViewBase* const* bufferViews);

        template <typename T>
        void SetSamplers(uint32_t start, uint32_t count, T* const* samplers) {
            static_assert(std::is_base_of<SamplerBase, T>::value, "");
            SetSamplers(start, count, reinterpret_cast<SamplerBase* const*>(samplers));
        }
        void SetSamplers(uint32_t start, uint32_t count, SamplerBase* const* samplers);

        template <typename T>
        void SetTextureViews(uint32_t start, uint32_t count, T* const* textureViews) {
            static_assert(std::is_base_of<TextureViewBase, T>::value, "");
            SetTextureViews(start, count, reinterpret_cast<TextureViewBase* const*>(textureViews));
        }
        void SetTextureViews(uint32_t start, uint32_t count, TextureViewBase* const* textureViews);

      private:
        friend class BindGroupBase;

        BindGroupBase* GetResultImpl() override;
        void SetBindingsBase(uint32_t start, uint32_t count, RefCounted* const* objects);
        bool SetBindingsValidationBase(uint32_t start, uint32_t count);

        std::bitset<kMaxBindingsPerGroup> mSetMask;
        int mPropertiesSet = 0;

        Ref<BindGroupLayoutBase> mLayout;
        dawn::BindGroupUsage mUsage;
        std::array<Ref<RefCounted>, kMaxBindingsPerGroup> mBindings;
    };

}  // namespace backend

#endif  // BACKEND_BINDGROUP_H_
