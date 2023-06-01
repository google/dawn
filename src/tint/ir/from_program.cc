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

#include "src/tint/ir/from_program.h"

#include <iostream>
#include <unordered_map>
#include <utility>

#include "src/tint/ast/alias.h"
#include "src/tint/ast/assignment_statement.h"
#include "src/tint/ast/binary_expression.h"
#include "src/tint/ast/bitcast_expression.h"
#include "src/tint/ast/block_statement.h"
#include "src/tint/ast/bool_literal_expression.h"
#include "src/tint/ast/break_if_statement.h"
#include "src/tint/ast/break_statement.h"
#include "src/tint/ast/call_expression.h"
#include "src/tint/ast/call_statement.h"
#include "src/tint/ast/compound_assignment_statement.h"
#include "src/tint/ast/const.h"
#include "src/tint/ast/const_assert.h"
#include "src/tint/ast/continue_statement.h"
#include "src/tint/ast/discard_statement.h"
#include "src/tint/ast/enable.h"
#include "src/tint/ast/float_literal_expression.h"
#include "src/tint/ast/for_loop_statement.h"
#include "src/tint/ast/function.h"
#include "src/tint/ast/id_attribute.h"
#include "src/tint/ast/identifier.h"
#include "src/tint/ast/identifier_expression.h"
#include "src/tint/ast/if_statement.h"
#include "src/tint/ast/increment_decrement_statement.h"
#include "src/tint/ast/int_literal_expression.h"
#include "src/tint/ast/interpolate_attribute.h"
#include "src/tint/ast/invariant_attribute.h"
#include "src/tint/ast/let.h"
#include "src/tint/ast/literal_expression.h"
#include "src/tint/ast/loop_statement.h"
#include "src/tint/ast/override.h"
#include "src/tint/ast/phony_expression.h"
#include "src/tint/ast/return_statement.h"
#include "src/tint/ast/statement.h"
#include "src/tint/ast/struct.h"
#include "src/tint/ast/struct_member_align_attribute.h"
#include "src/tint/ast/struct_member_size_attribute.h"
#include "src/tint/ast/switch_statement.h"
#include "src/tint/ast/templated_identifier.h"
#include "src/tint/ast/unary_op_expression.h"
#include "src/tint/ast/var.h"
#include "src/tint/ast/variable_decl_statement.h"
#include "src/tint/ast/while_statement.h"
#include "src/tint/ir/block_param.h"
#include "src/tint/ir/builder.h"
#include "src/tint/ir/exit_if.h"
#include "src/tint/ir/exit_loop.h"
#include "src/tint/ir/exit_switch.h"
#include "src/tint/ir/function.h"
#include "src/tint/ir/if.h"
#include "src/tint/ir/loop.h"
#include "src/tint/ir/module.h"
#include "src/tint/ir/store.h"
#include "src/tint/ir/switch.h"
#include "src/tint/ir/value.h"
#include "src/tint/program.h"
#include "src/tint/scope_stack.h"
#include "src/tint/sem/builtin.h"
#include "src/tint/sem/call.h"
#include "src/tint/sem/function.h"
#include "src/tint/sem/load.h"
#include "src/tint/sem/materialize.h"
#include "src/tint/sem/module.h"
#include "src/tint/sem/switch_statement.h"
#include "src/tint/sem/value_constructor.h"
#include "src/tint/sem/value_conversion.h"
#include "src/tint/sem/value_expression.h"
#include "src/tint/sem/variable.h"
#include "src/tint/switch.h"
#include "src/tint/type/pointer.h"
#include "src/tint/type/reference.h"
#include "src/tint/type/void.h"
#include "src/tint/utils/defer.h"
#include "src/tint/utils/result.h"
#include "src/tint/utils/scoped_assignment.h"

using namespace tint::number_suffixes;  // NOLINT

namespace tint::ir {

namespace {

using ResultType = utils::Result<Module, diag::List>;

bool IsConnected(const Block* b) {
    return b->InboundBranches().Length() > 0;
}

/// Impl is the private-implementation of FromProgram().
class Impl {
  public:
    /// Constructor
    /// @param program the program to convert to IR
    explicit Impl(const Program* program) : program_(program) {}

    /// Builds an IR module from the program passed to the constructor.
    /// @return the IR module or an error.
    ResultType Build() { return EmitModule(); }

  private:
    enum class ControlFlags { kNone, kExcludeSwitch };

    // The input Program
    const Program* program_ = nullptr;

    /// The IR module being built
    Module mod;

    /// The IR builder being used by the impl.
    Builder builder_{mod};

    // The clone context used to clone data from #program_
    constant::CloneContext clone_ctx_{
        /* type_ctx */ type::CloneContext{
            /* src */ {&program_->Symbols()},
            /* dst */ {&builder_.ir.symbols, &builder_.ir.Types()},
        },
        /* dst */ {builder_.ir.constant_values},
    };

    /// The stack of control blocks.
    utils::Vector<Branch*, 8> control_stack_;

    /// The current block for expressions.
    Block* current_block_ = nullptr;

    /// The current function being processed.
    Function* current_function_ = nullptr;

    /// The current stack of scopes being processed.
    ScopeStack<Symbol, Value*> scopes_;

    /// The diagnostic that have been raised.
    diag::List diagnostics_;

    class ControlStackScope {
      public:
        ControlStackScope(Impl* impl, Branch* b) : impl_(impl) { impl_->control_stack_.Push(b); }

        ~ControlStackScope() { impl_->control_stack_.Pop(); }

      private:
        Impl* impl_;
    };

    void add_error(const Source& s, const std::string& err) {
        diagnostics_.add_error(tint::diag::System::IR, err, s);
    }

    bool NeedBranch() { return current_block_ && !current_block_->HasBranchTarget(); }

    void SetBranch(Branch* br) {
        TINT_ASSERT(IR, current_block_);
        TINT_ASSERT(IR, !current_block_->HasBranchTarget());

        current_block_->Append(br);
        current_block_ = nullptr;
    }

    Branch* FindEnclosingControl(ControlFlags flags) {
        for (auto it = control_stack_.rbegin(); it != control_stack_.rend(); ++it) {
            if ((*it)->Is<Loop>()) {
                return *it;
            }
            if (flags == ControlFlags::kExcludeSwitch) {
                continue;
            }
            if ((*it)->Is<Switch>()) {
                return *it;
            }
        }
        return nullptr;
    }

    ResultType EmitModule() {
        auto* sem = program_->Sem().Module();

        for (auto* decl : sem->DependencyOrderedDeclarations()) {
            tint::Switch(
                decl,  //
                [&](const ast::Struct*) {
                    // Will be encoded into the `type::Struct` when used. We will then hoist all
                    // used structs up to module scope when converting IR.
                },
                [&](const ast::Alias*) {
                    // Folded away and doesn't appear in the IR.
                },
                [&](const ast::Variable* var) {
                    // Setup the current flow node to be the root block for the module. The builder
                    // will handle creating it if it doesn't exist already.
                    TINT_SCOPED_ASSIGNMENT(current_block_, builder_.CreateRootBlockIfNeeded());
                    EmitVariable(var);
                },
                [&](const ast::Function* func) { EmitFunction(func); },
                [&](const ast::Enable*) {
                    // TODO(dsinclair): Implement? I think these need to be passed along so further
                    // stages know what is enabled.
                },
                [&](const ast::ConstAssert*) {
                    // Evaluated by the resolver, drop from the IR.
                },
                [&](Default) {
                    add_error(decl->source, "unknown type: " + std::string(decl->TypeInfo().name));
                });
        }

        if (diagnostics_.contains_errors()) {
            return ResultType(std::move(diagnostics_));
        }

        return ResultType{std::move(mod)};
    }

    builtin::Interpolation ExtractInterpolation(const ast::InterpolateAttribute* interp) {
        auto type = program_->Sem()
                        .Get(interp->type)
                        ->As<sem::BuiltinEnumExpression<builtin::InterpolationType>>();
        builtin::InterpolationType interpolation_type = type->Value();

        builtin::InterpolationSampling interpolation_sampling =
            builtin::InterpolationSampling::kUndefined;
        if (interp->sampling) {
            auto sampling = program_->Sem()
                                .Get(interp->sampling)
                                ->As<sem::BuiltinEnumExpression<builtin::InterpolationSampling>>();
            interpolation_sampling = sampling->Value();
        }

        return builtin::Interpolation{interpolation_type, interpolation_sampling};
    }

    void EmitFunction(const ast::Function* ast_func) {
        // The flow stack should have been emptied when the previous function finished building.
        TINT_ASSERT(IR, control_stack_.IsEmpty());

        const auto* sem = program_->Sem().Get(ast_func);

        auto* ir_func = builder_.CreateFunction(ast_func->name->symbol.NameView(),
                                                sem->ReturnType()->Clone(clone_ctx_.type_ctx));
        current_function_ = ir_func;
        builder_.ir.functions.Push(ir_func);

        scopes_.Set(ast_func->name->symbol, ir_func);

        if (ast_func->IsEntryPoint()) {
            switch (ast_func->PipelineStage()) {
                case ast::PipelineStage::kVertex:
                    ir_func->SetStage(Function::PipelineStage::kVertex);
                    break;
                case ast::PipelineStage::kFragment:
                    ir_func->SetStage(Function::PipelineStage::kFragment);
                    break;
                case ast::PipelineStage::kCompute: {
                    ir_func->SetStage(Function::PipelineStage::kCompute);

                    auto wg_size = sem->WorkgroupSize();
                    ir_func->SetWorkgroupSize(wg_size[0].value(), wg_size[1].value_or(1),
                                              wg_size[2].value_or(1));
                    break;
                }
                default: {
                    TINT_ICE(IR, diagnostics_) << "Invalid pipeline stage";
                    return;
                }
            }

            // Note, interpolated is only valid when paired with Location, so it will only be set
            // when the location is set.
            std::optional<builtin::Interpolation> interpolation;
            for (auto* attr : ast_func->return_type_attributes) {
                tint::Switch(
                    attr,  //
                    [&](const ast::InterpolateAttribute* interp) {
                        interpolation = ExtractInterpolation(interp);
                    },
                    [&](const ast::InvariantAttribute*) { ir_func->SetReturnInvariant(true); },
                    [&](const ast::BuiltinAttribute* b) {
                        if (auto* ident_sem =
                                program_->Sem()
                                    .Get(b)
                                    ->As<sem::BuiltinEnumExpression<builtin::BuiltinValue>>()) {
                            switch (ident_sem->Value()) {
                                case builtin::BuiltinValue::kPosition:
                                    ir_func->SetReturnBuiltin(Function::ReturnBuiltin::kPosition);
                                    break;
                                case builtin::BuiltinValue::kFragDepth:
                                    ir_func->SetReturnBuiltin(Function::ReturnBuiltin::kFragDepth);
                                    break;
                                case builtin::BuiltinValue::kSampleMask:
                                    ir_func->SetReturnBuiltin(Function::ReturnBuiltin::kSampleMask);
                                    break;
                                default:
                                    TINT_ICE(IR, diagnostics_)
                                        << "Unknown builtin value in return attributes "
                                        << ident_sem->Value();
                                    return;
                            }
                        } else {
                            TINT_ICE(IR, diagnostics_) << "Builtin attribute sem invalid";
                            return;
                        }
                    });
            }
            if (sem->ReturnLocation().has_value()) {
                ir_func->SetReturnLocation(sem->ReturnLocation().value(), interpolation);
            }
        }

        scopes_.Push();
        TINT_DEFER(scopes_.Pop());

        utils::Vector<FunctionParam*, 1> params;
        for (auto* p : ast_func->params) {
            const auto* param_sem = program_->Sem().Get(p)->As<sem::Parameter>();
            auto* ty = param_sem->Type()->Clone(clone_ctx_.type_ctx);
            auto* param = builder_.FunctionParam(ty);

            // Note, interpolated is only valid when paired with Location, so it will only be set
            // when the location is set.
            std::optional<builtin::Interpolation> interpolation;
            for (auto* attr : p->attributes) {
                tint::Switch(
                    attr,  //
                    [&](const ast::InterpolateAttribute* interp) {
                        interpolation = ExtractInterpolation(interp);
                    },
                    [&](const ast::InvariantAttribute*) { param->SetInvariant(true); },
                    [&](const ast::BuiltinAttribute* b) {
                        if (auto* ident_sem =
                                program_->Sem()
                                    .Get(b)
                                    ->As<sem::BuiltinEnumExpression<builtin::BuiltinValue>>()) {
                            switch (ident_sem->Value()) {
                                case builtin::BuiltinValue::kVertexIndex:
                                    param->SetBuiltin(FunctionParam::Builtin::kVertexIndex);
                                    break;
                                case builtin::BuiltinValue::kInstanceIndex:
                                    param->SetBuiltin(FunctionParam::Builtin::kInstanceIndex);
                                    break;
                                case builtin::BuiltinValue::kPosition:
                                    param->SetBuiltin(FunctionParam::Builtin::kPosition);
                                    break;
                                case builtin::BuiltinValue::kFrontFacing:
                                    param->SetBuiltin(FunctionParam::Builtin::kFrontFacing);
                                    break;
                                case builtin::BuiltinValue::kLocalInvocationId:
                                    param->SetBuiltin(FunctionParam::Builtin::kLocalInvocationId);
                                    break;
                                case builtin::BuiltinValue::kLocalInvocationIndex:
                                    param->SetBuiltin(
                                        FunctionParam::Builtin::kLocalInvocationIndex);
                                    break;
                                case builtin::BuiltinValue::kGlobalInvocationId:
                                    param->SetBuiltin(FunctionParam::Builtin::kGlobalInvocationId);
                                    break;
                                case builtin::BuiltinValue::kWorkgroupId:
                                    param->SetBuiltin(FunctionParam::Builtin::kWorkgroupId);
                                    break;
                                case builtin::BuiltinValue::kNumWorkgroups:
                                    param->SetBuiltin(FunctionParam::Builtin::kNumWorkgroups);
                                    break;
                                case builtin::BuiltinValue::kSampleIndex:
                                    param->SetBuiltin(FunctionParam::Builtin::kSampleIndex);
                                    break;
                                case builtin::BuiltinValue::kSampleMask:
                                    param->SetBuiltin(FunctionParam::Builtin::kSampleMask);
                                    break;
                                default:
                                    TINT_ICE(IR, diagnostics_)
                                        << "Unknown builtin value in parameter attributes "
                                        << ident_sem->Value();
                                    return;
                            }
                        } else {
                            TINT_ICE(IR, diagnostics_) << "Builtin attribute sem invalid";
                            return;
                        }
                    });

                if (param_sem->Location().has_value()) {
                    param->SetLocation(param_sem->Location().value(), interpolation);
                }
                if (param_sem->BindingPoint().has_value()) {
                    param->SetBindingPoint(param_sem->BindingPoint()->group,
                                           param_sem->BindingPoint()->binding);
                }
            }

            scopes_.Set(p->name->symbol, param);
            builder_.ir.SetName(param, p->name->symbol.NameView());
            params.Push(param);
        }
        ir_func->SetParams(params);

        current_block_ = ir_func->StartTarget();
        EmitBlock(ast_func->body);

        // If the branch target has already been set then a `return` was called. Only set in
        // the case where `return` wasn't called.
        if (NeedBranch()) {
            SetBranch(builder_.Return(current_function_));
        }

        TINT_ASSERT(IR, control_stack_.IsEmpty());
        current_block_ = nullptr;
        current_function_ = nullptr;
    }

    void EmitStatements(utils::VectorRef<const ast::Statement*> stmts) {
        for (auto* s : stmts) {
            EmitStatement(s);

            // If the current flow block has a branch target then the rest of the statements in
            // this block are dead code. Skip them.
            if (!NeedBranch()) {
                break;
            }
        }
    }

    void EmitStatement(const ast::Statement* stmt) {
        tint::Switch(
            stmt,  //
            [&](const ast::AssignmentStatement* a) { EmitAssignment(a); },
            [&](const ast::BlockStatement* b) { EmitBlock(b); },
            [&](const ast::BreakStatement* b) { EmitBreak(b); },
            [&](const ast::BreakIfStatement* b) { EmitBreakIf(b); },
            [&](const ast::CallStatement* c) { EmitCall(c); },
            [&](const ast::CompoundAssignmentStatement* c) { EmitCompoundAssignment(c); },
            [&](const ast::ContinueStatement* c) { EmitContinue(c); },
            [&](const ast::DiscardStatement* d) { EmitDiscard(d); },
            [&](const ast::IfStatement* i) { EmitIf(i); },
            [&](const ast::LoopStatement* l) { EmitLoop(l); },
            [&](const ast::ForLoopStatement* l) { EmitForLoop(l); },
            [&](const ast::WhileStatement* l) { EmitWhile(l); },
            [&](const ast::ReturnStatement* r) { EmitReturn(r); },
            [&](const ast::SwitchStatement* s) { EmitSwitch(s); },
            [&](const ast::VariableDeclStatement* v) { EmitVariable(v->variable); },
            [&](const ast::IncrementDecrementStatement* i) { EmitIncrementDecrement(i); },
            [&](const ast::ConstAssert*) {
                // Not emitted
            },
            [&](Default) {
                add_error(stmt->source,
                          "unknown statement type: " + std::string(stmt->TypeInfo().name));
            });
    }

    void EmitAssignment(const ast::AssignmentStatement* stmt) {
        // If assigning to a phony, just generate the RHS and we're done. Note that, because
        // this isn't used, a subsequent transform could remove it due to it being dead code.
        // This could then change the interface for the program (i.e. a global var no longer
        // used). If that happens we have to either fix this to store to a phony value, or make
        // sure we pull the interface before doing the dead code elimination.
        if (stmt->lhs->Is<ast::PhonyExpression>()) {
            (void)EmitExpression(stmt->rhs);
            return;
        }

        auto lhs = EmitExpression(stmt->lhs);
        if (!lhs) {
            return;
        }

        auto rhs = EmitExpression(stmt->rhs);
        if (!rhs) {
            return;
        }
        auto store = builder_.Store(lhs.Get(), rhs.Get());
        current_block_->Append(store);
    }

    void EmitIncrementDecrement(const ast::IncrementDecrementStatement* stmt) {
        auto lhs = EmitExpression(stmt->lhs);
        if (!lhs) {
            return;
        }

        // Load from the LHS.
        auto* lhs_value = builder_.Load(lhs.Get());
        current_block_->Append(lhs_value);

        auto* ty = lhs_value->Type();

        auto* rhs =
            ty->is_signed_integer_scalar() ? builder_.Constant(1_i) : builder_.Constant(1_u);

        Binary* inst = nullptr;
        if (stmt->increment) {
            inst = builder_.Add(ty, lhs_value, rhs);
        } else {
            inst = builder_.Subtract(ty, lhs_value, rhs);
        }
        current_block_->Append(inst);

        auto store = builder_.Store(lhs.Get(), inst);
        current_block_->Append(store);
    }

    void EmitCompoundAssignment(const ast::CompoundAssignmentStatement* stmt) {
        auto lhs = EmitExpression(stmt->lhs);
        if (!lhs) {
            return;
        }

        auto rhs = EmitExpression(stmt->rhs);
        if (!rhs) {
            return;
        }

        // Load from the LHS.
        auto* lhs_value = builder_.Load(lhs.Get());
        current_block_->Append(lhs_value);

        auto* ty = lhs_value->Type();

        Binary* inst = nullptr;
        switch (stmt->op) {
            case ast::BinaryOp::kAnd:
                inst = builder_.And(ty, lhs_value, rhs.Get());
                break;
            case ast::BinaryOp::kOr:
                inst = builder_.Or(ty, lhs_value, rhs.Get());
                break;
            case ast::BinaryOp::kXor:
                inst = builder_.Xor(ty, lhs_value, rhs.Get());
                break;
            case ast::BinaryOp::kShiftLeft:
                inst = builder_.ShiftLeft(ty, lhs_value, rhs.Get());
                break;
            case ast::BinaryOp::kShiftRight:
                inst = builder_.ShiftRight(ty, lhs_value, rhs.Get());
                break;
            case ast::BinaryOp::kAdd:
                inst = builder_.Add(ty, lhs_value, rhs.Get());
                break;
            case ast::BinaryOp::kSubtract:
                inst = builder_.Subtract(ty, lhs_value, rhs.Get());
                break;
            case ast::BinaryOp::kMultiply:
                inst = builder_.Multiply(ty, lhs_value, rhs.Get());
                break;
            case ast::BinaryOp::kDivide:
                inst = builder_.Divide(ty, lhs_value, rhs.Get());
                break;
            case ast::BinaryOp::kModulo:
                inst = builder_.Modulo(ty, lhs_value, rhs.Get());
                break;
            case ast::BinaryOp::kLessThanEqual:
            case ast::BinaryOp::kGreaterThanEqual:
            case ast::BinaryOp::kGreaterThan:
            case ast::BinaryOp::kLessThan:
            case ast::BinaryOp::kNotEqual:
            case ast::BinaryOp::kEqual:
            case ast::BinaryOp::kLogicalAnd:
            case ast::BinaryOp::kLogicalOr:
                TINT_ICE(IR, diagnostics_) << "invalid compound assignment";
                return;
            case ast::BinaryOp::kNone:
                TINT_ICE(IR, diagnostics_) << "missing binary operand type";
                return;
        }
        current_block_->Append(inst);

        auto store = builder_.Store(lhs.Get(), inst);
        current_block_->Append(store);
    }

    void EmitBlock(const ast::BlockStatement* block) {
        scopes_.Push();
        TINT_DEFER(scopes_.Pop());

        // Note, this doesn't need to emit a Block as the current block flow node should be
        // sufficient as the blocks all get flattened. Each flow control node will inject the
        // basic blocks it requires.
        EmitStatements(block->statements);
    }

    void EmitIf(const ast::IfStatement* stmt) {
        // Emit the if condition into the end of the preceding block
        auto reg = EmitExpression(stmt->condition);
        if (!reg) {
            return;
        }
        auto* if_inst = builder_.CreateIf(reg.Get());
        current_block_->Append(if_inst);

        {
            ControlStackScope scope(this, if_inst);

            current_block_ = if_inst->True();
            EmitBlock(stmt->body);

            // If the true branch did not execute control flow, then go to the Merge().target
            if (NeedBranch()) {
                SetBranch(builder_.ExitIf(if_inst));
            }

            current_block_ = if_inst->False();
            if (stmt->else_statement) {
                EmitStatement(stmt->else_statement);
            }

            // If the false branch did not execute control flow, then go to the Merge().target
            if (NeedBranch()) {
                SetBranch(builder_.ExitIf(if_inst));
            }
        }
        current_block_ = nullptr;

        // If both branches went somewhere, then they both returned, continued or broke. So,
        // there is no need for the if merge-block and there is nothing to branch to the merge
        // block anyway.
        if (IsConnected(if_inst->Merge())) {
            current_block_ = if_inst->Merge();
        }
    }

    void EmitLoop(const ast::LoopStatement* stmt) {
        auto* loop_inst = builder_.CreateLoop();
        current_block_->Append(loop_inst);

        {
            ControlStackScope scope(this, loop_inst);
            current_block_ = loop_inst->Body();

            // The loop doesn't use EmitBlock because it needs the scope stack to not get popped
            // until after the continuing block.
            scopes_.Push();
            TINT_DEFER(scopes_.Pop());
            EmitStatements(stmt->body->statements);

            // The current block didn't `break`, `return` or `continue`, go to the continuing block.
            if (NeedBranch()) {
                SetBranch(builder_.Continue(loop_inst));
            }

            if (IsConnected(loop_inst->Continuing())) {
                // Note, even if there is no continuing block, we may have branched into the
                // continue so we have to set the current block and then emit the branch if needed
                // below otherwise empty continuing blocks will fail to branch back to the start
                // block.
                current_block_ = loop_inst->Continuing();
                if (stmt->continuing) {
                    EmitBlock(stmt->continuing);
                }
                // Branch back to the start node if the continue target didn't branch out already
                if (NeedBranch()) {
                    SetBranch(builder_.NextIteration(loop_inst));
                }
            }
        }

        // The loop merge can get disconnected if the loop returns directly, or the continuing
        // target branches, eventually, to the merge, but nothing branched to the
        // Continuing() block.
        current_block_ = loop_inst->Merge();
        if (!IsConnected(loop_inst->Merge())) {
            current_block_ = nullptr;
        }
    }

    void EmitWhile(const ast::WhileStatement* stmt) {
        auto* loop_inst = builder_.CreateLoop();
        current_block_->Append(loop_inst);

        // Continue is always empty, just go back to the start
        current_block_ = loop_inst->Continuing();
        SetBranch(builder_.NextIteration(loop_inst));

        {
            ControlStackScope scope(this, loop_inst);

            current_block_ = loop_inst->Body();

            // Emit the while condition into the Start().target of the loop
            auto reg = EmitExpression(stmt->condition);
            if (!reg) {
                return;
            }

            // Create an `if (cond) {} else {break;}` control flow
            auto* if_inst = builder_.CreateIf(reg.Get());
            current_block_->Append(if_inst);

            current_block_ = if_inst->True();
            SetBranch(builder_.ExitIf(if_inst));

            current_block_ = if_inst->False();
            SetBranch(builder_.ExitLoop(loop_inst));

            current_block_ = if_inst->Merge();
            EmitBlock(stmt->body);

            if (NeedBranch()) {
                SetBranch(builder_.Continue(loop_inst));
            }
        }
        // The while loop always has a path to the Merge().target as the break statement comes
        // before anything inside the loop.
        current_block_ = loop_inst->Merge();
    }

    void EmitForLoop(const ast::ForLoopStatement* stmt) {
        auto* loop_inst = builder_.CreateLoop();
        current_block_->Append(loop_inst);

        // Make sure the initializer ends up in a contained scope
        scopes_.Push();
        TINT_DEFER(scopes_.Pop());

        if (stmt->initializer) {
            // Emit the for initializer before branching to the loop
            EmitStatement(stmt->initializer);
        }

        {
            ControlStackScope scope(this, loop_inst);

            current_block_ = loop_inst->Body();

            if (stmt->condition) {
                // Emit the condition into the target target of the loop
                auto reg = EmitExpression(stmt->condition);
                if (!reg) {
                    return;
                }

                // Create an `if (cond) {} else {break;}` control flow
                auto* if_inst = builder_.CreateIf(reg.Get());
                current_block_->Append(if_inst);

                current_block_ = if_inst->True();
                SetBranch(builder_.ExitIf(if_inst));

                current_block_ = if_inst->False();
                SetBranch(builder_.ExitLoop(loop_inst));

                current_block_ = if_inst->Merge();
            }

            EmitBlock(stmt->body);
            if (NeedBranch()) {
                SetBranch(builder_.Continue(loop_inst));
            }

            if (stmt->continuing) {
                current_block_ = loop_inst->Continuing();
                EmitStatement(stmt->continuing);
                SetBranch(builder_.NextIteration(loop_inst));
            }
        }

        // The while loop always has a path to the Merge().target as the break statement comes
        // before anything inside the loop.
        current_block_ = loop_inst->Merge();
    }

    void EmitSwitch(const ast::SwitchStatement* stmt) {
        // Emit the condition into the preceding block
        auto reg = EmitExpression(stmt->condition);
        if (!reg) {
            return;
        }
        auto* switch_inst = builder_.CreateSwitch(reg.Get());
        current_block_->Append(switch_inst);

        {
            ControlStackScope scope(this, switch_inst);

            const auto* sem = program_->Sem().Get(stmt);
            for (const auto* c : sem->Cases()) {
                utils::Vector<Switch::CaseSelector, 4> selectors;
                for (const auto* selector : c->Selectors()) {
                    if (selector->IsDefault()) {
                        selectors.Push({nullptr});
                    } else {
                        selectors.Push({builder_.Constant(selector->Value()->Clone(clone_ctx_))});
                    }
                }

                current_block_ = builder_.CreateCase(switch_inst, selectors);
                EmitBlock(c->Body()->Declaration());

                if (NeedBranch()) {
                    SetBranch(builder_.ExitSwitch(switch_inst));
                }
            }
        }
        current_block_ = nullptr;

        if (IsConnected(switch_inst->Merge())) {
            current_block_ = switch_inst->Merge();
        }
    }

    void EmitReturn(const ast::ReturnStatement* stmt) {
        utils::Vector<Value*, 1> ret_value;
        if (stmt->value) {
            auto ret = EmitExpression(stmt->value);
            if (!ret) {
                return;
            }
            ret_value.Push(ret.Get());
        }
        SetBranch(builder_.Return(current_function_, std::move(ret_value)));
    }

    void EmitBreak(const ast::BreakStatement*) {
        auto* current_control = FindEnclosingControl(ControlFlags::kNone);
        TINT_ASSERT(IR, current_control);

        if (auto* c = current_control->As<Loop>()) {
            SetBranch(builder_.ExitLoop(c));
        } else if (auto* s = current_control->As<Switch>()) {
            SetBranch(builder_.ExitSwitch(s));
        } else {
            TINT_UNREACHABLE(IR, diagnostics_);
        }
    }

    void EmitContinue(const ast::ContinueStatement*) {
        auto* current_control = FindEnclosingControl(ControlFlags::kExcludeSwitch);
        TINT_ASSERT(IR, current_control);

        if (auto* c = current_control->As<Loop>()) {
            SetBranch(builder_.Continue(c));
        } else {
            TINT_UNREACHABLE(IR, diagnostics_);
        }
    }

    // Discard is being treated as an instruction. The semantics in WGSL is demote_to_helper, so
    // the code has to continue as before it just predicates writes. If WGSL grows some kind of
    // terminating discard that would probably make sense as a Block but would then require
    // figuring out the multi-level exit that is triggered.
    void EmitDiscard(const ast::DiscardStatement*) {
        auto* inst = builder_.Discard();
        current_block_->Append(inst);
    }

    void EmitBreakIf(const ast::BreakIfStatement* stmt) {
        auto* current_control = FindEnclosingControl(ControlFlags::kExcludeSwitch);

        // Emit the break-if condition into the end of the preceding block
        auto cond = EmitExpression(stmt->condition);
        if (!cond) {
            return;
        }
        SetBranch(builder_.BreakIf(cond.Get(), current_control->As<ir::Loop>()));
    }

    utils::Result<Value*> EmitExpression(const ast::Expression* expr) {
        // If this is a value that has been const-eval'd return the result.
        auto* sem = program_->Sem().GetVal(expr);
        if (sem) {
            if (auto* v = sem->ConstantValue()) {
                if (auto* cv = v->Clone(clone_ctx_)) {
                    return builder_.Constant(cv);
                }
            }
        }

        auto result = tint::Switch(
            expr,
            // [&](const ast::IndexAccessorExpression* a) {
            // TODO(dsinclair): Implement
            // },
            [&](const ast::BinaryExpression* b) { return EmitBinary(b); },
            [&](const ast::BitcastExpression* b) { return EmitBitcast(b); },
            [&](const ast::CallExpression* c) { return EmitCall(c); },
            [&](const ast::IdentifierExpression* i) -> utils::Result<Value*> {
                auto* v = scopes_.Get(i->identifier->symbol);
                if (TINT_UNLIKELY(!v)) {
                    add_error(expr->source,
                              "unable to find identifier " + i->identifier->symbol.Name());
                    return utils::Failure;
                }
                return {v};
            },
            [&](const ast::LiteralExpression* l) { return EmitLiteral(l); },
            // [&](const ast::MemberAccessorExpression* m) {
            // TODO(dsinclair): Implement
            // },
            [&](const ast::UnaryOpExpression* u) { return EmitUnary(u); },
            // Note, ast::PhonyExpression is explicitly not handled here as it should never get
            // into this method. The assignment statement should have filtered it out already.
            [&](Default) {
                add_error(expr->source,
                          "unknown expression type: " + std::string(expr->TypeInfo().name));
                return utils::Failure;
            });

        // If this expression maps to sem::Load, insert a load instruction to get the result.
        if (result && sem->Is<sem::Load>()) {
            auto* load = builder_.Load(result.Get());
            current_block_->Append(load);
            return load;
        }

        return result;
    }

    void EmitVariable(const ast::Variable* var) {
        auto* sem = program_->Sem().Get(var);

        return tint::Switch(  //
            var,
            [&](const ast::Var* v) {
                auto* ref = sem->Type()->As<type::Reference>();
                auto* ty = builder_.ir.Types().Get<type::Pointer>(
                    ref->StoreType()->Clone(clone_ctx_.type_ctx), ref->AddressSpace(),
                    ref->Access());

                auto* val = builder_.Declare(ty);
                if (v->initializer) {
                    auto init = EmitExpression(v->initializer);
                    if (!init) {
                        return;
                    }
                    val->SetInitializer(init.Get());
                }
                current_block_->Append(val);

                if (auto* gv = sem->As<sem::GlobalVariable>(); gv && var->HasBindingPoint()) {
                    val->SetBindingPoint(gv->BindingPoint().value().group,
                                         gv->BindingPoint().value().binding);
                }

                // Store the declaration so we can get the instruction to store too
                scopes_.Set(v->name->symbol, val);

                // Record the original name of the var
                builder_.ir.SetName(val, v->name->symbol.Name());
            },
            [&](const ast::Let* l) {
                // A `let` doesn't exist as a standalone item in the IR, it's just the result of
                // the initializer.
                auto init = EmitExpression(l->initializer);
                if (!init) {
                    return;
                }

                // Store the results of the initialization
                scopes_.Set(l->name->symbol, init.Get());

                // Record the original name of the let
                builder_.ir.SetName(init.Get(), l->name->symbol.Name());
            },
            [&](const ast::Override*) {
                add_error(var->source,
                          "found an `Override` variable. The SubstituteOverrides "
                          "transform must be run before converting to IR");
            },
            [&](const ast::Const*) {
                // Skip. This should be handled by const-eval already, so the const will be a
                // `constant::` value at the usage sites. Can just ignore the `const` variable
                // as it should never be used.
                //
                // TODO(dsinclair): Probably want to store the const variable somewhere and then
                // in identifier expression log an error if we ever see a const identifier. Add
                // this when identifiers and variables are supported.
            },
            [&](Default) {
                add_error(var->source, "unknown variable: " + std::string(var->TypeInfo().name));
            });
    }

    utils::Result<Value*> EmitUnary(const ast::UnaryOpExpression* expr) {
        auto val = EmitExpression(expr->expr);
        if (!val) {
            return utils::Failure;
        }

        auto* sem = program_->Sem().Get(expr);
        auto* ty = sem->Type()->Clone(clone_ctx_.type_ctx);

        Instruction* inst = nullptr;
        switch (expr->op) {
            case ast::UnaryOp::kAddressOf:
            case ast::UnaryOp::kIndirection:
                // 'address-of' and 'indirection' just fold away and we propagate the pointer.
                return val;
            case ast::UnaryOp::kComplement:
                inst = builder_.Complement(ty, val.Get());
                break;
            case ast::UnaryOp::kNegation:
                inst = builder_.Negation(ty, val.Get());
                break;
            case ast::UnaryOp::kNot:
                inst = builder_.Not(ty, val.Get());
                break;
        }

        current_block_->Append(inst);
        return inst;
    }

    // A short-circut needs special treatment. The short-circuit is decomposed into the relevant
    // if statements and declarations.
    utils::Result<Value*> EmitShortCircuit(const ast::BinaryExpression* expr) {
        switch (expr->op) {
            case ast::BinaryOp::kLogicalAnd:
            case ast::BinaryOp::kLogicalOr:
                break;
            default:
                TINT_ICE(IR, diagnostics_)
                    << "invalid operation type for short-circut decomposition";
                return utils::Failure;
        }

        // Evaluate the LHS of the short-circuit
        auto lhs = EmitExpression(expr->lhs);
        if (!lhs) {
            return utils::Failure;
        }

        auto* if_inst = builder_.CreateIf(lhs.Get());
        current_block_->Append(if_inst);

        auto* result = builder_.BlockParam(builder_.ir.Types().bool_());
        if_inst->Merge()->SetParams(utils::Vector{result});

        utils::Result<Value*> rhs;
        {
            ControlStackScope scope(this, if_inst);

            utils::Vector<Value*, 1> alt_args;
            alt_args.Push(lhs.Get());

            // If this is an `&&` then we only evaluate the RHS expression in the true block.
            // If this is an `||` then we only evaluate the RHS expression in the false block.
            if (expr->op == ast::BinaryOp::kLogicalAnd) {
                // If the lhs is false, then that is the result we want to pass to the merge
                // block as our argument
                current_block_ = if_inst->False();
                SetBranch(builder_.ExitIf(if_inst, std::move(alt_args)));

                current_block_ = if_inst->True();
            } else {
                // If the lhs is true, then that is the result we want to pass to the merge
                // block as our argument
                current_block_ = if_inst->True();
                SetBranch(builder_.ExitIf(if_inst, std::move(alt_args)));

                current_block_ = if_inst->False();
            }

            rhs = EmitExpression(expr->rhs);
            if (!rhs) {
                return utils::Failure;
            }
            utils::Vector<Value*, 1> args;
            args.Push(rhs.Get());

            SetBranch(builder_.ExitIf(if_inst, std::move(args)));
        }
        current_block_ = if_inst->Merge();

        return result;
    }

    utils::Result<Value*> EmitBinary(const ast::BinaryExpression* expr) {
        if (expr->op == ast::BinaryOp::kLogicalAnd || expr->op == ast::BinaryOp::kLogicalOr) {
            return EmitShortCircuit(expr);
        }

        auto lhs = EmitExpression(expr->lhs);
        if (!lhs) {
            return utils::Failure;
        }

        auto rhs = EmitExpression(expr->rhs);
        if (!rhs) {
            return utils::Failure;
        }

        auto* sem = program_->Sem().Get(expr);
        auto* ty = sem->Type()->Clone(clone_ctx_.type_ctx);

        Binary* inst = nullptr;
        switch (expr->op) {
            case ast::BinaryOp::kAnd:
                inst = builder_.And(ty, lhs.Get(), rhs.Get());
                break;
            case ast::BinaryOp::kOr:
                inst = builder_.Or(ty, lhs.Get(), rhs.Get());
                break;
            case ast::BinaryOp::kXor:
                inst = builder_.Xor(ty, lhs.Get(), rhs.Get());
                break;
            case ast::BinaryOp::kEqual:
                inst = builder_.Equal(ty, lhs.Get(), rhs.Get());
                break;
            case ast::BinaryOp::kNotEqual:
                inst = builder_.NotEqual(ty, lhs.Get(), rhs.Get());
                break;
            case ast::BinaryOp::kLessThan:
                inst = builder_.LessThan(ty, lhs.Get(), rhs.Get());
                break;
            case ast::BinaryOp::kGreaterThan:
                inst = builder_.GreaterThan(ty, lhs.Get(), rhs.Get());
                break;
            case ast::BinaryOp::kLessThanEqual:
                inst = builder_.LessThanEqual(ty, lhs.Get(), rhs.Get());
                break;
            case ast::BinaryOp::kGreaterThanEqual:
                inst = builder_.GreaterThanEqual(ty, lhs.Get(), rhs.Get());
                break;
            case ast::BinaryOp::kShiftLeft:
                inst = builder_.ShiftLeft(ty, lhs.Get(), rhs.Get());
                break;
            case ast::BinaryOp::kShiftRight:
                inst = builder_.ShiftRight(ty, lhs.Get(), rhs.Get());
                break;
            case ast::BinaryOp::kAdd:
                inst = builder_.Add(ty, lhs.Get(), rhs.Get());
                break;
            case ast::BinaryOp::kSubtract:
                inst = builder_.Subtract(ty, lhs.Get(), rhs.Get());
                break;
            case ast::BinaryOp::kMultiply:
                inst = builder_.Multiply(ty, lhs.Get(), rhs.Get());
                break;
            case ast::BinaryOp::kDivide:
                inst = builder_.Divide(ty, lhs.Get(), rhs.Get());
                break;
            case ast::BinaryOp::kModulo:
                inst = builder_.Modulo(ty, lhs.Get(), rhs.Get());
                break;
            case ast::BinaryOp::kLogicalAnd:
            case ast::BinaryOp::kLogicalOr:
                TINT_ICE(IR, diagnostics_) << "short circuit op should have already been handled";
                return utils::Failure;
            case ast::BinaryOp::kNone:
                TINT_ICE(IR, diagnostics_) << "missing binary operand type";
                return utils::Failure;
        }

        current_block_->Append(inst);
        return inst;
    }

    utils::Result<Value*> EmitBitcast(const ast::BitcastExpression* expr) {
        auto val = EmitExpression(expr->expr);
        if (!val) {
            return utils::Failure;
        }

        auto* sem = program_->Sem().Get(expr);
        auto* ty = sem->Type()->Clone(clone_ctx_.type_ctx);
        auto* inst = builder_.Bitcast(ty, val.Get());

        current_block_->Append(inst);
        return inst;
    }

    void EmitCall(const ast::CallStatement* stmt) { (void)EmitCall(stmt->expr); }

    utils::Result<Value*> EmitCall(const ast::CallExpression* expr) {
        // If this is a materialized semantic node, just use the constant value.
        if (auto* mat = program_->Sem().Get(expr)) {
            if (mat->ConstantValue()) {
                auto* cv = mat->ConstantValue()->Clone(clone_ctx_);
                if (!cv) {
                    add_error(expr->source, "failed to get constant value for call " +
                                                std::string(expr->TypeInfo().name));
                    return utils::Failure;
                }
                return builder_.Constant(cv);
            }
        }

        utils::Vector<Value*, 8> args;
        args.Reserve(expr->args.Length());

        // Emit the arguments
        for (const auto* arg : expr->args) {
            auto value = EmitExpression(arg);
            if (!value) {
                add_error(arg->source, "failed to convert arguments");
                return utils::Failure;
            }
            args.Push(value.Get());
        }

        auto* sem = program_->Sem().Get<sem::Call>(expr);
        if (!sem) {
            add_error(expr->source, "failed to get semantic information for call " +
                                        std::string(expr->TypeInfo().name));
            return utils::Failure;
        }

        auto* ty = sem->Target()->ReturnType()->Clone(clone_ctx_.type_ctx);

        Instruction* inst = nullptr;

        // If this is a builtin function, emit the specific builtin value
        if (auto* b = sem->Target()->As<sem::Builtin>()) {
            inst = builder_.Builtin(ty, b->Type(), args);
        } else if (sem->Target()->As<sem::ValueConstructor>()) {
            inst = builder_.Construct(ty, std::move(args));
        } else if (auto* conv = sem->Target()->As<sem::ValueConversion>()) {
            auto* from = conv->Source()->Clone(clone_ctx_.type_ctx);
            inst = builder_.Convert(ty, from, std::move(args));
        } else if (expr->target->identifier->Is<ast::TemplatedIdentifier>()) {
            TINT_UNIMPLEMENTED(IR, diagnostics_) << "missing templated ident support";
            return utils::Failure;
        } else {
            // Not a builtin and not a templated call, so this is a user function.
            inst = builder_.UserCall(
                ty, scopes_.Get(expr->target->identifier->symbol)->As<ir::Function>(),
                std::move(args));
        }
        if (inst == nullptr) {
            return utils::Failure;
        }
        current_block_->Append(inst);
        return inst;
    }

    utils::Result<Value*> EmitLiteral(const ast::LiteralExpression* lit) {
        auto* sem = program_->Sem().Get(lit);
        if (!sem) {
            add_error(lit->source, "failed to get semantic information for node " +
                                       std::string(lit->TypeInfo().name));
            return utils::Failure;
        }

        auto* cv = sem->ConstantValue()->Clone(clone_ctx_);
        if (!cv) {
            add_error(lit->source,
                      "failed to get constant value for node " + std::string(lit->TypeInfo().name));
            return utils::Failure;
        }
        return builder_.Constant(cv);
    }
};

}  // namespace

utils::Result<Module, std::string> FromProgram(const Program* program) {
    if (!program->IsValid()) {
        return std::string("input program is not valid");
    }

    Impl b(program);
    auto r = b.Build();
    if (!r) {
        return r.Failure().str();
    }

    return r.Move();
}

}  // namespace tint::ir
