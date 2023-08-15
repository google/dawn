// Copyright 2023 The Tint Authors.
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

#include "src/tint/lang/msl/writer/raise/raise.h"

#include <utility>

namespace tint::msl::raise {

Result<SuccessType, std::string> Raise(core::ir::Module*) {
    // #define RUN_TRANSFORM(name)
    //     do {
    //         auto result = core::ir::transform::name(module);
    //         if (!result) {
    //             return result;
    //         }
    //     } while (false)

    return Success;
}

}  // namespace tint::msl::raise
