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

#include "src/tint/ir/transform/builtin_polyfill_spirv.h"

#include <utility>

#include "src/tint/ir/builder.h"
#include "src/tint/ir/module.h"

TINT_INSTANTIATE_TYPEINFO(tint::ir::transform::BuiltinPolyfillSpirv);

using namespace tint::number_suffixes;  // NOLINT

namespace tint::ir::transform {

BuiltinPolyfillSpirv::BuiltinPolyfillSpirv() = default;

BuiltinPolyfillSpirv::~BuiltinPolyfillSpirv() = default;

/// PIMPL state for the transform.
struct BuiltinPolyfillSpirv::State {
    /// The IR module.
    Module* ir = nullptr;

    /// The IR builder.
    Builder b{*ir};

    /// The type manager.
    type::Manager& ty{ir->Types()};

    /// Process the module.
    void Process() {
        // Find the builtins that need replacing.
        utils::Vector<BuiltinCall*, 4> worklist;
        for (auto* inst : ir->instructions.Objects()) {
            if (!inst->Alive()) {
                continue;
            }
            if (auto* builtin = inst->As<BuiltinCall>()) {
                switch (builtin->Func()) {
                    case builtin::Function::kSelect:
                        worklist.Push(builtin);
                        break;
                    default:
                        break;
                }
            }
        }

        // Replace the builtins that we found.
        for (auto* builtin : worklist) {
            Value* replacement = nullptr;
            switch (builtin->Func()) {
                case builtin::Function::kSelect:
                    replacement = Select(builtin);
                    break;
                default:
                    break;
            }
            TINT_ASSERT_OR_RETURN(Transform, replacement);

            // Replace the old builtin result with the new value.
            if (auto name = ir->NameOf(builtin->Result())) {
                ir->SetName(replacement, name);
            }
            builtin->Result()->ReplaceAllUsesWith(replacement);
            builtin->Destroy();
        }
    }

    /// Handle a `select()` builtin.
    /// @param builtin the builtin call instruction
    /// @returns the replacement value
    Value* Select(BuiltinCall* builtin) {
        // Argument order is different in SPIR-V: (condition, true_operand, false_operand).
        utils::Vector<Value*, 4> args = {
            builtin->Args()[2],
            builtin->Args()[1],
            builtin->Args()[0],
        };

        // If the condition is scalar and the objects are vectors, we need to splat the condition
        // into a vector of the same size.
        // TODO(jrprice): We don't need to do this if we're targeting SPIR-V 1.4 or newer.
        auto* vec = builtin->Result()->Type()->As<type::Vector>();
        if (vec && args[0]->Type()->Is<type::Scalar>()) {
            utils::Vector<Value*, 4> elements;
            elements.Resize(vec->Width(), args[0]);

            auto* construct = b.Construct(ty.vec(ty.bool_(), vec->Width()), std::move(elements));
            construct->InsertBefore(builtin);
            args[0] = construct->Result();
        }

        // Replace the builtin call with a call to the spirv.select intrinsic.
        auto* call =
            b.Call(builtin->Result()->Type(), IntrinsicCall::Kind::kSpirvSelect, std::move(args));
        call->InsertBefore(builtin);
        return call->Result();
    }
};

void BuiltinPolyfillSpirv::Run(ir::Module* ir, const DataMap&, DataMap&) const {
    State{ir}.Process();
}

}  // namespace tint::ir::transform
