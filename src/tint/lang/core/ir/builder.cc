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

#include "src/tint/lang/core/ir/builder.h"

#include <utility>

#include "src/tint/lang/core/constant/scalar.h"
#include "src/tint/lang/core/type/pointer.h"
#include "src/tint/lang/core/type/reference.h"
#include "src/tint/utils/ice/ice.h"

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
    ir_func->SetBlock(Block());
    ir.SetName(ir_func, name);
    ir.functions.Push(ir_func);
    return ir_func;
}

ir::Loop* Builder::Loop() {
    return Append(ir.instructions.Create<ir::Loop>(Block(), MultiInBlock(), MultiInBlock()));
}

Block* Builder::Case(ir::Switch* s, VectorRef<Switch::CaseSelector> selectors) {
    auto* block = Block();
    s->Cases().Push(Switch::Case{std::move(selectors), block});
    block->SetParent(s);
    return block;
}

Block* Builder::Case(ir::Switch* s, std::initializer_list<Switch::CaseSelector> selectors) {
    return Case(s, Vector<Switch::CaseSelector, 4>(selectors));
}

ir::Discard* Builder::Discard() {
    return Append(ir.instructions.Create<ir::Discard>());
}

ir::Var* Builder::Var(const type::Pointer* type) {
    return Append(ir.instructions.Create<ir::Var>(InstructionResult(type)));
}

ir::Var* Builder::Var(std::string_view name, const type::Pointer* type) {
    auto* var = Var(type);
    ir.SetName(var, name);
    return var;
}

ir::BlockParam* Builder::BlockParam(const type::Type* type) {
    return ir.values.Create<ir::BlockParam>(type);
}

ir::FunctionParam* Builder::FunctionParam(const type::Type* type) {
    return ir.values.Create<ir::FunctionParam>(type);
}

ir::FunctionParam* Builder::FunctionParam(std::string_view name, const type::Type* type) {
    auto* param = ir.values.Create<ir::FunctionParam>(type);
    ir.SetName(param, name);
    return param;
}

ir::TerminateInvocation* Builder::TerminateInvocation() {
    return Append(ir.instructions.Create<ir::TerminateInvocation>());
}

ir::Unreachable* Builder::Unreachable() {
    return Append(ir.instructions.Create<ir::Unreachable>());
}

const type::Type* Builder::VectorPtrElementType(const type::Type* type) {
    auto* vec_ptr_ty = type->As<type::Pointer>();
    TINT_ASSERT(vec_ptr_ty);
    if (TINT_LIKELY(vec_ptr_ty)) {
        auto* vec_ty = vec_ptr_ty->StoreType()->As<type::Vector>();
        TINT_ASSERT(vec_ty);
        if (TINT_LIKELY(vec_ty)) {
            return vec_ty->type();
        }
    }
    return ir.Types().i32();
}

}  // namespace tint::ir
