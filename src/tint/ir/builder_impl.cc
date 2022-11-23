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
#include "src/tint/ast/block_statement.h"
#include "src/tint/ast/bool_literal_expression.h"
#include "src/tint/ast/break_if_statement.h"
#include "src/tint/ast/break_statement.h"
#include "src/tint/ast/continue_statement.h"
#include "src/tint/ast/float_literal_expression.h"
#include "src/tint/ast/for_loop_statement.h"
#include "src/tint/ast/function.h"
#include "src/tint/ast/if_statement.h"
#include "src/tint/ast/int_literal_expression.h"
#include "src/tint/ast/literal_expression.h"
#include "src/tint/ast/loop_statement.h"
#include "src/tint/ast/return_statement.h"
#include "src/tint/ast/statement.h"
#include "src/tint/ast/static_assert.h"
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
    return b->branch_target != nullptr;
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

BuilderImpl::BuilderImpl(const Program* program) : builder(program) {}

BuilderImpl::~BuilderImpl() = default;

void BuilderImpl::BranchTo(FlowNode* node) {
    TINT_ASSERT(IR, current_flow_block);
    TINT_ASSERT(IR, !IsBranched(current_flow_block));

    builder.Branch(current_flow_block, node);
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

ResultType BuilderImpl::Build() {
    auto* sem = builder.ir.program->Sem().Module();

    for (auto* decl : sem->DependencyOrderedDeclarations()) {
        bool ok = tint::Switch(
            decl,  //
            // [&](const ast::Struct* str) { },
            [&](const ast::Alias*) {
                // Folded away and doesn't appear in the IR.
                return true;
            },
            // [&](const ast::Variable* var) { },
            [&](const ast::Function* func) { return EmitFunction(func); },
            // [&](const ast::Enable*) { },
            [&](const ast::StaticAssert*) {
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
    // The flow stack should have been emptied when the previous function finshed building.
    TINT_ASSERT(IR, flow_stack.IsEmpty());

    auto* ir_func = builder.CreateFunction(ast_func);
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
        // [&](const ast::AssignmentStatement* a) { },
        [&](const ast::BlockStatement* b) { return EmitBlock(b); },
        [&](const ast::BreakStatement* b) { return EmitBreak(b); },
        [&](const ast::BreakIfStatement* b) { return EmitBreakIf(b); },
        // [&](const ast::CallStatement* c) { },
        // [&](const ast::CompoundAssignmentStatement* c) { },
        [&](const ast::ContinueStatement* c) { return EmitContinue(c); },
        // [&](const ast::DiscardStatement* d) { },
        [&](const ast::IfStatement* i) { return EmitIf(i); },
        [&](const ast::LoopStatement* l) { return EmitLoop(l); },
        [&](const ast::ForLoopStatement* l) { return EmitForLoop(l); },
        [&](const ast::WhileStatement* l) { return EmitWhile(l); },
        [&](const ast::ReturnStatement* r) { return EmitReturn(r); },
        [&](const ast::SwitchStatement* s) { return EmitSwitch(s); },
        [&](const ast::VariableDeclStatement* v) { return EmitVariable(v->variable); },
        [&](const ast::StaticAssert*) {
            return true;  // Not emitted
        },
        [&](Default) {
            diagnostics_.add_warning(
                tint::diag::System::IR,
                "unknown statement type: " + std::string(stmt->TypeInfo().name), stmt->source);
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
    auto* if_node = builder.CreateIf(stmt);

    // Emit the if condition into the end of the preceeding block
    auto reg = EmitExpression(stmt->condition);
    if (!reg) {
        return false;
    }
    if_node->condition = reg.Get();

    BranchTo(if_node);

    ast_to_flow_[stmt] = if_node;

    {
        FlowStackScope scope(this, if_node);

        current_flow_block = if_node->true_target;
        if (!EmitStatement(stmt->body)) {
            return false;
        }
        // If the true branch did not execute control flow, then go to the merge target
        BranchToIfNeeded(if_node->merge_target);

        current_flow_block = if_node->false_target;
        if (stmt->else_statement && !EmitStatement(stmt->else_statement)) {
            return false;
        }
        // If the false branch did not execute control flow, then go to the merge target
        BranchToIfNeeded(if_node->merge_target);
    }
    current_flow_block = nullptr;

    // If both branches went somewhere, then they both returned, continued or broke. So,
    // there is no need for the if merge-block and there is nothing to branch to the merge
    // block anyway.
    if (IsConnected(if_node->merge_target)) {
        current_flow_block = if_node->merge_target;
    }

    return true;
}

bool BuilderImpl::EmitLoop(const ast::LoopStatement* stmt) {
    auto* loop_node = builder.CreateLoop(stmt);

    BranchTo(loop_node);

    ast_to_flow_[stmt] = loop_node;

    {
        FlowStackScope scope(this, loop_node);

        current_flow_block = loop_node->start_target;
        if (!EmitStatement(stmt->body)) {
            return false;
        }

        // The current block didn't `break`, `return` or `continue`, go to the continuing block.
        BranchToIfNeeded(loop_node->continuing_target);

        current_flow_block = loop_node->continuing_target;
        if (stmt->continuing) {
            if (!EmitStatement(stmt->continuing)) {
                return false;
            }
        }

        // Branch back to the start node if the continue target didn't branch out already
        BranchToIfNeeded(loop_node->start_target);
    }

    // The loop merge can get disconnected if the loop returns directly, or the continuing target
    // branches, eventually, to the merge, but nothing branched to the continuing target.
    current_flow_block = loop_node->merge_target;
    if (!IsConnected(loop_node->merge_target)) {
        current_flow_block = nullptr;
    }
    return true;
}

bool BuilderImpl::EmitWhile(const ast::WhileStatement* stmt) {
    auto* loop_node = builder.CreateLoop(stmt);
    // Continue is always empty, just go back to the start
    builder.Branch(loop_node->continuing_target, loop_node->start_target);

    BranchTo(loop_node);

    ast_to_flow_[stmt] = loop_node;

    {
        FlowStackScope scope(this, loop_node);

        current_flow_block = loop_node->start_target;

        // Emit the while condition into the start target of the loop
        auto reg = EmitExpression(stmt->condition);
        if (!reg) {
            return false;
        }

        // Create an if (cond) {} else {break;} control flow
        auto* if_node = builder.CreateIf(nullptr);
        builder.Branch(if_node->true_target, if_node->merge_target);
        builder.Branch(if_node->false_target, loop_node->merge_target);
        if_node->condition = reg.Get();

        BranchTo(if_node);

        current_flow_block = if_node->merge_target;
        if (!EmitStatement(stmt->body)) {
            return false;
        }

        BranchToIfNeeded(loop_node->continuing_target);
    }
    // The while loop always has a path to the merge target as the break statement comes before
    // anything inside the loop.
    current_flow_block = loop_node->merge_target;
    return true;
}

bool BuilderImpl::EmitForLoop(const ast::ForLoopStatement* stmt) {
    auto* loop_node = builder.CreateLoop(stmt);
    builder.Branch(loop_node->continuing_target, loop_node->start_target);

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

        current_flow_block = loop_node->start_target;

        if (stmt->condition) {
            // Emit the condition into the target target of the loop
            auto reg = EmitExpression(stmt->condition);
            if (!reg) {
                return false;
            }

            // Create an if (cond) {} else {break;} control flow
            auto* if_node = builder.CreateIf(nullptr);
            builder.Branch(if_node->true_target, if_node->merge_target);
            builder.Branch(if_node->false_target, loop_node->merge_target);
            if_node->condition = reg.Get();

            BranchTo(if_node);
            current_flow_block = if_node->merge_target;
        }

        if (!EmitStatement(stmt->body)) {
            return false;
        }

        BranchToIfNeeded(loop_node->continuing_target);

        if (stmt->continuing) {
            current_flow_block = loop_node->continuing_target;
            if (!EmitStatement(stmt->continuing)) {
                return false;
            }
        }
    }
    // The while loop always has a path to the merge target as the break statement comes before
    // anything inside the loop.
    current_flow_block = loop_node->merge_target;
    return true;
}

bool BuilderImpl::EmitSwitch(const ast::SwitchStatement* stmt) {
    auto* switch_node = builder.CreateSwitch(stmt);

    // Emit the condition into the preceeding block
    auto reg = EmitExpression(stmt->condition);
    if (!reg) {
        return false;
    }
    switch_node->condition = reg.Get();

    BranchTo(switch_node);

    ast_to_flow_[stmt] = switch_node;

    {
        FlowStackScope scope(this, switch_node);

        for (const auto* c : stmt->body) {
            current_flow_block = builder.CreateCase(switch_node, c->selectors);
            if (!EmitStatement(c->body)) {
                return false;
            }
            BranchToIfNeeded(switch_node->merge_target);
        }
    }
    current_flow_block = nullptr;

    if (IsConnected(switch_node->merge_target)) {
        current_flow_block = switch_node->merge_target;
    }

    return true;
}

bool BuilderImpl::EmitReturn(const ast::ReturnStatement*) {
    // TODO(dsinclair): Emit the return value ....

    BranchTo(current_function_->end_target);
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

    return true;
}

bool BuilderImpl::EmitBreakIf(const ast::BreakIfStatement* stmt) {
    auto* if_node = builder.CreateIf(stmt);

    // Emit the break-if condition into the end of the preceeding block
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

    current_flow_block = if_node->true_target;
    BranchTo(loop->merge_target);

    current_flow_block = if_node->false_target;
    BranchTo(if_node->merge_target);

    current_flow_block = if_node->merge_target;

    // The `break-if` has to be the last item in the continuing block. The false branch of the
    // `break-if` will always take us back to the start of the loop.
    // break then we go back to the start of the loop.
    BranchTo(loop->start_target);

    return true;
}

utils::Result<Register> BuilderImpl::EmitExpression(const ast::Expression* expr) {
    return tint::Switch(
        expr,
        // [&](const ast::IndexAccessorExpression* a) { return EmitIndexAccessor(a); },
        [&](const ast::BinaryExpression* b) { return EmitBinary(b); },
        // [&](const ast::BitcastExpression* b) { return EmitBitcast(b); },
        // [&](const ast::CallExpression* c) { return EmitCall(c); },
        // [&](const ast::IdentifierExpression* i) { return EmitIdentifier(i); },
        [&](const ast::LiteralExpression* l) { return EmitLiteral(l); },
        // [&](const ast::MemberAccessorExpression* m) { return EmitMemberAccessor(m); },
        // [&](const ast::PhonyExpression*) { return true; },
        // [&](const ast::UnaryOpExpression* u) { return EmitUnaryOp(u); },
        [&](Default) {
            diagnostics_.add_warning(
                tint::diag::System::IR,
                "unknown expression type: " + std::string(expr->TypeInfo().name), expr->source);
            return utils::Failure;
        });
}

bool BuilderImpl::EmitVariable(const ast::Variable* var) {
    return tint::Switch(  //
        var,
        // [&](const ast::Var* var) {},
        // [&](const ast::Let*) {},
        // [&](const ast::Override*) { },
        // [&](const ast::Const* c) { },
        [&](Default) {
            diagnostics_.add_warning(tint::diag::System::IR,
                                     "unknown variable: " + std::string(var->TypeInfo().name),
                                     var->source);
            return false;
        });
}

utils::Result<Register> BuilderImpl::EmitBinary(const ast::BinaryExpression* expr) {
    auto lhs = EmitExpression(expr->lhs);
    if (!lhs) {
        return utils::Failure;
    }

    auto rhs = EmitExpression(expr->rhs);
    if (!rhs) {
        return utils::Failure;
    }

    Op op;
    switch (expr->op) {
        case ast::BinaryOp::kAnd:
            op = builder.And(lhs.Get(), rhs.Get());
            break;
        case ast::BinaryOp::kOr:
            op = builder.Or(lhs.Get(), rhs.Get());
            break;
        case ast::BinaryOp::kXor:
            op = builder.Xor(lhs.Get(), rhs.Get());
            break;
        case ast::BinaryOp::kLogicalAnd:
            op = builder.LogicalAnd(lhs.Get(), rhs.Get());
            break;
        case ast::BinaryOp::kLogicalOr:
            op = builder.LogicalOr(lhs.Get(), rhs.Get());
            break;
        case ast::BinaryOp::kEqual:
            op = builder.Equal(lhs.Get(), rhs.Get());
            break;
        case ast::BinaryOp::kNotEqual:
            op = builder.NotEqual(lhs.Get(), rhs.Get());
            break;
        case ast::BinaryOp::kLessThan:
            op = builder.LessThan(lhs.Get(), rhs.Get());
            break;
        case ast::BinaryOp::kGreaterThan:
            op = builder.GreaterThan(lhs.Get(), rhs.Get());
            break;
        case ast::BinaryOp::kLessThanEqual:
            op = builder.LessThanEqual(lhs.Get(), rhs.Get());
            break;
        case ast::BinaryOp::kGreaterThanEqual:
            op = builder.GreaterThanEqual(lhs.Get(), rhs.Get());
            break;
        case ast::BinaryOp::kShiftLeft:
            op = builder.ShiftLeft(lhs.Get(), rhs.Get());
            break;
        case ast::BinaryOp::kShiftRight:
            op = builder.ShiftRight(lhs.Get(), rhs.Get());
            break;
        case ast::BinaryOp::kAdd:
            op = builder.Add(lhs.Get(), rhs.Get());
            break;
        case ast::BinaryOp::kSubtract:
            op = builder.Subtract(lhs.Get(), rhs.Get());
            break;
        case ast::BinaryOp::kMultiply:
            op = builder.Multiply(lhs.Get(), rhs.Get());
            break;
        case ast::BinaryOp::kDivide:
            op = builder.Divide(lhs.Get(), rhs.Get());
            break;
        case ast::BinaryOp::kModulo:
            op = builder.Modulo(lhs.Get(), rhs.Get());
            break;
        case ast::BinaryOp::kNone:
            TINT_ICE(IR, diagnostics_) << "missing binary operand type";
            return utils::Failure;
    }

    auto result = op.Result();
    current_flow_block->ops.Push(op);
    return result;
}

utils::Result<Register> BuilderImpl::EmitLiteral(const ast::LiteralExpression* lit) {
    return tint::Switch(  //
        lit,
        [&](const ast::BoolLiteralExpression* l) {
            return utils::Result<Register>{Register(l->value)};
        },
        [&](const ast::FloatLiteralExpression* l) {
            if (l->suffix == ast::FloatLiteralExpression::Suffix::kF) {
                return utils::Result<Register>{Register(f32(static_cast<float>(l->value)))};
            }
            return utils::Result<Register>{Register(f16(static_cast<float>(l->value)))};
        },
        [&](const ast::IntLiteralExpression* l) {
            if (l->suffix == ast::IntLiteralExpression::Suffix::kI) {
                return utils::Result<Register>{Register(i32(l->value))};
            }
            return utils::Result<Register>{Register(u32(l->value))};
        },
        [&](Default) {
            diagnostics_.add_warning(tint::diag::System::IR,
                                     "unknown literal type: " + std::string(lit->TypeInfo().name),
                                     lit->source);
            return utils::Failure;
        });
}

bool BuilderImpl::EmitType(const ast::Type* ty) {
    return tint::Switch(
        ty,
        // [&](const ast::Array* ary) { },
        // [&](const ast::Bool* b) { },
        // [&](const ast::F32* f) { },
        // [&](const ast::F16* f) { },
        // [&](const ast::I32* i) { },
        // [&](const ast::U32* u) { },
        // [&](const ast::Vector* v) { },
        // [&](const ast::Matrix* mat) { },
        // [&](const ast::Pointer* ptr) { },'
        // [&](const ast::Atomic* a) { },
        // [&](const ast::Sampler* s) { },
        // [&](const ast::ExternalTexture* t) { },
        // [&](const ast::Texture* t) {
        //      return tint::Switch(
        //          t,
        //          [&](const ast::DepthTexture*) { },
        //          [&](const ast::DepthMultisampledTexture*) { },
        //          [&](const ast::SampledTexture*) { },
        //          [&](const ast::MultisampledTexture*) { },
        //          [&](const ast::StorageTexture*) {  },
        //          [&](Default) {
        //              diagnostics_.add_warning(tint::diag::System::IR,
        //                  "unknown texture: " + std::string(t->TypeInfo().name), t->source);
        //              return false;
        //          });
        // },
        // [&](const ast::Void* v) { },
        // [&](const ast::TypeName* tn) { },
        [&](Default) {
            diagnostics_.add_warning(tint::diag::System::IR,
                                     "unknown type: " + std::string(ty->TypeInfo().name),
                                     ty->source);
            return false;
        });
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
        // [&](const ast::WorkgroupAttribute* wg) {},
        // [&](const ast::StageAttribute* s) {},
        // [&](const ast::BindingAttribute* b) {},
        // [&](const ast::GroupAttribute* g) {},
        // [&](const ast::LocationAttribute* l) {},
        // [&](const ast::BuiltinAttribute* b) {},
        // [&](const ast::InterpolateAttribute* i) {},
        // [&](const ast::InvariantAttribute* i) {},
        // [&](const ast::IdAttribute* i) {},
        // [&](const ast::StructMemberSizeAttribute* s) {},
        // [&](const ast::StructMemberAlignAttribute* a) {},
        // [&](const ast::StrideAttribute* s) {}
        // [&](const ast::InternalAttribute *i) {},
        [&](Default) {
            diagnostics_.add_warning(tint::diag::System::IR,
                                     "unknown attribute: " + std::string(attr->TypeInfo().name),
                                     attr->source);
            return false;
        });
}

}  // namespace tint::ir
