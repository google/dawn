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

#include "src/tint/lang/core/ir/transform/builtin_polyfill.h"

#include <utility>

#include "src/tint/lang/core/ir/builder.h"
#include "src/tint/lang/core/ir/module.h"
#include "src/tint/lang/core/ir/validator.h"

using namespace tint::builtin::fluent_types;  // NOLINT
using namespace tint::number_suffixes;        // NOLINT

namespace tint::ir::transform {

namespace {

/// PIMPL state for the transform.
struct State {
    /// The polyfill config.
    const BuiltinPolyfillConfig& config;

    /// The IR module.
    Module* ir = nullptr;

    /// The IR builder.
    Builder b{*ir};

    /// The type manager.
    type::Manager& ty{ir->Types()};

    /// The symbol table.
    SymbolTable& sym{ir->symbols};

    /// Process the module.
    void Process() {
        // Find the builtin call instructions that may need to be polyfilled.
        Vector<ir::CoreBuiltinCall*, 4> worklist;
        for (auto* inst : ir->instructions.Objects()) {
            if (!inst->Alive()) {
                continue;
            }
            if (auto* builtin = inst->As<ir::CoreBuiltinCall>()) {
                switch (builtin->Func()) {
                    case builtin::Function::kSaturate:
                        if (config.saturate) {
                            worklist.Push(builtin);
                        }
                        break;
                    default:
                        break;
                }
            }
        }

        // Polyfill the builtin call instructions that we found.
        for (auto* builtin : worklist) {
            ir::Value* replacement = nullptr;
            switch (builtin->Func()) {
                case builtin::Function::kSaturate:
                    replacement = Saturate(builtin);
                    break;
                default:
                    break;
            }
            TINT_ASSERT_OR_RETURN(replacement);

            // Replace the old builtin call result with the new value.
            if (auto name = ir->NameOf(builtin->Result())) {
                ir->SetName(replacement, name);
            }
            builtin->Result()->ReplaceAllUsesWith(replacement);
            builtin->Destroy();
        }
    }

    /// Polyfill a `saturate()` builtin call.
    /// @param call the builtin call instruction
    /// @returns the replacement value
    ir::Value* Saturate(ir::CoreBuiltinCall* call) {
        // Replace `saturate(x)` with `clamp(x, 0., 1.)`.
        auto* type = call->Result()->Type();
        ir::Constant* zero = nullptr;
        ir::Constant* one = nullptr;
        if (type->DeepestElement()->Is<type::F32>()) {
            zero = b.Constant(0_f);
            one = b.Constant(1_f);
        } else if (type->DeepestElement()->Is<type::F16>()) {
            zero = b.Constant(0_h);
            one = b.Constant(1_h);
        }
        if (auto* vec = type->As<type::Vector>()) {
            // Splat the zero and one arguments to vectors.
            zero = b.Splat(vec, zero, vec->Width());
            one = b.Splat(vec, one, vec->Width());
        }
        auto* clamp = b.Call(type, builtin::Function::kClamp, Vector{call->Args()[0], zero, one});
        clamp->InsertBefore(call);
        return clamp->Result();
    }
};

}  // namespace

Result<SuccessType, std::string> BuiltinPolyfill(Module* ir, const BuiltinPolyfillConfig& config) {
    auto result = ValidateAndDumpIfNeeded(*ir, "BuiltinPolyfill transform");
    if (!result) {
        return result;
    }

    State{config, ir}.Process();

    return Success;
}

}  // namespace tint::ir::transform
