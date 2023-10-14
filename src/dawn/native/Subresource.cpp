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

#include "dawn/native/Subresource.h"

#include "absl/numeric/bits.h"
#include "dawn/common/Assert.h"
#include "dawn/native/Format.h"

namespace dawn::native {

Aspect ConvertSingleAspect(const Format& format, wgpu::TextureAspect aspect) {
    Aspect aspectMask = ConvertAspect(format, aspect);
    DAWN_ASSERT(HasOneBit(aspectMask));
    return aspectMask;
}

Aspect ConvertAspect(const Format& format, wgpu::TextureAspect aspect) {
    Aspect aspectMask = SelectFormatAspects(format, aspect);
    DAWN_ASSERT(aspectMask != Aspect::None);
    return aspectMask;
}

Aspect ConvertViewAspect(const Format& format, wgpu::TextureAspect aspect) {
    // Color view |format| must be treated as the same plane |aspect|.
    if (format.aspects == Aspect::Color) {
        switch (aspect) {
            case wgpu::TextureAspect::Plane0Only:
                return Aspect::Plane0;
            case wgpu::TextureAspect::Plane1Only:
                return Aspect::Plane1;
            default:
                break;
        }
    }
    return ConvertAspect(format, aspect);
}

Aspect GetPlaneAspect(const Format& format, uint32_t planeIndex) {
    wgpu::TextureAspect textureAspect;
    switch (planeIndex) {
        case 0:
            textureAspect = wgpu::TextureAspect::Plane0Only;
            break;
        case 1:
            textureAspect = wgpu::TextureAspect::Plane1Only;
            break;
        default:
            DAWN_UNREACHABLE();
    }

    return ConvertAspect(format, textureAspect);
}

Aspect SelectFormatAspects(const Format& format, wgpu::TextureAspect aspect) {
    switch (aspect) {
        case wgpu::TextureAspect::All:
            return format.aspects;
        case wgpu::TextureAspect::DepthOnly:
            return format.aspects & Aspect::Depth;
        case wgpu::TextureAspect::StencilOnly:
            return format.aspects & Aspect::Stencil;
        case wgpu::TextureAspect::Plane0Only:
            return format.aspects & Aspect::Plane0;
        case wgpu::TextureAspect::Plane1Only:
            return format.aspects & Aspect::Plane1;
    }
    DAWN_UNREACHABLE();
}

uint8_t GetAspectIndex(Aspect aspect) {
    DAWN_ASSERT(HasOneBit(aspect));
    switch (aspect) {
        case Aspect::Color:
        case Aspect::Depth:
        case Aspect::Plane0:
        case Aspect::CombinedDepthStencil:
            return 0;
        case Aspect::Plane1:
        case Aspect::Stencil:
            return 1;
        default:
            DAWN_UNREACHABLE();
    }
}

uint8_t GetAspectCount(Aspect aspects) {
    if (aspects == Aspect::Stencil) {
        // Fake a the existence of a depth aspect so that the stencil data stays at index 1.
        DAWN_ASSERT(GetAspectIndex(Aspect::Stencil) == 1);
        return 2;
    }
    return absl::popcount(static_cast<uint8_t>(aspects));
}

SubresourceRange::SubresourceRange(Aspect aspects,
                                   FirstAndCountRange<uint32_t> arrayLayerParam,
                                   FirstAndCountRange<uint32_t> mipLevelParams)
    : aspects(aspects),
      baseArrayLayer(arrayLayerParam.first),
      layerCount(arrayLayerParam.count),
      baseMipLevel(mipLevelParams.first),
      levelCount(mipLevelParams.count) {}

SubresourceRange::SubresourceRange()
    : aspects(Aspect::None), baseArrayLayer(0), layerCount(0), baseMipLevel(0), levelCount(0) {}

// static
SubresourceRange SubresourceRange::SingleMipAndLayer(uint32_t baseMipLevel,
                                                     uint32_t baseArrayLayer,
                                                     Aspect aspects) {
    return {aspects, {baseArrayLayer, 1}, {baseMipLevel, 1}};
}

// static
SubresourceRange SubresourceRange::MakeSingle(Aspect aspect,
                                              uint32_t baseArrayLayer,
                                              uint32_t baseMipLevel) {
    DAWN_ASSERT(HasOneBit(aspect));
    return {aspect, {baseArrayLayer, 1}, {baseMipLevel, 1}};
}

// static
SubresourceRange SubresourceRange::MakeFull(Aspect aspects,
                                            uint32_t layerCount,
                                            uint32_t levelCount) {
    return {aspects, {0, layerCount}, {0, levelCount}};
}

}  // namespace dawn::native
