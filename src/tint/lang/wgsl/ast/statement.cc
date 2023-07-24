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

#include "src/tint/lang/wgsl/ast/statement.h"

#include "src/tint/lang/wgsl/ast/assignment_statement.h"
#include "src/tint/lang/wgsl/ast/break_statement.h"
#include "src/tint/lang/wgsl/ast/call_statement.h"
#include "src/tint/lang/wgsl/ast/continue_statement.h"
#include "src/tint/lang/wgsl/ast/discard_statement.h"
#include "src/tint/lang/wgsl/ast/if_statement.h"
#include "src/tint/lang/wgsl/ast/loop_statement.h"
#include "src/tint/lang/wgsl/ast/return_statement.h"
#include "src/tint/lang/wgsl/ast/switch_statement.h"
#include "src/tint/lang/wgsl/ast/variable_decl_statement.h"

TINT_INSTANTIATE_TYPEINFO(tint::ast::Statement);

namespace tint::ast {

Statement::Statement(GenerationID pid, NodeID nid, const Source& src) : Base(pid, nid, src) {}

Statement::~Statement() = default;

const char* Statement::Name() const {
    if (Is<AssignmentStatement>()) {
        return "assignment statement";
    }
    if (Is<BlockStatement>()) {
        return "block statement";
    }
    if (Is<BreakStatement>()) {
        return "break statement";
    }
    if (Is<CaseStatement>()) {
        return "case statement";
    }
    if (Is<CallStatement>()) {
        return "function call";
    }
    if (Is<ContinueStatement>()) {
        return "continue statement";
    }
    if (Is<DiscardStatement>()) {
        return "discard statement";
    }
    if (Is<IfStatement>()) {
        return "if statement";
    }
    if (Is<LoopStatement>()) {
        return "loop statement";
    }
    if (Is<ReturnStatement>()) {
        return "return statement";
    }
    if (Is<SwitchStatement>()) {
        return "switch statement";
    }
    if (Is<VariableDeclStatement>()) {
        return "variable declaration";
    }
    return "statement";
}

}  // namespace tint::ast
