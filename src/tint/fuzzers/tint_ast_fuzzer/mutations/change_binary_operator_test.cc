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

#include "src/tint/fuzzers/tint_ast_fuzzer/mutations/change_binary_operator.h"

#include <string>
#include <unordered_set>
#include <vector>

#include "gtest/gtest.h"

#include "src/tint/fuzzers/tint_ast_fuzzer/mutator.h"
#include "src/tint/fuzzers/tint_ast_fuzzer/node_id_map.h"
#include "src/tint/lang/wgsl/program/program_builder.h"
#include "src/tint/lang/wgsl/reader/reader.h"
#include "src/tint/lang/wgsl/writer/writer.h"

namespace tint::fuzzers::ast_fuzzer {
namespace {

std::string OpToString(core::BinaryOp op) {
    switch (op) {
        case core::BinaryOp::kAnd:
            return "&";
        case core::BinaryOp::kOr:
            return "|";
        case core::BinaryOp::kXor:
            return "^";
        case core::BinaryOp::kLogicalAnd:
            return "&&";
        case core::BinaryOp::kLogicalOr:
            return "||";
        case core::BinaryOp::kEqual:
            return "==";
        case core::BinaryOp::kNotEqual:
            return "!=";
        case core::BinaryOp::kLessThan:
            return "<";
        case core::BinaryOp::kGreaterThan:
            return ">";
        case core::BinaryOp::kLessThanEqual:
            return "<=";
        case core::BinaryOp::kGreaterThanEqual:
            return ">=";
        case core::BinaryOp::kShiftLeft:
            return "<<";
        case core::BinaryOp::kShiftRight:
            return ">>";
        case core::BinaryOp::kAdd:
            return "+";
        case core::BinaryOp::kSubtract:
            return "-";
        case core::BinaryOp::kMultiply:
            return "*";
        case core::BinaryOp::kDivide:
            return "/";
        case core::BinaryOp::kModulo:
            return "%";
    }
    TINT_UNREACHABLE() << "unhandled BinaryOp: " << op;
    return "<error>";
}

TEST(ChangeBinaryOperatorTest, NotApplicable_Simple) {
    std::string content = R"(
    fn main() {
      let a : i32 = 1 + 2;
    }
  )";
    Source::File file("test.wgsl", content);
    auto program = wgsl::reader::Parse(&file);
    ASSERT_TRUE(program.IsValid()) << program.Diagnostics();

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
    EXPECT_FALSE(MutationChangeBinaryOperator(0, core::BinaryOp::kSubtract)
                     .IsApplicable(program, node_id_map));

    // binary_expr_id is not a binary expression.
    EXPECT_FALSE(MutationChangeBinaryOperator(a_var_id, core::BinaryOp::kSubtract)
                     .IsApplicable(program, node_id_map));

    // new_operator is applicable to the argument types.
    EXPECT_FALSE(MutationChangeBinaryOperator(0, core::BinaryOp::kLogicalAnd)
                     .IsApplicable(program, node_id_map));

    // new_operator does not have the right result type.
    EXPECT_FALSE(MutationChangeBinaryOperator(0, core::BinaryOp::kLessThan)
                     .IsApplicable(program, node_id_map));
}

TEST(ChangeBinaryOperatorTest, Applicable_Simple) {
    std::string shader = R"(fn main() {
  let a : i32 = (1 + 2);
}
)";
    Source::File file("test.wgsl", shader);
    auto program = wgsl::reader::Parse(&file);
    ASSERT_TRUE(program.IsValid()) << program.Diagnostics();

    NodeIdMap node_id_map(program);

    const auto& main_fn_stmts = program.AST().Functions()[0]->body->statements;

    const auto* a_var = main_fn_stmts[0]->As<ast::VariableDeclStatement>()->variable;
    ASSERT_NE(a_var, nullptr);

    const auto* sum_expr = a_var->initializer->As<ast::BinaryExpression>();
    ASSERT_NE(sum_expr, nullptr);

    auto sum_expr_id = node_id_map.GetId(sum_expr);
    ASSERT_NE(sum_expr_id, 0);

    ASSERT_TRUE(MaybeApplyMutation(
        program, MutationChangeBinaryOperator(sum_expr_id, core::BinaryOp::kSubtract), node_id_map,
        program, &node_id_map, nullptr));
    ASSERT_TRUE(program.IsValid()) << program.Diagnostics();

    wgsl::writer::Options options;
    auto result = wgsl::writer::Generate(program, options);
    ASSERT_TRUE(result) << result.Failure();

    std::string expected_shader = R"(fn main() {
  let a : i32 = (1 - 2);
}
)";
    ASSERT_EQ(expected_shader, result->wgsl);
}

void CheckMutations(const std::string& lhs_type,
                    const std::string& rhs_type,
                    const std::string& result_type,
                    core::BinaryOp original_operator,
                    const std::unordered_set<core::BinaryOp>& allowed_replacement_operators) {
    std::stringstream shader;
    shader << "fn foo(a : " << lhs_type << ", b : " << rhs_type + ") {\n"
           << "  let r : " << result_type << " = (a " + OpToString(original_operator)
           << " b);\n}\n";

    const std::vector<core::BinaryOp> all_operators = {core::BinaryOp::kAnd,
                                                       core::BinaryOp::kOr,
                                                       core::BinaryOp::kXor,
                                                       core::BinaryOp::kLogicalAnd,
                                                       core::BinaryOp::kLogicalOr,
                                                       core::BinaryOp::kEqual,
                                                       core::BinaryOp::kNotEqual,
                                                       core::BinaryOp::kLessThan,
                                                       core::BinaryOp::kGreaterThan,
                                                       core::BinaryOp::kLessThanEqual,
                                                       core::BinaryOp::kGreaterThanEqual,
                                                       core::BinaryOp::kShiftLeft,
                                                       core::BinaryOp::kShiftRight,
                                                       core::BinaryOp::kAdd,
                                                       core::BinaryOp::kSubtract,
                                                       core::BinaryOp::kMultiply,
                                                       core::BinaryOp::kDivide,
                                                       core::BinaryOp::kModulo};

    for (auto new_operator : all_operators) {
        Source::File file("test.wgsl", shader.str());
        auto program = wgsl::reader::Parse(&file);
        ASSERT_TRUE(program.IsValid()) << program.Diagnostics();

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
                auto invalid_program = wgsl::reader::Parse(&invalid_file);
                ASSERT_FALSE(invalid_program.IsValid()) << program.Diagnostics();
            }
        } else {
            ASSERT_TRUE(
                MaybeApplyMutation(program, mutation, node_id_map, program, &node_id_map, nullptr));
            ASSERT_TRUE(program.IsValid()) << program.Diagnostics();

            wgsl::writer::Options options;
            auto result = wgsl::writer::Generate(program, options);
            ASSERT_TRUE(result) << result.Failure();

            ASSERT_EQ(expected_shader.str(), result->wgsl);
        }
    }
}

TEST(ChangeBinaryOperatorTest, AddSubtract) {
    for (auto op : {core::BinaryOp::kAdd, core::BinaryOp::kSubtract}) {
        const core::BinaryOp other_op =
            op == core::BinaryOp::kAdd ? core::BinaryOp::kSubtract : core::BinaryOp::kAdd;
        for (std::string type : {"i32", "vec2<i32>", "vec3<i32>", "vec4<i32>"}) {
            CheckMutations(type, type, type, op,
                           {other_op, core::BinaryOp::kMultiply, core::BinaryOp::kDivide,
                            core::BinaryOp::kModulo, core::BinaryOp::kAnd, core::BinaryOp::kOr,
                            core::BinaryOp::kXor});
        }
        for (std::string type : {"u32", "vec2<u32>", "vec3<u32>", "vec4<u32>"}) {
            CheckMutations(
                type, type, type, op,
                {other_op, core::BinaryOp::kMultiply, core::BinaryOp::kDivide,
                 core::BinaryOp::kModulo, core::BinaryOp::kAnd, core::BinaryOp::kOr,
                 core::BinaryOp::kXor, core::BinaryOp::kShiftLeft, core::BinaryOp::kShiftRight});
        }
        for (std::string type : {"f32", "vec2<f32>", "vec3<f32>", "vec4<f32>"}) {
            CheckMutations(type, type, type, op,
                           {other_op, core::BinaryOp::kMultiply, core::BinaryOp::kDivide,
                            core::BinaryOp::kModulo});
        }
        for (std::string vector_type : {"vec2<i32>", "vec3<i32>", "vec4<i32>"}) {
            std::string scalar_type = "i32";
            CheckMutations(vector_type, scalar_type, vector_type, op,
                           {other_op, core::BinaryOp::kMultiply, core::BinaryOp::kDivide,
                            core::BinaryOp::kModulo});
            CheckMutations(scalar_type, vector_type, vector_type, op,
                           {other_op, core::BinaryOp::kMultiply, core::BinaryOp::kDivide,
                            core::BinaryOp::kModulo});
        }
        for (std::string vector_type : {"vec2<u32>", "vec3<u32>", "vec4<u32>"}) {
            std::string scalar_type = "u32";
            CheckMutations(vector_type, scalar_type, vector_type, op,
                           {other_op, core::BinaryOp::kMultiply, core::BinaryOp::kDivide,
                            core::BinaryOp::kModulo});
            CheckMutations(scalar_type, vector_type, vector_type, op,
                           {other_op, core::BinaryOp::kMultiply, core::BinaryOp::kDivide,
                            core::BinaryOp::kModulo});
        }
        for (std::string vector_type : {"vec2<f32>", "vec3<f32>", "vec4<f32>"}) {
            std::string scalar_type = "f32";
            CheckMutations(vector_type, scalar_type, vector_type, op,
                           {other_op, core::BinaryOp::kMultiply, core::BinaryOp::kDivide,
                            core::BinaryOp::kModulo});
            CheckMutations(scalar_type, vector_type, vector_type, op,
                           {other_op, core::BinaryOp::kMultiply, core::BinaryOp::kDivide,
                            core::BinaryOp::kModulo});
        }
        for (std::string square_matrix_type : {"mat2x2<f32>", "mat3x3<f32>", "mat4x4<f32>"}) {
            CheckMutations(square_matrix_type, square_matrix_type, square_matrix_type, op,
                           {other_op, core::BinaryOp::kMultiply});
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
        CheckMutations(type, type, type, core::BinaryOp::kMultiply,
                       {core::BinaryOp::kAdd, core::BinaryOp::kSubtract, core::BinaryOp::kDivide,
                        core::BinaryOp::kModulo, core::BinaryOp::kAnd, core::BinaryOp::kOr,
                        core::BinaryOp::kXor});
    }
    for (std::string type : {"u32", "vec2<u32>", "vec3<u32>", "vec4<u32>"}) {
        CheckMutations(
            type, type, type, core::BinaryOp::kMultiply,
            {core::BinaryOp::kAdd, core::BinaryOp::kSubtract, core::BinaryOp::kDivide,
             core::BinaryOp::kModulo, core::BinaryOp::kAnd, core::BinaryOp::kOr,
             core::BinaryOp::kXor, core::BinaryOp::kShiftLeft, core::BinaryOp::kShiftRight});
    }
    for (std::string type : {"f32", "vec2<f32>", "vec3<f32>", "vec4<f32>"}) {
        CheckMutations(type, type, type, core::BinaryOp::kMultiply,
                       {core::BinaryOp::kAdd, core::BinaryOp::kSubtract, core::BinaryOp::kDivide,
                        core::BinaryOp::kModulo});
    }
    for (std::string vector_type : {"vec2<i32>", "vec3<i32>", "vec4<i32>"}) {
        std::string scalar_type = "i32";
        CheckMutations(vector_type, scalar_type, vector_type, core::BinaryOp::kMultiply,
                       {core::BinaryOp::kAdd, core::BinaryOp::kSubtract, core::BinaryOp::kDivide,
                        core::BinaryOp::kModulo});
        CheckMutations(scalar_type, vector_type, vector_type, core::BinaryOp::kMultiply,
                       {core::BinaryOp::kAdd, core::BinaryOp::kSubtract, core::BinaryOp::kDivide,
                        core::BinaryOp::kModulo});
    }
    for (std::string vector_type : {"vec2<u32>", "vec3<u32>", "vec4<u32>"}) {
        std::string scalar_type = "u32";
        CheckMutations(vector_type, scalar_type, vector_type, core::BinaryOp::kMultiply,
                       {core::BinaryOp::kAdd, core::BinaryOp::kSubtract, core::BinaryOp::kDivide,
                        core::BinaryOp::kModulo});
        CheckMutations(scalar_type, vector_type, vector_type, core::BinaryOp::kMultiply,
                       {core::BinaryOp::kAdd, core::BinaryOp::kSubtract, core::BinaryOp::kDivide,
                        core::BinaryOp::kModulo});
    }
    for (std::string vector_type : {"vec2<f32>", "vec3<f32>", "vec4<f32>"}) {
        std::string scalar_type = "f32";
        CheckMutations(vector_type, scalar_type, vector_type, core::BinaryOp::kMultiply,
                       {core::BinaryOp::kAdd, core::BinaryOp::kSubtract, core::BinaryOp::kDivide,
                        core::BinaryOp::kModulo});
        CheckMutations(scalar_type, vector_type, vector_type, core::BinaryOp::kMultiply,
                       {core::BinaryOp::kAdd, core::BinaryOp::kSubtract, core::BinaryOp::kDivide,
                        core::BinaryOp::kModulo});
    }
    for (std::string square_matrix_type : {"mat2x2<f32>", "mat3x3<f32>", "mat4x4<f32>"}) {
        CheckMutations(square_matrix_type, square_matrix_type, square_matrix_type,
                       core::BinaryOp::kMultiply,
                       {core::BinaryOp::kAdd, core::BinaryOp::kSubtract});
    }

    CheckMutations("vec2<f32>", "mat2x2<f32>", "vec2<f32>", core::BinaryOp::kMultiply, {});
    CheckMutations("vec2<f32>", "mat3x2<f32>", "vec3<f32>", core::BinaryOp::kMultiply, {});
    CheckMutations("vec2<f32>", "mat4x2<f32>", "vec4<f32>", core::BinaryOp::kMultiply, {});

    CheckMutations("mat2x2<f32>", "vec2<f32>", "vec2<f32>", core::BinaryOp::kMultiply, {});
    CheckMutations("mat2x2<f32>", "mat3x2<f32>", "mat3x2<f32>", core::BinaryOp::kMultiply, {});
    CheckMutations("mat2x2<f32>", "mat4x2<f32>", "mat4x2<f32>", core::BinaryOp::kMultiply, {});

    CheckMutations("mat2x3<f32>", "vec2<f32>", "vec3<f32>", core::BinaryOp::kMultiply, {});
    CheckMutations("mat2x3<f32>", "mat2x2<f32>", "mat2x3<f32>", core::BinaryOp::kMultiply, {});
    CheckMutations("mat2x3<f32>", "mat3x2<f32>", "mat3x3<f32>", core::BinaryOp::kMultiply, {});
    CheckMutations("mat2x3<f32>", "mat4x2<f32>", "mat4x3<f32>", core::BinaryOp::kMultiply, {});

    CheckMutations("mat2x4<f32>", "vec2<f32>", "vec4<f32>", core::BinaryOp::kMultiply, {});
    CheckMutations("mat2x4<f32>", "mat2x2<f32>", "mat2x4<f32>", core::BinaryOp::kMultiply, {});
    CheckMutations("mat2x4<f32>", "mat3x2<f32>", "mat3x4<f32>", core::BinaryOp::kMultiply, {});
    CheckMutations("mat2x4<f32>", "mat4x2<f32>", "mat4x4<f32>", core::BinaryOp::kMultiply, {});

    CheckMutations("vec3<f32>", "mat2x3<f32>", "vec2<f32>", core::BinaryOp::kMultiply, {});
    CheckMutations("vec3<f32>", "mat3x3<f32>", "vec3<f32>", core::BinaryOp::kMultiply, {});
    CheckMutations("vec3<f32>", "mat4x3<f32>", "vec4<f32>", core::BinaryOp::kMultiply, {});

    CheckMutations("mat3x2<f32>", "vec3<f32>", "vec2<f32>", core::BinaryOp::kMultiply, {});
    CheckMutations("mat3x2<f32>", "mat2x3<f32>", "mat2x2<f32>", core::BinaryOp::kMultiply, {});
    CheckMutations("mat3x2<f32>", "mat3x3<f32>", "mat3x2<f32>", core::BinaryOp::kMultiply, {});
    CheckMutations("mat3x2<f32>", "mat4x3<f32>", "mat4x2<f32>", core::BinaryOp::kMultiply, {});

    CheckMutations("mat3x3<f32>", "vec3<f32>", "vec3<f32>", core::BinaryOp::kMultiply, {});
    CheckMutations("mat3x3<f32>", "mat2x3<f32>", "mat2x3<f32>", core::BinaryOp::kMultiply, {});
    CheckMutations("mat3x3<f32>", "mat4x3<f32>", "mat4x3<f32>", core::BinaryOp::kMultiply, {});

    CheckMutations("mat3x4<f32>", "vec3<f32>", "vec4<f32>", core::BinaryOp::kMultiply, {});
    CheckMutations("mat3x4<f32>", "mat2x3<f32>", "mat2x4<f32>", core::BinaryOp::kMultiply, {});
    CheckMutations("mat3x4<f32>", "mat3x3<f32>", "mat3x4<f32>", core::BinaryOp::kMultiply, {});
    CheckMutations("mat3x4<f32>", "mat4x3<f32>", "mat4x4<f32>", core::BinaryOp::kMultiply, {});

    CheckMutations("vec4<f32>", "mat2x4<f32>", "vec2<f32>", core::BinaryOp::kMultiply, {});
    CheckMutations("vec4<f32>", "mat3x4<f32>", "vec3<f32>", core::BinaryOp::kMultiply, {});
    CheckMutations("vec4<f32>", "mat4x4<f32>", "vec4<f32>", core::BinaryOp::kMultiply, {});

    CheckMutations("mat4x2<f32>", "vec4<f32>", "vec2<f32>", core::BinaryOp::kMultiply, {});
    CheckMutations("mat4x2<f32>", "mat2x4<f32>", "mat2x2<f32>", core::BinaryOp::kMultiply, {});
    CheckMutations("mat4x2<f32>", "mat3x4<f32>", "mat3x2<f32>", core::BinaryOp::kMultiply, {});
    CheckMutations("mat4x2<f32>", "mat4x4<f32>", "mat4x2<f32>", core::BinaryOp::kMultiply, {});

    CheckMutations("mat4x3<f32>", "vec4<f32>", "vec3<f32>", core::BinaryOp::kMultiply, {});
    CheckMutations("mat4x3<f32>", "mat2x4<f32>", "mat2x3<f32>", core::BinaryOp::kMultiply, {});
    CheckMutations("mat4x3<f32>", "mat3x4<f32>", "mat3x3<f32>", core::BinaryOp::kMultiply, {});
    CheckMutations("mat4x3<f32>", "mat4x4<f32>", "mat4x3<f32>", core::BinaryOp::kMultiply, {});

    CheckMutations("mat4x4<f32>", "vec4<f32>", "vec4<f32>", core::BinaryOp::kMultiply, {});
    CheckMutations("mat4x4<f32>", "mat2x4<f32>", "mat2x4<f32>", core::BinaryOp::kMultiply, {});
    CheckMutations("mat4x4<f32>", "mat3x4<f32>", "mat3x4<f32>", core::BinaryOp::kMultiply, {});
}

TEST(ChangeBinaryOperatorTest, DivideAndModulo) {
    for (std::string type : {"i32", "vec2<i32>", "vec3<i32>", "vec4<i32>"}) {
        CheckMutations(type, type, type, core::BinaryOp::kDivide,
                       {core::BinaryOp::kAdd, core::BinaryOp::kSubtract, core::BinaryOp::kMultiply,
                        core::BinaryOp::kModulo, core::BinaryOp::kAnd, core::BinaryOp::kOr,
                        core::BinaryOp::kXor});
    }
    for (std::string type : {"u32", "vec2<u32>", "vec3<u32>", "vec4<u32>"}) {
        CheckMutations(
            type, type, type, core::BinaryOp::kDivide,
            {core::BinaryOp::kAdd, core::BinaryOp::kSubtract, core::BinaryOp::kMultiply,
             core::BinaryOp::kModulo, core::BinaryOp::kAnd, core::BinaryOp::kOr,
             core::BinaryOp::kXor, core::BinaryOp::kShiftLeft, core::BinaryOp::kShiftRight});
    }
    for (std::string type : {"f32", "vec2<f32>", "vec3<f32>", "vec4<f32>"}) {
        CheckMutations(type, type, type, core::BinaryOp::kDivide,
                       {core::BinaryOp::kAdd, core::BinaryOp::kSubtract, core::BinaryOp::kMultiply,
                        core::BinaryOp::kModulo});
    }
    for (std::string vector_type : {"vec2<i32>", "vec3<i32>", "vec4<i32>"}) {
        std::string scalar_type = "i32";
        CheckMutations(vector_type, scalar_type, vector_type, core::BinaryOp::kDivide,
                       {core::BinaryOp::kAdd, core::BinaryOp::kSubtract, core::BinaryOp::kMultiply,
                        core::BinaryOp::kModulo});
        CheckMutations(scalar_type, vector_type, vector_type, core::BinaryOp::kDivide,
                       {core::BinaryOp::kAdd, core::BinaryOp::kSubtract, core::BinaryOp::kMultiply,
                        core::BinaryOp::kModulo});
    }
    for (std::string vector_type : {"vec2<u32>", "vec3<u32>", "vec4<u32>"}) {
        std::string scalar_type = "u32";
        CheckMutations(vector_type, scalar_type, vector_type, core::BinaryOp::kDivide,
                       {core::BinaryOp::kAdd, core::BinaryOp::kSubtract, core::BinaryOp::kMultiply,
                        core::BinaryOp::kModulo});
        CheckMutations(scalar_type, vector_type, vector_type, core::BinaryOp::kDivide,
                       {core::BinaryOp::kAdd, core::BinaryOp::kSubtract, core::BinaryOp::kMultiply,
                        core::BinaryOp::kModulo});
    }
    for (std::string vector_type : {"vec2<f32>", "vec3<f32>", "vec4<f32>"}) {
        std::string scalar_type = "f32";
        CheckMutations(vector_type, scalar_type, vector_type, core::BinaryOp::kDivide,
                       {core::BinaryOp::kAdd, core::BinaryOp::kSubtract, core::BinaryOp::kMultiply,
                        core::BinaryOp::kModulo});
        CheckMutations(scalar_type, vector_type, vector_type, core::BinaryOp::kDivide,
                       {core::BinaryOp::kAdd, core::BinaryOp::kSubtract, core::BinaryOp::kMultiply,
                        core::BinaryOp::kModulo});
    }
    for (std::string type : {"i32", "vec2<i32>", "vec3<i32>", "vec4<i32>"}) {
        CheckMutations(type, type, type, core::BinaryOp::kModulo,
                       {core::BinaryOp::kAdd, core::BinaryOp::kSubtract, core::BinaryOp::kMultiply,
                        core::BinaryOp::kDivide, core::BinaryOp::kAnd, core::BinaryOp::kOr,
                        core::BinaryOp::kXor});
    }
    for (std::string type : {"u32", "vec2<u32>", "vec3<u32>", "vec4<u32>"}) {
        CheckMutations(
            type, type, type, core::BinaryOp::kModulo,
            {core::BinaryOp::kAdd, core::BinaryOp::kSubtract, core::BinaryOp::kMultiply,
             core::BinaryOp::kDivide, core::BinaryOp::kAnd, core::BinaryOp::kOr,
             core::BinaryOp::kXor, core::BinaryOp::kShiftLeft, core::BinaryOp::kShiftRight});
    }
    for (std::string type : {"f32", "vec2<f32>", "vec3<f32>", "vec4<f32>"}) {
        CheckMutations(type, type, type, core::BinaryOp::kModulo,
                       {core::BinaryOp::kAdd, core::BinaryOp::kSubtract, core::BinaryOp::kMultiply,
                        core::BinaryOp::kDivide});
    }
    for (std::string vector_type : {"vec2<i32>", "vec3<i32>", "vec4<i32>"}) {
        std::string scalar_type = "i32";
        CheckMutations(vector_type, scalar_type, vector_type, core::BinaryOp::kModulo,
                       {core::BinaryOp::kAdd, core::BinaryOp::kSubtract, core::BinaryOp::kMultiply,
                        core::BinaryOp::kDivide});
        CheckMutations(scalar_type, vector_type, vector_type, core::BinaryOp::kModulo,
                       {core::BinaryOp::kAdd, core::BinaryOp::kSubtract, core::BinaryOp::kMultiply,
                        core::BinaryOp::kDivide});
    }
    for (std::string vector_type : {"vec2<u32>", "vec3<u32>", "vec4<u32>"}) {
        std::string scalar_type = "u32";
        CheckMutations(vector_type, scalar_type, vector_type, core::BinaryOp::kModulo,
                       {core::BinaryOp::kAdd, core::BinaryOp::kSubtract, core::BinaryOp::kMultiply,
                        core::BinaryOp::kDivide});
        CheckMutations(scalar_type, vector_type, vector_type, core::BinaryOp::kModulo,
                       {core::BinaryOp::kAdd, core::BinaryOp::kSubtract, core::BinaryOp::kMultiply,
                        core::BinaryOp::kDivide});
    }
}

TEST(ChangeBinaryOperatorTest, AndOrXor) {
    for (auto op : {core::BinaryOp::kAnd, core::BinaryOp::kOr, core::BinaryOp::kXor}) {
        std::unordered_set<core::BinaryOp> allowed_replacement_operators_signed{
            core::BinaryOp::kAdd,    core::BinaryOp::kSubtract, core::BinaryOp::kMultiply,
            core::BinaryOp::kDivide, core::BinaryOp::kModulo,   core::BinaryOp::kAnd,
            core::BinaryOp::kOr,     core::BinaryOp::kXor};
        allowed_replacement_operators_signed.erase(op);
        for (std::string type : {"i32", "vec2<i32>", "vec3<i32>", "vec4<i32>"}) {
            CheckMutations(type, type, type, op, allowed_replacement_operators_signed);
        }
        std::unordered_set<core::BinaryOp> allowed_replacement_operators_unsigned{
            core::BinaryOp::kAdd,        core::BinaryOp::kSubtract, core::BinaryOp::kMultiply,
            core::BinaryOp::kDivide,     core::BinaryOp::kModulo,   core::BinaryOp::kShiftLeft,
            core::BinaryOp::kShiftRight, core::BinaryOp::kAnd,      core::BinaryOp::kOr,
            core::BinaryOp::kXor};
        allowed_replacement_operators_unsigned.erase(op);
        for (std::string type : {"u32", "vec2<u32>", "vec3<u32>", "vec4<u32>"}) {
            CheckMutations(type, type, type, op, allowed_replacement_operators_unsigned);
        }
        if (op != core::BinaryOp::kXor) {
            for (std::string type : {"bool", "vec2<bool>", "vec3<bool>", "vec4<bool>"}) {
                std::unordered_set<core::BinaryOp> allowed_replacement_operators_bool{
                    core::BinaryOp::kAnd, core::BinaryOp::kOr, core::BinaryOp::kEqual,
                    core::BinaryOp::kNotEqual};
                allowed_replacement_operators_bool.erase(op);
                if (type == "bool") {
                    allowed_replacement_operators_bool.insert(core::BinaryOp::kLogicalAnd);
                    allowed_replacement_operators_bool.insert(core::BinaryOp::kLogicalOr);
                }
                CheckMutations(type, type, type, op, allowed_replacement_operators_bool);
            }
        }
    }
}

TEST(ChangeBinaryOperatorTest, EqualNotEqual) {
    for (auto op : {core::BinaryOp::kEqual, core::BinaryOp::kNotEqual}) {
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
                std::unordered_set<core::BinaryOp> allowed_replacement_operators{
                    core::BinaryOp::kLessThan,    core::BinaryOp::kLessThanEqual,
                    core::BinaryOp::kGreaterThan, core::BinaryOp::kGreaterThanEqual,
                    core::BinaryOp::kEqual,       core::BinaryOp::kNotEqual};
                allowed_replacement_operators.erase(op);
                CheckMutations(argument_type.str(), argument_type.str(), result_type.str(), op,
                               allowed_replacement_operators);
            }
        }
        {
            std::unordered_set<core::BinaryOp> allowed_replacement_operators{
                core::BinaryOp::kLogicalAnd, core::BinaryOp::kLogicalOr, core::BinaryOp::kAnd,
                core::BinaryOp::kOr,         core::BinaryOp::kEqual,     core::BinaryOp::kNotEqual};
            allowed_replacement_operators.erase(op);
            CheckMutations("bool", "bool", "bool", op, allowed_replacement_operators);
        }
        for (size_t element_count = 2; element_count <= 4; element_count++) {
            std::stringstream argument_and_result_type;
            argument_and_result_type << "vec" << element_count << "<bool>";
            std::unordered_set<core::BinaryOp> allowed_replacement_operators{
                core::BinaryOp::kAnd, core::BinaryOp::kOr, core::BinaryOp::kEqual,
                core::BinaryOp::kNotEqual};
            allowed_replacement_operators.erase(op);
            CheckMutations(argument_and_result_type.str(), argument_and_result_type.str(),
                           argument_and_result_type.str(), op, allowed_replacement_operators);
        }
    }
}

TEST(ChangeBinaryOperatorTest, LessThanLessThanEqualGreaterThanGreaterThanEqual) {
    for (auto op : {core::BinaryOp::kLessThan, core::BinaryOp::kLessThanEqual,
                    core::BinaryOp::kGreaterThan, core::BinaryOp::kGreaterThanEqual}) {
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
                std::unordered_set<core::BinaryOp> allowed_replacement_operators{
                    core::BinaryOp::kLessThan,    core::BinaryOp::kLessThanEqual,
                    core::BinaryOp::kGreaterThan, core::BinaryOp::kGreaterThanEqual,
                    core::BinaryOp::kEqual,       core::BinaryOp::kNotEqual};
                allowed_replacement_operators.erase(op);
                CheckMutations(argument_type.str(), argument_type.str(), result_type.str(), op,
                               allowed_replacement_operators);
            }
        }
    }
}

TEST(ChangeBinaryOperatorTest, LogicalAndLogicalOr) {
    for (auto op : {core::BinaryOp::kLogicalAnd, core::BinaryOp::kLogicalOr}) {
        std::unordered_set<core::BinaryOp> allowed_replacement_operators{
            core::BinaryOp::kLogicalAnd, core::BinaryOp::kLogicalOr, core::BinaryOp::kAnd,
            core::BinaryOp::kOr,         core::BinaryOp::kEqual,     core::BinaryOp::kNotEqual};
        allowed_replacement_operators.erase(op);
        CheckMutations("bool", "bool", "bool", op, allowed_replacement_operators);
    }
}

TEST(ChangeBinaryOperatorTest, ShiftLeftShiftRight) {
    for (auto op : {core::BinaryOp::kShiftLeft, core::BinaryOp::kShiftRight}) {
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
                std::unordered_set<core::BinaryOp> allowed_replacement_operators{
                    core::BinaryOp::kShiftLeft, core::BinaryOp::kShiftRight};
                allowed_replacement_operators.erase(op);
                if (lhs_element_type == "u32") {
                    allowed_replacement_operators.insert(core::BinaryOp::kAdd);
                    allowed_replacement_operators.insert(core::BinaryOp::kSubtract);
                    allowed_replacement_operators.insert(core::BinaryOp::kMultiply);
                    allowed_replacement_operators.insert(core::BinaryOp::kDivide);
                    allowed_replacement_operators.insert(core::BinaryOp::kModulo);
                    allowed_replacement_operators.insert(core::BinaryOp::kAnd);
                    allowed_replacement_operators.insert(core::BinaryOp::kOr);
                    allowed_replacement_operators.insert(core::BinaryOp::kXor);
                }
                CheckMutations(lhs_and_result_type.str(), rhs_type.str(), lhs_and_result_type.str(),
                               op, allowed_replacement_operators);
            }
        }
    }
}

}  // namespace
}  // namespace tint::fuzzers::ast_fuzzer
