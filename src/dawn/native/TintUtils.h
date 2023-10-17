// Copyright 2021 The Dawn & Tint Authors
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// 1. Redistributions of source code must retain the above copyright notice, this
//    list of conditions and the following disclaimer.
//
// 2. Redistributions in binary form must reproduce the above copyright notice,
//    this list of conditions and the following disclaimer in the documentation
//    and/or other materials provided with the distribution.
//
// 3. Neither the name of the copyright holder nor the names of its
//    contributors may be used to endorse or promote products derived from
//    this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
// FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
// DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
// CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
// OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#ifndef SRC_DAWN_NATIVE_TINTUTILS_H_
#define SRC_DAWN_NATIVE_TINTUTILS_H_

#include <functional>

#include "dawn/common/NonCopyable.h"
#include "dawn/native/IntegerTypes.h"

#include "tint/tint.h"

namespace dawn::native {

class DeviceBase;
class PipelineLayoutBase;
struct ProgrammableStage;
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

tint::ExternalTextureOptions BuildExternalTextureTransformBindings(
    const PipelineLayoutBase* layout);

tint::ast::transform::VertexPulling::Config BuildVertexPullingTransformConfig(
    const RenderPipelineBase& renderPipeline,
    BindGroupIndex pullingBufferBindingSet);

tint::ast::transform::SubstituteOverride::Config BuildSubstituteOverridesTransformConfig(
    const ProgrammableStage& stage);

}  // namespace dawn::native

namespace tint::sem {

// Defin operator< for std::map containing BindingPoint
bool operator<(const BindingPoint& a, const BindingPoint& b);

}  // namespace tint::sem

#endif  // SRC_DAWN_NATIVE_TINTUTILS_H_
