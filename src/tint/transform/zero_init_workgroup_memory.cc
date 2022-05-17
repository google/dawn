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

#include "src/tint/transform/zero_init_workgroup_memory.h"

#include <algorithm>
#include <map>
#include <unordered_map>
#include <utility>
#include <vector>

#include "src/tint/ast/workgroup_attribute.h"
#include "src/tint/program_builder.h"
#include "src/tint/sem/atomic.h"
#include "src/tint/sem/function.h"
#include "src/tint/sem/variable.h"
#include "src/tint/utils/map.h"
#include "src/tint/utils/unique_vector.h"

TINT_INSTANTIATE_TYPEINFO(tint::transform::ZeroInitWorkgroupMemory);

namespace tint::transform {

/// PIMPL state for the ZeroInitWorkgroupMemory transform
struct ZeroInitWorkgroupMemory::State {
    /// The clone context
    CloneContext& ctx;

    /// An alias to *ctx.dst
    ProgramBuilder& b = *ctx.dst;

    /// The constant size of the workgroup. If 0, then #workgroup_size_expr should
    /// be used instead.
    uint32_t workgroup_size_const = 0;
    /// The size of the workgroup as an expression generator. Use if
    /// #workgroup_size_const is 0.
    std::function<const ast::Expression*()> workgroup_size_expr;

    /// ArrayIndex represents a function on the local invocation index, of
    /// the form: `array_index = (local_invocation_index % modulo) / division`
    struct ArrayIndex {
        /// The RHS of the modulus part of the expression
        uint32_t modulo = 1;
        /// The RHS of the division part of the expression
        uint32_t division = 1;

        /// Equality operator
        /// @param i the ArrayIndex to compare to this ArrayIndex
        /// @returns true if `i` and this ArrayIndex are equal
        bool operator==(const ArrayIndex& i) const {
            return modulo == i.modulo && division == i.division;
        }

        /// Hash function for the ArrayIndex type
        struct Hasher {
            /// @param i the ArrayIndex to calculate a hash for
            /// @returns the hash value for the ArrayIndex `i`
            size_t operator()(const ArrayIndex& i) const {
                return utils::Hash(i.modulo, i.division);
            }
        };
    };

    /// A list of unique ArrayIndex
    using ArrayIndices = utils::UniqueVector<ArrayIndex, ArrayIndex::Hasher>;

    /// Expression holds information about an expression that is being built for a
    /// statement will zero workgroup values.
    struct Expression {
        /// The AST expression node
        const ast::Expression* expr = nullptr;
        /// The number of iterations required to zero the value
        uint32_t num_iterations = 0;
        /// All array indices used by this expression
        ArrayIndices array_indices;
    };

    /// Statement holds information about a statement that will zero workgroup
    /// values.
    struct Statement {
        /// The AST statement node
        const ast::Statement* stmt;
        /// The number of iterations required to zero the value
        uint32_t num_iterations;
        /// All array indices used by this statement
        ArrayIndices array_indices;
    };

    /// All statements that zero workgroup memory
    std::vector<Statement> statements;

    /// A map of ArrayIndex to the name reserved for the `let` declaration of that
    /// index.
    std::unordered_map<ArrayIndex, Symbol, ArrayIndex::Hasher> array_index_names;

    /// Constructor
    /// @param c the CloneContext used for the transform
    explicit State(CloneContext& c) : ctx(c) {}

    /// Run inserts the workgroup memory zero-initialization logic at the top of
    /// the given function
    /// @param fn a compute shader entry point function
    void Run(const ast::Function* fn) {
        auto& sem = ctx.src->Sem();

        CalculateWorkgroupSize(ast::GetAttribute<ast::WorkgroupAttribute>(fn->attributes));

        // Generate a list of statements to zero initialize each of the
        // workgroup storage variables used by `fn`. This will populate #statements.
        auto* func = sem.Get(fn);
        for (auto* var : func->TransitivelyReferencedGlobals()) {
            if (var->StorageClass() == ast::StorageClass::kWorkgroup) {
                BuildZeroingStatements(var->Type()->UnwrapRef(), [&](uint32_t num_values) {
                    auto var_name = ctx.Clone(var->Declaration()->symbol);
                    return Expression{b.Expr(var_name), num_values, ArrayIndices{}};
                });
            }
        }

        if (statements.empty()) {
            return;  // No workgroup variables to initialize.
        }

        // Scan the entry point for an existing local_invocation_index builtin
        // parameter
        std::function<const ast::Expression*()> local_index;
        for (auto* param : fn->params) {
            if (auto* builtin = ast::GetAttribute<ast::BuiltinAttribute>(param->attributes)) {
                if (builtin->builtin == ast::Builtin::kLocalInvocationIndex) {
                    local_index = [=] { return b.Expr(ctx.Clone(param->symbol)); };
                    break;
                }
            }

            if (auto* str = sem.Get(param)->Type()->As<sem::Struct>()) {
                for (auto* member : str->Members()) {
                    if (auto* builtin = ast::GetAttribute<ast::BuiltinAttribute>(
                            member->Declaration()->attributes)) {
                        if (builtin->builtin == ast::Builtin::kLocalInvocationIndex) {
                            local_index = [=] {
                                auto* param_expr = b.Expr(ctx.Clone(param->symbol));
                                auto member_name = ctx.Clone(member->Declaration()->symbol);
                                return b.MemberAccessor(param_expr, member_name);
                            };
                            break;
                        }
                    }
                }
            }
        }
        if (!local_index) {
            // No existing local index parameter. Append one to the entry point.
            auto* param = b.Param(b.Symbols().New("local_invocation_index"), b.ty.u32(),
                                  {b.Builtin(ast::Builtin::kLocalInvocationIndex)});
            ctx.InsertBack(fn->params, param);
            local_index = [=] { return b.Expr(param->symbol); };
        }

        // Take the zeroing statements and bin them by the number of iterations
        // required to zero the workgroup data. We then emit these in blocks,
        // possibly wrapped in if-statements or for-loops.
        std::unordered_map<uint32_t, std::vector<Statement>> stmts_by_num_iterations;
        std::vector<uint32_t> num_sorted_iterations;
        for (auto& s : statements) {
            auto& stmts = stmts_by_num_iterations[s.num_iterations];
            if (stmts.empty()) {
                num_sorted_iterations.emplace_back(s.num_iterations);
            }
            stmts.emplace_back(s);
        }
        std::sort(num_sorted_iterations.begin(), num_sorted_iterations.end());

        // Loop over the statements, grouped by num_iterations.
        for (auto num_iterations : num_sorted_iterations) {
            auto& stmts = stmts_by_num_iterations[num_iterations];

            // Gather all the array indices used by all the statements in the block.
            ArrayIndices array_indices;
            for (auto& s : stmts) {
                for (auto& idx : s.array_indices) {
                    array_indices.add(idx);
                }
            }

            // Determine the block type used to emit these statements.

            if (workgroup_size_const == 0 || num_iterations > workgroup_size_const) {
                // Either the workgroup size is dynamic, or smaller than num_iterations.
                // In either case, we need to generate a for loop to ensure we
                // initialize all the array elements.
                //
                //  for (var idx : u32 = local_index;
                //           idx < num_iterations;
                //           idx += workgroup_size) {
                //    ...
                //  }
                auto idx = b.Symbols().New("idx");
                auto* init = b.Decl(b.Var(idx, b.ty.u32(), local_index()));
                auto* cond = b.create<ast::BinaryExpression>(ast::BinaryOp::kLessThan, b.Expr(idx),
                                                             b.Expr(u32(num_iterations)));
                auto* cont = b.Assign(
                    idx, b.Add(idx, workgroup_size_const ? b.Expr(u32(workgroup_size_const))
                                                         : workgroup_size_expr()));

                auto block =
                    DeclareArrayIndices(num_iterations, array_indices, [&] { return b.Expr(idx); });
                for (auto& s : stmts) {
                    block.emplace_back(s.stmt);
                }
                auto* for_loop = b.For(init, cond, cont, b.Block(block));
                ctx.InsertFront(fn->body->statements, for_loop);
            } else if (num_iterations < workgroup_size_const) {
                // Workgroup size is a known constant, but is greater than
                // num_iterations. Emit an if statement:
                //
                //  if (local_index < num_iterations) {
                //    ...
                //  }
                auto* cond = b.create<ast::BinaryExpression>(
                    ast::BinaryOp::kLessThan, local_index(), b.Expr(u32(num_iterations)));
                auto block = DeclareArrayIndices(num_iterations, array_indices,
                                                 [&] { return b.Expr(local_index()); });
                for (auto& s : stmts) {
                    block.emplace_back(s.stmt);
                }
                auto* if_stmt = b.If(cond, b.Block(block));
                ctx.InsertFront(fn->body->statements, if_stmt);
            } else {
                // Workgroup size exactly equals num_iterations.
                // No need for any conditionals. Just emit a basic block:
                //
                // {
                //    ...
                // }
                auto block = DeclareArrayIndices(num_iterations, array_indices,
                                                 [&] { return b.Expr(local_index()); });
                for (auto& s : stmts) {
                    block.emplace_back(s.stmt);
                }
                ctx.InsertFront(fn->body->statements, b.Block(block));
            }
        }

        // Append a single workgroup barrier after the zero initialization.
        ctx.InsertFront(fn->body->statements, b.CallStmt(b.Call("workgroupBarrier")));
    }

    /// BuildZeroingExpr is a function that builds a sub-expression used to zero
    /// workgroup values. `num_values` is the number of elements that the
    /// expression will be used to zero. Returns the expression.
    using BuildZeroingExpr = std::function<Expression(uint32_t num_values)>;

    /// BuildZeroingStatements() generates the statements required to zero
    /// initialize the workgroup storage expression of type `ty`.
    /// @param ty the expression type
    /// @param get_expr a function that builds the AST nodes for the expression.
    void BuildZeroingStatements(const sem::Type* ty, const BuildZeroingExpr& get_expr) {
        if (CanTriviallyZero(ty)) {
            auto var = get_expr(1u);
            auto* zero_init = b.Construct(CreateASTTypeFor(ctx, ty));
            statements.emplace_back(
                Statement{b.Assign(var.expr, zero_init), var.num_iterations, var.array_indices});
            return;
        }

        if (auto* atomic = ty->As<sem::Atomic>()) {
            auto* zero_init = b.Construct(CreateASTTypeFor(ctx, atomic->Type()));
            auto expr = get_expr(1u);
            auto* store = b.Call("atomicStore", b.AddressOf(expr.expr), zero_init);
            statements.emplace_back(
                Statement{b.CallStmt(store), expr.num_iterations, expr.array_indices});
            return;
        }

        if (auto* str = ty->As<sem::Struct>()) {
            for (auto* member : str->Members()) {
                auto name = ctx.Clone(member->Declaration()->symbol);
                BuildZeroingStatements(member->Type(), [&](uint32_t num_values) {
                    auto s = get_expr(num_values);
                    return Expression{b.MemberAccessor(s.expr, name), s.num_iterations,
                                      s.array_indices};
                });
            }
            return;
        }

        if (auto* arr = ty->As<sem::Array>()) {
            BuildZeroingStatements(arr->ElemType(), [&](uint32_t num_values) {
                // num_values is the number of values to zero for the element type.
                // The number of iterations required to zero the array and its elements
                // is:
                //      `num_values * arr->Count()`
                // The index for this array is:
                //      `(idx % modulo) / division`
                auto modulo = num_values * arr->Count();
                auto division = num_values;
                auto a = get_expr(modulo);
                auto array_indices = a.array_indices;
                array_indices.add(ArrayIndex{modulo, division});
                auto index = utils::GetOrCreate(array_index_names, ArrayIndex{modulo, division},
                                                [&] { return b.Symbols().New("i"); });
                return Expression{b.IndexAccessor(a.expr, index), a.num_iterations, array_indices};
            });
            return;
        }

        TINT_UNREACHABLE(Transform, b.Diagnostics())
            << "could not zero workgroup type: " << ty->FriendlyName(ctx.src->Symbols());
    }

    /// DeclareArrayIndices returns a list of statements that contain the `let`
    /// declarations for all of the ArrayIndices.
    /// @param num_iterations the number of iterations for the block
    /// @param array_indices the list of array indices to generate `let`
    ///        declarations for
    /// @param iteration a function that returns the index of the current
    ///         iteration.
    /// @returns the list of `let` statements that declare the array indices
    ast::StatementList DeclareArrayIndices(
        uint32_t num_iterations,
        const ArrayIndices& array_indices,
        const std::function<const ast::Expression*()>& iteration) {
        ast::StatementList stmts;
        std::map<Symbol, ArrayIndex> indices_by_name;
        for (auto index : array_indices) {
            auto name = array_index_names.at(index);
            auto* mod = (num_iterations > index.modulo)
                            ? b.create<ast::BinaryExpression>(ast::BinaryOp::kModulo, iteration(),
                                                              b.Expr(u32(index.modulo)))
                            : iteration();
            auto* div = (index.division != 1u) ? b.Div(mod, u32(index.division)) : mod;
            auto* decl = b.Decl(b.Let(name, b.ty.u32(), div));
            stmts.emplace_back(decl);
        }
        return stmts;
    }

    /// CalculateWorkgroupSize initializes the members #workgroup_size_const and
    /// #workgroup_size_expr with the linear workgroup size.
    /// @param attr the workgroup attribute applied to the entry point function
    void CalculateWorkgroupSize(const ast::WorkgroupAttribute* attr) {
        bool is_signed = false;
        workgroup_size_const = 1u;
        workgroup_size_expr = nullptr;
        for (auto* expr : attr->Values()) {
            if (!expr) {
                continue;
            }
            auto* sem = ctx.src->Sem().Get(expr);
            if (auto c = sem->ConstantValue()) {
                workgroup_size_const *= c.Element<AInt>(0).value;
                continue;
            }
            // Constant value could not be found. Build expression instead.
            workgroup_size_expr = [this, expr, size = workgroup_size_expr] {
                auto* e = ctx.Clone(expr);
                if (ctx.src->TypeOf(expr)->UnwrapRef()->Is<sem::I32>()) {
                    e = b.Construct<u32>(e);
                }
                return size ? b.Mul(size(), e) : e;
            };
        }
        if (workgroup_size_expr) {
            if (workgroup_size_const != 1) {
                // Fold workgroup_size_const in to workgroup_size_expr
                workgroup_size_expr = [this, is_signed, const_size = workgroup_size_const,
                                       expr_size = workgroup_size_expr] {
                    return is_signed ? b.Mul(expr_size(), i32(const_size))
                                     : b.Mul(expr_size(), u32(const_size));
                };
            }
            // Indicate that workgroup_size_expr should be used instead of the
            // constant.
            workgroup_size_const = 0;
        }
    }

    /// @returns true if a variable with store type `ty` can be efficiently zeroed
    /// by assignment of a type constructor without operands. If
    /// CanTriviallyZero() returns false, then the type needs to be
    /// initialized by decomposing the initialization into multiple
    /// sub-initializations.
    /// @param ty the type to inspect
    bool CanTriviallyZero(const sem::Type* ty) {
        if (ty->Is<sem::Atomic>()) {
            return false;
        }
        if (auto* str = ty->As<sem::Struct>()) {
            for (auto* member : str->Members()) {
                if (!CanTriviallyZero(member->Type())) {
                    return false;
                }
            }
        }
        if (ty->Is<sem::Array>()) {
            return false;
        }
        // True for all other storable types
        return true;
    }
};

ZeroInitWorkgroupMemory::ZeroInitWorkgroupMemory() = default;

ZeroInitWorkgroupMemory::~ZeroInitWorkgroupMemory() = default;

bool ZeroInitWorkgroupMemory::ShouldRun(const Program* program, const DataMap&) const {
    for (auto* decl : program->AST().GlobalDeclarations()) {
        if (auto* var = decl->As<ast::Variable>()) {
            if (var->declared_storage_class == ast::StorageClass::kWorkgroup) {
                return true;
            }
        }
    }
    return false;
}

void ZeroInitWorkgroupMemory::Run(CloneContext& ctx, const DataMap&, DataMap&) const {
    for (auto* fn : ctx.src->AST().Functions()) {
        if (fn->PipelineStage() == ast::PipelineStage::kCompute) {
            State{ctx}.Run(fn);
        }
    }
    ctx.Clone();
}

}  // namespace tint::transform
