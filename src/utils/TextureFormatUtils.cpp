// Copyright 2020 The Dawn Authors
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

#include "TextureFormatUtils.h"

namespace utils {
    const char* GetColorTextureComponentTypePrefix(wgpu::TextureFormat textureFormat) {
        switch (textureFormat) {
            case wgpu::TextureFormat::R8Unorm:
            case wgpu::TextureFormat::R8Snorm:
            case wgpu::TextureFormat::R16Float:
            case wgpu::TextureFormat::RG8Unorm:
            case wgpu::TextureFormat::RG8Snorm:
            case wgpu::TextureFormat::R32Float:
            case wgpu::TextureFormat::RG16Float:
            case wgpu::TextureFormat::RGBA8Unorm:
            case wgpu::TextureFormat::RGBA8Snorm:
            case wgpu::TextureFormat::RGB10A2Unorm:
            case wgpu::TextureFormat::RG11B10Float:
            case wgpu::TextureFormat::RG32Float:
            case wgpu::TextureFormat::RGBA16Float:
            case wgpu::TextureFormat::RGBA32Float:
            case wgpu::TextureFormat::BGRA8Unorm:
            case wgpu::TextureFormat::BGRA8UnormSrgb:
            case wgpu::TextureFormat::RGBA8UnormSrgb:
                return "";

            case wgpu::TextureFormat::R8Uint:
            case wgpu::TextureFormat::R16Uint:
            case wgpu::TextureFormat::RG8Uint:
            case wgpu::TextureFormat::R32Uint:
            case wgpu::TextureFormat::RG16Uint:
            case wgpu::TextureFormat::RGBA8Uint:
            case wgpu::TextureFormat::RG32Uint:
            case wgpu::TextureFormat::RGBA16Uint:
            case wgpu::TextureFormat::RGBA32Uint:
                return "u";

            case wgpu::TextureFormat::R8Sint:
            case wgpu::TextureFormat::R16Sint:
            case wgpu::TextureFormat::RG8Sint:
            case wgpu::TextureFormat::R32Sint:
            case wgpu::TextureFormat::RG16Sint:
            case wgpu::TextureFormat::RGBA8Sint:
            case wgpu::TextureFormat::RG32Sint:
            case wgpu::TextureFormat::RGBA16Sint:
            case wgpu::TextureFormat::RGBA32Sint:
                return "i";
            default:
                UNREACHABLE();
                return "";
        }
    }

    bool TextureFormatSupportsStorageTexture(wgpu::TextureFormat format) {
        switch (format) {
            case wgpu::TextureFormat::R32Uint:
            case wgpu::TextureFormat::R32Sint:
            case wgpu::TextureFormat::R32Float:
            case wgpu::TextureFormat::RGBA8Unorm:
            case wgpu::TextureFormat::RGBA8Snorm:
            case wgpu::TextureFormat::RGBA8Uint:
            case wgpu::TextureFormat::RGBA8Sint:
            case wgpu::TextureFormat::RG32Uint:
            case wgpu::TextureFormat::RG32Sint:
            case wgpu::TextureFormat::RG32Float:
            case wgpu::TextureFormat::RGBA16Uint:
            case wgpu::TextureFormat::RGBA16Sint:
            case wgpu::TextureFormat::RGBA16Float:
            case wgpu::TextureFormat::RGBA32Uint:
            case wgpu::TextureFormat::RGBA32Sint:
            case wgpu::TextureFormat::RGBA32Float:
                return true;
            default:
                return false;
        }
    }
}  // namespace utils
