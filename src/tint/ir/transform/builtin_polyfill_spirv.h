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

#ifndef SRC_TINT_IR_TRANSFORM_BUILTIN_POLYFILL_SPIRV_H_
#define SRC_TINT_IR_TRANSFORM_BUILTIN_POLYFILL_SPIRV_H_

#include "src/tint/ir/transform/transform.h"

namespace tint::ir::transform {

/// BuiltinPolyfillSpirv is a transform that replaces calls to builtins with polyfills and calls to
/// SPIR-V backend intrinsic functions.
class BuiltinPolyfillSpirv final : public utils::Castable<BuiltinPolyfillSpirv, Transform> {
  public:
    /// Constructor
    BuiltinPolyfillSpirv();
    /// Destructor
    ~BuiltinPolyfillSpirv() override;

    /// @copydoc Transform::Run
    void Run(ir::Module* module, const DataMap& inputs, DataMap& outputs) const override;

  private:
    struct State;
};

}  // namespace tint::ir::transform

#endif  // SRC_TINT_IR_TRANSFORM_BUILTIN_POLYFILL_SPIRV_H_
