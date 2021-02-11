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

#include <sstream>

#include "src/ast/stage_decoration.h"
#include "src/ast/variable.h"
#include "src/ast/workgroup_decoration.h"
#include "src/clone_context.h"
#include "src/program_builder.h"
#include "src/type/multisampled_texture_type.h"
#include "src/type/sampled_texture_type.h"
#include "src/type/texture_type.h"

TINT_INSTANTIATE_CLASS_ID(tint::ast::Function);

namespace tint {
namespace ast {

Function::Function(const Source& source,
                   Symbol symbol,
                   VariableList params,
                   type::Type* return_type,
                   BlockStatement* body,
                   FunctionDecorationList decorations)
    : Base(source),
      symbol_(symbol),
      params_(std::move(params)),
      return_type_(return_type),
      body_(body),
      decorations_(std::move(decorations)) {}

Function::Function(Function&&) = default;

Function::~Function() = default;

std::tuple<uint32_t, uint32_t, uint32_t> Function::workgroup_size() const {
  for (auto* deco : decorations_) {
    if (auto* workgroup = deco->As<WorkgroupDecoration>()) {
      return workgroup->values();
    }
  }
  return {1, 1, 1};
}

PipelineStage Function::pipeline_stage() const {
  for (auto* deco : decorations_) {
    if (auto* stage = deco->As<StageDecoration>()) {
      return stage->value();
    }
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
  return ctx->dst->create<Function>(src, sym, p, ret, b, decos);
}

bool Function::IsValid() const {
  for (auto* param : params_) {
    if (param == nullptr || !param->IsValid())
      return false;
  }
  if (body_ == nullptr || !body_->IsValid()) {
    return false;
  }
  if (!symbol_.IsValid()) {
    return false;
  }
  if (return_type_ == nullptr) {
    return false;
  }
  return true;
}

void Function::to_str(const semantic::Info& sem,
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
