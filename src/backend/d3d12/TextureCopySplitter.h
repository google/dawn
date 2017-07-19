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

#ifndef BACKEND_D3D12_TEXTURECOPYSPLITTER_H_
#define BACKEND_D3D12_TEXTURECOPYSPLITTER_H_

#include "nxt/nxtcpp.h"

#include <array>

namespace backend {
namespace d3d12 {


    struct TextureCopySplit {

        static constexpr unsigned int kMaxTextureCopyRegions = 2;
        
        struct Extent {
            uint32_t width = 0;
            uint32_t height = 0;
            uint32_t depth = 0;
        };

        struct Origin {
            uint32_t x = 0;
            uint32_t y = 0;
            uint32_t z = 0;
        };

        struct CopyInfo {
            Origin textureOffset;
            Origin bufferOffset;
            Extent bufferSize;

            Extent copySize;
        };

        uint32_t offset = 0;
        uint32_t count = 0;
        std::array<CopyInfo, kMaxTextureCopyRegions> copies;
    };

    TextureCopySplit ComputeTextureCopySplit(uint32_t x, uint32_t y, uint32_t z, uint32_t width, uint32_t height, uint32_t depth, uint32_t texelSize, uint32_t offset, uint32_t rowPitch);

}
}

#endif // BACKEND_D3D12_TEXTURECOPYSPLITTER_H_
