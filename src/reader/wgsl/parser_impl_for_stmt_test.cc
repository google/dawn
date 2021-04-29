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

#include "src/reader/wgsl/parser_impl_test_helper.h"

namespace tint {
namespace reader {
namespace wgsl {
namespace {

class ForStmtTest : public ParserImplTest {
 public:
  void TestForLoop(std::string loop_str, std::string for_str) {
    auto p_loop = parser(loop_str);
    auto e_loop = p_loop->expect_statements();
    EXPECT_FALSE(e_loop.errored);
    EXPECT_FALSE(p_loop->has_error()) << p_loop->error();

    auto p_for = parser(for_str);
    auto e_for = p_for->expect_statements();
    EXPECT_FALSE(e_for.errored);
    EXPECT_FALSE(p_for->has_error()) << p_for->error();

    std::string loop = ast::BlockStatement({}, {}, e_loop.value).str(Sem());
    std::string for_ = ast::BlockStatement({}, {}, e_for.value).str(Sem());
    EXPECT_EQ(loop, for_);
  }
};

// Test an empty for loop.
TEST_F(ForStmtTest, Empty) {
  std::string for_str = "for (;;) { }";
  std::string loop_str = "loop { }";

  TestForLoop(loop_str, for_str);
}

// Test a for loop with non-empty body.
TEST_F(ForStmtTest, Body) {
  std::string for_str = "for (;;) { discard; }";
  std::string loop_str = "loop { discard; }";

  TestForLoop(loop_str, for_str);
}

// Test a for loop declaring a variable in the initializer statement.
TEST_F(ForStmtTest, InitializerStatementDecl) {
  std::string for_str = "for (var i: i32 ;;) { }";
  std::string loop_str = "{ var i: i32; loop { } }";

  TestForLoop(loop_str, for_str);
}

// Test a for loop declaring and initializing a variable in the initializer
// statement.
TEST_F(ForStmtTest, InitializerStatementDeclEqual) {
  std::string for_str = "for (var i: i32 = 0 ;;) { }";
  std::string loop_str = "{ var i: i32 = 0; loop { } }";

  TestForLoop(loop_str, for_str);
}

// Test a for loop declaring a const variable in the initializer statement.
TEST_F(ForStmtTest, InitializerStatementConstDecl) {
  std::string for_str = "for (let i: i32 = 0 ;;) { }";
  std::string loop_str = "{ let i: i32 = 0; loop { } }";

  TestForLoop(loop_str, for_str);
}

// Test a for loop assigning a variable in the initializer statement.
TEST_F(ForStmtTest, InitializerStatementAssignment) {
  std::string for_str = "var i: i32; for (i = 0 ;;) { }";
  std::string loop_str = "var i: i32; { i = 0; loop { } }";

  TestForLoop(loop_str, for_str);
}

// Test a for loop calling a function in the initializer statement.
TEST_F(ForStmtTest, InitializerStatementFuncCall) {
  std::string for_str = "for (a(b,c) ;;) { }";
  std::string loop_str = "{ a(b,c); loop { } }";

  TestForLoop(loop_str, for_str);
}

// Test a for loop with a break condition
TEST_F(ForStmtTest, BreakCondition) {
  std::string for_str = "for (; 0 == 1;) { }";
  std::string loop_str = "loop { if (!(0 == 1)) { break; } }";

  TestForLoop(loop_str, for_str);
}

// Test a for loop assigning a variable in the continuing statement.
TEST_F(ForStmtTest, ContinuingAssignment) {
  std::string for_str = "var x: i32; for (;; x = 2) { }";
  std::string loop_str = "var x: i32; loop { continuing { x = 2; }}";

  TestForLoop(loop_str, for_str);
}

// Test a for loop calling a function in the continuing statement.
TEST_F(ForStmtTest, ContinuingFuncCall) {
  std::string for_str = "for (;; a(b,c)) { }";
  std::string loop_str = "loop { continuing { a(b,c); }}";

  TestForLoop(loop_str, for_str);
}

// Test a for loop with all statements non-empty.
TEST_F(ForStmtTest, All) {
  std::string for_str =
      R"(for(var i : i32 = 0; i < 4; i = i + 1) {
       if (a == 0) {
        continue;
       }
       a = a + 2;
     })";

  std::string loop_str =
      R"({ // Introduce new scope for loop variable i
      var i : i32 = 0;
      loop {
        if (!(i < 4)) {
          break;
        }

        if (a == 0) {
          continue;
        }
        a = a + 2;

        continuing {
          i = i + 1;
        }
      }
    };)";

  TestForLoop(loop_str, for_str);
}

class ForStmtErrorTest : public ParserImplTest {
 public:
  void TestForWithError(std::string for_str, std::string error_str) {
    auto p_for = parser(for_str);
    auto e_for = p_for->for_stmt();

    EXPECT_FALSE(e_for.matched);
    EXPECT_TRUE(e_for.errored);
    EXPECT_TRUE(p_for->has_error());
    ASSERT_EQ(e_for.value, nullptr);
    EXPECT_EQ(p_for->error(), error_str);
  }
};

// Test a for loop with missing left parenthesis is invalid.
TEST_F(ForStmtErrorTest, MissingLeftParen) {
  std::string for_str = "for { }";
  std::string error_str = "1:5: expected '(' for for loop";

  TestForWithError(for_str, error_str);
}

// Test a for loop with missing first semicolon is invalid.
TEST_F(ForStmtErrorTest, MissingFirstSemicolon) {
  std::string for_str = "for () {}";
  std::string error_str = "1:6: expected ';' for initializer in for loop";

  TestForWithError(for_str, error_str);
}

// Test a for loop with missing second semicolon is invalid.
TEST_F(ForStmtErrorTest, MissingSecondSemicolon) {
  std::string for_str = "for (;) {}";
  std::string error_str = "1:7: expected ';' for condition in for loop";

  TestForWithError(for_str, error_str);
}

// Test a for loop with missing right parenthesis is invalid.
TEST_F(ForStmtErrorTest, MissingRightParen) {
  std::string for_str = "for (;; {}";
  std::string error_str = "1:9: expected ')' for for loop";

  TestForWithError(for_str, error_str);
}

// Test a for loop with missing left brace is invalid.
TEST_F(ForStmtErrorTest, MissingLeftBrace) {
  std::string for_str = "for (;;)";
  std::string error_str = "1:9: expected '{' for for loop";

  TestForWithError(for_str, error_str);
}

// Test a for loop with missing right brace is invalid.
TEST_F(ForStmtErrorTest, MissingRightBrace) {
  std::string for_str = "for (;;) {";
  std::string error_str = "1:11: expected '}' for for loop";

  TestForWithError(for_str, error_str);
}

// Test a for loop with an invalid initializer statement.
TEST_F(ForStmtErrorTest, InvalidInitializerAsConstDecl) {
  std::string for_str = "for (let x: i32;;) { }";
  std::string error_str = "1:16: expected '=' for let declaration";

  TestForWithError(for_str, error_str);
}

// Test a for loop with a initializer statement not matching
// variable_stmt | assignment_stmt | func_call_stmt.
TEST_F(ForStmtErrorTest, InvalidInitializerMatch) {
  std::string for_str = "for (if (true) {} ;;) { }";
  std::string error_str = "1:6: expected ';' for initializer in for loop";

  TestForWithError(for_str, error_str);
}

// Test a for loop with an invalid break condition.
TEST_F(ForStmtErrorTest, InvalidBreakConditionAsExpression) {
  std::string for_str = "for (; (0 == 1; ) { }";
  std::string error_str = "1:15: expected ')'";

  TestForWithError(for_str, error_str);
}

// Test a for loop with a break condition not matching
// logical_or_expression.
TEST_F(ForStmtErrorTest, InvalidBreakConditionMatch) {
  std::string for_str = "for (; var i: i32 = 0;) { }";
  std::string error_str = "1:8: expected ';' for condition in for loop";

  TestForWithError(for_str, error_str);
}

// Test a for loop with an invalid continuing statement.
TEST_F(ForStmtErrorTest, InvalidContinuingAsFuncCall) {
  std::string for_str = "for (;; a(,) ) { }";
  std::string error_str = "1:11: expected ')' for function call";

  TestForWithError(for_str, error_str);
}

// Test a for loop with a continuing statement not matching
// assignment_stmt | func_call_stmt.
TEST_F(ForStmtErrorTest, InvalidContinuingMatch) {
  std::string for_str = "for (;; var i: i32 = 0) { }";
  std::string error_str = "1:9: expected ')' for for loop";

  TestForWithError(for_str, error_str);
}

// Test a for loop with an invalid body.
TEST_F(ForStmtErrorTest, InvalidBody) {
  std::string for_str = "for (;;) { let x: i32; }";
  std::string error_str = "1:22: expected '=' for let declaration";

  TestForWithError(for_str, error_str);
}

// Test a for loop with a body not matching statements
TEST_F(ForStmtErrorTest, InvalidBodyMatch) {
  std::string for_str = "for (;;) { fn main() {} }";
  std::string error_str = "1:12: expected '}' for for loop";

  TestForWithError(for_str, error_str);
}

}  // namespace
}  // namespace wgsl
}  // namespace reader
}  // namespace tint
