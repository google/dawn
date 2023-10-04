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

#ifndef SRC_TINT_LANG_SPIRV_WRITER_RAISE_BUILTIN_POLYFILL_H_
#define SRC_TINT_LANG_SPIRV_WRITER_RAISE_BUILTIN_POLYFILL_H_

#include <string>

#include "src/tint/lang/core/ir/constant.h"
#include "src/tint/lang/core/type/type.h"
#include "src/tint/utils/diagnostic/diagnostic.h"
#include "src/tint/utils/result/result.h"

// Forward declarations.
namespace tint::core::ir {
class Module;
class Texture;
}  // namespace tint::core::ir

namespace tint::spirv::writer::raise {

/// BuiltinPolyfill is a transform that replaces calls to builtins with polyfills and calls to
/// SPIR-V backend intrinsic functions.
/// @param module the module to transform
/// @returns success or failure
Result<SuccessType> BuiltinPolyfill(core::ir::Module& module);

}  // namespace tint::spirv::writer::raise

#endif  // SRC_TINT_LANG_SPIRV_WRITER_RAISE_BUILTIN_POLYFILL_H_
