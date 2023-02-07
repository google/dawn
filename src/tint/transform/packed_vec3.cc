// Copyright 2022 The Tint Authors.
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

#include "src/tint/transform/packed_vec3.h"

#include <algorithm>
#include <string>
#include <utility>

#include "src/tint/program_builder.h"
#include "src/tint/sem/index_accessor_expression.h"
#include "src/tint/sem/member_accessor_expression.h"
#include "src/tint/sem/statement.h"
#include "src/tint/sem/variable.h"
#include "src/tint/utils/hashmap.h"
#include "src/tint/utils/hashset.h"

TINT_INSTANTIATE_TYPEINFO(tint::transform::PackedVec3);
TINT_INSTANTIATE_TYPEINFO(tint::transform::PackedVec3::Attribute);

using namespace tint::number_suffixes;  // NOLINT

namespace tint::transform {

/// PIMPL state for the transform
struct PackedVec3::State {
    /// Constructor
    /// @param program the source program
    explicit State(const Program* program) : src(program) {}

    /// Runs the transform
    /// @returns the new program or SkipTransform if the transform is not required
    ApplyResult Run() {
        // Packed vec3<T> struct members
        utils::Hashset<const sem::StructMember*, 8> members;

        // Find all the packed vector struct members, and apply the @internal(packed_vector)
        // attribute.
        for (auto* decl : ctx.src->AST().GlobalDeclarations()) {
            if (auto* str = sem.Get<sem::Struct>(decl)) {
                if (str->IsHostShareable()) {
                    for (auto* member : str->Members()) {
                        if (auto* vec = member->Type()->As<type::Vector>()) {
                            if (vec->Width() == 3) {
                                members.Add(member);

                                // Apply the PackedVec3::Attribute to the member
                                ctx.InsertFront(
                                    member->Declaration()->attributes,
                                    b.ASTNodes().Create<Attribute>(b.ID(), b.AllocateNodeID()));
                            }
                        }
                    }
                }
            }
        }

        if (members.IsEmpty()) {
            return SkipTransform;
        }

        // Walk the nodes, starting with the most deeply nested, finding all the AST expressions
        // that load a whole packed vector (not a scalar / swizzle of the vector).
        utils::Hashset<const sem::ValueExpression*, 16> refs;
        for (auto* node : ctx.src->ASTNodes().Objects()) {
            auto* sem_node = sem.Get(node);
            if (sem_node) {
                if (auto* expr = sem_node->As<sem::ValueExpression>()) {
                    sem_node = expr->UnwrapLoad();
                }
            }
            Switch(
                sem_node,  //
                [&](const sem::StructMemberAccess* access) {
                    if (members.Contains(access->Member())) {
                        // Access to a packed vector member. Seed the expression tracking.
                        refs.Add(access);
                    }
                },
                [&](const sem::IndexAccessorExpression* access) {
                    // Not loading a whole packed vector. Ignore.
                    refs.Remove(access->Object()->UnwrapLoad());
                },
                [&](const sem::Swizzle* access) {
                    // Not loading a whole packed vector. Ignore.
                    refs.Remove(access->Object()->UnwrapLoad());
                },
                [&](const sem::VariableUser* user) {
                    auto* v = user->Variable();
                    if (v->Declaration()->Is<ast::Let>() &&  // if variable is let...
                        v->Type()->Is<type::Pointer>() &&    // and let is a pointer...
                        refs.Contains(v->Initializer())) {   // and pointer is to a packed vector...
                        refs.Add(user);  // then propagate tracking to pointer usage
                    }
                },
                [&](const sem::ValueExpression* expr) {
                    if (auto* unary = expr->Declaration()->As<ast::UnaryOpExpression>()) {
                        if (unary->op == ast::UnaryOp::kAddressOf ||
                            unary->op == ast::UnaryOp::kIndirection) {
                            // Memory access on the packed vector. Track these.
                            auto* inner = sem.GetVal(unary->expr);
                            if (refs.Remove(inner)) {
                                refs.Add(expr);
                            }
                        }
                        // Note: non-memory ops (e.g. '-') are ignored, leaving any tracked
                        // reference at the inner expression, so we'd cast, then apply the unary op.
                    }
                },
                [&](const sem::Statement* e) {
                    if (auto* assign = e->Declaration()->As<ast::AssignmentStatement>()) {
                        // We don't want to cast packed_vectors if they're being assigned to.
                        refs.Remove(sem.GetVal(assign->lhs));
                    }
                });
        }

        // Wrap the load expressions with a cast to the unpacked type.
        utils::Hashmap<const type::Vector*, Symbol, 3> unpack_fns;
        for (auto* ref : refs) {
            // ref is either a packed vec3 that needs casting, or a pointer to a vec3 which we just
            // leave alone.
            if (auto* vec_ty = ref->Type()->UnwrapRef()->As<type::Vector>()) {
                auto* expr = ref->Declaration();
                ctx.Replace(expr, [this, vec_ty, expr] {  //
                    auto* packed = ctx.CloneWithoutTransform(expr);
                    return b.Call(CreateASTTypeFor(ctx, vec_ty), packed);
                });
            }
        }

        ctx.Clone();
        return Program(std::move(b));
    }

  private:
    /// The source program
    const Program* const src;
    /// The target program builder
    ProgramBuilder b;
    /// The clone context
    CloneContext ctx = {&b, src, /* auto_clone_symbols */ true};
    /// Alias to the semantic info in ctx.src
    const sem::Info& sem = ctx.src->Sem();
    /// Alias to the symbols in ctx.src
    const SymbolTable& sym = ctx.src->Symbols();
};

PackedVec3::Attribute::Attribute(ProgramID pid, ast::NodeID nid) : Base(pid, nid) {}
PackedVec3::Attribute::~Attribute() = default;

const PackedVec3::Attribute* PackedVec3::Attribute::Clone(CloneContext* ctx) const {
    return ctx->dst->ASTNodes().Create<Attribute>(ctx->dst->ID(), ctx->dst->AllocateNodeID());
}

std::string PackedVec3::Attribute::InternalName() const {
    return "packed_vector";
}

PackedVec3::PackedVec3() = default;
PackedVec3::~PackedVec3() = default;

Transform::ApplyResult PackedVec3::Apply(const Program* src, const DataMap&, DataMap&) const {
    return State{src}.Run();
}

}  // namespace tint::transform
