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

#include "src/ast/variable.h"

#include <assert.h>

#include "src/ast/constant_id_decoration.h"
#include "src/clone_context.h"
#include "src/program_builder.h"
#include "src/semantic/variable.h"

TINT_INSTANTIATE_CLASS_ID(tint::ast::Variable);

namespace tint {
namespace ast {

Variable::Variable(const Source& source,
                   const Symbol& sym,
                   StorageClass sc,
                   type::Type* type,
                   bool is_const,
                   Expression* constructor,
                   VariableDecorationList decorations)
    : Base(source),
      symbol_(sym),
      type_(type),
      is_const_(is_const),
      constructor_(constructor),
      decorations_(std::move(decorations)),
      declared_storage_class_(sc) {}

Variable::Variable(Variable&&) = default;

Variable::~Variable() = default;

bool Variable::HasLocationDecoration() const {
  for (auto* deco : decorations_) {
    if (deco->Is<LocationDecoration>()) {
      return true;
    }
  }
  return false;
}

bool Variable::HasBuiltinDecoration() const {
  for (auto* deco : decorations_) {
    if (deco->Is<BuiltinDecoration>()) {
      return true;
    }
  }
  return false;
}

bool Variable::HasConstantIdDecoration() const {
  for (auto* deco : decorations_) {
    if (deco->Is<ConstantIdDecoration>()) {
      return true;
    }
  }
  return false;
}

LocationDecoration* Variable::GetLocationDecoration() const {
  for (auto* deco : decorations_) {
    if (deco->Is<LocationDecoration>()) {
      return deco->As<LocationDecoration>();
    }
  }
  return nullptr;
}

uint32_t Variable::constant_id() const {
  assert(HasConstantIdDecoration());
  for (auto* deco : decorations_) {
    if (auto* cid = deco->As<ConstantIdDecoration>()) {
      return cid->value();
    }
  }
  return 0;
}

Variable* Variable::Clone(CloneContext* ctx) const {
  return ctx->dst->create<Variable>(
      ctx->Clone(source()), ctx->Clone(symbol_), declared_storage_class(),
      ctx->Clone(type()), is_const_, ctx->Clone(constructor()),
      ctx->Clone(decorations_));
}

bool Variable::IsValid() const {
  if (!symbol_.IsValid()) {
    return false;
  }
  if (type_ == nullptr) {
    return false;
  }
  if (constructor_ && !constructor_->IsValid()) {
    return false;
  }
  return true;
}

void Variable::info_to_str(const semantic::Info& sem,
                           std::ostream& out,
                           size_t indent) const {
  auto* var_sem = sem.Get(this);
  make_indent(out, indent);
  out << symbol_.to_str() << std::endl;
  make_indent(out, indent);
  out << (var_sem ? var_sem->StorageClass() : declared_storage_class())
      << std::endl;
  make_indent(out, indent);
  out << type_->type_name() << std::endl;
}

void Variable::constructor_to_str(const semantic::Info& sem,
                                  std::ostream& out,
                                  size_t indent) const {
  if (constructor_ == nullptr)
    return;

  make_indent(out, indent);
  out << "{" << std::endl;

  constructor_->to_str(sem, out, indent + 2);

  make_indent(out, indent);
  out << "}" << std::endl;
}

void Variable::to_str(const semantic::Info& sem,
                      std::ostream& out,
                      size_t indent) const {
  make_indent(out, indent);
  out << "Variable";
  if (is_const()) {
    out << "Const";
  }
  out << "{" << std::endl;

  if (!decorations_.empty()) {
    make_indent(out, indent + 2);
    out << "Decorations{" << std::endl;
    for (auto* deco : decorations_) {
      deco->to_str(sem, out, indent + 4);
    }
    make_indent(out, indent + 2);
    out << "}" << std::endl;
  }

  info_to_str(sem, out, indent + 2);
  constructor_to_str(sem, out, indent + 2);
  make_indent(out, indent);
  out << "}" << std::endl;
}

}  // namespace ast
}  // namespace tint
