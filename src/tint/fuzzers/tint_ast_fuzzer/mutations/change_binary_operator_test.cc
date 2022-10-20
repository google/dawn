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

#include "src/tint/fuzzers/tint_ast_fuzzer/mutations/change_binary_operator.h"

#include <string>
#include <unordered_set>
#include <vector>

#include "gtest/gtest.h"

#include "src/tint/fuzzers/tint_ast_fuzzer/mutator.h"
#include "src/tint/fuzzers/tint_ast_fuzzer/node_id_map.h"
#include "src/tint/program_builder.h"
#include "src/tint/reader/wgsl/parser.h"
#include "src/tint/writer/wgsl/generator.h"

namespace tint::fuzzers::ast_fuzzer {
namespace {

std::string OpToString(ast::BinaryOp op) {
    switch (op) {
        case ast::BinaryOp::kNone:
            assert(false && "Unreachable");
            return "";
        case ast::BinaryOp::kAnd:
            return "&";
        case ast::BinaryOp::kOr:
            return "|";
        case ast::BinaryOp::kXor:
            return "^";
        case ast::BinaryOp::kLogicalAnd:
            return "&&";
        case ast::BinaryOp::kLogicalOr:
            return "||";
        case ast::BinaryOp::kEqual:
            return "==";
        case ast::BinaryOp::kNotEqual:
            return "!=";
        case ast::BinaryOp::kLessThan:
            return "<";
        case ast::BinaryOp::kGreaterThan:
            return ">";
        case ast::BinaryOp::kLessThanEqual:
            return "<=";
        case ast::BinaryOp::kGreaterThanEqual:
            return ">=";
        case ast::BinaryOp::kShiftLeft:
            return "<<";
        case ast::BinaryOp::kShiftRight:
            return ">>";
        case ast::BinaryOp::kAdd:
            return "+";
        case ast::BinaryOp::kSubtract:
            return "-";
        case ast::BinaryOp::kMultiply:
            return "*";
        case ast::BinaryOp::kDivide:
            return "/";
        case ast::BinaryOp::kModulo:
            return "%";
    }
}

TEST(ChangeBinaryOperatorTest, NotApplicable_Simple) {
    std::string content = R"(
    fn main() {
      let a : i32 = 1 + 2;
    }
  )";
    Source::File file("test.wgsl", content);
    auto program = reader::wgsl::Parse(&file);
    ASSERT_TRUE(program.IsValid()) << program.Diagnostics().str();

    NodeIdMap node_id_map(program);

    const auto& main_fn_stmts = program.AST().Functions()[0]->body->statements;

    const auto* a_var = main_fn_stmts[0]->As<ast::VariableDeclStatement>()->variable;
    ASSERT_NE(a_var, nullptr);

    auto a_var_id = node_id_map.GetId(a_var);

    const auto* sum_expr = a_var->initializer->As<ast::BinaryExpression>();
    ASSERT_NE(sum_expr, nullptr);

    auto sum_expr_id = node_id_map.GetId(sum_expr);
    ASSERT_NE(sum_expr_id, 0);

    // binary_expr_id is invalid.
    EXPECT_FALSE(MutationChangeBinaryOperator(0, ast::BinaryOp::kSubtract)
                     .IsApplicable(program, node_id_map));

    // binary_expr_id is not a binary expression.
    EXPECT_FALSE(MutationChangeBinaryOperator(a_var_id, ast::BinaryOp::kSubtract)
                     .IsApplicable(program, node_id_map));

    // new_operator is applicable to the argument types.
    EXPECT_FALSE(MutationChangeBinaryOperator(0, ast::BinaryOp::kLogicalAnd)
                     .IsApplicable(program, node_id_map));

    // new_operator does not have the right result type.
    EXPECT_FALSE(MutationChangeBinaryOperator(0, ast::BinaryOp::kLessThan)
                     .IsApplicable(program, node_id_map));
}

TEST(ChangeBinaryOperatorTest, Applicable_Simple) {
    std::string shader = R"(fn main() {
  let a : i32 = (1 + 2);
}
)";
    Source::File file("test.wgsl", shader);
    auto program = reader::wgsl::Parse(&file);
    ASSERT_TRUE(program.IsValid()) << program.Diagnostics().str();

    NodeIdMap node_id_map(program);

    const auto& main_fn_stmts = program.AST().Functions()[0]->body->statements;

    const auto* a_var = main_fn_stmts[0]->As<ast::VariableDeclStatement>()->variable;
    ASSERT_NE(a_var, nullptr);

    const auto* sum_expr = a_var->initializer->As<ast::BinaryExpression>();
    ASSERT_NE(sum_expr, nullptr);

    auto sum_expr_id = node_id_map.GetId(sum_expr);
    ASSERT_NE(sum_expr_id, 0);

    ASSERT_TRUE(MaybeApplyMutation(
        program, MutationChangeBinaryOperator(sum_expr_id, ast::BinaryOp::kSubtract), node_id_map,
        &program, &node_id_map, nullptr));
    ASSERT_TRUE(program.IsValid()) << program.Diagnostics().str();

    writer::wgsl::Options options;
    auto result = writer::wgsl::Generate(&program, options);
    ASSERT_TRUE(result.success) << result.error;

    std::string expected_shader = R"(fn main() {
  let a : i32 = (1 - 2);
}
)";
    ASSERT_EQ(expected_shader, result.wgsl);
}

void CheckMutations(const std::string& lhs_type,
                    const std::string& rhs_type,
                    const std::string& result_type,
                    ast::BinaryOp original_operator,
                    const std::unordered_set<ast::BinaryOp>& allowed_replacement_operators) {
    std::stringstream shader;
    shader << "fn foo(a : " << lhs_type << ", b : " << rhs_type + ") {\n"
           << "  let r : " << result_type << " = (a " + OpToString(original_operator)
           << " b);\n}\n";

    const std::vector<ast::BinaryOp> all_operators = {ast::BinaryOp::kAnd,
                                                      ast::BinaryOp::kOr,
                                                      ast::BinaryOp::kXor,
                                                      ast::BinaryOp::kLogicalAnd,
                                                      ast::BinaryOp::kLogicalOr,
                                                      ast::BinaryOp::kEqual,
                                                      ast::BinaryOp::kNotEqual,
                                                      ast::BinaryOp::kLessThan,
                                                      ast::BinaryOp::kGreaterThan,
                                                      ast::BinaryOp::kLessThanEqual,
                                                      ast::BinaryOp::kGreaterThanEqual,
                                                      ast::BinaryOp::kShiftLeft,
                                                      ast::BinaryOp::kShiftRight,
                                                      ast::BinaryOp::kAdd,
                                                      ast::BinaryOp::kSubtract,
                                                      ast::BinaryOp::kMultiply,
                                                      ast::BinaryOp::kDivide,
                                                      ast::BinaryOp::kModulo};

    for (auto new_operator : all_operators) {
        Source::File file("test.wgsl", shader.str());
        auto program = reader::wgsl::Parse(&file);
        ASSERT_TRUE(program.IsValid()) << program.Diagnostics().str();

        NodeIdMap node_id_map(program);

        const auto& stmts = program.AST().Functions()[0]->body->statements;

        const auto* r_var = stmts[0]->As<ast::VariableDeclStatement>()->variable;
        ASSERT_NE(r_var, nullptr);

        const auto* binary_expr = r_var->initializer->As<ast::BinaryExpression>();
        ASSERT_NE(binary_expr, nullptr);

        auto binary_expr_id = node_id_map.GetId(binary_expr);
        ASSERT_NE(binary_expr_id, 0);

        MutationChangeBinaryOperator mutation(binary_expr_id, new_operator);

        std::stringstream expected_shader;
        expected_shader << "fn foo(a : " << lhs_type << ", b : " << rhs_type << ") {\n"
                        << "  let r : " << result_type << " = (a " << OpToString(new_operator)
                        << " b);\n}\n";

        if (allowed_replacement_operators.count(new_operator) == 0) {
            ASSERT_FALSE(mutation.IsApplicable(program, node_id_map));
            if (new_operator != binary_expr->op) {
                Source::File invalid_file("test.wgsl", expected_shader.str());
                auto invalid_program = reader::wgsl::Parse(&invalid_file);
                ASSERT_FALSE(invalid_program.IsValid()) << program.Diagnostics().str();
            }
        } else {
            ASSERT_TRUE(MaybeApplyMutation(program, mutation, node_id_map, &program, &node_id_map,
                                           nullptr));
            ASSERT_TRUE(program.IsValid()) << program.Diagnostics().str();

            writer::wgsl::Options options;
            auto result = writer::wgsl::Generate(&program, options);
            ASSERT_TRUE(result.success) << result.error;

            ASSERT_EQ(expected_shader.str(), result.wgsl);
        }
    }
}

TEST(ChangeBinaryOperatorTest, AddSubtract) {
    for (auto op : {ast::BinaryOp::kAdd, ast::BinaryOp::kSubtract}) {
        const ast::BinaryOp other_op =
            op == ast::BinaryOp::kAdd ? ast::BinaryOp::kSubtract : ast::BinaryOp::kAdd;
        for (std::string type : {"i32", "vec2<i32>", "vec3<i32>", "vec4<i32>"}) {
            CheckMutations(
                type, type, type, op,
                {other_op, ast::BinaryOp::kMultiply, ast::BinaryOp::kDivide, ast::BinaryOp::kModulo,
                 ast::BinaryOp::kAnd, ast::BinaryOp::kOr, ast::BinaryOp::kXor});
        }
        for (std::string type : {"u32", "vec2<u32>", "vec3<u32>", "vec4<u32>"}) {
            CheckMutations(
                type, type, type, op,
                {other_op, ast::BinaryOp::kMultiply, ast::BinaryOp::kDivide, ast::BinaryOp::kModulo,
                 ast::BinaryOp::kAnd, ast::BinaryOp::kOr, ast::BinaryOp::kXor,
                 ast::BinaryOp::kShiftLeft, ast::BinaryOp::kShiftRight});
        }
        for (std::string type : {"f32", "vec2<f32>", "vec3<f32>", "vec4<f32>"}) {
            CheckMutations(type, type, type, op,
                           {other_op, ast::BinaryOp::kMultiply, ast::BinaryOp::kDivide,
                            ast::BinaryOp::kModulo});
        }
        for (std::string vector_type : {"vec2<i32>", "vec3<i32>", "vec4<i32>"}) {
            std::string scalar_type = "i32";
            CheckMutations(vector_type, scalar_type, vector_type, op,
                           {other_op, ast::BinaryOp::kMultiply, ast::BinaryOp::kDivide,
                            ast::BinaryOp::kModulo});
            CheckMutations(scalar_type, vector_type, vector_type, op,
                           {other_op, ast::BinaryOp::kMultiply, ast::BinaryOp::kDivide,
                            ast::BinaryOp::kModulo});
        }
        for (std::string vector_type : {"vec2<u32>", "vec3<u32>", "vec4<u32>"}) {
            std::string scalar_type = "u32";
            CheckMutations(vector_type, scalar_type, vector_type, op,
                           {other_op, ast::BinaryOp::kMultiply, ast::BinaryOp::kDivide,
                            ast::BinaryOp::kModulo});
            CheckMutations(scalar_type, vector_type, vector_type, op,
                           {other_op, ast::BinaryOp::kMultiply, ast::BinaryOp::kDivide,
                            ast::BinaryOp::kModulo});
        }
        for (std::string vector_type : {"vec2<f32>", "vec3<f32>", "vec4<f32>"}) {
            std::string scalar_type = "f32";
            CheckMutations(vector_type, scalar_type, vector_type, op,
                           {other_op, ast::BinaryOp::kMultiply, ast::BinaryOp::kDivide,
                            ast::BinaryOp::kModulo});
            CheckMutations(scalar_type, vector_type, vector_type, op,
                           {other_op, ast::BinaryOp::kMultiply, ast::BinaryOp::kDivide,
                            ast::BinaryOp::kModulo});
        }
        for (std::string square_matrix_type : {"mat2x2<f32>", "mat3x3<f32>", "mat4x4<f32>"}) {
            CheckMutations(square_matrix_type, square_matrix_type, square_matrix_type, op,
                           {other_op, ast::BinaryOp::kMultiply});
        }
        for (std::string non_square_matrix_type : {"mat2x3<f32>", "mat2x4<f32>", "mat3x2<f32>",
                                                   "mat3x4<f32>", "mat4x2<f32>", "mat4x3<f32>"}) {
            CheckMutations(non_square_matrix_type, non_square_matrix_type, non_square_matrix_type,
                           op, {other_op});
        }
    }
}

TEST(ChangeBinaryOperatorTest, Mul) {
    for (std::string type : {"i32", "vec2<i32>", "vec3<i32>", "vec4<i32>"}) {
        CheckMutations(
            type, type, type, ast::BinaryOp::kMultiply,
            {ast::BinaryOp::kAdd, ast::BinaryOp::kSubtract, ast::BinaryOp::kDivide,
             ast::BinaryOp::kModulo, ast::BinaryOp::kAnd, ast::BinaryOp::kOr, ast::BinaryOp::kXor});
    }
    for (std::string type : {"u32", "vec2<u32>", "vec3<u32>", "vec4<u32>"}) {
        CheckMutations(
            type, type, type, ast::BinaryOp::kMultiply,
            {ast::BinaryOp::kAdd, ast::BinaryOp::kSubtract, ast::BinaryOp::kDivide,
             ast::BinaryOp::kModulo, ast::BinaryOp::kAnd, ast::BinaryOp::kOr, ast::BinaryOp::kXor,
             ast::BinaryOp::kShiftLeft, ast::BinaryOp::kShiftRight});
    }
    for (std::string type : {"f32", "vec2<f32>", "vec3<f32>", "vec4<f32>"}) {
        CheckMutations(type, type, type, ast::BinaryOp::kMultiply,
                       {ast::BinaryOp::kAdd, ast::BinaryOp::kSubtract, ast::BinaryOp::kDivide,
                        ast::BinaryOp::kModulo});
    }
    for (std::string vector_type : {"vec2<i32>", "vec3<i32>", "vec4<i32>"}) {
        std::string scalar_type = "i32";
        CheckMutations(vector_type, scalar_type, vector_type, ast::BinaryOp::kMultiply,
                       {ast::BinaryOp::kAdd, ast::BinaryOp::kSubtract, ast::BinaryOp::kDivide,
                        ast::BinaryOp::kModulo});
        CheckMutations(scalar_type, vector_type, vector_type, ast::BinaryOp::kMultiply,
                       {ast::BinaryOp::kAdd, ast::BinaryOp::kSubtract, ast::BinaryOp::kDivide,
                        ast::BinaryOp::kModulo});
    }
    for (std::string vector_type : {"vec2<u32>", "vec3<u32>", "vec4<u32>"}) {
        std::string scalar_type = "u32";
        CheckMutations(vector_type, scalar_type, vector_type, ast::BinaryOp::kMultiply,
                       {ast::BinaryOp::kAdd, ast::BinaryOp::kSubtract, ast::BinaryOp::kDivide,
                        ast::BinaryOp::kModulo});
        CheckMutations(scalar_type, vector_type, vector_type, ast::BinaryOp::kMultiply,
                       {ast::BinaryOp::kAdd, ast::BinaryOp::kSubtract, ast::BinaryOp::kDivide,
                        ast::BinaryOp::kModulo});
    }
    for (std::string vector_type : {"vec2<f32>", "vec3<f32>", "vec4<f32>"}) {
        std::string scalar_type = "f32";
        CheckMutations(vector_type, scalar_type, vector_type, ast::BinaryOp::kMultiply,
                       {ast::BinaryOp::kAdd, ast::BinaryOp::kSubtract, ast::BinaryOp::kDivide,
                        ast::BinaryOp::kModulo});
        CheckMutations(scalar_type, vector_type, vector_type, ast::BinaryOp::kMultiply,
                       {ast::BinaryOp::kAdd, ast::BinaryOp::kSubtract, ast::BinaryOp::kDivide,
                        ast::BinaryOp::kModulo});
    }
    for (std::string square_matrix_type : {"mat2x2<f32>", "mat3x3<f32>", "mat4x4<f32>"}) {
        CheckMutations(square_matrix_type, square_matrix_type, square_matrix_type,
                       ast::BinaryOp::kMultiply, {ast::BinaryOp::kAdd, ast::BinaryOp::kSubtract});
    }

    CheckMutations("vec2<f32>", "mat2x2<f32>", "vec2<f32>", ast::BinaryOp::kMultiply, {});
    CheckMutations("vec2<f32>", "mat3x2<f32>", "vec3<f32>", ast::BinaryOp::kMultiply, {});
    CheckMutations("vec2<f32>", "mat4x2<f32>", "vec4<f32>", ast::BinaryOp::kMultiply, {});

    CheckMutations("mat2x2<f32>", "vec2<f32>", "vec2<f32>", ast::BinaryOp::kMultiply, {});
    CheckMutations("mat2x2<f32>", "mat3x2<f32>", "mat3x2<f32>", ast::BinaryOp::kMultiply, {});
    CheckMutations("mat2x2<f32>", "mat4x2<f32>", "mat4x2<f32>", ast::BinaryOp::kMultiply, {});

    CheckMutations("mat2x3<f32>", "vec2<f32>", "vec3<f32>", ast::BinaryOp::kMultiply, {});
    CheckMutations("mat2x3<f32>", "mat2x2<f32>", "mat2x3<f32>", ast::BinaryOp::kMultiply, {});
    CheckMutations("mat2x3<f32>", "mat3x2<f32>", "mat3x3<f32>", ast::BinaryOp::kMultiply, {});
    CheckMutations("mat2x3<f32>", "mat4x2<f32>", "mat4x3<f32>", ast::BinaryOp::kMultiply, {});

    CheckMutations("mat2x4<f32>", "vec2<f32>", "vec4<f32>", ast::BinaryOp::kMultiply, {});
    CheckMutations("mat2x4<f32>", "mat2x2<f32>", "mat2x4<f32>", ast::BinaryOp::kMultiply, {});
    CheckMutations("mat2x4<f32>", "mat3x2<f32>", "mat3x4<f32>", ast::BinaryOp::kMultiply, {});
    CheckMutations("mat2x4<f32>", "mat4x2<f32>", "mat4x4<f32>", ast::BinaryOp::kMultiply, {});

    CheckMutations("vec3<f32>", "mat2x3<f32>", "vec2<f32>", ast::BinaryOp::kMultiply, {});
    CheckMutations("vec3<f32>", "mat3x3<f32>", "vec3<f32>", ast::BinaryOp::kMultiply, {});
    CheckMutations("vec3<f32>", "mat4x3<f32>", "vec4<f32>", ast::BinaryOp::kMultiply, {});

    CheckMutations("mat3x2<f32>", "vec3<f32>", "vec2<f32>", ast::BinaryOp::kMultiply, {});
    CheckMutations("mat3x2<f32>", "mat2x3<f32>", "mat2x2<f32>", ast::BinaryOp::kMultiply, {});
    CheckMutations("mat3x2<f32>", "mat3x3<f32>", "mat3x2<f32>", ast::BinaryOp::kMultiply, {});
    CheckMutations("mat3x2<f32>", "mat4x3<f32>", "mat4x2<f32>", ast::BinaryOp::kMultiply, {});

    CheckMutations("mat3x3<f32>", "vec3<f32>", "vec3<f32>", ast::BinaryOp::kMultiply, {});
    CheckMutations("mat3x3<f32>", "mat2x3<f32>", "mat2x3<f32>", ast::BinaryOp::kMultiply, {});
    CheckMutations("mat3x3<f32>", "mat4x3<f32>", "mat4x3<f32>", ast::BinaryOp::kMultiply, {});

    CheckMutations("mat3x4<f32>", "vec3<f32>", "vec4<f32>", ast::BinaryOp::kMultiply, {});
    CheckMutations("mat3x4<f32>", "mat2x3<f32>", "mat2x4<f32>", ast::BinaryOp::kMultiply, {});
    CheckMutations("mat3x4<f32>", "mat3x3<f32>", "mat3x4<f32>", ast::BinaryOp::kMultiply, {});
    CheckMutations("mat3x4<f32>", "mat4x3<f32>", "mat4x4<f32>", ast::BinaryOp::kMultiply, {});

    CheckMutations("vec4<f32>", "mat2x4<f32>", "vec2<f32>", ast::BinaryOp::kMultiply, {});
    CheckMutations("vec4<f32>", "mat3x4<f32>", "vec3<f32>", ast::BinaryOp::kMultiply, {});
    CheckMutations("vec4<f32>", "mat4x4<f32>", "vec4<f32>", ast::BinaryOp::kMultiply, {});

    CheckMutations("mat4x2<f32>", "vec4<f32>", "vec2<f32>", ast::BinaryOp::kMultiply, {});
    CheckMutations("mat4x2<f32>", "mat2x4<f32>", "mat2x2<f32>", ast::BinaryOp::kMultiply, {});
    CheckMutations("mat4x2<f32>", "mat3x4<f32>", "mat3x2<f32>", ast::BinaryOp::kMultiply, {});
    CheckMutations("mat4x2<f32>", "mat4x4<f32>", "mat4x2<f32>", ast::BinaryOp::kMultiply, {});

    CheckMutations("mat4x3<f32>", "vec4<f32>", "vec3<f32>", ast::BinaryOp::kMultiply, {});
    CheckMutations("mat4x3<f32>", "mat2x4<f32>", "mat2x3<f32>", ast::BinaryOp::kMultiply, {});
    CheckMutations("mat4x3<f32>", "mat3x4<f32>", "mat3x3<f32>", ast::BinaryOp::kMultiply, {});
    CheckMutations("mat4x3<f32>", "mat4x4<f32>", "mat4x3<f32>", ast::BinaryOp::kMultiply, {});

    CheckMutations("mat4x4<f32>", "vec4<f32>", "vec4<f32>", ast::BinaryOp::kMultiply, {});
    CheckMutations("mat4x4<f32>", "mat2x4<f32>", "mat2x4<f32>", ast::BinaryOp::kMultiply, {});
    CheckMutations("mat4x4<f32>", "mat3x4<f32>", "mat3x4<f32>", ast::BinaryOp::kMultiply, {});
}

TEST(ChangeBinaryOperatorTest, DivideAndModulo) {
    for (std::string type : {"i32", "vec2<i32>", "vec3<i32>", "vec4<i32>"}) {
        CheckMutations(
            type, type, type, ast::BinaryOp::kDivide,
            {ast::BinaryOp::kAdd, ast::BinaryOp::kSubtract, ast::BinaryOp::kMultiply,
             ast::BinaryOp::kModulo, ast::BinaryOp::kAnd, ast::BinaryOp::kOr, ast::BinaryOp::kXor});
    }
    for (std::string type : {"u32", "vec2<u32>", "vec3<u32>", "vec4<u32>"}) {
        CheckMutations(
            type, type, type, ast::BinaryOp::kDivide,
            {ast::BinaryOp::kAdd, ast::BinaryOp::kSubtract, ast::BinaryOp::kMultiply,
             ast::BinaryOp::kModulo, ast::BinaryOp::kAnd, ast::BinaryOp::kOr, ast::BinaryOp::kXor,
             ast::BinaryOp::kShiftLeft, ast::BinaryOp::kShiftRight});
    }
    for (std::string type : {"f32", "vec2<f32>", "vec3<f32>", "vec4<f32>"}) {
        CheckMutations(type, type, type, ast::BinaryOp::kDivide,
                       {ast::BinaryOp::kAdd, ast::BinaryOp::kSubtract, ast::BinaryOp::kMultiply,
                        ast::BinaryOp::kModulo});
    }
    for (std::string vector_type : {"vec2<i32>", "vec3<i32>", "vec4<i32>"}) {
        std::string scalar_type = "i32";
        CheckMutations(vector_type, scalar_type, vector_type, ast::BinaryOp::kDivide,
                       {ast::BinaryOp::kAdd, ast::BinaryOp::kSubtract, ast::BinaryOp::kMultiply,
                        ast::BinaryOp::kModulo});
        CheckMutations(scalar_type, vector_type, vector_type, ast::BinaryOp::kDivide,
                       {ast::BinaryOp::kAdd, ast::BinaryOp::kSubtract, ast::BinaryOp::kMultiply,
                        ast::BinaryOp::kModulo});
    }
    for (std::string vector_type : {"vec2<u32>", "vec3<u32>", "vec4<u32>"}) {
        std::string scalar_type = "u32";
        CheckMutations(vector_type, scalar_type, vector_type, ast::BinaryOp::kDivide,
                       {ast::BinaryOp::kAdd, ast::BinaryOp::kSubtract, ast::BinaryOp::kMultiply,
                        ast::BinaryOp::kModulo});
        CheckMutations(scalar_type, vector_type, vector_type, ast::BinaryOp::kDivide,
                       {ast::BinaryOp::kAdd, ast::BinaryOp::kSubtract, ast::BinaryOp::kMultiply,
                        ast::BinaryOp::kModulo});
    }
    for (std::string vector_type : {"vec2<f32>", "vec3<f32>", "vec4<f32>"}) {
        std::string scalar_type = "f32";
        CheckMutations(vector_type, scalar_type, vector_type, ast::BinaryOp::kDivide,
                       {ast::BinaryOp::kAdd, ast::BinaryOp::kSubtract, ast::BinaryOp::kMultiply,
                        ast::BinaryOp::kModulo});
        CheckMutations(scalar_type, vector_type, vector_type, ast::BinaryOp::kDivide,
                       {ast::BinaryOp::kAdd, ast::BinaryOp::kSubtract, ast::BinaryOp::kMultiply,
                        ast::BinaryOp::kModulo});
    }
    for (std::string type : {"i32", "vec2<i32>", "vec3<i32>", "vec4<i32>"}) {
        CheckMutations(
            type, type, type, ast::BinaryOp::kModulo,
            {ast::BinaryOp::kAdd, ast::BinaryOp::kSubtract, ast::BinaryOp::kMultiply,
             ast::BinaryOp::kDivide, ast::BinaryOp::kAnd, ast::BinaryOp::kOr, ast::BinaryOp::kXor});
    }
    for (std::string type : {"u32", "vec2<u32>", "vec3<u32>", "vec4<u32>"}) {
        CheckMutations(
            type, type, type, ast::BinaryOp::kModulo,
            {ast::BinaryOp::kAdd, ast::BinaryOp::kSubtract, ast::BinaryOp::kMultiply,
             ast::BinaryOp::kDivide, ast::BinaryOp::kAnd, ast::BinaryOp::kOr, ast::BinaryOp::kXor,
             ast::BinaryOp::kShiftLeft, ast::BinaryOp::kShiftRight});
    }
    for (std::string type : {"f32", "vec2<f32>", "vec3<f32>", "vec4<f32>"}) {
        CheckMutations(type, type, type, ast::BinaryOp::kModulo,
                       {ast::BinaryOp::kAdd, ast::BinaryOp::kSubtract, ast::BinaryOp::kMultiply,
                        ast::BinaryOp::kDivide});
    }
    for (std::string vector_type : {"vec2<i32>", "vec3<i32>", "vec4<i32>"}) {
        std::string scalar_type = "i32";
        CheckMutations(vector_type, scalar_type, vector_type, ast::BinaryOp::kModulo,
                       {ast::BinaryOp::kAdd, ast::BinaryOp::kSubtract, ast::BinaryOp::kMultiply,
                        ast::BinaryOp::kDivide});
        CheckMutations(scalar_type, vector_type, vector_type, ast::BinaryOp::kModulo,
                       {ast::BinaryOp::kAdd, ast::BinaryOp::kSubtract, ast::BinaryOp::kMultiply,
                        ast::BinaryOp::kDivide});
    }
    for (std::string vector_type : {"vec2<u32>", "vec3<u32>", "vec4<u32>"}) {
        std::string scalar_type = "u32";
        CheckMutations(vector_type, scalar_type, vector_type, ast::BinaryOp::kModulo,
                       {ast::BinaryOp::kAdd, ast::BinaryOp::kSubtract, ast::BinaryOp::kMultiply,
                        ast::BinaryOp::kDivide});
        CheckMutations(scalar_type, vector_type, vector_type, ast::BinaryOp::kModulo,
                       {ast::BinaryOp::kAdd, ast::BinaryOp::kSubtract, ast::BinaryOp::kMultiply,
                        ast::BinaryOp::kDivide});
    }
}

TEST(ChangeBinaryOperatorTest, AndOrXor) {
    for (auto op : {ast::BinaryOp::kAnd, ast::BinaryOp::kOr, ast::BinaryOp::kXor}) {
        std::unordered_set<ast::BinaryOp> allowed_replacement_operators_signed{
            ast::BinaryOp::kAdd,    ast::BinaryOp::kSubtract, ast::BinaryOp::kMultiply,
            ast::BinaryOp::kDivide, ast::BinaryOp::kModulo,   ast::BinaryOp::kAnd,
            ast::BinaryOp::kOr,     ast::BinaryOp::kXor};
        allowed_replacement_operators_signed.erase(op);
        for (std::string type : {"i32", "vec2<i32>", "vec3<i32>", "vec4<i32>"}) {
            CheckMutations(type, type, type, op, allowed_replacement_operators_signed);
        }
        std::unordered_set<ast::BinaryOp> allowed_replacement_operators_unsigned{
            ast::BinaryOp::kAdd,        ast::BinaryOp::kSubtract, ast::BinaryOp::kMultiply,
            ast::BinaryOp::kDivide,     ast::BinaryOp::kModulo,   ast::BinaryOp::kShiftLeft,
            ast::BinaryOp::kShiftRight, ast::BinaryOp::kAnd,      ast::BinaryOp::kOr,
            ast::BinaryOp::kXor};
        allowed_replacement_operators_unsigned.erase(op);
        for (std::string type : {"u32", "vec2<u32>", "vec3<u32>", "vec4<u32>"}) {
            CheckMutations(type, type, type, op, allowed_replacement_operators_unsigned);
        }
        if (op != ast::BinaryOp::kXor) {
            for (std::string type : {"bool", "vec2<bool>", "vec3<bool>", "vec4<bool>"}) {
                std::unordered_set<ast::BinaryOp> allowed_replacement_operators_bool{
                    ast::BinaryOp::kAnd, ast::BinaryOp::kOr, ast::BinaryOp::kEqual,
                    ast::BinaryOp::kNotEqual};
                allowed_replacement_operators_bool.erase(op);
                if (type == "bool") {
                    allowed_replacement_operators_bool.insert(ast::BinaryOp::kLogicalAnd);
                    allowed_replacement_operators_bool.insert(ast::BinaryOp::kLogicalOr);
                }
                CheckMutations(type, type, type, op, allowed_replacement_operators_bool);
            }
        }
    }
}

TEST(ChangeBinaryOperatorTest, EqualNotEqual) {
    for (auto op : {ast::BinaryOp::kEqual, ast::BinaryOp::kNotEqual}) {
        for (std::string element_type : {"i32", "u32", "f32"}) {
            for (size_t element_count = 1; element_count <= 4; element_count++) {
                std::stringstream argument_type;
                std::stringstream result_type;
                if (element_count == 1) {
                    argument_type << element_type;
                    result_type << "bool";
                } else {
                    argument_type << "vec" << element_count << "<" << element_type << ">";
                    result_type << "vec" << element_count << "<bool>";
                }
                std::unordered_set<ast::BinaryOp> allowed_replacement_operators{
                    ast::BinaryOp::kLessThan,    ast::BinaryOp::kLessThanEqual,
                    ast::BinaryOp::kGreaterThan, ast::BinaryOp::kGreaterThanEqual,
                    ast::BinaryOp::kEqual,       ast::BinaryOp::kNotEqual};
                allowed_replacement_operators.erase(op);
                CheckMutations(argument_type.str(), argument_type.str(), result_type.str(), op,
                               allowed_replacement_operators);
            }
        }
        {
            std::unordered_set<ast::BinaryOp> allowed_replacement_operators{
                ast::BinaryOp::kLogicalAnd, ast::BinaryOp::kLogicalOr, ast::BinaryOp::kAnd,
                ast::BinaryOp::kOr,         ast::BinaryOp::kEqual,     ast::BinaryOp::kNotEqual};
            allowed_replacement_operators.erase(op);
            CheckMutations("bool", "bool", "bool", op, allowed_replacement_operators);
        }
        for (size_t element_count = 2; element_count <= 4; element_count++) {
            std::stringstream argument_and_result_type;
            argument_and_result_type << "vec" << element_count << "<bool>";
            std::unordered_set<ast::BinaryOp> allowed_replacement_operators{
                ast::BinaryOp::kAnd, ast::BinaryOp::kOr, ast::BinaryOp::kEqual,
                ast::BinaryOp::kNotEqual};
            allowed_replacement_operators.erase(op);
            CheckMutations(argument_and_result_type.str(), argument_and_result_type.str(),
                           argument_and_result_type.str(), op, allowed_replacement_operators);
        }
    }
}

TEST(ChangeBinaryOperatorTest, LessThanLessThanEqualGreaterThanGreaterThanEqual) {
    for (auto op : {ast::BinaryOp::kLessThan, ast::BinaryOp::kLessThanEqual,
                    ast::BinaryOp::kGreaterThan, ast::BinaryOp::kGreaterThanEqual}) {
        for (std::string element_type : {"i32", "u32", "f32"}) {
            for (size_t element_count = 1; element_count <= 4; element_count++) {
                std::stringstream argument_type;
                std::stringstream result_type;
                if (element_count == 1) {
                    argument_type << element_type;
                    result_type << "bool";
                } else {
                    argument_type << "vec" << element_count << "<" << element_type << ">";
                    result_type << "vec" << element_count << "<bool>";
                }
                std::unordered_set<ast::BinaryOp> allowed_replacement_operators{
                    ast::BinaryOp::kLessThan,    ast::BinaryOp::kLessThanEqual,
                    ast::BinaryOp::kGreaterThan, ast::BinaryOp::kGreaterThanEqual,
                    ast::BinaryOp::kEqual,       ast::BinaryOp::kNotEqual};
                allowed_replacement_operators.erase(op);
                CheckMutations(argument_type.str(), argument_type.str(), result_type.str(), op,
                               allowed_replacement_operators);
            }
        }
    }
}

TEST(ChangeBinaryOperatorTest, LogicalAndLogicalOr) {
    for (auto op : {ast::BinaryOp::kLogicalAnd, ast::BinaryOp::kLogicalOr}) {
        std::unordered_set<ast::BinaryOp> allowed_replacement_operators{
            ast::BinaryOp::kLogicalAnd, ast::BinaryOp::kLogicalOr, ast::BinaryOp::kAnd,
            ast::BinaryOp::kOr,         ast::BinaryOp::kEqual,     ast::BinaryOp::kNotEqual};
        allowed_replacement_operators.erase(op);
        CheckMutations("bool", "bool", "bool", op, allowed_replacement_operators);
    }
}

TEST(ChangeBinaryOperatorTest, ShiftLeftShiftRight) {
    for (auto op : {ast::BinaryOp::kShiftLeft, ast::BinaryOp::kShiftRight}) {
        for (std::string lhs_element_type : {"i32", "u32"}) {
            for (size_t element_count = 1; element_count <= 4; element_count++) {
                std::stringstream lhs_and_result_type;
                std::stringstream rhs_type;
                if (element_count == 1) {
                    lhs_and_result_type << lhs_element_type;
                    rhs_type << "u32";
                } else {
                    lhs_and_result_type << "vec" << element_count << "<" << lhs_element_type << ">";
                    rhs_type << "vec" << element_count << "<u32>";
                }
                std::unordered_set<ast::BinaryOp> allowed_replacement_operators{
                    ast::BinaryOp::kShiftLeft, ast::BinaryOp::kShiftRight};
                allowed_replacement_operators.erase(op);
                if (lhs_element_type == "u32") {
                    allowed_replacement_operators.insert(ast::BinaryOp::kAdd);
                    allowed_replacement_operators.insert(ast::BinaryOp::kSubtract);
                    allowed_replacement_operators.insert(ast::BinaryOp::kMultiply);
                    allowed_replacement_operators.insert(ast::BinaryOp::kDivide);
                    allowed_replacement_operators.insert(ast::BinaryOp::kModulo);
                    allowed_replacement_operators.insert(ast::BinaryOp::kAnd);
                    allowed_replacement_operators.insert(ast::BinaryOp::kOr);
                    allowed_replacement_operators.insert(ast::BinaryOp::kXor);
                }
                CheckMutations(lhs_and_result_type.str(), rhs_type.str(), lhs_and_result_type.str(),
                               op, allowed_replacement_operators);
            }
        }
    }
}

}  // namespace
}  // namespace tint::fuzzers::ast_fuzzer
