// Copyright 2020 The Tint Authors.
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

#include "src/tint/ast/function.h"

#include "src/tint/ast/stage_attribute.h"
#include "src/tint/ast/workgroup_attribute.h"
#include "src/tint/program_builder.h"

TINT_INSTANTIATE_TYPEINFO(tint::ast::Function);

namespace tint::ast {

Function::Function(ProgramID pid,
                   const Source& src,
                   Symbol sym,
                   VariableList parameters,
                   const Type* return_ty,
                   const BlockStatement* b,
                   AttributeList attrs,
                   AttributeList return_type_attrs)
    : Base(pid, src),
      symbol(sym),
      params(std::move(parameters)),
      return_type(return_ty),
      body(b),
      attributes(std::move(attrs)),
      return_type_attributes(std::move(return_type_attrs)) {
    TINT_ASSERT_PROGRAM_IDS_EQUAL_IF_VALID(AST, symbol, program_id);
    TINT_ASSERT_PROGRAM_IDS_EQUAL_IF_VALID(AST, body, program_id);
    for (auto* param : params) {
        TINT_ASSERT(AST, param && param->is_const);
        TINT_ASSERT_PROGRAM_IDS_EQUAL_IF_VALID(AST, param, program_id);
    }
    TINT_ASSERT(AST, symbol.IsValid());
    TINT_ASSERT(AST, return_type);
    for (auto* attr : attributes) {
        TINT_ASSERT_PROGRAM_IDS_EQUAL_IF_VALID(AST, attr, program_id);
    }
    for (auto* attr : return_type_attributes) {
        TINT_ASSERT_PROGRAM_IDS_EQUAL_IF_VALID(AST, attr, program_id);
    }
}

Function::Function(Function&&) = default;

Function::~Function() = default;

PipelineStage Function::PipelineStage() const {
    if (auto* stage = GetAttribute<StageAttribute>(attributes)) {
        return stage->stage;
    }
    return PipelineStage::kNone;
}

const Function* Function::Clone(CloneContext* ctx) const {
    // Clone arguments outside of create() call to have deterministic ordering
    auto src = ctx->Clone(source);
    auto sym = ctx->Clone(symbol);
    auto p = ctx->Clone(params);
    auto* ret = ctx->Clone(return_type);
    auto* b = ctx->Clone(body);
    auto attrs = ctx->Clone(attributes);
    auto ret_attrs = ctx->Clone(return_type_attributes);
    return ctx->dst->create<Function>(src, sym, p, ret, b, attrs, ret_attrs);
}

const Function* FunctionList::Find(Symbol sym) const {
    for (auto* func : *this) {
        if (func->symbol == sym) {
            return func;
        }
    }
    return nullptr;
}

const Function* FunctionList::Find(Symbol sym, PipelineStage stage) const {
    for (auto* func : *this) {
        if (func->symbol == sym && func->PipelineStage() == stage) {
            return func;
        }
    }
    return nullptr;
}

bool FunctionList::HasStage(ast::PipelineStage stage) const {
    for (auto* func : *this) {
        if (func->PipelineStage() == stage) {
            return true;
        }
    }
    return false;
}

}  // namespace tint::ast
