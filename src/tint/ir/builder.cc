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

Block* Builder::CreateCase(Switch* s, utils::VectorRef<const ast::CaseSelector*> selectors) {
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

const Binary* Builder::CreateBinary(Binary::Kind kind, const Value* lhs, const Value* rhs) {
    return ir.instructions.Create<ir::Binary>(kind, Temp(), lhs, rhs);
}

const Binary* Builder::And(const Value* lhs, const Value* rhs) {
    return CreateBinary(Binary::Kind::kAnd, lhs, rhs);
}

const Binary* Builder::Or(const Value* lhs, const Value* rhs) {
    return CreateBinary(Binary::Kind::kOr, lhs, rhs);
}

const Binary* Builder::Xor(const Value* lhs, const Value* rhs) {
    return CreateBinary(Binary::Kind::kXor, lhs, rhs);
}

const Binary* Builder::LogicalAnd(const Value* lhs, const Value* rhs) {
    return CreateBinary(Binary::Kind::kLogicalAnd, lhs, rhs);
}

const Binary* Builder::LogicalOr(const Value* lhs, const Value* rhs) {
    return CreateBinary(Binary::Kind::kLogicalOr, lhs, rhs);
}

const Binary* Builder::Equal(const Value* lhs, const Value* rhs) {
    return CreateBinary(Binary::Kind::kEqual, lhs, rhs);
}

const Binary* Builder::NotEqual(const Value* lhs, const Value* rhs) {
    return CreateBinary(Binary::Kind::kNotEqual, lhs, rhs);
}

const Binary* Builder::LessThan(const Value* lhs, const Value* rhs) {
    return CreateBinary(Binary::Kind::kLessThan, lhs, rhs);
}

const Binary* Builder::GreaterThan(const Value* lhs, const Value* rhs) {
    return CreateBinary(Binary::Kind::kGreaterThan, lhs, rhs);
}

const Binary* Builder::LessThanEqual(const Value* lhs, const Value* rhs) {
    return CreateBinary(Binary::Kind::kLessThanEqual, lhs, rhs);
}

const Binary* Builder::GreaterThanEqual(const Value* lhs, const Value* rhs) {
    return CreateBinary(Binary::Kind::kGreaterThanEqual, lhs, rhs);
}

const Binary* Builder::ShiftLeft(const Value* lhs, const Value* rhs) {
    return CreateBinary(Binary::Kind::kShiftLeft, lhs, rhs);
}

const Binary* Builder::ShiftRight(const Value* lhs, const Value* rhs) {
    return CreateBinary(Binary::Kind::kShiftRight, lhs, rhs);
}

const Binary* Builder::Add(const Value* lhs, const Value* rhs) {
    return CreateBinary(Binary::Kind::kAdd, lhs, rhs);
}

const Binary* Builder::Subtract(const Value* lhs, const Value* rhs) {
    return CreateBinary(Binary::Kind::kSubtract, lhs, rhs);
}

const Binary* Builder::Multiply(const Value* lhs, const Value* rhs) {
    return CreateBinary(Binary::Kind::kMultiply, lhs, rhs);
}

const Binary* Builder::Divide(const Value* lhs, const Value* rhs) {
    return CreateBinary(Binary::Kind::kDivide, lhs, rhs);
}

const Binary* Builder::Modulo(const Value* lhs, const Value* rhs) {
    return CreateBinary(Binary::Kind::kModulo, lhs, rhs);
}

}  // namespace tint::ir
