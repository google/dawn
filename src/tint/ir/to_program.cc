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
#include <tuple>
#include <utility>

#include "src/tint/ir/access.h"
#include "src/tint/ir/binary.h"
#include "src/tint/ir/block.h"
#include "src/tint/ir/break_if.h"
#include "src/tint/ir/call.h"
#include "src/tint/ir/constant.h"
#include "src/tint/ir/continue.h"
#include "src/tint/ir/exit_if.h"
#include "src/tint/ir/exit_loop.h"
#include "src/tint/ir/exit_switch.h"
#include "src/tint/ir/if.h"
#include "src/tint/ir/instruction.h"
#include "src/tint/ir/load.h"
#include "src/tint/ir/loop.h"
#include "src/tint/ir/module.h"
#include "src/tint/ir/multi_in_block.h"
#include "src/tint/ir/next_iteration.h"
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
#include "src/tint/utils/reverse.h"
#include "src/tint/utils/scoped_assignment.h"
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

/// Empty struct used as a sentinel value to indicate that an ast::Value has been consumed by its
/// single place of usage. Attempting to use this value a second time should result in an ICE.
struct ConsumedValue {};

class State {
  public:
    explicit State(Module& m) : mod(m) {}

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
    Module& mod;

    /// The target ProgramBuilder
    ProgramBuilder b;

    using ValueBinding = std::variant<Symbol, const ast::Expression*, ConsumedValue>;

    /// A hashmap of value to one of:
    /// * Symbol           - Name of 'let' (non-inlinable value), 'var' or parameter.
    /// * ast::Expression* - single use, inlined expression.
    /// * ConsumedValue    - a special value used to indicate that the value has already been
    ///                      consumed.
    utils::Hashmap<Value*, ValueBinding, 32> bindings_;

    /// The nesting depth of the currently generated AST
    /// 0  is module scope
    /// 1  is root-level function scope
    /// 2+ is within control flow
    uint32_t nesting_depth_ = 0;

    using StatementList = utils::Vector<const ast::Statement*,
                                        decltype(ast::BlockStatement::statements)::static_length>;
    StatementList* statements_ = nullptr;

    /// The current switch case block
    ir::Block* current_switch_case_ = nullptr;

    // Values that can be inlined.
    utils::Hashset<ir::Value*, 64> can_inline_;

    const ast::Function* Fn(ir::Function* fn) {
        SCOPED_NESTING();

        // TODO(crbug.com/tint/1915): Properly implement this when we've fleshed out Function
        static constexpr size_t N = decltype(ast::Function::params)::static_length;
        auto params = utils::Transform<N>(fn->Params(), [&](FunctionParam* param) {
            auto name = BindName(param);
            auto ty = Type(param->Type());
            return b.Param(name, ty);
        });

        auto name = BindName(fn);
        auto ret_ty = Type(fn->ReturnType());
        auto* body = Block(fn->Block());
        utils::Vector<const ast::Attribute*, 1> attrs{};
        utils::Vector<const ast::Attribute*, 1> ret_attrs{};
        return b.Func(name, std::move(params), ret_ty, body, std::move(attrs),
                      std::move(ret_attrs));
    }

    const ast::BlockStatement* Block(ir::Block* block) {
        // TODO(crbug.com/tint/1902): Handle block arguments.
        return b.Block(Statements(block));
    }

    StatementList Statements(ir::Block* block) {
        StatementList stmts;
        if (block) {
            MarkInlinable(block);
            TINT_SCOPED_ASSIGNMENT(statements_, &stmts);
            for (auto* inst : *block) {
                Instruction(inst);
            }
        }
        return stmts;
    }

    void MarkInlinable(ir::Block* block) {
        // An ordered list of possibly-inlinable values returned by sequenced instructions that have
        // not yet been marked-for or ruled-out-for inlining.
        utils::UniqueVector<ir::Value*, 32> pending_resolution;

        // Walk the instructions of the block starting with the first.
        for (auto* inst : *block) {
            // Is the instruction sequenced?
            bool sequenced = inst->Sequenced();

            // Walk the instruction's operands starting with the right-most.
            auto operands = inst->Operands();
            for (auto* operand : utils::Reverse(operands)) {
                if (!pending_resolution.Contains(operand)) {
                    continue;
                }
                // Operand is in 'pending_resolution'

                if (pending_resolution.TryPop(operand)) {
                    // Operand was the last sequenced value to be added to 'pending_resolution'
                    // This operand can be inlined as it does not change the sequencing order.
                    can_inline_.Add(operand);
                    sequenced = true;  // Inherit the 'sequenced' flag from the inlined value
                } else {
                    // Operand was in 'pending_resolution', but was not the last sequenced value to
                    // be added. Inlining this operand would break the sequencing order, so must be
                    // emitted as a let. All preceding pending values must also be emitted as a
                    // let to prevent them being inlined and breaking the sequencing order.
                    // Remove all the values in pending upto and including 'operand'.
                    for (size_t i = 0; i < pending_resolution.Length(); i++) {
                        if (pending_resolution[i] == operand) {
                            pending_resolution.Erase(0, i + 1);
                            break;
                        }
                    }
                }
            }

            if (inst->Results().Length() == 1) {
                // Instruction has a single result value.
                // Check to see if the result of this instruction is a candidate for inlining.
                auto* result = inst->Result();
                // Only values with a single usage can be inlined.
                // Named values are not inlined, as we want to emit the name for a let.
                if (result->Usages().Count() == 1 && !mod.NameOf(result).IsValid()) {
                    if (sequenced) {
                        // The value comes from a sequenced instruction. We need to ensure
                        // instruction ordering so add it to 'pending_resolution'.
                        pending_resolution.Add(result);
                    } else {
                        // The value comes from an unsequenced instruction. Just inline.
                        can_inline_.Add(result);
                    }
                    continue;
                }
            }

            // At this point the value has been ruled out for inlining.

            if (sequenced) {
                // A sequenced instruction with zero or multiple return values cannot be inlined.
                // All preceding sequenced instructions cannot be inlined past this point.
                pending_resolution.Clear();
            }
        }
    }

    void Append(const ast::Statement* inst) { statements_->Push(inst); }

    void Instruction(ir::Instruction* inst) {
        tint::Switch(
            inst,                                       //
            [&](ir::Binary* i) { Binary(i); },          //
            [&](ir::BreakIf* i) { BreakIf(i); },        //
            [&](ir::Call* i) { Call(i); },              //
            [&](ir::ExitIf*) {},                        //
            [&](ir::ExitSwitch* i) { ExitSwitch(i); },  //
            [&](ir::ExitLoop* i) { ExitLoop(i); },      //
            [&](ir::If* i) { If(i); },                  //
            [&](ir::Load* l) { Load(l); },              //
            [&](ir::Loop* l) { Loop(l); },              //
            [&](ir::Return* i) { Return(i); },          //
            [&](ir::Store* i) { Store(i); },            //
            [&](ir::Switch* i) { Switch(i); },          //
            [&](ir::Unary* u) { Unary(u); },            //
            [&](ir::Var* i) { Var(i); },                //
            [&](ir::NextIteration*) {},                 //
            [&](ir::Continue*) {},                      //
            [&](Default) { UNHANDLED_CASE(inst); });
    }

    void If(ir::If* if_) {
        SCOPED_NESTING();

        auto true_stmts = Statements(if_->True());
        auto false_stmts = Statements(if_->False());
        if (AsShortCircuit(if_, true_stmts, false_stmts)) {
            return;
        }

        auto* cond = Expr(if_->Condition());
        auto* true_block = b.Block(std::move(true_stmts));

        switch (false_stmts.Length()) {
            case 0:
                Append(b.If(cond, true_block));
                return;
            case 1:
                if (auto* else_if = false_stmts.Front()->As<ast::IfStatement>()) {
                    Append(b.If(cond, true_block, b.Else(else_if)));
                    return;
                }
                break;
        }

        auto* false_block = b.Block(std::move(false_stmts));
        Append(b.If(cond, true_block, b.Else(false_block)));
    }

    void Loop(ir::Loop* l) {
        auto init_stmts = Statements(l->Initializer());
        auto* init = init_stmts.Length() == 1 ? init_stmts.Front()->As<ast::VariableDeclStatement>()
                                              : nullptr;

        const ast::Expression* cond = nullptr;

        StatementList body_stmts;
        {
            MarkInlinable(l->Body());
            TINT_SCOPED_ASSIGNMENT(statements_, &body_stmts);
            for (auto* inst : *l->Body()) {
                if (body_stmts.IsEmpty()) {
                    if (auto* if_ = inst->As<ir::If>()) {
                        if (!if_->HasResults() &&                          //
                            if_->True()->Length() == 1 &&                  //
                            if_->False()->Length() == 1 &&                 //
                            tint::Is<ir::ExitIf>(if_->True()->Front()) &&  //
                            tint::Is<ir::ExitLoop>(if_->False()->Front())) {
                            cond = Expr(if_->Condition());
                            continue;
                        }
                    }
                }

                Instruction(inst);
            }
        }

        auto cont_stmts = Statements(l->Continuing());
        auto* cont = cont_stmts.Length() == 1 ? cont_stmts.Front() : nullptr;

        auto* body = b.Block(std::move(body_stmts));

        const ast::Statement* loop = nullptr;
        if (cond) {
            if (init || cont) {
                loop = b.For(init, cond, cont, body);
            } else {
                loop = b.While(cond, body);
            }
        } else {
            loop = cont_stmts.IsEmpty() ? b.Loop(body)  //
                                        : b.Loop(body, b.Block(std::move(cont_stmts)));
            if (!init_stmts.IsEmpty()) {
                init_stmts.Push(loop);
                loop = b.Block(std::move(init_stmts));
            }
        }
        statements_->Push(loop);
    }

    void Switch(ir::Switch* s) {
        SCOPED_NESTING();

        auto* cond = Expr(s->Condition());

        auto cases = utils::Transform(
            s->Cases(),  //
            [&](ir::Switch::Case c) -> const tint::ast::CaseStatement* {
                SCOPED_NESTING();

                const ast::BlockStatement* body = nullptr;
                {
                    TINT_SCOPED_ASSIGNMENT(current_switch_case_, c.Block());
                    body = Block(c.Block());
                }

                auto selectors = utils::Transform(c.selectors,  //
                                                  [&](ir::Switch::CaseSelector cs) {
                                                      return cs.IsDefault()
                                                                 ? b.DefaultCaseSelector()
                                                                 : b.CaseSelector(Expr(cs.val));
                                                  });
                return b.Case(std::move(selectors), body);
            });

        Append(b.Switch(cond, std::move(cases)));
    }

    void ExitSwitch(const ir::ExitSwitch* e) {
        if (current_switch_case_ && current_switch_case_->Terminator() == e) {
            return;  // No need to emit
        }
        Append(b.Break());
    }

    void ExitLoop(const ir::ExitLoop*) { Append(b.Break()); }

    void BreakIf(ir::BreakIf* i) { Append(b.BreakIf(Expr(i->Condition()))); }

    void Return(ir::Return* ret) {
        if (ret->Args().IsEmpty()) {
            // Return has no arguments.
            // If this block is nested withing some control flow, then we must
            // emit a 'return' statement, otherwise we've just naturally reached
            // the end of the function where the 'return' is redundant.
            if (nesting_depth_ > 1) {
                Append(b.Return());
            }
            return;
        }

        // Return has arguments - this is the return value.
        if (ret->Args().Length() != 1) {
            TINT_ICE(IR, b.Diagnostics())
                << "expected 1 value for return, got " << ret->Args().Length();
            return;
        }

        Append(b.Return(Expr(ret->Args().Front())));
    }

    void Var(ir::Var* var) {
        auto* val = var->Result();
        Symbol name = BindName(val);
        auto* ptr = As<type::Pointer>(val->Type());
        auto ty = Type(ptr->StoreType());
        const ast::Expression* init = nullptr;
        if (var->Initializer()) {
            init = Expr(var->Initializer());
        }
        switch (ptr->AddressSpace()) {
            case builtin::AddressSpace::kFunction:
                Append(b.Decl(b.Var(name, ty, init)));
                return;
            case builtin::AddressSpace::kStorage:
                Append(b.Decl(b.Var(name, ty, init, ptr->Access(), ptr->AddressSpace())));
                return;
            default:
                Append(b.Decl(b.Var(name, ty, init, ptr->AddressSpace())));
                return;
        }
    }

    void Store(ir::Store* store) {
        auto* dst = Expr(store->To());
        auto* src = Expr(store->From());
        Append(b.Assign(dst, src));
    }

    void Call(ir::Call* call) {
        auto args = utils::Transform<2>(call->Args(), [&](ir::Value* arg) { return Expr(arg); });
        tint::Switch(
            call,  //
            [&](ir::UserCall* c) {
                auto* expr = b.Call(BindName(c->Func()), std::move(args));
                if (!call->HasResults() || call->Result()->Usages().IsEmpty()) {
                    Append(b.CallStmt(expr));
                    return;
                }
                Bind(c->Result(), expr);
            },
            [&](Default) { UNHANDLED_CASE(call); });
    }

    void Load(ir::Load* l) { Bind(l->Result(), Expr(l->From())); }

    void Unary(ir::Unary* u) {
        const ast::Expression* expr = nullptr;
        switch (u->Kind()) {
            case ir::Unary::Kind::kComplement:
                expr = b.Complement(Expr(u->Val()));
                break;
            case ir::Unary::Kind::kNegation:
                expr = b.Negation(Expr(u->Val()));
                break;
        }
        Bind(u->Result(), expr);
    }

    void Binary(ir::Binary* e) {
        if (e->Kind() == ir::Binary::Kind::kEqual) {
            auto* rhs = e->RHS()->As<ir::Constant>();
            if (rhs && rhs->Type()->Is<type::Bool>() && rhs->Value()->ValueAs<bool>() == false) {
                // expr == false
                Bind(e->Result(), b.Not(Expr(e->LHS())));
                return;
            }
        }
        auto* lhs = Expr(e->LHS());
        auto* rhs = Expr(e->RHS());
        const ast::Expression* expr = nullptr;
        switch (e->Kind()) {
            case ir::Binary::Kind::kAdd:
                expr = b.Add(lhs, rhs);
                break;
            case ir::Binary::Kind::kSubtract:
                expr = b.Sub(lhs, rhs);
                break;
            case ir::Binary::Kind::kMultiply:
                expr = b.Mul(lhs, rhs);
                break;
            case ir::Binary::Kind::kDivide:
                expr = b.Div(lhs, rhs);
                break;
            case ir::Binary::Kind::kModulo:
                expr = b.Mod(lhs, rhs);
                break;
            case ir::Binary::Kind::kAnd:
                expr = b.And(lhs, rhs);
                break;
            case ir::Binary::Kind::kOr:
                expr = b.Or(lhs, rhs);
                break;
            case ir::Binary::Kind::kXor:
                expr = b.Xor(lhs, rhs);
                break;
            case ir::Binary::Kind::kEqual:
                expr = b.Equal(lhs, rhs);
                break;
            case ir::Binary::Kind::kNotEqual:
                expr = b.NotEqual(lhs, rhs);
                break;
            case ir::Binary::Kind::kLessThan:
                expr = b.LessThan(lhs, rhs);
                break;
            case ir::Binary::Kind::kGreaterThan:
                expr = b.GreaterThan(lhs, rhs);
                break;
            case ir::Binary::Kind::kLessThanEqual:
                expr = b.LessThanEqual(lhs, rhs);
                break;
            case ir::Binary::Kind::kGreaterThanEqual:
                expr = b.GreaterThanEqual(lhs, rhs);
                break;
            case ir::Binary::Kind::kShiftLeft:
                expr = b.Shl(lhs, rhs);
                break;
            case ir::Binary::Kind::kShiftRight:
                expr = b.Shr(lhs, rhs);
                break;
        }
        Bind(e->Result(), expr);
    }

    TINT_BEGIN_DISABLE_WARNING(UNREACHABLE_CODE);

    const ast::Expression* Expr(ir::Value* value) {
        return tint::Switch(
            value,                                         //
            [&](ir::Constant* c) { return Constant(c); },  //
            [&](Default) -> const ast::Expression* {
                auto lookup = bindings_.Find(value);
                if (TINT_UNLIKELY(!lookup)) {
                    TINT_ICE(IR, b.Diagnostics())
                        << "Expr(" << (value ? value->TypeInfo().name : "null")
                        << ") value has no expression";
                    return b.Expr("<error>");
                }
                return std::visit(
                    [&](auto&& got) -> const ast::Expression* {
                        using T = std::decay_t<decltype(got)>;

                        if constexpr (std::is_same_v<T, Symbol>) {
                            return b.Expr(got);  // var, let or parameter.
                        }

                        if constexpr (std::is_same_v<T, const ast::Expression*>) {
                            // Single use (inlined) expression.
                            // Mark the bindings_ map entry as consumed.
                            *lookup = ConsumedValue{};
                            return got;
                        }

                        if constexpr (std::is_same_v<T, ConsumedValue>) {
                            TINT_ICE(IR, b.Diagnostics()) << "Expr(" << value->TypeInfo().name
                                                          << ") called twice on the same value";
                        } else {
                            TINT_ICE(IR, b.Diagnostics())
                                << "Expr(" << value->TypeInfo().name << ") has unhandled value";
                        }
                        return b.Expr("<error>");
                    },
                    *lookup);
            });
    }

    TINT_END_DISABLE_WARNING(UNREACHABLE_CODE);

    const ast::Expression* Constant(ir::Constant* c) {
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
                return b.ty.ptr(address_space, el, access);
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
    // Bindings
    ////////////////////////////////////////////////////////////////////////////////////////////////

    /// Creates and returns a new, unique name for the given value, or returns the previously
    /// created name.
    /// @return the value's name
    Symbol BindName(Value* value, std::string_view suggested = {}) {
        TINT_ASSERT(IR, value);
        auto& existing = bindings_.GetOrCreate(value, [&] {
            if (!suggested.empty()) {
                return b.Symbols().New(suggested);
            }
            if (auto sym = mod.NameOf(value)) {
                return b.Symbols().New(sym.NameView());
            }
            return b.Symbols().New("v");
        });
        if (auto* name = std::get_if<Symbol>(&existing); TINT_LIKELY(name)) {
            return *name;
        }

        TINT_ICE(IR, b.Diagnostics()) << "BindName(" << value->TypeInfo().name
                                      << ") called on value that has non-name binding";
        return {};
    }

    template <typename T>
    void Bind(ir::Value* value, const T* expr) {
        TINT_ASSERT(IR, value);
        if (can_inline_.Remove(value)) {
            // Value will be inlined at its place of usage.
            bool added = bindings_.Add(value, expr);
            if (TINT_UNLIKELY(!added)) {
                TINT_ICE(IR, b.Diagnostics())
                    << "Bind(" << value->TypeInfo().name << ") called twice for same node";
            }
        } else {
            Append(b.Decl(b.Let(BindName(value), expr)));
        }
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Helpers
    ////////////////////////////////////////////////////////////////////////////////////////////////
    bool AsShortCircuit(ir::If* i,
                        const StatementList& true_stmts,
                        const StatementList& false_stmts) {
        if (!i->HasResults()) {
            return false;
        }
        auto* result = i->Result();
        if (!result->Type()->Is<type::Bool>()) {
            return false;  // Wrong result type
        }
        if (i->Exits().Count() != 2) {
            return false;  // Doesn't have two exits
        }
        if (!true_stmts.IsEmpty() || !false_stmts.IsEmpty()) {
            return false;  // True or False blocks contain statements
        }

        auto* cond = i->Condition();
        auto* true_val = i->True()->Back()->Operands().Front();
        auto* false_val = i->False()->Back()->Operands().Front();
        if (IsConstant(false_val, false)) {
            //  %res = if %cond {
            //     block {  # true
            //       exit_if %true_val;
            //     }
            //     block {  # false
            //       exit_if false;
            //     }
            //  }
            //
            // transform into:
            //
            //   res = cond && true_val;
            //
            auto* lhs = Expr(cond);
            auto* rhs = Expr(true_val);
            Bind(result, b.LogicalAnd(lhs, rhs));
            return true;
        }
        if (IsConstant(true_val, true)) {
            //  %res = if %cond {
            //     block {  # true
            //       exit_if true;
            //     }
            //     block {  # false
            //       exit_if %false_val;
            //     }
            //  }
            //
            // transform into:
            //
            //   res = cond || false_val;
            //
            auto* lhs = Expr(cond);
            auto* rhs = Expr(false_val);
            Bind(result, b.LogicalOr(lhs, rhs));
            return true;
        }
        return false;
    }

    bool IsConstant(ir::Value* val, bool value) {
        if (auto* c = val->As<ir::Constant>()) {
            if (c->Type()->Is<type::Bool>()) {
                return c->Value()->ValueAs<bool>() == value;
            }
        }
        return false;
    }
};

}  // namespace

Program ToProgram(Module& i) {
    return State{i}.Run();
}

}  // namespace tint::ir
