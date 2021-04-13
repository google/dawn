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

#include "src/transform/emit_vertex_point_size.h"

#include <utility>

#include "src/ast/assignment_statement.h"
#include "src/program_builder.h"

namespace tint {
namespace transform {

EmitVertexPointSize::EmitVertexPointSize() = default;
EmitVertexPointSize::~EmitVertexPointSize() = default;

Transform::Output EmitVertexPointSize::Run(const Program* in, const DataMap&) {
  if (!in->AST().Functions().HasStage(ast::PipelineStage::kVertex)) {
    // If the module doesn't have any vertex stages, then there's nothing to do.
    return Output(Program(in->Clone()));
  }

  ProgramBuilder out;

  CloneContext ctx(&out, in);

  // Start by cloning all the symbols. This ensures that the authored symbols
  // won't get renamed if they collide with new symbols below.
  ctx.CloneSymbols();

  Symbol pointsize = out.Symbols().New("tint_pointsize");

  // Declare the pointsize builtin output variable.
  out.Global(pointsize, out.ty.f32(), ast::StorageClass::kOutput, nullptr,
             ast::DecorationList{
                 out.create<ast::BuiltinDecoration>(ast::Builtin::kPointSize),
             });

  // Add the pointsize assignment statement to the front of all vertex stages.
  ctx.ReplaceAll([&](ast::Function* func) -> ast::Function* {
    if (func->pipeline_stage() != ast::PipelineStage::kVertex) {
      return nullptr;  // Just clone func
    }

    return CloneWithStatementsAtStart(&ctx, func,
                                      {
                                          out.Assign(pointsize, 1.0f),
                                      });
  });
  ctx.Clone();

  return Output(Program(std::move(out)));
}

}  // namespace transform
}  // namespace tint
