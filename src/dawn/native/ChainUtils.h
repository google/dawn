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

#ifndef SRC_DAWN_NATIVE_CHAINUTILS_H_
#define SRC_DAWN_NATIVE_CHAINUTILS_H_

#include "dawn/native/ChainUtils_autogen.h"

namespace dawn::native {

struct DawnInstanceDescriptor;

namespace d3d {
struct RequestAdapterOptionsLUID;
}

namespace opengl {
struct RequestAdapterOptionsGetGLProc;
}

template <>
inline wgpu::SType STypeFor<DawnInstanceDescriptor> = wgpu::SType(WGPUSType_DawnInstanceDescriptor);

template <>
inline wgpu::SType STypeFor<d3d::RequestAdapterOptionsLUID> =
    wgpu::SType(WGPUSType_RequestAdapterOptionsLUID);

template <>
inline wgpu::SType STypeFor<opengl::RequestAdapterOptionsGetGLProc> =
    wgpu::SType(WGPUSType_RequestAdapterOptionsGetGLProc);

}  // namespace dawn::native

#endif  // SRC_DAWN_NATIVE_CHAINUTILS_H_
