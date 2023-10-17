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

#include <string>

#include "gtest/gtest.h"

#include "src/tint/fuzzers/tint_ast_fuzzer/mutations/wrap_unary_operator.h"
#include "src/tint/fuzzers/tint_ast_fuzzer/mutator.h"
#include "src/tint/fuzzers/tint_ast_fuzzer/node_id_map.h"
#include "src/tint/fuzzers/tint_ast_fuzzer/probability_context.h"
#include "src/tint/lang/wgsl/program/program_builder.h"
#include "src/tint/lang/wgsl/reader/reader.h"
#include "src/tint/lang/wgsl/writer/writer.h"

namespace tint::fuzzers::ast_fuzzer {
namespace {

TEST(WrapUnaryOperatorTest, Applicable1) {
    std::string content = R"(
    fn main() {
      var a = 5;
      if (a < 5) {
        a = 6;
      }
    }
  )";
    Source::File file("test.wgsl", content);
    auto program = wgsl::reader::Parse(&file);
    ASSERT_TRUE(program.IsValid()) << program.Diagnostics();

    NodeIdMap node_id_map(program);

    const auto& main_fn_statements = program.AST().Functions()[0]->body->statements;

    auto expression_id =
        node_id_map.GetId(main_fn_statements[1]->As<ast::IfStatement>()->condition);
    ASSERT_NE(expression_id, 0);

    ASSERT_TRUE(MaybeApplyMutation(
        program,
        MutationWrapUnaryOperator(expression_id, node_id_map.TakeFreshId(), core::UnaryOp::kNot),
        node_id_map, program, &node_id_map, nullptr));
    ASSERT_TRUE(program.IsValid()) << program.Diagnostics();

    wgsl::writer::Options options;
    auto result = wgsl::writer::Generate(program, options);
    ASSERT_TRUE(result) << result.Failure();

    std::string expected_shader = R"(fn main() {
  var a = 5;
  if (!((a < 5))) {
    a = 6;
  }
}
)";
    ASSERT_EQ(expected_shader, result->wgsl);
}

TEST(WrapUnaryOperatorTest, Applicable2) {
    std::string content = R"(
    fn main() {
      let a = vec3<bool>(true, false, true);
    }
  )";
    Source::File file("test.wgsl", content);
    auto program = wgsl::reader::Parse(&file);
    ASSERT_TRUE(program.IsValid()) << program.Diagnostics();

    NodeIdMap node_id_map(program);

    const auto& main_fn_statements = program.AST().Functions()[0]->body->statements;

    const auto* expr = main_fn_statements[0]
                           ->As<ast::VariableDeclStatement>()
                           ->variable->initializer->As<ast::Expression>();

    const auto expression_id = node_id_map.GetId(expr);
    ASSERT_NE(expression_id, 0);

    ASSERT_TRUE(MaybeApplyMutation(
        program,
        MutationWrapUnaryOperator(expression_id, node_id_map.TakeFreshId(), core::UnaryOp::kNot),
        node_id_map, program, &node_id_map, nullptr));
    ASSERT_TRUE(program.IsValid()) << program.Diagnostics();

    wgsl::writer::Options options;
    auto result = wgsl::writer::Generate(program, options);
    ASSERT_TRUE(result) << result.Failure();

    std::string expected_shader = R"(fn main() {
  let a = !(vec3<bool>(true, false, true));
}
)";
    ASSERT_EQ(expected_shader, result->wgsl);
}

TEST(WrapUnaryOperatorTest, Applicable3) {
    std::string content = R"(
    fn main() {
      var a : u32;
      a = 6u;
    }
  )";
    Source::File file("test.wgsl", content);
    auto program = wgsl::reader::Parse(&file);
    ASSERT_TRUE(program.IsValid()) << program.Diagnostics();

    NodeIdMap node_id_map(program);

    const auto& main_fn_statements = program.AST().Functions()[0]->body->statements;

    const auto* expr = main_fn_statements[1]->As<ast::AssignmentStatement>()->rhs;

    const auto expression_id = node_id_map.GetId(expr);
    ASSERT_NE(expression_id, 0);

    ASSERT_TRUE(
        MaybeApplyMutation(program,
                           MutationWrapUnaryOperator(expression_id, node_id_map.TakeFreshId(),
                                                     core::UnaryOp::kComplement),
                           node_id_map, program, &node_id_map, nullptr));
    ASSERT_TRUE(program.IsValid()) << program.Diagnostics();

    wgsl::writer::Options options;
    auto result = wgsl::writer::Generate(program, options);
    ASSERT_TRUE(result) << result.Failure();

    std::string expected_shader = R"(fn main() {
  var a : u32;
  a = ~(6u);
}
)";
    ASSERT_EQ(expected_shader, result->wgsl);
}

TEST(WrapUnaryOperatorTest, Applicable4) {
    std::string content = R"(
    fn main() -> vec2<bool> {
      var a = (vec2<u32> (1u, 2u) == vec2<u32> (1u, 2u));
      return a;
    }
  )";
    Source::File file("test.wgsl", content);
    auto program = wgsl::reader::Parse(&file);
    ASSERT_TRUE(program.IsValid()) << program.Diagnostics();

    NodeIdMap node_id_map(program);

    const auto& main_fn_statements = program.AST().Functions()[0]->body->statements;

    const auto* expr = main_fn_statements[0]
                           ->As<ast::VariableDeclStatement>()
                           ->variable->initializer->As<ast::BinaryExpression>()
                           ->lhs;

    const auto expression_id = node_id_map.GetId(expr);
    ASSERT_NE(expression_id, 0);

    ASSERT_TRUE(
        MaybeApplyMutation(program,
                           MutationWrapUnaryOperator(expression_id, node_id_map.TakeFreshId(),
                                                     core::UnaryOp::kComplement),
                           node_id_map, program, &node_id_map, nullptr));
    ASSERT_TRUE(program.IsValid()) << program.Diagnostics();

    wgsl::writer::Options options;
    auto result = wgsl::writer::Generate(program, options);
    ASSERT_TRUE(result) << result.Failure();

    std::string expected_shader = R"(fn main() -> vec2<bool> {
  var a = (~(vec2<u32>(1u, 2u)) == vec2<u32>(1u, 2u));
  return a;
}
)";
    ASSERT_EQ(expected_shader, result->wgsl);
}

TEST(WrapUnaryOperatorTest, Applicable5) {
    std::string content = R"(
    fn main() {
      let a : f32 = -(1.0);
    }
  )";
    Source::File file("test.wgsl", content);
    auto program = wgsl::reader::Parse(&file);
    ASSERT_TRUE(program.IsValid()) << program.Diagnostics();

    NodeIdMap node_id_map(program);

    const auto& main_fn_statements = program.AST().Functions()[0]->body->statements;

    const auto* expr = main_fn_statements[0]
                           ->As<ast::VariableDeclStatement>()
                           ->variable->initializer->As<ast::UnaryOpExpression>()
                           ->expr;

    const auto expression_id = node_id_map.GetId(expr);
    ASSERT_NE(expression_id, 0);

    ASSERT_TRUE(
        MaybeApplyMutation(program,
                           MutationWrapUnaryOperator(expression_id, node_id_map.TakeFreshId(),
                                                     core::UnaryOp::kNegation),
                           node_id_map, program, &node_id_map, nullptr));
    ASSERT_TRUE(program.IsValid()) << program.Diagnostics();

    wgsl::writer::Options options;
    auto result = wgsl::writer::Generate(program, options);
    ASSERT_TRUE(result) << result.Failure();

    std::string expected_shader = R"(fn main() {
  let a : f32 = -(-(1.0));
}
)";
    ASSERT_EQ(expected_shader, result->wgsl);
}

TEST(WrapUnaryOperatorTest, Applicable6) {
    std::string content = R"(
    fn main() {
      var a : vec4<f32> = vec4<f32>(-1.0, -1.0, -1.0, -1.0);
    }
  )";
    Source::File file("test.wgsl", content);
    auto program = wgsl::reader::Parse(&file);
    ASSERT_TRUE(program.IsValid()) << program.Diagnostics();

    NodeIdMap node_id_map(program);

    const auto& main_fn_statements = program.AST().Functions()[0]->body->statements;

    const auto* expr = main_fn_statements[0]
                           ->As<ast::VariableDeclStatement>()
                           ->variable->initializer->As<ast::Expression>();

    const auto expression_id = node_id_map.GetId(expr);
    ASSERT_NE(expression_id, 0);

    ASSERT_TRUE(
        MaybeApplyMutation(program,
                           MutationWrapUnaryOperator(expression_id, node_id_map.TakeFreshId(),
                                                     core::UnaryOp::kNegation),
                           node_id_map, program, &node_id_map, nullptr));
    ASSERT_TRUE(program.IsValid()) << program.Diagnostics();

    wgsl::writer::Options options;
    auto result = wgsl::writer::Generate(program, options);
    ASSERT_TRUE(result) << result.Failure();

    std::string expected_shader = R"(fn main() {
  var a : vec4<f32> = -(vec4<f32>(-(1.0), -(1.0), -(1.0), -(1.0)));
}
)";
    ASSERT_EQ(expected_shader, result->wgsl);
}

TEST(WrapUnaryOperatorTest, Applicable7) {
    std::string content = R"(
    fn main() {
      var a = 1;
      for(var i : i32 = 1; i < 5; i = i + 1) {
        a = a + 1;
      }
    }
  )";
    Source::File file("test.wgsl", content);
    auto program = wgsl::reader::Parse(&file);
    ASSERT_TRUE(program.IsValid()) << program.Diagnostics();

    NodeIdMap node_id_map(program);

    const auto& main_fn_statements = program.AST().Functions()[0]->body->statements;

    const auto* expr = main_fn_statements[1]
                           ->As<ast::ForLoopStatement>()
                           ->initializer->As<ast::VariableDeclStatement>()
                           ->variable->initializer->As<ast::Expression>();

    const auto expression_id = node_id_map.GetId(expr);
    ASSERT_NE(expression_id, 0);

    ASSERT_TRUE(
        MaybeApplyMutation(program,
                           MutationWrapUnaryOperator(expression_id, node_id_map.TakeFreshId(),
                                                     core::UnaryOp::kNegation),
                           node_id_map, program, &node_id_map, nullptr));
    ASSERT_TRUE(program.IsValid()) << program.Diagnostics();

    wgsl::writer::Options options;
    auto result = wgsl::writer::Generate(program, options);
    ASSERT_TRUE(result) << result.Failure();

    std::string expected_shader = R"(fn main() {
  var a = 1;
  for(var i : i32 = -(1); (i < 5); i = (i + 1)) {
    a = (a + 1);
  }
}
)";
    ASSERT_EQ(expected_shader, result->wgsl);
}

TEST(WrapUnaryOperatorTest, Applicable8) {
    std::string content = R"(
    fn main() {
      var a : vec4<i32> = vec4<i32>(1, 0, -1, 0);
    }
  )";
    Source::File file("test.wgsl", content);
    auto program = wgsl::reader::Parse(&file);
    ASSERT_TRUE(program.IsValid()) << program.Diagnostics();

    NodeIdMap node_id_map(program);

    const auto& main_fn_statements = program.AST().Functions()[0]->body->statements;

    const auto* expr = main_fn_statements[0]
                           ->As<ast::VariableDeclStatement>()
                           ->variable->initializer->As<ast::Expression>();

    const auto expression_id = node_id_map.GetId(expr);
    ASSERT_NE(expression_id, 0);

    ASSERT_TRUE(
        MaybeApplyMutation(program,
                           MutationWrapUnaryOperator(expression_id, node_id_map.TakeFreshId(),
                                                     core::UnaryOp::kComplement),
                           node_id_map, program, &node_id_map, nullptr));
    ASSERT_TRUE(program.IsValid()) << program.Diagnostics();

    wgsl::writer::Options options;
    auto result = wgsl::writer::Generate(program, options);
    ASSERT_TRUE(result) << result.Failure();

    std::string expected_shader = R"(fn main() {
  var a : vec4<i32> = ~(vec4<i32>(1, 0, -(1), 0));
}
)";
    ASSERT_EQ(expected_shader, result->wgsl);
}

TEST(WrapUnaryOperatorTest, NotApplicable1) {
    std::string content = R"(
    fn main() {
      let a = mat2x3<f32>(vec3<f32>(1.,0.,1.), vec3<f32>(0.,1.,0.));
    }
  )";
    Source::File file("test.wgsl", content);
    auto program = wgsl::reader::Parse(&file);
    ASSERT_TRUE(program.IsValid()) << program.Diagnostics();

    NodeIdMap node_id_map(program);

    const auto& main_fn_statements = program.AST().Functions()[0]->body->statements;

    const auto* expr = main_fn_statements[0]
                           ->As<ast::VariableDeclStatement>()
                           ->variable->initializer->As<ast::Expression>();

    const auto expression_id = node_id_map.GetId(expr);
    ASSERT_NE(expression_id, 0);

    // There is no unary operator that can be applied to matrix type.
    ASSERT_FALSE(
        MaybeApplyMutation(program,
                           MutationWrapUnaryOperator(expression_id, node_id_map.TakeFreshId(),
                                                     core::UnaryOp::kNegation),
                           node_id_map, program, &node_id_map, nullptr));
}

TEST(WrapUnaryOperatorTest, NotApplicable2) {
    std::string content = R"(
    fn main() {
      let a = 1;
    }
  )";
    Source::File file("test.wgsl", content);
    auto program = wgsl::reader::Parse(&file);
    ASSERT_TRUE(program.IsValid()) << program.Diagnostics();

    NodeIdMap node_id_map(program);

    const auto& main_fn_statements = program.AST().Functions()[0]->body->statements;

    const auto* expr = main_fn_statements[0]
                           ->As<ast::VariableDeclStatement>()
                           ->variable->initializer->As<ast::Expression>();

    const auto expression_id = node_id_map.GetId(expr);
    ASSERT_NE(expression_id, 0);

    // Not cannot be applied to integer types.
    ASSERT_FALSE(MaybeApplyMutation(
        program,
        MutationWrapUnaryOperator(expression_id, node_id_map.TakeFreshId(), core::UnaryOp::kNot),
        node_id_map, program, &node_id_map, nullptr));
}

TEST(WrapUnaryOperatorTest, NotApplicable3) {
    std::string content = R"(
    fn main() {
      let a = vec2<u32>(1u, 2u);
    }
  )";
    Source::File file("test.wgsl", content);
    auto program = wgsl::reader::Parse(&file);
    ASSERT_TRUE(program.IsValid()) << program.Diagnostics();

    NodeIdMap node_id_map(program);

    const auto& main_fn_statements = program.AST().Functions()[0]->body->statements;

    const auto* expr = main_fn_statements[0]
                           ->As<ast::VariableDeclStatement>()
                           ->variable->initializer->As<ast::Expression>();

    const auto expression_id = node_id_map.GetId(expr);
    ASSERT_NE(expression_id, 0);

    // Negation cannot be applied to unsigned integer scalar or vectors.
    ASSERT_FALSE(
        MaybeApplyMutation(program,
                           MutationWrapUnaryOperator(expression_id, node_id_map.TakeFreshId(),
                                                     core::UnaryOp::kNegation),
                           node_id_map, program, &node_id_map, nullptr));
}

TEST(WrapUnaryOperatorTest, NotApplicable4) {
    std::string content = R"(
    fn main() {
      let a = 1.5;
    }
  )";
    Source::File file("test.wgsl", content);
    auto program = wgsl::reader::Parse(&file);
    ASSERT_TRUE(program.IsValid()) << program.Diagnostics();

    NodeIdMap node_id_map(program);

    const auto& main_fn_statements = program.AST().Functions()[0]->body->statements;

    const auto* expr = main_fn_statements[0]
                           ->As<ast::VariableDeclStatement>()
                           ->variable->initializer->As<ast::Expression>();

    const auto expression_id = node_id_map.GetId(expr);
    ASSERT_NE(expression_id, 0);

    // Cannot wrap float types with complement operator.
    ASSERT_FALSE(
        MaybeApplyMutation(program,
                           MutationWrapUnaryOperator(expression_id, node_id_map.TakeFreshId(),
                                                     core::UnaryOp::kComplement),
                           node_id_map, program, &node_id_map, nullptr));
}

TEST(WrapUnaryOperatorTest, NotApplicable5) {
    std::string content = R"(
    fn main() {
      let a = 1.5;
    }
  )";
    Source::File file("test.wgsl", content);
    auto program = wgsl::reader::Parse(&file);
    ASSERT_TRUE(program.IsValid()) << program.Diagnostics();

    NodeIdMap node_id_map(program);

    const auto& main_fn_statements = program.AST().Functions()[0]->body->statements;

    const auto* expr = main_fn_statements[0]
                           ->As<ast::VariableDeclStatement>()
                           ->variable->initializer->As<ast::Expression>();

    const auto expression_id = node_id_map.GetId(expr);
    ASSERT_NE(expression_id, 0);

    // Id for the replacement expression is not fresh.
    ASSERT_FALSE(MaybeApplyMutation(
        program, MutationWrapUnaryOperator(expression_id, expression_id, core::UnaryOp::kNegation),
        node_id_map, program, &node_id_map, nullptr));
}

TEST(WrapUnaryOperatorTest, NotApplicable6) {
    std::string content = R"(
    fn main() {
      let a = 1.5;
    }
  )";
    Source::File file("test.wgsl", content);
    auto program = wgsl::reader::Parse(&file);
    ASSERT_TRUE(program.IsValid()) << program.Diagnostics();

    NodeIdMap node_id_map(program);

    const auto& main_fn_statements = program.AST().Functions()[0]->body->statements;

    const auto* statement = main_fn_statements[0]->As<ast::VariableDeclStatement>();

    const auto statement_id = node_id_map.GetId(statement);
    ASSERT_NE(statement_id, 0);

    // The id provided for the expression is not a valid expression type.
    ASSERT_FALSE(
        MaybeApplyMutation(program,
                           MutationWrapUnaryOperator(statement_id, node_id_map.TakeFreshId(),
                                                     core::UnaryOp::kNegation),
                           node_id_map, program, &node_id_map, nullptr));
}

TEST(WrapUnaryOperatorTest, NotApplicable_CallStmt) {
    std::string content = R"(
    fn main() {
      f();
    }
    fn f() -> bool {
      return false;
    }
  )";
    Source::File file("test.wgsl", content);
    auto program = wgsl::reader::Parse(&file);
    ASSERT_TRUE(program.IsValid()) << program.Diagnostics();

    NodeIdMap node_id_map(program);

    const auto& main_fn_statements = program.AST().Functions()[0]->body->statements;

    const auto* call_stmt = main_fn_statements[0]->As<ast::CallStatement>();
    ASSERT_NE(call_stmt, nullptr);

    const auto expr_id = node_id_map.GetId(call_stmt->expr);
    ASSERT_NE(expr_id, 0);

    // The id provided for the expression is not a valid expression type.
    ASSERT_FALSE(MaybeApplyMutation(
        program, MutationWrapUnaryOperator(expr_id, node_id_map.TakeFreshId(), core::UnaryOp::kNot),
        node_id_map, program, &node_id_map, nullptr));
}

}  // namespace
}  // namespace tint::fuzzers::ast_fuzzer
