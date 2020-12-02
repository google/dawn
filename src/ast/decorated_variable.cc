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

#include "src/ast/decorated_variable.h"

#include <cassert>

#include "src/ast/builtin_decoration.h"
#include "src/ast/clone_context.h"
#include "src/ast/constant_id_decoration.h"
#include "src/ast/location_decoration.h"
#include "src/ast/module.h"

TINT_INSTANTIATE_CLASS_ID(tint::ast::DecoratedVariable);

namespace tint {
namespace ast {

DecoratedVariable::DecoratedVariable() = default;

DecoratedVariable::DecoratedVariable(Variable* var)
    : Base(var->source(), var->name(), var->storage_class(), var->type()) {}

DecoratedVariable::DecoratedVariable(DecoratedVariable&&) = default;

DecoratedVariable::~DecoratedVariable() = default;

bool DecoratedVariable::HasLocationDecoration() const {
  for (auto* deco : decorations_) {
    if (deco->Is<LocationDecoration>()) {
      return true;
    }
  }
  return false;
}

bool DecoratedVariable::HasBuiltinDecoration() const {
  for (auto* deco : decorations_) {
    if (deco->Is<BuiltinDecoration>()) {
      return true;
    }
  }
  return false;
}

bool DecoratedVariable::HasConstantIdDecoration() const {
  for (auto* deco : decorations_) {
    if (deco->Is<ConstantIdDecoration>()) {
      return true;
    }
  }
  return false;
}

uint32_t DecoratedVariable::constant_id() const {
  assert(HasConstantIdDecoration());
  for (auto* deco : decorations_) {
    if (auto* cid = deco->As<ConstantIdDecoration>()) {
      return cid->value();
    }
  }
  return 0;
}

DecoratedVariable* DecoratedVariable::Clone(CloneContext* ctx) const {
  auto* cloned = ctx->mod->create<DecoratedVariable>();
  cloned->set_source(ctx->Clone(source()));
  cloned->set_name(name());
  cloned->set_storage_class(storage_class());
  cloned->set_type(ctx->Clone(type()));
  cloned->set_constructor(ctx->Clone(constructor()));
  cloned->set_is_const(is_const());
  cloned->set_decorations(ctx->Clone(decorations()));
  return cloned;
}

bool DecoratedVariable::IsValid() const {
  return Variable::IsValid();
}

void DecoratedVariable::to_str(std::ostream& out, size_t indent) const {
  make_indent(out, indent);
  out << "DecoratedVariable";
  if (is_const()) {
    out << "Const";
  }
  out << "{" << std::endl;

  make_indent(out, indent + 2);
  out << "Decorations{" << std::endl;
  for (auto* deco : decorations_) {
    deco->to_str(out, indent + 4);
  }

  make_indent(out, indent + 2);
  out << "}" << std::endl;

  info_to_str(out, indent + 2);
  constructor_to_str(out, indent + 2);
  make_indent(out, indent);
  out << "}" << std::endl;
}

}  // namespace ast
}  // namespace tint
