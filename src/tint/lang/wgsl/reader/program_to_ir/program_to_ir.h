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

#ifndef SRC_TINT_LANG_WGSL_READER_PROGRAM_TO_IR_PROGRAM_TO_IR_H_
#define SRC_TINT_LANG_WGSL_READER_PROGRAM_TO_IR_PROGRAM_TO_IR_H_

#include <string>

#include "src/tint/lang/core/ir/module.h"
#include "src/tint/utils/result/result.h"

// Forward Declarations
namespace tint {
class Program;
}  // namespace tint

namespace tint::wgsl::reader {

/// Builds an core::ir::Module from the given Program
/// @param program the Program to use.
/// @returns the `utiils::Result` of generating the IR. The result will contain the
/// `core::ir::Module` on success, otherwise the `std::string` error.
///
/// @note this assumes the `program.IsValid()`, and has had const-eval done so
/// any abstract values have been calculated and converted into the relevant
/// concrete types.
tint::Result<core::ir::Module, std::string> ProgramToIR(const Program& program);

}  // namespace tint::wgsl::reader

#endif  // SRC_TINT_LANG_WGSL_READER_PROGRAM_TO_IR_PROGRAM_TO_IR_H_
