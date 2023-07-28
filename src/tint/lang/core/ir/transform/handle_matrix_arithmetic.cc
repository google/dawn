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

#include "src/tint/lang/core/ir/transform/handle_matrix_arithmetic.h"

#include <utility>

#include "src/tint/lang/core/ir/builder.h"
#include "src/tint/lang/core/ir/module.h"
#include "src/tint/lang/core/type/matrix.h"
#include "src/tint/utils/ice/ice.h"

TINT_INSTANTIATE_TYPEINFO(tint::ir::transform::HandleMatrixArithmetic);

using namespace tint::number_suffixes;  // NOLINT

namespace tint::ir::transform {

HandleMatrixArithmetic::HandleMatrixArithmetic() = default;

HandleMatrixArithmetic::~HandleMatrixArithmetic() = default;

void HandleMatrixArithmetic::Run(ir::Module* ir) const {
    ir::Builder b(*ir);

    // Find the instructions that need to be modified.
    Vector<Binary*, 4> binary_worklist;
    Vector<Convert*, 4> convert_worklist;
    for (auto* inst : ir->instructions.Objects()) {
        if (!inst->Alive()) {
            continue;
        }
        if (auto* binary = inst->As<Binary>()) {
            TINT_ASSERT(binary->Operands().Length() == 2);
            if (binary->LHS()->Type()->Is<type::Matrix>() ||
                binary->RHS()->Type()->Is<type::Matrix>()) {
                binary_worklist.Push(binary);
            }
        } else if (auto* convert = inst->As<Convert>()) {
            if (convert->Result()->Type()->Is<type::Matrix>()) {
                convert_worklist.Push(convert);
            }
        }
    }

    // Replace the matrix arithmetic instructions that we found.
    for (auto* binary : binary_worklist) {
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
            Vector<Value*, 4> args;
            for (uint32_t col = 0; col < mat->columns(); col++) {
                b.InsertBefore(binary, [&] {
                    auto* lhs_col = b.Access(mat->ColumnType(), lhs, u32(col));
                    auto* rhs_col = b.Access(mat->ColumnType(), rhs, u32(col));
                    auto* add = b.Binary(op, mat->ColumnType(), lhs_col, rhs_col);
                    args.Push(add->Result());
                });
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
                TINT_UNREACHABLE() << "unhandled matrix arithmetic instruction";
                break;
        }
    }

    // Replace the matrix convert instructions that we found.
    for (auto* convert : convert_worklist) {
        auto* arg = convert->Args()[Convert::kValueOperandOffset];
        auto* in_mat = arg->Type()->As<type::Matrix>();
        auto* out_mat = convert->Result()->Type()->As<type::Matrix>();

        // Extract and convert each column separately.
        Vector<Value*, 4> args;
        for (uint32_t c = 0; c < out_mat->columns(); c++) {
            b.InsertBefore(convert, [&] {
                auto* col = b.Access(in_mat->ColumnType(), arg, u32(c));
                auto* new_col = b.Convert(out_mat->ColumnType(), col);
                args.Push(new_col->Result());
            });
        }

        // Reconstruct the result matrix from the converted columns.
        auto* construct = b.Construct(out_mat, std::move(args));
        if (auto name = ir->NameOf(convert)) {
            ir->SetName(construct->Result(), name);
        }
        convert->Result()->ReplaceAllUsesWith(construct->Result());
        convert->ReplaceWith(construct);
        convert->Destroy();
    }
}

}  // namespace tint::ir::transform
