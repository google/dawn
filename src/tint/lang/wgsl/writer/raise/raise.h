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

#ifndef SRC_TINT_LANG_WGSL_WRITER_RAISE_RAISE_H_
#define SRC_TINT_LANG_WGSL_WRITER_RAISE_RAISE_H_

#include "src/tint/lang/core/ir/module.h"
#include "src/tint/utils/diagnostic/diagnostic.h"
#include "src/tint/utils/result/result.h"

namespace tint::wgsl::writer::raise {

/// Raise converts a core-dialect IR module to a WGSL-dialect IR module
/// @param  mod the IR module
/// @return the result of the operation
Result<SuccessType> Raise(core::ir::Module& mod);

}  // namespace tint::wgsl::writer::raise

#endif  // SRC_TINT_LANG_WGSL_WRITER_RAISE_RAISE_H_
