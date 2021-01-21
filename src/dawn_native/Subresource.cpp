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

#include "dawn_native/Subresource.h"

#include "common/Assert.h"
#include "dawn_native/Format.h"

namespace dawn_native {

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

    Aspect SelectFormatAspects(const Format& format, wgpu::TextureAspect aspect) {
        switch (aspect) {
            case wgpu::TextureAspect::All:
                return format.aspects;
            case wgpu::TextureAspect::DepthOnly:
                return format.aspects & Aspect::Depth;
            case wgpu::TextureAspect::StencilOnly:
                return format.aspects & Aspect::Stencil;
        }
    }

    uint8_t GetAspectIndex(Aspect aspect) {
        ASSERT(HasOneBit(aspect));
        switch (aspect) {
            case Aspect::Color:
            case Aspect::Depth:
            case Aspect::CombinedDepthStencil:
                return 0;
            case Aspect::Stencil:
                return 1;
            default:
                UNREACHABLE();
        }
    }

    uint8_t GetAspectCount(Aspect aspects) {
        // TODO(cwallez@chromium.org): This should use popcount once Dawn has such a function.
        // Note that we can't do a switch because compilers complain that Depth | Stencil is not
        // a valid enum value.
        if (aspects == Aspect::Color || aspects == Aspect::Depth ||
            aspects == Aspect::CombinedDepthStencil) {
            return 1;
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
          levelCount(mipLevelParams.count) {
    }

    SubresourceRange::SubresourceRange()
        : aspects(Aspect::None), baseArrayLayer(0), layerCount(0), baseMipLevel(0), levelCount(0) {
    }

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

}  // namespace dawn_native
