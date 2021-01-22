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

#include "src/ast/module.h"
#include "src/ast/stage_decoration.h"
#include "src/ast/variable.h"
#include "src/ast/workgroup_decoration.h"
#include "src/clone_context.h"
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

void Function::add_referenced_module_variable(Variable* var) {
  for (const auto* v : referenced_module_vars_) {
    if (v->symbol() == var->symbol()) {
      return;
    }
  }
  referenced_module_vars_.push_back(var);
}

void Function::add_local_referenced_module_variable(Variable* var) {
  for (const auto* v : local_referenced_module_vars_) {
    if (v->symbol() == var->symbol()) {
      return;
    }
  }
  local_referenced_module_vars_.push_back(var);
}

const std::vector<std::pair<Variable*, LocationDecoration*>>
Function::referenced_location_variables() const {
  std::vector<std::pair<Variable*, LocationDecoration*>> ret;

  for (auto* var : referenced_module_variables()) {
    for (auto* deco : var->decorations()) {
      if (auto* location = deco->As<LocationDecoration>()) {
        ret.push_back({var, location});
        break;
      }
    }
  }
  return ret;
}

const std::vector<std::pair<Variable*, Function::BindingInfo>>
Function::referenced_uniform_variables() const {
  std::vector<std::pair<Variable*, Function::BindingInfo>> ret;

  for (auto* var : referenced_module_variables()) {
    if (var->storage_class() != StorageClass::kUniform) {
      continue;
    }

    BindingDecoration* binding = nullptr;
    GroupDecoration* group = nullptr;
    for (auto* deco : var->decorations()) {
      if (auto* b = deco->As<BindingDecoration>()) {
        binding = b;
      } else if (auto* g = deco->As<GroupDecoration>()) {
        group = g;
      }
    }
    if (binding == nullptr || group == nullptr) {
      continue;
    }

    ret.push_back({var, BindingInfo{binding, group}});
  }
  return ret;
}

const std::vector<std::pair<Variable*, Function::BindingInfo>>
Function::referenced_storagebuffer_variables() const {
  std::vector<std::pair<Variable*, Function::BindingInfo>> ret;

  for (auto* var : referenced_module_variables()) {
    if (var->storage_class() != StorageClass::kStorage) {
      continue;
    }

    BindingDecoration* binding = nullptr;
    GroupDecoration* group = nullptr;
    for (auto* deco : var->decorations()) {
      if (auto* b = deco->As<BindingDecoration>()) {
        binding = b;
      } else if (auto* s = deco->As<GroupDecoration>()) {
        group = s;
      }
    }
    if (binding == nullptr || group == nullptr) {
      continue;
    }

    ret.push_back({var, BindingInfo{binding, group}});
  }
  return ret;
}

const std::vector<std::pair<Variable*, BuiltinDecoration*>>
Function::referenced_builtin_variables() const {
  std::vector<std::pair<Variable*, BuiltinDecoration*>> ret;

  for (auto* var : referenced_module_variables()) {
    for (auto* deco : var->decorations()) {
      if (auto* builtin = deco->As<BuiltinDecoration>()) {
        ret.push_back({var, builtin});
        break;
      }
    }
  }
  return ret;
}

const std::vector<std::pair<Variable*, Function::BindingInfo>>
Function::referenced_sampler_variables() const {
  return ReferencedSamplerVariablesImpl(type::SamplerKind::kSampler);
}

const std::vector<std::pair<Variable*, Function::BindingInfo>>
Function::referenced_comparison_sampler_variables() const {
  return ReferencedSamplerVariablesImpl(type::SamplerKind::kComparisonSampler);
}

const std::vector<std::pair<Variable*, Function::BindingInfo>>
Function::referenced_sampled_texture_variables() const {
  return ReferencedSampledTextureVariablesImpl(false);
}

const std::vector<std::pair<Variable*, Function::BindingInfo>>
Function::referenced_multisampled_texture_variables() const {
  return ReferencedSampledTextureVariablesImpl(true);
}

const std::vector<std::pair<Variable*, BuiltinDecoration*>>
Function::local_referenced_builtin_variables() const {
  std::vector<std::pair<Variable*, BuiltinDecoration*>> ret;

  for (auto* var : local_referenced_module_variables()) {
    for (auto* deco : var->decorations()) {
      if (auto* builtin = deco->As<BuiltinDecoration>()) {
        ret.push_back({var, builtin});
        break;
      }
    }
  }
  return ret;
}

void Function::add_ancestor_entry_point(Symbol ep) {
  for (const auto& point : ancestor_entry_points_) {
    if (point == ep) {
      return;
    }
  }
  ancestor_entry_points_.push_back(ep);
}

bool Function::HasAncestorEntryPoint(Symbol symbol) const {
  for (const auto& point : ancestor_entry_points_) {
    if (point == symbol) {
      return true;
    }
  }
  return false;
}

const Statement* Function::get_last_statement() const {
  return body_->last();
}

Function* Function::Clone(CloneContext* ctx) const {
  return ctx->dst->create<Function>(
      ctx->Clone(source()), ctx->Clone(symbol()), ctx->Clone(params_),
      ctx->Clone(return_type_), ctx->Clone(body_), ctx->Clone(decorations_));
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

void Function::to_str(std::ostream& out, size_t indent) const {
  make_indent(out, indent);
  out << "Function " << symbol_.to_str() << " -> " << return_type_->type_name()
      << std::endl;

  for (auto* deco : decorations()) {
    deco->to_str(out, indent);
  }

  make_indent(out, indent);
  out << "(";

  if (params_.size() > 0) {
    out << std::endl;

    for (auto* param : params_)
      param->to_str(out, indent + 2);

    make_indent(out, indent);
  }
  out << ")" << std::endl;

  make_indent(out, indent);
  out << "{" << std::endl;

  if (body_ != nullptr) {
    for (auto* stmt : *body_) {
      stmt->to_str(out, indent + 2);
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

const std::vector<std::pair<Variable*, Function::BindingInfo>>
Function::ReferencedSamplerVariablesImpl(type::SamplerKind kind) const {
  std::vector<std::pair<Variable*, Function::BindingInfo>> ret;

  for (auto* var : referenced_module_variables()) {
    auto* unwrapped_type = var->type()->UnwrapIfNeeded();
    auto* sampler = unwrapped_type->As<type::Sampler>();
    if (sampler == nullptr || sampler->kind() != kind) {
      continue;
    }

    BindingDecoration* binding = nullptr;
    GroupDecoration* group = nullptr;
    for (auto* deco : var->decorations()) {
      if (auto* b = deco->As<BindingDecoration>()) {
        binding = b;
      }
      if (auto* s = deco->As<GroupDecoration>()) {
        group = s;
      }
    }
    if (binding == nullptr || group == nullptr) {
      continue;
    }

    ret.push_back({var, BindingInfo{binding, group}});
  }
  return ret;
}

const std::vector<std::pair<Variable*, Function::BindingInfo>>
Function::ReferencedSampledTextureVariablesImpl(bool multisampled) const {
  std::vector<std::pair<Variable*, Function::BindingInfo>> ret;

  for (auto* var : referenced_module_variables()) {
    auto* unwrapped_type = var->type()->UnwrapIfNeeded();
    auto* texture = unwrapped_type->As<type::Texture>();
    if (texture == nullptr) {
      continue;
    }

    auto is_multisampled = texture->Is<type::MultisampledTexture>();
    auto is_sampled = texture->Is<type::SampledTexture>();

    if ((multisampled && !is_multisampled) || (!multisampled && !is_sampled)) {
      continue;
    }

    BindingDecoration* binding = nullptr;
    GroupDecoration* group = nullptr;
    for (auto* deco : var->decorations()) {
      if (auto* b = deco->As<BindingDecoration>()) {
        binding = b;
      } else if (auto* s = deco->As<GroupDecoration>()) {
        group = s;
      }
    }
    if (binding == nullptr || group == nullptr) {
      continue;
    }

    ret.push_back({var, BindingInfo{binding, group}});
  }

  return ret;
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
