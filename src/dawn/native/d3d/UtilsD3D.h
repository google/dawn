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

#ifndef SRC_DAWN_NATIVE_D3D_UTILSD3D_H_
#define SRC_DAWN_NATIVE_D3D_UTILSD3D_H_

#include <string>

#include "dawn/native/Error.h"
#include "dawn/native/d3d/d3d_platform.h"

namespace dawn::native::d3d {

ResultOrError<std::wstring> ConvertStringToWstring(std::string_view s);

bool IsTypeless(DXGI_FORMAT format);

uint64_t MakeDXCVersion(uint64_t majorVersion, uint64_t minorVersion);

DXGI_FORMAT DXGITypelessTextureFormat(wgpu::TextureFormat format);

DXGI_FORMAT DXGITextureFormat(wgpu::TextureFormat format);

DXGI_FORMAT DXGIVertexFormat(wgpu::VertexFormat format);

}  // namespace dawn::native::d3d

#endif  // SRC_DAWN_NATIVE_D3D_UTILSD3D_H_
