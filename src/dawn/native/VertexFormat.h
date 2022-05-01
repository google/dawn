// Copyright 2021 The Dawn Authors
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

#ifndef SRC_DAWN_NATIVE_VERTEXFORMAT_H_
#define SRC_DAWN_NATIVE_VERTEXFORMAT_H_

#include "dawn/native/dawn_platform.h"

namespace dawn::native {

enum class VertexFormatBaseType {
    Float,
    Uint,
    Sint,
};

struct VertexFormatInfo {
    wgpu::VertexFormat format;
    uint32_t byteSize;
    uint32_t componentCount;
    uint32_t componentByteSize;
    VertexFormatBaseType baseType;
};

const VertexFormatInfo& GetVertexFormatInfo(wgpu::VertexFormat format);

}  // namespace dawn::native

#endif  // SRC_DAWN_NATIVE_VERTEXFORMAT_H_
