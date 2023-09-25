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

#ifndef SRC_TINT_LANG_CORE_IR_TRANSFORM_ZERO_INIT_WORKGROUP_MEMORY_H_
#define SRC_TINT_LANG_CORE_IR_TRANSFORM_ZERO_INIT_WORKGROUP_MEMORY_H_

#include <string>

#include "src/tint/utils/result/result.h"

// Forward declarations.
namespace tint::core::ir {
class Module;
}

namespace tint::core::ir::transform {

/// ZeroInitWorkgroupMemory is a transform that injects code at the top of each entry point to
/// zero-initialize workgroup memory used by that entry point.
/// @param module the module to transform
/// @returns success or failure
Result<SuccessType> ZeroInitWorkgroupMemory(Module& module);

}  // namespace tint::core::ir::transform

#endif  // SRC_TINT_LANG_CORE_IR_TRANSFORM_ZERO_INIT_WORKGROUP_MEMORY_H_
