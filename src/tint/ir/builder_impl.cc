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

#include "src/tint/ast/alias.h"
#include "src/tint/ast/binary_expression.h"
#include "src/tint/ast/bitcast_expression.h"
#include "src/tint/ast/block_statement.h"
#include "src/tint/ast/bool_literal_expression.h"
#include "src/tint/ast/break_if_statement.h"
#include "src/tint/ast/break_statement.h"
#include "src/tint/ast/const_assert.h"
#include "src/tint/ast/continue_statement.h"
#include "src/tint/ast/float_literal_expression.h"
#include "src/tint/ast/for_loop_statement.h"
#include "src/tint/ast/function.h"
#include "src/tint/ast/id_attribute.h"
#include "src/tint/ast/identifier.h"
#include "src/tint/ast/if_statement.h"
#include "src/tint/ast/int_literal_expression.h"
#include "src/tint/ast/literal_expression.h"
#include "src/tint/ast/loop_statement.h"
#include "src/tint/ast/override.h"
#include "src/tint/ast/return_statement.h"
#include "src/tint/ast/statement.h"
#include "src/tint/ast/struct.h"
#include "src/tint/ast/struct_member_align_attribute.h"
#include "src/tint/ast/struct_member_size_attribute.h"
#include "src/tint/ast/switch_statement.h"
#include "src/tint/ast/variable_decl_statement.h"
#include "src/tint/ast/while_statement.h"
#include "src/tint/ir/function.h"
#include "src/tint/ir/if.h"
#include "src/tint/ir/loop.h"
#include "src/tint/ir/module.h"
#include "src/tint/ir/switch.h"
#include "src/tint/ir/terminator.h"
#include "src/tint/program.h"
#include "src/tint/sem/module.h"
#include "src/tint/sem/switch_statement.h"
#include "src/tint/sem/value_expression.h"
#include "src/tint/type/void.h"

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
    return clone_ctx_.type_ctx.dst.st->Register(clone_ctx_.type_ctx.src.st->NameFor(sym));
}

ResultType BuilderImpl::Build() {
    auto* sem = program_->Sem().Module();

    for (auto* decl : sem->DependencyOrderedDeclarations()) {
        bool ok = tint::Switch(
            decl,  //
            [&](const ast::Struct*) {
                // Will be encoded into the `type::Struct` when used. We will then hoist all
                // used structs up to module scope when converting IR.
                return true;
            },
            [&](const ast::Alias*) {
                // Folded away and doesn't appear in the IR.
                return true;
            },
            // [&](const ast::Variable* var) {
            // TODO(dsinclair): Implement
            // },
            [&](const ast::Function* func) { return EmitFunction(func); },
            // [&](const ast::Enable*) {
            // TODO(dsinclair): Implement? I think these need to be passed along so further stages
            // know what is enabled.
            // },
            [&](const ast::ConstAssert*) {
                // Evaluated by the resolver, drop from the IR.
                return true;
            },
            [&](Default) {
                diagnostics_.add_warning(tint::diag::System::IR,
                                         "unknown type: " + std::string(decl->TypeInfo().name),
                                         decl->source);
                return true;
            });
        if (!ok) {
            return utils::Failure;
        }
    }

    return ResultType{std::move(builder.ir)};
}

bool BuilderImpl::EmitFunction(const ast::Function* ast_func) {
    // The flow stack should have been emptied when the previous function finished building.
    TINT_ASSERT(IR, flow_stack.IsEmpty());

    auto* ir_func = builder.CreateFunction();
    ir_func->name = CloneSymbol(ast_func->name->symbol);
    current_function_ = ir_func;
    builder.ir.functions.Push(ir_func);

    ast_to_flow_[ast_func] = ir_func;

    if (ast_func->IsEntryPoint()) {
        builder.ir.entry_points.Push(ir_func);
    }

    {
        FlowStackScope scope(this, ir_func);

        current_flow_block = ir_func->start_target;
        if (!EmitStatements(ast_func->body->statements)) {
            return false;
        }

        // TODO(dsinclair): Store return type and attributes
        // TODO(dsinclair): Store parameters
        // TODO(dsinclair): Store attributes

        // If the branch target has already been set then a `return` was called. Only set in the
        // case where `return` wasn't called.
        BranchToIfNeeded(current_function_->end_target);
    }

    TINT_ASSERT(IR, flow_stack.IsEmpty());
    current_flow_block = nullptr;
    current_function_ = nullptr;

    return true;
}

bool BuilderImpl::EmitStatements(utils::VectorRef<const ast::Statement*> stmts) {
    for (auto* s : stmts) {
        if (!EmitStatement(s)) {
            return false;
        }

        // If the current flow block has a branch target then the rest of the statements in this
        // block are dead code. Skip them.
        if (!current_flow_block || IsBranched(current_flow_block)) {
            break;
        }
    }
    return true;
}

bool BuilderImpl::EmitStatement(const ast::Statement* stmt) {
    return tint::Switch(
        stmt,
        // [&](const ast::AssignmentStatement* a) {
        // TODO(dsinclair): Implement
        // },
        [&](const ast::BlockStatement* b) { return EmitBlock(b); },
        [&](const ast::BreakStatement* b) { return EmitBreak(b); },
        [&](const ast::BreakIfStatement* b) { return EmitBreakIf(b); },
        // [&](const ast::CallStatement* c) {
        // TODO(dsinclair): Implement
        // },
        // [&](const ast::CompoundAssignmentStatement* c) {
        // TODO(dsinclair): Implement
        // },
        [&](const ast::ContinueStatement* c) { return EmitContinue(c); },
        // [&](const ast::DiscardStatement* d) {
        // TODO(dsinclair): Implement
        // },
        [&](const ast::IfStatement* i) { return EmitIf(i); },
        [&](const ast::LoopStatement* l) { return EmitLoop(l); },
        [&](const ast::ForLoopStatement* l) { return EmitForLoop(l); },
        [&](const ast::WhileStatement* l) { return EmitWhile(l); },
        [&](const ast::ReturnStatement* r) { return EmitReturn(r); },
        [&](const ast::SwitchStatement* s) { return EmitSwitch(s); },
        [&](const ast::VariableDeclStatement* v) { return EmitVariable(v->variable); },
        [&](const ast::ConstAssert*) {
            return true;  // Not emitted
        },
        [&](Default) {
            diagnostics_.add_warning(
                tint::diag::System::IR,
                "unknown statement type: " + std::string(stmt->TypeInfo().name), stmt->source);
            // TODO(dsinclair): This should return `false`, switch back when all
            // the cases are handled.
            return true;
        });
}

bool BuilderImpl::EmitBlock(const ast::BlockStatement* block) {
    // Note, this doesn't need to emit a Block as the current block flow node should be
    // sufficient as the blocks all get flattened. Each flow control node will inject the basic
    // blocks it requires.
    return EmitStatements(block->statements);
}

bool BuilderImpl::EmitIf(const ast::IfStatement* stmt) {
    auto* if_node = builder.CreateIf();

    // Emit the if condition into the end of the preceding block
    auto reg = EmitExpression(stmt->condition);
    if (!reg) {
        return false;
    }
    if_node->condition = reg.Get();

    BranchTo(if_node);

    ast_to_flow_[stmt] = if_node;

    {
        FlowStackScope scope(this, if_node);

        current_flow_block = if_node->true_.target->As<Block>();
        if (!EmitStatement(stmt->body)) {
            return false;
        }
        // If the true branch did not execute control flow, then go to the merge target
        BranchToIfNeeded(if_node->merge.target);

        current_flow_block = if_node->false_.target->As<Block>();
        if (stmt->else_statement && !EmitStatement(stmt->else_statement)) {
            return false;
        }
        // If the false branch did not execute control flow, then go to the merge target
        BranchToIfNeeded(if_node->merge.target);
    }
    current_flow_block = nullptr;

    // If both branches went somewhere, then they both returned, continued or broke. So,
    // there is no need for the if merge-block and there is nothing to branch to the merge
    // block anyway.
    if (IsConnected(if_node->merge.target)) {
        current_flow_block = if_node->merge.target->As<Block>();
    }

    return true;
}

bool BuilderImpl::EmitLoop(const ast::LoopStatement* stmt) {
    auto* loop_node = builder.CreateLoop();

    BranchTo(loop_node);

    ast_to_flow_[stmt] = loop_node;

    {
        FlowStackScope scope(this, loop_node);

        current_flow_block = loop_node->start.target->As<Block>();
        if (!EmitStatement(stmt->body)) {
            return false;
        }

        // The current block didn't `break`, `return` or `continue`, go to the continuing block.
        BranchToIfNeeded(loop_node->continuing.target);

        current_flow_block = loop_node->continuing.target->As<Block>();
        if (stmt->continuing) {
            if (!EmitStatement(stmt->continuing)) {
                return false;
            }
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
    return true;
}

bool BuilderImpl::EmitWhile(const ast::WhileStatement* stmt) {
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
            return false;
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
        if (!EmitStatement(stmt->body)) {
            return false;
        }

        BranchToIfNeeded(loop_node->continuing.target);
    }
    // The while loop always has a path to the merge target as the break statement comes before
    // anything inside the loop.
    current_flow_block = loop_node->merge.target->As<Block>();
    return true;
}

bool BuilderImpl::EmitForLoop(const ast::ForLoopStatement* stmt) {
    auto* loop_node = builder.CreateLoop();
    TINT_ASSERT(IR, loop_node->continuing.target->Is<Block>());
    builder.Branch(loop_node->continuing.target->As<Block>(), loop_node->start.target,
                   utils::Empty);

    if (stmt->initializer) {
        // Emit the for initializer before branching to the loop
        if (!EmitStatement(stmt->initializer)) {
            return false;
        }
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
                return false;
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

        if (!EmitStatement(stmt->body)) {
            return false;
        }

        BranchToIfNeeded(loop_node->continuing.target);

        if (stmt->continuing) {
            current_flow_block = loop_node->continuing.target->As<Block>();
            if (!EmitStatement(stmt->continuing)) {
                return false;
            }
        }
    }
    // The while loop always has a path to the merge target as the break statement comes before
    // anything inside the loop.
    current_flow_block = loop_node->merge.target->As<Block>();
    return true;
}

bool BuilderImpl::EmitSwitch(const ast::SwitchStatement* stmt) {
    auto* switch_node = builder.CreateSwitch();

    // Emit the condition into the preceding block
    auto reg = EmitExpression(stmt->condition);
    if (!reg) {
        return false;
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
            if (!EmitStatement(c->Body()->Declaration())) {
                return false;
            }
            BranchToIfNeeded(switch_node->merge.target);
        }
    }
    current_flow_block = nullptr;

    if (IsConnected(switch_node->merge.target)) {
        current_flow_block = switch_node->merge.target->As<Block>();
    }

    return true;
}

bool BuilderImpl::EmitReturn(const ast::ReturnStatement* stmt) {
    utils::Vector<Value*, 1> ret_value;
    if (stmt->value) {
        auto ret = EmitExpression(stmt->value);
        if (!ret) {
            return false;
        }
        ret_value.Push(ret.Get());
    }

    BranchTo(current_function_->end_target, std::move(ret_value));
    return true;
}

bool BuilderImpl::EmitBreak(const ast::BreakStatement*) {
    auto* current_control = FindEnclosingControl(ControlFlags::kNone);
    TINT_ASSERT(IR, current_control);

    if (auto* c = current_control->As<Loop>()) {
        BranchTo(c->merge.target);
    } else if (auto* s = current_control->As<Switch>()) {
        BranchTo(s->merge.target);
    } else {
        TINT_UNREACHABLE(IR, diagnostics_);
        return false;
    }

    return true;
}

bool BuilderImpl::EmitContinue(const ast::ContinueStatement*) {
    auto* current_control = FindEnclosingControl(ControlFlags::kExcludeSwitch);
    TINT_ASSERT(IR, current_control);

    if (auto* c = current_control->As<Loop>()) {
        BranchTo(c->continuing.target);
    } else {
        TINT_UNREACHABLE(IR, diagnostics_);
    }

    return true;
}

bool BuilderImpl::EmitBreakIf(const ast::BreakIfStatement* stmt) {
    auto* if_node = builder.CreateIf();

    // Emit the break-if condition into the end of the preceding block
    auto reg = EmitExpression(stmt->condition);
    if (!reg) {
        return false;
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

    return true;
}

utils::Result<Value*> BuilderImpl::EmitExpression(const ast::Expression* expr) {
    return tint::Switch(
        expr,
        // [&](const ast::IndexAccessorExpression* a) {
        // TODO(dsinclair): Implement
        // },
        [&](const ast::BinaryExpression* b) { return EmitBinary(b); },
        [&](const ast::BitcastExpression* b) { return EmitBitcast(b); },
        // [&](const ast::CallExpression* c) {
        // TODO(dsinclair): Implement
        // },
        // [&](const ast::IdentifierExpression* i) {
        // TODO(dsinclair): Implement
        // },
        [&](const ast::LiteralExpression* l) { return EmitLiteral(l); },
        // [&](const ast::MemberAccessorExpression* m) {
        // TODO(dsinclair): Implement
        // },
        // [&](const ast::PhonyExpression*) {
        // TODO(dsinclair): Implement. The call may have side effects so has to be made.
        // },
        // [&](const ast::UnaryOpExpression* u) {
        // TODO(dsinclair): Implement
        // },
        [&](Default) {
            diagnostics_.add_warning(
                tint::diag::System::IR,
                "unknown expression type: " + std::string(expr->TypeInfo().name), expr->source);
            // TODO(dsinclair): This should return utils::Failure; Switch back
            // once all the above cases are handled.
            auto* v = builder.ir.types.Get<type::Void>();
            return builder.Temp(v);
        });
}

bool BuilderImpl::EmitVariable(const ast::Variable* var) {
    return tint::Switch(  //
        var,
        // [&](const ast::Var* var) {
        // TODO(dsinclair): Implement
        // },
        // [&](const ast::Let*) {
        // TODO(dsinclair): Implement
        // },
        [&](const ast::Override*) {
            diagnostics_.add_warning(tint::diag::System::IR,
                                     "found an `Override` variable. The SubstituteOverrides "
                                     "transform must be run before converting to IR",
                                     var->source);
            return false;
        },
        // [&](const ast::Const* c) {
        // TODO(dsinclair): Implement
        // },
        [&](Default) {
            diagnostics_.add_warning(tint::diag::System::IR,
                                     "unknown variable: " + std::string(var->TypeInfo().name),
                                     var->source);

            // TODO(dsinclair): This should return `false`, switch back when all
            // the cases are handled.
            return true;
        });
}

utils::Result<Value*> BuilderImpl::EmitBinary(const ast::BinaryExpression* expr) {
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

    Binary* instr = nullptr;
    switch (expr->op) {
        case ast::BinaryOp::kAnd:
            instr = builder.And(ty, lhs.Get(), rhs.Get());
            break;
        case ast::BinaryOp::kOr:
            instr = builder.Or(ty, lhs.Get(), rhs.Get());
            break;
        case ast::BinaryOp::kXor:
            instr = builder.Xor(ty, lhs.Get(), rhs.Get());
            break;
        case ast::BinaryOp::kLogicalAnd:
            instr = builder.LogicalAnd(ty, lhs.Get(), rhs.Get());
            break;
        case ast::BinaryOp::kLogicalOr:
            instr = builder.LogicalOr(ty, lhs.Get(), rhs.Get());
            break;
        case ast::BinaryOp::kEqual:
            instr = builder.Equal(ty, lhs.Get(), rhs.Get());
            break;
        case ast::BinaryOp::kNotEqual:
            instr = builder.NotEqual(ty, lhs.Get(), rhs.Get());
            break;
        case ast::BinaryOp::kLessThan:
            instr = builder.LessThan(ty, lhs.Get(), rhs.Get());
            break;
        case ast::BinaryOp::kGreaterThan:
            instr = builder.GreaterThan(ty, lhs.Get(), rhs.Get());
            break;
        case ast::BinaryOp::kLessThanEqual:
            instr = builder.LessThanEqual(ty, lhs.Get(), rhs.Get());
            break;
        case ast::BinaryOp::kGreaterThanEqual:
            instr = builder.GreaterThanEqual(ty, lhs.Get(), rhs.Get());
            break;
        case ast::BinaryOp::kShiftLeft:
            instr = builder.ShiftLeft(ty, lhs.Get(), rhs.Get());
            break;
        case ast::BinaryOp::kShiftRight:
            instr = builder.ShiftRight(ty, lhs.Get(), rhs.Get());
            break;
        case ast::BinaryOp::kAdd:
            instr = builder.Add(ty, lhs.Get(), rhs.Get());
            break;
        case ast::BinaryOp::kSubtract:
            instr = builder.Subtract(ty, lhs.Get(), rhs.Get());
            break;
        case ast::BinaryOp::kMultiply:
            instr = builder.Multiply(ty, lhs.Get(), rhs.Get());
            break;
        case ast::BinaryOp::kDivide:
            instr = builder.Divide(ty, lhs.Get(), rhs.Get());
            break;
        case ast::BinaryOp::kModulo:
            instr = builder.Modulo(ty, lhs.Get(), rhs.Get());
            break;
        case ast::BinaryOp::kNone:
            TINT_ICE(IR, diagnostics_) << "missing binary operand type";
            return utils::Failure;
    }

    current_flow_block->instructions.Push(instr);
    return instr->Result();
}

utils::Result<Value*> BuilderImpl::EmitBitcast(const ast::BitcastExpression* expr) {
    auto val = EmitExpression(expr->expr);
    if (!val) {
        return utils::Failure;
    }

    auto* sem = program_->Sem().Get(expr);
    auto* ty = sem->Type()->Clone(clone_ctx_.type_ctx);
    auto* instr = builder.Bitcast(ty, val.Get());

    current_flow_block->instructions.Push(instr);
    return instr->Result();
}

utils::Result<Value*> BuilderImpl::EmitLiteral(const ast::LiteralExpression* lit) {
    auto* sem = program_->Sem().Get(lit);
    if (!sem) {
        diagnostics_.add_error(
            tint::diag::System::IR,
            "Failed to get semantic information for node " + std::string(lit->TypeInfo().name),
            lit->source);
        return utils::Failure;
    }

    auto* cv = sem->ConstantValue()->Clone(clone_ctx_);
    if (!cv) {
        diagnostics_.add_error(
            tint::diag::System::IR,
            "Failed to get constant value for node " + std::string(lit->TypeInfo().name),
            lit->source);
        return utils::Failure;
    }
    return builder.Constant(cv);
}

bool BuilderImpl::EmitAttributes(utils::VectorRef<const ast::Attribute*> attrs) {
    for (auto* attr : attrs) {
        if (!EmitAttribute(attr)) {
            return false;
        }
    }
    return true;
}

bool BuilderImpl::EmitAttribute(const ast::Attribute* attr) {
    return tint::Switch(  //
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
            diagnostics_.add_warning(tint::diag::System::IR,
                                     "found an `Id` attribute. The SubstituteOverrides transform "
                                     "must be run before converting to IR",
                                     attr->source);
            return false;
        },
        [&](const ast::StructMemberSizeAttribute*) {
            TINT_ICE(IR, diagnostics_)
                << "StructMemberSizeAttribute encountered during IR conversion";
            return false;
        },
        [&](const ast::StructMemberAlignAttribute*) {
            TINT_ICE(IR, diagnostics_)
                << "StructMemberAlignAttribute encountered during IR conversion";
            return false;
        },
        // [&](const ast::StrideAttribute* s) {
        // TODO(dsinclair): Implement
        // },
        // [&](const ast::InternalAttribute *i) {
        // TODO(dsinclair): Implement
        // },
        [&](Default) {
            diagnostics_.add_warning(tint::diag::System::IR,
                                     "unknown attribute: " + std::string(attr->TypeInfo().name),
                                     attr->source);
            return false;
        });
}

}  // namespace tint::ir
