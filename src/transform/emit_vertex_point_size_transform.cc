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

#include "src/transform/emit_vertex_point_size_transform.h"

#include <memory>
#include <utility>

#include "src/ast/assignment_statement.h"
#include "src/ast/block_statement.h"
#include "src/ast/decorated_variable.h"
#include "src/ast/float_literal.h"
#include "src/ast/identifier_expression.h"
#include "src/ast/scalar_constructor_expression.h"
#include "src/ast/type/f32_type.h"
#include "src/ast/type_manager.h"

namespace tint {
namespace transform {
namespace {

const char kPointSizeVar[] = "tint_pointsize";

}  // namespace

EmitVertexPointSizeTransform::EmitVertexPointSizeTransform(ast::Module* mod)
    : Transformer(mod) {}

EmitVertexPointSizeTransform::~EmitVertexPointSizeTransform() = default;

bool EmitVertexPointSizeTransform::Run() {
  if (!mod_->HasStage(ast::PipelineStage::kVertex)) {
    // If the module doesn't have any vertex stages, then there's nothing to do.
    return true;
  }

  auto* f32 = mod_->create<ast::type::F32>();

  // Declare the pointsize builtin output variable.
  auto* pointsize_var =
      mod_->create<ast::DecoratedVariable>(mod_->create<ast::Variable>(
          kPointSizeVar, ast::StorageClass::kOutput, f32));
  pointsize_var->set_decorations({
      mod_->create<ast::BuiltinDecoration>(ast::Builtin::kPointSize, Source{}),
  });
  mod_->AddGlobalVariable(pointsize_var);

  // Build the AST expression & statement for assigning pointsize one.
  auto* one = mod_->create<ast::ScalarConstructorExpression>(
      mod_->create<ast::FloatLiteral>(f32, 1.0f));
  auto* pointsize_ident =
      mod_->create<ast::IdentifierExpression>(Source{}, kPointSizeVar);
  auto* pointsize_assign =
      mod_->create<ast::AssignmentStatement>(pointsize_ident, one);

  // Add the pointsize assignment statement to the front of all vertex stages.
  for (auto* func : mod_->functions()) {
    if (func->pipeline_stage() == ast::PipelineStage::kVertex) {
      func->body()->insert(0, pointsize_assign);
    }
  }

  return true;
}

}  // namespace transform
}  // namespace tint
