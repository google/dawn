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

#include "src/tint/ir/block.h"
#include "src/tint/ir/call.h"
#include "src/tint/ir/constant.h"
#include "src/tint/ir/if.h"
#include "src/tint/ir/instruction.h"
#include "src/tint/ir/load.h"
#include "src/tint/ir/module.h"
#include "src/tint/ir/return.h"
#include "src/tint/ir/store.h"
#include "src/tint/ir/switch.h"
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

        auto name = NameOf(fn);
        // TODO(crbug.com/tint/1915): Properly implement this when we've fleshed out Function
        utils::Vector<const ast::Parameter*, 1> params{};
        auto ret_ty = Type(fn->ReturnType());
        if (!ret_ty) {
            return nullptr;
        }
        auto* body = BlockGraph(fn->StartTarget());
        if (!body) {
            return nullptr;
        }
        utils::Vector<const ast::Attribute*, 1> attrs{};
        utils::Vector<const ast::Attribute*, 1> ret_attrs{};
        return b.Func(name, std::move(params), ret_ty.Get(), body, std::move(attrs),
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

            enum Status { kContinue, kStop, kError };

            Status status = tint::Switch(
                block,

                [&](const ir::Block* blk) {
                    for (auto* inst : blk->Instructions()) {
                        auto stmt = Stmt(inst);
                        if (TINT_UNLIKELY(!stmt)) {
                            return kError;
                        }
                        if (auto* s = stmt.Get()) {
                            stmts.Push(s);
                        }
                    }
                    if (auto* if_ = blk->Branch()->As<ir::If>()) {
                        if (if_->Merge()->HasBranchTarget()) {
                            block = if_->Merge();
                            return kContinue;
                        }
                    } else if (auto* switch_ = blk->Branch()->As<ir::Switch>()) {
                        if (switch_->Merge()->HasBranchTarget()) {
                            block = switch_->Merge();
                            return kContinue;
                        }
                    }
                    return kStop;
                },

                [&](Default) {
                    UNHANDLED_CASE(block);
                    return kError;
                });

            if (TINT_UNLIKELY(status == kError)) {
                return nullptr;
            }
            if (status == kStop) {
                break;
            }
        }

        return b.Block(std::move(stmts));
    }

    const ast::IfStatement* If(const ir::If* i) {
        SCOPED_NESTING();
        auto* cond = Expr(i->Condition());
        auto* t = BlockGraph(i->True());
        if (TINT_UNLIKELY(!t)) {
            return nullptr;
        }

        if (!IsEmpty(i->False(), i->Merge())) {
            // If the else target is an `if` which has a merge target that just bounces to the outer
            // if merge target then emit an 'else if' instead of a block statement for the else.
            if (auto* inst = i->False()->Instructions().Front()->As<ir::If>();
                inst && inst->Merge()->IsTrampoline(i->Merge())) {
                auto* f = If(inst);
                if (!f) {
                    return nullptr;
                }
                return b.If(cond, t, b.Else(f));
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

    utils::Result<const ast::ReturnStatement*> Return(const ir::Return* ret) {
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
            return utils::Failure;
        }

        auto* val = Expr(ret->Args().Front());
        if (TINT_UNLIKELY(!val)) {
            return utils::Failure;
        }

        return b.Return(val);
    }

    /// @return true if there are no instructions between @p node and and @p stop_at
    bool IsEmpty(const ir::Block* node, const ir::Block* stop_at) {
        if (node->Instructions().IsEmpty()) {
            return true;
        }
        if (auto* br = node->Instructions().Front()->As<Branch>()) {
            return !br->Is<ir::Return>() && br->To() == stop_at;
        }
        return false;
    }

    utils::Result<const ast::Statement*> Stmt(const ir::Instruction* inst) {
        return tint::Switch<utils::Result<const ast::Statement*>>(
            inst,                                            //
            [&](const ir::Call* i) { return CallStmt(i); },  //
            [&](const ir::Var* i) { return Var(i); },        //
            [&](const ir::Load*) { return nullptr; },
            [&](const ir::Store* i) { return Store(i); },  //
            [&](const ir::If* if_) { return If(if_); },
            [&](const ir::Switch* switch_) { return Switch(switch_); },
            [&](const ir::Return* ret) { return Return(ret); },
            // TODO(dsinclair): Remove when branch is only a parent ...
            [&](const ir::Branch*) { return utils::Result<const ast::Statement*>{nullptr}; },
            [&](Default) {
                UNHANDLED_CASE(inst);
                return utils::Failure;
            });
    }

    const ast::CallStatement* CallStmt(const ir::Call* call) {
        auto* expr = Call(call);
        if (!expr) {
            return nullptr;
        }
        return b.CallStmt(expr);
    }

    const ast::VariableDeclStatement* Var(const ir::Var* var) {
        Symbol name = NameOf(var);
        auto* ptr = var->Type()->As<type::Pointer>();
        if (!ptr) {
            Err("Incorrect type for var");
            return nullptr;
        }
        auto ty = Type(ptr->StoreType());
        const ast::Expression* init = nullptr;
        if (var->Initializer()) {
            init = Expr(var->Initializer());
            if (!init) {
                return nullptr;
            }
        }
        switch (ptr->AddressSpace()) {
            case builtin::AddressSpace::kFunction:
                return b.Decl(b.Var(name, ty.Get(), init));
            case builtin::AddressSpace::kStorage:
                return b.Decl(b.Var(name, ty.Get(), init, ptr->Access(), ptr->AddressSpace()));
            default:
                return b.Decl(b.Var(name, ty.Get(), init, ptr->AddressSpace()));
        }
    }

    const ast::AssignmentStatement* Store(const ir::Store* store) {
        auto* expr = Expr(store->From());
        return b.Assign(NameOf(store->To()), expr);
    }

    const ast::CallExpression* Call(const ir::Call* call) {
        auto args =
            utils::Transform<2>(call->Args(), [&](const ir::Value* arg) { return Expr(arg); });
        if (args.Any(utils::IsNull)) {
            return nullptr;
        }
        return tint::Switch(
            call,  //
            [&](const ir::UserCall* c) { return b.Call(NameOf(c->Func()), std::move(args)); },
            [&](Default) {
                UNHANDLED_CASE(call);
                return nullptr;
            });
    }

    const ast::Expression* Expr(const ir::Value* val) {
        return tint::Switch(
            val,  //
            [&](const ir::Constant* c) { return ConstExpr(c); },
            [&](const ir::Load* l) { return LoadExpr(l); },
            [&](const ir::Var* v) { return VarExpr(v); },
            [&](Default) {
                UNHANDLED_CASE(val);
                return nullptr;
            });
    }

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
                return nullptr;
            });
    }

    const ast::Expression* LoadExpr(const ir::Load* l) { return Expr(l->From()); }

    const ast::Expression* VarExpr(const ir::Var* v) { return b.Expr(NameOf(v)); }

    utils::Result<ast::Type> Type(const type::Type* ty) {
        return tint::Switch<utils::Result<ast::Type>>(
            ty,                                              //
            [&](const type::Void*) { return ast::Type{}; },  //
            [&](const type::I32*) { return b.ty.i32(); },    //
            [&](const type::U32*) { return b.ty.u32(); },    //
            [&](const type::F16*) { return b.ty.f16(); },    //
            [&](const type::F32*) { return b.ty.f32(); },    //
            [&](const type::Bool*) { return b.ty.bool_(); },
            [&](const type::Matrix* m) -> utils::Result<ast::Type> {
                auto el = Type(m->type());
                if (!el) {
                    return utils::Failure;
                }
                return b.ty.mat(el.Get(), m->columns(), m->rows());
            },
            [&](const type::Vector* v) -> utils::Result<ast::Type> {
                auto el = Type(v->type());
                if (!el) {
                    return utils::Failure;
                }
                if (v->Packed()) {
                    TINT_ASSERT(IR, v->Width() == 3u);
                    return b.ty(builtin::Builtin::kPackedVec3, el.Get());
                } else {
                    return b.ty.vec(el.Get(), v->Width());
                }
            },
            [&](const type::Array* a) -> utils::Result<ast::Type> {
                auto el = Type(a->ElemType());
                if (!el) {
                    return utils::Failure;
                }
                utils::Vector<const ast::Attribute*, 1> attrs;
                if (!a->IsStrideImplicit()) {
                    attrs.Push(b.Stride(a->Stride()));
                }
                if (a->Count()->Is<type::RuntimeArrayCount>()) {
                    return b.ty.array(el.Get(), std::move(attrs));
                }
                auto count = a->ConstantCount();
                if (TINT_UNLIKELY(!count)) {
                    TINT_ICE(IR, b.Diagnostics()) << type::Array::kErrExpectedConstantCount;
                    return b.ty.array(el.Get(), u32(1), std::move(attrs));
                }
                return b.ty.array(el.Get(), u32(count.value()), std::move(attrs));
            },
            [&](const type::Struct* s) { return b.ty(s->Name().NameView()); },
            [&](const type::Atomic* a) -> utils::Result<ast::Type> {
                auto el = Type(a->Type());
                if (!el) {
                    return utils::Failure;
                }
                return b.ty.atomic(el.Get());
            },
            [&](const type::DepthTexture* t) { return b.ty.depth_texture(t->dim()); },
            [&](const type::DepthMultisampledTexture* t) {
                return b.ty.depth_multisampled_texture(t->dim());
            },
            [&](const type::ExternalTexture*) { return b.ty.external_texture(); },
            [&](const type::MultisampledTexture* t) -> utils::Result<ast::Type> {
                auto el = Type(t->type());
                if (!el) {
                    return utils::Failure;
                }
                return b.ty.multisampled_texture(t->dim(), el.Get());
            },
            [&](const type::SampledTexture* t) -> utils::Result<ast::Type> {
                auto el = Type(t->type());
                if (!el) {
                    return utils::Failure;
                }
                return b.ty.sampled_texture(t->dim(), el.Get());
            },
            [&](const type::StorageTexture* t) {
                return b.ty.storage_texture(t->dim(), t->texel_format(), t->access());
            },
            [&](const type::Sampler* s) { return b.ty.sampler(s->kind()); },
            [&](const type::Pointer* p) -> utils::Result<ast::Type> {
                // Note: type::Pointer always has an inferred access, but WGSL only allows an
                // explicit access in the 'storage' address space.
                auto el = Type(p->StoreType());
                if (!el) {
                    return utils::Failure;
                }
                auto address_space = p->AddressSpace();
                auto access = address_space == builtin::AddressSpace::kStorage
                                  ? p->Access()
                                  : builtin::Access::kUndefined;
                return b.ty.pointer(el.Get(), address_space, access);
            },
            [&](const type::Reference*) -> utils::Result<ast::Type> {
                TINT_ICE(IR, b.Diagnostics()) << "reference types should never appear in the IR";
                return ast::Type{};
            },
            [&](Default) {
                UNHANDLED_CASE(ty);
                return ast::Type{};
            });
    }

    Symbol NameOf(const Value* value) {
        TINT_ASSERT(IR, value);
        return value_names_.GetOrCreate(value, [&] {
            if (auto sym = mod.NameOf(value)) {
                return b.Symbols().New(sym.Name());
            }
            return b.Symbols().New("v" + std::to_string(value_names_.Count()));
        });
    }

    void Err(std::string str) { b.Diagnostics().add_error(diag::System::IR, std::move(str)); }
};

}  // namespace

Program ToProgram(const Module& i) {
    return State{i}.Run();
}

}  // namespace tint::ir
