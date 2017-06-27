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

#ifndef BACKEND_OPENGL_SHADERMODULEGL_H_
#define BACKEND_OPENGL_SHADERMODULEGL_H_

#include "common/ShaderModule.h"

#include "glad/glad.h"

namespace backend {
namespace opengl {

    class Device;

    std::string GetBindingName(uint32_t group, uint32_t binding);

    struct BindingLocation {
        uint32_t group;
        uint32_t binding;
    };
    bool operator < (const BindingLocation& a, const BindingLocation& b);

    struct CombinedSampler {
        BindingLocation samplerLocation;
        BindingLocation textureLocation;
        std::string GetName() const;
    };
    bool operator < (const CombinedSampler& a, const CombinedSampler& b);

    class ShaderModule : public ShaderModuleBase {
        public:
            ShaderModule(ShaderModuleBuilder* builder);

            using CombinedSamplerInfo = std::vector<CombinedSampler>;

            const char* GetSource() const;
            const CombinedSamplerInfo& GetCombinedSamplerInfo() const;

        private:
            CombinedSamplerInfo combinedInfo;
            std::string glslSource;
    };

}
}

#endif // BACKEND_OPENGL_SHADERMODULEGL_H_
