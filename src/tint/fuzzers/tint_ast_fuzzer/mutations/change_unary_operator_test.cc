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

#include "src/tint/ast/call_statement.h"
#include "src/tint/fuzzers/tint_ast_fuzzer/mutations/change_unary_operator.h"
#include "src/tint/fuzzers/tint_ast_fuzzer/mutator.h"
#include "src/tint/fuzzers/tint_ast_fuzzer/node_id_map.h"
#include "src/tint/fuzzers/tint_ast_fuzzer/probability_context.h"
#include "src/tint/program_builder.h"
#include "src/tint/reader/wgsl/parser.h"
#include "src/tint/writer/wgsl/generator.h"

namespace tint::fuzzers::ast_fuzzer {
namespace {

TEST(ChangeUnaryOperatorTest, Operator_Not_Applicable) {
    std::string content = R"(
    fn main() {
      let a : f32 = 1.1;
      let b = vec2<i32>(1, -1);
      let c : u32 = 0u;
      let d : vec3<bool> = vec3<bool> (false, false, true);

      var neg_a = -a;
      var not_b = ~b;
      var not_c = ~c;
      var neg_d = !d;
    }
  )";
    Source::File file("test.wgsl", content);
    auto program = reader::wgsl::Parse(&file);
    ASSERT_TRUE(program.IsValid()) << program.Diagnostics().str();

    NodeIdMap node_id_map(program);

    const auto& main_fn_statements = program.AST().Functions()[0]->body->statements;

    // Get variable from statements.
    const auto* neg_a_var = main_fn_statements[4]->As<ast::VariableDeclStatement>()->variable;
    ASSERT_NE(neg_a_var, nullptr);

    const auto* not_b_var = main_fn_statements[5]->As<ast::VariableDeclStatement>()->variable;
    ASSERT_NE(not_b_var, nullptr);

    const auto* not_c_var = main_fn_statements[6]->As<ast::VariableDeclStatement>()->variable;
    ASSERT_NE(not_c_var, nullptr);

    const auto* neg_d_var = main_fn_statements[7]->As<ast::VariableDeclStatement>()->variable;
    ASSERT_NE(neg_d_var, nullptr);

    // Get the expression from variable declaration.
    const auto* neg_a_expr = neg_a_var->initializer->As<ast::UnaryOpExpression>();
    ASSERT_NE(neg_a_expr, nullptr);

    const auto* not_b_expr = not_b_var->initializer->As<ast::UnaryOpExpression>();
    ASSERT_NE(not_b_expr, nullptr);

    const auto* not_c_expr = not_c_var->initializer->As<ast::UnaryOpExpression>();
    ASSERT_NE(not_c_expr, nullptr);

    const auto* neg_d_expr = neg_d_var->initializer->As<ast::UnaryOpExpression>();
    ASSERT_NE(neg_d_expr, nullptr);

    // The following mutations are not applicable.
    auto neg_a_id = node_id_map.GetId(neg_a_expr);
    // Only negation is allowed for float type. Cannot change
    // the operator of float types to any other.
    ASSERT_FALSE(MaybeApplyMutation(
        program, MutationChangeUnaryOperator(neg_a_id, ast::UnaryOp::kComplement), node_id_map,
        &program, &node_id_map, nullptr));
    ASSERT_FALSE(MaybeApplyMutation(program,
                                    MutationChangeUnaryOperator(neg_a_id, ast::UnaryOp::kNot),
                                    node_id_map, &program, &node_id_map, nullptr));
    ASSERT_FALSE(MaybeApplyMutation(program,
                                    MutationChangeUnaryOperator(neg_a_id, ast::UnaryOp::kNegation),
                                    node_id_map, &program, &node_id_map, nullptr));

    auto not_b_id = node_id_map.GetId(not_b_expr);
    // Only complement and negation is allowed for signed integer type.
    ASSERT_FALSE(MaybeApplyMutation(program,
                                    MutationChangeUnaryOperator(not_b_id, ast::UnaryOp::kNot),
                                    node_id_map, &program, &node_id_map, nullptr));
    // Cannot change to the same unary operator.
    ASSERT_FALSE(MaybeApplyMutation(
        program, MutationChangeUnaryOperator(not_b_id, ast::UnaryOp::kComplement), node_id_map,
        &program, &node_id_map, nullptr));

    auto not_c_id = node_id_map.GetId(not_c_expr);
    // Only complement is allowed for unsigned integer.Cannot change
    //  // the operator of float types to any other.
    ASSERT_FALSE(MaybeApplyMutation(program,
                                    MutationChangeUnaryOperator(not_c_id, ast::UnaryOp::kNot),
                                    node_id_map, &program, &node_id_map, nullptr));
    ASSERT_FALSE(MaybeApplyMutation(program,
                                    MutationChangeUnaryOperator(not_c_id, ast::UnaryOp::kNegation),
                                    node_id_map, &program, &node_id_map, nullptr));
    ASSERT_FALSE(MaybeApplyMutation(
        program, MutationChangeUnaryOperator(not_c_id, ast::UnaryOp::kComplement), node_id_map,
        &program, &node_id_map, nullptr));

    auto neg_d_id = node_id_map.GetId(neg_d_expr);
    // Only logical negation (not) is allowed for bool type.  Cannot change
    // the operator of float types to any other.
    ASSERT_FALSE(MaybeApplyMutation(
        program, MutationChangeUnaryOperator(neg_d_id, ast::UnaryOp::kComplement), node_id_map,
        &program, &node_id_map, nullptr));
    ASSERT_FALSE(MaybeApplyMutation(program,
                                    MutationChangeUnaryOperator(neg_d_id, ast::UnaryOp::kNegation),
                                    node_id_map, &program, &node_id_map, nullptr));
    ASSERT_FALSE(MaybeApplyMutation(program,
                                    MutationChangeUnaryOperator(neg_d_id, ast::UnaryOp::kNot),
                                    node_id_map, &program, &node_id_map, nullptr));
}

TEST(ChangeUnaryOperatorTest, Signed_Integer_Types_Applicable1) {
    std::string content = R"(
    fn main() {
      let a : i32 = 5;
      let comp_a = ~a;
    }
  )";
    Source::File file("test.wgsl", content);
    auto program = reader::wgsl::Parse(&file);
    ASSERT_TRUE(program.IsValid()) << program.Diagnostics().str();

    NodeIdMap node_id_map(program);

    const auto& main_fn_statements = program.AST().Functions()[0]->body->statements;

    // Get variable from statements.
    const auto* comp_a_var = main_fn_statements[1]->As<ast::VariableDeclStatement>()->variable;
    ASSERT_NE(comp_a_var, nullptr);

    // Get the expression from variable declaration.
    const auto* comp_a_expr = comp_a_var->initializer->As<ast::UnaryOpExpression>();
    ASSERT_NE(comp_a_expr, nullptr);

    // Assert mutation to be applicable and apply mutation.
    auto comp_a_id = node_id_map.GetId(comp_a_expr);
    ASSERT_TRUE(MaybeApplyMutation(program,
                                   MutationChangeUnaryOperator(comp_a_id, ast::UnaryOp::kNegation),
                                   node_id_map, &program, &node_id_map, nullptr));
    ASSERT_TRUE(program.IsValid()) << program.Diagnostics().str();

    writer::wgsl::Options options;
    auto result = writer::wgsl::Generate(&program, options);
    ASSERT_TRUE(result.success) << result.error;

    std::string expected_shader = R"(fn main() {
  let a : i32 = 5;
  let comp_a = -(a);
}
)";
    ASSERT_EQ(expected_shader, result.wgsl);
}

TEST(ChangeUnaryOperatorTest, Signed_Integer_Types_Applicable2) {
    std::string content = R"(
    fn main() {
      let b : vec3<i32> = vec3<i32>(1, 3, -1);
      var comp_b : vec3<i32> = ~b;
    })";
    Source::File file("test.wgsl", content);
    auto program = reader::wgsl::Parse(&file);
    ASSERT_TRUE(program.IsValid()) << program.Diagnostics().str();

    NodeIdMap node_id_map(program);

    const auto& main_fn_statements = program.AST().Functions()[0]->body->statements;

    // Get variable from statements.
    const auto* comp_b_var = main_fn_statements[1]->As<ast::VariableDeclStatement>()->variable;
    ASSERT_NE(comp_b_var, nullptr);

    // Get the expression from variable declaration.
    const auto* comp_b_expr = comp_b_var->initializer->As<ast::UnaryOpExpression>();
    ASSERT_NE(comp_b_expr, nullptr);

    // Assert mutation to be applicable and apply mutation.
    auto comp_b_id = node_id_map.GetId(comp_b_expr);
    ASSERT_TRUE(MaybeApplyMutation(program,
                                   MutationChangeUnaryOperator(comp_b_id, ast::UnaryOp::kNegation),
                                   node_id_map, &program, &node_id_map, nullptr));
    ASSERT_TRUE(program.IsValid()) << program.Diagnostics().str();

    writer::wgsl::Options options;
    auto result = writer::wgsl::Generate(&program, options);
    ASSERT_TRUE(result.success) << result.error;

    std::string expected_shader = R"(fn main() {
  let b : vec3<i32> = vec3<i32>(1, 3, -(1));
  var comp_b : vec3<i32> = -(b);
}
)";
    ASSERT_EQ(expected_shader, result.wgsl);
}

TEST(ChangeUnaryOperatorTest, Signed_Integer_Types_Applicable3) {
    std::string content = R"(
    fn main() {
      var a = -5;

      var neg_a = -(a);
    }
  )";
    Source::File file("test.wgsl", content);
    auto program = reader::wgsl::Parse(&file);
    ASSERT_TRUE(program.IsValid()) << program.Diagnostics().str();

    NodeIdMap node_id_map(program);

    const auto& main_fn_statements = program.AST().Functions()[0]->body->statements;

    // Get variable from statements.
    const auto* neg_a_var = main_fn_statements[1]->As<ast::VariableDeclStatement>()->variable;
    ASSERT_NE(neg_a_var, nullptr);

    // Get the expression from variable declaration.
    const auto* neg_a_expr = neg_a_var->initializer->As<ast::UnaryOpExpression>();
    ASSERT_NE(neg_a_expr, nullptr);

    // Assert mutation to be applicable and apply mutation.
    auto neg_a_id = node_id_map.GetId(neg_a_expr);
    ASSERT_TRUE(MaybeApplyMutation(program,
                                   MutationChangeUnaryOperator(neg_a_id, ast::UnaryOp::kComplement),
                                   node_id_map, &program, &node_id_map, nullptr));
    ASSERT_TRUE(program.IsValid()) << program.Diagnostics().str();

    writer::wgsl::Options options;
    auto result = writer::wgsl::Generate(&program, options);
    ASSERT_TRUE(result.success) << result.error;

    std::string expected_shader = R"(fn main() {
  var a = -(5);
  var neg_a = ~(a);
}
)";
    ASSERT_EQ(expected_shader, result.wgsl);
}

TEST(ChangeUnaryOperatorTest, Signed_Integer_Types_Applicable4) {
    std::string content = R"(
    fn main() {
      var b : vec3<i32> = vec3<i32>(1, 3, -1);
      let neg_b : vec3<i32> = -b;
    }
  )";
    Source::File file("test.wgsl", content);
    auto program = reader::wgsl::Parse(&file);
    ASSERT_TRUE(program.IsValid()) << program.Diagnostics().str();

    NodeIdMap node_id_map(program);

    const auto& main_fn_statements = program.AST().Functions()[0]->body->statements;

    // Get variable from statements.
    const auto* neg_b_var = main_fn_statements[1]->As<ast::VariableDeclStatement>()->variable;
    ASSERT_NE(neg_b_var, nullptr);

    // Get the expression from variable declaration.
    const auto* neg_b_expr = neg_b_var->initializer->As<ast::UnaryOpExpression>();
    ASSERT_NE(neg_b_expr, nullptr);

    // Assert mutation to be applicable and apply mutation.
    auto neg_b_id = node_id_map.GetId(neg_b_expr);
    ASSERT_TRUE(MaybeApplyMutation(program,
                                   MutationChangeUnaryOperator(neg_b_id, ast::UnaryOp::kComplement),
                                   node_id_map, &program, &node_id_map, nullptr));
    ASSERT_TRUE(program.IsValid()) << program.Diagnostics().str();

    writer::wgsl::Options options;
    auto result = writer::wgsl::Generate(&program, options);
    ASSERT_TRUE(result.success) << result.error;

    std::string expected_shader = R"(fn main() {
  var b : vec3<i32> = vec3<i32>(1, 3, -(1));
  let neg_b : vec3<i32> = ~(b);
}
)";
    ASSERT_EQ(expected_shader, result.wgsl);
}

}  // namespace
}  // namespace tint::fuzzers::ast_fuzzer
