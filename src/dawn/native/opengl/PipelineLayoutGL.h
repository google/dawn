// Copyright 2017 The Dawn Authors
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

#ifndef SRC_DAWN_NATIVE_OPENGL_PIPELINELAYOUTGL_H_
#define SRC_DAWN_NATIVE_OPENGL_PIPELINELAYOUTGL_H_

#include "dawn/native/PipelineLayout.h"

#include "dawn/common/ityp_array.h"
#include "dawn/common/ityp_vector.h"
#include "dawn/native/BindingInfo.h"
#include "dawn/native/opengl/opengl_platform.h"

namespace dawn::native::opengl {

class Device;

class PipelineLayout final : public PipelineLayoutBase {
  public:
    PipelineLayout(Device* device, const PipelineLayoutDescriptor* descriptor);

    // GL backend does not support separate bind group index
    // BindingIndexInfo is a map from BindingPoint(group, binding) to a flattened GLuint binding
    // number.
    using BindingIndexInfo =
        ityp::array<BindGroupIndex, ityp::vector<BindingIndex, GLuint>, kMaxBindGroups>;
    const BindingIndexInfo& GetBindingIndexInfo() const;

    GLuint GetTextureUnitsUsed() const;
    size_t GetNumSamplers() const;
    size_t GetNumSampledTextures() const;

    GLuint GetInternalUniformBinding() const;

  private:
    ~PipelineLayout() override = default;
    BindingIndexInfo mIndexInfo;
    size_t mNumSamplers;
    size_t mNumSampledTextures;

    GLuint mInternalUniformBinding;
};

}  // namespace dawn::native::opengl

#endif  // SRC_DAWN_NATIVE_OPENGL_PIPELINELAYOUTGL_H_
