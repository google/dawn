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

#include "src/ast/statement.h"

#include <assert.h>

#include "src/ast/assignment_statement.h"
#include "src/ast/break_statement.h"
#include "src/ast/case_statement.h"
#include "src/ast/continue_statement.h"
#include "src/ast/else_statement.h"
#include "src/ast/fallthrough_statement.h"
#include "src/ast/if_statement.h"
#include "src/ast/kill_statement.h"
#include "src/ast/loop_statement.h"
#include "src/ast/return_statement.h"
#include "src/ast/switch_statement.h"
#include "src/ast/variable_decl_statement.h"

namespace tint {
namespace ast {

Statement::Statement() = default;

Statement::Statement(const Source& source) : Node(source) {}

Statement::Statement(Statement&&) = default;

Statement::~Statement() = default;

bool Statement::IsAssign() const {
  return false;
}

bool Statement::IsBreak() const {
  return false;
}

bool Statement::IsCase() const {
  return false;
}

bool Statement::IsContinue() const {
  return false;
}

bool Statement::IsElse() const {
  return false;
}

bool Statement::IsFallthrough() const {
  return false;
}

bool Statement::IsIf() const {
  return false;
}

bool Statement::IsKill() const {
  return false;
}

bool Statement::IsLoop() const {
  return false;
}

bool Statement::IsReturn() const {
  return false;
}

bool Statement::IsSwitch() const {
  return false;
}

bool Statement::IsVariableDecl() const {
  return false;
}

const AssignmentStatement* Statement::AsAssign() const {
  assert(IsAssign());
  return static_cast<const AssignmentStatement*>(this);
}

const BreakStatement* Statement::AsBreak() const {
  assert(IsBreak());
  return static_cast<const BreakStatement*>(this);
}

const CaseStatement* Statement::AsCase() const {
  assert(IsCase());
  return static_cast<const CaseStatement*>(this);
}

const ContinueStatement* Statement::AsContinue() const {
  assert(IsContinue());
  return static_cast<const ContinueStatement*>(this);
}

const ElseStatement* Statement::AsElse() const {
  assert(IsElse());
  return static_cast<const ElseStatement*>(this);
}

const FallthroughStatement* Statement::AsFallthrough() const {
  assert(IsFallthrough());
  return static_cast<const FallthroughStatement*>(this);
}

const IfStatement* Statement::AsIf() const {
  assert(IsIf());
  return static_cast<const IfStatement*>(this);
}

const KillStatement* Statement::AsKill() const {
  assert(IsKill());
  return static_cast<const KillStatement*>(this);
}

const LoopStatement* Statement::AsLoop() const {
  assert(IsLoop());
  return static_cast<const LoopStatement*>(this);
}

const ReturnStatement* Statement::AsReturn() const {
  assert(IsReturn());
  return static_cast<const ReturnStatement*>(this);
}

const SwitchStatement* Statement::AsSwitch() const {
  assert(IsSwitch());
  return static_cast<const SwitchStatement*>(this);
}

const VariableDeclStatement* Statement::AsVariableDecl() const {
  assert(IsVariableDecl());
  return static_cast<const VariableDeclStatement*>(this);
}

AssignmentStatement* Statement::AsAssign() {
  assert(IsAssign());
  return static_cast<AssignmentStatement*>(this);
}

BreakStatement* Statement::AsBreak() {
  assert(IsBreak());
  return static_cast<BreakStatement*>(this);
}

CaseStatement* Statement::AsCase() {
  assert(IsCase());
  return static_cast<CaseStatement*>(this);
}

ContinueStatement* Statement::AsContinue() {
  assert(IsContinue());
  return static_cast<ContinueStatement*>(this);
}

ElseStatement* Statement::AsElse() {
  assert(IsElse());
  return static_cast<ElseStatement*>(this);
}

FallthroughStatement* Statement::AsFallthrough() {
  assert(IsFallthrough());
  return static_cast<FallthroughStatement*>(this);
}

IfStatement* Statement::AsIf() {
  assert(IsIf());
  return static_cast<IfStatement*>(this);
}

KillStatement* Statement::AsKill() {
  assert(IsKill());
  return static_cast<KillStatement*>(this);
}

LoopStatement* Statement::AsLoop() {
  assert(IsLoop());
  return static_cast<LoopStatement*>(this);
}

ReturnStatement* Statement::AsReturn() {
  assert(IsReturn());
  return static_cast<ReturnStatement*>(this);
}

SwitchStatement* Statement::AsSwitch() {
  assert(IsSwitch());
  return static_cast<SwitchStatement*>(this);
}

VariableDeclStatement* Statement::AsVariableDecl() {
  assert(IsVariableDecl());
  return static_cast<VariableDeclStatement*>(this);
}

}  // namespace ast
}  // namespace tint
