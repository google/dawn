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

#include "src/tint/fuzzers/tint_ast_fuzzer/mutations/delete_statement.h"

#include <functional>
#include <string>

#include "gtest/gtest.h"

#include "src/tint/ast/assignment_statement.h"
#include "src/tint/ast/block_statement.h"
#include "src/tint/ast/case_statement.h"
#include "src/tint/ast/for_loop_statement.h"
#include "src/tint/ast/if_statement.h"
#include "src/tint/ast/switch_statement.h"
#include "src/tint/fuzzers/tint_ast_fuzzer/mutator.h"
#include "src/tint/fuzzers/tint_ast_fuzzer/node_id_map.h"
#include "src/tint/fuzzers/tint_ast_fuzzer/probability_context.h"
#include "src/tint/program_builder.h"
#include "src/tint/reader/wgsl/parser.h"
#include "src/tint/writer/wgsl/generator.h"

namespace tint::fuzzers::ast_fuzzer {
namespace {

void CheckStatementDeletionWorks(
    const std::string& original,
    const std::string& expected,
    const std::function<const ast::Statement*(const Program&)>& statement_finder) {
    Source::File original_file("original.wgsl", original);
    auto program = reader::wgsl::Parse(&original_file);

    Source::File expected_file("expected.wgsl", expected);
    auto expected_program = reader::wgsl::Parse(&expected_file);

    ASSERT_TRUE(program.IsValid()) << program.Diagnostics().str();
    ASSERT_TRUE(expected_program.IsValid()) << expected_program.Diagnostics().str();

    NodeIdMap node_id_map(program);
    const auto* statement = statement_finder(program);
    ASSERT_NE(statement, nullptr);
    auto statement_id = node_id_map.GetId(statement);
    ASSERT_NE(statement_id, 0);
    ASSERT_TRUE(MaybeApplyMutation(program, MutationDeleteStatement(statement_id), node_id_map,
                                   &program, &node_id_map, nullptr));
    ASSERT_TRUE(program.IsValid()) << program.Diagnostics().str();
    writer::wgsl::Options options;
    auto transformed_result = writer::wgsl::Generate(&program, options);
    auto expected_result = writer::wgsl::Generate(&expected_program, options);
    ASSERT_TRUE(transformed_result.success) << transformed_result.error;
    ASSERT_TRUE(expected_result.success) << expected_result.error;
    ASSERT_EQ(expected_result.wgsl, transformed_result.wgsl);
}

void CheckStatementDeletionNotAllowed(
    const std::string& original,
    const std::function<const ast::Statement*(const Program&)>& statement_finder) {
    Source::File original_file("original.wgsl", original);
    auto program = reader::wgsl::Parse(&original_file);

    ASSERT_TRUE(program.IsValid()) << program.Diagnostics().str();

    NodeIdMap node_id_map(program);
    const auto* statement = statement_finder(program);
    ASSERT_NE(statement, nullptr);
    auto statement_id = node_id_map.GetId(statement);
    ASSERT_NE(statement_id, 0);
    ASSERT_FALSE(MaybeApplyMutation(program, MutationDeleteStatement(statement_id), node_id_map,
                                    &program, &node_id_map, nullptr));
}

TEST(DeleteStatementTest, DeleteAssignStatement) {
    auto original = R"(
    fn main() {
      {
        var a : i32 = 5;
        a = 6;
      }
    })";
    auto expected = R"(fn main() {
  {
    var a : i32 = 5;
  }
}
)";
    auto statement_finder = [](const Program& program) -> const ast::Statement* {
        return program.AST()
            .Functions()[0]
            ->body->statements[0]
            ->As<ast::BlockStatement>()
            ->statements[1]
            ->As<ast::AssignmentStatement>();
    };
    CheckStatementDeletionWorks(original, expected, statement_finder);
}

TEST(DeleteStatementTest, DeleteForStatement) {
    auto original =
        R"(
    fn main() {
      for (var i : i32 = 0; i < 10; i++) {
      }
    }
  )";
    auto expected = "fn main() { }";
    auto statement_finder = [](const Program& program) -> const ast::Statement* {
        return program.AST().Functions()[0]->body->statements[0]->As<ast::ForLoopStatement>();
    };
    CheckStatementDeletionWorks(original, expected, statement_finder);
}

TEST(DeleteStatementTest, DeleteIfStatement) {
    auto original =
        R"(
    fn main() {
      if (true) { } else { }
    }
  )";
    auto expected = "fn main() { }";
    auto statement_finder = [](const Program& program) -> const ast::Statement* {
        return program.AST().Functions()[0]->body->statements[0]->As<ast::IfStatement>();
    };
    CheckStatementDeletionWorks(original, expected, statement_finder);
}

TEST(DeleteStatementTest, DeleteBlockStatement) {
    auto original = "fn main() { { } }";
    auto expected = "fn main() { }";
    auto statement_finder = [](const Program& program) -> const ast::Statement* {
        return program.AST().Functions()[0]->body->statements[0]->As<ast::BlockStatement>();
    };
    CheckStatementDeletionWorks(original, expected, statement_finder);
}

TEST(DeleteStatementTest, DeleteSwitchStatement) {
    auto original = R"(
fn main() {
  switch(1) {
    case 0, 1: {
    }
    case 2, default: {
    }
  }
})";
    auto expected = R"(fn main() { })";
    auto statement_finder = [](const Program& program) -> const ast::Statement* {
        return program.AST().Functions()[0]->body->statements[0]->As<ast::SwitchStatement>();
    };
    CheckStatementDeletionWorks(original, expected, statement_finder);
}

TEST(DeleteStatementTest, DeleteCaseStatement) {
    auto original = R"(
fn main() {
  switch(1) {
    case 0, 1: {
    }
    case 2, default: {
    }
  }
})";
    auto expected = R"(
fn main() {
  switch(1) {
    case 2, default: {
    }
  }
})";
    auto statement_finder = [](const Program& program) -> const ast::Statement* {
        return program.AST()
            .Functions()[0]
            ->body->statements[0]
            ->As<ast::SwitchStatement>()
            ->body[0]
            ->As<ast::CaseStatement>();
    };
    CheckStatementDeletionWorks(original, expected, statement_finder);
}

TEST(DeleteStatementTest, DeleteElse) {
    auto original = R"(
fn main() {
  if (true) {
  } else {
  }
})";
    auto expected = R"(
fn main() {
  if (true) {
  }
})";
    auto statement_finder = [](const Program& program) -> const ast::Statement* {
        return program.AST()
            .Functions()[0]
            ->body->statements[0]
            ->As<ast::IfStatement>()
            ->else_statement;
    };
    CheckStatementDeletionWorks(original, expected, statement_finder);
}

TEST(DeleteStatementTest, DeleteCall) {
    auto original = R"(
fn main() {
  workgroupBarrier();
})";
    auto expected = R"(
fn main() {
})";
    auto statement_finder = [](const Program& program) -> const ast::Statement* {
        return program.AST().Functions()[0]->body->statements[0]->As<ast::CallStatement>();
    };
    CheckStatementDeletionWorks(original, expected, statement_finder);
}

TEST(DeleteStatementTest, DeleteCompoundAssign) {
    auto original = R"(
fn main() {
  var x : i32 = 0;
  x += 2;;
})";
    auto expected = R"(
fn main() {
  var x : i32 = 0;
})";
    auto statement_finder = [](const Program& program) -> const ast::Statement* {
        return program.AST()
            .Functions()[0]
            ->body->statements[1]
            ->As<ast::CompoundAssignmentStatement>();
    };
    CheckStatementDeletionWorks(original, expected, statement_finder);
}

TEST(DeleteStatementTest, DeleteLoop) {
    auto original = R"(
fn main() {
  var x : i32 = 0;
  loop {
    if (x > 100) {
      break;
    }
    continuing {
      x++;
    }
  }
})";
    auto expected = R"(
fn main() {
  var x : i32 = 0;
})";
    auto statement_finder = [](const Program& program) -> const ast::Statement* {
        return program.AST().Functions()[0]->body->statements[1]->As<ast::LoopStatement>();
    };
    CheckStatementDeletionWorks(original, expected, statement_finder);
}

TEST(DeleteStatementTest, DeleteContinuingBlock) {
    auto original = R"(
fn main() {
  var x : i32 = 0;
  loop {
    if (x > 100) {
      break;
    }
    continuing {
      x++;
    }
  }
})";
    auto expected = R"(
fn main() {
  var x : i32 = 0;
  loop {
    if (x > 100) {
      break;
    }
  }
})";
    auto statement_finder = [](const Program& program) -> const ast::Statement* {
        return program.AST()
            .Functions()[0]
            ->body->statements[1]
            ->As<ast::LoopStatement>()
            ->continuing;
    };
    CheckStatementDeletionWorks(original, expected, statement_finder);
}

TEST(DeleteStatementTest, DeleteContinue) {
    auto original = R"(
fn main() {
  var x : i32 = 0;
  loop {
    if (x > 100) {
      break;
    }
    continue;
    continuing {
      x++;
    }
  }
})";
    auto expected = R"(
fn main() {
  var x : i32 = 0;
  loop {
    if (x > 100) {
      break;
    }
    continuing {
      x++;
    }
  }
})";
    auto statement_finder = [](const Program& program) -> const ast::Statement* {
        return program.AST()
            .Functions()[0]
            ->body->statements[1]
            ->As<ast::LoopStatement>()
            ->body->statements[1]
            ->As<ast::ContinueStatement>();
    };
    CheckStatementDeletionWorks(original, expected, statement_finder);
}

TEST(DeleteStatementTest, DeleteIncrement) {
    auto original = R"(
fn main() {
  var x : i32 = 0;
  loop {
    if (x > 100) {
      break;
    }
    continuing {
      x++;
    }
  }
})";
    auto expected = R"(
fn main() {
  var x : i32 = 0;
  loop {
    if (x > 100) {
      break;
    }
    continuing {
    }
  }
})";
    auto statement_finder = [](const Program& program) -> const ast::Statement* {
        return program.AST()
            .Functions()[0]
            ->body->statements[1]
            ->As<ast::LoopStatement>()
            ->continuing->statements[0]
            ->As<ast::IncrementDecrementStatement>();
    };
    CheckStatementDeletionWorks(original, expected, statement_finder);
}

TEST(DeleteStatementTest, DeleteForLoopInitializer) {
    auto original = R"(
fn main() {
  var x : i32;
  for (x = 0; x < 100; x++) {
  }
})";
    auto expected = R"(
fn main() {
  var x : i32;
  for (; x < 100; x++) {
  }
})";
    auto statement_finder = [](const Program& program) -> const ast::Statement* {
        return program.AST()
            .Functions()[0]
            ->body->statements[1]
            ->As<ast::ForLoopStatement>()
            ->initializer->As<ast::AssignmentStatement>();
    };
    CheckStatementDeletionWorks(original, expected, statement_finder);
}

TEST(DeleteStatementTest, DeleteForLoopContinuing) {
    auto original = R"(
fn main() {
  var x : i32;
  for (x = 0; x < 100; x++) {
  }
})";
    auto expected = R"(
fn main() {
  var x : i32;
  for (x = 0; x < 100;) {
  }
})";
    auto statement_finder = [](const Program& program) -> const ast::Statement* {
        return program.AST()
            .Functions()[0]
            ->body->statements[1]
            ->As<ast::ForLoopStatement>()
            ->continuing->As<ast::IncrementDecrementStatement>();
    };
    CheckStatementDeletionWorks(original, expected, statement_finder);
}

TEST(DeleteStatementTest, AllowDeletionOfInnerLoopWithBreak) {
    auto original = R"(
fn main() {
  loop {
    loop {
      break;
    }
    break;
  }
})";
    auto expected = R"(
fn main() {
  loop {
    break;
  }
})";
    auto statement_finder = [](const Program& program) -> const ast::Statement* {
        return program.AST()
            .Functions()[0]
            ->body->statements[0]
            ->As<ast::LoopStatement>()
            ->body->statements[0]
            ->As<ast::LoopStatement>();
    };
    CheckStatementDeletionWorks(original, expected, statement_finder);
}

TEST(DeleteStatementTest, AllowDeletionOfInnerCaseWithBreak) {
    auto original = R"(
fn main() {
  loop {
    switch(0) {
      case 1: {
        break;
      }
      default: {
      }
    }
    break;
  }
})";
    auto expected = R"(
fn main() {
  loop {
    switch(0) {
      default: {
      }
    }
    break;
  }
})";
    auto statement_finder = [](const Program& program) -> const ast::Statement* {
        return program.AST()
            .Functions()[0]
            ->body->statements[0]
            ->As<ast::LoopStatement>()
            ->body->statements[0]
            ->As<ast::SwitchStatement>()
            ->body[0];
    };
    CheckStatementDeletionWorks(original, expected, statement_finder);
}

TEST(DeleteStatementTest, AllowDeletionOfBreakFromSwitch) {
    auto original = R"(
fn main() {
  switch(0) {
    case 1: {
      break;
    }
    default: {
    }
  }
})";
    auto expected = R"(
fn main() {
  switch(0) {
    case 1: {
    }
    default: {
    }
  }
})";
    auto statement_finder = [](const Program& program) -> const ast::Statement* {
        return program.AST()
            .Functions()[0]
            ->body->statements[0]
            ->As<ast::SwitchStatement>()
            ->body[0]
            ->body->statements[0]
            ->As<ast::BreakStatement>();
    };
    CheckStatementDeletionWorks(original, expected, statement_finder);
}

TEST(DeleteStatementTest, DoNotDeleteVariableDeclaration) {
    auto original = R"(
fn main() {
  var x : i32;
})";
    auto statement_finder = [](const Program& program) -> const ast::Statement* {
        return program.AST().Functions()[0]->body->statements[0]->As<ast::VariableDeclStatement>();
    };
    CheckStatementDeletionNotAllowed(original, statement_finder);
}

TEST(DeleteStatementTest, DoNotDeleteCaseDueToDefault) {
    auto original = R"(
fn main() {
  switch(1) {
    case 2, default: {
    }
  }
})";
    auto statement_finder = [](const Program& program) -> const ast::Statement* {
        return program.AST()
            .Functions()[0]
            ->body->statements[0]
            ->As<ast::SwitchStatement>()
            ->body[0]
            ->As<ast::CaseStatement>();
    };
    CheckStatementDeletionNotAllowed(original, statement_finder);
}

TEST(DeleteStatementTest, DoNotMakeLoopInfinite1) {
    auto original = R"(
fn main() {
  loop {
    break;
  }
})";
    auto statement_finder = [](const Program& program) -> const ast::Statement* {
        return program.AST()
            .Functions()[0]
            ->body->statements[0]
            ->As<ast::LoopStatement>()
            ->body->statements[0]
            ->As<ast::BreakStatement>();
    };
    CheckStatementDeletionNotAllowed(original, statement_finder);
}

TEST(DeleteStatementTest, DoNotMakeLoopInfinite2) {
    auto original = R"(
fn main() {
  loop {
    if (true) {
      break;
    }
  }
})";
    auto statement_finder = [](const Program& program) -> const ast::Statement* {
        return program.AST()
            .Functions()[0]
            ->body->statements[0]
            ->As<ast::LoopStatement>()
            ->body->statements[0]
            ->As<ast::IfStatement>();
    };
    CheckStatementDeletionNotAllowed(original, statement_finder);
}

TEST(DeleteStatementTest, DoNotRemoveReturn) {
    auto original = R"(
fn main() {
  return;
})";
    auto statement_finder = [](const Program& program) -> const ast::Statement* {
        return program.AST().Functions()[0]->body->statements[0]->As<ast::ReturnStatement>();
    };
    CheckStatementDeletionNotAllowed(original, statement_finder);
}

TEST(DeleteStatementTest, DoNotRemoveStatementContainingReturn) {
    auto original = R"(
fn foo() -> i32 {
  if (true) {
    return 1;
  } else {
    return 2;
  }
})";
    auto statement_finder = [](const Program& program) -> const ast::Statement* {
        return program.AST().Functions()[0]->body->statements[0]->As<ast::IfStatement>();
    };
    CheckStatementDeletionNotAllowed(original, statement_finder);
}

TEST(DeleteStatementTest, DoNotRemoveForLoopBody) {
    auto original = R"(
fn main() {
  for(var i : i32 = 0; i < 10; i++) {
  }
})";
    auto statement_finder = [](const Program& program) -> const ast::Statement* {
        return program.AST().Functions()[0]->body->statements[0]->As<ast::ForLoopStatement>()->body;
    };
    CheckStatementDeletionNotAllowed(original, statement_finder);
}

TEST(DeleteStatementTest, DoNotRemoveWhileBody) {
    auto original = R"(
fn main() {
  var i : i32 = 0;
  while(i < 10) {
    i++;
  }
})";
    auto statement_finder = [](const Program& program) -> const ast::Statement* {
        return program.AST().Functions()[0]->body->statements[1]->As<ast::WhileStatement>()->body;
    };
    CheckStatementDeletionNotAllowed(original, statement_finder);
}

TEST(DeleteStatementTest, DoNotRemoveIfBody) {
    auto original = R"(
fn main() {
  if(true) {
  }
})";
    auto statement_finder = [](const Program& program) -> const ast::Statement* {
        return program.AST().Functions()[0]->body->statements[0]->As<ast::IfStatement>()->body;
    };
    CheckStatementDeletionNotAllowed(original, statement_finder);
}

TEST(DeleteStatementTest, DoNotRemoveFunctionBody) {
    auto original = R"(
fn main() {
})";
    auto statement_finder = [](const Program& program) -> const ast::Statement* {
        return program.AST().Functions()[0]->body;
    };
    CheckStatementDeletionNotAllowed(original, statement_finder);
}

}  // namespace
}  // namespace tint::fuzzers::ast_fuzzer
