// Copyright 2017 The NXT Authors
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

#ifndef BACKEND_COMMON_TOBACKEND_H_
#define BACKEND_COMMON_TOBACKEND_H_

#include "Forward.h"

namespace backend {

    // ToBackendTraits implements the mapping from base type to member type of BackendTraits
    template<typename T, typename BackendTraits>
    struct ToBackendTraits;

    template<typename BackendTraits>
    struct ToBackendTraits<BindGroupBase, BackendTraits> {
        using BackendType = typename BackendTraits::BindGroupType;
    };

    template<typename BackendTraits>
    struct ToBackendTraits<BindGroupLayoutBase, BackendTraits> {
        using BackendType = typename BackendTraits::BindGroupLayoutType;
    };

    template<typename BackendTraits>
    struct ToBackendTraits<BufferBase, BackendTraits> {
        using BackendType = typename BackendTraits::BufferType;
    };

    template<typename BackendTraits>
    struct ToBackendTraits<BufferViewBase, BackendTraits> {
        using BackendType = typename BackendTraits::BufferViewType;
    };

    template<typename BackendTraits>
    struct ToBackendTraits<CommandBufferBase, BackendTraits> {
        using BackendType = typename BackendTraits::CommandBufferType;
    };

    template<typename BackendTraits>
    struct ToBackendTraits<DepthStencilStateBase, BackendTraits> {
        using BackendType = typename BackendTraits::DepthStencilStateType;
    };

    template<typename BackendTraits>
    struct ToBackendTraits<FramebufferBase, BackendTraits> {
        using BackendType = typename BackendTraits::FramebufferType;
    };

    template<typename BackendTraits>
    struct ToBackendTraits<InputStateBase, BackendTraits> {
        using BackendType = typename BackendTraits::InputStateType;
    };

    template<typename BackendTraits>
    struct ToBackendTraits<PipelineBase, BackendTraits> {
        using BackendType = typename BackendTraits::PipelineType;
    };

    template<typename BackendTraits>
    struct ToBackendTraits<PipelineLayoutBase, BackendTraits> {
        using BackendType = typename BackendTraits::PipelineLayoutType;
    };

    template<typename BackendTraits>
    struct ToBackendTraits<QueueBase, BackendTraits> {
        using BackendType = typename BackendTraits::QueueType;
    };

    template<typename BackendTraits>
    struct ToBackendTraits<RenderPassBase, BackendTraits> {
        using BackendType = typename BackendTraits::RenderPassType;
    };

    template<typename BackendTraits>
    struct ToBackendTraits<SamplerBase, BackendTraits> {
        using BackendType = typename BackendTraits::SamplerType;
    };

    template<typename BackendTraits>
    struct ToBackendTraits<ShaderModuleBase, BackendTraits> {
        using BackendType = typename BackendTraits::ShaderModuleType;
    };

    template<typename BackendTraits>
    struct ToBackendTraits<TextureBase, BackendTraits> {
        using BackendType = typename BackendTraits::TextureType;
    };

    template<typename BackendTraits>
    struct ToBackendTraits<TextureViewBase, BackendTraits> {
        using BackendType = typename BackendTraits::TextureViewType;
    };

    // ToBackendBase implements conversion to the given BackendTraits
    // To use it in a backend, use the following:
    //   template<typename T>
    //   auto ToBackend(T&& common) -> decltype(ToBackendBase<MyBackendTraits>(common)) {
    //       return ToBackendBase<MyBackendTraits>(common);
    //   }

    template<typename BackendTraits, typename T>
    Ref<typename ToBackendTraits<T, BackendTraits>::BackendType>& ToBackendBase(Ref<T>& common) {
        return reinterpret_cast<Ref<typename ToBackendTraits<T, BackendTraits>::BackendType>&>(common);
    }

    template<typename BackendTraits, typename T>
    const Ref<typename ToBackendTraits<T, BackendTraits>::BackendType>& ToBackendBase(const Ref<T>& common) {
        return reinterpret_cast<const Ref<typename ToBackendTraits<T, BackendTraits>::BackendType>&>(common);
    }

    template<typename BackendTraits, typename T>
    typename ToBackendTraits<T, BackendTraits>::BackendType* ToBackendBase(T* common) {
        return reinterpret_cast<typename ToBackendTraits<T, BackendTraits>::BackendType*>(common);
    }

    template<typename BackendTraits, typename T>
    const typename ToBackendTraits<T, BackendTraits>::BackendType* ToBackendBase(const T* common) {
        return reinterpret_cast<const typename ToBackendTraits<T, BackendTraits>::BackendType*>(common);
    }

}

#endif // BACKEND_COMMON_TOBACKEND_H_
