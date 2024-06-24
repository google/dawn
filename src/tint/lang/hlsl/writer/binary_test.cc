// Copyright 2024 The Dawn & Tint Authors
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

#include "src/tint/lang/core/fluent_types.h"
#include "src/tint/lang/hlsl/writer/helper_test.h"
#include "src/tint/utils/text/string_stream.h"

using namespace tint::core::number_suffixes;  // NOLINT
using namespace tint::core::fluent_types;     // NOLINT

namespace tint::hlsl::writer {
namespace {

struct BinaryData {
    const char* result;
    core::BinaryOp op;
};
inline std::ostream& operator<<(std::ostream& out, BinaryData data) {
    StringStream str;
    str << data.op;
    out << str.str();
    return out;
}

using HlslWriterBinaryU32Test = HlslWriterTestWithParam<BinaryData>;
TEST_P(HlslWriterBinaryU32Test, Emit) {
    auto params = GetParam();

    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kCompute);
    func->SetWorkgroupSize(1, 1, 1);
    b.Append(func->Block(), [&] {
        auto* l = b.Let("left", b.Constant(1_u));
        auto* r = b.Let("right", b.Constant(2_u));
        auto* bin = b.Binary(params.op, ty.u32(), l, r);
        b.Let("val", bin);
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.hlsl;
    EXPECT_EQ(output_.hlsl, R"(
[numthreads(1, 1, 1)]
void foo() {
  uint left = 1u;
  uint right = 2u;
  uint val = )" + std::string(params.result) +
                                R"(;
}

)");
}
INSTANTIATE_TEST_SUITE_P(HlslWriterTest,
                         HlslWriterBinaryU32Test,
                         testing::Values(BinaryData{"(left + right)", core::BinaryOp::kAdd},
                                         BinaryData{"(left - right)", core::BinaryOp::kSubtract},
                                         BinaryData{"(left * right)", core::BinaryOp::kMultiply},
                                         BinaryData{"(left & right)", core::BinaryOp::kAnd},
                                         BinaryData{"(left | right)", core::BinaryOp::kOr},
                                         BinaryData{"(left ^ right)", core::BinaryOp::kXor}));

TEST_F(HlslWriterTest, BinaryU32Div) {
    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kCompute);
    func->SetWorkgroupSize(1, 1, 1);
    b.Append(func->Block(), [&] {
        auto* l = b.Let("left", b.Constant(1_u));
        auto* r = b.Let("right", b.Constant(2_u));
        auto* bin = b.Binary(core::BinaryOp::kDivide, ty.u32(), l, r);
        b.Let("val", bin);
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.hlsl;
    EXPECT_EQ(output_.hlsl, R"(
uint tint_div_u32(uint lhs, uint rhs) {
  return (lhs / (((rhs == 0u)) ? (1u) : (rhs)));
}

[numthreads(1, 1, 1)]
void foo() {
  uint left = 1u;
  uint right = 2u;
  uint val = tint_div_u32(left, right);
}

)");
}

TEST_F(HlslWriterTest, BinaryU32Mod) {
    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kCompute);
    func->SetWorkgroupSize(1, 1, 1);
    b.Append(func->Block(), [&] {
        auto* l = b.Let("left", b.Constant(1_u));
        auto* r = b.Let("right", b.Constant(2_u));
        auto* bin = b.Binary(core::BinaryOp::kModulo, ty.u32(), l, r);
        b.Let("val", bin);
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.hlsl;
    EXPECT_EQ(output_.hlsl, R"(
uint tint_mod_u32(uint lhs, uint rhs) {
  return (lhs - ((lhs / (((rhs == 0u)) ? (1u) : (rhs))) * (((rhs == 0u)) ? (1u) : (rhs))));
}

[numthreads(1, 1, 1)]
void foo() {
  uint left = 1u;
  uint right = 2u;
  uint val = tint_mod_u32(left, right);
}

)");
}

TEST_F(HlslWriterTest, BinaryU32ShiftLeft) {
    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kCompute);
    func->SetWorkgroupSize(1, 1, 1);
    b.Append(func->Block(), [&] {
        auto* l = b.Let("left", b.Constant(1_u));
        auto* r = b.Let("right", b.Constant(2_u));
        auto* bin = b.Binary(core::BinaryOp::kShiftLeft, ty.u32(), l, r);
        b.Let("val", bin);
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.hlsl;
    EXPECT_EQ(output_.hlsl, R"(
[numthreads(1, 1, 1)]
void foo() {
  uint left = 1u;
  uint right = 2u;
  uint val = (left << (right & 31u));
}

)");
}

TEST_F(HlslWriterTest, BinaryU32ShiftRight) {
    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kCompute);
    func->SetWorkgroupSize(1, 1, 1);
    b.Append(func->Block(), [&] {
        auto* l = b.Let("left", b.Constant(1_u));
        auto* r = b.Let("right", b.Constant(2_u));
        auto* bin = b.Binary(core::BinaryOp::kShiftRight, ty.u32(), l, r);
        b.Let("val", bin);
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.hlsl;
    EXPECT_EQ(output_.hlsl, R"(
[numthreads(1, 1, 1)]
void foo() {
  uint left = 1u;
  uint right = 2u;
  uint val = (left >> (right & 31u));
}

)");
}

using HlslWriterBinaryBoolTest = HlslWriterTestWithParam<BinaryData>;
TEST_P(HlslWriterBinaryBoolTest, Emit) {
    auto params = GetParam();

    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kCompute);
    func->SetWorkgroupSize(1, 1, 1);
    b.Append(func->Block(), [&] {
        auto* l = b.Let("left", b.Constant(1_u));
        auto* r = b.Let("right", b.Constant(2_u));
        auto* bin = b.Binary(params.op, ty.bool_(), l, r);
        b.Let("val", bin);
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.hlsl;
    EXPECT_EQ(output_.hlsl, R"(
[numthreads(1, 1, 1)]
void foo() {
  uint left = 1u;
  uint right = 2u;
  bool val = )" + std::string(params.result) +
                                R"(;
}

)");
}
INSTANTIATE_TEST_SUITE_P(
    HlslWriterTest,
    HlslWriterBinaryBoolTest,
    testing::Values(BinaryData{"(left == right)", core::BinaryOp::kEqual},
                    BinaryData{"(left != right)", core::BinaryOp::kNotEqual},
                    BinaryData{"(left < right)", core::BinaryOp::kLessThan},
                    BinaryData{"(left > right)", core::BinaryOp::kGreaterThan},
                    BinaryData{"(left <= right)", core::BinaryOp::kLessThanEqual},
                    BinaryData{"(left >= right)", core::BinaryOp::kGreaterThanEqual}));

// TODO(dsinclair): Needs binary polyfill
TEST_F(HlslWriterTest, DISABLED_BinaryF32Mod) {
    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kCompute);
    func->SetWorkgroupSize(1, 1, 1);
    b.Append(func->Block(), [&] {
        auto* left = b.Var("left", ty.ptr<core::AddressSpace::kFunction, f32>());
        auto* right = b.Var("right", ty.ptr<core::AddressSpace::kFunction, f32>());

        auto* l = b.Load(left);
        auto* r = b.Load(right);
        auto* expr1 = b.Binary(core::BinaryOp::kModulo, ty.f32(), l, r);

        b.Let("val", expr1);
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.hlsl;
    EXPECT_EQ(output_.hlsl, R"(
float tint_trunc(float param_0) {
  return param_0 < 0 ? ceil(param_0) : floor(param_0);
}

float tint_float_mod(float lhs, float rhs) {
  return (lhs - (tint_trunc((lhs / rhs)) * rhs));
}

[numthreads(1, 1, 1)]
void foo() {
  float left = 0.0f;
  float right = 0.0f;
  float const val = tint_float_mod(left, right);
}

)");
}

// TODO(dsinclair): Needs binary polyfill
TEST_F(HlslWriterTest, DISABLED_BinaryF16Mod) {
    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kCompute);
    func->SetWorkgroupSize(1, 1, 1);
    b.Append(func->Block(), [&] {
        auto* left = b.Var("left", ty.ptr<core::AddressSpace::kFunction, f16>());
        auto* right = b.Var("right", ty.ptr<core::AddressSpace::kFunction, f16>());

        auto* l = b.Load(left);
        auto* r = b.Load(right);
        auto* expr1 = b.Binary(core::BinaryOp::kModulo, ty.f16(), l, r);

        b.Let("val", expr1);
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.hlsl;
    EXPECT_EQ(output_.hlsl, R"(
float16_t tint_trunc(float16_t param_0) {
  return param_0 < 0 ? ceil(param_0) : floor(param_0);
}

float16_t tint_float_mod(float16_t lhs, float16_t rhs) {
  return (lhs - (tint_trunc((lhs / rhs)) * rhs));
}

[numthreads(1, 1, 1)]
void foo() {
  float16_t left = float16_t(0.0h);
  float16_t right = float16_t(0.0h);
  float16_t const val = tint_float_mod(left, right);
}

)");
}

// TODO(dsinclair): Needs binary polyfill
TEST_F(HlslWriterTest, DISABLED_BinaryF32ModVec3) {
    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kCompute);
    func->SetWorkgroupSize(1, 1, 1);
    b.Append(func->Block(), [&] {
        auto* left = b.Var("left", ty.ptr(core::AddressSpace::kFunction, ty.vec3<f32>()));
        auto* right = b.Var("right", ty.ptr(core::AddressSpace::kFunction, ty.vec3<f32>()));

        auto* l = b.Load(left);
        auto* r = b.Load(right);
        auto* expr1 = b.Binary(core::BinaryOp::kModulo, ty.vec3<f32>(), l, r);

        b.Let("val", expr1);
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.hlsl;
    EXPECT_EQ(output_.hlsl, R"(
float3 tint_trunc(float3 param_0) {
  return param_0 < 0 ? ceil(param_0) : floor(param_0);
}

float3 tint_float_mod(float3 lhs, float3 rhs) {
  return (lhs - (tint_trunc((lhs / rhs)) * rhs));
}

[numthreads(1, 1, 1)]
void foo() {
  float3 left = (0.0f).xxx;
  float3 right = (0.0f).xxx;
  float3 const val = tint_float_mod(left, right);
}

)");
}

// TODO(dsinclair): Needs binary polyfill
TEST_F(HlslWriterTest, DISABLED_BinaryF16ModVec3) {
    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kCompute);
    func->SetWorkgroupSize(1, 1, 1);
    b.Append(func->Block(), [&] {
        auto* left = b.Var("left", ty.ptr(core::AddressSpace::kFunction, ty.vec3<f16>()));
        auto* right = b.Var("right", ty.ptr(core::AddressSpace::kFunction, ty.vec3<f16>()));

        auto* l = b.Load(left);
        auto* r = b.Load(right);
        auto* expr1 = b.Binary(core::BinaryOp::kModulo, ty.vec3<f16>(), l, r);

        b.Let("val", expr1);
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.hlsl;
    EXPECT_EQ(output_.hlsl, R"(
vector<float16_t, 3> tint_trunc(vector<float16_t, 3> param_0) {
  return param_0 < 0 ? ceil(param_0) : floor(param_0);
}

vector<float16_t, 3> tint_float_mod(vector<float16_t, 3> lhs, vector<float16_t, 3> rhs) {
  return (lhs - (tint_trunc((lhs / rhs)) * rhs));
}

[numthreads(1, 1, 1)]
void foo() {
  vector<float16_t, 3> left = (float16_t(0.0h)).xxx;
  vector<float16_t, 3> right = (float16_t(0.0h)).xxx;
  vector<float16_t, 3> const val = tint_float_mod(left, right);
}

)");
}

TEST_F(HlslWriterTest, BinaryBoolAnd) {
    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kCompute);
    func->SetWorkgroupSize(1, 1, 1);
    b.Append(func->Block(), [&] {
        auto* left = b.Var("left", ty.ptr(core::AddressSpace::kFunction, ty.bool_()));
        auto* right = b.Var("right", ty.ptr(core::AddressSpace::kFunction, ty.bool_()));

        auto* l = b.Load(left);
        auto* r = b.Load(right);
        auto* expr1 = b.Binary(core::BinaryOp::kAnd, ty.bool_(), l, r);

        b.Let("val", expr1);
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.hlsl;
    EXPECT_EQ(output_.hlsl, R"(
[numthreads(1, 1, 1)]
void foo() {
  bool left = false;
  bool right = false;
  bool val = (left & right);
}

)");
}

TEST_F(HlslWriterTest, BinaryBoolOr) {
    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kCompute);
    func->SetWorkgroupSize(1, 1, 1);
    b.Append(func->Block(), [&] {
        auto* left = b.Var("left", ty.ptr(core::AddressSpace::kFunction, ty.bool_()));
        auto* right = b.Var("right", ty.ptr(core::AddressSpace::kFunction, ty.bool_()));

        auto* l = b.Load(left);
        auto* r = b.Load(right);
        auto* expr1 = b.Binary(core::BinaryOp::kOr, ty.bool_(), l, r);

        b.Let("val", expr1);
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.hlsl;
    EXPECT_EQ(output_.hlsl, R"(
[numthreads(1, 1, 1)]
void foo() {
  bool left = false;
  bool right = false;
  bool val = (left | right);
}

)");
}

// TODO(dsinclair): Needs binary polyfill
TEST_F(HlslWriterTest, DISABLED_BinaryMulMatVec) {
    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kCompute);
    func->SetWorkgroupSize(1, 1, 1);
    b.Append(func->Block(), [&] {
        auto* x = b.Var("x", b.Zero<mat4x4<f32>>());
        auto* y = b.Var("y", b.Zero<vec4<f32>>());
        auto* l = b.Load(x);
        auto* r = b.Load(y);
        b.Var("c", b.Multiply(ty.vec4<f32>(), l, r));
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.hlsl;
    EXPECT_EQ(output_.hlsl, R"(
[numthreads(1, 1, 1)]
void foo() {
  float4x4 x = float4x4((0.0f).xxxx, (0.0f).xxxx, (0.0f).xxxx, (0.0f).xxxx);
  float4 y = (0.0f).xxxx;
  float4 c = mul(y, x);
}

)");
}

// TODO(dsinclair): Needs binary polyfill
TEST_F(HlslWriterTest, DISABLED_BinaryMulVecMat) {
    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kCompute);
    func->SetWorkgroupSize(1, 1, 1);
    b.Append(func->Block(), [&] {
        auto* x = b.Var("x", b.Zero<mat4x4<f32>>());
        auto* y = b.Var("y", b.Zero<vec4<f32>>());
        auto* l = b.Load(x);
        auto* r = b.Load(y);
        b.Var("c", b.Multiply(ty.vec4<f32>(), r, l));
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.hlsl;
    EXPECT_EQ(output_.hlsl, R"(
[numthreads(1, 1, 1)]
void foo() {
  float4x4 x = float4x4((0.0f).xxxx, (0.0f).xxxx, (0.0f).xxxx, (0.0f).xxxx);
  float4 y = (0.0f).xxxx;
  float4 c = mul(x, y);
}

)");
}

// TODO(dsinclair): Needs binary polyfill
TEST_F(HlslWriterTest, DISABLED_BinaryMulMatMat) {
    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kCompute);
    func->SetWorkgroupSize(1, 1, 1);
    b.Append(func->Block(), [&] {
        auto* x = b.Var("x", b.Zero<mat4x4<f32>>());
        auto* y = b.Var("y", b.Zero<mat4x4<f32>>());
        auto* l = b.Load(x);
        auto* r = b.Load(y);
        b.Var("c", b.Multiply(ty.mat4x4<f32>(), l, r));
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.hlsl;
    EXPECT_EQ(output_.hlsl, R"(
[numthreads(1, 1, 1)]
void foo() {
  float4x4 x = float4x4((0.0f).xxxx, (0.0f).xxxx, (0.0f).xxxx, (0.0f).xxxx);
  float4x4 y = float4x4((0.0f).xxxx, (0.0f).xxxx, (0.0f).xxxx, (0.0f).xxxx);
  float4 c = mul(y, x);
}

)");
}

}  // namespace
}  // namespace tint::hlsl::writer
