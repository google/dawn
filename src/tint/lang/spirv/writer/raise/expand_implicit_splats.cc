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

#include "src/tint/lang/spirv/writer/raise/expand_implicit_splats.h"

#include <utility>

#include "src/tint/lang/core/ir/builder.h"
#include "src/tint/lang/core/ir/module.h"
#include "src/tint/lang/core/ir/validator.h"
#include "src/tint/lang/spirv/builtin_fn.h"
#include "src/tint/lang/spirv/ir/builtin_call.h"

using namespace tint::core::number_suffixes;  // NOLINT

namespace tint::spirv::writer::raise {

namespace {

void Run(core::ir::Module* ir) {
    core::ir::Builder b(*ir);

    // Find the instructions that use implicit splats and either modify them in place or record them
    // to be replaced in a second pass.
    Vector<core::ir::Binary*, 4> binary_worklist;
    Vector<core::ir::CoreBuiltinCall*, 4> builtin_worklist;
    for (auto* inst : ir->instructions.Objects()) {
        if (!inst->Alive()) {
            continue;
        }
        if (auto* construct = inst->As<core::ir::Construct>()) {
            // A vector constructor with a single scalar argument needs to be modified to replicate
            // the argument N times.
            auto* vec = construct->Result()->Type()->As<core::type::Vector>();
            if (vec &&  //
                construct->Args().Length() == 1 &&
                construct->Args()[0]->Type()->Is<core::type::Scalar>()) {
                for (uint32_t i = 1; i < vec->Width(); i++) {
                    construct->AppendArg(construct->Args()[0]);
                }
            }
        } else if (auto* binary = inst->As<core::ir::Binary>()) {
            // A binary instruction that mixes vector and scalar operands needs to have the scalar
            // operand replaced with an explicit vector constructor.
            if (binary->Result()->Type()->Is<core::type::Vector>()) {
                if (binary->LHS()->Type()->Is<core::type::Scalar>() ||
                    binary->RHS()->Type()->Is<core::type::Scalar>()) {
                    binary_worklist.Push(binary);
                }
            }
        } else if (auto* builtin = inst->As<core::ir::CoreBuiltinCall>()) {
            // A mix builtin call that mixes vector and scalar operands needs to have the scalar
            // operand replaced with an explicit vector constructor.
            if (builtin->Func() == core::BuiltinFn::kMix) {
                if (builtin->Result()->Type()->Is<core::type::Vector>()) {
                    if (builtin->Args()[2]->Type()->Is<core::type::Scalar>()) {
                        builtin_worklist.Push(builtin);
                    }
                }
            }
        }
    }

    // Helper to expand a scalar operand of an instruction by replacing it with an explicitly
    // constructed vector that matches the result type.
    auto expand_operand = [&](core::ir::Instruction* inst, size_t operand_idx) {
        auto* vec = inst->Result()->Type()->As<core::type::Vector>();

        Vector<core::ir::Value*, 4> args;
        args.Resize(vec->Width(), inst->Operands()[operand_idx]);

        auto* construct = b.Construct(vec, std::move(args));
        construct->InsertBefore(inst);
        inst->SetOperand(operand_idx, construct->Result());
    };

    // Replace scalar operands to binary instructions that produce vectors.
    for (auto* binary : binary_worklist) {
        auto* result_ty = binary->Result()->Type();
        if (result_ty->is_float_vector() && binary->Kind() == core::ir::Binary::Kind::kMultiply) {
            // Use OpVectorTimesScalar for floating point multiply.
            auto* vts =
                b.Call<spirv::ir::BuiltinCall>(result_ty, spirv::BuiltinFn::kVectorTimesScalar);
            if (binary->LHS()->Type()->Is<core::type::Scalar>()) {
                vts->AppendArg(binary->RHS());
                vts->AppendArg(binary->LHS());
            } else {
                vts->AppendArg(binary->LHS());
                vts->AppendArg(binary->RHS());
            }
            if (auto name = ir->NameOf(binary)) {
                ir->SetName(vts->Result(), name);
            }
            binary->Result()->ReplaceAllUsesWith(vts->Result());
            binary->ReplaceWith(vts);
            binary->Destroy();
        } else {
            // Expand the scalar argument into an explicitly constructed vector.
            if (binary->LHS()->Type()->Is<core::type::Scalar>()) {
                expand_operand(binary, core::ir::Binary::kLhsOperandOffset);
            } else if (binary->RHS()->Type()->Is<core::type::Scalar>()) {
                expand_operand(binary, core::ir::Binary::kRhsOperandOffset);
            }
        }
    }

    // Replace scalar arguments to builtin calls that produce vectors.
    for (auto* builtin : builtin_worklist) {
        switch (builtin->Func()) {
            case core::BuiltinFn::kMix:
                // Expand the scalar argument into an explicitly constructed vector.
                expand_operand(builtin, core::ir::CoreBuiltinCall::kArgsOperandOffset + 2);
                break;
            default:
                TINT_UNREACHABLE() << "unhandled builtin call";
                break;
        }
    }
}

}  // namespace

Result<SuccessType, std::string> ExpandImplicitSplats(core::ir::Module* ir) {
    auto result = ValidateAndDumpIfNeeded(*ir, "ExpandImplicitSplats transform");
    if (!result) {
        return result;
    }

    Run(ir);

    return Success;
}

}  // namespace tint::spirv::writer::raise
