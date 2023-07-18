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

#include "src/tint/ir/transform/handle_matrix_arithmetic.h"

#include <utility>

#include "src/tint/ir/builder.h"
#include "src/tint/ir/module.h"
#include "src/tint/type/matrix.h"

TINT_INSTANTIATE_TYPEINFO(tint::ir::transform::HandleMatrixArithmetic);

using namespace tint::number_suffixes;  // NOLINT

namespace tint::ir::transform {

HandleMatrixArithmetic::HandleMatrixArithmetic() = default;

HandleMatrixArithmetic::~HandleMatrixArithmetic() = default;

void HandleMatrixArithmetic::Run(ir::Module* ir, const DataMap&, DataMap&) const {
    ir::Builder b(*ir);

    // Find the instructions that needs to be modified.
    utils::Vector<Binary*, 4> worklist;
    for (auto* inst : ir->instructions.Objects()) {
        if (!inst->Alive()) {
            continue;
        }
        if (auto* binary = inst->As<Binary>()) {
            TINT_ASSERT(Transform, binary->Operands().Length() == 2);
            if (binary->LHS()->Type()->Is<type::Matrix>() ||
                binary->RHS()->Type()->Is<type::Matrix>()) {
                worklist.Push(binary);
            }
        }
    }

    // Replace the matrix arithmetic instructions that we found.
    for (auto* binary : worklist) {
        auto* lhs = binary->LHS();
        auto* rhs = binary->RHS();
        auto* lhs_ty = lhs->Type();
        auto* rhs_ty = rhs->Type();
        auto* ty = binary->Result()->Type();

        // Helper to replace the instruction with a new one.
        auto replace = [&](Instruction* inst) {
            if (auto name = ir->NameOf(binary)) {
                ir->SetName(inst->Result(), name);
            }
            binary->Result()->ReplaceAllUsesWith(inst->Result());
            binary->ReplaceWith(inst);
            binary->Destroy();
        };

        // Helper to replace the instruction with a column-wise operation.
        auto column_wise = [&](enum Binary::Kind op) {
            auto* mat = ty->As<type::Matrix>();
            utils::Vector<Value*, 4> args;
            for (uint32_t col = 0; col < mat->columns(); col++) {
                auto* lhs_col = b.Access(mat->ColumnType(), lhs, u32(col));
                lhs_col->InsertBefore(binary);
                auto* rhs_col = b.Access(mat->ColumnType(), rhs, u32(col));
                rhs_col->InsertBefore(binary);
                auto* add = b.Binary(op, mat->ColumnType(), lhs_col, rhs_col);
                add->InsertBefore(binary);
                args.Push(add->Result());
            }
            replace(b.Construct(ty, std::move(args)));
        };

        switch (binary->Kind()) {
            case Binary::Kind::kAdd:
                column_wise(Binary::Kind::kAdd);
                break;
            case Binary::Kind::kSubtract:
                column_wise(Binary::Kind::kSubtract);
                break;
            case Binary::Kind::kMultiply:
                // Select the SPIR-V intrinsic that corresponds to the operation being performed.
                if (lhs_ty->Is<type::Matrix>()) {
                    if (rhs_ty->Is<type::Scalar>()) {
                        replace(b.Call(ty, IntrinsicCall::Kind::kSpirvMatrixTimesScalar, lhs, rhs));
                    } else if (rhs_ty->Is<type::Vector>()) {
                        replace(b.Call(ty, IntrinsicCall::Kind::kSpirvMatrixTimesVector, lhs, rhs));
                    } else if (rhs_ty->Is<type::Matrix>()) {
                        replace(b.Call(ty, IntrinsicCall::Kind::kSpirvMatrixTimesMatrix, lhs, rhs));
                    }
                } else {
                    if (lhs_ty->Is<type::Scalar>()) {
                        replace(b.Call(ty, IntrinsicCall::Kind::kSpirvMatrixTimesScalar, rhs, lhs));
                    } else if (lhs_ty->Is<type::Vector>()) {
                        replace(b.Call(ty, IntrinsicCall::Kind::kSpirvVectorTimesMatrix, lhs, rhs));
                    }
                }
                break;

            default:
                TINT_ASSERT(Transform, false && "unhandled matrix arithmetic instruction");
                break;
        }
    }
}

}  // namespace tint::ir::transform
