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

#include "src/tint/transform/localize_struct_array_assignment.h"

#include <unordered_map>
#include <utility>

#include "src/tint/ast/assignment_statement.h"
#include "src/tint/ast/traverse_expressions.h"
#include "src/tint/program_builder.h"
#include "src/tint/sem/expression.h"
#include "src/tint/sem/member_accessor_expression.h"
#include "src/tint/sem/reference_type.h"
#include "src/tint/sem/statement.h"
#include "src/tint/sem/variable.h"
#include "src/tint/transform/simplify_pointers.h"
#include "src/tint/utils/scoped_assignment.h"

TINT_INSTANTIATE_TYPEINFO(tint::transform::LocalizeStructArrayAssignment);

namespace tint::transform {

/// Private implementation of LocalizeStructArrayAssignment transform
class LocalizeStructArrayAssignment::State {
 private:
  CloneContext& ctx;
  ProgramBuilder& b;

  /// Returns true if `expr` contains an index accessor expression to a
  /// structure member of array type.
  bool ContainsStructArrayIndex(const ast::Expression* expr) {
    bool result = false;
    ast::TraverseExpressions(
        expr, b.Diagnostics(), [&](const ast::IndexAccessorExpression* ia) {
          // Indexing using a runtime value?
          auto* idx_sem = ctx.src->Sem().Get(ia->index);
          if (!idx_sem->ConstantValue().IsValid()) {
            // Indexing a member access expr?
            if (auto* ma = ia->object->As<ast::MemberAccessorExpression>()) {
              // That accesses an array?
              if (ctx.src->TypeOf(ma)->UnwrapRef()->Is<sem::Array>()) {
                result = true;
                return ast::TraverseAction::Stop;
              }
            }
          }
          return ast::TraverseAction::Descend;
        });

    return result;
  }

  // Returns the type and storage class of the originating variable of the lhs
  // of the assignment statement.
  // See https://www.w3.org/TR/WGSL/#originating-variable-section
  std::pair<const sem::Type*, ast::StorageClass>
  GetOriginatingTypeAndStorageClass(
      const ast::AssignmentStatement* assign_stmt) {
    auto* source_var = ctx.src->Sem().Get(assign_stmt->lhs)->SourceVariable();
    if (!source_var) {
      TINT_ICE(Transform, b.Diagnostics())
          << "Unable to determine originating variable for lhs of assignment "
             "statement";
      return {};
    }

    auto* type = source_var->Type();
    if (auto* ref = type->As<sem::Reference>()) {
      return {ref->StoreType(), ref->StorageClass()};
    } else if (auto* ptr = type->As<sem::Pointer>()) {
      return {ptr->StoreType(), ptr->StorageClass()};
    }

    TINT_ICE(Transform, b.Diagnostics())
        << "Expecting to find variable of type pointer or reference on lhs "
           "of assignment statement";
    return {};
  }

 public:
  /// Constructor
  /// @param ctx_in the CloneContext primed with the input program and
  /// ProgramBuilder
  explicit State(CloneContext& ctx_in) : ctx(ctx_in), b(*ctx_in.dst) {}

  /// Runs the transform
  void Run() {
    struct Shared {
      bool process_nested_nodes = false;
      ast::StatementList insert_before_stmts;
      ast::StatementList insert_after_stmts;
    } s;

    ctx.ReplaceAll([&](const ast::AssignmentStatement* assign_stmt)
                       -> const ast::Statement* {
      // Process if it's an assignment statement to a dynamically indexed array
      // within a struct on a function or private storage variable. This
      // specific use-case is what FXC fails to compile with:
      // error X3500: array reference cannot be used as an l-value; not natively
      // addressable
      if (!ContainsStructArrayIndex(assign_stmt->lhs)) {
        return nullptr;
      }
      auto og = GetOriginatingTypeAndStorageClass(assign_stmt);
      if (!(og.first->Is<sem::Struct>() &&
            (og.second == ast::StorageClass::kFunction ||
             og.second == ast::StorageClass::kPrivate))) {
        return nullptr;
      }

      // Reset shared state for this assignment statement
      s = Shared{};

      const ast::Expression* new_lhs = nullptr;
      {
        TINT_SCOPED_ASSIGNMENT(s.process_nested_nodes, true);
        new_lhs = ctx.Clone(assign_stmt->lhs);
      }

      auto* new_assign_stmt = b.Assign(new_lhs, ctx.Clone(assign_stmt->rhs));

      // Combine insert_before_stmts + new_assign_stmt + insert_after_stmts into
      // a block and return it
      ast::StatementList stmts = std::move(s.insert_before_stmts);
      stmts.reserve(1 + s.insert_after_stmts.size());
      stmts.emplace_back(new_assign_stmt);
      stmts.insert(stmts.end(), s.insert_after_stmts.begin(),
                   s.insert_after_stmts.end());

      return b.Block(std::move(stmts));
    });

    ctx.ReplaceAll([&](const ast::IndexAccessorExpression* index_access)
                       -> const ast::Expression* {
      if (!s.process_nested_nodes) {
        return nullptr;
      }

      // Indexing a member access expr?
      auto* mem_access =
          index_access->object->As<ast::MemberAccessorExpression>();
      if (!mem_access) {
        return nullptr;
      }

      // Process any nested IndexAccessorExpressions
      mem_access = ctx.Clone(mem_access);

      // Store the address of the member access into a let as we need to read
      // the value twice e.g. let tint_symbol = &(s.a1);
      auto mem_access_ptr = b.Sym();
      s.insert_before_stmts.push_back(
          b.Decl(b.Let(mem_access_ptr, nullptr, b.AddressOf(mem_access))));

      // Disable further transforms when cloning
      TINT_SCOPED_ASSIGNMENT(s.process_nested_nodes, false);

      // Copy entire array out of struct into local temp var
      // e.g. var tint_symbol_1 = *(tint_symbol);
      auto tmp_var = b.Sym();
      s.insert_before_stmts.push_back(
          b.Decl(b.Var(tmp_var, nullptr, b.Deref(mem_access_ptr))));

      // Replace input index_access with a clone of itself, but with its
      // .object replaced by the new temp var. This is returned from this
      // function to modify the original assignment statement. e.g.
      // tint_symbol_1[uniforms.i]
      auto* new_index_access =
          b.IndexAccessor(tmp_var, ctx.Clone(index_access->index));

      // Assign temp var back to array
      // e.g. *(tint_symbol) = tint_symbol_1;
      auto* assign_rhs_to_temp = b.Assign(b.Deref(mem_access_ptr), tmp_var);
      s.insert_after_stmts.insert(s.insert_after_stmts.begin(),
                                  assign_rhs_to_temp);  // push_front

      return new_index_access;
    });

    ctx.Clone();
  }
};

LocalizeStructArrayAssignment::LocalizeStructArrayAssignment() = default;

LocalizeStructArrayAssignment::~LocalizeStructArrayAssignment() = default;

void LocalizeStructArrayAssignment::Run(CloneContext& ctx,
                                        const DataMap&,
                                        DataMap&) const {
  State state(ctx);
  state.Run();
}

}  // namespace tint::transform
