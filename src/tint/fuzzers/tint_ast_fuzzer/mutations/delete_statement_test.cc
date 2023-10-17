// Copyright 2021 The Dawn & Tint Authors
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

#include "src/tint/fuzzers/tint_ast_fuzzer/mutations/delete_statement.h"

#include <functional>
#include <string>

#include "gtest/gtest.h"

#include "src/tint/fuzzers/tint_ast_fuzzer/mutator.h"
#include "src/tint/fuzzers/tint_ast_fuzzer/node_id_map.h"
#include "src/tint/fuzzers/tint_ast_fuzzer/probability_context.h"
#include "src/tint/lang/wgsl/ast/assignment_statement.h"
#include "src/tint/lang/wgsl/ast/block_statement.h"
#include "src/tint/lang/wgsl/ast/case_statement.h"
#include "src/tint/lang/wgsl/ast/for_loop_statement.h"
#include "src/tint/lang/wgsl/ast/if_statement.h"
#include "src/tint/lang/wgsl/ast/switch_statement.h"
#include "src/tint/lang/wgsl/program/program_builder.h"
#include "src/tint/lang/wgsl/reader/reader.h"
#include "src/tint/lang/wgsl/writer/writer.h"

namespace tint::fuzzers::ast_fuzzer {
namespace {

void CheckStatementDeletionWorks(
    const std::string& original,
    const std::string& expected,
    const std::function<const ast::Statement*(const Program&)>& statement_finder) {
    Source::File original_file("original.wgsl", original);
    auto program = wgsl::reader::Parse(&original_file);

    Source::File expected_file("expected.wgsl", expected);
    auto expected_program = wgsl::reader::Parse(&expected_file);

    ASSERT_TRUE(program.IsValid()) << program.Diagnostics();
    ASSERT_TRUE(expected_program.IsValid()) << expected_program.Diagnostics();

    NodeIdMap node_id_map(program);
    const auto* statement = statement_finder(program);
    ASSERT_NE(statement, nullptr);
    auto statement_id = node_id_map.GetId(statement);
    ASSERT_NE(statement_id, 0);
    ASSERT_TRUE(MaybeApplyMutation(program, MutationDeleteStatement(statement_id), node_id_map,
                                   program, &node_id_map, nullptr));
    ASSERT_TRUE(program.IsValid()) << program.Diagnostics();
    wgsl::writer::Options options;
    auto transformed_result = wgsl::writer::Generate(program, options);
    auto expected_result = wgsl::writer::Generate(expected_program, options);
    ASSERT_TRUE(transformed_result) << transformed_result.Failure();
    ASSERT_TRUE(expected_result) << expected_result.Failure();
    ASSERT_EQ(expected_result->wgsl, transformed_result->wgsl);
}

void CheckStatementDeletionNotAllowed(
    const std::string& original,
    const std::function<const ast::Statement*(const Program&)>& statement_finder) {
    Source::File original_file("original.wgsl", original);
    auto program = wgsl::reader::Parse(&original_file);

    ASSERT_TRUE(program.IsValid()) << program.Diagnostics();

    NodeIdMap node_id_map(program);
    const auto* statement = statement_finder(program);
    ASSERT_NE(statement, nullptr);
    auto statement_id = node_id_map.GetId(statement);
    ASSERT_NE(statement_id, 0);
    ASSERT_FALSE(MaybeApplyMutation(program, MutationDeleteStatement(statement_id), node_id_map,
                                    program, &node_id_map, nullptr));
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
