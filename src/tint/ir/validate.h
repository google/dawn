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

#ifndef SRC_TINT_IR_VALIDATE_H_
#define SRC_TINT_IR_VALIDATE_H_

#include "src/tint/diagnostic/diagnostic.h"
#include "src/tint/ir/module.h"
#include "src/tint/utils/result.h"

namespace tint::ir {

/// Signifies the validation completed successfully
struct Success {};

/// Validates that a given IR module is correctly formed
/// @param mod the module to validate
/// @returns true on success, an error result otherwise
utils::Result<Success, diag::List> Validate(Module& mod);

}  // namespace tint::ir

#endif  // SRC_TINT_IR_VALIDATE_H_
