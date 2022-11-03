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

#include "tint/tint.h"

namespace tint {

void Initialize() {
#if TINT_BUILD_WGSL_WRITER
    // Register the Program printer. This is used for debugging purposes.
    tint::Program::printer = [](const tint::Program* program) {
        auto result = writer::wgsl::Generate(program, {});
        if (!result.error.empty()) {
            return "error: " + result.error;
        }
        return result.wgsl;
    };
#endif
}

void Shutdown() {
    // Currently no-op, but may release tint resources in the future.
}

}  // namespace tint
