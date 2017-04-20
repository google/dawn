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

#ifndef BACKEND_OPENGL_SAMPLERGL_H_
#define BACKEND_OPENGL_SAMPLERGL_H_

#include "common/Sampler.h"

#include "glad/glad.h"

namespace backend {
namespace opengl {

    class Device;

    class Sampler : public SamplerBase {
        public:
            Sampler(Device* device, SamplerBuilder* builder);

            GLuint GetHandle() const;

        private:
            Device* device;
            GLuint handle;
    };

}
}

#endif // BACKEND_OPENGL_SAMPLERGL_H_
