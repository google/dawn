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

#include "src/tint/ir/builder.h"

#include <utility>

#include "src/tint/ir/builder_impl.h"
#include "src/tint/program.h"

namespace tint::ir {

Builder::Builder(const Program* prog) : ir(prog) {}

Builder::Builder(Module&& mod) : ir(std::move(mod)) {}

Builder::~Builder() = default;

Block* Builder::CreateBlock() {
    return ir.flow_nodes.Create<Block>();
}

Terminator* Builder::CreateTerminator() {
    return ir.flow_nodes.Create<Terminator>();
}

Function* Builder::CreateFunction(const ast::Function* ast_func) {
    auto* ir_func = ir.flow_nodes.Create<Function>(ast_func);
    ir_func->start_target = CreateBlock();
    ir_func->end_target = CreateTerminator();

    // Function is always branching into the start target
    ir_func->start_target->inbound_branches.Push(ir_func);

    return ir_func;
}

If* Builder::CreateIf(const ast::Statement* stmt) {
    auto* ir_if = ir.flow_nodes.Create<If>(stmt);
    ir_if->true_target = CreateBlock();
    ir_if->false_target = CreateBlock();
    ir_if->merge_target = CreateBlock();

    // An if always branches to both the true and false block.
    ir_if->true_target->inbound_branches.Push(ir_if);
    ir_if->false_target->inbound_branches.Push(ir_if);

    return ir_if;
}

Loop* Builder::CreateLoop(const ast::Statement* stmt) {
    auto* ir_loop = ir.flow_nodes.Create<Loop>(stmt);
    ir_loop->start_target = CreateBlock();
    ir_loop->continuing_target = CreateBlock();
    ir_loop->merge_target = CreateBlock();

    // A loop always branches to the start block.
    ir_loop->start_target->inbound_branches.Push(ir_loop);

    return ir_loop;
}

Switch* Builder::CreateSwitch(const ast::SwitchStatement* stmt) {
    auto* ir_switch = ir.flow_nodes.Create<Switch>(stmt);
    ir_switch->merge_target = CreateBlock();
    return ir_switch;
}

Block* Builder::CreateCase(Switch* s, const utils::VectorRef<const ast::CaseSelector*> selectors) {
    s->cases.Push(Switch::Case{selectors, CreateBlock()});

    Block* b = s->cases.Back().start_target;
    // Switch branches into the case block
    b->inbound_branches.Push(s);
    return b;
}

void Builder::Branch(Block* from, FlowNode* to) {
    TINT_ASSERT(IR, from);
    TINT_ASSERT(IR, to);
    from->branch_target = to;
    to->inbound_branches.Push(from);
}

Register::Id Builder::AllocateRegister() {
    return next_register_id++;
}

Op Builder::CreateOp(Op::Kind kind, Register lhs, Register rhs) {
    return Op(kind, Register(AllocateRegister()), lhs, rhs);
}

Op Builder::And(Register lhs, Register rhs) {
    return CreateOp(Op::Kind::kAnd, lhs, rhs);
}

Op Builder::Or(Register lhs, Register rhs) {
    return CreateOp(Op::Kind::kOr, lhs, rhs);
}

Op Builder::Xor(Register lhs, Register rhs) {
    return CreateOp(Op::Kind::kXor, lhs, rhs);
}

Op Builder::LogicalAnd(Register lhs, Register rhs) {
    return CreateOp(Op::Kind::kLogicalAnd, lhs, rhs);
}

Op Builder::LogicalOr(Register lhs, Register rhs) {
    return CreateOp(Op::Kind::kLogicalOr, lhs, rhs);
}

Op Builder::Equal(Register lhs, Register rhs) {
    return CreateOp(Op::Kind::kEqual, lhs, rhs);
}

Op Builder::NotEqual(Register lhs, Register rhs) {
    return CreateOp(Op::Kind::kNotEqual, lhs, rhs);
}

Op Builder::LessThan(Register lhs, Register rhs) {
    return CreateOp(Op::Kind::kLessThan, lhs, rhs);
}

Op Builder::GreaterThan(Register lhs, Register rhs) {
    return CreateOp(Op::Kind::kGreaterThan, lhs, rhs);
}

Op Builder::LessThanEqual(Register lhs, Register rhs) {
    return CreateOp(Op::Kind::kLessThanEqual, lhs, rhs);
}

Op Builder::GreaterThanEqual(Register lhs, Register rhs) {
    return CreateOp(Op::Kind::kGreaterThanEqual, lhs, rhs);
}

Op Builder::ShiftLeft(Register lhs, Register rhs) {
    return CreateOp(Op::Kind::kShiftLeft, lhs, rhs);
}

Op Builder::ShiftRight(Register lhs, Register rhs) {
    return CreateOp(Op::Kind::kShiftRight, lhs, rhs);
}

Op Builder::Add(Register lhs, Register rhs) {
    return CreateOp(Op::Kind::kAdd, lhs, rhs);
}

Op Builder::Subtract(Register lhs, Register rhs) {
    return CreateOp(Op::Kind::kSubtract, lhs, rhs);
}

Op Builder::Multiply(Register lhs, Register rhs) {
    return CreateOp(Op::Kind::kMultiply, lhs, rhs);
}

Op Builder::Divide(Register lhs, Register rhs) {
    return CreateOp(Op::Kind::kDivide, lhs, rhs);
}

Op Builder::Modulo(Register lhs, Register rhs) {
    return CreateOp(Op::Kind::kModulo, lhs, rhs);
}

}  // namespace tint::ir
