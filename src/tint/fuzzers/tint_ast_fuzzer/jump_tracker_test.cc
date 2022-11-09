// Copyright 2022 The Tint Authors.
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

#include "src/tint/fuzzers/tint_ast_fuzzer/jump_tracker.h"

#include <string>

#include "gtest/gtest.h"

#include "src/tint/ast/block_statement.h"
#include "src/tint/ast/break_statement.h"
#include "src/tint/ast/discard_statement.h"
#include "src/tint/ast/for_loop_statement.h"
#include "src/tint/ast/if_statement.h"
#include "src/tint/ast/loop_statement.h"
#include "src/tint/ast/module.h"
#include "src/tint/ast/return_statement.h"
#include "src/tint/ast/switch_statement.h"
#include "src/tint/ast/while_statement.h"
#include "src/tint/program.h"
#include "src/tint/reader/wgsl/parser.h"

namespace tint::fuzzers::ast_fuzzer {
namespace {

TEST(JumpTrackerTest, Breaks) {
    std::string content = R"(
fn main() {
  var x : u32;
  for (var i : i32 = 0; i < 100; i++) {
    if (i == 40) {
      {
        break;
      }
    }
    for (var j : i32 = 0; j < 10; j++) {
      loop {
        if (i > j) {
          break;
        }
        continuing {
          i++;
          j-=2;
        }
      }
      switch (j) {
        case 0: {
          if (i == j) {
            break;
          }
          i = i + 1;
          continue;
        }
        default: {
          break;
        }
      }
    }
  }
}
  )";
    Source::File file("test.wgsl", content);
    auto program = reader::wgsl::Parse(&file);
    ASSERT_TRUE(program.IsValid()) << program.Diagnostics().str();

    JumpTracker jump_tracker(program);

    const auto* outer_loop_body =
        program.AST().Functions()[0]->body->statements[1]->As<ast::ForLoopStatement>()->body;
    const auto* first_if = outer_loop_body->statements[0]->As<ast::IfStatement>();
    const auto* first_if_body = first_if->body;
    const auto* block_in_first_if = first_if_body->statements[0]->As<ast::BlockStatement>();
    const auto* break_in_first_if = block_in_first_if->statements[0]->As<ast::BreakStatement>();

    const auto* innermost_loop_body = outer_loop_body->statements[1]
                                          ->As<ast::ForLoopStatement>()
                                          ->body->statements[0]
                                          ->As<ast::LoopStatement>()
                                          ->body;
    const auto* innermost_loop_if = innermost_loop_body->statements[0]->As<ast::IfStatement>();
    const auto* innermost_loop_if_body = innermost_loop_if->body;
    const auto* break_in_innermost_loop =
        innermost_loop_if_body->statements[0]->As<ast::BreakStatement>();

    std::unordered_set<const ast::Statement*> containing_loop_break = {
        outer_loop_body,        first_if,
        first_if_body,          block_in_first_if,
        break_in_first_if,      innermost_loop_body,
        innermost_loop_if,      innermost_loop_if_body,
        break_in_innermost_loop};

    for (auto* node : program.ASTNodes().Objects()) {
        auto* stmt = node->As<ast::Statement>();
        if (stmt == nullptr) {
            continue;
        }
        if (containing_loop_break.count(stmt) > 0) {
            ASSERT_TRUE(jump_tracker.ContainsBreakForInnermostLoop(*stmt));
        } else {
            ASSERT_FALSE(jump_tracker.ContainsBreakForInnermostLoop(*stmt));
        }
    }
}

TEST(JumpTrackerTest, Returns) {
    std::string content = R"(
fn main() {
  var x : u32;
  for (var i : i32 = 0; i < 100; i++) {
    if (i == 40) {
      {
        return;
      }
    }
    for (var j : i32 = 0; j < 10; j++) {
      loop {
        if (i > j) {
          return;
        }
        continuing {
          i++;
          j-=2;
        }
      }
      switch (j) {
        case 0: {
          if (i == j) {
            break;
          }
          i = i + 1;
          continue;
        }
        default: {
          return;
        }
      }
    }
  }
}
  )";
    Source::File file("test.wgsl", content);
    auto program = reader::wgsl::Parse(&file);
    ASSERT_TRUE(program.IsValid()) << program.Diagnostics().str();

    JumpTracker jump_tracker(program);

    const auto* function_body = program.AST().Functions()[0]->body;
    const auto* outer_loop = function_body->statements[1]->As<ast::ForLoopStatement>();
    const auto* outer_loop_body = outer_loop->body;
    const auto* first_if = outer_loop_body->statements[0]->As<ast::IfStatement>();
    const auto* first_if_body = first_if->body;
    const auto* block_in_first_if = first_if_body->statements[0]->As<ast::BlockStatement>();
    const auto* return_in_first_if = block_in_first_if->statements[0]->As<ast::ReturnStatement>();
    const auto* inner_for_loop = outer_loop_body->statements[1]->As<ast::ForLoopStatement>();
    const auto* inner_for_loop_body = inner_for_loop->body;
    const auto* innermost_loop = inner_for_loop_body->statements[0]->As<ast::LoopStatement>();
    const auto* innermost_loop_body = innermost_loop->body;
    const auto* innermost_loop_if = innermost_loop_body->statements[0]->As<ast::IfStatement>();
    const auto* innermost_loop_if_body = innermost_loop_if->body;
    const auto* return_in_innermost_loop =
        innermost_loop_if_body->statements[0]->As<ast::ReturnStatement>();
    const auto* switch_statement = inner_for_loop_body->statements[1]->As<ast::SwitchStatement>();
    const auto* default_statement = switch_statement->body[1];
    const auto* default_statement_body = default_statement->body;
    const auto* return_in_default_statement =
        default_statement_body->statements[0]->As<ast::ReturnStatement>();

    std::unordered_set<const ast::Statement*> containing_return = {
        function_body,          outer_loop,
        outer_loop_body,        first_if,
        first_if_body,          block_in_first_if,
        return_in_first_if,     inner_for_loop,
        inner_for_loop_body,    innermost_loop,
        innermost_loop_body,    innermost_loop_if,
        innermost_loop_if_body, return_in_innermost_loop,
        switch_statement,       default_statement,
        default_statement_body, return_in_default_statement};

    for (auto* node : program.ASTNodes().Objects()) {
        auto* stmt = node->As<ast::Statement>();
        if (stmt == nullptr) {
            continue;
        }
        if (containing_return.count(stmt) > 0) {
            ASSERT_TRUE(jump_tracker.ContainsReturn(*stmt));
        } else {
            ASSERT_FALSE(jump_tracker.ContainsReturn(*stmt));
        }
    }
}

TEST(JumpTrackerTest, WhileLoop) {
    std::string content = R"(
fn main() {
  var x : u32;
  x = 0;
  while (x < 100) {
    if (x > 50) {
      break;
    }
    x = x + 1;
  }
}
  )";
    Source::File file("test.wgsl", content);
    auto program = reader::wgsl::Parse(&file);
    ASSERT_TRUE(program.IsValid()) << program.Diagnostics().str();

    JumpTracker jump_tracker(program);

    const auto* while_loop_body =
        program.AST().Functions()[0]->body->statements[2]->As<ast::WhileStatement>()->body;
    const auto* if_statement = while_loop_body->statements[0]->As<ast::IfStatement>();
    const auto* if_statement_body = if_statement->body;
    const auto* break_in_if = if_statement_body->statements[0]->As<ast::BreakStatement>();

    std::unordered_set<const ast::Statement*> containing_loop_break = {
        while_loop_body, if_statement, if_statement_body, break_in_if};

    for (auto* node : program.ASTNodes().Objects()) {
        auto* stmt = node->As<ast::Statement>();
        if (stmt == nullptr) {
            continue;
        }
        if (containing_loop_break.count(stmt) > 0) {
            ASSERT_TRUE(jump_tracker.ContainsBreakForInnermostLoop(*stmt));
        } else {
            ASSERT_FALSE(jump_tracker.ContainsBreakForInnermostLoop(*stmt));
        }
    }
}

}  // namespace
}  // namespace tint::fuzzers::ast_fuzzer
