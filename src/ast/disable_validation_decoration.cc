// Copyright 2021 The Tint Authors.
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

#include "src/ast/disable_validation_decoration.h"
#include "src/clone_context.h"
#include "src/program_builder.h"

TINT_INSTANTIATE_TYPEINFO(tint::ast::DisableValidationDecoration);

namespace tint {
namespace ast {

DisableValidationDecoration::DisableValidationDecoration(
    ProgramID program_id,
    DisabledValidation validation)
    : Base(program_id), validation_(validation) {}

DisableValidationDecoration::~DisableValidationDecoration() = default;

std::string DisableValidationDecoration::Name() const {
  switch (validation_) {
    case DisabledValidation::kFunctionHasNoBody:
      return "disable_validation__function_has_no_body";
    case DisabledValidation::kBindingPointCollision:
      return "disable_validation__binding_point_collision";
  }
  return "<invalid>";
}

DisableValidationDecoration* DisableValidationDecoration::Clone(
    CloneContext* ctx) const {
  return ctx->dst->ASTNodes().Create<DisableValidationDecoration>(
      ctx->dst->ID(), validation_);
}

}  // namespace ast
}  // namespace tint
