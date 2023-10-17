// Copyright 2022 The Dawn & Tint Authors
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// 1. Redistributions of source code must retain the above copyright notice, this
//    list of conditions and the following disclaimer.
//
// 2. Redistributions in binary form must reproduce the above copyright notice,
//    this list of conditions and the following disclaimer in the documentation
//    and/or other materials provided with the distribution.
//
// 3. Neither the name of the copyright holder nor the names of its
//    contributors may be used to endorse or promote products derived from
//    this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
// FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
// DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
// CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
// OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#include "src/tint/fuzzers/tint_ast_fuzzer/jump_tracker.h"

#include <string>

#include "gtest/gtest.h"

#include "src/tint/lang/wgsl/ast/block_statement.h"
#include "src/tint/lang/wgsl/ast/break_statement.h"
#include "src/tint/lang/wgsl/ast/discard_statement.h"
#include "src/tint/lang/wgsl/ast/for_loop_statement.h"
#include "src/tint/lang/wgsl/ast/if_statement.h"
#include "src/tint/lang/wgsl/ast/loop_statement.h"
#include "src/tint/lang/wgsl/ast/module.h"
#include "src/tint/lang/wgsl/ast/return_statement.h"
#include "src/tint/lang/wgsl/ast/switch_statement.h"
#include "src/tint/lang/wgsl/ast/while_statement.h"
#include "src/tint/lang/wgsl/program/program.h"
#include "src/tint/lang/wgsl/reader/reader.h"

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
    auto program = wgsl::reader::Parse(&file);
    ASSERT_TRUE(program.IsValid()) << program.Diagnostics();

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
    auto program = wgsl::reader::Parse(&file);
    ASSERT_TRUE(program.IsValid()) << program.Diagnostics();

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
    auto program = wgsl::reader::Parse(&file);
    ASSERT_TRUE(program.IsValid()) << program.Diagnostics();

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
