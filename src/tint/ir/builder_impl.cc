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
#include "src/tint/ast/block_statement.h"
#include "src/tint/ast/break_if_statement.h"
#include "src/tint/ast/break_statement.h"
#include "src/tint/ast/continue_statement.h"
#include "src/tint/ast/function.h"
#include "src/tint/ast/if_statement.h"
#include "src/tint/ast/return_statement.h"
#include "src/tint/ast/statement.h"
#include "src/tint/ast/static_assert.h"
#include "src/tint/ir/function.h"
#include "src/tint/ir/if.h"
#include "src/tint/ir/loop.h"
#include "src/tint/ir/module.h"
#include "src/tint/ir/switch.h"
#include "src/tint/ir/terminator.h"
#include "src/tint/program.h"
#include "src/tint/sem/module.h"

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

}  // namespace

BuilderImpl::BuilderImpl(const Program* program) : builder_(program) {}

BuilderImpl::~BuilderImpl() = default;

void BuilderImpl::BranchTo(const FlowNode* node) {
    TINT_ASSERT(IR, current_flow_block_);
    TINT_ASSERT(IR, !current_flow_block_->branch_target);

    builder_.Branch(current_flow_block_, node);
    current_flow_block_->branch_target = node;
    current_flow_block_ = nullptr;
}

void BuilderImpl::BranchToIfNeeded(const FlowNode* node) {
    if (!current_flow_block_ || current_flow_block_->branch_target) {
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

ResultType BuilderImpl::Build() {
    auto* sem = builder_.ir.program->Sem().Module();

    for (auto* decl : sem->DependencyOrderedDeclarations()) {
        bool ok = tint::Switch(
            decl,  //
            // [&](const ast::Struct* str) {
            //   return false;
            // },
            [&](const ast::Alias*) {
                // Folded away and doesn't appear in the IR.
                return true;
            },
            // [&](const ast::Const*) {
            //   return false;
            // },
            // [&](const ast::Override*) {
            //   return false;
            // },
            [&](const ast::Function* func) { return EmitFunction(func); },
            // [&](const ast::Enable*) {
            //   return false;
            // },
            [&](const ast::StaticAssert*) {
                // Evaluated by the resolver, drop from the IR.
                return true;
            },
            [&](Default) {
                TINT_ICE(IR, diagnostics_) << "unhandled type: " << decl->TypeInfo().name;
                return false;
            });
        if (!ok) {
            return utils::Failure;
        }
    }

    return ResultType{std::move(builder_.ir)};
}

bool BuilderImpl::EmitFunction(const ast::Function* ast_func) {
    // The flow stack should have been emptied when the previous function finshed building.
    TINT_ASSERT(IR, flow_stack.IsEmpty());

    auto* ir_func = builder_.CreateFunction(ast_func);
    current_function_ = ir_func;
    builder_.ir.functions.Push(ir_func);

    ast_to_flow_[ast_func] = ir_func;

    if (ast_func->IsEntryPoint()) {
        builder_.ir.entry_points.Push(ir_func);
    }

    {
        FlowStackScope scope(this, ir_func);

        current_flow_block_ = ir_func->start_target;
        if (!EmitStatements(ast_func->body->statements)) {
            return false;
        }

        // If the branch target has already been set then a `return` was called. Only set in the
        // case where `return` wasn't called.
        BranchToIfNeeded(current_function_->end_target);
    }

    TINT_ASSERT(IR, flow_stack.IsEmpty());
    current_flow_block_ = nullptr;
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
        if (!current_flow_block_ || current_flow_block_->branch_target) {
            break;
        }
    }
    return true;
}

bool BuilderImpl::EmitStatement(const ast::Statement* stmt) {
    return tint::Switch(
        stmt,
        //        [&](const ast::AssignmentStatement* a) { },
        [&](const ast::BlockStatement* b) { return EmitBlock(b); },
        [&](const ast::BreakStatement* b) { return EmitBreak(b); },
        [&](const ast::BreakIfStatement* b) { return EmitBreakIf(b); },
        //        [&](const ast::CallStatement* c) { },
        [&](const ast::ContinueStatement* c) { return EmitContinue(c); },
        //        [&](const ast::DiscardStatement* d) { },
        //        [&](const ast::FallthroughStatement*) { },
        [&](const ast::IfStatement* i) { return EmitIf(i); },
        [&](const ast::LoopStatement* l) { return EmitLoop(l); },
        //        [&](const ast::ForLoopStatement* l) { },
        //        [&](const ast::WhileStatement* l) { },
        [&](const ast::ReturnStatement* r) { return EmitReturn(r); },
        //        [&](const ast::SwitchStatement* s) { },
        //        [&](const ast::VariableDeclStatement* v) { },
        [&](const ast::StaticAssert*) {
            return true;  // Not emitted
        },
        [&](Default) {
            TINT_ICE(IR, diagnostics_)
                << "unknown statement type: " << std::string(stmt->TypeInfo().name);
            return false;
        });
}

bool BuilderImpl::EmitBlock(const ast::BlockStatement* block) {
    // Note, this doesn't need to emit a Block as the current block flow node should be
    // sufficient as the blocks all get flattened. Each flow control node will inject the basic
    // blocks it requires.
    return EmitStatements(block->statements);
}

bool BuilderImpl::EmitIf(const ast::IfStatement* stmt) {
    auto* if_node = builder_.CreateIf(stmt);

    // TODO(dsinclair): Emit the condition expression into the current block

    BranchTo(if_node);

    ast_to_flow_[stmt] = if_node;

    {
        FlowStackScope scope(this, if_node);

        // TODO(dsinclair): set if condition register into if flow node

        current_flow_block_ = if_node->true_target;
        if (!EmitStatement(stmt->body)) {
            return false;
        }

        current_flow_block_ = if_node->false_target;
        if (stmt->else_statement && !EmitStatement(stmt->else_statement)) {
            return false;
        }
    }
    current_flow_block_ = nullptr;

    // If both branches went somewhere, then they both returned, continued or broke. So,
    // there is no need for the if merge-block and there is nothing to branch to the merge
    // block anyway.
    if (if_node->true_target->branch_target && if_node->false_target->branch_target) {
        return true;
    }

    if_node->merge_target = builder_.CreateBlock();
    current_flow_block_ = if_node->merge_target;

    // If the true branch did not execute control flow, then go to the merge target
    if (!if_node->true_target->branch_target) {
        if_node->true_target->branch_target = if_node->merge_target;
    }
    // If the false branch did not execute control flow, then go to the merge target
    if (!if_node->false_target->branch_target) {
        if_node->false_target->branch_target = if_node->merge_target;
    }

    return true;
}

bool BuilderImpl::EmitLoop(const ast::LoopStatement* stmt) {
    auto* loop_node = builder_.CreateLoop(stmt);

    BranchTo(loop_node);

    ast_to_flow_[stmt] = loop_node;

    {
        FlowStackScope scope(this, loop_node);

        current_flow_block_ = loop_node->start_target;
        if (!EmitStatement(stmt->body)) {
            return false;
        }

        // The current block didn't `break`, `return` or `continue`, go to the continuing block.
        BranchToIfNeeded(loop_node->continuing_target);

        current_flow_block_ = loop_node->continuing_target;
        if (stmt->continuing) {
            if (!EmitStatement(stmt->continuing)) {
                return false;
            }
        }

        // Branch back to the start node if the continue target didn't branch out already
        BranchToIfNeeded(loop_node->start_target);
    }

    current_flow_block_ = loop_node->merge_target;
    return true;
}

bool BuilderImpl::EmitReturn(const ast::ReturnStatement*) {
    // TODO(dsinclair): Emit the return value ....

    BranchTo(current_function_->end_target);

    // TODO(dsinclair): The BranchTo will reset the current block. What happens with dead code
    // after the return?

    return true;
}

bool BuilderImpl::EmitBreak(const ast::BreakStatement*) {
    auto* current_control = FindEnclosingControl(ControlFlags::kNone);
    TINT_ASSERT(IR, current_control);

    if (auto* c = current_control->As<Loop>()) {
        BranchTo(c->merge_target);
    } else if (auto* s = current_control->As<Switch>()) {
        BranchTo(s->merge_target);
    } else {
        TINT_UNREACHABLE(IR, diagnostics_);
        return false;
    }

    // TODO(dsinclair): The BranchTo will reset the current block. What happens with dead code
    // after the break?
    return true;
}

bool BuilderImpl::EmitContinue(const ast::ContinueStatement*) {
    auto* current_control = FindEnclosingControl(ControlFlags::kExcludeSwitch);
    TINT_ASSERT(IR, current_control);

    if (auto* c = current_control->As<Loop>()) {
        BranchTo(c->continuing_target);
    } else {
        TINT_UNREACHABLE(IR, diagnostics_);
    }

    // TODO(dsinclair): The BranchTo will reset the current block. What happens with dead code
    // after the continue?

    return true;
}

bool BuilderImpl::EmitBreakIf(const ast::BreakIfStatement* stmt) {
    auto* if_node = builder_.CreateIf(stmt, Builder::IfFlags::kCreateMerge);

    // TODO(dsinclair): Emit the condition expression into the current block

    BranchTo(if_node);

    ast_to_flow_[stmt] = if_node;

    auto* current_control = FindEnclosingControl(ControlFlags::kExcludeSwitch);
    TINT_ASSERT(IR, current_control);
    TINT_ASSERT(IR, current_control->Is<Loop>());

    auto* loop = current_control->As<Loop>();

    // TODO(dsinclair): set if condition register into if flow node

    current_flow_block_ = if_node->true_target;
    BranchTo(loop->merge_target);

    current_flow_block_ = if_node->false_target;
    BranchTo(if_node->merge_target);

    current_flow_block_ = if_node->merge_target;

    // The `break-if` has to be the last item in the continuing block. The false branch of the
    // `break-if` will always take us back to the start of the loop.
    // break then we go back to the start of the loop.
    BranchTo(loop->start_target);

    return true;
}

}  // namespace tint::ir
