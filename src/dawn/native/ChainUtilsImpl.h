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

#ifndef SRC_DAWN_NATIVE_CHAINUTILSIMPL_H_
#define SRC_DAWN_NATIVE_CHAINUTILSIMPL_H_

namespace dawn::native {

struct DawnInstanceDescriptor;

namespace d3d {
struct RequestAdapterOptionsLUID;
}

namespace d3d11 {
struct SharedTextureMemoryD3D11Texture2DDescriptor;
}

namespace opengl {
struct RequestAdapterOptionsGetGLProc;
}

namespace detail {

template <>
constexpr inline wgpu::SType STypeForImpl<DawnInstanceDescriptor> =
    wgpu::SType(WGPUSType_DawnInstanceDescriptor);

template <>
struct AdditionalExtensions<InstanceDescriptor> {
    using List = AdditionalExtensionsList<const DawnInstanceDescriptor*>;
};

template <>
constexpr inline wgpu::SType STypeForImpl<d3d::RequestAdapterOptionsLUID> =
    wgpu::SType(WGPUSType_RequestAdapterOptionsLUID);

template <>
constexpr inline wgpu::SType STypeForImpl<opengl::RequestAdapterOptionsGetGLProc> =
    wgpu::SType(WGPUSType_RequestAdapterOptionsGetGLProc);

template <>
struct AdditionalExtensions<RequestAdapterOptions> {
    using List = AdditionalExtensionsList<const d3d::RequestAdapterOptionsLUID*,
                                          const opengl::RequestAdapterOptionsGetGLProc*>;
};

template <>
constexpr inline wgpu::SType STypeForImpl<d3d11::SharedTextureMemoryD3D11Texture2DDescriptor> =
    wgpu::SType(WGPUSType_SharedTextureMemoryD3D11Texture2DDescriptor);

template <>
struct AdditionalExtensions<SharedTextureMemoryDescriptor> {
    using List =
        AdditionalExtensionsList<const d3d11::SharedTextureMemoryD3D11Texture2DDescriptor*>;
};

}  // namespace detail
}  // namespace dawn::native

#endif  // SRC_DAWN_NATIVE_CHAINUTILSIMPL_H_
