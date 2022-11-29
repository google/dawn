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

Temp::Id Builder::AllocateTempId() {
    return next_temp_id++;
}

const Instruction* Builder::CreateInstruction(Instruction::Kind kind,
                                              const Value* lhs,
                                              const Value* rhs) {
    return ir.instructions.Create<ir::Instruction>(kind, Temp(), lhs, rhs);
}

const Instruction* Builder::And(const Value* lhs, const Value* rhs) {
    return CreateInstruction(Instruction::Kind::kAnd, lhs, rhs);
}

const Instruction* Builder::Or(const Value* lhs, const Value* rhs) {
    return CreateInstruction(Instruction::Kind::kOr, lhs, rhs);
}

const Instruction* Builder::Xor(const Value* lhs, const Value* rhs) {
    return CreateInstruction(Instruction::Kind::kXor, lhs, rhs);
}

const Instruction* Builder::LogicalAnd(const Value* lhs, const Value* rhs) {
    return CreateInstruction(Instruction::Kind::kLogicalAnd, lhs, rhs);
}

const Instruction* Builder::LogicalOr(const Value* lhs, const Value* rhs) {
    return CreateInstruction(Instruction::Kind::kLogicalOr, lhs, rhs);
}

const Instruction* Builder::Equal(const Value* lhs, const Value* rhs) {
    return CreateInstruction(Instruction::Kind::kEqual, lhs, rhs);
}

const Instruction* Builder::NotEqual(const Value* lhs, const Value* rhs) {
    return CreateInstruction(Instruction::Kind::kNotEqual, lhs, rhs);
}

const Instruction* Builder::LessThan(const Value* lhs, const Value* rhs) {
    return CreateInstruction(Instruction::Kind::kLessThan, lhs, rhs);
}

const Instruction* Builder::GreaterThan(const Value* lhs, const Value* rhs) {
    return CreateInstruction(Instruction::Kind::kGreaterThan, lhs, rhs);
}

const Instruction* Builder::LessThanEqual(const Value* lhs, const Value* rhs) {
    return CreateInstruction(Instruction::Kind::kLessThanEqual, lhs, rhs);
}

const Instruction* Builder::GreaterThanEqual(const Value* lhs, const Value* rhs) {
    return CreateInstruction(Instruction::Kind::kGreaterThanEqual, lhs, rhs);
}

const Instruction* Builder::ShiftLeft(const Value* lhs, const Value* rhs) {
    return CreateInstruction(Instruction::Kind::kShiftLeft, lhs, rhs);
}

const Instruction* Builder::ShiftRight(const Value* lhs, const Value* rhs) {
    return CreateInstruction(Instruction::Kind::kShiftRight, lhs, rhs);
}

const Instruction* Builder::Add(const Value* lhs, const Value* rhs) {
    return CreateInstruction(Instruction::Kind::kAdd, lhs, rhs);
}

const Instruction* Builder::Subtract(const Value* lhs, const Value* rhs) {
    return CreateInstruction(Instruction::Kind::kSubtract, lhs, rhs);
}

const Instruction* Builder::Multiply(const Value* lhs, const Value* rhs) {
    return CreateInstruction(Instruction::Kind::kMultiply, lhs, rhs);
}

const Instruction* Builder::Divide(const Value* lhs, const Value* rhs) {
    return CreateInstruction(Instruction::Kind::kDivide, lhs, rhs);
}

const Instruction* Builder::Modulo(const Value* lhs, const Value* rhs) {
    return CreateInstruction(Instruction::Kind::kModulo, lhs, rhs);
}

}  // namespace tint::ir
