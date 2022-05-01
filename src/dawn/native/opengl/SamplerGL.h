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

#ifndef SRC_DAWN_NATIVE_OPENGL_SAMPLERGL_H_
#define SRC_DAWN_NATIVE_OPENGL_SAMPLERGL_H_

#include "dawn/native/Sampler.h"

#include "dawn/native/opengl/opengl_platform.h"

namespace dawn::native::opengl {

class Device;

class Sampler final : public SamplerBase {
  public:
    Sampler(Device* device, const SamplerDescriptor* descriptor);

    GLuint GetFilteringHandle() const;
    GLuint GetNonFilteringHandle() const;

  private:
    ~Sampler() override;
    void DestroyImpl() override;

    void SetupGLSampler(GLuint sampler, const SamplerDescriptor* descriptor, bool forceNearest);

    GLuint mFilteringHandle;

    // This is a sampler equivalent to mFilteringHandle except that it uses NEAREST filtering
    // for everything, which is important to preserve texture completeness for u/int textures.
    GLuint mNonFilteringHandle;
};

}  // namespace dawn::native::opengl

#endif  // SRC_DAWN_NATIVE_OPENGL_SAMPLERGL_H_
