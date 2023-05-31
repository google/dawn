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

#include "src/tint/constant/scalar.h"
#include "src/tint/type/pointer.h"
#include "src/tint/type/reference.h"

namespace tint::ir {

Builder::Builder(Module& mod) : ir(mod) {}

Builder::~Builder() = default;

ir::Block* Builder::CreateRootBlockIfNeeded() {
    if (!ir.root_block) {
        ir.root_block = CreateBlock();
    }
    return ir.root_block;
}

Block* Builder::CreateBlock() {
    return ir.blocks.Create<Block>();
}

Function* Builder::CreateFunction(std::string_view name,
                                  const type::Type* return_type,
                                  Function::PipelineStage stage,
                                  std::optional<std::array<uint32_t, 3>> wg_size) {
    TINT_ASSERT(IR, return_type);

    auto* ir_func = ir.values.Create<Function>(return_type, stage, wg_size);
    ir_func->SetStartTarget(CreateBlock());
    ir.SetName(ir_func, name);
    return ir_func;
}

If* Builder::CreateIf(Value* condition) {
    TINT_ASSERT(IR, condition);
    return ir.values.Create<If>(condition, CreateBlock(), CreateBlock(), CreateBlock());
}

Loop* Builder::CreateLoop(utils::VectorRef<Value*> args /* = utils::Empty */) {
    return ir.values.Create<Loop>(CreateBlock(), CreateBlock(), CreateBlock(), std::move(args));
}

Switch* Builder::CreateSwitch(Value* condition) {
    return ir.values.Create<Switch>(condition, CreateBlock());
}

Block* Builder::CreateCase(Switch* s, utils::VectorRef<Switch::CaseSelector> selectors) {
    s->Cases().Push(Switch::Case{std::move(selectors), CreateBlock()});

    Block* b = s->Cases().Back().Start();
    b->AddInboundBranch(s);
    return b;
}

Binary* Builder::CreateBinary(enum Binary::Kind kind,
                              const type::Type* type,
                              Value* lhs,
                              Value* rhs) {
    return ir.values.Create<ir::Binary>(kind, type, lhs, rhs);
}

Binary* Builder::And(const type::Type* type, Value* lhs, Value* rhs) {
    return CreateBinary(Binary::Kind::kAnd, type, lhs, rhs);
}

Binary* Builder::Or(const type::Type* type, Value* lhs, Value* rhs) {
    return CreateBinary(Binary::Kind::kOr, type, lhs, rhs);
}

Binary* Builder::Xor(const type::Type* type, Value* lhs, Value* rhs) {
    return CreateBinary(Binary::Kind::kXor, type, lhs, rhs);
}

Binary* Builder::Equal(const type::Type* type, Value* lhs, Value* rhs) {
    return CreateBinary(Binary::Kind::kEqual, type, lhs, rhs);
}

Binary* Builder::NotEqual(const type::Type* type, Value* lhs, Value* rhs) {
    return CreateBinary(Binary::Kind::kNotEqual, type, lhs, rhs);
}

Binary* Builder::LessThan(const type::Type* type, Value* lhs, Value* rhs) {
    return CreateBinary(Binary::Kind::kLessThan, type, lhs, rhs);
}

Binary* Builder::GreaterThan(const type::Type* type, Value* lhs, Value* rhs) {
    return CreateBinary(Binary::Kind::kGreaterThan, type, lhs, rhs);
}

Binary* Builder::LessThanEqual(const type::Type* type, Value* lhs, Value* rhs) {
    return CreateBinary(Binary::Kind::kLessThanEqual, type, lhs, rhs);
}

Binary* Builder::GreaterThanEqual(const type::Type* type, Value* lhs, Value* rhs) {
    return CreateBinary(Binary::Kind::kGreaterThanEqual, type, lhs, rhs);
}

Binary* Builder::ShiftLeft(const type::Type* type, Value* lhs, Value* rhs) {
    return CreateBinary(Binary::Kind::kShiftLeft, type, lhs, rhs);
}

Binary* Builder::ShiftRight(const type::Type* type, Value* lhs, Value* rhs) {
    return CreateBinary(Binary::Kind::kShiftRight, type, lhs, rhs);
}

Binary* Builder::Add(const type::Type* type, Value* lhs, Value* rhs) {
    return CreateBinary(Binary::Kind::kAdd, type, lhs, rhs);
}

Binary* Builder::Subtract(const type::Type* type, Value* lhs, Value* rhs) {
    return CreateBinary(Binary::Kind::kSubtract, type, lhs, rhs);
}

Binary* Builder::Multiply(const type::Type* type, Value* lhs, Value* rhs) {
    return CreateBinary(Binary::Kind::kMultiply, type, lhs, rhs);
}

Binary* Builder::Divide(const type::Type* type, Value* lhs, Value* rhs) {
    return CreateBinary(Binary::Kind::kDivide, type, lhs, rhs);
}

Binary* Builder::Modulo(const type::Type* type, Value* lhs, Value* rhs) {
    return CreateBinary(Binary::Kind::kModulo, type, lhs, rhs);
}

Unary* Builder::CreateUnary(enum Unary::Kind kind, const type::Type* type, Value* val) {
    return ir.values.Create<ir::Unary>(kind, type, val);
}

Unary* Builder::Complement(const type::Type* type, Value* val) {
    return CreateUnary(Unary::Kind::kComplement, type, val);
}

Unary* Builder::Negation(const type::Type* type, Value* val) {
    return CreateUnary(Unary::Kind::kNegation, type, val);
}

Binary* Builder::Not(const type::Type* type, Value* val) {
    return Equal(type, val, Constant(false));
}

ir::Bitcast* Builder::Bitcast(const type::Type* type, Value* val) {
    return ir.values.Create<ir::Bitcast>(type, val);
}

ir::Discard* Builder::Discard() {
    return ir.values.Create<ir::Discard>();
}

ir::UserCall* Builder::UserCall(const type::Type* type,
                                Function* func,
                                utils::VectorRef<Value*> args) {
    return ir.values.Create<ir::UserCall>(type, func, std::move(args));
}

ir::Convert* Builder::Convert(const type::Type* to,
                              const type::Type* from,
                              utils::VectorRef<Value*> args) {
    return ir.values.Create<ir::Convert>(to, from, std::move(args));
}

ir::Construct* Builder::Construct(const type::Type* to, utils::VectorRef<Value*> args) {
    return ir.values.Create<ir::Construct>(to, std::move(args));
}

ir::Builtin* Builder::Builtin(const type::Type* type,
                              builtin::Function func,
                              utils::VectorRef<Value*> args) {
    return ir.values.Create<ir::Builtin>(type, func, args);
}

ir::Load* Builder::Load(Value* from) {
    auto* ptr = from->Type()->As<type::Pointer>();
    TINT_ASSERT(IR, ptr);
    return ir.values.Create<ir::Load>(ptr->StoreType(), from);
}

ir::Store* Builder::Store(Value* to, Value* from) {
    return ir.values.Create<ir::Store>(to, from);
}

ir::Var* Builder::Declare(const type::Type* type) {
    return ir.values.Create<ir::Var>(type);
}

ir::Return* Builder::Return(Function* func, utils::VectorRef<Value*> args /* = utils::Empty */) {
    return ir.values.Create<ir::Return>(func, std::move(args));
}

ir::NextIteration* Builder::NextIteration(Loop* loop,
                                          utils::VectorRef<Value*> args /* = utils::Empty */) {
    return ir.values.Create<ir::NextIteration>(loop, std::move(args));
}

ir::BreakIf* Builder::BreakIf(Value* condition,
                              Loop* loop,
                              utils::VectorRef<Value*> args /* = utils::Empty */) {
    return ir.values.Create<ir::BreakIf>(condition, loop, std::move(args));
}

ir::Continue* Builder::Continue(Loop* loop, utils::VectorRef<Value*> args /* = utils::Empty */) {
    return ir.values.Create<ir::Continue>(loop, std::move(args));
}

ir::ExitSwitch* Builder::ExitSwitch(Switch* sw,
                                    utils::VectorRef<Value*> args /* = utils::Empty */) {
    return ir.values.Create<ir::ExitSwitch>(sw, std::move(args));
}

ir::ExitLoop* Builder::ExitLoop(Loop* loop, utils::VectorRef<Value*> args /* = utils::Empty */) {
    return ir.values.Create<ir::ExitLoop>(loop, std::move(args));
}

ir::ExitIf* Builder::ExitIf(If* i, utils::VectorRef<Value*> args /* = utils::Empty */) {
    return ir.values.Create<ir::ExitIf>(i, std::move(args));
}

ir::BlockParam* Builder::BlockParam(const type::Type* type) {
    return ir.values.Create<ir::BlockParam>(type);
}

ir::FunctionParam* Builder::FunctionParam(const type::Type* type) {
    return ir.values.Create<ir::FunctionParam>(type);
}

}  // namespace tint::ir
