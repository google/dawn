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

#include "src/program.h"

#include <sstream>
#include <utility>

#include "src/ast/module.h"
#include "src/clone_context.h"
#include "src/program_builder.h"

namespace tint {

Program::Program() = default;

Program::Program(Program&& program)
    : types_(std::move(program.types_)),
      nodes_(std::move(program.nodes_)),
      ast_(std::move(program.ast_)),
      symbols_(std::move(program.symbols_)) {
  program.AssertNotMoved();
  program.moved_ = true;
}

Program::Program(ProgramBuilder&& builder)
    : types_(std::move(builder.Types())),
      nodes_(std::move(builder.Nodes())),
      ast_(nodes_.Create<ast::Module>(Source{},
                                      builder.AST().ConstructedTypes(),
                                      builder.AST().Functions(),
                                      builder.AST().GlobalVariables())),
      symbols_(std::move(builder.Symbols())) {
  builder.MarkAsMoved();
}

Program::~Program() = default;

Program& Program::operator=(Program&& program) {
  program.AssertNotMoved();
  program.moved_ = true;
  types_ = std::move(program.types_);
  nodes_ = std::move(program.nodes_);
  ast_ = std::move(program.ast_);
  symbols_ = std::move(program.symbols_);
  return *this;
}

Program Program::Clone() const {
  AssertNotMoved();
  return Program(CloneAsBuilder());
}

ProgramBuilder Program::CloneAsBuilder() const {
  AssertNotMoved();
  ProgramBuilder out;
  CloneContext(&out, this).Clone();
  return out;
}

bool Program::IsValid() const {
  AssertNotMoved();
  return ast_->IsValid();
}

std::string Program::to_str() const {
  AssertNotMoved();
  return ast_->to_str();
}

void Program::AssertNotMoved() const {
  assert(!moved_);
}

}  // namespace tint
