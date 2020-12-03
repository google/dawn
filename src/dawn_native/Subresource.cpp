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
        Aspect aspectMask = TryConvertAspect(format, aspect);
        ASSERT(aspectMask != Aspect::None);
        return aspectMask;
    }

    Aspect TryConvertAspect(const Format& format, wgpu::TextureAspect aspect) {
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
                return 0;
            case Aspect::Depth:
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
        if (aspects == Aspect::Color || aspects == Aspect::Depth) {
            return 1;
        } else {
            ASSERT(aspects == (Aspect::Depth | Aspect::Stencil));
            return 2;
        }
    }

    // static
    SubresourceRange SubresourceRange::SingleMipAndLayer(uint32_t baseMipLevel,
                                                         uint32_t baseArrayLayer,
                                                         Aspect aspects) {
        return {baseMipLevel, 1, baseArrayLayer, 1, aspects};
    }

}  // namespace dawn_native
