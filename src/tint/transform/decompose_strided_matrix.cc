// Copyright 2021 The Tint Authors.
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

#include "src/tint/transform/decompose_strided_matrix.h"

#include <unordered_map>
#include <utility>
#include <vector>

#include "src/tint/program_builder.h"
#include "src/tint/sem/expression.h"
#include "src/tint/sem/member_accessor_expression.h"
#include "src/tint/transform/simplify_pointers.h"
#include "src/tint/utils/hash.h"
#include "src/tint/utils/map.h"

TINT_INSTANTIATE_TYPEINFO(tint::transform::DecomposeStridedMatrix);

namespace tint::transform {
namespace {

/// MatrixInfo describes a matrix member with a custom stride
struct MatrixInfo {
    /// The stride in bytes between columns of the matrix
    uint32_t stride = 0;
    /// The type of the matrix
    const sem::Matrix* matrix = nullptr;

    /// @returns a new ast::Array that holds an vector column for each row of the
    /// matrix.
    const ast::Array* array(ProgramBuilder* b) const {
        return b->ty.array(b->ty.vec<f32>(matrix->rows()), u32(matrix->columns()), stride);
    }

    /// Equality operator
    bool operator==(const MatrixInfo& info) const {
        return stride == info.stride && matrix == info.matrix;
    }
    /// Hash function
    struct Hasher {
        size_t operator()(const MatrixInfo& t) const { return utils::Hash(t.stride, t.matrix); }
    };
};

/// Return type of the callback function of GatherCustomStrideMatrixMembers
enum GatherResult { kContinue, kStop };

/// GatherCustomStrideMatrixMembers scans `program` for all matrix members of
/// storage and uniform structs, which are of a matrix type, and have a custom
/// matrix stride attribute. For each matrix member found, `callback` is called.
/// `callback` is a function with the signature:
///      GatherResult(const sem::StructMember* member,
///                   sem::Matrix* matrix,
///                   uint32_t stride)
/// If `callback` return GatherResult::kStop, then the scanning will immediately
/// terminate, and GatherCustomStrideMatrixMembers() will return, otherwise
/// scanning will continue.
template <typename F>
void GatherCustomStrideMatrixMembers(const Program* program, F&& callback) {
    for (auto* node : program->ASTNodes().Objects()) {
        if (auto* str = node->As<ast::Struct>()) {
            auto* str_ty = program->Sem().Get(str);
            if (!str_ty->UsedAs(ast::StorageClass::kUniform) &&
                !str_ty->UsedAs(ast::StorageClass::kStorage)) {
                continue;
            }
            for (auto* member : str_ty->Members()) {
                auto* matrix = member->Type()->As<sem::Matrix>();
                if (!matrix) {
                    continue;
                }
                auto* attr =
                    ast::GetAttribute<ast::StrideAttribute>(member->Declaration()->attributes);
                if (!attr) {
                    continue;
                }
                uint32_t stride = attr->stride;
                if (matrix->ColumnStride() == stride) {
                    continue;
                }
                if (callback(member, matrix, stride) == GatherResult::kStop) {
                    return;
                }
            }
        }
    }
}

}  // namespace

DecomposeStridedMatrix::DecomposeStridedMatrix() = default;

DecomposeStridedMatrix::~DecomposeStridedMatrix() = default;

bool DecomposeStridedMatrix::ShouldRun(const Program* program, const DataMap&) const {
    bool should_run = false;
    GatherCustomStrideMatrixMembers(program, [&](const sem::StructMember*, sem::Matrix*, uint32_t) {
        should_run = true;
        return GatherResult::kStop;
    });
    return should_run;
}

void DecomposeStridedMatrix::Run(CloneContext& ctx, const DataMap&, DataMap&) const {
    // Scan the program for all storage and uniform structure matrix members with
    // a custom stride attribute. Replace these matrices with an equivalent array,
    // and populate the `decomposed` map with the members that have been replaced.
    std::unordered_map<const ast::StructMember*, MatrixInfo> decomposed;
    GatherCustomStrideMatrixMembers(
        ctx.src, [&](const sem::StructMember* member, sem::Matrix* matrix, uint32_t stride) {
            // We've got ourselves a struct member of a matrix type with a custom
            // stride. Replace this with an array of column vectors.
            MatrixInfo info{stride, matrix};
            auto* replacement =
                ctx.dst->Member(member->Offset(), ctx.Clone(member->Name()), info.array(ctx.dst));
            ctx.Replace(member->Declaration(), replacement);
            decomposed.emplace(member->Declaration(), info);
            return GatherResult::kContinue;
        });

    // For all expressions where a single matrix column vector was indexed, we can
    // preserve these without calling conversion functions.
    // Example:
    //   ssbo.mat[2] -> ssbo.mat[2]
    ctx.ReplaceAll(
        [&](const ast::IndexAccessorExpression* expr) -> const ast::IndexAccessorExpression* {
            if (auto* access = ctx.src->Sem().Get<sem::StructMemberAccess>(expr->object)) {
                auto it = decomposed.find(access->Member()->Declaration());
                if (it != decomposed.end()) {
                    auto* obj = ctx.CloneWithoutTransform(expr->object);
                    auto* idx = ctx.Clone(expr->index);
                    return ctx.dst->IndexAccessor(obj, idx);
                }
            }
            return nullptr;
        });

    // For all struct member accesses to the matrix on the LHS of an assignment,
    // we need to convert the matrix to the array before assigning to the
    // structure.
    // Example:
    //   ssbo.mat = mat_to_arr(m)
    std::unordered_map<MatrixInfo, Symbol, MatrixInfo::Hasher> mat_to_arr;
    ctx.ReplaceAll([&](const ast::AssignmentStatement* stmt) -> const ast::Statement* {
        if (auto* access = ctx.src->Sem().Get<sem::StructMemberAccess>(stmt->lhs)) {
            auto it = decomposed.find(access->Member()->Declaration());
            if (it == decomposed.end()) {
                return nullptr;
            }
            MatrixInfo info = it->second;
            auto fn = utils::GetOrCreate(mat_to_arr, info, [&] {
                auto name =
                    ctx.dst->Symbols().New("mat" + std::to_string(info.matrix->columns()) + "x" +
                                           std::to_string(info.matrix->rows()) + "_stride_" +
                                           std::to_string(info.stride) + "_to_arr");

                auto matrix = [&] { return CreateASTTypeFor(ctx, info.matrix); };
                auto array = [&] { return info.array(ctx.dst); };

                auto mat = ctx.dst->Sym("m");
                ast::ExpressionList columns(info.matrix->columns());
                for (uint32_t i = 0; i < static_cast<uint32_t>(columns.size()); i++) {
                    columns[i] = ctx.dst->IndexAccessor(mat, u32(i));
                }
                ctx.dst->Func(name,
                              {
                                  ctx.dst->Param(mat, matrix()),
                              },
                              array(),
                              {
                                  ctx.dst->Return(ctx.dst->Construct(array(), columns)),
                              });
                return name;
            });
            auto* lhs = ctx.CloneWithoutTransform(stmt->lhs);
            auto* rhs = ctx.dst->Call(fn, ctx.Clone(stmt->rhs));
            return ctx.dst->Assign(lhs, rhs);
        }
        return nullptr;
    });

    // For all other struct member accesses, we need to convert the array to the
    // matrix type. Example:
    //   m = arr_to_mat(ssbo.mat)
    std::unordered_map<MatrixInfo, Symbol, MatrixInfo::Hasher> arr_to_mat;
    ctx.ReplaceAll([&](const ast::MemberAccessorExpression* expr) -> const ast::Expression* {
        if (auto* access = ctx.src->Sem().Get<sem::StructMemberAccess>(expr)) {
            auto it = decomposed.find(access->Member()->Declaration());
            if (it == decomposed.end()) {
                return nullptr;
            }
            MatrixInfo info = it->second;
            auto fn = utils::GetOrCreate(arr_to_mat, info, [&] {
                auto name = ctx.dst->Symbols().New(
                    "arr_to_mat" + std::to_string(info.matrix->columns()) + "x" +
                    std::to_string(info.matrix->rows()) + "_stride_" + std::to_string(info.stride));

                auto matrix = [&] { return CreateASTTypeFor(ctx, info.matrix); };
                auto array = [&] { return info.array(ctx.dst); };

                auto arr = ctx.dst->Sym("arr");
                ast::ExpressionList columns(info.matrix->columns());
                for (uint32_t i = 0; i < static_cast<uint32_t>(columns.size()); i++) {
                    columns[i] = ctx.dst->IndexAccessor(arr, u32(i));
                }
                ctx.dst->Func(name,
                              {
                                  ctx.dst->Param(arr, array()),
                              },
                              matrix(),
                              {
                                  ctx.dst->Return(ctx.dst->Construct(matrix(), columns)),
                              });
                return name;
            });
            return ctx.dst->Call(fn, ctx.CloneWithoutTransform(expr));
        }
        return nullptr;
    });

    ctx.Clone();
}

}  // namespace tint::transform
