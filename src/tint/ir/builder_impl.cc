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

#include "src/tint/ir/builder_impl.h"

#include <iostream>

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
#include "src/tint/ast/float_literal_expression.h"
#include "src/tint/ast/for_loop_statement.h"
#include "src/tint/ast/function.h"
#include "src/tint/ast/id_attribute.h"
#include "src/tint/ast/identifier.h"
#include "src/tint/ast/identifier_expression.h"
#include "src/tint/ast/if_statement.h"
#include "src/tint/ast/int_literal_expression.h"
#include "src/tint/ast/invariant_attribute.h"
#include "src/tint/ast/let.h"
#include "src/tint/ast/literal_expression.h"
#include "src/tint/ast/loop_statement.h"
#include "src/tint/ast/override.h"
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
#include "src/tint/ir/function.h"
#include "src/tint/ir/if.h"
#include "src/tint/ir/loop.h"
#include "src/tint/ir/module.h"
#include "src/tint/ir/store.h"
#include "src/tint/ir/switch.h"
#include "src/tint/ir/value.h"
#include "src/tint/program.h"
#include "src/tint/sem/builtin.h"
#include "src/tint/sem/call.h"
#include "src/tint/sem/function.h"
#include "src/tint/sem/materialize.h"
#include "src/tint/sem/module.h"
#include "src/tint/sem/switch_statement.h"
#include "src/tint/sem/value_constructor.h"
#include "src/tint/sem/value_conversion.h"
#include "src/tint/sem/value_expression.h"
#include "src/tint/sem/variable.h"
#include "src/tint/switch.h"
#include "src/tint/type/void.h"
#include "src/tint/utils/defer.h"
#include "src/tint/utils/scoped_assignment.h"

namespace tint::ir {
namespace {

using ResultType = utils::Result<Module>;

class FlowStackScope {
  public:
    FlowStackScope(BuilderImpl* impl, FlowNode* node) : impl_(impl) {
        impl_->flow_stack.Push(node);
    }

    ~FlowStackScope() { impl_->flow_stack.Pop(); }

  private:
    BuilderImpl* impl_;
};

bool IsBranched(const Block* b) {
    return b->branch.target != nullptr;
}

bool IsConnected(const FlowNode* b) {
    // Function is always connected as it's the start.
    if (b->Is<ir::Function>()) {
        return true;
    }

    for (auto* parent : b->inbound_branches) {
        if (IsConnected(parent)) {
            return true;
        }
    }
    // Getting here means all the incoming branches are disconnected.
    return false;
}

}  // namespace

BuilderImpl::BuilderImpl(const Program* program)
    : program_(program),
      clone_ctx_{
          type::CloneContext{{&program->Symbols()}, {&builder.ir.symbols, &builder.ir.types}},
          {&builder.ir.constants}} {}

BuilderImpl::~BuilderImpl() = default;

void BuilderImpl::add_error(const Source& s, const std::string& err) {
    diagnostics_.add_error(tint::diag::System::IR, err, s);
}

void BuilderImpl::BranchTo(FlowNode* node, utils::VectorRef<Value*> args) {
    TINT_ASSERT(IR, current_flow_block);
    TINT_ASSERT(IR, !IsBranched(current_flow_block));

    builder.Branch(current_flow_block, node, args);
    current_flow_block = nullptr;
}

void BuilderImpl::BranchToIfNeeded(FlowNode* node) {
    if (!current_flow_block || IsBranched(current_flow_block)) {
        return;
    }
    BranchTo(node);
}

FlowNode* BuilderImpl::FindEnclosingControl(ControlFlags flags) {
    for (auto it = flow_stack.rbegin(); it != flow_stack.rend(); ++it) {
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

Symbol BuilderImpl::CloneSymbol(Symbol sym) const {
    return clone_ctx_.type_ctx.dst.st->Register(sym.Name());
}

ResultType BuilderImpl::Build() {
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
                // Setup the current flow node to be the root block for the module. The builder will
                // handle creating it if it doesn't exist already.
                TINT_SCOPED_ASSIGNMENT(current_flow_block, builder.CreateRootBlockIfNeeded());
                EmitVariable(var);
            },
            [&](const ast::Function* func) { EmitFunction(func); },
            // [&](const ast::Enable*) {
            // TODO(dsinclair): Implement? I think these need to be passed along so further stages
            // know what is enabled.
            // },
            [&](const ast::ConstAssert*) {
                // Evaluated by the resolver, drop from the IR.
            },
            [&](Default) {
                add_error(decl->source, "unknown type: " + std::string(decl->TypeInfo().name));
            });
    }
    if (!diagnostics_.empty()) {
        return utils::Failure;
    }

    return ResultType{std::move(builder.ir)};
}

void BuilderImpl::EmitFunction(const ast::Function* ast_func) {
    // The flow stack should have been emptied when the previous function finished building.
    TINT_ASSERT(IR, flow_stack.IsEmpty());

    auto* ir_func = builder.CreateFunction();
    ir_func->name = CloneSymbol(ast_func->name->symbol);
    current_function_ = ir_func;
    builder.ir.functions.Push(ir_func);

    ast_to_flow_[ast_func] = ir_func;

    const auto* sem = program_->Sem().Get(ast_func);
    if (ast_func->IsEntryPoint()) {
        builder.ir.entry_points.Push(ir_func);

        switch (ast_func->PipelineStage()) {
            case ast::PipelineStage::kVertex:
                ir_func->pipeline_stage = Function::PipelineStage::kVertex;
                break;
            case ast::PipelineStage::kFragment:
                ir_func->pipeline_stage = Function::PipelineStage::kFragment;
                break;
            case ast::PipelineStage::kCompute: {
                ir_func->pipeline_stage = Function::PipelineStage::kCompute;

                auto wg_size = sem->WorkgroupSize();
                ir_func->workgroup_size = {
                    wg_size[0].value(),
                    wg_size[1].value_or(1),
                    wg_size[2].value_or(1),
                };
                break;
            }
            default: {
                TINT_ICE(IR, diagnostics_) << "Invalid pipeline stage";
                return;
            }
        }

        for (auto* attr : ast_func->return_type_attributes) {
            tint::Switch(
                attr,  //
                [&](const ast::LocationAttribute*) {
                    ir_func->return_attributes.Push(Function::ReturnAttribute::kLocation);
                },
                [&](const ast::InvariantAttribute*) {
                    ir_func->return_attributes.Push(Function::ReturnAttribute::kInvariant);
                },
                [&](const ast::BuiltinAttribute* b) {
                    if (auto* ident_sem =
                            program_->Sem()
                                .Get(b)
                                ->As<sem::BuiltinEnumExpression<builtin::BuiltinValue>>()) {
                        switch (ident_sem->Value()) {
                            case builtin::BuiltinValue::kPosition:
                                ir_func->return_attributes.Push(
                                    Function::ReturnAttribute::kPosition);
                                break;
                            case builtin::BuiltinValue::kFragDepth:
                                ir_func->return_attributes.Push(
                                    Function::ReturnAttribute::kFragDepth);
                                break;
                            case builtin::BuiltinValue::kSampleMask:
                                ir_func->return_attributes.Push(
                                    Function::ReturnAttribute::kSampleMask);
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
    }
    ir_func->return_type = sem->ReturnType()->Clone(clone_ctx_.type_ctx);
    ir_func->return_location = sem->ReturnLocation();

    {
        FlowStackScope scope(this, ir_func);

        current_flow_block = ir_func->start_target;
        EmitBlock(ast_func->body);

        // TODO(dsinclair): Store return type and attributes
        // TODO(dsinclair): Store parameters

        // If the branch target has already been set then a `return` was called. Only set in the
        // case where `return` wasn't called.
        BranchToIfNeeded(current_function_->end_target);
    }

    TINT_ASSERT(IR, flow_stack.IsEmpty());
    current_flow_block = nullptr;
    current_function_ = nullptr;
}

void BuilderImpl::EmitStatements(utils::VectorRef<const ast::Statement*> stmts) {
    for (auto* s : stmts) {
        EmitStatement(s);

        // If the current flow block has a branch target then the rest of the statements in this
        // block are dead code. Skip them.
        if (!current_flow_block || IsBranched(current_flow_block)) {
            break;
        }
    }
}

void BuilderImpl::EmitStatement(const ast::Statement* stmt) {
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
        [&](const ast::ConstAssert*) {
            // Not emitted
        },
        [&](Default) {
            add_error(stmt->source,
                      "unknown statement type: " + std::string(stmt->TypeInfo().name));
        });
}

void BuilderImpl::EmitAssignment(const ast::AssignmentStatement* stmt) {
    auto lhs = EmitExpression(stmt->lhs);
    if (!lhs) {
        return;
    }

    auto rhs = EmitExpression(stmt->rhs);
    if (!rhs) {
        return;
    }
    auto store = builder.Store(lhs.Get(), rhs.Get());
    current_flow_block->instructions.Push(store);
}

void BuilderImpl::EmitCompoundAssignment(const ast::CompoundAssignmentStatement* stmt) {
    auto lhs = EmitExpression(stmt->lhs);
    if (!lhs) {
        return;
    }

    auto rhs = EmitExpression(stmt->rhs);
    if (!rhs) {
        return;
    }

    auto* ty = lhs.Get()->Type();
    Binary* inst = nullptr;
    switch (stmt->op) {
        case ast::BinaryOp::kAnd:
            inst = builder.And(ty, lhs.Get(), rhs.Get());
            break;
        case ast::BinaryOp::kOr:
            inst = builder.Or(ty, lhs.Get(), rhs.Get());
            break;
        case ast::BinaryOp::kXor:
            inst = builder.Xor(ty, lhs.Get(), rhs.Get());
            break;
        case ast::BinaryOp::kShiftLeft:
            inst = builder.ShiftLeft(ty, lhs.Get(), rhs.Get());
            break;
        case ast::BinaryOp::kShiftRight:
            inst = builder.ShiftRight(ty, lhs.Get(), rhs.Get());
            break;
        case ast::BinaryOp::kAdd:
            inst = builder.Add(ty, lhs.Get(), rhs.Get());
            break;
        case ast::BinaryOp::kSubtract:
            inst = builder.Subtract(ty, lhs.Get(), rhs.Get());
            break;
        case ast::BinaryOp::kMultiply:
            inst = builder.Multiply(ty, lhs.Get(), rhs.Get());
            break;
        case ast::BinaryOp::kDivide:
            inst = builder.Divide(ty, lhs.Get(), rhs.Get());
            break;
        case ast::BinaryOp::kModulo:
            inst = builder.Modulo(ty, lhs.Get(), rhs.Get());
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
    current_flow_block->instructions.Push(inst);

    auto store = builder.Store(lhs.Get(), inst);
    current_flow_block->instructions.Push(store);
}

void BuilderImpl::EmitBlock(const ast::BlockStatement* block) {
    scopes_.Push();
    TINT_DEFER(scopes_.Pop());

    // Note, this doesn't need to emit a Block as the current block flow node should be sufficient
    // as the blocks all get flattened. Each flow control node will inject the basic blocks it
    // requires.
    EmitStatements(block->statements);
}

void BuilderImpl::EmitIf(const ast::IfStatement* stmt) {
    auto* if_node = builder.CreateIf();

    // Emit the if condition into the end of the preceding block
    auto reg = EmitExpression(stmt->condition);
    if (!reg) {
        return;
    }
    if_node->condition = reg.Get();

    BranchTo(if_node);

    ast_to_flow_[stmt] = if_node;

    {
        FlowStackScope scope(this, if_node);

        current_flow_block = if_node->true_.target->As<Block>();
        EmitBlock(stmt->body);

        // If the true branch did not execute control flow, then go to the merge target
        BranchToIfNeeded(if_node->merge.target);

        current_flow_block = if_node->false_.target->As<Block>();
        if (stmt->else_statement) {
            EmitStatement(stmt->else_statement);
        }

        // If the false branch did not execute control flow, then go to the merge target
        BranchToIfNeeded(if_node->merge.target);
    }
    current_flow_block = nullptr;

    // If both branches went somewhere, then they both returned, continued or broke. So, there is no
    // need for the if merge-block and there is nothing to branch to the merge block anyway.
    if (IsConnected(if_node->merge.target)) {
        current_flow_block = if_node->merge.target->As<Block>();
    }
}

void BuilderImpl::EmitLoop(const ast::LoopStatement* stmt) {
    auto* loop_node = builder.CreateLoop();

    BranchTo(loop_node);

    ast_to_flow_[stmt] = loop_node;

    {
        FlowStackScope scope(this, loop_node);

        current_flow_block = loop_node->start.target->As<Block>();
        EmitBlock(stmt->body);

        // The current block didn't `break`, `return` or `continue`, go to the continuing block.
        BranchToIfNeeded(loop_node->continuing.target);

        current_flow_block = loop_node->continuing.target->As<Block>();
        if (stmt->continuing) {
            EmitBlock(stmt->continuing);
        }

        // Branch back to the start node if the continue target didn't branch out already
        BranchToIfNeeded(loop_node->start.target);
    }

    // The loop merge can get disconnected if the loop returns directly, or the continuing target
    // branches, eventually, to the merge, but nothing branched to the continuing target.
    current_flow_block = loop_node->merge.target->As<Block>();
    if (!IsConnected(loop_node->merge.target)) {
        current_flow_block = nullptr;
    }
}

void BuilderImpl::EmitWhile(const ast::WhileStatement* stmt) {
    auto* loop_node = builder.CreateLoop();
    // Continue is always empty, just go back to the start
    TINT_ASSERT(IR, loop_node->continuing.target->Is<Block>());
    builder.Branch(loop_node->continuing.target->As<Block>(), loop_node->start.target,
                   utils::Empty);

    BranchTo(loop_node);

    ast_to_flow_[stmt] = loop_node;

    {
        FlowStackScope scope(this, loop_node);

        current_flow_block = loop_node->start.target->As<Block>();

        // Emit the while condition into the start target of the loop
        auto reg = EmitExpression(stmt->condition);
        if (!reg) {
            return;
        }

        // Create an `if (cond) {} else {break;}` control flow
        auto* if_node = builder.CreateIf();
        TINT_ASSERT(IR, if_node->true_.target->Is<Block>());
        builder.Branch(if_node->true_.target->As<Block>(), if_node->merge.target, utils::Empty);

        TINT_ASSERT(IR, if_node->false_.target->Is<Block>());
        builder.Branch(if_node->false_.target->As<Block>(), loop_node->merge.target, utils::Empty);
        if_node->condition = reg.Get();

        BranchTo(if_node);

        current_flow_block = if_node->merge.target->As<Block>();
        EmitBlock(stmt->body);

        BranchToIfNeeded(loop_node->continuing.target);
    }
    // The while loop always has a path to the merge target as the break statement comes before
    // anything inside the loop.
    current_flow_block = loop_node->merge.target->As<Block>();
}

void BuilderImpl::EmitForLoop(const ast::ForLoopStatement* stmt) {
    auto* loop_node = builder.CreateLoop();
    TINT_ASSERT(IR, loop_node->continuing.target->Is<Block>());
    builder.Branch(loop_node->continuing.target->As<Block>(), loop_node->start.target,
                   utils::Empty);

    // Make sure the initializer ends up in a contained scope
    scopes_.Push();
    TINT_DEFER(scopes_.Pop());

    if (stmt->initializer) {
        // Emit the for initializer before branching to the loop
        EmitStatement(stmt->initializer);
    }

    BranchTo(loop_node);

    ast_to_flow_[stmt] = loop_node;

    {
        FlowStackScope scope(this, loop_node);

        current_flow_block = loop_node->start.target->As<Block>();

        if (stmt->condition) {
            // Emit the condition into the target target of the loop
            auto reg = EmitExpression(stmt->condition);
            if (!reg) {
                return;
            }

            // Create an `if (cond) {} else {break;}` control flow
            auto* if_node = builder.CreateIf();
            TINT_ASSERT(IR, if_node->true_.target->Is<Block>());
            builder.Branch(if_node->true_.target->As<Block>(), if_node->merge.target, utils::Empty);

            TINT_ASSERT(IR, if_node->false_.target->Is<Block>());
            builder.Branch(if_node->false_.target->As<Block>(), loop_node->merge.target,
                           utils::Empty);
            if_node->condition = reg.Get();

            BranchTo(if_node);
            current_flow_block = if_node->merge.target->As<Block>();
        }

        EmitBlock(stmt->body);
        BranchToIfNeeded(loop_node->continuing.target);

        if (stmt->continuing) {
            current_flow_block = loop_node->continuing.target->As<Block>();
            EmitStatement(stmt->continuing);
        }
    }

    // The while loop always has a path to the merge target as the break statement comes before
    // anything inside the loop.
    current_flow_block = loop_node->merge.target->As<Block>();
}

void BuilderImpl::EmitSwitch(const ast::SwitchStatement* stmt) {
    auto* switch_node = builder.CreateSwitch();

    // Emit the condition into the preceding block
    auto reg = EmitExpression(stmt->condition);
    if (!reg) {
        return;
    }
    switch_node->condition = reg.Get();

    BranchTo(switch_node);

    ast_to_flow_[stmt] = switch_node;

    {
        FlowStackScope scope(this, switch_node);

        const auto* sem = program_->Sem().Get(stmt);
        for (const auto* c : sem->Cases()) {
            utils::Vector<Switch::CaseSelector, 4> selectors;
            for (const auto* selector : c->Selectors()) {
                if (selector->IsDefault()) {
                    selectors.Push({nullptr});
                } else {
                    selectors.Push({builder.Constant(selector->Value()->Clone(clone_ctx_))});
                }
            }

            current_flow_block = builder.CreateCase(switch_node, selectors);
            EmitBlock(c->Body()->Declaration());

            BranchToIfNeeded(switch_node->merge.target);
        }
    }
    current_flow_block = nullptr;

    if (IsConnected(switch_node->merge.target)) {
        current_flow_block = switch_node->merge.target->As<Block>();
    }
}

void BuilderImpl::EmitReturn(const ast::ReturnStatement* stmt) {
    utils::Vector<Value*, 1> ret_value;
    if (stmt->value) {
        auto ret = EmitExpression(stmt->value);
        if (!ret) {
            return;
        }
        ret_value.Push(ret.Get());
    }

    BranchTo(current_function_->end_target, std::move(ret_value));
}

void BuilderImpl::EmitBreak(const ast::BreakStatement*) {
    auto* current_control = FindEnclosingControl(ControlFlags::kNone);
    TINT_ASSERT(IR, current_control);

    if (auto* c = current_control->As<Loop>()) {
        BranchTo(c->merge.target);
    } else if (auto* s = current_control->As<Switch>()) {
        BranchTo(s->merge.target);
    } else {
        TINT_UNREACHABLE(IR, diagnostics_);
    }
}

void BuilderImpl::EmitContinue(const ast::ContinueStatement*) {
    auto* current_control = FindEnclosingControl(ControlFlags::kExcludeSwitch);
    TINT_ASSERT(IR, current_control);

    if (auto* c = current_control->As<Loop>()) {
        BranchTo(c->continuing.target);
    } else {
        TINT_UNREACHABLE(IR, diagnostics_);
    }
}

// Discard is being treated as an instruction. The semantics in WGSL is demote_to_helper, so the
// code has to continue as before it just predicates writes. If WGSL grows some kind of terminating
// discard that would probably make sense as a FlowNode but would then require figuring out the
// multi-level exit that is triggered.
void BuilderImpl::EmitDiscard(const ast::DiscardStatement*) {
    auto* inst = builder.Discard();
    current_flow_block->instructions.Push(inst);
}

void BuilderImpl::EmitBreakIf(const ast::BreakIfStatement* stmt) {
    auto* if_node = builder.CreateIf();

    // Emit the break-if condition into the end of the preceding block
    auto reg = EmitExpression(stmt->condition);
    if (!reg) {
        return;
    }
    if_node->condition = reg.Get();

    BranchTo(if_node);

    ast_to_flow_[stmt] = if_node;

    auto* current_control = FindEnclosingControl(ControlFlags::kExcludeSwitch);
    TINT_ASSERT(IR, current_control);
    TINT_ASSERT(IR, current_control->Is<Loop>());

    auto* loop = current_control->As<Loop>();

    current_flow_block = if_node->true_.target->As<Block>();
    BranchTo(loop->merge.target);

    current_flow_block = if_node->false_.target->As<Block>();
    BranchTo(if_node->merge.target);

    current_flow_block = if_node->merge.target->As<Block>();

    // The `break-if` has to be the last item in the continuing block. The false branch of the
    // `break-if` will always take us back to the start of the loop.
    BranchTo(loop->start.target);
}

utils::Result<Value*> BuilderImpl::EmitExpression(const ast::Expression* expr) {
    // If this is a value that has been const-eval'd return the result.
    if (auto* sem = program_->Sem().Get(expr)->As<sem::ValueExpression>()) {
        if (auto* v = sem->ConstantValue()) {
            if (auto* cv = v->Clone(clone_ctx_)) {
                return builder.Constant(cv);
            }
        }
    }

    return tint::Switch(
        expr,
        // [&](const ast::IndexAccessorExpression* a) {
        // TODO(dsinclair): Implement
        // },
        [&](const ast::BinaryExpression* b) { return EmitBinary(b); },
        [&](const ast::BitcastExpression* b) { return EmitBitcast(b); },
        [&](const ast::CallExpression* c) { return EmitCall(c); },
        [&](const ast::IdentifierExpression* i) {
            auto* v = scopes_.Get(i->identifier->symbol);
            return utils::Result<Value*>{v};
        },
        [&](const ast::LiteralExpression* l) { return EmitLiteral(l); },
        // [&](const ast::MemberAccessorExpression* m) {
        // TODO(dsinclair): Implement
        // },
        // [&](const ast::PhonyExpression*) {
        // TODO(dsinclair): Implement. The call may have side effects so has to be made.
        // },
        [&](const ast::UnaryOpExpression* u) { return EmitUnary(u); },
        [&](Default) {
            add_error(expr->source,
                      "unknown expression type: " + std::string(expr->TypeInfo().name));
            return utils::Failure;
        });
}

void BuilderImpl::EmitVariable(const ast::Variable* var) {
    auto* sem = program_->Sem().Get(var);

    return tint::Switch(  //
        var,
        [&](const ast::Var* v) {
            auto* ty = sem->Type()->Clone(clone_ctx_.type_ctx);
            auto* val = builder.Declare(ty, sem->AddressSpace(), sem->Access());
            current_flow_block->instructions.Push(val);

            if (v->initializer) {
                auto init = EmitExpression(v->initializer);
                if (!init) {
                    return;
                }
                val->initializer = init.Get();
            }
            // Store the declaration so we can get the instruction to store too
            scopes_.Set(v->name->symbol, val);

            // Record the original name of the var
            builder.ir.SetName(val, v->name->symbol.Name());
        },
        [&](const ast::Let* l) {
            // A `let` doesn't exist as a standalone item in the IR, it's just the result of the
            // initializer.
            auto init = EmitExpression(l->initializer);
            if (!init) {
                return;
            }

            // Store the results of the initialization
            scopes_.Set(l->name->symbol, init.Get());

            // Record the original name of the let
            builder.ir.SetName(init.Get(), l->name->symbol.Name());
        },
        [&](const ast::Override*) {
            add_error(var->source,
                      "found an `Override` variable. The SubstituteOverrides "
                      "transform must be run before converting to IR");
        },
        [&](const ast::Const*) {
            // Skip. This should be handled by const-eval already, so the const will be a
            // `constant::` value at the usage sites. Can just ignore the `const` variable as it
            // should never be used.
            //
            // TODO(dsinclair): Probably want to store the const variable somewhere and then in
            // identifier expression log an error if we ever see a const identifier. Add this when
            // identifiers and variables are supported.
        },
        [&](Default) {
            add_error(var->source, "unknown variable: " + std::string(var->TypeInfo().name));
        });
}

utils::Result<Value*> BuilderImpl::EmitUnary(const ast::UnaryOpExpression* expr) {
    auto val = EmitExpression(expr->expr);
    if (!val) {
        return utils::Failure;
    }

    auto* sem = program_->Sem().Get(expr);
    auto* ty = sem->Type()->Clone(clone_ctx_.type_ctx);

    Instruction* inst = nullptr;
    switch (expr->op) {
        case ast::UnaryOp::kAddressOf:
            inst = builder.AddressOf(ty, val.Get());
            break;
        case ast::UnaryOp::kComplement:
            inst = builder.Complement(ty, val.Get());
            break;
        case ast::UnaryOp::kIndirection:
            inst = builder.Indirection(ty, val.Get());
            break;
        case ast::UnaryOp::kNegation:
            inst = builder.Negation(ty, val.Get());
            break;
        case ast::UnaryOp::kNot:
            inst = builder.Not(ty, val.Get());
            break;
    }

    current_flow_block->instructions.Push(inst);
    return inst;
}

// A short-circut needs special treatment. The short-circuit is decomposed into the relevant if
// statements and declarations.
utils::Result<Value*> BuilderImpl::EmitShortCircuit(const ast::BinaryExpression* expr) {
    switch (expr->op) {
        case ast::BinaryOp::kLogicalAnd:
        case ast::BinaryOp::kLogicalOr:
            break;
        default:
            TINT_ICE(IR, diagnostics_) << "invalid operation type for short-circut decomposition";
            return utils::Failure;
    }

    // Evaluate the LHS of the short-circuit
    auto lhs = EmitExpression(expr->lhs);
    if (!lhs) {
        return utils::Failure;
    }

    // Generate a variable to store the short-circut into
    auto* ty = builder.ir.types.Get<type::Bool>();
    auto* result_var =
        builder.Declare(ty, builtin::AddressSpace::kFunction, builtin::Access::kReadWrite);
    current_flow_block->instructions.Push(result_var);

    auto* lhs_store = builder.Store(result_var, lhs.Get());
    current_flow_block->instructions.Push(lhs_store);

    auto* if_node = builder.CreateIf();
    if_node->condition = lhs.Get();
    BranchTo(if_node);

    utils::Result<Value*> rhs;
    {
        FlowStackScope scope(this, if_node);

        // If this is an `&&` then we only evaluate the RHS expression in the true block.
        // If this is an `||` then we only evaluate the RHS expression in the false block.
        if (expr->op == ast::BinaryOp::kLogicalAnd) {
            current_flow_block = if_node->true_.target->As<Block>();
        } else {
            current_flow_block = if_node->false_.target->As<Block>();
        }

        rhs = EmitExpression(expr->rhs);
        if (!rhs) {
            return utils::Failure;
        }
        auto* rhs_store = builder.Store(result_var, rhs.Get());
        current_flow_block->instructions.Push(rhs_store);

        BranchTo(if_node->merge.target);
    }
    current_flow_block = if_node->merge.target->As<Block>();

    return result_var;
}

utils::Result<Value*> BuilderImpl::EmitBinary(const ast::BinaryExpression* expr) {
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
            inst = builder.And(ty, lhs.Get(), rhs.Get());
            break;
        case ast::BinaryOp::kOr:
            inst = builder.Or(ty, lhs.Get(), rhs.Get());
            break;
        case ast::BinaryOp::kXor:
            inst = builder.Xor(ty, lhs.Get(), rhs.Get());
            break;
        case ast::BinaryOp::kEqual:
            inst = builder.Equal(ty, lhs.Get(), rhs.Get());
            break;
        case ast::BinaryOp::kNotEqual:
            inst = builder.NotEqual(ty, lhs.Get(), rhs.Get());
            break;
        case ast::BinaryOp::kLessThan:
            inst = builder.LessThan(ty, lhs.Get(), rhs.Get());
            break;
        case ast::BinaryOp::kGreaterThan:
            inst = builder.GreaterThan(ty, lhs.Get(), rhs.Get());
            break;
        case ast::BinaryOp::kLessThanEqual:
            inst = builder.LessThanEqual(ty, lhs.Get(), rhs.Get());
            break;
        case ast::BinaryOp::kGreaterThanEqual:
            inst = builder.GreaterThanEqual(ty, lhs.Get(), rhs.Get());
            break;
        case ast::BinaryOp::kShiftLeft:
            inst = builder.ShiftLeft(ty, lhs.Get(), rhs.Get());
            break;
        case ast::BinaryOp::kShiftRight:
            inst = builder.ShiftRight(ty, lhs.Get(), rhs.Get());
            break;
        case ast::BinaryOp::kAdd:
            inst = builder.Add(ty, lhs.Get(), rhs.Get());
            break;
        case ast::BinaryOp::kSubtract:
            inst = builder.Subtract(ty, lhs.Get(), rhs.Get());
            break;
        case ast::BinaryOp::kMultiply:
            inst = builder.Multiply(ty, lhs.Get(), rhs.Get());
            break;
        case ast::BinaryOp::kDivide:
            inst = builder.Divide(ty, lhs.Get(), rhs.Get());
            break;
        case ast::BinaryOp::kModulo:
            inst = builder.Modulo(ty, lhs.Get(), rhs.Get());
            break;
        case ast::BinaryOp::kLogicalAnd:
        case ast::BinaryOp::kLogicalOr:
            TINT_ICE(IR, diagnostics_) << "short circuit op should have already been handled";
            return utils::Failure;
        case ast::BinaryOp::kNone:
            TINT_ICE(IR, diagnostics_) << "missing binary operand type";
            return utils::Failure;
    }

    current_flow_block->instructions.Push(inst);
    return inst;
}

utils::Result<Value*> BuilderImpl::EmitBitcast(const ast::BitcastExpression* expr) {
    auto val = EmitExpression(expr->expr);
    if (!val) {
        return utils::Failure;
    }

    auto* sem = program_->Sem().Get(expr);
    auto* ty = sem->Type()->Clone(clone_ctx_.type_ctx);
    auto* inst = builder.Bitcast(ty, val.Get());

    current_flow_block->instructions.Push(inst);
    return inst;
}

void BuilderImpl::EmitCall(const ast::CallStatement* stmt) {
    (void)EmitCall(stmt->expr);
}

utils::Result<Value*> BuilderImpl::EmitCall(const ast::CallExpression* expr) {
    // If this is a materialized semantic node, just use the constant value.
    if (auto* mat = program_->Sem().Get(expr)) {
        if (mat->ConstantValue()) {
            auto* cv = mat->ConstantValue()->Clone(clone_ctx_);
            if (!cv) {
                add_error(expr->source, "failed to get constant value for call " +
                                            std::string(expr->TypeInfo().name));
                return utils::Failure;
            }
            return builder.Constant(cv);
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
        inst = builder.Builtin(ty, b->Type(), args);
    } else if (sem->Target()->As<sem::ValueConstructor>()) {
        inst = builder.Construct(ty, std::move(args));
    } else if (auto* conv = sem->Target()->As<sem::ValueConversion>()) {
        auto* from = conv->Source()->Clone(clone_ctx_.type_ctx);
        inst = builder.Convert(ty, from, std::move(args));
    } else if (expr->target->identifier->Is<ast::TemplatedIdentifier>()) {
        TINT_UNIMPLEMENTED(IR, diagnostics_) << "missing templated ident support";
        return utils::Failure;
    } else {
        // Not a builtin and not a templated call, so this is a user function.
        auto name = CloneSymbol(expr->target->identifier->symbol);
        inst = builder.UserCall(ty, name, std::move(args));
    }
    if (inst == nullptr) {
        return utils::Failure;
    }
    current_flow_block->instructions.Push(inst);
    return inst;
}

utils::Result<Value*> BuilderImpl::EmitLiteral(const ast::LiteralExpression* lit) {
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
    return builder.Constant(cv);
}

void BuilderImpl::EmitAttributes(utils::VectorRef<const ast::Attribute*> attrs) {
    for (auto* attr : attrs) {
        EmitAttribute(attr);
    }
}

void BuilderImpl::EmitAttribute(const ast::Attribute* attr) {
    tint::Switch(  //
        attr,
        // [&](const ast::WorkgroupAttribute* wg) {
        // TODO(dsinclair): Implement
        // },
        // [&](const ast::StageAttribute* s) {
        // TODO(dsinclair): Implement
        // },
        // [&](const ast::BindingAttribute* b) {
        // TODO(dsinclair): Implement
        // },
        // [&](const ast::GroupAttribute* g) {
        // TODO(dsinclair): Implement
        // },
        // [&](const ast::LocationAttribute* l) {
        // TODO(dsinclair): Implement
        // },
        // [&](const ast::BuiltinAttribute* b) {
        // TODO(dsinclair): Implement
        // },
        // [&](const ast::InterpolateAttribute* i) {
        // TODO(dsinclair): Implement
        // },
        // [&](const ast::InvariantAttribute* i) {
        // TODO(dsinclair): Implement
        // },
        // [&](const ast::MustUseAttribute* i) {
        // TODO(dsinclair): Implement
        // },
        [&](const ast::IdAttribute*) {
            add_error(attr->source,
                      "found an `Id` attribute. The SubstituteOverrides transform "
                      "must be run before converting to IR");
        },
        [&](const ast::StructMemberSizeAttribute*) {
            TINT_ICE(IR, diagnostics_)
                << "StructMemberSizeAttribute encountered during IR conversion";
        },
        [&](const ast::StructMemberAlignAttribute*) {
            TINT_ICE(IR, diagnostics_)
                << "StructMemberAlignAttribute encountered during IR conversion";
        },
        // [&](const ast::StrideAttribute* s) {
        // TODO(dsinclair): Implement
        // },
        // [&](const ast::InternalAttribute *i) {
        // TODO(dsinclair): Implement
        // },
        [&](Default) {
            add_error(attr->source, "unknown attribute: " + std::string(attr->TypeInfo().name));
        });
}

}  // namespace tint::ir
