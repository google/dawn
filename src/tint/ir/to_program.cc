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

#include "src/tint/ir/to_program.h"

#include <string>
#include <utility>

#include "src/tint/ir/binary.h"
#include "src/tint/ir/block.h"
#include "src/tint/ir/call.h"
#include "src/tint/ir/constant.h"
#include "src/tint/ir/exit_if.h"
#include "src/tint/ir/if.h"
#include "src/tint/ir/instruction.h"
#include "src/tint/ir/load.h"
#include "src/tint/ir/module.h"
#include "src/tint/ir/return.h"
#include "src/tint/ir/store.h"
#include "src/tint/ir/switch.h"
#include "src/tint/ir/unary.h"
#include "src/tint/ir/user_call.h"
#include "src/tint/ir/var.h"
#include "src/tint/program_builder.h"
#include "src/tint/switch.h"
#include "src/tint/type/atomic.h"
#include "src/tint/type/depth_multisampled_texture.h"
#include "src/tint/type/depth_texture.h"
#include "src/tint/type/multisampled_texture.h"
#include "src/tint/type/pointer.h"
#include "src/tint/type/reference.h"
#include "src/tint/type/sampler.h"
#include "src/tint/type/texture.h"
#include "src/tint/utils/hashmap.h"
#include "src/tint/utils/predicates.h"
#include "src/tint/utils/transform.h"
#include "src/tint/utils/vector.h"

// Helper for calling TINT_UNIMPLEMENTED() from a Switch(object_ptr) default case.
#define UNHANDLED_CASE(object_ptr)          \
    TINT_UNIMPLEMENTED(IR, b.Diagnostics()) \
        << "unhandled case in Switch(): " << (object_ptr ? object_ptr->TypeInfo().name : "<null>")

// Helper for incrementing nesting_depth_ and then decrementing nesting_depth_ at the end
// of the scope that holds the call.
#define SCOPED_NESTING() \
    nesting_depth_++;    \
    TINT_DEFER(nesting_depth_--)

namespace tint::ir {

namespace {

class State {
  public:
    explicit State(const Module& m) : mod(m) {}

    Program Run() {
        // TODO(crbug.com/tint/1902): Emit root block
        // TODO(crbug.com/tint/1902): Emit user-declared types
        for (auto* fn : mod.functions) {
            Fn(fn);
        }
        return Program{std::move(b)};
    }

  private:
    /// The source IR module
    const Module& mod;

    /// The target ProgramBuilder
    ProgramBuilder b;

    /// A hashmap of value to symbol used in the emitted AST
    utils::Hashmap<const Value*, Symbol, 32> value_names_;

    // The nesting depth of the currently generated AST
    // 0 is module scope
    // 1 is root-level function scope
    // 2+ is within control flow
    uint32_t nesting_depth_ = 0;

    const ast::Function* Fn(const Function* fn) {
        SCOPED_NESTING();

        // TODO(crbug.com/tint/1915): Properly implement this when we've fleshed out Function
        static constexpr size_t N = decltype(ast::Function::params)::static_length;
        auto params = utils::Transform<N>(fn->Params(), [&](const ir::FunctionParam* param) {
            auto name = AssignNameTo(param);
            auto ty = Type(param->Type());
            return b.Param(name, ty);
        });

        auto name = AssignNameTo(fn);
        auto ret_ty = Type(fn->ReturnType());
        auto* body = BlockGraph(fn->StartTarget());
        utils::Vector<const ast::Attribute*, 1> attrs{};
        utils::Vector<const ast::Attribute*, 1> ret_attrs{};
        return b.Func(name, std::move(params), ret_ty, body, std::move(attrs),
                      std::move(ret_attrs));
    }

    const ast::BlockStatement* BlockGraph(const ir::Block* start_node) {
        // TODO(crbug.com/tint/1902): Check if the block is dead
        utils::Vector<const ast::Statement*,
                      decltype(ast::BlockStatement::statements)::static_length>
            stmts;

        const ir::Block* block = start_node;

        // TODO(crbug.com/tint/1902): Handle block arguments.

        while (block) {
            TINT_ASSERT(IR, block->HasBranchTarget());

            for (auto* inst : *block) {
                if (auto* stmt = Stmt(inst)) {
                    stmts.Push(stmt);
                }
            }
            if (auto* if_ = block->Branch()->As<ir::If>()) {
                if (if_->Merge()->HasBranchTarget()) {
                    block = if_->Merge();
                    continue;
                }
            } else if (auto* switch_ = block->Branch()->As<ir::Switch>()) {
                if (switch_->Merge()->HasBranchTarget()) {
                    block = switch_->Merge();
                    continue;
                }
            }
            break;
        }

        return b.Block(std::move(stmts));
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Statements
    //
    // Statement methods may return nullptr, in the case of instructions that do not map to an AST
    // statement, or in the case of an error. These should simply be ignored.
    ////////////////////////////////////////////////////////////////////////////////////////////////

    /// @param inst the ir::Instruction
    /// @return an ast::Statement from @p inst, or nullptr if there was an error
    const ast::Statement* Stmt(const ir::Instruction* inst) {
        return tint::Switch(
            inst,                                                        //
            [&](const ir::Store* i) { return Store(i); },                //
            [&](const ir::Call* i) { return CallStmt(i); },              //
            [&](const ir::Var* i) { return Var(i); },                    //
            [&](const ir::If* if_) { return If(if_); },                  //
            [&](const ir::Switch* switch_) { return Switch(switch_); },  //
            [&](const ir::Return* ret) { return Return(ret); },          //
            [&](const ir::Value*) { return ValueStmt(inst); },
            // TODO(dsinclair): Remove when branch is only a parent ...
            [&](const ir::Branch*) { return nullptr; },
            [&](Default) {
                UNHANDLED_CASE(inst);
                return nullptr;
            });
    }

    /// @param i the ir::If
    /// @return an ast::IfStatement from @p i, or nullptr if there was an error
    const ast::IfStatement* If(const ir::If* i) {
        SCOPED_NESTING();
        auto* cond = Expr(i->Condition());
        auto* t = BlockGraph(i->True());
        if (TINT_UNLIKELY(!t)) {
            return nullptr;
        }

        auto* false_blk = i->False();
        if (false_blk->Length() > 1 || (false_blk->Length() == 1 && false_blk->HasBranchTarget() &&
                                        !false_blk->Branch()->Is<ir::ExitIf>())) {
            // If the else target is an `if` which has a merge target that just bounces to the outer
            // if merge target then emit an 'else if' instead of a block statement for the else.
            if (auto* inst = i->False()->Instructions(); inst && inst->As<ir::If>()) {
                auto* if_ = inst->As<ir::If>();
                if (auto* br = if_->Merge()->Branch()->As<ir::ExitIf>(); br && br->If() == i) {
                    auto* f = If(if_);
                    if (!f) {
                        return nullptr;
                    }
                    return b.If(cond, t, b.Else(f));
                }
            } else {
                auto* f = BlockGraph(i->False());
                if (!f) {
                    return nullptr;
                }
                return b.If(cond, t, b.Else(f));
            }
        }
        return b.If(cond, t);
    }

    /// @param s the ir::Switch
    /// @return an ast::SwitchStatement from @p s, or nullptr if there was an error
    const ast::SwitchStatement* Switch(const ir::Switch* s) {
        SCOPED_NESTING();

        auto* cond = Expr(s->Condition());
        if (!cond) {
            return nullptr;
        }

        auto cases = utils::Transform<2>(
            s->Cases(),  //
            [&](const ir::Switch::Case c) -> const tint::ast::CaseStatement* {
                SCOPED_NESTING();
                auto* body = BlockGraph(c.start);
                if (!body) {
                    return nullptr;
                }

                auto selectors = utils::Transform(
                    c.selectors,  //
                    [&](const ir::Switch::CaseSelector& cs) -> const ast::CaseSelector* {
                        if (cs.IsDefault()) {
                            return b.DefaultCaseSelector();
                        }
                        auto* expr = Expr(cs.val);
                        if (!expr) {
                            return nullptr;
                        }
                        return b.CaseSelector(expr);
                    });
                if (selectors.Any(utils::IsNull)) {
                    return nullptr;
                }

                return b.Case(std::move(selectors), body);
            });
        if (cases.Any(utils::IsNull)) {
            return nullptr;
        }

        return b.Switch(cond, std::move(cases));
    }

    /// @param ret the ir::Return
    /// @return an ast::ReturnStatement from @p ret, or nullptr if there was an error
    const ast::ReturnStatement* Return(const ir::Return* ret) {
        if (ret->Args().IsEmpty()) {
            // Return has no arguments.
            // If this block is nested withing some control flow, then we must
            // emit a 'return' statement, otherwise we've just naturally reached
            // the end of the function where the 'return' is redundant.
            if (nesting_depth_ > 1) {
                return b.Return();
            }
            return nullptr;
        }

        // Return has arguments - this is the return value.
        if (ret->Args().Length() != 1) {
            TINT_ICE(IR, b.Diagnostics())
                << "expected 1 value for return, got " << ret->Args().Length();
            return b.Return();
        }

        auto* val = Expr(ret->Args().Front());
        if (TINT_UNLIKELY(!val)) {
            return b.Return();
        }

        return b.Return(val);
    }

    /// @param call the ir::Call
    /// @return an ast::CallStatement from @p call, or nullptr if there was an error
    const ast::CallStatement* CallStmt(const ir::Call* call) { return b.CallStmt(Call(call)); }

    /// @param var the ir::Var
    /// @return an ast::VariableDeclStatement from @p var
    const ast::VariableDeclStatement* Var(const ir::Var* var) {
        Symbol name = AssignNameTo(var);
        auto* ptr = var->Type()->As<type::Pointer>();
        auto ty = Type(ptr->StoreType());
        const ast::Expression* init = nullptr;
        if (var->Initializer()) {
            init = Expr(var->Initializer());
        }
        switch (ptr->AddressSpace()) {
            case builtin::AddressSpace::kFunction:
                return b.Decl(b.Var(name, ty, init));
            case builtin::AddressSpace::kStorage:
                return b.Decl(b.Var(name, ty, init, ptr->Access(), ptr->AddressSpace()));
            default:
                return b.Decl(b.Var(name, ty, init, ptr->AddressSpace()));
        }
    }

    /// @param store the ir::Store
    /// @return an ast::AssignmentStatement from @p call
    const ast::AssignmentStatement* Store(const ir::Store* store) {
        auto* expr = Expr(store->From());
        return b.Assign(AssignNameTo(store->To()), expr);
    }

    /// @param val the ir::Value
    /// @return an ast::Statement from @p val, or nullptr if the value does not produce a statement.
    const ast::Statement* ValueStmt(const ir::Value* val) {
        // As we're visiting this value's declaration it shouldn't already have a name reserved.
        TINT_ASSERT(IR, !value_names_.Contains(val));

        // Determine whether the value should be placed into a let, or inlined in its single place
        // of usage. Currently a value is inlined if it has a single usage and is unnamed.
        // TODO(crbug.com/tint/1902): This logic needs to check that the sequence of side-effecting
        // expressions is not changed by inlining the expression. This needs fixing.
        bool create_let = val->Usages().Count() > 1 || mod.NameOf(val).IsValid();
        if (create_let) {
            auto* init = Expr(val);  // Must come before giving the value a name
            auto name = AssignNameTo(val);
            return b.Decl(b.Let(name, init));
        }
        return nullptr;  // Value will be inlined at its place of usage.
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Expressions
    //
    // The the case of an error:
    // * The expression generating methods must return a non-null ast expression pointer, which may
    //   not be semantically legal, but is enough to populate the AST.
    // * A diagnostic error must be added to the ast::ProgramBuilder.
    // This prevents littering the ToProgram logic with expensive error checking code.
    ////////////////////////////////////////////////////////////////////////////////////////////////

    /// @param val the ir::Expression
    /// @return an ast::Expression from @p val.
    /// @note May be a semantically-invalid placeholder expression on error.
    const ast::Expression* Expr(const ir::Value* val) {
        if (auto name = value_names_.Get(val)) {
            return b.Expr(name.value());
        }

        return tint::Switch(
            val,  //
            [&](const ir::Constant* c) { return ConstExpr(c); },
            [&](const ir::Load* l) { return LoadExpr(l); },
            [&](const ir::Unary* u) { return UnaryExpr(u); },
            [&](const ir::Binary* u) { return BinaryExpr(u); },
            [&](Default) {
                UNHANDLED_CASE(val);
                return b.Expr("<error>");
            });
    }

    /// @param call the ir::Call
    /// @return an ast::CallExpression from @p call.
    /// @note May be a semantically-invalid placeholder expression on error.
    const ast::CallExpression* Call(const ir::Call* call) {
        auto args =
            utils::Transform<2>(call->Args(), [&](const ir::Value* arg) { return Expr(arg); });
        return tint::Switch(
            call,  //
            [&](const ir::UserCall* c) { return b.Call(AssignNameTo(c->Func()), std::move(args)); },
            [&](Default) {
                UNHANDLED_CASE(call);
                return b.Call("<error>");
            });
    }

    /// @param c the ir::Constant
    /// @return an ast::Expression from @p c.
    /// @note May be a semantically-invalid placeholder expression on error.
    const ast::Expression* ConstExpr(const ir::Constant* c) {
        return tint::Switch(
            c->Type(),  //
            [&](const type::I32*) { return b.Expr(c->Value()->ValueAs<i32>()); },
            [&](const type::U32*) { return b.Expr(c->Value()->ValueAs<u32>()); },
            [&](const type::F32*) { return b.Expr(c->Value()->ValueAs<f32>()); },
            [&](const type::F16*) { return b.Expr(c->Value()->ValueAs<f16>()); },
            [&](const type::Bool*) { return b.Expr(c->Value()->ValueAs<bool>()); },
            [&](Default) {
                UNHANDLED_CASE(c);
                return b.Expr("<error>");
            });
    }

    /// @param l the ir::Load
    /// @return an ast::Expression from @p l.
    /// @note May be a semantically-invalid placeholder expression on error.
    const ast::Expression* LoadExpr(const ir::Load* l) { return Expr(l->From()); }

    /// @param u the ir::Unary
    /// @return an ast::UnaryOpExpression from @p u.
    /// @note May be a semantically-invalid placeholder expression on error.
    const ast::Expression* UnaryExpr(const ir::Unary* u) {
        switch (u->Kind()) {
            case ir::Unary::Kind::kComplement:
                return b.Complement(Expr(u->Val()));
            case ir::Unary::Kind::kNegation:
                return b.Negation(Expr(u->Val()));
        }
        return b.Expr("<error>");
    }

    /// @param e the ir::Binary
    /// @return an ast::BinaryOpExpression from @p e.
    /// @note May be a semantically-invalid placeholder expression on error.
    const ast::Expression* BinaryExpr(const ir::Binary* e) {
        if (e->Kind() == ir::Binary::Kind::kEqual) {
            auto* rhs = e->RHS()->As<ir::Constant>();
            if (rhs && rhs->Type()->Is<type::Bool>() && rhs->Value()->ValueAs<bool>() == false) {
                // expr == false
                return b.Not(Expr(e->LHS()));
            }
        }
        auto* lhs = Expr(e->LHS());
        auto* rhs = Expr(e->RHS());
        switch (e->Kind()) {
            case ir::Binary::Kind::kAdd:
                return b.Add(lhs, rhs);
            case ir::Binary::Kind::kSubtract:
                return b.Sub(lhs, rhs);
            case ir::Binary::Kind::kMultiply:
                return b.Mul(lhs, rhs);
            case ir::Binary::Kind::kDivide:
                return b.Div(lhs, rhs);
            case ir::Binary::Kind::kModulo:
                return b.Mod(lhs, rhs);
            case ir::Binary::Kind::kAnd:
                return b.And(lhs, rhs);
            case ir::Binary::Kind::kOr:
                return b.Or(lhs, rhs);
            case ir::Binary::Kind::kXor:
                return b.Xor(lhs, rhs);
            case ir::Binary::Kind::kEqual:
                return b.Equal(lhs, rhs);
            case ir::Binary::Kind::kNotEqual:
                return b.NotEqual(lhs, rhs);
            case ir::Binary::Kind::kLessThan:
                return b.LessThan(lhs, rhs);
            case ir::Binary::Kind::kGreaterThan:
                return b.GreaterThan(lhs, rhs);
            case ir::Binary::Kind::kLessThanEqual:
                return b.LessThanEqual(lhs, rhs);
            case ir::Binary::Kind::kGreaterThanEqual:
                return b.GreaterThanEqual(lhs, rhs);
            case ir::Binary::Kind::kShiftLeft:
                return b.Shl(lhs, rhs);
            case ir::Binary::Kind::kShiftRight:
                return b.Shr(lhs, rhs);
        }
        return b.Expr("<error>");
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Types
    //
    // The the case of an error:
    // * The types generating methods must return a non-null ast type, which may not be semantically
    //   legal, but is enough to populate the AST.
    // * A diagnostic error must be added to the ast::ProgramBuilder.
    // This prevents littering the ToProgram logic with expensive error checking code.
    ////////////////////////////////////////////////////////////////////////////////////////////////

    /// @param ty the type::Type
    /// @return an ast::Type from @p ty.
    /// @note May be a semantically-invalid placeholder type on error.
    ast::Type Type(const type::Type* ty) {
        return tint::Switch(
            ty,                                              //
            [&](const type::Void*) { return ast::Type{}; },  //
            [&](const type::I32*) { return b.ty.i32(); },    //
            [&](const type::U32*) { return b.ty.u32(); },    //
            [&](const type::F16*) { return b.ty.f16(); },    //
            [&](const type::F32*) { return b.ty.f32(); },    //
            [&](const type::Bool*) { return b.ty.bool_(); },
            [&](const type::Matrix* m) {
                return b.ty.mat(Type(m->type()), m->columns(), m->rows());
            },
            [&](const type::Vector* v) {
                auto el = Type(v->type());
                if (v->Packed()) {
                    TINT_ASSERT(IR, v->Width() == 3u);
                    return b.ty(builtin::Builtin::kPackedVec3, el);
                } else {
                    return b.ty.vec(el, v->Width());
                }
            },
            [&](const type::Array* a) {
                auto el = Type(a->ElemType());
                utils::Vector<const ast::Attribute*, 1> attrs;
                if (!a->IsStrideImplicit()) {
                    attrs.Push(b.Stride(a->Stride()));
                }
                if (a->Count()->Is<type::RuntimeArrayCount>()) {
                    return b.ty.array(el, std::move(attrs));
                }
                auto count = a->ConstantCount();
                if (TINT_UNLIKELY(!count)) {
                    TINT_ICE(IR, b.Diagnostics()) << type::Array::kErrExpectedConstantCount;
                    return b.ty.array(el, u32(1), std::move(attrs));
                }
                return b.ty.array(el, u32(count.value()), std::move(attrs));
            },
            [&](const type::Struct* s) { return b.ty(s->Name().NameView()); },
            [&](const type::Atomic* a) { return b.ty.atomic(Type(a->Type())); },
            [&](const type::DepthTexture* t) { return b.ty.depth_texture(t->dim()); },
            [&](const type::DepthMultisampledTexture* t) {
                return b.ty.depth_multisampled_texture(t->dim());
            },
            [&](const type::ExternalTexture*) { return b.ty.external_texture(); },
            [&](const type::MultisampledTexture* t) {
                auto el = Type(t->type());
                return b.ty.multisampled_texture(t->dim(), el);
            },
            [&](const type::SampledTexture* t) {
                auto el = Type(t->type());
                return b.ty.sampled_texture(t->dim(), el);
            },
            [&](const type::StorageTexture* t) {
                return b.ty.storage_texture(t->dim(), t->texel_format(), t->access());
            },
            [&](const type::Sampler* s) { return b.ty.sampler(s->kind()); },
            [&](const type::Pointer* p) {
                // Note: type::Pointer always has an inferred access, but WGSL only allows an
                // explicit access in the 'storage' address space.
                auto el = Type(p->StoreType());
                auto address_space = p->AddressSpace();
                auto access = address_space == builtin::AddressSpace::kStorage
                                  ? p->Access()
                                  : builtin::Access::kUndefined;
                return b.ty.pointer(el, address_space, access);
            },
            [&](const type::Reference*) {
                TINT_ICE(IR, b.Diagnostics()) << "reference types should never appear in the IR";
                return b.ty.i32();
            },
            [&](Default) {
                UNHANDLED_CASE(ty);
                return b.ty.i32();
            });
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Helpers
    ////////////////////////////////////////////////////////////////////////////////////////////////

    /// Creates and returns a new, unique name for the given value, or returns the previously
    /// created name.
    /// @return the value's name
    Symbol AssignNameTo(const Value* value) {
        TINT_ASSERT(IR, value);
        return value_names_.GetOrCreate(value, [&] {
            if (auto sym = mod.NameOf(value)) {
                return b.Symbols().New(sym.Name());
            }
            return b.Symbols().New("v" + std::to_string(value_names_.Count()));
        });
    }
};

}  // namespace

Program ToProgram(const Module& i) {
    return State{i}.Run();
}

}  // namespace tint::ir
