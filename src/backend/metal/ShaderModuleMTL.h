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

#ifndef BACKEND_METAL_SHADERMODULEMTL_H_
#define BACKEND_METAL_SHADERMODULEMTL_H_

#include "common/ShaderModule.h"

#import <Metal/Metal.h>

namespace spirv_cross {
    class CompilerMSL;
}

namespace backend {
namespace metal {

    class ShaderModule : public ShaderModuleBase {
        public:
            ShaderModule(ShaderModuleBuilder* builder);
            ~ShaderModule();

            id<MTLFunction> GetFunction(const char* functionName) const;
            MTLSize GetLocalWorkGroupSize(const std::string& entryPoint) const;

        private:
            id<MTLLibrary> mtlLibrary = nil;
            spirv_cross::CompilerMSL* compiler = nullptr;
    };

}
}

#endif // BACKEND_METAL_SHADERMODULEMTL_H_
