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

#include "dawn/common/Assert.h"
#include "dawn/native/Format.h"

namespace dawn::native {

Aspect ConvertSingleAspect(const Format& format, wgpu::TextureAspect aspect) {
    Aspect aspectMask = ConvertAspect(format, aspect);
    ASSERT(HasOneBit(aspectMask));
    return aspectMask;
}

Aspect ConvertAspect(const Format& format, wgpu::TextureAspect aspect) {
    Aspect aspectMask = SelectFormatAspects(format, aspect);
    ASSERT(aspectMask != Aspect::None);
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
    UNREACHABLE();
}

uint8_t GetAspectIndex(Aspect aspect) {
    ASSERT(HasOneBit(aspect));
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
            UNREACHABLE();
    }
}

uint8_t GetAspectCount(Aspect aspects) {
    // TODO(crbug.com/dawn/829): This should use popcount once Dawn has such a function.
    // Note that we can't do a switch because compilers complain that Depth | Stencil is not
    // a valid enum value.
    if (aspects == Aspect::Color || aspects == Aspect::Depth ||
        aspects == Aspect::CombinedDepthStencil) {
        return 1;
    } else if (aspects == (Aspect::Plane0 | Aspect::Plane1)) {
        return 2;
    } else if (aspects == Aspect::Stencil) {
        // Fake a the existence of a depth aspect so that the stencil data stays at index 1.
        ASSERT(GetAspectIndex(Aspect::Stencil) == 1);
        return 2;
    } else {
        ASSERT(aspects == (Aspect::Depth | Aspect::Stencil));
        return 2;
    }
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
    ASSERT(HasOneBit(aspect));
    return {aspect, {baseArrayLayer, 1}, {baseMipLevel, 1}};
}

// static
SubresourceRange SubresourceRange::MakeFull(Aspect aspects,
                                            uint32_t layerCount,
                                            uint32_t levelCount) {
    return {aspects, {0, layerCount}, {0, levelCount}};
}

}  // namespace dawn::native
