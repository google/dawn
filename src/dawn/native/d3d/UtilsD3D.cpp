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

#include "dawn/native/d3d/UtilsD3D.h"

#include <utility>

namespace dawn::native::d3d {

ResultOrError<std::wstring> ConvertStringToWstring(std::string_view s) {
    size_t len = s.length();
    if (len == 0) {
        return std::wstring();
    }
    int numChars = MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, s.data(), len, nullptr, 0);
    if (numChars == 0) {
        return DAWN_INTERNAL_ERROR("Failed to convert string to wide string");
    }
    std::wstring result;
    result.resize(numChars);
    int numConvertedChars =
        MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, s.data(), len, &result[0], numChars);
    if (numConvertedChars != numChars) {
        return DAWN_INTERNAL_ERROR("Failed to convert string to wide string");
    }
    return std::move(result);
}

bool IsTypeless(DXGI_FORMAT format) {
    // List generated from <dxgiformat.h>
    switch (format) {
        case DXGI_FORMAT_R32G32B32A32_TYPELESS:
        case DXGI_FORMAT_R32G32B32_TYPELESS:
        case DXGI_FORMAT_R16G16B16A16_TYPELESS:
        case DXGI_FORMAT_R32G32_TYPELESS:
        case DXGI_FORMAT_R32G8X24_TYPELESS:
        case DXGI_FORMAT_R32_FLOAT_X8X24_TYPELESS:
        case DXGI_FORMAT_R10G10B10A2_TYPELESS:
        case DXGI_FORMAT_R8G8B8A8_TYPELESS:
        case DXGI_FORMAT_R16G16_TYPELESS:
        case DXGI_FORMAT_R32_TYPELESS:
        case DXGI_FORMAT_R24G8_TYPELESS:
        case DXGI_FORMAT_R24_UNORM_X8_TYPELESS:
        case DXGI_FORMAT_R8G8_TYPELESS:
        case DXGI_FORMAT_R16_TYPELESS:
        case DXGI_FORMAT_R8_TYPELESS:
        case DXGI_FORMAT_BC1_TYPELESS:
        case DXGI_FORMAT_BC2_TYPELESS:
        case DXGI_FORMAT_BC3_TYPELESS:
        case DXGI_FORMAT_BC4_TYPELESS:
        case DXGI_FORMAT_BC5_TYPELESS:
        case DXGI_FORMAT_B8G8R8A8_TYPELESS:
        case DXGI_FORMAT_B8G8R8X8_TYPELESS:
        case DXGI_FORMAT_BC6H_TYPELESS:
        case DXGI_FORMAT_BC7_TYPELESS:
            return true;
        default:
            return false;
    }
}

uint64_t MakeDXCVersion(uint64_t majorVersion, uint64_t minorVersion) {
    return (majorVersion << 32) + minorVersion;
}

}  // namespace dawn::native::d3d
