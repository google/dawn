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

#ifndef SRC_TINT_LANG_CORE_IR_VALIDATOR_H_
#define SRC_TINT_LANG_CORE_IR_VALIDATOR_H_

#include <string>

#include "src/tint/utils/diagnostic/diagnostic.h"
#include "src/tint/utils/result/result.h"

// Forward declarations
namespace tint::core::ir {
class Module;
}  // namespace tint::core::ir

namespace tint::core::ir {

/// Validates that a given IR module is correctly formed
/// @param mod the module to validate
/// @returns true on success, an error result otherwise
Result<SuccessType, diag::List> Validate(Module& mod);

/// Validates the module @p ir and dumps its contents if required by the build configuration.
/// @param ir the module to transform
/// @param msg the msg to accompany the output
/// @returns an error string if the module is not valid
Result<SuccessType, std::string> ValidateAndDumpIfNeeded(Module& ir, const char* msg);

}  // namespace tint::core::ir

#endif  // SRC_TINT_LANG_CORE_IR_VALIDATOR_H_
