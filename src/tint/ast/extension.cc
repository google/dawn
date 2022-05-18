// Copyright 2022 The Tint Authors.
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

#include "src/tint/ast/extension.h"

namespace tint::ast {

Extension ParseExtension(const std::string& name) {
    if (name == "chromium_experimental_dp4a") {
        return Extension::kChromiumExperimentalDP4a;
    }
    if (name == "chromium_disable_uniformity_analysis") {
        return Extension::kChromiumDisableUniformityAnalysis;
    }
    if (name == "f16") {
        return Extension::kF16;
    }
    return Extension::kNone;
}

const char* str(Extension ext) {
    switch (ext) {
        case Extension::kChromiumExperimentalDP4a:
            return "chromium_experimental_dp4a";
        case Extension::kChromiumDisableUniformityAnalysis:
            return "chromium_disable_uniformity_analysis";
        case Extension::kF16:
            return "f16";
        case Extension::kNone:
            return "<none>";
    }
    return "<unknown>";
}

std::ostream& operator<<(std::ostream& out, Extension i) {
    out << str(i);
    return out;
}

}  // namespace tint::ast
