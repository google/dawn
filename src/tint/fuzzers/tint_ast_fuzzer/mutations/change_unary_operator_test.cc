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

#include "src/tint/fuzzers/tint_ast_fuzzer/mutations/change_unary_operator.h"
#include "src/tint/fuzzers/tint_ast_fuzzer/mutator.h"
#include "src/tint/fuzzers/tint_ast_fuzzer/node_id_map.h"
#include "src/tint/fuzzers/tint_ast_fuzzer/probability_context.h"
#include "src/tint/lang/wgsl/ast/call_statement.h"
#include "src/tint/lang/wgsl/program/program_builder.h"
#include "src/tint/lang/wgsl/reader/reader.h"
#include "src/tint/lang/wgsl/writer/writer.h"

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
    auto program = wgsl::reader::Parse(&file);
    ASSERT_TRUE(program.IsValid()) << program.Diagnostics();

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
        program, MutationChangeUnaryOperator(neg_a_id, core::UnaryOp::kComplement), node_id_map,
        program, &node_id_map, nullptr));
    ASSERT_FALSE(MaybeApplyMutation(program,
                                    MutationChangeUnaryOperator(neg_a_id, core::UnaryOp::kNot),
                                    node_id_map, program, &node_id_map, nullptr));
    ASSERT_FALSE(MaybeApplyMutation(program,
                                    MutationChangeUnaryOperator(neg_a_id, core::UnaryOp::kNegation),
                                    node_id_map, program, &node_id_map, nullptr));

    auto not_b_id = node_id_map.GetId(not_b_expr);
    // Only complement and negation is allowed for signed integer type.
    ASSERT_FALSE(MaybeApplyMutation(program,
                                    MutationChangeUnaryOperator(not_b_id, core::UnaryOp::kNot),
                                    node_id_map, program, &node_id_map, nullptr));
    // Cannot change to the same unary operator.
    ASSERT_FALSE(MaybeApplyMutation(
        program, MutationChangeUnaryOperator(not_b_id, core::UnaryOp::kComplement), node_id_map,
        program, &node_id_map, nullptr));

    auto not_c_id = node_id_map.GetId(not_c_expr);
    // Only complement is allowed for unsigned integer.Cannot change
    //  // the operator of float types to any other.
    ASSERT_FALSE(MaybeApplyMutation(program,
                                    MutationChangeUnaryOperator(not_c_id, core::UnaryOp::kNot),
                                    node_id_map, program, &node_id_map, nullptr));
    ASSERT_FALSE(MaybeApplyMutation(program,
                                    MutationChangeUnaryOperator(not_c_id, core::UnaryOp::kNegation),
                                    node_id_map, program, &node_id_map, nullptr));
    ASSERT_FALSE(MaybeApplyMutation(
        program, MutationChangeUnaryOperator(not_c_id, core::UnaryOp::kComplement), node_id_map,
        program, &node_id_map, nullptr));

    auto neg_d_id = node_id_map.GetId(neg_d_expr);
    // Only logical negation (not) is allowed for bool type.  Cannot change
    // the operator of float types to any other.
    ASSERT_FALSE(MaybeApplyMutation(
        program, MutationChangeUnaryOperator(neg_d_id, core::UnaryOp::kComplement), node_id_map,
        program, &node_id_map, nullptr));
    ASSERT_FALSE(MaybeApplyMutation(program,
                                    MutationChangeUnaryOperator(neg_d_id, core::UnaryOp::kNegation),
                                    node_id_map, program, &node_id_map, nullptr));
    ASSERT_FALSE(MaybeApplyMutation(program,
                                    MutationChangeUnaryOperator(neg_d_id, core::UnaryOp::kNot),
                                    node_id_map, program, &node_id_map, nullptr));
}

TEST(ChangeUnaryOperatorTest, Signed_Integer_Types_Applicable1) {
    std::string content = R"(
    fn main() {
      let a : i32 = 5;
      let comp_a = ~a;
    }
  )";
    Source::File file("test.wgsl", content);
    auto program = wgsl::reader::Parse(&file);
    ASSERT_TRUE(program.IsValid()) << program.Diagnostics();

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
                                   MutationChangeUnaryOperator(comp_a_id, core::UnaryOp::kNegation),
                                   node_id_map, program, &node_id_map, nullptr));
    ASSERT_TRUE(program.IsValid()) << program.Diagnostics();

    wgsl::writer::Options options;
    auto result = wgsl::writer::Generate(program, options);
    ASSERT_TRUE(result) << result.Failure();

    std::string expected_shader = R"(fn main() {
  let a : i32 = 5;
  let comp_a = -(a);
}
)";
    ASSERT_EQ(expected_shader, result->wgsl);
}

TEST(ChangeUnaryOperatorTest, Signed_Integer_Types_Applicable2) {
    std::string content = R"(
    fn main() {
      let b : vec3<i32> = vec3<i32>(1, 3, -1);
      var comp_b : vec3<i32> = ~b;
    })";
    Source::File file("test.wgsl", content);
    auto program = wgsl::reader::Parse(&file);
    ASSERT_TRUE(program.IsValid()) << program.Diagnostics();

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
                                   MutationChangeUnaryOperator(comp_b_id, core::UnaryOp::kNegation),
                                   node_id_map, program, &node_id_map, nullptr));
    ASSERT_TRUE(program.IsValid()) << program.Diagnostics();

    wgsl::writer::Options options;
    auto result = wgsl::writer::Generate(program, options);
    ASSERT_TRUE(result) << result.Failure();

    std::string expected_shader = R"(fn main() {
  let b : vec3<i32> = vec3<i32>(1, 3, -(1));
  var comp_b : vec3<i32> = -(b);
}
)";
    ASSERT_EQ(expected_shader, result->wgsl);
}

TEST(ChangeUnaryOperatorTest, Signed_Integer_Types_Applicable3) {
    std::string content = R"(
    fn main() {
      var a = -5;

      var neg_a = -(a);
    }
  )";
    Source::File file("test.wgsl", content);
    auto program = wgsl::reader::Parse(&file);
    ASSERT_TRUE(program.IsValid()) << program.Diagnostics();

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
    ASSERT_TRUE(MaybeApplyMutation(
        program, MutationChangeUnaryOperator(neg_a_id, core::UnaryOp::kComplement), node_id_map,
        program, &node_id_map, nullptr));
    ASSERT_TRUE(program.IsValid()) << program.Diagnostics();

    wgsl::writer::Options options;
    auto result = wgsl::writer::Generate(program, options);
    ASSERT_TRUE(result) << result.Failure();

    std::string expected_shader = R"(fn main() {
  var a = -(5);
  var neg_a = ~(a);
}
)";
    ASSERT_EQ(expected_shader, result->wgsl);
}

TEST(ChangeUnaryOperatorTest, Signed_Integer_Types_Applicable4) {
    std::string content = R"(
    fn main() {
      var b : vec3<i32> = vec3<i32>(1, 3, -1);
      let neg_b : vec3<i32> = -b;
    }
  )";
    Source::File file("test.wgsl", content);
    auto program = wgsl::reader::Parse(&file);
    ASSERT_TRUE(program.IsValid()) << program.Diagnostics();

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
    ASSERT_TRUE(MaybeApplyMutation(
        program, MutationChangeUnaryOperator(neg_b_id, core::UnaryOp::kComplement), node_id_map,
        program, &node_id_map, nullptr));
    ASSERT_TRUE(program.IsValid()) << program.Diagnostics();

    wgsl::writer::Options options;
    auto result = wgsl::writer::Generate(program, options);
    ASSERT_TRUE(result) << result.Failure();

    std::string expected_shader = R"(fn main() {
  var b : vec3<i32> = vec3<i32>(1, 3, -(1));
  let neg_b : vec3<i32> = ~(b);
}
)";
    ASSERT_EQ(expected_shader, result->wgsl);
}

}  // namespace
}  // namespace tint::fuzzers::ast_fuzzer
