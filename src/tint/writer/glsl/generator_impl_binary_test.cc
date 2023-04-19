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

#include "src/tint/ast/call_statement.h"
#include "src/tint/ast/variable_decl_statement.h"
#include "src/tint/utils/string_stream.h"
#include "src/tint/writer/glsl/test_helper.h"

#include "gmock/gmock.h"

using namespace tint::number_suffixes;  // NOLINT

namespace tint::writer::glsl {
namespace {

using GlslGeneratorImplTest_Binary = TestHelper;

struct BinaryData {
    const char* result;
    ast::BinaryOp op;
};
inline std::ostream& operator<<(std::ostream& out, BinaryData data) {
    utils::StringStream str;
    str << data.op;
    out << str.str();
    return out;
}

using GlslBinaryTest = TestParamHelper<BinaryData>;
TEST_P(GlslBinaryTest, Emit_f32) {
    auto params = GetParam();

    // Skip ops that are illegal for this type
    if (params.op == ast::BinaryOp::kAnd || params.op == ast::BinaryOp::kOr ||
        params.op == ast::BinaryOp::kXor || params.op == ast::BinaryOp::kShiftLeft ||
        params.op == ast::BinaryOp::kShiftRight || params.op == ast::BinaryOp::kModulo) {
        return;
    }

    GlobalVar("left", ty.f32(), builtin::AddressSpace::kPrivate);
    GlobalVar("right", ty.f32(), builtin::AddressSpace::kPrivate);

    auto* left = Expr("left");
    auto* right = Expr("right");

    auto* expr = create<ast::BinaryExpression>(params.op, left, right);

    WrapInFunction(expr);

    GeneratorImpl& gen = Build();

    utils::StringStream out;
    gen.EmitExpression(out, expr);
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_EQ(out.str(), params.result);
}
TEST_P(GlslBinaryTest, Emit_f16) {
    auto params = GetParam();

    // Skip ops that are illegal for this type
    if (params.op == ast::BinaryOp::kAnd || params.op == ast::BinaryOp::kOr ||
        params.op == ast::BinaryOp::kXor || params.op == ast::BinaryOp::kShiftLeft ||
        params.op == ast::BinaryOp::kShiftRight || params.op == ast::BinaryOp::kModulo) {
        return;
    }

    Enable(builtin::Extension::kF16);

    GlobalVar("left", ty.f16(), builtin::AddressSpace::kPrivate);
    GlobalVar("right", ty.f16(), builtin::AddressSpace::kPrivate);

    auto* left = Expr("left");
    auto* right = Expr("right");

    auto* expr = create<ast::BinaryExpression>(params.op, left, right);

    WrapInFunction(expr);

    GeneratorImpl& gen = Build();

    utils::StringStream out;
    gen.EmitExpression(out, expr);
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_EQ(out.str(), params.result);
}
TEST_P(GlslBinaryTest, Emit_u32) {
    auto params = GetParam();

    GlobalVar("left", ty.u32(), builtin::AddressSpace::kPrivate);
    GlobalVar("right", ty.u32(), builtin::AddressSpace::kPrivate);

    auto* left = Expr("left");
    auto* right = Expr("right");

    auto* expr = create<ast::BinaryExpression>(params.op, left, right);

    WrapInFunction(expr);

    GeneratorImpl& gen = Build();

    utils::StringStream out;
    gen.EmitExpression(out, expr);
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_EQ(out.str(), params.result);
}
TEST_P(GlslBinaryTest, Emit_i32) {
    auto params = GetParam();

    // Skip ops that are illegal for this type
    if (params.op == ast::BinaryOp::kShiftLeft || params.op == ast::BinaryOp::kShiftRight) {
        return;
    }

    GlobalVar("left", ty.i32(), builtin::AddressSpace::kPrivate);
    GlobalVar("right", ty.i32(), builtin::AddressSpace::kPrivate);

    auto* left = Expr("left");
    auto* right = Expr("right");

    auto* expr = create<ast::BinaryExpression>(params.op, left, right);

    WrapInFunction(expr);

    GeneratorImpl& gen = Build();

    utils::StringStream out;
    gen.EmitExpression(out, expr);
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_EQ(out.str(), params.result);
}
INSTANTIATE_TEST_SUITE_P(
    GlslGeneratorImplTest,
    GlslBinaryTest,
    testing::Values(BinaryData{"(left & right)", ast::BinaryOp::kAnd},
                    BinaryData{"(left | right)", ast::BinaryOp::kOr},
                    BinaryData{"(left ^ right)", ast::BinaryOp::kXor},
                    BinaryData{"(left == right)", ast::BinaryOp::kEqual},
                    BinaryData{"(left != right)", ast::BinaryOp::kNotEqual},
                    BinaryData{"(left < right)", ast::BinaryOp::kLessThan},
                    BinaryData{"(left > right)", ast::BinaryOp::kGreaterThan},
                    BinaryData{"(left <= right)", ast::BinaryOp::kLessThanEqual},
                    BinaryData{"(left >= right)", ast::BinaryOp::kGreaterThanEqual},
                    BinaryData{"(left << right)", ast::BinaryOp::kShiftLeft},
                    BinaryData{"(left >> right)", ast::BinaryOp::kShiftRight},
                    BinaryData{"(left + right)", ast::BinaryOp::kAdd},
                    BinaryData{"(left - right)", ast::BinaryOp::kSubtract},
                    BinaryData{"(left * right)", ast::BinaryOp::kMultiply},
                    BinaryData{"(left / right)", ast::BinaryOp::kDivide},
                    BinaryData{"(left % right)", ast::BinaryOp::kModulo}));

TEST_F(GlslGeneratorImplTest_Binary, Multiply_VectorScalar_f32) {
    GlobalVar("a", vec3<f32>(1_f, 1_f, 1_f), builtin::AddressSpace::kPrivate);
    auto* lhs = Expr("a");
    auto* rhs = Expr(1_f);

    auto* expr = create<ast::BinaryExpression>(ast::BinaryOp::kMultiply, lhs, rhs);

    WrapInFunction(expr);

    GeneratorImpl& gen = Build();

    utils::StringStream out;
    gen.EmitExpression(out, expr);
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_EQ(out.str(), "(a * 1.0f)");
}

TEST_F(GlslGeneratorImplTest_Binary, Multiply_VectorScalar_f16) {
    Enable(builtin::Extension::kF16);

    GlobalVar("a", vec3<f16>(1_h, 1_h, 1_h), builtin::AddressSpace::kPrivate);
    auto* lhs = Expr("a");
    auto* rhs = Expr(1_h);

    auto* expr = create<ast::BinaryExpression>(ast::BinaryOp::kMultiply, lhs, rhs);

    WrapInFunction(expr);

    GeneratorImpl& gen = Build();

    utils::StringStream out;
    gen.EmitExpression(out, expr);
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_EQ(out.str(), "(a * 1.0hf)");
}

TEST_F(GlslGeneratorImplTest_Binary, Multiply_ScalarVector_f32) {
    GlobalVar("a", vec3<f32>(1_f, 1_f, 1_f), builtin::AddressSpace::kPrivate);
    auto* lhs = Expr(1_f);
    auto* rhs = Expr("a");

    auto* expr = create<ast::BinaryExpression>(ast::BinaryOp::kMultiply, lhs, rhs);

    WrapInFunction(expr);

    GeneratorImpl& gen = Build();

    utils::StringStream out;
    gen.EmitExpression(out, expr);
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_EQ(out.str(), "(1.0f * a)");
}

TEST_F(GlslGeneratorImplTest_Binary, Multiply_ScalarVector_f16) {
    Enable(builtin::Extension::kF16);

    GlobalVar("a", vec3<f16>(1_h, 1_h, 1_h), builtin::AddressSpace::kPrivate);
    auto* lhs = Expr(1_h);
    auto* rhs = Expr("a");

    auto* expr = create<ast::BinaryExpression>(ast::BinaryOp::kMultiply, lhs, rhs);

    WrapInFunction(expr);

    GeneratorImpl& gen = Build();

    utils::StringStream out;
    gen.EmitExpression(out, expr);
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_EQ(out.str(), "(1.0hf * a)");
}

TEST_F(GlslGeneratorImplTest_Binary, Multiply_MatrixScalar_f32) {
    GlobalVar("mat", ty.mat3x3<f32>(), builtin::AddressSpace::kPrivate);
    auto* lhs = Expr("mat");
    auto* rhs = Expr(1_f);

    auto* expr = create<ast::BinaryExpression>(ast::BinaryOp::kMultiply, lhs, rhs);
    WrapInFunction(expr);

    GeneratorImpl& gen = Build();

    utils::StringStream out;
    gen.EmitExpression(out, expr);
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_EQ(out.str(), "(mat * 1.0f)");
}

TEST_F(GlslGeneratorImplTest_Binary, Multiply_MatrixScalar_f16) {
    Enable(builtin::Extension::kF16);

    GlobalVar("mat", ty.mat3x3<f16>(), builtin::AddressSpace::kPrivate);
    auto* lhs = Expr("mat");
    auto* rhs = Expr(1_h);

    auto* expr = create<ast::BinaryExpression>(ast::BinaryOp::kMultiply, lhs, rhs);
    WrapInFunction(expr);

    GeneratorImpl& gen = Build();

    utils::StringStream out;
    gen.EmitExpression(out, expr);
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_EQ(out.str(), "(mat * 1.0hf)");
}

TEST_F(GlslGeneratorImplTest_Binary, Multiply_ScalarMatrix_f32) {
    GlobalVar("mat", ty.mat3x3<f32>(), builtin::AddressSpace::kPrivate);
    auto* lhs = Expr(1_f);
    auto* rhs = Expr("mat");

    auto* expr = create<ast::BinaryExpression>(ast::BinaryOp::kMultiply, lhs, rhs);
    WrapInFunction(expr);

    GeneratorImpl& gen = Build();

    utils::StringStream out;
    gen.EmitExpression(out, expr);
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_EQ(out.str(), "(1.0f * mat)");
}

TEST_F(GlslGeneratorImplTest_Binary, Multiply_ScalarMatrix_f16) {
    Enable(builtin::Extension::kF16);

    GlobalVar("mat", ty.mat3x3<f16>(), builtin::AddressSpace::kPrivate);
    auto* lhs = Expr(1_h);
    auto* rhs = Expr("mat");

    auto* expr = create<ast::BinaryExpression>(ast::BinaryOp::kMultiply, lhs, rhs);
    WrapInFunction(expr);

    GeneratorImpl& gen = Build();

    utils::StringStream out;
    gen.EmitExpression(out, expr);
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_EQ(out.str(), "(1.0hf * mat)");
}

TEST_F(GlslGeneratorImplTest_Binary, Multiply_MatrixVector_f32) {
    GlobalVar("mat", ty.mat3x3<f32>(), builtin::AddressSpace::kPrivate);
    auto* lhs = Expr("mat");
    auto* rhs = vec3<f32>(1_f, 1_f, 1_f);

    auto* expr = create<ast::BinaryExpression>(ast::BinaryOp::kMultiply, lhs, rhs);
    WrapInFunction(expr);

    GeneratorImpl& gen = Build();

    utils::StringStream out;
    gen.EmitExpression(out, expr);
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_EQ(out.str(), "(mat * vec3(1.0f))");
}

TEST_F(GlslGeneratorImplTest_Binary, Multiply_MatrixVector_f16) {
    Enable(builtin::Extension::kF16);

    GlobalVar("mat", ty.mat3x3<f16>(), builtin::AddressSpace::kPrivate);
    auto* lhs = Expr("mat");
    auto* rhs = vec3<f16>(1_h, 1_h, 1_h);

    auto* expr = create<ast::BinaryExpression>(ast::BinaryOp::kMultiply, lhs, rhs);
    WrapInFunction(expr);

    GeneratorImpl& gen = Build();

    utils::StringStream out;
    gen.EmitExpression(out, expr);
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_EQ(out.str(), "(mat * f16vec3(1.0hf))");
}

TEST_F(GlslGeneratorImplTest_Binary, Multiply_VectorMatrix_f32) {
    GlobalVar("mat", ty.mat3x3<f32>(), builtin::AddressSpace::kPrivate);
    auto* lhs = vec3<f32>(1_f, 1_f, 1_f);
    auto* rhs = Expr("mat");

    auto* expr = create<ast::BinaryExpression>(ast::BinaryOp::kMultiply, lhs, rhs);
    WrapInFunction(expr);

    GeneratorImpl& gen = Build();

    utils::StringStream out;
    gen.EmitExpression(out, expr);
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_EQ(out.str(), "(vec3(1.0f) * mat)");
}

TEST_F(GlslGeneratorImplTest_Binary, Multiply_VectorMatrix_f16) {
    Enable(builtin::Extension::kF16);

    GlobalVar("mat", ty.mat3x3<f16>(), builtin::AddressSpace::kPrivate);
    auto* lhs = vec3<f16>(1_h, 1_h, 1_h);
    auto* rhs = Expr("mat");

    auto* expr = create<ast::BinaryExpression>(ast::BinaryOp::kMultiply, lhs, rhs);
    WrapInFunction(expr);

    GeneratorImpl& gen = Build();

    utils::StringStream out;
    gen.EmitExpression(out, expr);
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_EQ(out.str(), "(f16vec3(1.0hf) * mat)");
}

TEST_F(GlslGeneratorImplTest_Binary, Multiply_MatrixMatrix_f32) {
    GlobalVar("lhs", ty.mat3x3<f32>(), builtin::AddressSpace::kPrivate);
    GlobalVar("rhs", ty.mat3x3<f32>(), builtin::AddressSpace::kPrivate);

    auto* expr = create<ast::BinaryExpression>(ast::BinaryOp::kMultiply, Expr("lhs"), Expr("rhs"));
    WrapInFunction(expr);

    GeneratorImpl& gen = Build();

    utils::StringStream out;
    gen.EmitExpression(out, expr);
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_EQ(out.str(), "(lhs * rhs)");
}

TEST_F(GlslGeneratorImplTest_Binary, Multiply_MatrixMatrix_f16) {
    Enable(builtin::Extension::kF16);

    GlobalVar("lhs", ty.mat3x3<f16>(), builtin::AddressSpace::kPrivate);
    GlobalVar("rhs", ty.mat3x3<f16>(), builtin::AddressSpace::kPrivate);

    auto* expr = create<ast::BinaryExpression>(ast::BinaryOp::kMultiply, Expr("lhs"), Expr("rhs"));
    WrapInFunction(expr);

    GeneratorImpl& gen = Build();

    utils::StringStream out;
    gen.EmitExpression(out, expr);
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_EQ(out.str(), "(lhs * rhs)");
}

TEST_F(GlslGeneratorImplTest_Binary, ModF32) {
    GlobalVar("a", ty.f32(), builtin::AddressSpace::kPrivate);
    GlobalVar("b", ty.f32(), builtin::AddressSpace::kPrivate);

    auto* expr = create<ast::BinaryExpression>(ast::BinaryOp::kModulo, Expr("a"), Expr("b"));
    WrapInFunction(expr);

    GeneratorImpl& gen = Build();

    utils::StringStream out;
    gen.EmitExpression(out, expr);
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_EQ(out.str(), "tint_float_modulo(a, b)");
}

TEST_F(GlslGeneratorImplTest_Binary, ModF16) {
    Enable(builtin::Extension::kF16);

    GlobalVar("a", ty.f16(), builtin::AddressSpace::kPrivate);
    GlobalVar("b", ty.f16(), builtin::AddressSpace::kPrivate);

    auto* expr = create<ast::BinaryExpression>(ast::BinaryOp::kModulo, Expr("a"), Expr("b"));
    WrapInFunction(expr);

    GeneratorImpl& gen = Build();

    utils::StringStream out;
    gen.EmitExpression(out, expr);
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_EQ(out.str(), "tint_float_modulo(a, b)");
}

TEST_F(GlslGeneratorImplTest_Binary, ModVec3F32) {
    GlobalVar("a", ty.vec3<f32>(), builtin::AddressSpace::kPrivate);
    GlobalVar("b", ty.vec3<f32>(), builtin::AddressSpace::kPrivate);

    auto* expr = create<ast::BinaryExpression>(ast::BinaryOp::kModulo, Expr("a"), Expr("b"));
    WrapInFunction(expr);

    GeneratorImpl& gen = Build();

    utils::StringStream out;
    gen.EmitExpression(out, expr);
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_EQ(out.str(), "tint_float_modulo(a, b)");
}

TEST_F(GlslGeneratorImplTest_Binary, ModVec3F16) {
    Enable(builtin::Extension::kF16);

    GlobalVar("a", ty.vec3<f16>(), builtin::AddressSpace::kPrivate);
    GlobalVar("b", ty.vec3<f16>(), builtin::AddressSpace::kPrivate);

    auto* expr = create<ast::BinaryExpression>(ast::BinaryOp::kModulo, Expr("a"), Expr("b"));
    WrapInFunction(expr);

    GeneratorImpl& gen = Build();

    utils::StringStream out;
    gen.EmitExpression(out, expr);
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_EQ(out.str(), "tint_float_modulo(a, b)");
}

TEST_F(GlslGeneratorImplTest_Binary, ModVec3F32ScalarF32) {
    GlobalVar("a", ty.vec3<f32>(), builtin::AddressSpace::kPrivate);
    GlobalVar("b", ty.f32(), builtin::AddressSpace::kPrivate);

    auto* expr = create<ast::BinaryExpression>(ast::BinaryOp::kModulo, Expr("a"), Expr("b"));
    WrapInFunction(expr);

    GeneratorImpl& gen = Build();

    utils::StringStream out;
    gen.EmitExpression(out, expr);
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_EQ(out.str(), "tint_float_modulo(a, b)");
}

TEST_F(GlslGeneratorImplTest_Binary, ModVec3F16ScalarF16) {
    Enable(builtin::Extension::kF16);

    GlobalVar("a", ty.vec3<f16>(), builtin::AddressSpace::kPrivate);
    GlobalVar("b", ty.f16(), builtin::AddressSpace::kPrivate);

    auto* expr = create<ast::BinaryExpression>(ast::BinaryOp::kModulo, Expr("a"), Expr("b"));
    WrapInFunction(expr);

    GeneratorImpl& gen = Build();

    utils::StringStream out;
    gen.EmitExpression(out, expr);
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_EQ(out.str(), "tint_float_modulo(a, b)");
}

TEST_F(GlslGeneratorImplTest_Binary, ModScalarF32Vec3F32) {
    GlobalVar("a", ty.f32(), builtin::AddressSpace::kPrivate);
    GlobalVar("b", ty.vec3<f32>(), builtin::AddressSpace::kPrivate);

    auto* expr = create<ast::BinaryExpression>(ast::BinaryOp::kModulo, Expr("a"), Expr("b"));
    WrapInFunction(expr);

    GeneratorImpl& gen = Build();

    utils::StringStream out;
    gen.EmitExpression(out, expr);
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_EQ(out.str(), "tint_float_modulo(a, b)");
}

TEST_F(GlslGeneratorImplTest_Binary, ModScalarF16Vec3F16) {
    Enable(builtin::Extension::kF16);

    GlobalVar("a", ty.f16(), builtin::AddressSpace::kPrivate);
    GlobalVar("b", ty.vec3<f16>(), builtin::AddressSpace::kPrivate);

    auto* expr = create<ast::BinaryExpression>(ast::BinaryOp::kModulo, Expr("a"), Expr("b"));
    WrapInFunction(expr);

    GeneratorImpl& gen = Build();

    utils::StringStream out;
    gen.EmitExpression(out, expr);
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_EQ(out.str(), "tint_float_modulo(a, b)");
}

TEST_F(GlslGeneratorImplTest_Binary, ModMixedVec3ScalarF32) {
    GlobalVar("a", ty.vec3<f32>(), builtin::AddressSpace::kPrivate);
    GlobalVar("b", ty.f32(), builtin::AddressSpace::kPrivate);

    auto* expr_vec_mod_vec =
        create<ast::BinaryExpression>(ast::BinaryOp::kModulo, Expr("a"), Expr("a"));
    auto* expr_vec_mod_scalar =
        create<ast::BinaryExpression>(ast::BinaryOp::kModulo, Expr("a"), Expr("b"));
    auto* expr_scalar_mod_vec =
        create<ast::BinaryExpression>(ast::BinaryOp::kModulo, Expr("b"), Expr("a"));
    WrapInFunction(expr_vec_mod_vec, expr_vec_mod_scalar, expr_scalar_mod_vec);

    GeneratorImpl& gen = Build();

    gen.Generate();
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_EQ(gen.result(), R"(#version 310 es

vec3 tint_float_modulo(vec3 lhs, vec3 rhs) {
  return (lhs - rhs * trunc(lhs / rhs));
}

vec3 tint_float_modulo_1(vec3 lhs, float rhs) {
  return (lhs - rhs * trunc(lhs / rhs));
}

vec3 tint_float_modulo_2(float lhs, vec3 rhs) {
  return (lhs - rhs * trunc(lhs / rhs));
}


vec3 a = vec3(0.0f, 0.0f, 0.0f);
float b = 0.0f;
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void test_function() {
  vec3 tint_symbol = tint_float_modulo(a, a);
  vec3 tint_symbol_1 = tint_float_modulo_1(a, b);
  vec3 tint_symbol_2 = tint_float_modulo_2(b, a);
  return;
}
)");
}

TEST_F(GlslGeneratorImplTest_Binary, ModMixedVec3ScalarF16) {
    Enable(builtin::Extension::kF16);

    GlobalVar("a", ty.vec3<f16>(), builtin::AddressSpace::kPrivate);
    GlobalVar("b", ty.f16(), builtin::AddressSpace::kPrivate);

    auto* expr_vec_mod_vec =
        create<ast::BinaryExpression>(ast::BinaryOp::kModulo, Expr("a"), Expr("a"));
    auto* expr_vec_mod_scalar =
        create<ast::BinaryExpression>(ast::BinaryOp::kModulo, Expr("a"), Expr("b"));
    auto* expr_scalar_mod_vec =
        create<ast::BinaryExpression>(ast::BinaryOp::kModulo, Expr("b"), Expr("a"));
    WrapInFunction(expr_vec_mod_vec, expr_vec_mod_scalar, expr_scalar_mod_vec);

    GeneratorImpl& gen = Build();

    gen.Generate();
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_EQ(gen.result(), R"(#version 310 es
#extension GL_AMD_gpu_shader_half_float : require

f16vec3 tint_float_modulo(f16vec3 lhs, f16vec3 rhs) {
  return (lhs - rhs * trunc(lhs / rhs));
}

f16vec3 tint_float_modulo_1(f16vec3 lhs, float16_t rhs) {
  return (lhs - rhs * trunc(lhs / rhs));
}

f16vec3 tint_float_modulo_2(float16_t lhs, f16vec3 rhs) {
  return (lhs - rhs * trunc(lhs / rhs));
}


f16vec3 a = f16vec3(0.0hf, 0.0hf, 0.0hf);
float16_t b = 0.0hf;
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void test_function() {
  f16vec3 tint_symbol = tint_float_modulo(a, a);
  f16vec3 tint_symbol_1 = tint_float_modulo_1(a, b);
  f16vec3 tint_symbol_2 = tint_float_modulo_2(b, a);
  return;
}
)");
}

TEST_F(GlslGeneratorImplTest_Binary, Logical_And) {
    GlobalVar("a", ty.bool_(), builtin::AddressSpace::kPrivate);
    GlobalVar("b", ty.bool_(), builtin::AddressSpace::kPrivate);

    auto* expr = create<ast::BinaryExpression>(ast::BinaryOp::kLogicalAnd, Expr("a"), Expr("b"));
    WrapInFunction(expr);

    GeneratorImpl& gen = Build();

    utils::StringStream out;
    gen.EmitExpression(out, expr);
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_EQ(out.str(), "(tint_tmp)");
    EXPECT_EQ(gen.result(), R"(bool tint_tmp = a;
if (tint_tmp) {
  tint_tmp = b;
}
)");
}

TEST_F(GlslGeneratorImplTest_Binary, Logical_Multi) {
    // (a && b) || (c || d)
    GlobalVar("a", ty.bool_(), builtin::AddressSpace::kPrivate);
    GlobalVar("b", ty.bool_(), builtin::AddressSpace::kPrivate);
    GlobalVar("c", ty.bool_(), builtin::AddressSpace::kPrivate);
    GlobalVar("d", ty.bool_(), builtin::AddressSpace::kPrivate);

    auto* expr = create<ast::BinaryExpression>(
        ast::BinaryOp::kLogicalOr,
        create<ast::BinaryExpression>(ast::BinaryOp::kLogicalAnd, Expr("a"), Expr("b")),
        create<ast::BinaryExpression>(ast::BinaryOp::kLogicalOr, Expr("c"), Expr("d")));
    WrapInFunction(expr);

    GeneratorImpl& gen = Build();

    utils::StringStream out;
    gen.EmitExpression(out, expr);
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_EQ(out.str(), "(tint_tmp)");
    EXPECT_EQ(gen.result(), R"(bool tint_tmp_1 = a;
if (tint_tmp_1) {
  tint_tmp_1 = b;
}
bool tint_tmp = (tint_tmp_1);
if (!tint_tmp) {
  bool tint_tmp_2 = c;
  if (!tint_tmp_2) {
    tint_tmp_2 = d;
  }
  tint_tmp = (tint_tmp_2);
}
)");
}

TEST_F(GlslGeneratorImplTest_Binary, Logical_Or) {
    GlobalVar("a", ty.bool_(), builtin::AddressSpace::kPrivate);
    GlobalVar("b", ty.bool_(), builtin::AddressSpace::kPrivate);

    auto* expr = create<ast::BinaryExpression>(ast::BinaryOp::kLogicalOr, Expr("a"), Expr("b"));
    WrapInFunction(expr);

    GeneratorImpl& gen = Build();

    utils::StringStream out;
    gen.EmitExpression(out, expr);
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_EQ(out.str(), "(tint_tmp)");
    EXPECT_EQ(gen.result(), R"(bool tint_tmp = a;
if (!tint_tmp) {
  tint_tmp = b;
}
)");
}

TEST_F(GlslGeneratorImplTest_Binary, If_WithLogical) {
    // if (a && b) {
    //   return 1i;
    // } else if (b || c) {
    //   return 2i;
    // } else {
    //   return 3i;
    // }

    GlobalVar("a", ty.bool_(), builtin::AddressSpace::kPrivate);
    GlobalVar("b", ty.bool_(), builtin::AddressSpace::kPrivate);
    GlobalVar("c", ty.bool_(), builtin::AddressSpace::kPrivate);

    auto* expr =
        If(create<ast::BinaryExpression>(ast::BinaryOp::kLogicalAnd, Expr("a"), Expr("b")),
           Block(Return(1_i)),
           Else(If(create<ast::BinaryExpression>(ast::BinaryOp::kLogicalOr, Expr("b"), Expr("c")),
                   Block(Return(2_i)), Else(Block(Return(3_i))))));
    Func("func", utils::Empty, ty.i32(), utils::Vector{WrapInStatement(expr)});

    GeneratorImpl& gen = Build();

    gen.EmitStatement(expr);
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_EQ(gen.result(), R"(bool tint_tmp = a;
if (tint_tmp) {
  tint_tmp = b;
}
if ((tint_tmp)) {
  return 1;
} else {
  bool tint_tmp_1 = b;
  if (!tint_tmp_1) {
    tint_tmp_1 = c;
  }
  if ((tint_tmp_1)) {
    return 2;
  } else {
    return 3;
  }
}
)");
}

TEST_F(GlslGeneratorImplTest_Binary, Return_WithLogical) {
    // return (a && b) || c;

    GlobalVar("a", ty.bool_(), builtin::AddressSpace::kPrivate);
    GlobalVar("b", ty.bool_(), builtin::AddressSpace::kPrivate);
    GlobalVar("c", ty.bool_(), builtin::AddressSpace::kPrivate);

    auto* expr = Return(create<ast::BinaryExpression>(
        ast::BinaryOp::kLogicalOr,
        create<ast::BinaryExpression>(ast::BinaryOp::kLogicalAnd, Expr("a"), Expr("b")),
        Expr("c")));
    Func("func", utils::Empty, ty.bool_(), utils::Vector{WrapInStatement(expr)});

    GeneratorImpl& gen = Build();

    gen.EmitStatement(expr);
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_EQ(gen.result(), R"(bool tint_tmp_1 = a;
if (tint_tmp_1) {
  tint_tmp_1 = b;
}
bool tint_tmp = (tint_tmp_1);
if (!tint_tmp) {
  tint_tmp = c;
}
return (tint_tmp);
)");
}

TEST_F(GlslGeneratorImplTest_Binary, Assign_WithLogical) {
    // a = (b || c) && d;

    GlobalVar("a", ty.bool_(), builtin::AddressSpace::kPrivate);
    GlobalVar("b", ty.bool_(), builtin::AddressSpace::kPrivate);
    GlobalVar("c", ty.bool_(), builtin::AddressSpace::kPrivate);
    GlobalVar("d", ty.bool_(), builtin::AddressSpace::kPrivate);

    auto* expr =
        Assign(Expr("a"),
               create<ast::BinaryExpression>(
                   ast::BinaryOp::kLogicalAnd,
                   create<ast::BinaryExpression>(ast::BinaryOp::kLogicalOr, Expr("b"), Expr("c")),
                   Expr("d")));
    WrapInFunction(expr);

    GeneratorImpl& gen = Build();

    gen.EmitStatement(expr);
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_EQ(gen.result(), R"(bool tint_tmp_1 = b;
if (!tint_tmp_1) {
  tint_tmp_1 = c;
}
bool tint_tmp = (tint_tmp_1);
if (tint_tmp) {
  tint_tmp = d;
}
a = (tint_tmp);
)");
}

TEST_F(GlslGeneratorImplTest_Binary, Decl_WithLogical) {
    // var a : bool = (b && c) || d;

    GlobalVar("b", ty.bool_(), builtin::AddressSpace::kPrivate);
    GlobalVar("c", ty.bool_(), builtin::AddressSpace::kPrivate);
    GlobalVar("d", ty.bool_(), builtin::AddressSpace::kPrivate);

    auto* var =
        Var("a", ty.bool_(),
            create<ast::BinaryExpression>(
                ast::BinaryOp::kLogicalOr,
                create<ast::BinaryExpression>(ast::BinaryOp::kLogicalAnd, Expr("b"), Expr("c")),
                Expr("d")));

    auto* decl = Decl(var);
    WrapInFunction(decl);

    GeneratorImpl& gen = Build();

    gen.EmitStatement(decl);
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_EQ(gen.result(), R"(bool tint_tmp_1 = b;
if (tint_tmp_1) {
  tint_tmp_1 = c;
}
bool tint_tmp = (tint_tmp_1);
if (!tint_tmp) {
  tint_tmp = d;
}
bool a = (tint_tmp);
)");
}

TEST_F(GlslGeneratorImplTest_Binary, Call_WithLogical) {
    // foo(a && b, c || d, (a || c) && (b || d))

    Func("foo",
         utils::Vector{
             Param(Sym(), ty.bool_()),
             Param(Sym(), ty.bool_()),
             Param(Sym(), ty.bool_()),
         },
         ty.void_(), utils::Empty, utils::Empty);
    GlobalVar("a", ty.bool_(), builtin::AddressSpace::kPrivate);
    GlobalVar("b", ty.bool_(), builtin::AddressSpace::kPrivate);
    GlobalVar("c", ty.bool_(), builtin::AddressSpace::kPrivate);
    GlobalVar("d", ty.bool_(), builtin::AddressSpace::kPrivate);

    utils::Vector params{
        create<ast::BinaryExpression>(ast::BinaryOp::kLogicalAnd, Expr("a"), Expr("b")),
        create<ast::BinaryExpression>(ast::BinaryOp::kLogicalOr, Expr("c"), Expr("d")),
        create<ast::BinaryExpression>(
            ast::BinaryOp::kLogicalAnd,
            create<ast::BinaryExpression>(ast::BinaryOp::kLogicalOr, Expr("a"), Expr("c")),
            create<ast::BinaryExpression>(ast::BinaryOp::kLogicalOr, Expr("b"), Expr("d"))),
    };

    auto* expr = CallStmt(Call("foo", params));
    WrapInFunction(expr);

    GeneratorImpl& gen = Build();

    gen.EmitStatement(expr);
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_EQ(gen.result(), R"(bool tint_tmp = a;
if (tint_tmp) {
  tint_tmp = b;
}
bool tint_tmp_1 = c;
if (!tint_tmp_1) {
  tint_tmp_1 = d;
}
bool tint_tmp_3 = a;
if (!tint_tmp_3) {
  tint_tmp_3 = c;
}
bool tint_tmp_2 = (tint_tmp_3);
if (tint_tmp_2) {
  bool tint_tmp_4 = b;
  if (!tint_tmp_4) {
    tint_tmp_4 = d;
  }
  tint_tmp_2 = (tint_tmp_4);
}
foo((tint_tmp), (tint_tmp_1), (tint_tmp_2));
)");
}

}  // namespace
}  // namespace tint::writer::glsl
