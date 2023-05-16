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

#include <utility>

#include "src/tint/ir/block.h"
#include "src/tint/ir/call.h"
#include "src/tint/ir/constant.h"
#include "src/tint/ir/if.h"
#include "src/tint/ir/instruction.h"
#include "src/tint/ir/module.h"
#include "src/tint/ir/store.h"
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
    const Module& mod;
    ProgramBuilder b;
    utils::Hashmap<const Value*, Symbol, 32> value_names_;

    void Fn(const Function* fn) {
        auto name = Sym(fn->name);
        // TODO(crbug.com/tint/1915): Properly implement this when we've fleshed out Function
        utils::Vector<const ast::Parameter*, 1> params{};
        ast::Type ret_ty;
        auto* body = Block(fn->start_target);
        utils::Vector<const ast::Attribute*, 1> attrs{};
        utils::Vector<const ast::Attribute*, 1> ret_attrs{};
        b.Func(name, std::move(params), ret_ty, body, std::move(attrs), std::move(ret_attrs));
    }

    const ast::BlockStatement* Block(const ir::Block* block) {
        // TODO(crbug.com/tint/1902): Check if the block is dead
        utils::Vector<const ast::Statement*, decltype(ir::Block::instructions)::static_length>
            stmts;
        for (auto* inst : block->instructions) {
            auto* stmt = Stmt(inst);
            if (!stmt) {
                return nullptr;
            }
            stmts.Push(stmt);
        }
        return b.Block(std::move(stmts));
    }

    const ast::Statement* FlowNode(const ir::FlowNode* node) {
        // TODO(crbug.com/tint/1902): Check the node is connected
        return Switch(
            node,  //
            [&](const ir::If* i) {
                auto* cond = Expr(i->condition);
                auto* t = Branch(i->true_);
                if (auto* f = Branch(i->false_)) {
                    return b.If(cond, t, b.Else(f));
                }
                // TODO(crbug.com/tint/1902): Emit merge block
                return b.If(cond, t);
            },
            [&](Default) {
                TINT_UNIMPLEMENTED(IR, b.Diagnostics())
                    << "unhandled case in Switch(): " << node->TypeInfo().name;
                return nullptr;
            });
    }

    const ast::BlockStatement* Branch(const ir::Branch& branch) {
        auto* stmt = FlowNode(branch.target);
        if (!stmt) {
            return nullptr;
        }
        if (auto* block = stmt->As<ast::BlockStatement>()) {
            return block;
        }
        return b.Block(stmt);
    }

    const ast::Statement* Stmt(const ir::Instruction* inst) {
        return Switch(
            inst,                                            //
            [&](const ir::Call* i) { return CallStmt(i); },  //
            [&](const ir::Var* i) { return Var(i); },        //
            [&](const ir::Store* i) { return Store(i); },
            [&](Default) {
                TINT_UNIMPLEMENTED(IR, b.Diagnostics())
                    << "unhandled case in Switch(): " << inst->TypeInfo().name;
                return nullptr;
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
        auto ty = Type(var->Type());
        const ast::Expression* init = nullptr;
        if (var->initializer) {
            init = Expr(var->initializer);
            if (!init) {
                return nullptr;
            }
        }
        switch (var->address_space) {
            case builtin::AddressSpace::kFunction:
                return b.Decl(b.Var(name, ty, init));
            case builtin::AddressSpace::kStorage:
                return b.Decl(b.Var(name, ty, init, var->access, var->address_space));
            default:
                return b.Decl(b.Var(name, ty, init, var->address_space));
        }
    }

    const ast::AssignmentStatement* Store(const ir::Store* store) {
        auto* expr = Expr(store->from);
        return b.Assign(NameOf(store->to), expr);
    }

    const ast::CallExpression* Call(const ir::Call* call) {
        auto args = utils::Transform(call->args, [&](const ir::Value* arg) { return Expr(arg); });
        if (args.Any(utils::IsNull)) {
            return nullptr;
        }
        return Switch(
            call,  //
            [&](const ir::UserCall* c) { return b.Call(Sym(c->name), std::move(args)); },
            [&](Default) {
                TINT_UNIMPLEMENTED(IR, b.Diagnostics())
                    << "unhandled case in Switch(): " << call->TypeInfo().name;
                return nullptr;
            });
    }

    const ast::Expression* Expr(const ir::Value* val) {
        return Switch(
            val,  //
            [&](const ir::Constant* c) { return ConstExpr(c); },
            [&](Default) {
                TINT_UNIMPLEMENTED(IR, b.Diagnostics())
                    << "unhandled case in Switch(): " << val->TypeInfo().name;
                return nullptr;
            });
    }

    const ast::Expression* ConstExpr(const ir::Constant* c) {
        return Switch(
            c->Type(),  //
            [&](const type::I32*) { return b.Expr(c->value->ValueAs<i32>()); },
            [&](const type::U32*) { return b.Expr(c->value->ValueAs<u32>()); },
            [&](const type::F32*) { return b.Expr(c->value->ValueAs<f32>()); },
            [&](const type::F16*) { return b.Expr(c->value->ValueAs<f16>()); },
            [&](const type::Bool*) { return b.Expr(c->value->ValueAs<bool>()); },
            [&](Default) {
                TINT_UNIMPLEMENTED(IR, b.Diagnostics())
                    << "unhandled case in Switch(): " << c->TypeInfo().name;
                return nullptr;
            });
    }

    const ast::Type Type(const type::Type* ty) {
        return Switch(
            ty,                                              //
            [&](const type::Void*) { return ast::Type{}; },  //
            [&](const type::I32*) { return b.ty.i32(); },    //
            [&](const type::U32*) { return b.ty.u32(); },    //
            [&](const type::F16*) { return b.ty.f16(); },    //
            [&](const type::F32*) { return b.ty.f32(); },    //
            [&](const type::Bool*) { return b.ty.bool_(); },
            [&](const type::Matrix* m) {
                auto el = Type(m->type());
                return b.ty.mat(el, m->columns(), m->rows());
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
                return b.ty.multisampled_texture(t->dim(), Type(t->type()));
            },
            [&](const type::SampledTexture* t) {
                return b.ty.sampled_texture(t->dim(), Type(t->type()));
            },
            [&](const type::StorageTexture* t) {
                return b.ty.storage_texture(t->dim(), t->texel_format(), t->access());
            },
            [&](const type::Sampler* s) { return b.ty.sampler(s->kind()); },
            [&](const type::Pointer* p) {
                // Note: type::Pointer always has an inferred access, but WGSL only allows an
                // explicit access in the 'storage' address space.
                auto address_space = p->AddressSpace();
                auto access = address_space == builtin::AddressSpace::kStorage
                                  ? p->Access()
                                  : builtin::Access::kUndefined;
                return b.ty.pointer(Type(p->StoreType()), address_space, access);
            },
            [&](const type::Reference* r) { return Type(r->StoreType()); },
            [&](Default) {
                TINT_UNREACHABLE(IR, b.Diagnostics()) << "unhandled type: " << ty->TypeInfo().name;
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

    Symbol Sym(const Symbol& s) { return b.Symbols().Register(s.NameView()); }

    // void Err(std::string str) { b.Diagnostics().add_error(diag::System::IR, std::move(str)); }
};

}  // namespace

Program ToProgram(const Module& i) {
    return State{i}.Run();
}

}  // namespace tint::ir
