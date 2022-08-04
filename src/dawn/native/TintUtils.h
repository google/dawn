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

#ifndef SRC_DAWN_NATIVE_TINTUTILS_H_
#define SRC_DAWN_NATIVE_TINTUTILS_H_

#include <functional>

#include "dawn/common/NonCopyable.h"
#include "dawn/native/IntegerTypes.h"

#include "tint/tint.h"

namespace dawn::native {

class DeviceBase;
class PipelineLayoutBase;
class RenderPipelineBase;

// Indicates that for the lifetime of this object tint internal compiler errors should be
// reported to the given device.
class ScopedTintICEHandler : public NonCopyable {
  public:
    explicit ScopedTintICEHandler(DeviceBase* device);
    ~ScopedTintICEHandler();

  private:
    ScopedTintICEHandler(ScopedTintICEHandler&&) = delete;
};

tint::transform::MultiplanarExternalTexture::BindingsMap BuildExternalTextureTransformBindings(
    const PipelineLayoutBase* layout);

tint::transform::VertexPulling::Config BuildVertexPullingTransformConfig(
    const RenderPipelineBase& renderPipeline,
    const std::string_view& entryPoint,
    BindGroupIndex pullingBufferBindingSet);

}  // namespace dawn::native

// std::less operator for std::map containing BindingPoint
template <>
struct std::less<tint::sem::BindingPoint> {
    bool operator()(const tint::sem::BindingPoint& a, const tint::sem::BindingPoint& b) const;
};

#endif  // SRC_DAWN_NATIVE_TINTUTILS_H_
