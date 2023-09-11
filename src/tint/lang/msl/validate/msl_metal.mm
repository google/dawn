// Copyright 2021 The Tint Authors.
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

// GEN_BUILD:CONDITION(is_mac)

#import <Metal/Metal.h>

#include "src/tint/lang/msl/validate/val.h"

namespace tint::msl::validate {

Result UsingMetalAPI(const std::string& src, MslVersion version) {
    Result result;

    NSError* error = nil;

    id<MTLDevice> device = MTLCreateSystemDefaultDevice();
    if (!device) {
        result.output = "MTLCreateSystemDefaultDevice returned null";
        result.failed = true;
        return result;
    }

    NSString* source = [NSString stringWithCString:src.c_str() encoding:NSUTF8StringEncoding];

    MTLCompileOptions* compileOptions = [MTLCompileOptions new];
    compileOptions.fastMathEnabled = true;
    switch (version) {
        case MslVersion::kMsl_1_2:
            compileOptions.languageVersion = MTLLanguageVersion1_2;
            break;
        case MslVersion::kMsl_2_1:
            compileOptions.languageVersion = MTLLanguageVersion2_1;
            break;
        case MslVersion::kMsl_2_3:
            if (@available(macOS 11.0, *)) {
                compileOptions.languageVersion = MTLLanguageVersion2_3;
            }
            break;
    }

    id<MTLLibrary> library = [device newLibraryWithSource:source
                                                  options:compileOptions
                                                    error:&error];
    if (!library) {
        NSString* output = [error localizedDescription];
        result.output = [output UTF8String];
        result.failed = true;
    }

    return result;
}

}  // namespace tint::msl::validate
