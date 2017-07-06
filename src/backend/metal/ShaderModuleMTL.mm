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

#include "backend/metal/ShaderModuleMTL.h"

#include "backend/metal/MetalBackend.h"

#include <spirv-cross/spirv_msl.hpp>

#include <sstream>

namespace backend {
namespace metal {

    ShaderModule::ShaderModule(ShaderModuleBuilder* builder)
        : ShaderModuleBase(builder) {
        compiler = new spirv_cross::CompilerMSL(builder->AcquireSpirv());
        ExtractSpirvInfo(*compiler);

        std::string msl = compiler->compile();
        NSString* mslSource = [NSString stringWithFormat:@"%s", msl.c_str()];

        auto mtlDevice = ToBackend(builder->GetDevice())->GetMTLDevice();
        NSError *error = nil;
        mtlLibrary = [mtlDevice newLibraryWithSource:mslSource options:nil error:&error];
        if (error != nil) {
            NSLog(@"MTLDevice newLibraryWithSource => %@", error);
            builder->HandleError("Error creating MTLLibrary from MSL source");
        }
    }

    ShaderModule::~ShaderModule() {
        delete compiler;
    }

    id<MTLFunction> ShaderModule::GetFunction(const char* functionName) const {
        // TODO(kainino@chromium.org): make this somehow more robust; it needs to behave like clean_func_name:
        // https://github.com/KhronosGroup/SPIRV-Cross/blob/4e915e8c483e319d0dd7a1fa22318bef28f8cca3/spirv_msl.cpp#L1213
        if (strcmp(functionName, "main") == 0) {
            functionName = "main0";
        }
        NSString* name = [NSString stringWithFormat:@"%s", functionName];
        return [mtlLibrary newFunctionWithName:name];
    }

    MTLSize ShaderModule::GetLocalWorkGroupSize(const std::string& entryPoint) const {
        auto size = compiler->get_entry_point(entryPoint).workgroup_size;
        return MTLSizeMake(size.x, size.y, size.z);
    }

}
}
