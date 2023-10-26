// Copyright 2023 The Dawn & Tint Authors
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// 1. Redistributions of source code must retain the above copyright notice, this
//    list of conditions and the following disclaimer.
//
// 2. Redistributions in binary form must reproduce the above copyright notice,
//    this list of conditions and the following disclaimer in the documentation
//    and/or other materials provided with the distribution.
//
// 3. Neither the name of the copyright holder nor the names of its
//    contributors may be used to endorse or promote products derived from
//    this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
// FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
// DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
// CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
// OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#include "src/tint/lang/wgsl/writer/ir_to_program/ir_to_program.h"

#include <string>
#include <tuple>
#include <utility>

#include "src/tint/lang/core/builtin_type.h"
#include "src/tint/lang/core/constant/splat.h"
#include "src/tint/lang/core/fluent_types.h"
#include "src/tint/lang/core/ir/access.h"
#include "src/tint/lang/core/ir/binary.h"
#include "src/tint/lang/core/ir/bitcast.h"
#include "src/tint/lang/core/ir/block.h"
#include "src/tint/lang/core/ir/break_if.h"
#include "src/tint/lang/core/ir/call.h"
#include "src/tint/lang/core/ir/constant.h"
#include "src/tint/lang/core/ir/construct.h"
#include "src/tint/lang/core/ir/continue.h"
#include "src/tint/lang/core/ir/convert.h"
#include "src/tint/lang/core/ir/core_builtin_call.h"
#include "src/tint/lang/core/ir/discard.h"
#include "src/tint/lang/core/ir/exit_if.h"
#include "src/tint/lang/core/ir/exit_loop.h"
#include "src/tint/lang/core/ir/exit_switch.h"
#include "src/tint/lang/core/ir/if.h"
#include "src/tint/lang/core/ir/instruction.h"
#include "src/tint/lang/core/ir/let.h"
#include "src/tint/lang/core/ir/load.h"
#include "src/tint/lang/core/ir/load_vector_element.h"
#include "src/tint/lang/core/ir/loop.h"
#include "src/tint/lang/core/ir/module.h"
#include "src/tint/lang/core/ir/multi_in_block.h"
#include "src/tint/lang/core/ir/next_iteration.h"
#include "src/tint/lang/core/ir/return.h"
#include "src/tint/lang/core/ir/store.h"
#include "src/tint/lang/core/ir/store_vector_element.h"
#include "src/tint/lang/core/ir/switch.h"
#include "src/tint/lang/core/ir/swizzle.h"
#include "src/tint/lang/core/ir/unary.h"
#include "src/tint/lang/core/ir/unreachable.h"
#include "src/tint/lang/core/ir/user_call.h"
#include "src/tint/lang/core/ir/validator.h"
#include "src/tint/lang/core/ir/var.h"
#include "src/tint/lang/core/type/atomic.h"
#include "src/tint/lang/core/type/depth_multisampled_texture.h"
#include "src/tint/lang/core/type/depth_texture.h"
#include "src/tint/lang/core/type/multisampled_texture.h"
#include "src/tint/lang/core/type/pointer.h"
#include "src/tint/lang/core/type/reference.h"
#include "src/tint/lang/core/type/sampler.h"
#include "src/tint/lang/core/type/texture.h"
#include "src/tint/lang/wgsl/ir/builtin_call.h"
#include "src/tint/lang/wgsl/program/program_builder.h"
#include "src/tint/lang/wgsl/resolver/resolve.h"
#include "src/tint/lang/wgsl/writer/ir_to_program/rename_conflicts.h"
#include "src/tint/utils/containers/hashmap.h"
#include "src/tint/utils/containers/predicates.h"
#include "src/tint/utils/containers/reverse.h"
#include "src/tint/utils/containers/transform.h"
#include "src/tint/utils/containers/vector.h"
#include "src/tint/utils/macros/scoped_assignment.h"
#include "src/tint/utils/math/math.h"
#include "src/tint/utils/rtti/switch.h"

// Helper for incrementing nesting_depth_ and then decrementing nesting_depth_ at the end
// of the scope that holds the call.
#define SCOPED_NESTING() \
    nesting_depth_++;    \
    TINT_DEFER(nesting_depth_--)

using namespace tint::core::fluent_types;  // NOLINT

namespace tint::wgsl::writer {
namespace {

class State {
  public:
    explicit State(core::ir::Module& m) : mod(m) {}

    Program Run() {
        // Run transforms need to sanitize for WGSL.
        {
            auto result = RenameConflicts(&mod);
            if (!result) {
                b.Diagnostics().add(result.Failure().reason);
                return Program(std::move(b));
            }
        }

        if (auto res = core::ir::Validate(mod); !res) {
            // IR module failed validation.
            b.Diagnostics() = res.Failure().reason;
            return Program{resolver::Resolve(b)};
        }

        RootBlock(mod.root_block);

        // TODO(crbug.com/tint/1902): Emit user-declared types
        for (auto* fn : mod.functions) {
            Fn(fn);
        }
        return Program{resolver::Resolve(b)};
    }

  private:
    /// The AST representation for an IR pointer type
    enum class PtrKind {
        kPtr,  // IR pointer is represented in the AST as a pointer
        kRef,  // IR pointer is represented in the AST as a reference
    };

    /// The source IR module
    core::ir::Module& mod;

    /// The target ProgramBuilder
    ProgramBuilder b;

    /// The structure for a value held by a 'let', 'var' or parameter.
    struct VariableValue {
        Symbol name;  // Name of the variable
        PtrKind ptr_kind = PtrKind::kRef;
    };

    /// The structure for an inlined value
    struct InlinedValue {
        const ast::Expression* expr = nullptr;
        PtrKind ptr_kind = PtrKind::kRef;
    };

    /// Empty struct used as a sentinel value to indicate that an ast::Value has been consumed by
    /// its single place of usage. Attempting to use this value a second time should result in an
    /// ICE.
    struct ConsumedValue {};

    using ValueBinding = std::variant<VariableValue, InlinedValue, ConsumedValue>;

    /// IR values to their representation
    Hashmap<core::ir::Value*, ValueBinding, 32> bindings_;

    /// Names for values
    Hashmap<core::ir::Value*, Symbol, 32> names_;

    /// The nesting depth of the currently generated AST
    /// 0  is module scope
    /// 1  is root-level function scope
    /// 2+ is within control flow
    uint32_t nesting_depth_ = 0;

    using StatementList =
        Vector<const ast::Statement*, decltype(ast::BlockStatement::statements)::static_length>;
    StatementList* statements_ = nullptr;

    /// The current switch case block
    core::ir::Block* current_switch_case_ = nullptr;

    /// Values that can be inlined.
    Hashset<core::ir::Value*, 64> can_inline_;

    /// Set of enable directives emitted.
    Hashset<wgsl::Extension, 4> enables_;

    /// Map of struct to output program name.
    Hashmap<const core::type::Struct*, Symbol, 8> structs_;

    /// True if 'diagnostic(off, derivative_uniformity)' has been emitted
    bool disabled_derivative_uniformity_ = false;

    void RootBlock(core::ir::Block* root) {
        for (auto* inst : *root) {
            tint::Switch(
                inst,                                   //
                [&](core::ir::Var* var) { Var(var); },  //
                TINT_ICE_ON_NO_MATCH);
        }
    }
    const ast::Function* Fn(core::ir::Function* fn) {
        SCOPED_NESTING();

        // TODO(crbug.com/tint/1915): Properly implement this when we've fleshed out Function
        static constexpr size_t N = decltype(ast::Function::params)::static_length;
        auto params = tint::Transform<N>(fn->Params(), [&](core::ir::FunctionParam* param) {
            auto ty = Type(param->Type());
            auto name = NameFor(param);
            Bind(param, name, PtrKind::kPtr);

            if (ParamRequiresFullPtrParameters(param->Type())) {
                Enable(wgsl::Extension::kChromiumExperimentalFullPtrParameters);
            }
            return b.Param(name, ty);
        });

        auto name = NameFor(fn);
        auto ret_ty = Type(fn->ReturnType());
        auto* body = Block(fn->Block());
        Vector<const ast::Attribute*, 1> attrs{};
        Vector<const ast::Attribute*, 1> ret_attrs{};
        return b.Func(name, std::move(params), ret_ty, body, std::move(attrs),
                      std::move(ret_attrs));
    }

    const ast::BlockStatement* Block(core::ir::Block* block) {
        // TODO(crbug.com/tint/1902): Handle block arguments.
        return b.Block(Statements(block));
    }

    StatementList Statements(core::ir::Block* block) {
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

    void MarkInlinable(core::ir::Block* block) {
        // An ordered list of possibly-inlinable values returned by sequenced instructions that have
        // not yet been marked-for or ruled-out-for inlining.
        UniqueVector<core::ir::Value*, 32> pending_resolution;

        // Walk the instructions of the block starting with the first.
        for (auto* inst : *block) {
            // Is the instruction sequenced?
            bool sequenced = inst->Sequenced();

            // Walk the instruction's operands starting with the right-most.
            auto operands = inst->Operands();
            for (auto* operand : tint::Reverse(operands)) {
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

    void Instruction(core::ir::Instruction* inst) {
        tint::Switch(
            inst,                                                             //
            [&](core::ir::Access* i) { Access(i); },                          //
            [&](core::ir::Binary* i) { Binary(i); },                          //
            [&](core::ir::BreakIf* i) { BreakIf(i); },                        //
            [&](core::ir::Call* i) { Call(i); },                              //
            [&](core::ir::Continue*) {},                                      //
            [&](core::ir::ExitIf*) {},                                        //
            [&](core::ir::ExitLoop* i) { ExitLoop(i); },                      //
            [&](core::ir::ExitSwitch* i) { ExitSwitch(i); },                  //
            [&](core::ir::If* i) { If(i); },                                  //
            [&](core::ir::Let* i) { Let(i); },                                //
            [&](core::ir::Load* l) { Load(l); },                              //
            [&](core::ir::LoadVectorElement* i) { LoadVectorElement(i); },    //
            [&](core::ir::Loop* l) { Loop(l); },                              //
            [&](core::ir::NextIteration*) {},                                 //
            [&](core::ir::Return* i) { Return(i); },                          //
            [&](core::ir::Store* i) { Store(i); },                            //
            [&](core::ir::StoreVectorElement* i) { StoreVectorElement(i); },  //
            [&](core::ir::Switch* i) { Switch(i); },                          //
            [&](core::ir::Swizzle* i) { Swizzle(i); },                        //
            [&](core::ir::Unary* i) { Unary(i); },                            //
            [&](core::ir::Unreachable*) {},                                   //
            [&](core::ir::Var* i) { Var(i); },                                //
            TINT_ICE_ON_NO_MATCH);
    }

    void If(core::ir::If* if_) {
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

    void Loop(core::ir::Loop* l) {
        SCOPED_NESTING();

        // Build all the initializer statements
        auto init_stmts = Statements(l->Initializer());

        // If there's a single initializer statement and meets the WGSL 'for_init' pattern, then
        // this can be used as the initializer for a for-loop.
        // @see https://www.w3.org/TR/WGSL/#syntax-for_init
        auto* init = (init_stmts.Length() == 1) &&
                             init_stmts.Front()
                                 ->IsAnyOf<ast::VariableDeclStatement, ast::AssignmentStatement,
                                           ast::CompoundAssignmentStatement,
                                           ast::IncrementDecrementStatement, ast::CallStatement>()
                         ? init_stmts.Front()
                         : nullptr;

        // Build the loop body statements. If the loop body starts with a if with the following
        // pattern, then treat it as the loop condition:
        //   if cond {
        //     block { exit_if   }
        //     block { exit_loop }
        //   }
        const ast::Expression* cond = nullptr;
        StatementList body_stmts;
        {
            MarkInlinable(l->Body());
            TINT_SCOPED_ASSIGNMENT(statements_, &body_stmts);
            for (auto* inst : *l->Body()) {
                if (body_stmts.IsEmpty()) {
                    if (auto* if_ = inst->As<core::ir::If>()) {
                        if (!if_->HasResults() &&                                //
                            if_->True()->Length() == 1 &&                        //
                            if_->False()->Length() == 1 &&                       //
                            tint::Is<core::ir::ExitIf>(if_->True()->Front()) &&  //
                            tint::Is<core::ir::ExitLoop>(if_->False()->Front())) {
                            // Matched the loop condition.
                            cond = Expr(if_->Condition());
                            continue;  // Don't emit this as an instruction in the body.
                        }
                    }
                }

                // Process the loop body instruction. Append to 'body_stmts'
                Instruction(inst);
            }
        }

        // Build any continuing statements
        auto cont_stmts = Statements(l->Continuing());
        // If there's a single continuing statement and meets the WGSL 'for_update' pattern then
        // this can be used as the continuing for a for-loop.
        // @see https://www.w3.org/TR/WGSL/#syntax-for_update
        auto* cont =
            (cont_stmts.Length() == 1) &&
                    cont_stmts.Front()
                        ->IsAnyOf<ast::AssignmentStatement, ast::CompoundAssignmentStatement,
                                  ast::IncrementDecrementStatement, ast::CallStatement>()
                ? cont_stmts.Front()
                : nullptr;

        // Depending on 'init', 'cond' and 'cont', build a 'for', 'while' or 'loop'
        const ast::Statement* loop = nullptr;
        if ((!cont && !cont_stmts.IsEmpty())  // Non-trivial continuing
            || !cond                          // or non-trivial or no condition
        ) {
            // Build a loop
            if (cond) {
                body_stmts.Insert(0, b.If(b.Not(cond), b.Block(b.Break())));
            }
            auto* body = b.Block(std::move(body_stmts));
            loop = cont_stmts.IsEmpty() ? b.Loop(body)  //
                                        : b.Loop(body, b.Block(std::move(cont_stmts)));
            if (!init_stmts.IsEmpty()) {
                init_stmts.Push(loop);
                loop = b.Block(std::move(init_stmts));
            }
        } else if (init || cont) {
            // Build a for-loop
            auto* body = b.Block(std::move(body_stmts));
            loop = b.For(init, cond, cont, body);
            if (!init && !init_stmts.IsEmpty()) {
                init_stmts.Push(loop);
                loop = b.Block(std::move(init_stmts));
            }
        } else {
            // Build a while-loop
            auto* body = b.Block(std::move(body_stmts));
            loop = b.While(cond, body);
            if (!init_stmts.IsEmpty()) {
                init_stmts.Push(loop);
                loop = b.Block(std::move(init_stmts));
            }
        }
        statements_->Push(loop);
    }

    void Switch(core::ir::Switch* s) {
        SCOPED_NESTING();

        auto* cond = Expr(s->Condition());

        auto cases = tint::Transform(
            s->Cases(),  //
            [&](core::ir::Switch::Case c) -> const tint::ast::CaseStatement* {
                SCOPED_NESTING();

                const ast::BlockStatement* body = nullptr;
                {
                    TINT_SCOPED_ASSIGNMENT(current_switch_case_, c.Block());
                    body = Block(c.Block());
                }

                auto selectors = tint::Transform(c.selectors,  //
                                                 [&](core::ir::Switch::CaseSelector cs) {
                                                     return cs.IsDefault()
                                                                ? b.DefaultCaseSelector()
                                                                : b.CaseSelector(Expr(cs.val));
                                                 });
                return b.Case(std::move(selectors), body);
            });

        Append(b.Switch(cond, std::move(cases)));
    }

    void ExitSwitch(const core::ir::ExitSwitch* e) {
        if (current_switch_case_ && current_switch_case_->Terminator() == e) {
            return;  // No need to emit
        }
        Append(b.Break());
    }

    void ExitLoop(const core::ir::ExitLoop*) { Append(b.Break()); }

    void BreakIf(core::ir::BreakIf* i) { Append(b.BreakIf(Expr(i->Condition()))); }

    void Return(core::ir::Return* ret) {
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
            TINT_ICE() << "expected 1 value for return, got " << ret->Args().Length();
            return;
        }

        Append(b.Return(Expr(ret->Args().Front())));
    }

    void Var(core::ir::Var* var) {
        auto* val = var->Result();
        auto* ptr = As<core::type::Pointer>(val->Type());
        auto ty = Type(ptr->StoreType());
        Symbol name = NameFor(var->Result());
        Bind(var->Result(), name, PtrKind::kRef);

        Vector<const ast::Attribute*, 4> attrs;
        if (auto bp = var->BindingPoint()) {
            attrs.Push(b.Group(AInt(bp->group)));
            attrs.Push(b.Binding(AInt(bp->binding)));
        }

        const ast::Expression* init = nullptr;
        if (var->Initializer()) {
            init = Expr(var->Initializer());
        }
        switch (ptr->AddressSpace()) {
            case core::AddressSpace::kFunction:
                Append(b.Decl(b.Var(name, ty, init, std::move(attrs))));
                return;
            case core::AddressSpace::kStorage:
                b.GlobalVar(name, ty, init, ptr->Access(), ptr->AddressSpace(), std::move(attrs));
                return;
            case core::AddressSpace::kHandle:
                b.GlobalVar(name, ty, init, std::move(attrs));
                return;
            default:
                b.GlobalVar(name, ty, init, ptr->AddressSpace(), std::move(attrs));
                return;
        }
    }

    void Let(core::ir::Let* let) {
        Symbol name = NameFor(let->Result());
        Append(b.Decl(b.Let(name, Expr(let->Value(), PtrKind::kPtr))));
        Bind(let->Result(), name, PtrKind::kPtr);
    }

    void Store(core::ir::Store* store) {
        auto* dst = Expr(store->To());
        auto* src = Expr(store->From());
        Append(b.Assign(dst, src));
    }

    void StoreVectorElement(core::ir::StoreVectorElement* store) {
        auto* ptr = Expr(store->To());
        auto* val = Expr(store->Value());
        Append(b.Assign(VectorMemberAccess(ptr, store->Index()), val));
    }

    void Call(core::ir::Call* call) {
        auto args = tint::Transform<4>(call->Args(), [&](core::ir::Value* arg) {
            // Pointer-like arguments are passed by pointer, never reference.
            return Expr(arg, PtrKind::kPtr);
        });
        tint::Switch(
            call,  //
            [&](core::ir::UserCall* c) {
                for (auto* arg : call->Args()) {
                    if (ArgRequiresFullPtrParameters(arg)) {
                        Enable(wgsl::Extension::kChromiumExperimentalFullPtrParameters);
                        break;
                    }
                }
                auto* expr = b.Call(NameFor(c->Target()), std::move(args));
                if (!call->HasResults() || call->Result()->Usages().IsEmpty()) {
                    Append(b.CallStmt(expr));
                    return;
                }
                Bind(c->Result(), expr, PtrKind::kPtr);
            },
            [&](wgsl::ir::BuiltinCall* c) {
                if (!disabled_derivative_uniformity_ && RequiresDerivativeUniformity(c->Func())) {
                    // TODO(crbug.com/tint/1985): Be smarter about disabling derivative uniformity.
                    b.DiagnosticDirective(wgsl::DiagnosticSeverity::kOff,
                                          wgsl::CoreDiagnosticRule::kDerivativeUniformity);
                    disabled_derivative_uniformity_ = true;
                }

                switch (c->Func()) {
                    case wgsl::BuiltinFn::kTextureBarrier:
                        Enable(wgsl::Extension::kChromiumExperimentalReadWriteStorageTexture);
                        break;
                    case wgsl::BuiltinFn::kSubgroupBallot:
                    case wgsl::BuiltinFn::kSubgroupBroadcast:
                        Enable(wgsl::Extension::kChromiumExperimentalSubgroups);
                        break;
                    default:
                        break;
                }

                auto* expr = b.Call(c->Func(), std::move(args));
                if (!call->HasResults() || call->Result()->Type()->Is<core::type::Void>()) {
                    Append(b.CallStmt(expr));
                    return;
                }
                Bind(c->Result(), expr, PtrKind::kPtr);
            },
            [&](core::ir::Construct* c) {
                auto ty = Type(c->Result()->Type());
                Bind(c->Result(), b.Call(ty, std::move(args)), PtrKind::kPtr);
            },
            [&](core::ir::Convert* c) {
                auto ty = Type(c->Result()->Type());
                Bind(c->Result(), b.Call(ty, std::move(args)), PtrKind::kPtr);
            },
            [&](core::ir::Bitcast* c) {
                auto ty = Type(c->Result()->Type());
                Bind(c->Result(), b.Bitcast(ty, args[0]), PtrKind::kPtr);
            },
            [&](core::ir::Discard*) { Append(b.Discard()); },  //
            TINT_ICE_ON_NO_MATCH);
    }

    void Load(core::ir::Load* l) { Bind(l->Result(), Expr(l->From())); }

    void LoadVectorElement(core::ir::LoadVectorElement* load) {
        auto* ptr = Expr(load->From());
        Bind(load->Result(), VectorMemberAccess(ptr, load->Index()));
    }

    void Unary(core::ir::Unary* u) {
        const ast::Expression* expr = nullptr;
        switch (u->Op()) {
            case core::ir::UnaryOp::kComplement:
                expr = b.Complement(Expr(u->Val()));
                break;
            case core::ir::UnaryOp::kNegation:
                expr = b.Negation(Expr(u->Val()));
                break;
        }
        Bind(u->Result(), expr);
    }

    void Access(core::ir::Access* a) {
        auto* expr = Expr(a->Object());
        auto* obj_ty = a->Object()->Type()->UnwrapPtr();
        for (auto* index : a->Indices()) {
            tint::Switch(
                obj_ty,
                [&](const core::type::Vector* vec) {
                    TINT_DEFER(obj_ty = vec->type());
                    expr = VectorMemberAccess(expr, index);
                },
                [&](const core::type::Matrix* mat) {
                    obj_ty = mat->ColumnType();
                    expr = b.IndexAccessor(expr, Expr(index));
                },
                [&](const core::type::Array* arr) {
                    obj_ty = arr->ElemType();
                    expr = b.IndexAccessor(expr, Expr(index));
                },
                [&](const core::type::Struct* s) {
                    if (auto* c = index->As<core::ir::Constant>()) {
                        auto i = c->Value()->ValueAs<uint32_t>();
                        TINT_ASSERT_OR_RETURN(i < s->Members().Length());
                        auto* member = s->Members()[i];
                        obj_ty = member->Type();
                        expr = b.MemberAccessor(expr, member->Name().NameView());
                    } else {
                        TINT_ICE() << "invalid index for struct type: " << index->TypeInfo().name;
                    }
                },  //
                TINT_ICE_ON_NO_MATCH);
        }
        Bind(a->Result(), expr);
    }

    void Swizzle(core::ir::Swizzle* s) {
        auto* vec = Expr(s->Object());
        Vector<char, 4> components;
        for (uint32_t i : s->Indices()) {
            if (TINT_UNLIKELY(i >= 4)) {
                TINT_ICE() << "invalid swizzle index: " << i;
                return;
            }
            components.Push("xyzw"[i]);
        }
        auto* swizzle =
            b.MemberAccessor(vec, std::string_view(components.begin(), components.Length()));
        Bind(s->Result(), swizzle);
    }

    void Binary(core::ir::Binary* e) {
        if (e->Op() == core::ir::BinaryOp::kEqual) {
            auto* rhs = e->RHS()->As<core::ir::Constant>();
            if (rhs && rhs->Type()->Is<core::type::Bool>() &&
                rhs->Value()->ValueAs<bool>() == false) {
                // expr == false
                Bind(e->Result(), b.Not(Expr(e->LHS())));
                return;
            }
        }
        auto* lhs = Expr(e->LHS());
        auto* rhs = Expr(e->RHS());
        const ast::Expression* expr = nullptr;
        switch (e->Op()) {
            case core::ir::BinaryOp::kAdd:
                expr = b.Add(lhs, rhs);
                break;
            case core::ir::BinaryOp::kSubtract:
                expr = b.Sub(lhs, rhs);
                break;
            case core::ir::BinaryOp::kMultiply:
                expr = b.Mul(lhs, rhs);
                break;
            case core::ir::BinaryOp::kDivide:
                expr = b.Div(lhs, rhs);
                break;
            case core::ir::BinaryOp::kModulo:
                expr = b.Mod(lhs, rhs);
                break;
            case core::ir::BinaryOp::kAnd:
                expr = b.And(lhs, rhs);
                break;
            case core::ir::BinaryOp::kOr:
                expr = b.Or(lhs, rhs);
                break;
            case core::ir::BinaryOp::kXor:
                expr = b.Xor(lhs, rhs);
                break;
            case core::ir::BinaryOp::kEqual:
                expr = b.Equal(lhs, rhs);
                break;
            case core::ir::BinaryOp::kNotEqual:
                expr = b.NotEqual(lhs, rhs);
                break;
            case core::ir::BinaryOp::kLessThan:
                expr = b.LessThan(lhs, rhs);
                break;
            case core::ir::BinaryOp::kGreaterThan:
                expr = b.GreaterThan(lhs, rhs);
                break;
            case core::ir::BinaryOp::kLessThanEqual:
                expr = b.LessThanEqual(lhs, rhs);
                break;
            case core::ir::BinaryOp::kGreaterThanEqual:
                expr = b.GreaterThanEqual(lhs, rhs);
                break;
            case core::ir::BinaryOp::kShiftLeft:
                expr = b.Shl(lhs, rhs);
                break;
            case core::ir::BinaryOp::kShiftRight:
                expr = b.Shr(lhs, rhs);
                break;
        }
        Bind(e->Result(), expr);
    }

    TINT_BEGIN_DISABLE_WARNING(UNREACHABLE_CODE);

    const ast::Expression* Expr(core::ir::Value* value, PtrKind want_ptr_kind = PtrKind::kRef) {
        using ExprAndPtrKind = std::pair<const ast::Expression*, PtrKind>;

        auto [expr, got_ptr_kind] = tint::Switch(
            value,
            [&](core::ir::Constant* c) -> ExprAndPtrKind {
                return {Constant(c), PtrKind::kRef};
            },
            [&](Default) -> ExprAndPtrKind {
                auto lookup = bindings_.Find(value);
                if (TINT_UNLIKELY(!lookup)) {
                    TINT_ICE() << "Expr(" << (value ? value->TypeInfo().name : "null")
                               << ") value has no expression";
                    return {};
                }
                return std::visit(
                    [&](auto&& got) -> ExprAndPtrKind {
                        using T = std::decay_t<decltype(got)>;

                        if constexpr (std::is_same_v<T, VariableValue>) {
                            return {b.Expr(got.name), got.ptr_kind};
                        }

                        if constexpr (std::is_same_v<T, InlinedValue>) {
                            auto result = ExprAndPtrKind{got.expr, got.ptr_kind};
                            // Single use (inlined) expression.
                            // Mark the bindings_ map entry as consumed.
                            *lookup = ConsumedValue{};
                            return result;
                        }

                        if constexpr (std::is_same_v<T, ConsumedValue>) {
                            TINT_ICE() << "Expr(" << value->TypeInfo().name
                                       << ") called twice on the same value";
                        } else {
                            TINT_ICE()
                                << "Expr(" << value->TypeInfo().name << ") has unhandled value";
                        }
                        return {};
                    },
                    *lookup);
            });

        if (!expr) {
            return b.Expr("<error>");
        }

        if (value->Type()->Is<core::type::Pointer>()) {
            return ToPtrKind(expr, got_ptr_kind, want_ptr_kind);
        }

        return expr;
    }

    TINT_END_DISABLE_WARNING(UNREACHABLE_CODE);

    const ast::Expression* Constant(core::ir::Constant* c) { return Constant(c->Value()); }

    const ast::Expression* Constant(const core::constant::Value* c) {
        auto composite = [&](bool can_splat) {
            auto ty = Type(c->Type());
            if (c->AllZero()) {
                return b.Call(ty);
            }
            if (can_splat && c->Is<core::constant::Splat>()) {
                return b.Call(ty, Constant(c->Index(0)));
            }

            Vector<const ast::Expression*, 8> els;
            for (size_t i = 0, n = c->NumElements(); i < n; i++) {
                els.Push(Constant(c->Index(i)));
            }
            return b.Call(ty, std::move(els));
        };
        return tint::Switch(
            c->Type(),  //
            [&](const core::type::I32*) { return b.Expr(c->ValueAs<i32>()); },
            [&](const core::type::U32*) { return b.Expr(c->ValueAs<u32>()); },
            [&](const core::type::F32*) { return b.Expr(c->ValueAs<f32>()); },
            [&](const core::type::F16*) {
                Enable(wgsl::Extension::kF16);
                return b.Expr(c->ValueAs<f16>());
            },
            [&](const core::type::Bool*) { return b.Expr(c->ValueAs<bool>()); },
            [&](const core::type::Array*) { return composite(/* can_splat */ false); },
            [&](const core::type::Vector*) { return composite(/* can_splat */ true); },
            [&](const core::type::Matrix*) { return composite(/* can_splat */ false); },
            [&](const core::type::Struct*) { return composite(/* can_splat */ false); },  //
            TINT_ICE_ON_NO_MATCH);
    }

    void Enable(wgsl::Extension ext) {
        if (enables_.Add(ext)) {
            b.Enable(ext);
        }
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
    ast::Type Type(const core::type::Type* ty) {
        return tint::Switch(
            ty,                                                    //
            [&](const core::type::Void*) { return ast::Type{}; },  //
            [&](const core::type::I32*) { return b.ty.i32(); },    //
            [&](const core::type::U32*) { return b.ty.u32(); },    //
            [&](const core::type::F16*) {
                Enable(wgsl::Extension::kF16);
                return b.ty.f16();
            },
            [&](const core::type::F32*) { return b.ty.f32(); },  //
            [&](const core::type::Bool*) { return b.ty.bool_(); },
            [&](const core::type::Matrix* m) {
                return b.ty.mat(Type(m->type()), m->columns(), m->rows());
            },
            [&](const core::type::Vector* v) {
                auto el = Type(v->type());
                if (v->Packed()) {
                    TINT_ASSERT(v->Width() == 3u);
                    return b.ty(core::BuiltinType::kPackedVec3, el);
                } else {
                    return b.ty.vec(el, v->Width());
                }
            },
            [&](const core::type::Array* a) {
                auto el = Type(a->ElemType());
                Vector<const ast::Attribute*, 1> attrs;
                if (!a->IsStrideImplicit()) {
                    attrs.Push(b.Stride(a->Stride()));
                }
                if (a->Count()->Is<core::type::RuntimeArrayCount>()) {
                    return b.ty.array(el, std::move(attrs));
                }
                auto count = a->ConstantCount();
                if (TINT_UNLIKELY(!count)) {
                    TINT_ICE() << core::type::Array::kErrExpectedConstantCount;
                    return b.ty.array(el, u32(1), std::move(attrs));
                }
                return b.ty.array(el, u32(count.value()), std::move(attrs));
            },
            [&](const core::type::Struct* s) { return Struct(s); },
            [&](const core::type::Atomic* a) { return b.ty.atomic(Type(a->Type())); },
            [&](const core::type::DepthTexture* t) { return b.ty.depth_texture(t->dim()); },
            [&](const core::type::DepthMultisampledTexture* t) {
                return b.ty.depth_multisampled_texture(t->dim());
            },
            [&](const core::type::ExternalTexture*) { return b.ty.external_texture(); },
            [&](const core::type::MultisampledTexture* t) {
                auto el = Type(t->type());
                return b.ty.multisampled_texture(t->dim(), el);
            },
            [&](const core::type::SampledTexture* t) {
                auto el = Type(t->type());
                return b.ty.sampled_texture(t->dim(), el);
            },
            [&](const core::type::StorageTexture* t) {
                if (t->access() == core::Access::kRead || t->access() == core::Access::kReadWrite) {
                    Enable(wgsl::Extension::kChromiumExperimentalReadWriteStorageTexture);
                }
                return b.ty.storage_texture(t->dim(), t->texel_format(), t->access());
            },
            [&](const core::type::Sampler* s) { return b.ty.sampler(s->kind()); },
            [&](const core::type::Pointer* p) {
                // Note: type::Pointer always has an inferred access, but WGSL only allows an
                // explicit access in the 'storage' address space.
                auto el = Type(p->StoreType());
                auto address_space = p->AddressSpace();
                auto access = address_space == core::AddressSpace::kStorage
                                  ? p->Access()
                                  : core::Access::kUndefined;
                return b.ty.ptr(address_space, el, access);
            },
            [&](const core::type::Reference*) {
                TINT_ICE() << "reference types should never appear in the IR";
                return b.ty.i32();
            },  //
            TINT_ICE_ON_NO_MATCH);
    }

    ast::Type Struct(const core::type::Struct* s) {
        auto n = structs_.GetOrCreate(s, [&] {
            auto members = tint::Transform<8>(s->Members(), [&](const core::type::StructMember* m) {
                auto ty = Type(m->Type());
                const auto& ir_attrs = m->Attributes();
                Vector<const ast::Attribute*, 4> ast_attrs;
                if (m->Type()->Align() != m->Align()) {
                    ast_attrs.Push(b.MemberAlign(u32(m->Align())));
                }
                if (m->Type()->Size() != m->Size()) {
                    ast_attrs.Push(b.MemberSize(u32(m->Size())));
                }
                if (auto location = ir_attrs.location) {
                    ast_attrs.Push(b.Location(u32(*location)));
                }
                if (auto index = ir_attrs.index) {
                    Enable(wgsl::Extension::kChromiumInternalDualSourceBlending);
                    ast_attrs.Push(b.Index(u32(*index)));
                }
                if (auto builtin = ir_attrs.builtin) {
                    if (RequiresSubgroups(*builtin)) {
                        Enable(wgsl::Extension::kChromiumExperimentalSubgroups);
                    }
                    ast_attrs.Push(b.Builtin(*builtin));
                }
                if (auto interpolation = ir_attrs.interpolation) {
                    ast_attrs.Push(b.Interpolate(interpolation->type, interpolation->sampling));
                }
                if (ir_attrs.invariant) {
                    ast_attrs.Push(b.Invariant());
                }
                return b.Member(m->Name().NameView(), ty, std::move(ast_attrs));
            });

            // TODO(crbug.com/tint/1902): Emit structure attributes
            Vector<const ast::Attribute*, 2> attrs;

            auto name = b.Symbols().Register(s->Name().NameView());
            b.Structure(name, std::move(members), std::move(attrs));
            return name;
        });

        return b.ty(n);
    }

    const ast::Expression* ToPtrKind(const ast::Expression* in, PtrKind got, PtrKind want) {
        if (want == PtrKind::kRef && got == PtrKind::kPtr) {
            return b.Deref(in);
        }
        if (want == PtrKind::kPtr && got == PtrKind::kRef) {
            return b.AddressOf(in);
        }
        return in;
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Bindings
    ////////////////////////////////////////////////////////////////////////////////////////////////

    /// @returns the AST name for the given value, creating and returning a new name on the first
    /// call.
    Symbol NameFor(core::ir::Value* value, std::string_view suggested = {}) {
        return names_.GetOrCreate(value, [&] {
            if (!suggested.empty()) {
                return b.Symbols().Register(suggested);
            }
            if (auto sym = mod.NameOf(value)) {
                return b.Symbols().Register(sym.NameView());
            }
            return b.Symbols().New("v");
        });
    }

    /// Associates the IR value @p value with the AST expression @p expr.
    /// @p ptr_kind defines how pointer values are represented by @p expr.
    void Bind(core::ir::Value* value,
              const ast::Expression* expr,
              PtrKind ptr_kind = PtrKind::kRef) {
        TINT_ASSERT(value);
        if (can_inline_.Remove(value)) {
            // Value will be inlined at its place of usage.
            if (TINT_LIKELY(bindings_.Add(value, InlinedValue{expr, ptr_kind}))) {
                return;
            }
        } else {
            if (value->Type()->Is<core::type::Pointer>()) {
                expr = ToPtrKind(expr, ptr_kind, PtrKind::kPtr);
            }
            auto mod_name = mod.NameOf(value);
            if (value->Usages().IsEmpty() && !mod_name.IsValid()) {
                // Value has no usages and no name.
                // Assign to a phony. These support more data types than a 'let', and avoids
                // allocation of unused names.
                Append(b.Assign(b.Phony(), expr));
            } else {
                Symbol name = NameFor(value, mod_name.NameView());
                Append(b.Decl(b.Let(name, expr)));
                Bind(value, name, PtrKind::kPtr);
            }
            return;
        }

        TINT_ICE() << "Bind(" << value->TypeInfo().name << ") called twice for same value";
    }

    /// Associates the IR value @p value with the AST 'var', 'let' or parameter with the name @p
    /// name.
    /// @p ptr_kind defines how pointer values are represented by @p expr.
    void Bind(core::ir::Value* value, Symbol name, PtrKind ptr_kind) {
        TINT_ASSERT(value);

        bool added = bindings_.Add(value, VariableValue{name, ptr_kind});
        if (TINT_UNLIKELY(!added)) {
            TINT_ICE() << "Bind(" << value->TypeInfo().name << ") called twice for same value";
        }
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Helpers
    ////////////////////////////////////////////////////////////////////////////////////////////////
    bool AsShortCircuit(core::ir::If* i,
                        const StatementList& true_stmts,
                        const StatementList& false_stmts) {
        if (!i->HasResults()) {
            return false;
        }
        auto* result = i->Result();
        if (!result->Type()->Is<core::type::Bool>()) {
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

    bool IsConstant(core::ir::Value* val, bool value) {
        if (auto* c = val->As<core::ir::Constant>()) {
            if (c->Type()->Is<core::type::Bool>()) {
                return c->Value()->ValueAs<bool>() == value;
            }
        }
        return false;
    }

    const ast::Expression* VectorMemberAccess(const ast::Expression* expr, core::ir::Value* index) {
        if (auto* c = index->As<core::ir::Constant>()) {
            switch (c->Value()->ValueAs<int>()) {
                case 0:
                    return b.MemberAccessor(expr, "x");
                case 1:
                    return b.MemberAccessor(expr, "y");
                case 2:
                    return b.MemberAccessor(expr, "z");
                case 3:
                    return b.MemberAccessor(expr, "w");
            }
        }
        return b.IndexAccessor(expr, Expr(index));
    }

    bool RequiresDerivativeUniformity(wgsl::BuiltinFn fn) {
        switch (fn) {
            case wgsl::BuiltinFn::kDpdxCoarse:
            case wgsl::BuiltinFn::kDpdyCoarse:
            case wgsl::BuiltinFn::kFwidthCoarse:
            case wgsl::BuiltinFn::kDpdxFine:
            case wgsl::BuiltinFn::kDpdyFine:
            case wgsl::BuiltinFn::kFwidthFine:
            case wgsl::BuiltinFn::kDpdx:
            case wgsl::BuiltinFn::kDpdy:
            case wgsl::BuiltinFn::kFwidth:
            case wgsl::BuiltinFn::kTextureSample:
            case wgsl::BuiltinFn::kTextureSampleBias:
            case wgsl::BuiltinFn::kTextureSampleCompare:
                return true;
            default:
                return false;
        }
    }

    /// @returns true if the builtin value requires the kChromiumExperimentalSubgroups extension to
    /// be enabled.
    bool RequiresSubgroups(core::BuiltinValue builtin) {
        switch (builtin) {
            case core::BuiltinValue::kSubgroupInvocationId:
            case core::BuiltinValue::kSubgroupSize:
                return true;
            default:
                return false;
        }
    }

    /// @returns true if a parameter of the type @p ty requires the
    /// kChromiumExperimentalFullPtrParameters extension to be enabled.
    bool ParamRequiresFullPtrParameters(const core::type::Type* ty) {
        if (auto* ptr = ty->As<core::type::Pointer>()) {
            switch (ptr->AddressSpace()) {
                case core::AddressSpace::kUniform:
                case core::AddressSpace::kStorage:
                case core::AddressSpace::kWorkgroup:
                    return true;
                default:
                    return false;
            }
        }
        return false;
    }

    /// @returns true if the argument @p arg requires the kChromiumExperimentalFullPtrParameters
    /// extension to be enabled.
    bool ArgRequiresFullPtrParameters(core::ir::Value* arg) {
        if (!arg->Type()->Is<core::type::Pointer>()) {
            return false;
        }

        auto res = arg->As<core::ir::InstructionResult>();
        while (res) {
            auto* inst = res->Source();
            if (inst->Is<core::ir::Access>()) {
                return true;  // Passing pointer into sub-object
            }
            if (auto* let = inst->As<core::ir::Let>()) {
                res = let->Value()->As<core::ir::InstructionResult>();
            } else {
                break;
            }
        }
        return false;
    }
};

}  // namespace

Program IRToProgram(core::ir::Module& i) {
    return State{i}.Run();
}

}  // namespace tint::wgsl::writer
