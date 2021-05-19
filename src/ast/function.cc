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

#include "src/ast/function.h"

#include "src/ast/stage_decoration.h"
#include "src/ast/workgroup_decoration.h"
#include "src/program_builder.h"

TINT_INSTANTIATE_TYPEINFO(tint::ast::Function);

namespace tint {
namespace ast {

Function::Function(ProgramID program_id,
                   const Source& source,
                   Symbol symbol,
                   VariableList params,
                   ast::Type* return_type,
                   BlockStatement* body,
                   DecorationList decorations,
                   DecorationList return_type_decorations)
    : Base(program_id, source),
      symbol_(symbol),
      params_(std::move(params)),
      return_type_(return_type),
      body_(body),
      decorations_(std::move(decorations)),
      return_type_decorations_(std::move(return_type_decorations)) {
  TINT_ASSERT_PROGRAM_IDS_EQUAL_IF_VALID(symbol_, program_id);
  TINT_ASSERT_PROGRAM_IDS_EQUAL_IF_VALID(body, program_id);
  for (auto* param : params_) {
    TINT_ASSERT(param && param->is_const());
    TINT_ASSERT_PROGRAM_IDS_EQUAL_IF_VALID(param, program_id);
  }
  TINT_ASSERT(symbol_.IsValid());
  TINT_ASSERT(return_type_);
  for (auto* deco : decorations_) {
    TINT_ASSERT_PROGRAM_IDS_EQUAL_IF_VALID(deco, program_id);
  }
  for (auto* deco : return_type_decorations_) {
    TINT_ASSERT_PROGRAM_IDS_EQUAL_IF_VALID(deco, program_id);
  }
}

Function::Function(Function&&) = default;

Function::~Function() = default;

PipelineStage Function::pipeline_stage() const {
  if (auto* stage = GetDecoration<StageDecoration>(decorations_)) {
    return stage->value();
  }
  return PipelineStage::kNone;
}

const Statement* Function::get_last_statement() const {
  return body_->last();
}

Function* Function::Clone(CloneContext* ctx) const {
  // Clone arguments outside of create() call to have deterministic ordering
  auto src = ctx->Clone(source());
  auto sym = ctx->Clone(symbol());
  auto p = ctx->Clone(params_);
  auto* ret = ctx->Clone(return_type_);
  auto* b = ctx->Clone(body_);
  auto decos = ctx->Clone(decorations_);
  auto ret_decos = ctx->Clone(return_type_decorations_);
  return ctx->dst->create<Function>(src, sym, p, ret, b, decos, ret_decos);
}

void Function::to_str(const sem::Info& sem,
                      std::ostream& out,
                      size_t indent) const {
  make_indent(out, indent);
  out << "Function " << symbol_.to_str() << " -> " << return_type_->type_name()
      << std::endl;

  for (auto* deco : decorations()) {
    deco->to_str(sem, out, indent);
  }

  make_indent(out, indent);
  out << "(";

  if (params_.size() > 0) {
    out << std::endl;

    for (auto* param : params_)
      param->to_str(sem, out, indent + 2);

    make_indent(out, indent);
  }
  out << ")" << std::endl;

  make_indent(out, indent);
  out << "{" << std::endl;

  if (body_ != nullptr) {
    for (auto* stmt : *body_) {
      stmt->to_str(sem, out, indent + 2);
    }
  }

  make_indent(out, indent);
  out << "}" << std::endl;
}

std::string Function::type_name() const {
  std::ostringstream out;

  out << "__func" + return_type_->type_name();
  for (auto* param : params_) {
    // No need for the sem::Variable here, functions params must have a
    // type
    out << param->type()->type_name();
  }

  return out.str();
}

Function* FunctionList::Find(Symbol sym) const {
  for (auto* func : *this) {
    if (func->symbol() == sym) {
      return func;
    }
  }
  return nullptr;
}

Function* FunctionList::Find(Symbol sym, PipelineStage stage) const {
  for (auto* func : *this) {
    if (func->symbol() == sym && func->pipeline_stage() == stage) {
      return func;
    }
  }
  return nullptr;
}

bool FunctionList::HasStage(ast::PipelineStage stage) const {
  for (auto* func : *this) {
    if (func->pipeline_stage() == stage) {
      return true;
    }
  }
  return false;
}

}  // namespace ast
}  // namespace tint
