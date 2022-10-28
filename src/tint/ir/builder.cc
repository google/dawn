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
    return ir_func;
}

If* Builder::CreateIf(const ast::Statement* stmt, IfFlags flags) {
    auto* ir_if = ir.flow_nodes.Create<If>(stmt);
    ir_if->false_target = CreateBlock();
    ir_if->true_target = CreateBlock();

    if (flags == IfFlags::kCreateMerge) {
        ir_if->merge_target = CreateBlock();
    } else {
        ir_if->merge_target = nullptr;
    }
    return ir_if;
}

Loop* Builder::CreateLoop(const ast::LoopStatement* stmt) {
    auto* ir_loop = ir.flow_nodes.Create<Loop>(stmt);
    ir_loop->start_target = CreateBlock();
    ir_loop->continuing_target = CreateBlock();
    ir_loop->merge_target = CreateBlock();

    return ir_loop;
}

void Builder::Branch(Block* from, const FlowNode* to) {
    TINT_ASSERT(IR, from);
    TINT_ASSERT(IR, to);
    from->branch_target = to;
}

}  // namespace tint::ir
