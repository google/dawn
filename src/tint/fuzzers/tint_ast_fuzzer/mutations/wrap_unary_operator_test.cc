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

#include <string>

#include "gtest/gtest.h"

#include "src/tint/fuzzers/tint_ast_fuzzer/mutations/wrap_unary_operator.h"
#include "src/tint/fuzzers/tint_ast_fuzzer/mutator.h"
#include "src/tint/fuzzers/tint_ast_fuzzer/node_id_map.h"
#include "src/tint/fuzzers/tint_ast_fuzzer/probability_context.h"
#include "src/tint/program_builder.h"
#include "src/tint/reader/wgsl/parser.h"
#include "src/tint/writer/wgsl/generator.h"

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
    auto program = reader::wgsl::Parse(&file);
    ASSERT_TRUE(program.IsValid()) << program.Diagnostics().str();

    NodeIdMap node_id_map(program);

    const auto& main_fn_statements = program.AST().Functions()[0]->body->statements;

    auto expression_id =
        node_id_map.GetId(main_fn_statements[1]->As<ast::IfStatement>()->condition);
    ASSERT_NE(expression_id, 0);

    ASSERT_TRUE(MaybeApplyMutation(
        program,
        MutationWrapUnaryOperator(expression_id, node_id_map.TakeFreshId(), ast::UnaryOp::kNot),
        node_id_map, &program, &node_id_map, nullptr));
    ASSERT_TRUE(program.IsValid()) << program.Diagnostics().str();

    writer::wgsl::Options options;
    auto result = writer::wgsl::Generate(&program, options);
    ASSERT_TRUE(result.success) << result.error;

    std::string expected_shader = R"(fn main() {
  var a = 5;
  if (!((a < 5))) {
    a = 6;
  }
}
)";
    ASSERT_EQ(expected_shader, result.wgsl);
}

TEST(WrapUnaryOperatorTest, Applicable2) {
    std::string content = R"(
    fn main() {
      let a = vec3<bool>(true, false, true);
    }
  )";
    Source::File file("test.wgsl", content);
    auto program = reader::wgsl::Parse(&file);
    ASSERT_TRUE(program.IsValid()) << program.Diagnostics().str();

    NodeIdMap node_id_map(program);

    const auto& main_fn_statements = program.AST().Functions()[0]->body->statements;

    const auto* expr = main_fn_statements[0]
                           ->As<ast::VariableDeclStatement>()
                           ->variable->initializer->As<ast::Expression>();

    const auto expression_id = node_id_map.GetId(expr);
    ASSERT_NE(expression_id, 0);

    ASSERT_TRUE(MaybeApplyMutation(
        program,
        MutationWrapUnaryOperator(expression_id, node_id_map.TakeFreshId(), ast::UnaryOp::kNot),
        node_id_map, &program, &node_id_map, nullptr));
    ASSERT_TRUE(program.IsValid()) << program.Diagnostics().str();

    writer::wgsl::Options options;
    auto result = writer::wgsl::Generate(&program, options);
    ASSERT_TRUE(result.success) << result.error;

    std::string expected_shader = R"(fn main() {
  let a = !(vec3<bool>(true, false, true));
}
)";
    ASSERT_EQ(expected_shader, result.wgsl);
}

TEST(WrapUnaryOperatorTest, Applicable3) {
    std::string content = R"(
    fn main() {
      var a : u32;
      a = 6u;
    }
  )";
    Source::File file("test.wgsl", content);
    auto program = reader::wgsl::Parse(&file);
    ASSERT_TRUE(program.IsValid()) << program.Diagnostics().str();

    NodeIdMap node_id_map(program);

    const auto& main_fn_statements = program.AST().Functions()[0]->body->statements;

    const auto* expr = main_fn_statements[1]->As<ast::AssignmentStatement>()->rhs;

    const auto expression_id = node_id_map.GetId(expr);
    ASSERT_NE(expression_id, 0);

    ASSERT_TRUE(
        MaybeApplyMutation(program,
                           MutationWrapUnaryOperator(expression_id, node_id_map.TakeFreshId(),
                                                     ast::UnaryOp::kComplement),
                           node_id_map, &program, &node_id_map, nullptr));
    ASSERT_TRUE(program.IsValid()) << program.Diagnostics().str();

    writer::wgsl::Options options;
    auto result = writer::wgsl::Generate(&program, options);
    ASSERT_TRUE(result.success) << result.error;

    std::string expected_shader = R"(fn main() {
  var a : u32;
  a = ~(6u);
}
)";
    ASSERT_EQ(expected_shader, result.wgsl);
}

TEST(WrapUnaryOperatorTest, Applicable4) {
    std::string content = R"(
    fn main() -> vec2<bool> {
      var a = (vec2<u32> (1u, 2u) == vec2<u32> (1u, 2u));
      return a;
    }
  )";
    Source::File file("test.wgsl", content);
    auto program = reader::wgsl::Parse(&file);
    ASSERT_TRUE(program.IsValid()) << program.Diagnostics().str();

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
                                                     ast::UnaryOp::kComplement),
                           node_id_map, &program, &node_id_map, nullptr));
    ASSERT_TRUE(program.IsValid()) << program.Diagnostics().str();

    writer::wgsl::Options options;
    auto result = writer::wgsl::Generate(&program, options);
    ASSERT_TRUE(result.success) << result.error;

    std::string expected_shader = R"(fn main() -> vec2<bool> {
  var a = (~(vec2<u32>(1u, 2u)) == vec2<u32>(1u, 2u));
  return a;
}
)";
    ASSERT_EQ(expected_shader, result.wgsl);
}

TEST(WrapUnaryOperatorTest, Applicable5) {
    std::string content = R"(
    fn main() {
      let a : f32 = -(1.0);
    }
  )";
    Source::File file("test.wgsl", content);
    auto program = reader::wgsl::Parse(&file);
    ASSERT_TRUE(program.IsValid()) << program.Diagnostics().str();

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
                                                     ast::UnaryOp::kNegation),
                           node_id_map, &program, &node_id_map, nullptr));
    ASSERT_TRUE(program.IsValid()) << program.Diagnostics().str();

    writer::wgsl::Options options;
    auto result = writer::wgsl::Generate(&program, options);
    ASSERT_TRUE(result.success) << result.error;

    std::string expected_shader = R"(fn main() {
  let a : f32 = -(-(1.0));
}
)";
    ASSERT_EQ(expected_shader, result.wgsl);
}

TEST(WrapUnaryOperatorTest, Applicable6) {
    std::string content = R"(
    fn main() {
      var a : vec4<f32> = vec4<f32>(-1.0, -1.0, -1.0, -1.0);
    }
  )";
    Source::File file("test.wgsl", content);
    auto program = reader::wgsl::Parse(&file);
    ASSERT_TRUE(program.IsValid()) << program.Diagnostics().str();

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
                                                     ast::UnaryOp::kNegation),
                           node_id_map, &program, &node_id_map, nullptr));
    ASSERT_TRUE(program.IsValid()) << program.Diagnostics().str();

    writer::wgsl::Options options;
    auto result = writer::wgsl::Generate(&program, options);
    ASSERT_TRUE(result.success) << result.error;

    std::string expected_shader = R"(fn main() {
  var a : vec4<f32> = -(vec4<f32>(-(1.0), -(1.0), -(1.0), -(1.0)));
}
)";
    ASSERT_EQ(expected_shader, result.wgsl);
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
    auto program = reader::wgsl::Parse(&file);
    ASSERT_TRUE(program.IsValid()) << program.Diagnostics().str();

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
                                                     ast::UnaryOp::kNegation),
                           node_id_map, &program, &node_id_map, nullptr));
    ASSERT_TRUE(program.IsValid()) << program.Diagnostics().str();

    writer::wgsl::Options options;
    auto result = writer::wgsl::Generate(&program, options);
    ASSERT_TRUE(result.success) << result.error;

    std::string expected_shader = R"(fn main() {
  var a = 1;
  for(var i : i32 = -(1); (i < 5); i = (i + 1)) {
    a = (a + 1);
  }
}
)";
    ASSERT_EQ(expected_shader, result.wgsl);
}

TEST(WrapUnaryOperatorTest, Applicable8) {
    std::string content = R"(
    fn main() {
      var a : vec4<i32> = vec4<i32>(1, 0, -1, 0);
    }
  )";
    Source::File file("test.wgsl", content);
    auto program = reader::wgsl::Parse(&file);
    ASSERT_TRUE(program.IsValid()) << program.Diagnostics().str();

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
                                                     ast::UnaryOp::kComplement),
                           node_id_map, &program, &node_id_map, nullptr));
    ASSERT_TRUE(program.IsValid()) << program.Diagnostics().str();

    writer::wgsl::Options options;
    auto result = writer::wgsl::Generate(&program, options);
    ASSERT_TRUE(result.success) << result.error;

    std::string expected_shader = R"(fn main() {
  var a : vec4<i32> = ~(vec4<i32>(1, 0, -(1), 0));
}
)";
    ASSERT_EQ(expected_shader, result.wgsl);
}

TEST(WrapUnaryOperatorTest, NotApplicable1) {
    std::string content = R"(
    fn main() {
      let a = mat2x3<f32>(vec3<f32>(1.,0.,1.), vec3<f32>(0.,1.,0.));
    }
  )";
    Source::File file("test.wgsl", content);
    auto program = reader::wgsl::Parse(&file);
    ASSERT_TRUE(program.IsValid()) << program.Diagnostics().str();

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
                                                     ast::UnaryOp::kNegation),
                           node_id_map, &program, &node_id_map, nullptr));
}

TEST(WrapUnaryOperatorTest, NotApplicable2) {
    std::string content = R"(
    fn main() {
      let a = 1;
    }
  )";
    Source::File file("test.wgsl", content);
    auto program = reader::wgsl::Parse(&file);
    ASSERT_TRUE(program.IsValid()) << program.Diagnostics().str();

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
        MutationWrapUnaryOperator(expression_id, node_id_map.TakeFreshId(), ast::UnaryOp::kNot),
        node_id_map, &program, &node_id_map, nullptr));
}

TEST(WrapUnaryOperatorTest, NotApplicable3) {
    std::string content = R"(
    fn main() {
      let a = vec2<u32>(1u, 2u);
    }
  )";
    Source::File file("test.wgsl", content);
    auto program = reader::wgsl::Parse(&file);
    ASSERT_TRUE(program.IsValid()) << program.Diagnostics().str();

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
                                                     ast::UnaryOp::kNegation),
                           node_id_map, &program, &node_id_map, nullptr));
}

TEST(WrapUnaryOperatorTest, NotApplicable4) {
    std::string content = R"(
    fn main() {
      let a = 1.5;
    }
  )";
    Source::File file("test.wgsl", content);
    auto program = reader::wgsl::Parse(&file);
    ASSERT_TRUE(program.IsValid()) << program.Diagnostics().str();

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
                                                     ast::UnaryOp::kComplement),
                           node_id_map, &program, &node_id_map, nullptr));
}

TEST(WrapUnaryOperatorTest, NotApplicable5) {
    std::string content = R"(
    fn main() {
      let a = 1.5;
    }
  )";
    Source::File file("test.wgsl", content);
    auto program = reader::wgsl::Parse(&file);
    ASSERT_TRUE(program.IsValid()) << program.Diagnostics().str();

    NodeIdMap node_id_map(program);

    const auto& main_fn_statements = program.AST().Functions()[0]->body->statements;

    const auto* expr = main_fn_statements[0]
                           ->As<ast::VariableDeclStatement>()
                           ->variable->initializer->As<ast::Expression>();

    const auto expression_id = node_id_map.GetId(expr);
    ASSERT_NE(expression_id, 0);

    // Id for the replacement expression is not fresh.
    ASSERT_FALSE(MaybeApplyMutation(
        program, MutationWrapUnaryOperator(expression_id, expression_id, ast::UnaryOp::kNegation),
        node_id_map, &program, &node_id_map, nullptr));
}

TEST(WrapUnaryOperatorTest, NotApplicable6) {
    std::string content = R"(
    fn main() {
      let a = 1.5;
    }
  )";
    Source::File file("test.wgsl", content);
    auto program = reader::wgsl::Parse(&file);
    ASSERT_TRUE(program.IsValid()) << program.Diagnostics().str();

    NodeIdMap node_id_map(program);

    const auto& main_fn_statements = program.AST().Functions()[0]->body->statements;

    const auto* statement = main_fn_statements[0]->As<ast::VariableDeclStatement>();

    const auto statement_id = node_id_map.GetId(statement);
    ASSERT_NE(statement_id, 0);

    // The id provided for the expression is not a valid expression type.
    ASSERT_FALSE(MaybeApplyMutation(
        program,
        MutationWrapUnaryOperator(statement_id, node_id_map.TakeFreshId(), ast::UnaryOp::kNegation),
        node_id_map, &program, &node_id_map, nullptr));
}

}  // namespace
}  // namespace tint::fuzzers::ast_fuzzer
