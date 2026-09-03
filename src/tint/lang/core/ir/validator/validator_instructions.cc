// Copyright 2026 The Dawn & Tint Authors
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

#include <string_view>

#include "src/tint/lang/core/binary_op.h"
#include "src/tint/lang/core/intrinsic/table.h"
#include "src/tint/lang/core/ir/constexpr_if.h"
#include "src/tint/lang/core/ir/multi_in_block.h"
#include "src/tint/lang/core/ir/terminate_invocation.h"
#include "src/tint/lang/core/ir/validator/validator.h"
#include "src/tint/lang/core/type/array.h"
#include "src/tint/lang/core/type/bool.h"
#include "src/tint/lang/core/type/pointer.h"
#include "src/tint/lang/core/type/vector.h"
#include "src/tint/lang/core/type/void.h"
#include "src/tint/utils/containers/predicates.h"
#include "src/tint/utils/containers/reverse.h"
#include "src/tint/utils/containers/transform.h"
#include "src/tint/utils/result.h"

namespace tint::core::ir::validator {
namespace {

/// @returns the parent block of @p block
const Block* ParentBlockOf(const Block* block) {
    if (auto* parent = block->Parent()) {
        return parent->Block();
    }
    return nullptr;
}

/// @returns true if @p block directly or transitively holds the instruction @p inst
bool TransitivelyHolds(const Block* block, const Instruction* inst) {
    for (auto* b = inst->Block(); b; b = ParentBlockOf(b)) {
        if (b == block) {
            return true;
        }
    }
    return false;
}

}  // namespace
void Validator::CheckInstruction(const Instruction* inst) {
    visited_instructions_.Add(inst);
    if (!inst->Alive()) {
        AddError(inst) << "destroyed instruction found in instruction list";
        return;
    }

    auto results = inst->Results();
    for (size_t i = 0; i < results.Length(); ++i) {
        auto* res = results[i];
        if (!res) {
            continue;
        }

        CheckType(res->Type(), [&]() -> diag::Diagnostic& { return AddResultError(inst, i); });
    }

    auto ops = inst->Operands();
    for (size_t i = 0; i < ops.Length(); ++i) {
        auto* op = ops[i];
        if (!op) {
            continue;
        }

        CheckType(op->Type(), [&]() -> diag::Diagnostic& { return AddError(inst, i); });
    }

    // Push a task to add the results to the scope.
    // This ensures that for control instructions, the results are only added to the scope
    // after their nested blocks have been evaluated (since tasks are processed LIFO).
    tasks_.Push([this, inst] {
        for (auto* result : inst->Results()) {
            if (result) {
                scope_stack_.Add(result);
            }
        }
    });

    tint::Switch(
        inst,                                                              //
        [&](const Access* a) { CheckAccess(a); },                          //
        [&](const Binary* b) { CheckBinary(b); },                          //
        [&](const Call* c) { CheckCall(c); },                              //
        [&](const If* if_) { CheckIf(if_); },                              //
        [&](const Let* let) { CheckLet(let); },                            //
        [&](const Load* load) { CheckLoad(load); },                        //
        [&](const LoadVectorElement* l) { CheckLoadVectorElement(l); },    //
        [&](const Loop* l) { CheckLoop(l); },                              //
        [&](const Phony* p) { CheckPhony(p); },                            //
        [&](const Store* s) { CheckStore(s); },                            //
        [&](const StoreVectorElement* s) { CheckStoreVectorElement(s); },  //
        [&](const Switch* s) { CheckSwitch(s); },                          //
        [&](const Swizzle* s) { CheckSwizzle(s); },                        //
        [&](const Terminator* b) { CheckTerminator(b); },                  //
        [&](const Unary* u) { CheckUnary(u); },                            //
        [&](const Override* o) { CheckOverride(o); },                      //
        [&](const Var* var) { CheckVar(var); },                            //
        TINT_ICE_ON_NO_MATCH);
}

void Validator::CheckOverride(const Override* o) {
    if (!CheckResultsAndOperands(o, Override::kNumResults, Override::kNumOperands)) {
        return;
    }

    if (o->Block() != ir_.root_block) {
        AddError(o) << "override must be declared at module scope";
    }
}

void Validator::CheckVar(const Var* var) {
    if (!CheckResultsAndOperands(var, Var::kNumResults, Var::kNumOperands)) {
        return;
    }

    auto* result_type = var->Result()->Type();
    auto* mv = result_type->As<core::type::MemoryView>();
    if (!mv) {
        AddError(var) << "result type " << NameOf(result_type)
                      << " must be a pointer or a reference";
        return;
    }
    const type::ValueArrayCount* count = nullptr;
    if (auto* ary = result_type->UnwrapPtr()->As<core::type::Array>()) {
        count = ary->Count()->As<type::ValueArrayCount>();
    } else if (auto* buf = result_type->UnwrapPtr()->As<core::type::Buffer>()) {
        count = buf->Count()->As<type::ValueArrayCount>();
    }

    if (count) {
        if (!scope_stack_.Contains(count->value)) {
            AddError(var) << NameOf(count->value) << " is not in scope";
        }
    }

    if (var->Initializer()) {
        if (!CheckOperand(var, ir::Var::kInitializerOperandOffset)) {
            return;
        }
    }

    CheckBindingPoint(var->Result(), var->Result(0)->Type(), var->Attributes(),
                      ShaderIOKind::kModuleScopeVar);

    auto address_space = mv->AddressSpace();
    if (address_space != AddressSpace::kIn && address_space != AddressSpace::kOut) {
        CheckInterpolation(var->Result(), mv->StoreType(), var->Attributes(),
                           Function::PipelineStage::kUndefined, IODirection::kResource);
    }

    if (var->Block() == ir_.root_block) {
        if (mv->AddressSpace() == AddressSpace::kIn || mv->AddressSpace() == AddressSpace::kOut) {
            ValidateShaderIOAnnotations(var->Result(), var->Result()->Type(), var->BindingPoint(),
                                        var->Attributes(), ShaderIOKind::kModuleScopeVar);
        }
    }
}

void Validator::CheckLet(const Let* l) {
    if (!CheckResultsAndOperands(l, Let::kNumResults, Let::kNumOperands)) {
        return;
    }
}

void Validator::CheckCall(const Call* call) {
    tint::Switch(
        call,                                                            //
        [&](const BuiltinCall* c) { CheckBuiltinCall(c); },              //
        [&](const MemberBuiltinCall* c) { CheckMemberBuiltinCall(c); },  //
        [&](const Construct* c) { CheckConstruct(c); },                  //
        [&](const Convert* c) { CheckConvert(c); },                      //
        [&](const Discard* d) {                                          //
            stage_restricted_instructions_.Add(
                d, SupportedStages{Function::PipelineStage::kFragment});        //
            CheckDiscard(d);                                                    //
        },                                                                      //
        [&](const UserCall* c) {                                                //
            if (c->Target()) {                                                  //
                auto calls =                                                    //
                    user_func_calls_.GetOr(c->Target(),                         //
                                           Hashset<const ir::UserCall*, 4>{});  //
                calls.Add(c);                                                   //
                user_func_calls_.Replace(c->Target(), calls);                   //
            }
            CheckUserCall(c);
        },
        [&](Default) {
            // Validation of custom IR instructions
        });
}

void Validator::CheckBuiltinCall(const BuiltinCall* call) {
    // This check cannot be more precise, since until intrinsic lookup below, it is unknown what
    // number of operands are expected, but still need to enforce things are in scope,
    // have types, etc.
    if (!CheckResults(call, BuiltinCall::kNumResults) || !CheckOperands(call)) {
        return;
    }

    auto args = Transform<8>(call->Args(), [&](const ir::Value* v) { return v->Type(); });

    intrinsic::Context context{call->TableData(), type_mgr_, symbols_};
    auto builtin = core::intrinsic::LookupFn(context, call->FriendlyName().c_str(), call->FuncId(),
                                             call->ExplicitTemplateParams(), args,
                                             core::EvaluationStage::kRuntime);
    if (builtin != Success) {
        AddError(call) << builtin.Failure();
        return;
    }

    // Track the stages that this builtin call is limited to, so that we can check them against the
    // entry points that they are used from.
    SupportedStages stages;
    if (builtin->info->flags.Contains(intrinsic::OverloadFlag::kSupportsComputePipeline)) {
        stages.Add(Function::PipelineStage::kCompute);
    }
    if (builtin->info->flags.Contains(intrinsic::OverloadFlag::kSupportsFragmentPipeline)) {
        stages.Add(Function::PipelineStage::kFragment);
    }
    if (builtin->info->flags.Contains(intrinsic::OverloadFlag::kSupportsVertexPipeline)) {
        stages.Add(Function::PipelineStage::kVertex);
    }
    stage_restricted_instructions_.Add(call, stages);

    const CoreBuiltinCall* bc = call->As<CoreBuiltinCall>();
    if (bc == nullptr) {
        return;
    }
    CheckCoreBuiltinCall(bc);
}

void Validator::CheckCoreBuiltinCall(const CoreBuiltinCall* call) {
    if (ir_.properties.Contains(Property::kDisallowVectorMinMaxClamp)) {
        switch (call->Func()) {
            case core::BuiltinFn::kClamp:
            case core::BuiltinFn::kMax:
            case core::BuiltinFn::kMin:
                if (call->Result()->Type()->Is<core::type::Vector>()) {
                    AddError(call) << "vector " << call->FriendlyName()
                                   << " disallowed by the DisallowVectorMinMaxClamp property";
                }
                break;
            default:
                break;
        }
    }
    if (ir_.properties.Contains(Property::kAllowBufferTypes)) {
        switch (call->Func()) {
            case core::BuiltinFn::kBufferArrayView:
                if (call->Result()->Type()->UnwrapPtr()->HasFixedFootprint()) {
                    AddError(call)
                        << call->FriendlyName() << " result type must not have a fixed footprint";
                }
                break;
            default:
                break;
        }
    }
}

void Validator::CheckMemberBuiltinCall(const MemberBuiltinCall* call) {
    // This check cannot be more precise, since until intrinsic lookup below, it is unknown what
    // number of operands are expected, but still need to enforce things are in scope,
    // have types, etc.
    CheckResults(call, MemberBuiltinCall::kNumResults) || !CheckOperands(call);
}

void Validator::CheckConstruct(const Construct* construct) {
    if (!CheckResultsAndOperandRange(construct, Construct::kNumResults, Construct::kMinOperands)) {
        return;
    }
}

void Validator::CheckConvert(const Convert* convert) {
    if (!CheckResultsAndOperands(convert, Convert::kNumResults, Convert::kNumOperands)) {
        return;
    }
}

void Validator::CheckDiscard(const Discard* discard) {
    if (!CheckResultsAndOperands(discard, Discard::kNumResults, Discard::kNumOperands)) {
        return;
    }
}

void Validator::CheckUserCall(const UserCall* call) {
    if (!CheckResultsAndOperandRange(call, UserCall::kNumResults, UserCall::kMinOperands)) {
        return;
    }

    if (!call->Target()) {
        AddError(call, UserCall::kFunctionOperandOffset) << "target not defined or not a function";
        return;
    }
    if (call->Target()->IsEntryPoint()) {
        AddError(call, UserCall::kFunctionOperandOffset)
            << "call target must not have a pipeline stage";
    }

    if (call->Target()->ReturnType() != call->Result()->Type()) {
        AddError(call) << "result type does not match function return type";
        return;
    }

    auto args = call->Args();
    auto params = call->Target()->Params();
    if (args.size() != params.Length()) {
        AddError(call, UserCall::kFunctionOperandOffset)
            << "function has " << params.Length() << " parameters, but call provides "
            << args.size() << " arguments";
        return;
    }

    for (size_t i = 0; i < args.size(); i++) {
        bool allow_mismatch = false;
        if (auto* arg_buffer_ty = args[i]->Type()->UnwrapPtrOrRef()->As<core::type::Buffer>()) {
            auto* arg_ptr_ty = args[i]->Type()->As<core::type::Pointer>();
            if (auto* param_ptr_ty = params[i]->Type()->As<core::type::Pointer>()) {
                if (auto* param_buffer_ty =
                        param_ptr_ty->UnwrapPtrOrRef()->As<core::type::Buffer>()) {
                    allow_mismatch = arg_ptr_ty->AddressSpace() == param_ptr_ty->AddressSpace() &&
                                     arg_ptr_ty->Access() == param_ptr_ty->Access();
                    const bool both_constant =
                        arg_buffer_ty->Count()->Is<core::type::ConstantArrayCount>() &&
                        param_buffer_ty->Count()->Is<core::type::ConstantArrayCount>();
                    uint32_t arg_size = arg_buffer_ty->ConstantCount().value_or(0);
                    uint32_t param_size = param_buffer_ty->ConstantCount().value_or(0);
                    allow_mismatch &=
                        param_buffer_ty->Count()->Is<core::type::RuntimeArrayCount>() ||
                        (both_constant && param_size < arg_size);
                }
            }
        }
        if (!allow_mismatch && args[i]->Type() != params[i]->Type()) {
            AddError(call, UserCall::kArgsOperandOffset + i)
                << "type " << NameOf(params[i]->Type()) << " of function parameter " << i
                << " does not match argument type " << NameOf(args[i]->Type());
        }
    }
}

void Validator::CheckAccess(const Access* a) {
    if (!CheckResultsAndOperandRange(a, Access::kNumResults, Access::kMinNumOperands)) {
        return;
    }
}

void Validator::CheckBinary(const Binary* b) {
    if (!CheckResultsAndOperands(b, Binary::kNumResults, Binary::kNumOperands)) {
        return;
    }
    if (b->Op() == core::BinaryOp::kLogicalAnd) {
        AddError(b) << "logical-and is not valid in the IR";
        return;
    }
    if (b->Op() == core::BinaryOp::kLogicalOr) {
        AddError(b) << "logical-or is not valid in the IR";
        return;
    }
}

void Validator::CheckUnary(const Unary* u) {
    if (!CheckResultsAndOperands(u, Unary::kNumResults, Unary::kNumOperands)) {
        return;
    }
}

void Validator::CheckIf(const If* if_) {
    CheckResults(if_);
    CheckOperands(if_, If::kNumOperands);

    if (if_->False() && if_->False()->Is<MultiInBlock>()) {
        AddError(if_) << "if false block must be a block";
    }
    if (if_->True() && if_->True()->Is<MultiInBlock>()) {
        AddError(if_) << "if true block must be a block";
    }

    if (auto* constexpr_if = if_->As<ConstExprIf>()) {
        if (constexpr_if->Results().Length() != 1) {
            AddError(constexpr_if) << "constexpr_if must have exactly one result";
        } else if (!constexpr_if->Result(0)->Type()->Is<core::type::Bool>()) {
            AddError(constexpr_if) << "constexpr_if result type must be 'bool'";
        }
        if (constexpr_if->False()->IsEmpty()) {
            AddError(constexpr_if) << "constexpr_if must have a false block";
        } else if (!constexpr_if->False()->Terminator() ||
                   !constexpr_if->False()->Terminator()->Is<ExitIf>()) {
            AddError(constexpr_if->False())
                << "constexpr_if false block terminator must be an exit_if";
        }
        if (!constexpr_if->True()->Terminator() ||
            !constexpr_if->True()->Terminator()->Is<ExitIf>()) {
            AddError(constexpr_if->True())
                << "constexpr_if true block terminator must be an exit_if";
        }
    }

    QueueTasks(
        PushControlStack(if_),
        [this, if_] {
            if (!if_->False()->IsEmpty()) {
                QueueBlock(if_->False());
            }
            QueueBlock(if_->True());
        },
        PopControlStack());
}

void Validator::CheckLoop(const Loop* l) {
    CheckResults(l);
    CheckOperands(l, 0);

    if (l->Initializer()->Is<MultiInBlock>()) {
        AddError(l->Initializer()) << "loop initializer must be a block";
    }

    if (!l->Initializer()->IsEmpty()) {
        if (!l->Initializer()->Terminator() ||
            !l->Initializer()->Terminator()->Is<NextIteration>()) {
            AddError(l->Initializer()) << "loop initializer must have a NextIteration terminator";
        }
    }

    if (!l->Body()->Params().IsEmpty()) {
        if (!l->HasInitializer()) {
            AddError(l) << "loop with body block parameters must have an initializer";
        }
    }

    if (l->Body()->IsEmpty()) {
        AddError(l->Body()) << "loop body block must not be empty";
    }

    if (l->Continuing()->IsEmpty()) {
        if (!l->Continuing()->Params().IsEmpty()) {
            AddError(l) << "loop continuing block has parameters but is empty";
        }
    } else if (!l->Continuing()->Terminator()->IsAnyOf<NextIteration, BreakIf>()) {
        AddError(l->Continuing())
            << "loop continuing terminator can only be next_iteration or break_if";
    }

    // ⎡Initializer              ⎤
    // ⎢    ⎡Body               ⎤⎥
    // ⎣    ⎣    [Continuing ]  ⎦⎦
    QueueTasks(PushControlStack(l),
               QueueNestedTasks(  //
                   BeginBlockTask(l->Initializer()),
                   QueueNestedTasks(  //
                       BeginBlockTask(l->Body()),
                       QueueNestedTasks(                     //
                           BeginBlockTask(l->Continuing()),  //
                           [] {},                            //
                           EndBlockTask(l->Continuing())),
                       EndBlockTask(l->Body())),
                   EndBlockTask(l->Initializer())),
               PopControlStack());
}

void Validator::CheckSwitch(const Switch* s) {
    CheckResults(s);
    CheckOperands(s, Switch::kNumOperands);

    QueueTasks(
        PushControlStack(s),
        [this, s] {
            for (auto& cse : s->Cases()) {
                if (cse.selectors.IsEmpty()) {
                    AddError(s) << "case does not have any selectors";
                }
                if (cse.block->Is<MultiInBlock>()) {
                    AddError(s) << "case block must be a block";
                }
                QueueBlock(cse.block);
            }
        },
        PopControlStack());
}

void Validator::CheckSwizzle(const Swizzle* s) {
    if (!CheckResultsAndOperands(s, Swizzle::kNumResults, Swizzle::kNumOperands)) {
        return;
    }
}

void Validator::CheckTerminator(const Terminator* b) {
    // All terminators should have zero results
    if (!CheckResults(b, 0)) {
        return;
    }

    // Operands must be alive and in scope if they are not nullptr.
    if (!CheckOperands(b)) {
        return;
    }

    tint::Switch(
        b,                                                           //
        [&](const ir::BreakIf* i) { CheckBreakIf(i); },              //
        [&](const ir::Continue* c) { CheckContinue(c); },            //
        [&](const ir::Exit* e) { CheckExit(e); },                    //
        [&](const ir::NextIteration* n) { CheckNextIteration(n); },  //
        [&](const ir::Return* ret) { CheckReturn(ret); },            //
        [&](const ir::TerminateInvocation*) {},                      //
        [&](const ir::Unreachable* u) { CheckUnreachable(u); },      //
        TINT_ICE_ON_NO_MATCH);

    if (b->next) {
        AddError(b) << "must be the last instruction in the block";
    }
}

void Validator::CheckBreakIf(const BreakIf* b) {
    auto* loop = b->Loop();
    if (loop == nullptr) {
        AddError(b) << "has no associated loop";
        return;
    }
    if (b->Condition() == nullptr) {
        AddError(b) << "break_if condition cannot be nullptr";
        return;
    }

    auto next_iter_values = b->NextIterValues();
    if (auto* body = loop->Body()) {
        CheckOperandsMatchTarget(b, b->ArgsOperandOffset(), next_iter_values.size(), body,
                                 body->Params());
    }

    auto exit_values = b->ExitValues();
    CheckOperandsMatchTarget(b, b->ArgsOperandOffset() + next_iter_values.size(),
                             exit_values.size(), loop, loop->Results());

    if (!b->Condition()->Type() || !b->Condition()->Type()->Is<core::type::Bool>()) {
        AddError(b) << "condition must be a 'bool'";
        return;
    }

    if (loop->Continuing() != b->Block()) {
        AddError(b) << "must only be called directly from loop continuing";
    }
}

void Validator::CheckContinue(const Continue* c) {
    auto* loop = c->Loop();
    if (loop == nullptr) {
        AddError(c) << "has no associated loop";
        return;
    }
    if (!TransitivelyHolds(loop->Body(), c)) {
        if (control_stack_.Any(Eq<const ControlInstruction*>(loop))) {
            AddError(c) << "must only be called from loop body";
        } else {
            AddError(c) << "called outside of associated loop";
        }
    }

    if (auto* cont = loop->Continuing()) {
        CheckOperandsMatchTarget(c, Continue::kArgsOperandOffset, c->Args().size(), cont,
                                 cont->Params());
    }
}

void Validator::CheckExit(const Exit* e) {
    if (control_stack_.IsEmpty()) {
        AddError(e) << "found outside all control instructions";
        return;
    }
    if (e->ControlInstruction() == nullptr) {
        AddError(e) << "has no parent control instruction";
        return;
    }

    auto args = e->Args();
    CheckOperandsMatchTarget(e, e->ArgsOperandOffset(), args.size(), e->ControlInstruction(),
                             e->ControlInstruction()->Results());

    tint::Switch(
        e,                                                     //
        [&](const ir::ExitIf* i) { CheckExitIf(i); },          //
        [&](const ir::ExitLoop* l) { CheckExitLoop(l); },      //
        [&](const ir::ExitSwitch* s) { CheckExitSwitch(s); },  //
        TINT_ICE_ON_NO_MATCH);
}

void Validator::CheckNextIteration(const NextIteration* n) {
    auto* loop = n->Loop();
    if (loop == nullptr) {
        AddError(n) << "has no associated loop";
        return;
    }

    if (loop->Initializer() != n->Block() && loop->Continuing() != n->Block()) {
        if (control_stack_.Any(Eq<const ControlInstruction*>(loop))) {
            AddError(n) << "must only be called directly from loop initializer or continuing";
        } else {
            AddError(n) << "called outside of associated loop";
        }
    }

    if (auto* body = loop->Body()) {
        CheckOperandsMatchTarget(n, NextIteration::kArgsOperandOffset, n->Args().size(), body,
                                 body->Params());
    }
}

void Validator::CheckExitIf(const ExitIf* e) {
    if (control_stack_.Back() != e->If()) {
        AddError(e) << "if target jumps over other control instructions";
        AddNote(control_stack_.Back()) << "first control instruction jumped";
    }
}

void Validator::CheckReturn(const Return* ret) {
    if (!CheckOperands(ret, Return::kMinOperands, Return::kMaxOperands)) {
        return;
    }

    auto* func = ret->Func();
    if (func == nullptr) {
        // Func() returning nullptr after CheckResultsAndOperandRange is due to the first
        // operand being not a function
        AddError(ret) << "expected function for first operand";
        return;
    }

    if (func != ContainingFunction(ret)) {
        AddError(ret) << "function operand does not match containing function";
        return;
    }

    if (func->ReturnType()->Is<core::type::Void>()) {
        if (ret->HasValue()) {
            AddError(ret) << "unexpected return value";
        }
        return;
    }

    if (!ret->Value()) {
        AddError(ret) << "expected return value";
    } else if (ret->Value()->Type() != func->ReturnType()) {
        AddError(ret) << "return value type " << NameOf(ret->Value()->Type())
                      << " does not match function return type " << NameOf(func->ReturnType());
    }
}

void Validator::CheckUnreachable(const Unreachable* u) {
    if (!CheckResultsAndOperands(u, Unreachable::kNumResults, Unreachable::kNumOperands)) {
        return;
    }
}

void Validator::CheckControlsAllowingIf(const Exit* exit, const Instruction* control) {
    bool found = false;
    for (auto ctrl : tint::Reverse(control_stack_)) {
        if (ctrl == control) {
            found = true;
            break;
        }
        // A exit switch can step over if instructions, but no others.
        if (!ctrl->Is<ir::If>()) {
            AddError(exit) << control->FriendlyName()
                           << " target jumps over other control instructions";
            AddNote(ctrl) << "first control instruction jumped";
            return;
        }
    }
    if (!found) {
        AddError(exit) << control->FriendlyName() << " not found in parent control instructions";
    }
}

void Validator::CheckExitSwitch(const ExitSwitch* s) {
    CheckControlsAllowingIf(s, s->ControlInstruction());
}

void Validator::CheckExitLoop(const ExitLoop* l) {
    CheckControlsAllowingIf(l, l->ControlInstruction());

    const Instruction* inst = l;
    const Loop* control = l->Loop();
    while (inst) {
        // Found parent loop
        if (inst->Block()->Parent() == control) {
            if (inst->Block() == control->Continuing()) {
                AddError(l) << "loop exit jumps out of continuing block";
                if (control->Continuing() != l->Block()) {
                    AddNote(control->Continuing()) << "in continuing block";
                }
            } else if (inst->Block() == control->Initializer()) {
                AddError(l) << "loop exit not permitted in loop initializer";
                if (control->Initializer() != l->Block()) {
                    AddNote(control->Initializer()) << "in initializer block";
                }
            }
            break;
        }
        inst = inst->Block()->Parent();
    }
}

void Validator::CheckLoad(const Load* l) {
    if (!CheckResultsAndOperands(l, Load::kNumResults, Load::kNumOperands)) {
        return;
    }
}

void Validator::CheckStore(const Store* s) {
    if (!CheckResultsAndOperands(s, Store::kNumResults, Store::kNumOperands)) {
        return;
    }
}

const core::type::Type* Validator::GetVectorPtrElementType(const Instruction* inst, size_t idx) {
    auto* operand = inst->Operands()[idx];
    TINT_ASSERT(operand) << "missing element operand";

    auto* type = operand->Type();
    TINT_ASSERT(type) << "missing operand type";

    auto* memory_view_ty = type->As<core::type::MemoryView>();
    if (DAWN_LIKELY(memory_view_ty)) {
        auto* vec_ty = memory_view_ty->StoreType()->As<core::type::Vector>();
        if (DAWN_LIKELY(vec_ty)) {
            return vec_ty->Type();
        }
    }

    AddError(inst, idx) << "operand " << NameOf(type) << " must be a pointer to a vector";
    return nullptr;
}

void Validator::CheckLoadVectorElement(const LoadVectorElement* l) {
    if (!CheckResultsAndOperands(l, LoadVectorElement::kNumResults,
                                 LoadVectorElement::kNumOperands)) {
        return;
    }

    const core::type::Type* el_ty =
        GetVectorPtrElementType(l, LoadVectorElement::kFromOperandOffset);
    if (!el_ty) {
        return;
    }

    auto* res = l->Result(0);
    if (res->Type() != el_ty) {
        AddError(l) << "result type " << NameOf(res->Type())
                    << " does not match vector pointer element type " << NameOf(el_ty);
        return;
    }

    if (!l->Index()->Type()->IsIntegerScalar()) {
        AddError(l, LoadVectorElement::kIndexOperandOffset)
            << "load vector element index must be an integer scalar";
    }
    if (auto* c = l->Index()->As<Constant>()) {
        uint32_t val = c->Value()->ValueAs<uint32_t>();

        const core::type::Vector* vec_ty =
            l->From()->Type()->UnwrapPtrOrRef()->As<core::type::Vector>();
        TINT_ASSERT(vec_ty);

        if (val >= vec_ty->Width()) {
            AddError(l, LoadVectorElement::kIndexOperandOffset)
                << "load vector element index must be in range [0, " << (vec_ty->Width() - 1)
                << "]";
        }
    }
}

void Validator::CheckStoreVectorElement(const StoreVectorElement* s) {
    if (!CheckResultsAndOperands(s, StoreVectorElement::kNumResults,
                                 StoreVectorElement::kNumOperands)) {
        return;
    }

    const core::type::Type* el_ty =
        GetVectorPtrElementType(s, StoreVectorElement::kToOperandOffset);
    if (!el_ty) {
        return;
    }
    auto* value = s->Value();
    if (value->Type() != el_ty) {
        AddError(s, StoreVectorElement::kValueOperandOffset)
            << "value type " << NameOf(value->Type())
            << " does not match vector pointer element type " << NameOf(el_ty);
        return;
    }

    // The `GetVectorPtrElementType` has already validated that the pointer exists.
    const core::type::MemoryView* mv = s->To()->Type()->As<core::type::MemoryView>();
    if (mv->Access() != core::Access::kWrite && mv->Access() != core::Access::kReadWrite) {
        AddError(s, StoreVectorElement::kToOperandOffset)
            << "store_vector_element target operand has a non-writeable access type, "
            << style::Literal(ToString(mv->Access()));
        return;
    }

    if (!s->Index()->Type()->IsIntegerScalar()) {
        AddError(s, StoreVectorElement::kIndexOperandOffset)
            << "store vector element index must be an integer scalar";
    }

    const Constant* c = s->Index()->As<Constant>();
    if (c == nullptr) {
        return;
    }

    uint32_t val = c->Value()->ValueAs<uint32_t>();
    const core::type::Vector* vec_ty = s->To()->Type()->UnwrapPtrOrRef()->As<core::type::Vector>();
    TINT_ASSERT(vec_ty);

    if (val >= vec_ty->Width()) {
        AddError(s, StoreVectorElement::kIndexOperandOffset)
            << "store vector element index must be in range [0, " << (vec_ty->Width() - 1) << "]";
    }
}

void Validator::CheckPhony(const Phony* p) {
    if (!ir_.properties.Contains(Property::kAllowPhonyInstructions)) {
        AddError(p) << "missing property 'kAllowPhonyInstructions'";
        return;
    }

    if (!CheckResultsAndOperands(p, Phony::kNumResults, Phony::kNumOperands)) {
        return;
    }
}

}  // namespace tint::core::ir::validator
