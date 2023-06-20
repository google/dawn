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

Builder::Builder(Module& mod, ir::Block* block) : current_block_(block), ir(mod) {}

Builder::~Builder() = default;

ir::Block* Builder::RootBlock() {
    if (!ir.root_block) {
        ir.root_block = Block();
    }
    return ir.root_block;
}

Block* Builder::Block() {
    return ir.blocks.Create<ir::Block>();
}

MultiInBlock* Builder::MultiInBlock() {
    return ir.blocks.Create<ir::MultiInBlock>();
}

Function* Builder::Function(std::string_view name,
                            const type::Type* return_type,
                            Function::PipelineStage stage,
                            std::optional<std::array<uint32_t, 3>> wg_size) {
    auto* ir_func = ir.values.Create<ir::Function>(return_type, stage, wg_size);
    ir_func->SetStartTarget(Block());
    ir.SetName(ir_func, name);
    return ir_func;
}

ir::Loop* Builder::Loop() {
    return Append(ir.instructions.Create<ir::Loop>(Block(), MultiInBlock(), MultiInBlock()));
}

Block* Builder::Case(ir::Switch* s, utils::VectorRef<Switch::CaseSelector> selectors) {
    auto* block = Block();
    s->Cases().Push(Switch::Case{std::move(selectors), block});
    block->SetParent(s);
    return block;
}

Block* Builder::Case(ir::Switch* s, std::initializer_list<Switch::CaseSelector> selectors) {
    return Case(s, utils::Vector<Switch::CaseSelector, 4>(selectors));
}

ir::Discard* Builder::Discard() {
    return Append(ir.instructions.Create<ir::Discard>());
}

ir::Var* Builder::Var(const type::Pointer* type) {
    return Append(ir.instructions.Create<ir::Var>(InstructionResult(type)));
}

ir::BlockParam* Builder::BlockParam(const type::Type* type) {
    return ir.values.Create<ir::BlockParam>(type);
}

ir::FunctionParam* Builder::FunctionParam(const type::Type* type) {
    return ir.values.Create<ir::FunctionParam>(type);
}

ir::Unreachable* Builder::Unreachable() {
    return Append(ir.instructions.Create<ir::Unreachable>());
}

}  // namespace tint::ir
