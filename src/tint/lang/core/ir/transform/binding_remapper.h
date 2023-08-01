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

#ifndef SRC_TINT_LANG_CORE_IR_TRANSFORM_BINDING_REMAPPER_H_
#define SRC_TINT_LANG_CORE_IR_TRANSFORM_BINDING_REMAPPER_H_

#include <string>

#include "src/tint/utils/result/result.h"
#include "tint/binding_remapper_options.h"

// Forward declarations.
namespace tint::ir {
class Module;
}

namespace tint::ir::transform {

/// BindingRemapper is a transform that remaps binding point indices and access controls.
/// @param module the module to transform
/// @param options the remapping options
/// @returns an error string on failure
Result<SuccessType, std::string> BindingRemapper(Module* module,
                                                 const BindingRemapperOptions& options);

}  // namespace tint::ir::transform

#endif  // SRC_TINT_LANG_CORE_IR_TRANSFORM_BINDING_REMAPPER_H_
