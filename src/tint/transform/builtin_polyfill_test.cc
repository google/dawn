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

#include "src/tint/transform/builtin_polyfill.h"

#include <utility>

#include "src/tint/transform/test_helper.h"

namespace tint::transform {
namespace {

using Level = BuiltinPolyfill::Level;

using BuiltinPolyfillTest = TransformTest;

TEST_F(BuiltinPolyfillTest, ShouldRunEmptyModule) {
    auto* src = R"()";

    EXPECT_FALSE(ShouldRun<BuiltinPolyfill>(src));
}

TEST_F(BuiltinPolyfillTest, EmptyModule) {
    auto* src = R"()";

    auto* expect = src;

    auto got = Run<BuiltinPolyfill>(src);

    EXPECT_EQ(expect, str(got));
}

////////////////////////////////////////////////////////////////////////////////
// acosh
////////////////////////////////////////////////////////////////////////////////
DataMap polyfillAcosh(Level level) {
    BuiltinPolyfill::Builtins builtins;
    builtins.acosh = level;
    DataMap data;
    data.Add<BuiltinPolyfill::Config>(builtins);
    return data;
}

TEST_F(BuiltinPolyfillTest, ShouldRunAcosh) {
    auto* src = R"(
fn f() {
  let v = 1.0;
  acosh(v);
}
)";

    EXPECT_FALSE(ShouldRun<BuiltinPolyfill>(src));
    EXPECT_FALSE(ShouldRun<BuiltinPolyfill>(src, polyfillAcosh(Level::kNone)));
    EXPECT_TRUE(ShouldRun<BuiltinPolyfill>(src, polyfillAcosh(Level::kRangeCheck)));
    EXPECT_TRUE(ShouldRun<BuiltinPolyfill>(src, polyfillAcosh(Level::kFull)));
}

TEST_F(BuiltinPolyfillTest, Acosh_ConstantExpression) {
    auto* src = R"(
fn f() {
  let r : f32 = acosh(1.0);
}
)";

    auto* expect = src;

    auto got = Run<BuiltinPolyfill>(src, polyfillAcosh(Level::kFull));

    EXPECT_EQ(expect, str(got));
}

TEST_F(BuiltinPolyfillTest, Acosh_Full_f32) {
    auto* src = R"(
fn f() {
  let v = 1.0;
  let r : f32 = acosh(v);
}
)";

    auto* expect = R"(
fn tint_acosh(x : f32) -> f32 {
  return log((x + sqrt(((x * x) - 1))));
}

fn f() {
  let v = 1.0;
  let r : f32 = tint_acosh(v);
}
)";

    auto got = Run<BuiltinPolyfill>(src, polyfillAcosh(Level::kFull));

    EXPECT_EQ(expect, str(got));
}

TEST_F(BuiltinPolyfillTest, Acosh_Full_vec3_f32) {
    auto* src = R"(
fn f() {
  let v = 1.0;
  let r : vec3<f32> = acosh(vec3<f32>(v));
}
)";

    auto* expect = R"(
fn tint_acosh(x : vec3<f32>) -> vec3<f32> {
  return log((x + sqrt(((x * x) - 1))));
}

fn f() {
  let v = 1.0;
  let r : vec3<f32> = tint_acosh(vec3<f32>(v));
}
)";

    auto got = Run<BuiltinPolyfill>(src, polyfillAcosh(Level::kFull));

    EXPECT_EQ(expect, str(got));
}

TEST_F(BuiltinPolyfillTest, Acosh_Range_f32) {
    auto* src = R"(
fn f() {
  let v = 1.0;
  let r : f32 = acosh(v);
}
)";

    auto* expect = R"(
fn tint_acosh(x : f32) -> f32 {
  return select(acosh(x), 0.0, (x < 1.0));
}

fn f() {
  let v = 1.0;
  let r : f32 = tint_acosh(v);
}
)";

    auto got = Run<BuiltinPolyfill>(src, polyfillAcosh(Level::kRangeCheck));

    EXPECT_EQ(expect, str(got));
}

TEST_F(BuiltinPolyfillTest, Acosh_Range_vec3_f32) {
    auto* src = R"(
fn f() {
  let v = 1.0;
  let r : vec3<f32> = acosh(vec3<f32>(v));
}
)";

    auto* expect = R"(
fn tint_acosh(x : vec3<f32>) -> vec3<f32> {
  return select(acosh(x), vec3<f32>(0.0), (x < vec3<f32>(1.0)));
}

fn f() {
  let v = 1.0;
  let r : vec3<f32> = tint_acosh(vec3<f32>(v));
}
)";

    auto got = Run<BuiltinPolyfill>(src, polyfillAcosh(Level::kRangeCheck));

    EXPECT_EQ(expect, str(got));
}

////////////////////////////////////////////////////////////////////////////////
// asinh
////////////////////////////////////////////////////////////////////////////////
DataMap polyfillSinh() {
    BuiltinPolyfill::Builtins builtins;
    builtins.asinh = true;
    DataMap data;
    data.Add<BuiltinPolyfill::Config>(builtins);
    return data;
}

TEST_F(BuiltinPolyfillTest, ShouldRunAsinh) {
    auto* src = R"(
fn f() {
  let v = 1.0;
  asinh(v);
}
)";

    EXPECT_FALSE(ShouldRun<BuiltinPolyfill>(src));
    EXPECT_TRUE(ShouldRun<BuiltinPolyfill>(src, polyfillSinh()));
}

TEST_F(BuiltinPolyfillTest, Asinh_ConstantExpression) {
    auto* src = R"(
fn f() {
  let r : f32 = asinh(1.0);
}
)";

    auto* expect = src;

    auto got = Run<BuiltinPolyfill>(src, polyfillSinh());

    EXPECT_EQ(expect, str(got));
}

TEST_F(BuiltinPolyfillTest, Asinh_f32) {
    auto* src = R"(
fn f() {
  let v = 1.0;
  let r : f32 = asinh(v);
}
)";

    auto* expect = R"(
fn tint_sinh(x : f32) -> f32 {
  return log((x + sqrt(((x * x) + 1))));
}

fn f() {
  let v = 1.0;
  let r : f32 = tint_sinh(v);
}
)";

    auto got = Run<BuiltinPolyfill>(src, polyfillSinh());

    EXPECT_EQ(expect, str(got));
}

TEST_F(BuiltinPolyfillTest, Asinh_vec3_f32) {
    auto* src = R"(
fn f() {
  let v = 1.0;
  let r : vec3<f32> = asinh(vec3<f32>(v));
}
)";

    auto* expect = R"(
fn tint_sinh(x : vec3<f32>) -> vec3<f32> {
  return log((x + sqrt(((x * x) + 1))));
}

fn f() {
  let v = 1.0;
  let r : vec3<f32> = tint_sinh(vec3<f32>(v));
}
)";

    auto got = Run<BuiltinPolyfill>(src, polyfillSinh());

    EXPECT_EQ(expect, str(got));
}

////////////////////////////////////////////////////////////////////////////////
// atanh
////////////////////////////////////////////////////////////////////////////////
DataMap polyfillAtanh(Level level) {
    BuiltinPolyfill::Builtins builtins;
    builtins.atanh = level;
    DataMap data;
    data.Add<BuiltinPolyfill::Config>(builtins);
    return data;
}

TEST_F(BuiltinPolyfillTest, ShouldRunAtanh) {
    auto* src = R"(
fn f() {
  let v = 1.0;
  atanh(v);
}
)";

    EXPECT_FALSE(ShouldRun<BuiltinPolyfill>(src));
    EXPECT_FALSE(ShouldRun<BuiltinPolyfill>(src, polyfillAtanh(Level::kNone)));
    EXPECT_TRUE(ShouldRun<BuiltinPolyfill>(src, polyfillAtanh(Level::kRangeCheck)));
    EXPECT_TRUE(ShouldRun<BuiltinPolyfill>(src, polyfillAtanh(Level::kFull)));
}

TEST_F(BuiltinPolyfillTest, Atanh_ConstantExpression) {
    auto* src = R"(
fn f() {
  let r : f32 = atanh(0.23);
}
)";

    auto* expect = src;

    auto got = Run<BuiltinPolyfill>(src, polyfillAtanh(Level::kFull));

    EXPECT_EQ(expect, str(got));
}

TEST_F(BuiltinPolyfillTest, Atanh_Full_f32) {
    auto* src = R"(
fn f() {
  let v = 1.0;
  let r : f32 = atanh(v);
}
)";

    auto* expect = R"(
fn tint_atanh(x : f32) -> f32 {
  return (log(((1 + x) / (1 - x))) * 0.5);
}

fn f() {
  let v = 1.0;
  let r : f32 = tint_atanh(v);
}
)";

    auto got = Run<BuiltinPolyfill>(src, polyfillAtanh(Level::kFull));

    EXPECT_EQ(expect, str(got));
}

TEST_F(BuiltinPolyfillTest, Atanh_Full_vec3_f32) {
    auto* src = R"(
fn f() {
  let v = 1.0;
  let r : vec3<f32> = atanh(vec3<f32>(v));
}
)";

    auto* expect = R"(
fn tint_atanh(x : vec3<f32>) -> vec3<f32> {
  return (log(((1 + x) / (1 - x))) * 0.5);
}

fn f() {
  let v = 1.0;
  let r : vec3<f32> = tint_atanh(vec3<f32>(v));
}
)";

    auto got = Run<BuiltinPolyfill>(src, polyfillAtanh(Level::kFull));

    EXPECT_EQ(expect, str(got));
}

TEST_F(BuiltinPolyfillTest, Atanh_Range_f32) {
    auto* src = R"(
fn f() {
  let v = 1.0;
  let r : f32 = atanh(v);
}
)";

    auto* expect = R"(
fn tint_atanh(x : f32) -> f32 {
  return select(atanh(x), 0.0, (x >= 1.0));
}

fn f() {
  let v = 1.0;
  let r : f32 = tint_atanh(v);
}
)";

    auto got = Run<BuiltinPolyfill>(src, polyfillAtanh(Level::kRangeCheck));

    EXPECT_EQ(expect, str(got));
}

TEST_F(BuiltinPolyfillTest, Atanh_Range_vec3_f32) {
    auto* src = R"(
fn f() {
  let v = 1.0;
  let r : vec3<f32> = atanh(vec3<f32>(v));
}
)";

    auto* expect = R"(
fn tint_atanh(x : vec3<f32>) -> vec3<f32> {
  return select(atanh(x), vec3<f32>(0.0), (x >= vec3<f32>(1.0)));
}

fn f() {
  let v = 1.0;
  let r : vec3<f32> = tint_atanh(vec3<f32>(v));
}
)";

    auto got = Run<BuiltinPolyfill>(src, polyfillAtanh(Level::kRangeCheck));

    EXPECT_EQ(expect, str(got));
}

////////////////////////////////////////////////////////////////////////////////
// bitshiftModulo
////////////////////////////////////////////////////////////////////////////////
DataMap polyfillBitshiftModulo() {
    BuiltinPolyfill::Builtins builtins;
    builtins.bitshift_modulo = true;
    DataMap data;
    data.Add<BuiltinPolyfill::Config>(builtins);
    return data;
}

TEST_F(BuiltinPolyfillTest, ShouldRunBitshiftModulo_shl_scalar) {
    auto* src = R"(
fn f() {
  let v = 15u;
  let r = 1i << v;
}
)";

    EXPECT_FALSE(ShouldRun<BuiltinPolyfill>(src));
    EXPECT_TRUE(ShouldRun<BuiltinPolyfill>(src, polyfillBitshiftModulo()));
}

TEST_F(BuiltinPolyfillTest, ShouldRunBitshiftModulo_shl_vector) {
    auto* src = R"(
fn f() {
  let v = 15u;
  let r = vec3(1i) << vec3(v);
}
)";

    EXPECT_FALSE(ShouldRun<BuiltinPolyfill>(src));
    EXPECT_TRUE(ShouldRun<BuiltinPolyfill>(src, polyfillBitshiftModulo()));
}

TEST_F(BuiltinPolyfillTest, ShouldRunBitshiftModulo_shr_scalar) {
    auto* src = R"(
fn f() {
  let v = 15u;
  let r = 1i >> v;
}
)";

    EXPECT_FALSE(ShouldRun<BuiltinPolyfill>(src));
    EXPECT_TRUE(ShouldRun<BuiltinPolyfill>(src, polyfillBitshiftModulo()));
}

TEST_F(BuiltinPolyfillTest, ShouldRunBitshiftModulo_shr_vector) {
    auto* src = R"(
fn f() {
  let v = 15u;
  let r = vec3(1i) >> vec3(v);
}
)";

    EXPECT_FALSE(ShouldRun<BuiltinPolyfill>(src));
    EXPECT_TRUE(ShouldRun<BuiltinPolyfill>(src, polyfillBitshiftModulo()));
}

TEST_F(BuiltinPolyfillTest, BitshiftModulo_shl_scalar) {
    auto* src = R"(
fn f() {
  let v = 15u;
  let r = 1i << v;
}
)";

    auto* expect = R"(
fn f() {
  let v = 15u;
  let r = (1i << (v & 31));
}
)";

    auto got = Run<BuiltinPolyfill>(src, polyfillBitshiftModulo());

    EXPECT_EQ(expect, str(got));
}

TEST_F(BuiltinPolyfillTest, BitshiftModulo_shl_vector) {
    auto* src = R"(
fn f() {
  let v = 15u;
  let r = vec3(1i) << vec3(v);
}
)";

    auto* expect = R"(
fn f() {
  let v = 15u;
  let r = (vec3(1i) << (vec3(v) & vec3<u32>(31)));
}
)";

    auto got = Run<BuiltinPolyfill>(src, polyfillBitshiftModulo());

    EXPECT_EQ(expect, str(got));
}

TEST_F(BuiltinPolyfillTest, BitshiftModulo_shr_scalar) {
    auto* src = R"(
fn f() {
  let v = 15u;
  let r = 1i >> v;
}
)";

    auto* expect = R"(
fn f() {
  let v = 15u;
  let r = (1i >> (v & 31));
}
)";

    auto got = Run<BuiltinPolyfill>(src, polyfillBitshiftModulo());

    EXPECT_EQ(expect, str(got));
}

TEST_F(BuiltinPolyfillTest, BitshiftModulo_shr_vector) {
    auto* src = R"(
fn f() {
  let v = 15u;
  let r = vec3(1i) >> vec3(v);
}
)";

    auto* expect = R"(
fn f() {
  let v = 15u;
  let r = (vec3(1i) >> (vec3(v) & vec3<u32>(31)));
}
)";

    auto got = Run<BuiltinPolyfill>(src, polyfillBitshiftModulo());

    EXPECT_EQ(expect, str(got));
}

////////////////////////////////////////////////////////////////////////////////
// clampInteger
////////////////////////////////////////////////////////////////////////////////
DataMap polyfillClampInteger() {
    BuiltinPolyfill::Builtins builtins;
    builtins.clamp_int = true;
    DataMap data;
    data.Add<BuiltinPolyfill::Config>(builtins);
    return data;
}

TEST_F(BuiltinPolyfillTest, ShouldRunClampInteger_i32) {
    auto* src = R"(
fn f() {
  let v = 1i;
  clamp(v, 2i, 3i);
}
)";

    EXPECT_FALSE(ShouldRun<BuiltinPolyfill>(src));
    EXPECT_TRUE(ShouldRun<BuiltinPolyfill>(src, polyfillClampInteger()));
}

TEST_F(BuiltinPolyfillTest, ShouldRunClampInteger_u32) {
    auto* src = R"(
fn f() {
  let v = 1u;
  clamp(v, 2u, 3u);
}
)";

    EXPECT_FALSE(ShouldRun<BuiltinPolyfill>(src));
    EXPECT_TRUE(ShouldRun<BuiltinPolyfill>(src, polyfillClampInteger()));
}

TEST_F(BuiltinPolyfillTest, ShouldRunClampInteger_f32) {
    auto* src = R"(
fn f() {
  let v = 1f;
  clamp(v, 2f, 3f);
}
)";

    EXPECT_FALSE(ShouldRun<BuiltinPolyfill>(src));
    EXPECT_FALSE(ShouldRun<BuiltinPolyfill>(src, polyfillClampInteger()));
}

TEST_F(BuiltinPolyfillTest, ShouldRunClampInteger_f16) {
    auto* src = R"(
enable f16;

fn f() {
  let v = 1h;
  clamp(v, 2h, 3h);
}
)";

    EXPECT_FALSE(ShouldRun<BuiltinPolyfill>(src));
    EXPECT_FALSE(ShouldRun<BuiltinPolyfill>(src, polyfillClampInteger()));
}

TEST_F(BuiltinPolyfillTest, ClampInteger_ConstantExpression) {
    auto* src = R"(
fn f() {
  let r : i32 = clamp(1i, 2i, 3i);
}
)";

    auto* expect = src;

    auto got = Run<BuiltinPolyfill>(src, polyfillClampInteger());

    EXPECT_EQ(expect, str(got));
}

TEST_F(BuiltinPolyfillTest, ClampInteger_i32) {
    auto* src = R"(
fn f() {
  let v = 1i;
  let r : i32 = clamp(v, 2i, 3i);
}
)";

    auto* expect = R"(
fn tint_clamp(e : i32, low : i32, high : i32) -> i32 {
  return min(max(e, low), high);
}

fn f() {
  let v = 1i;
  let r : i32 = tint_clamp(v, 2i, 3i);
}
)";

    auto got = Run<BuiltinPolyfill>(src, polyfillClampInteger());

    EXPECT_EQ(expect, str(got));
}

TEST_F(BuiltinPolyfillTest, ClampInteger_vec3_i32) {
    auto* src = R"(
fn f() {
  let v = 1i;
  let r : vec3<i32> = clamp(vec3(v), vec3(2i), vec3(3i));
}
)";

    auto* expect =
        R"(
fn tint_clamp(e : vec3<i32>, low : vec3<i32>, high : vec3<i32>) -> vec3<i32> {
  return min(max(e, low), high);
}

fn f() {
  let v = 1i;
  let r : vec3<i32> = tint_clamp(vec3(v), vec3(2i), vec3(3i));
}
)";

    auto got = Run<BuiltinPolyfill>(src, polyfillClampInteger());

    EXPECT_EQ(expect, str(got));
}

TEST_F(BuiltinPolyfillTest, ClampInteger_u32) {
    auto* src = R"(
fn f() {
  let r : u32 = clamp(1u, 2u, 3u);
}
)";

    auto* expect = R"(
fn f() {
  let r : u32 = clamp(1u, 2u, 3u);
}
)";

    auto got = Run<BuiltinPolyfill>(src, polyfillClampInteger());

    EXPECT_EQ(expect, str(got));
}

TEST_F(BuiltinPolyfillTest, ClampInteger_vec3_u32) {
    auto* src = R"(
fn f() {
  let v = 1u;
  let r : vec3<u32> = clamp(vec3(v), vec3(2u), vec3(3u));
}
)";

    auto* expect =
        R"(
fn tint_clamp(e : vec3<u32>, low : vec3<u32>, high : vec3<u32>) -> vec3<u32> {
  return min(max(e, low), high);
}

fn f() {
  let v = 1u;
  let r : vec3<u32> = tint_clamp(vec3(v), vec3(2u), vec3(3u));
}
)";

    auto got = Run<BuiltinPolyfill>(src, polyfillClampInteger());

    EXPECT_EQ(expect, str(got));
}

////////////////////////////////////////////////////////////////////////////////
// countLeadingZeros
////////////////////////////////////////////////////////////////////////////////
DataMap polyfillCountLeadingZeros() {
    BuiltinPolyfill::Builtins builtins;
    builtins.count_leading_zeros = true;
    DataMap data;
    data.Add<BuiltinPolyfill::Config>(builtins);
    return data;
}

TEST_F(BuiltinPolyfillTest, ShouldRunCountLeadingZeros) {
    auto* src = R"(
fn f() {
  let v = 15;
  countLeadingZeros(v);
}
)";

    EXPECT_FALSE(ShouldRun<BuiltinPolyfill>(src));
    EXPECT_TRUE(ShouldRun<BuiltinPolyfill>(src, polyfillCountLeadingZeros()));
}

TEST_F(BuiltinPolyfillTest, CountLeadingZeros_ConstantExpression) {
    auto* src = R"(
fn f() {
  let r : i32 = countLeadingZeros(15i);
}
)";

    auto* expect = src;

    auto got = Run<BuiltinPolyfill>(src, polyfillCountLeadingZeros());

    EXPECT_EQ(expect, str(got));
}

TEST_F(BuiltinPolyfillTest, CountLeadingZeros_i32) {
    auto* src = R"(
fn f() {
  let v = 15i;
  let r : i32 = countLeadingZeros(v);
}
)";

    auto* expect = R"(
fn tint_count_leading_zeros(v : i32) -> i32 {
  var x = u32(v);
  let b16 = select(0u, 16u, (x <= 65535u));
  x = (x << b16);
  let b8 = select(0u, 8u, (x <= 16777215u));
  x = (x << b8);
  let b4 = select(0u, 4u, (x <= 268435455u));
  x = (x << b4);
  let b2 = select(0u, 2u, (x <= 1073741823u));
  x = (x << b2);
  let b1 = select(0u, 1u, (x <= 2147483647u));
  let is_zero = select(0u, 1u, (x == 0u));
  return i32((((((b16 | b8) | b4) | b2) | b1) + is_zero));
}

fn f() {
  let v = 15i;
  let r : i32 = tint_count_leading_zeros(v);
}
)";

    auto got = Run<BuiltinPolyfill>(src, polyfillCountLeadingZeros());

    EXPECT_EQ(expect, str(got));
}

TEST_F(BuiltinPolyfillTest, CountLeadingZeros_u32) {
    auto* src = R"(
fn f() {
  let v = 15u;
  let r : u32 = countLeadingZeros(v);
}
)";

    auto* expect = R"(
fn tint_count_leading_zeros(v : u32) -> u32 {
  var x = u32(v);
  let b16 = select(0u, 16u, (x <= 65535u));
  x = (x << b16);
  let b8 = select(0u, 8u, (x <= 16777215u));
  x = (x << b8);
  let b4 = select(0u, 4u, (x <= 268435455u));
  x = (x << b4);
  let b2 = select(0u, 2u, (x <= 1073741823u));
  x = (x << b2);
  let b1 = select(0u, 1u, (x <= 2147483647u));
  let is_zero = select(0u, 1u, (x == 0u));
  return u32((((((b16 | b8) | b4) | b2) | b1) + is_zero));
}

fn f() {
  let v = 15u;
  let r : u32 = tint_count_leading_zeros(v);
}
)";

    auto got = Run<BuiltinPolyfill>(src, polyfillCountLeadingZeros());

    EXPECT_EQ(expect, str(got));
}

TEST_F(BuiltinPolyfillTest, CountLeadingZeros_vec3_i32) {
    auto* src = R"(
fn f() {
  let v = 15i;
  let r : vec3<i32> = countLeadingZeros(vec3<i32>(v));
}
)";

    auto* expect = R"(
fn tint_count_leading_zeros(v : vec3<i32>) -> vec3<i32> {
  var x = vec3<u32>(v);
  let b16 = select(vec3<u32>(0u), vec3<u32>(16u), (x <= vec3<u32>(65535u)));
  x = (x << b16);
  let b8 = select(vec3<u32>(0u), vec3<u32>(8u), (x <= vec3<u32>(16777215u)));
  x = (x << b8);
  let b4 = select(vec3<u32>(0u), vec3<u32>(4u), (x <= vec3<u32>(268435455u)));
  x = (x << b4);
  let b2 = select(vec3<u32>(0u), vec3<u32>(2u), (x <= vec3<u32>(1073741823u)));
  x = (x << b2);
  let b1 = select(vec3<u32>(0u), vec3<u32>(1u), (x <= vec3<u32>(2147483647u)));
  let is_zero = select(vec3<u32>(0u), vec3<u32>(1u), (x == vec3<u32>(0u)));
  return vec3<i32>((((((b16 | b8) | b4) | b2) | b1) + is_zero));
}

fn f() {
  let v = 15i;
  let r : vec3<i32> = tint_count_leading_zeros(vec3<i32>(v));
}
)";

    auto got = Run<BuiltinPolyfill>(src, polyfillCountLeadingZeros());

    EXPECT_EQ(expect, str(got));
}

TEST_F(BuiltinPolyfillTest, CountLeadingZeros_vec3_u32) {
    auto* src = R"(
fn f() {
  let v = 15u;
  let r : vec3<u32> = countLeadingZeros(vec3<u32>(v));
}
)";

    auto* expect = R"(
fn tint_count_leading_zeros(v : vec3<u32>) -> vec3<u32> {
  var x = vec3<u32>(v);
  let b16 = select(vec3<u32>(0u), vec3<u32>(16u), (x <= vec3<u32>(65535u)));
  x = (x << b16);
  let b8 = select(vec3<u32>(0u), vec3<u32>(8u), (x <= vec3<u32>(16777215u)));
  x = (x << b8);
  let b4 = select(vec3<u32>(0u), vec3<u32>(4u), (x <= vec3<u32>(268435455u)));
  x = (x << b4);
  let b2 = select(vec3<u32>(0u), vec3<u32>(2u), (x <= vec3<u32>(1073741823u)));
  x = (x << b2);
  let b1 = select(vec3<u32>(0u), vec3<u32>(1u), (x <= vec3<u32>(2147483647u)));
  let is_zero = select(vec3<u32>(0u), vec3<u32>(1u), (x == vec3<u32>(0u)));
  return vec3<u32>((((((b16 | b8) | b4) | b2) | b1) + is_zero));
}

fn f() {
  let v = 15u;
  let r : vec3<u32> = tint_count_leading_zeros(vec3<u32>(v));
}
)";

    auto got = Run<BuiltinPolyfill>(src, polyfillCountLeadingZeros());

    EXPECT_EQ(expect, str(got));
}

////////////////////////////////////////////////////////////////////////////////
// countTrailingZeros
////////////////////////////////////////////////////////////////////////////////
DataMap polyfillCountTrailingZeros() {
    BuiltinPolyfill::Builtins builtins;
    builtins.count_trailing_zeros = true;
    DataMap data;
    data.Add<BuiltinPolyfill::Config>(builtins);
    return data;
}

TEST_F(BuiltinPolyfillTest, ShouldRunCountTrailingZeros) {
    auto* src = R"(
fn f() {
  let v = 15;
  countTrailingZeros(v);
}
)";

    EXPECT_FALSE(ShouldRun<BuiltinPolyfill>(src));
    EXPECT_TRUE(ShouldRun<BuiltinPolyfill>(src, polyfillCountTrailingZeros()));
}

TEST_F(BuiltinPolyfillTest, CountTrailingZeros_ConstantExpression) {
    auto* src = R"(
fn f() {
  let r : i32 = countTrailingZeros(15i);
}
)";

    auto* expect = src;

    auto got = Run<BuiltinPolyfill>(src, polyfillCountTrailingZeros());

    EXPECT_EQ(expect, str(got));
}

TEST_F(BuiltinPolyfillTest, CountTrailingZeros_i32) {
    auto* src = R"(
fn f() {
  let v = 15i;
  let r : i32 = countTrailingZeros(v);
}
)";

    auto* expect = R"(
fn tint_count_trailing_zeros(v : i32) -> i32 {
  var x = u32(v);
  let b16 = select(16u, 0u, bool((x & 65535u)));
  x = (x >> b16);
  let b8 = select(8u, 0u, bool((x & 255u)));
  x = (x >> b8);
  let b4 = select(4u, 0u, bool((x & 15u)));
  x = (x >> b4);
  let b2 = select(2u, 0u, bool((x & 3u)));
  x = (x >> b2);
  let b1 = select(1u, 0u, bool((x & 1u)));
  let is_zero = select(0u, 1u, (x == 0u));
  return i32((((((b16 | b8) | b4) | b2) | b1) + is_zero));
}

fn f() {
  let v = 15i;
  let r : i32 = tint_count_trailing_zeros(v);
}
)";

    auto got = Run<BuiltinPolyfill>(src, polyfillCountTrailingZeros());

    EXPECT_EQ(expect, str(got));
}

TEST_F(BuiltinPolyfillTest, CountTrailingZeros_u32) {
    auto* src = R"(
fn f() {
  let v = 15u;
  let r : u32 = countTrailingZeros(v);
}
)";

    auto* expect = R"(
fn tint_count_trailing_zeros(v : u32) -> u32 {
  var x = u32(v);
  let b16 = select(16u, 0u, bool((x & 65535u)));
  x = (x >> b16);
  let b8 = select(8u, 0u, bool((x & 255u)));
  x = (x >> b8);
  let b4 = select(4u, 0u, bool((x & 15u)));
  x = (x >> b4);
  let b2 = select(2u, 0u, bool((x & 3u)));
  x = (x >> b2);
  let b1 = select(1u, 0u, bool((x & 1u)));
  let is_zero = select(0u, 1u, (x == 0u));
  return u32((((((b16 | b8) | b4) | b2) | b1) + is_zero));
}

fn f() {
  let v = 15u;
  let r : u32 = tint_count_trailing_zeros(v);
}
)";

    auto got = Run<BuiltinPolyfill>(src, polyfillCountTrailingZeros());

    EXPECT_EQ(expect, str(got));
}

TEST_F(BuiltinPolyfillTest, CountTrailingZeros_vec3_i32) {
    auto* src = R"(
fn f() {
  let v = 15i;
  let r : vec3<i32> = countTrailingZeros(vec3<i32>(v));
}
)";

    auto* expect = R"(
fn tint_count_trailing_zeros(v : vec3<i32>) -> vec3<i32> {
  var x = vec3<u32>(v);
  let b16 = select(vec3<u32>(16u), vec3<u32>(0u), vec3<bool>((x & vec3<u32>(65535u))));
  x = (x >> b16);
  let b8 = select(vec3<u32>(8u), vec3<u32>(0u), vec3<bool>((x & vec3<u32>(255u))));
  x = (x >> b8);
  let b4 = select(vec3<u32>(4u), vec3<u32>(0u), vec3<bool>((x & vec3<u32>(15u))));
  x = (x >> b4);
  let b2 = select(vec3<u32>(2u), vec3<u32>(0u), vec3<bool>((x & vec3<u32>(3u))));
  x = (x >> b2);
  let b1 = select(vec3<u32>(1u), vec3<u32>(0u), vec3<bool>((x & vec3<u32>(1u))));
  let is_zero = select(vec3<u32>(0u), vec3<u32>(1u), (x == vec3<u32>(0u)));
  return vec3<i32>((((((b16 | b8) | b4) | b2) | b1) + is_zero));
}

fn f() {
  let v = 15i;
  let r : vec3<i32> = tint_count_trailing_zeros(vec3<i32>(v));
}
)";

    auto got = Run<BuiltinPolyfill>(src, polyfillCountTrailingZeros());

    EXPECT_EQ(expect, str(got));
}

TEST_F(BuiltinPolyfillTest, CountTrailingZeros_vec3_u32) {
    auto* src = R"(
fn f() {
  let v = 15u;
  let r : vec3<u32> = countTrailingZeros(vec3<u32>(v));
}
)";

    auto* expect = R"(
fn tint_count_trailing_zeros(v : vec3<u32>) -> vec3<u32> {
  var x = vec3<u32>(v);
  let b16 = select(vec3<u32>(16u), vec3<u32>(0u), vec3<bool>((x & vec3<u32>(65535u))));
  x = (x >> b16);
  let b8 = select(vec3<u32>(8u), vec3<u32>(0u), vec3<bool>((x & vec3<u32>(255u))));
  x = (x >> b8);
  let b4 = select(vec3<u32>(4u), vec3<u32>(0u), vec3<bool>((x & vec3<u32>(15u))));
  x = (x >> b4);
  let b2 = select(vec3<u32>(2u), vec3<u32>(0u), vec3<bool>((x & vec3<u32>(3u))));
  x = (x >> b2);
  let b1 = select(vec3<u32>(1u), vec3<u32>(0u), vec3<bool>((x & vec3<u32>(1u))));
  let is_zero = select(vec3<u32>(0u), vec3<u32>(1u), (x == vec3<u32>(0u)));
  return vec3<u32>((((((b16 | b8) | b4) | b2) | b1) + is_zero));
}

fn f() {
  let v = 15u;
  let r : vec3<u32> = tint_count_trailing_zeros(vec3<u32>(v));
}
)";

    auto got = Run<BuiltinPolyfill>(src, polyfillCountTrailingZeros());

    EXPECT_EQ(expect, str(got));
}

////////////////////////////////////////////////////////////////////////////////
// extractBits
////////////////////////////////////////////////////////////////////////////////
DataMap polyfillExtractBits(Level level) {
    BuiltinPolyfill::Builtins builtins;
    builtins.extract_bits = level;
    DataMap data;
    data.Add<BuiltinPolyfill::Config>(builtins);
    return data;
}

TEST_F(BuiltinPolyfillTest, ShouldRunExtractBits) {
    auto* src = R"(
fn f() {
  let v = 1234i;
  extractBits(v, 5u, 6u);
}
)";

    EXPECT_FALSE(ShouldRun<BuiltinPolyfill>(src));
    EXPECT_FALSE(ShouldRun<BuiltinPolyfill>(src, polyfillExtractBits(Level::kNone)));
    EXPECT_TRUE(ShouldRun<BuiltinPolyfill>(src, polyfillExtractBits(Level::kClampParameters)));
    EXPECT_TRUE(ShouldRun<BuiltinPolyfill>(src, polyfillExtractBits(Level::kFull)));
}

TEST_F(BuiltinPolyfillTest, ExtractBits_ConstantExpression) {
    auto* src = R"(
fn f() {
  let r : i32 = countTrailingZeros(15i);
}
)";

    auto* expect = src;

    auto got = Run<BuiltinPolyfill>(src, polyfillExtractBits(Level::kFull));

    EXPECT_EQ(expect, str(got));
}

TEST_F(BuiltinPolyfillTest, ExtractBits_Full_i32) {
    auto* src = R"(
fn f() {
  let v = 1234i;
  let r : i32 = extractBits(v, 5u, 6u);
}
)";

    auto* expect = R"(
fn tint_extract_bits(v : i32, offset : u32, count : u32) -> i32 {
  let s = min(offset, 32u);
  let e = min(32u, (s + count));
  let shl = (32u - e);
  let shr = (shl + s);
  return ((v << shl) >> shr);
}

fn f() {
  let v = 1234i;
  let r : i32 = tint_extract_bits(v, 5u, 6u);
}
)";

    auto got = Run<BuiltinPolyfill>(src, polyfillExtractBits(Level::kFull));

    EXPECT_EQ(expect, str(got));
}

TEST_F(BuiltinPolyfillTest, ExtractBits_Full_u32) {
    auto* src = R"(
fn f() {
  let v = 1234u;
  let r : u32 = extractBits(v, 5u, 6u);
}
)";

    auto* expect = R"(
fn tint_extract_bits(v : u32, offset : u32, count : u32) -> u32 {
  let s = min(offset, 32u);
  let e = min(32u, (s + count));
  let shl = (32u - e);
  let shr = (shl + s);
  return ((v << shl) >> shr);
}

fn f() {
  let v = 1234u;
  let r : u32 = tint_extract_bits(v, 5u, 6u);
}
)";

    auto got = Run<BuiltinPolyfill>(src, polyfillExtractBits(Level::kFull));

    EXPECT_EQ(expect, str(got));
}

TEST_F(BuiltinPolyfillTest, ExtractBits_Full_vec3_i32) {
    auto* src = R"(
fn f() {
  let v = 1234i;
  let r : vec3<i32> = extractBits(vec3<i32>(v), 5u, 6u);
}
)";

    auto* expect = R"(
fn tint_extract_bits(v : vec3<i32>, offset : u32, count : u32) -> vec3<i32> {
  let s = min(offset, 32u);
  let e = min(32u, (s + count));
  let shl = (32u - e);
  let shr = (shl + s);
  return ((v << vec3<u32>(shl)) >> vec3<u32>(shr));
}

fn f() {
  let v = 1234i;
  let r : vec3<i32> = tint_extract_bits(vec3<i32>(v), 5u, 6u);
}
)";

    auto got = Run<BuiltinPolyfill>(src, polyfillExtractBits(Level::kFull));

    EXPECT_EQ(expect, str(got));
}

TEST_F(BuiltinPolyfillTest, ExtractBits_Full_vec3_u32) {
    auto* src = R"(
fn f() {
  let v = 1234u;
  let r : vec3<u32> = extractBits(vec3<u32>(v), 5u, 6u);
}
)";

    auto* expect = R"(
fn tint_extract_bits(v : vec3<u32>, offset : u32, count : u32) -> vec3<u32> {
  let s = min(offset, 32u);
  let e = min(32u, (s + count));
  let shl = (32u - e);
  let shr = (shl + s);
  return ((v << vec3<u32>(shl)) >> vec3<u32>(shr));
}

fn f() {
  let v = 1234u;
  let r : vec3<u32> = tint_extract_bits(vec3<u32>(v), 5u, 6u);
}
)";

    auto got = Run<BuiltinPolyfill>(src, polyfillExtractBits(Level::kFull));

    EXPECT_EQ(expect, str(got));
}

TEST_F(BuiltinPolyfillTest, ExtractBits_Clamp_i32) {
    auto* src = R"(
fn f() {
  let v = 1234i;
  let r : i32 = extractBits(v, 5u, 6u);
}
)";

    auto* expect = R"(
fn tint_extract_bits(v : i32, offset : u32, count : u32) -> i32 {
  let s = min(offset, 32u);
  let e = min(32u, (s + count));
  return extractBits(v, s, (e - s));
}

fn f() {
  let v = 1234i;
  let r : i32 = tint_extract_bits(v, 5u, 6u);
}
)";

    auto got = Run<BuiltinPolyfill>(src, polyfillExtractBits(Level::kClampParameters));

    EXPECT_EQ(expect, str(got));
}

TEST_F(BuiltinPolyfillTest, ExtractBits_Clamp_u32) {
    auto* src = R"(
fn f() {
  let v = 1234u;
  let r : u32 = extractBits(v, 5u, 6u);
}
)";

    auto* expect = R"(
fn tint_extract_bits(v : u32, offset : u32, count : u32) -> u32 {
  let s = min(offset, 32u);
  let e = min(32u, (s + count));
  return extractBits(v, s, (e - s));
}

fn f() {
  let v = 1234u;
  let r : u32 = tint_extract_bits(v, 5u, 6u);
}
)";

    auto got = Run<BuiltinPolyfill>(src, polyfillExtractBits(Level::kClampParameters));

    EXPECT_EQ(expect, str(got));
}

TEST_F(BuiltinPolyfillTest, ExtractBits_Clamp_vec3_i32) {
    auto* src = R"(
fn f() {
  let v = 1234i;
  let r : vec3<i32> = extractBits(vec3<i32>(v), 5u, 6u);
}
)";

    auto* expect = R"(
fn tint_extract_bits(v : vec3<i32>, offset : u32, count : u32) -> vec3<i32> {
  let s = min(offset, 32u);
  let e = min(32u, (s + count));
  return extractBits(v, s, (e - s));
}

fn f() {
  let v = 1234i;
  let r : vec3<i32> = tint_extract_bits(vec3<i32>(v), 5u, 6u);
}
)";

    auto got = Run<BuiltinPolyfill>(src, polyfillExtractBits(Level::kClampParameters));

    EXPECT_EQ(expect, str(got));
}

TEST_F(BuiltinPolyfillTest, ExtractBits_Clamp_vec3_u32) {
    auto* src = R"(
fn f() {
  let v = 1234u;
  let r : vec3<u32> = extractBits(vec3<u32>(v), 5u, 6u);
}
)";

    auto* expect = R"(
fn tint_extract_bits(v : vec3<u32>, offset : u32, count : u32) -> vec3<u32> {
  let s = min(offset, 32u);
  let e = min(32u, (s + count));
  return extractBits(v, s, (e - s));
}

fn f() {
  let v = 1234u;
  let r : vec3<u32> = tint_extract_bits(vec3<u32>(v), 5u, 6u);
}
)";

    auto got = Run<BuiltinPolyfill>(src, polyfillExtractBits(Level::kClampParameters));

    EXPECT_EQ(expect, str(got));
}

////////////////////////////////////////////////////////////////////////////////
// firstLeadingBit
////////////////////////////////////////////////////////////////////////////////
DataMap polyfillFirstLeadingBit() {
    BuiltinPolyfill::Builtins builtins;
    builtins.first_leading_bit = true;
    DataMap data;
    data.Add<BuiltinPolyfill::Config>(builtins);
    return data;
}

TEST_F(BuiltinPolyfillTest, ShouldRunFirstLeadingBit) {
    auto* src = R"(
fn f() {
  let v = 15i;
  firstLeadingBit(v);
}
)";

    EXPECT_FALSE(ShouldRun<BuiltinPolyfill>(src));
    EXPECT_TRUE(ShouldRun<BuiltinPolyfill>(src, polyfillFirstLeadingBit()));
}

TEST_F(BuiltinPolyfillTest, FirstLeadingBit_ConstantExpression) {
    auto* src = R"(
fn f() {
  let r : i32 = firstLeadingBit(15i);
}
)";

    auto* expect = src;

    auto got = Run<BuiltinPolyfill>(src, polyfillFirstLeadingBit());

    EXPECT_EQ(expect, str(got));
}

TEST_F(BuiltinPolyfillTest, FirstLeadingBit_i32) {
    auto* src = R"(
fn f() {
  let v = 15i;
  let r : i32 = firstLeadingBit(v);
}
)";

    auto* expect = R"(
fn tint_first_leading_bit(v : i32) -> i32 {
  var x = select(u32(v), u32(~(v)), (v < 0i));
  let b16 = select(0u, 16u, bool((x & 4294901760u)));
  x = (x >> b16);
  let b8 = select(0u, 8u, bool((x & 65280u)));
  x = (x >> b8);
  let b4 = select(0u, 4u, bool((x & 240u)));
  x = (x >> b4);
  let b2 = select(0u, 2u, bool((x & 12u)));
  x = (x >> b2);
  let b1 = select(0u, 1u, bool((x & 2u)));
  let is_zero = select(0u, 4294967295u, (x == 0u));
  return i32((((((b16 | b8) | b4) | b2) | b1) | is_zero));
}

fn f() {
  let v = 15i;
  let r : i32 = tint_first_leading_bit(v);
}
)";

    auto got = Run<BuiltinPolyfill>(src, polyfillFirstLeadingBit());

    EXPECT_EQ(expect, str(got));
}

TEST_F(BuiltinPolyfillTest, FirstLeadingBit_u32) {
    auto* src = R"(
fn f() {
  let v = 15u;
  let r : u32 = firstLeadingBit(v);
}
)";

    auto* expect = R"(
fn tint_first_leading_bit(v : u32) -> u32 {
  var x = v;
  let b16 = select(0u, 16u, bool((x & 4294901760u)));
  x = (x >> b16);
  let b8 = select(0u, 8u, bool((x & 65280u)));
  x = (x >> b8);
  let b4 = select(0u, 4u, bool((x & 240u)));
  x = (x >> b4);
  let b2 = select(0u, 2u, bool((x & 12u)));
  x = (x >> b2);
  let b1 = select(0u, 1u, bool((x & 2u)));
  let is_zero = select(0u, 4294967295u, (x == 0u));
  return u32((((((b16 | b8) | b4) | b2) | b1) | is_zero));
}

fn f() {
  let v = 15u;
  let r : u32 = tint_first_leading_bit(v);
}
)";

    auto got = Run<BuiltinPolyfill>(src, polyfillFirstLeadingBit());

    EXPECT_EQ(expect, str(got));
}

TEST_F(BuiltinPolyfillTest, FirstLeadingBit_vec3_i32) {
    auto* src = R"(
fn f() {
  let v = 15i;
  let r : vec3<i32> = firstLeadingBit(vec3<i32>(v));
}
)";

    auto* expect = R"(
fn tint_first_leading_bit(v : vec3<i32>) -> vec3<i32> {
  var x = select(vec3<u32>(v), vec3<u32>(~(v)), (v < vec3<i32>(0i)));
  let b16 = select(vec3<u32>(0u), vec3<u32>(16u), vec3<bool>((x & vec3<u32>(4294901760u))));
  x = (x >> b16);
  let b8 = select(vec3<u32>(0u), vec3<u32>(8u), vec3<bool>((x & vec3<u32>(65280u))));
  x = (x >> b8);
  let b4 = select(vec3<u32>(0u), vec3<u32>(4u), vec3<bool>((x & vec3<u32>(240u))));
  x = (x >> b4);
  let b2 = select(vec3<u32>(0u), vec3<u32>(2u), vec3<bool>((x & vec3<u32>(12u))));
  x = (x >> b2);
  let b1 = select(vec3<u32>(0u), vec3<u32>(1u), vec3<bool>((x & vec3<u32>(2u))));
  let is_zero = select(vec3<u32>(0u), vec3<u32>(4294967295u), (x == vec3<u32>(0u)));
  return vec3<i32>((((((b16 | b8) | b4) | b2) | b1) | is_zero));
}

fn f() {
  let v = 15i;
  let r : vec3<i32> = tint_first_leading_bit(vec3<i32>(v));
}
)";

    auto got = Run<BuiltinPolyfill>(src, polyfillFirstLeadingBit());

    EXPECT_EQ(expect, str(got));
}

TEST_F(BuiltinPolyfillTest, FirstLeadingBit_vec3_u32) {
    auto* src = R"(
fn f() {
  let v = 15u;
  let r : vec3<u32> = firstLeadingBit(vec3<u32>(v));
}
)";

    auto* expect = R"(
fn tint_first_leading_bit(v : vec3<u32>) -> vec3<u32> {
  var x = v;
  let b16 = select(vec3<u32>(0u), vec3<u32>(16u), vec3<bool>((x & vec3<u32>(4294901760u))));
  x = (x >> b16);
  let b8 = select(vec3<u32>(0u), vec3<u32>(8u), vec3<bool>((x & vec3<u32>(65280u))));
  x = (x >> b8);
  let b4 = select(vec3<u32>(0u), vec3<u32>(4u), vec3<bool>((x & vec3<u32>(240u))));
  x = (x >> b4);
  let b2 = select(vec3<u32>(0u), vec3<u32>(2u), vec3<bool>((x & vec3<u32>(12u))));
  x = (x >> b2);
  let b1 = select(vec3<u32>(0u), vec3<u32>(1u), vec3<bool>((x & vec3<u32>(2u))));
  let is_zero = select(vec3<u32>(0u), vec3<u32>(4294967295u), (x == vec3<u32>(0u)));
  return vec3<u32>((((((b16 | b8) | b4) | b2) | b1) | is_zero));
}

fn f() {
  let v = 15u;
  let r : vec3<u32> = tint_first_leading_bit(vec3<u32>(v));
}
)";

    auto got = Run<BuiltinPolyfill>(src, polyfillFirstLeadingBit());

    EXPECT_EQ(expect, str(got));
}

////////////////////////////////////////////////////////////////////////////////
// firstTrailingBit
////////////////////////////////////////////////////////////////////////////////
DataMap polyfillFirstTrailingBit() {
    BuiltinPolyfill::Builtins builtins;
    builtins.first_trailing_bit = true;
    DataMap data;
    data.Add<BuiltinPolyfill::Config>(builtins);
    return data;
}

TEST_F(BuiltinPolyfillTest, ShouldRunFirstTrailingBit) {
    auto* src = R"(
fn f() {
  let v = 15i;
  firstTrailingBit(v);
}
)";

    EXPECT_FALSE(ShouldRun<BuiltinPolyfill>(src));
    EXPECT_TRUE(ShouldRun<BuiltinPolyfill>(src, polyfillFirstTrailingBit()));
}

TEST_F(BuiltinPolyfillTest, FirstTrailingBit_ConstantExpression) {
    auto* src = R"(
fn f() {
  let r : i32 = firstTrailingBit(15i);
}
)";

    auto* expect = src;

    auto got = Run<BuiltinPolyfill>(src, polyfillFirstTrailingBit());

    EXPECT_EQ(expect, str(got));
}

TEST_F(BuiltinPolyfillTest, FirstTrailingBit_i32) {
    auto* src = R"(
fn f() {
  let v = 15i;
  let r : i32 = firstTrailingBit(v);
}
)";

    auto* expect = R"(
fn tint_first_trailing_bit(v : i32) -> i32 {
  var x = u32(v);
  let b16 = select(16u, 0u, bool((x & 65535u)));
  x = (x >> b16);
  let b8 = select(8u, 0u, bool((x & 255u)));
  x = (x >> b8);
  let b4 = select(4u, 0u, bool((x & 15u)));
  x = (x >> b4);
  let b2 = select(2u, 0u, bool((x & 3u)));
  x = (x >> b2);
  let b1 = select(1u, 0u, bool((x & 1u)));
  let is_zero = select(0u, 4294967295u, (x == 0u));
  return i32((((((b16 | b8) | b4) | b2) | b1) | is_zero));
}

fn f() {
  let v = 15i;
  let r : i32 = tint_first_trailing_bit(v);
}
)";

    auto got = Run<BuiltinPolyfill>(src, polyfillFirstTrailingBit());

    EXPECT_EQ(expect, str(got));
}

TEST_F(BuiltinPolyfillTest, FirstTrailingBit_u32) {
    auto* src = R"(
fn f() {
  let v = 15u;
  let r : u32 = firstTrailingBit(v);
}
)";

    auto* expect = R"(
fn tint_first_trailing_bit(v : u32) -> u32 {
  var x = u32(v);
  let b16 = select(16u, 0u, bool((x & 65535u)));
  x = (x >> b16);
  let b8 = select(8u, 0u, bool((x & 255u)));
  x = (x >> b8);
  let b4 = select(4u, 0u, bool((x & 15u)));
  x = (x >> b4);
  let b2 = select(2u, 0u, bool((x & 3u)));
  x = (x >> b2);
  let b1 = select(1u, 0u, bool((x & 1u)));
  let is_zero = select(0u, 4294967295u, (x == 0u));
  return u32((((((b16 | b8) | b4) | b2) | b1) | is_zero));
}

fn f() {
  let v = 15u;
  let r : u32 = tint_first_trailing_bit(v);
}
)";

    auto got = Run<BuiltinPolyfill>(src, polyfillFirstTrailingBit());

    EXPECT_EQ(expect, str(got));
}

TEST_F(BuiltinPolyfillTest, FirstTrailingBit_vec3_i32) {
    auto* src = R"(
fn f() {
  let v = 15i;
  let r : vec3<i32> = firstTrailingBit(vec3<i32>(v));
}
)";

    auto* expect = R"(
fn tint_first_trailing_bit(v : vec3<i32>) -> vec3<i32> {
  var x = vec3<u32>(v);
  let b16 = select(vec3<u32>(16u), vec3<u32>(0u), vec3<bool>((x & vec3<u32>(65535u))));
  x = (x >> b16);
  let b8 = select(vec3<u32>(8u), vec3<u32>(0u), vec3<bool>((x & vec3<u32>(255u))));
  x = (x >> b8);
  let b4 = select(vec3<u32>(4u), vec3<u32>(0u), vec3<bool>((x & vec3<u32>(15u))));
  x = (x >> b4);
  let b2 = select(vec3<u32>(2u), vec3<u32>(0u), vec3<bool>((x & vec3<u32>(3u))));
  x = (x >> b2);
  let b1 = select(vec3<u32>(1u), vec3<u32>(0u), vec3<bool>((x & vec3<u32>(1u))));
  let is_zero = select(vec3<u32>(0u), vec3<u32>(4294967295u), (x == vec3<u32>(0u)));
  return vec3<i32>((((((b16 | b8) | b4) | b2) | b1) | is_zero));
}

fn f() {
  let v = 15i;
  let r : vec3<i32> = tint_first_trailing_bit(vec3<i32>(v));
}
)";

    auto got = Run<BuiltinPolyfill>(src, polyfillFirstTrailingBit());

    EXPECT_EQ(expect, str(got));
}

TEST_F(BuiltinPolyfillTest, FirstTrailingBit_vec3_u32) {
    auto* src = R"(
fn f() {
  let v = 15u;
  let r : vec3<u32> = firstTrailingBit(vec3<u32>(v));
}
)";

    auto* expect = R"(
fn tint_first_trailing_bit(v : vec3<u32>) -> vec3<u32> {
  var x = vec3<u32>(v);
  let b16 = select(vec3<u32>(16u), vec3<u32>(0u), vec3<bool>((x & vec3<u32>(65535u))));
  x = (x >> b16);
  let b8 = select(vec3<u32>(8u), vec3<u32>(0u), vec3<bool>((x & vec3<u32>(255u))));
  x = (x >> b8);
  let b4 = select(vec3<u32>(4u), vec3<u32>(0u), vec3<bool>((x & vec3<u32>(15u))));
  x = (x >> b4);
  let b2 = select(vec3<u32>(2u), vec3<u32>(0u), vec3<bool>((x & vec3<u32>(3u))));
  x = (x >> b2);
  let b1 = select(vec3<u32>(1u), vec3<u32>(0u), vec3<bool>((x & vec3<u32>(1u))));
  let is_zero = select(vec3<u32>(0u), vec3<u32>(4294967295u), (x == vec3<u32>(0u)));
  return vec3<u32>((((((b16 | b8) | b4) | b2) | b1) | is_zero));
}

fn f() {
  let v = 15u;
  let r : vec3<u32> = tint_first_trailing_bit(vec3<u32>(v));
}
)";

    auto got = Run<BuiltinPolyfill>(src, polyfillFirstTrailingBit());

    EXPECT_EQ(expect, str(got));
}

////////////////////////////////////////////////////////////////////////////////
// insertBits
////////////////////////////////////////////////////////////////////////////////
DataMap polyfillInsertBits(Level level) {
    BuiltinPolyfill::Builtins builtins;
    builtins.insert_bits = level;
    DataMap data;
    data.Add<BuiltinPolyfill::Config>(builtins);
    return data;
}

TEST_F(BuiltinPolyfillTest, ShouldRunInsertBits) {
    auto* src = R"(
fn f() {
  let v = 1234i;
  insertBits(v, 5678, 5u, 6u);
}
)";

    EXPECT_FALSE(ShouldRun<BuiltinPolyfill>(src));
    EXPECT_FALSE(ShouldRun<BuiltinPolyfill>(src, polyfillInsertBits(Level::kNone)));
    EXPECT_TRUE(ShouldRun<BuiltinPolyfill>(src, polyfillInsertBits(Level::kClampParameters)));
    EXPECT_TRUE(ShouldRun<BuiltinPolyfill>(src, polyfillInsertBits(Level::kFull)));
}

TEST_F(BuiltinPolyfillTest, InsertBits_ConstantExpression) {
    auto* src = R"(
fn f() {
  let r : i32 = insertBits(1234i, 5678i, 5u, 6u);
}
)";

    auto* expect = src;

    auto got = Run<BuiltinPolyfill>(src, polyfillInsertBits(Level::kFull));

    EXPECT_EQ(expect, str(got));
}

TEST_F(BuiltinPolyfillTest, InsertBits_Full_i32) {
    auto* src = R"(
fn f() {
  let v = 1234i;
  let r : i32 = insertBits(v, 5678, 5u, 6u);
}
)";

    auto* expect = R"(
fn tint_insert_bits(v : i32, n : i32, offset : u32, count : u32) -> i32 {
  let e = (offset + count);
  let mask = ((select(0u, (1u << offset), (offset < 32u)) - 1u) ^ (select(0u, (1u << e), (e < 32u)) - 1u));
  return ((select(i32(), (n << offset), (offset < 32u)) & i32(mask)) | (v & i32(~(mask))));
}

fn f() {
  let v = 1234i;
  let r : i32 = tint_insert_bits(v, 5678, 5u, 6u);
}
)";

    auto got = Run<BuiltinPolyfill>(src, polyfillInsertBits(Level::kFull));

    EXPECT_EQ(expect, str(got));
}

TEST_F(BuiltinPolyfillTest, InsertBits_Full_u32) {
    auto* src = R"(
fn f() {
  let v = 1234u;
  let r : u32 = insertBits(v, 5678u, 5u, 6u);
}
)";

    auto* expect = R"(
fn tint_insert_bits(v : u32, n : u32, offset : u32, count : u32) -> u32 {
  let e = (offset + count);
  let mask = ((select(0u, (1u << offset), (offset < 32u)) - 1u) ^ (select(0u, (1u << e), (e < 32u)) - 1u));
  return ((select(u32(), (n << offset), (offset < 32u)) & mask) | (v & ~(mask)));
}

fn f() {
  let v = 1234u;
  let r : u32 = tint_insert_bits(v, 5678u, 5u, 6u);
}
)";

    auto got = Run<BuiltinPolyfill>(src, polyfillInsertBits(Level::kFull));

    EXPECT_EQ(expect, str(got));
}

TEST_F(BuiltinPolyfillTest, InsertBits_Full_vec3_i32) {
    auto* src = R"(
fn f() {
  let v = 1234i;
  let r : vec3<i32> = insertBits(vec3<i32>(v), vec3<i32>(5678), 5u, 6u);
}
)";

    auto* expect = R"(
fn tint_insert_bits(v : vec3<i32>, n : vec3<i32>, offset : u32, count : u32) -> vec3<i32> {
  let e = (offset + count);
  let mask = ((select(0u, (1u << offset), (offset < 32u)) - 1u) ^ (select(0u, (1u << e), (e < 32u)) - 1u));
  return ((select(vec3<i32>(), (n << vec3<u32>(offset)), (offset < 32u)) & vec3<i32>(i32(mask))) | (v & vec3<i32>(i32(~(mask)))));
}

fn f() {
  let v = 1234i;
  let r : vec3<i32> = tint_insert_bits(vec3<i32>(v), vec3<i32>(5678), 5u, 6u);
}
)";

    auto got = Run<BuiltinPolyfill>(src, polyfillInsertBits(Level::kFull));

    EXPECT_EQ(expect, str(got));
}

TEST_F(BuiltinPolyfillTest, InsertBits_Full_vec3_u32) {
    auto* src = R"(
fn f() {
  let v = 1234u;
  let r : vec3<u32> = insertBits(vec3<u32>(v), vec3<u32>(5678u), 5u, 6u);
}
)";

    auto* expect = R"(
fn tint_insert_bits(v : vec3<u32>, n : vec3<u32>, offset : u32, count : u32) -> vec3<u32> {
  let e = (offset + count);
  let mask = ((select(0u, (1u << offset), (offset < 32u)) - 1u) ^ (select(0u, (1u << e), (e < 32u)) - 1u));
  return ((select(vec3<u32>(), (n << vec3<u32>(offset)), (offset < 32u)) & vec3<u32>(mask)) | (v & vec3<u32>(~(mask))));
}

fn f() {
  let v = 1234u;
  let r : vec3<u32> = tint_insert_bits(vec3<u32>(v), vec3<u32>(5678u), 5u, 6u);
}
)";

    auto got = Run<BuiltinPolyfill>(src, polyfillInsertBits(Level::kFull));

    EXPECT_EQ(expect, str(got));
}

TEST_F(BuiltinPolyfillTest, InsertBits_Clamp_i32) {
    auto* src = R"(
fn f() {
  let v = 1234i;
  let r : i32 = insertBits(v, 5678, 5u, 6u);
}
)";

    auto* expect = R"(
fn tint_insert_bits(v : i32, n : i32, offset : u32, count : u32) -> i32 {
  let s = min(offset, 32u);
  let e = min(32u, (s + count));
  return insertBits(v, n, s, (e - s));
}

fn f() {
  let v = 1234i;
  let r : i32 = tint_insert_bits(v, 5678, 5u, 6u);
}
)";

    auto got = Run<BuiltinPolyfill>(src, polyfillInsertBits(Level::kClampParameters));

    EXPECT_EQ(expect, str(got));
}

TEST_F(BuiltinPolyfillTest, InsertBits_Clamp_u32) {
    auto* src = R"(
fn f() {
  let v = 1234u;
  let r : u32 = insertBits(v, 5678u, 5u, 6u);
}
)";

    auto* expect = R"(
fn tint_insert_bits(v : u32, n : u32, offset : u32, count : u32) -> u32 {
  let s = min(offset, 32u);
  let e = min(32u, (s + count));
  return insertBits(v, n, s, (e - s));
}

fn f() {
  let v = 1234u;
  let r : u32 = tint_insert_bits(v, 5678u, 5u, 6u);
}
)";

    auto got = Run<BuiltinPolyfill>(src, polyfillInsertBits(Level::kClampParameters));

    EXPECT_EQ(expect, str(got));
}

TEST_F(BuiltinPolyfillTest, InsertBits_Clamp_vec3_i32) {
    auto* src = R"(
fn f() {
  let v = 1234i;
  let r : vec3<i32> = insertBits(vec3<i32>(v), vec3<i32>(5678), 5u, 6u);
}
)";

    auto* expect = R"(
fn tint_insert_bits(v : vec3<i32>, n : vec3<i32>, offset : u32, count : u32) -> vec3<i32> {
  let s = min(offset, 32u);
  let e = min(32u, (s + count));
  return insertBits(v, n, s, (e - s));
}

fn f() {
  let v = 1234i;
  let r : vec3<i32> = tint_insert_bits(vec3<i32>(v), vec3<i32>(5678), 5u, 6u);
}
)";

    auto got = Run<BuiltinPolyfill>(src, polyfillInsertBits(Level::kClampParameters));

    EXPECT_EQ(expect, str(got));
}

TEST_F(BuiltinPolyfillTest, InsertBits_Clamp_vec3_u32) {
    auto* src = R"(
fn f() {
  let v = 1234u;
  let r : vec3<u32> = insertBits(vec3<u32>(v), vec3<u32>(5678u), 5u, 6u);
}
)";

    auto* expect = R"(
fn tint_insert_bits(v : vec3<u32>, n : vec3<u32>, offset : u32, count : u32) -> vec3<u32> {
  let s = min(offset, 32u);
  let e = min(32u, (s + count));
  return insertBits(v, n, s, (e - s));
}

fn f() {
  let v = 1234u;
  let r : vec3<u32> = tint_insert_bits(vec3<u32>(v), vec3<u32>(5678u), 5u, 6u);
}
)";

    auto got = Run<BuiltinPolyfill>(src, polyfillInsertBits(Level::kClampParameters));

    EXPECT_EQ(expect, str(got));
}

////////////////////////////////////////////////////////////////////////////////
// int_div_mod
////////////////////////////////////////////////////////////////////////////////
DataMap polyfillIntDivMod() {
    BuiltinPolyfill::Builtins builtins;
    builtins.int_div_mod = true;
    DataMap data;
    data.Add<BuiltinPolyfill::Config>(builtins);
    return data;
}

TEST_F(BuiltinPolyfillTest, ShouldRunIntDiv) {
    auto* src = R"(
fn f() {
  let v = 10i;
  let x = 20i / v;
}
)";

    EXPECT_FALSE(ShouldRun<BuiltinPolyfill>(src));
    EXPECT_TRUE(ShouldRun<BuiltinPolyfill>(src, polyfillIntDivMod()));
}

TEST_F(BuiltinPolyfillTest, ShouldRunIntMod) {
    auto* src = R"(
fn f() {
  let v = 10i;
  let x = 20i % v;
}
)";

    EXPECT_FALSE(ShouldRun<BuiltinPolyfill>(src));
    EXPECT_TRUE(ShouldRun<BuiltinPolyfill>(src, polyfillIntDivMod()));
}

TEST_F(BuiltinPolyfillTest, IntDiv_ai_i32) {
    auto* src = R"(
fn f() {
  let v = 10i;
  let x = 20 / v;
}
)";

    auto* expect = R"(
fn tint_div(lhs : i32, rhs : i32) -> i32 {
  return (lhs / select(rhs, 1, ((rhs == 0) | ((lhs == -2147483648) & (rhs == -1)))));
}

fn f() {
  let v = 10i;
  let x = tint_div(20, v);
}
)";

    auto got = Run<BuiltinPolyfill>(src, polyfillIntDivMod());

    EXPECT_EQ(expect, str(got));
}

TEST_F(BuiltinPolyfillTest, IntMod_ai_i32) {
    auto* src = R"(
fn f() {
  let v = 10i;
  let x = 20 % v;
}
)";

    auto* expect = R"(
fn tint_mod(lhs : i32, rhs : i32) -> i32 {
  return (lhs % select(rhs, 1, ((rhs == 0) | ((lhs == -2147483648) & (rhs == -1)))));
}

fn f() {
  let v = 10i;
  let x = tint_mod(20, v);
}
)";

    auto got = Run<BuiltinPolyfill>(src, polyfillIntDivMod());

    EXPECT_EQ(expect, str(got));
}

TEST_F(BuiltinPolyfillTest, IntDiv_i32_ai) {
    auto* src = R"(
fn f() {
  let v = 10i;
  let x = v / 20;
}
)";

    auto* expect = R"(
fn tint_div(lhs : i32, rhs : i32) -> i32 {
  return (lhs / select(rhs, 1, ((rhs == 0) | ((lhs == -2147483648) & (rhs == -1)))));
}

fn f() {
  let v = 10i;
  let x = tint_div(v, 20);
}
)";

    auto got = Run<BuiltinPolyfill>(src, polyfillIntDivMod());

    EXPECT_EQ(expect, str(got));
}

TEST_F(BuiltinPolyfillTest, IntMod_i32_ai) {
    auto* src = R"(
fn f() {
  let v = 10i;
  let x = v % 20;
}
)";

    auto* expect = R"(
fn tint_mod(lhs : i32, rhs : i32) -> i32 {
  return (lhs % select(rhs, 1, ((rhs == 0) | ((lhs == -2147483648) & (rhs == -1)))));
}

fn f() {
  let v = 10i;
  let x = tint_mod(v, 20);
}
)";

    auto got = Run<BuiltinPolyfill>(src, polyfillIntDivMod());

    EXPECT_EQ(expect, str(got));
}

TEST_F(BuiltinPolyfillTest, IntDiv_i32_i32) {
    auto* src = R"(
fn f() {
  let v = 10i;
  let x = 20i / v;
}
)";

    auto* expect = R"(
fn tint_div(lhs : i32, rhs : i32) -> i32 {
  return (lhs / select(rhs, 1, ((rhs == 0) | ((lhs == -2147483648) & (rhs == -1)))));
}

fn f() {
  let v = 10i;
  let x = tint_div(20i, v);
}
)";

    auto got = Run<BuiltinPolyfill>(src, polyfillIntDivMod());

    EXPECT_EQ(expect, str(got));
}

TEST_F(BuiltinPolyfillTest, IntMod_i32_i32) {
    auto* src = R"(
fn f() {
  let v = 10i;
  let x = 20i % v;
}
)";

    auto* expect = R"(
fn tint_mod(lhs : i32, rhs : i32) -> i32 {
  return (lhs % select(rhs, 1, ((rhs == 0) | ((lhs == -2147483648) & (rhs == -1)))));
}

fn f() {
  let v = 10i;
  let x = tint_mod(20i, v);
}
)";

    auto got = Run<BuiltinPolyfill>(src, polyfillIntDivMod());

    EXPECT_EQ(expect, str(got));
}

TEST_F(BuiltinPolyfillTest, IntDiv_ai_u32) {
    auto* src = R"(
fn f() {
  let v = 10u;
  let x = 20 / v;
}
)";

    auto* expect = R"(
fn tint_div(lhs : u32, rhs : u32) -> u32 {
  return (lhs / select(rhs, 1, (rhs == 0)));
}

fn f() {
  let v = 10u;
  let x = tint_div(20, v);
}
)";

    auto got = Run<BuiltinPolyfill>(src, polyfillIntDivMod());

    EXPECT_EQ(expect, str(got));
}

TEST_F(BuiltinPolyfillTest, IntMod_ai_u32) {
    auto* src = R"(
fn f() {
  let v = 10u;
  let x = 20 % v;
}
)";

    auto* expect = R"(
fn tint_mod(lhs : u32, rhs : u32) -> u32 {
  return (lhs % select(rhs, 1, (rhs == 0)));
}

fn f() {
  let v = 10u;
  let x = tint_mod(20, v);
}
)";

    auto got = Run<BuiltinPolyfill>(src, polyfillIntDivMod());

    EXPECT_EQ(expect, str(got));
}

TEST_F(BuiltinPolyfillTest, IntDiv_u32_ai) {
    auto* src = R"(
fn f() {
  let v = 10u;
  let x = v / 20;
}
)";

    auto* expect = R"(
fn tint_div(lhs : u32, rhs : u32) -> u32 {
  return (lhs / select(rhs, 1, (rhs == 0)));
}

fn f() {
  let v = 10u;
  let x = tint_div(v, 20);
}
)";

    auto got = Run<BuiltinPolyfill>(src, polyfillIntDivMod());

    EXPECT_EQ(expect, str(got));
}

TEST_F(BuiltinPolyfillTest, IntMod_u32_ai) {
    auto* src = R"(
fn f() {
  let v = 10u;
  let x = v % 20;
}
)";

    auto* expect = R"(
fn tint_mod(lhs : u32, rhs : u32) -> u32 {
  return (lhs % select(rhs, 1, (rhs == 0)));
}

fn f() {
  let v = 10u;
  let x = tint_mod(v, 20);
}
)";

    auto got = Run<BuiltinPolyfill>(src, polyfillIntDivMod());

    EXPECT_EQ(expect, str(got));
}

TEST_F(BuiltinPolyfillTest, IntDiv_u32_u32) {
    auto* src = R"(
fn f() {
  let v = 10u;
  let x = 20u / v;
}
)";

    auto* expect = R"(
fn tint_div(lhs : u32, rhs : u32) -> u32 {
  return (lhs / select(rhs, 1, (rhs == 0)));
}

fn f() {
  let v = 10u;
  let x = tint_div(20u, v);
}
)";

    auto got = Run<BuiltinPolyfill>(src, polyfillIntDivMod());

    EXPECT_EQ(expect, str(got));
}

TEST_F(BuiltinPolyfillTest, IntMod_u32_u32) {
    auto* src = R"(
fn f() {
  let v = 10u;
  let x = 20u % v;
}
)";

    auto* expect = R"(
fn tint_mod(lhs : u32, rhs : u32) -> u32 {
  return (lhs % select(rhs, 1, (rhs == 0)));
}

fn f() {
  let v = 10u;
  let x = tint_mod(20u, v);
}
)";

    auto got = Run<BuiltinPolyfill>(src, polyfillIntDivMod());

    EXPECT_EQ(expect, str(got));
}

TEST_F(BuiltinPolyfillTest, IntDiv_vec3_ai_i32) {
    auto* src = R"(
fn f() {
  let v = 10i;
  let x = vec3(20) / v;
}
)";

    auto* expect = R"(
fn tint_div(lhs : vec3<i32>, rhs : i32) -> vec3<i32> {
  let r = vec3<i32>(rhs);
  return (lhs / select(r, vec3(1), ((r == vec3(0)) | ((lhs == vec3(-2147483648)) & (r == vec3(-1))))));
}

fn f() {
  let v = 10i;
  let x = tint_div(vec3(20), v);
}
)";

    auto got = Run<BuiltinPolyfill>(src, polyfillIntDivMod());

    EXPECT_EQ(expect, str(got));
}

TEST_F(BuiltinPolyfillTest, IntMod_vec3_ai_i32) {
    auto* src = R"(
fn f() {
  let v = 10i;
  let x = vec3(20) % v;
}
)";

    auto* expect = R"(
fn tint_mod(lhs : vec3<i32>, rhs : i32) -> vec3<i32> {
  let r = vec3<i32>(rhs);
  return (lhs % select(r, vec3(1), ((r == vec3(0)) | ((lhs == vec3(-2147483648)) & (r == vec3(-1))))));
}

fn f() {
  let v = 10i;
  let x = tint_mod(vec3(20), v);
}
)";

    auto got = Run<BuiltinPolyfill>(src, polyfillIntDivMod());

    EXPECT_EQ(expect, str(got));
}

TEST_F(BuiltinPolyfillTest, IntDiv_vec3_i32_ai) {
    auto* src = R"(
fn f() {
  let v = 10i;
  let x = vec3(v) / 20;
}
)";

    auto* expect = R"(
fn tint_div(lhs : vec3<i32>, rhs : i32) -> vec3<i32> {
  let r = vec3<i32>(rhs);
  return (lhs / select(r, vec3(1), ((r == vec3(0)) | ((lhs == vec3(-2147483648)) & (r == vec3(-1))))));
}

fn f() {
  let v = 10i;
  let x = tint_div(vec3(v), 20);
}
)";

    auto got = Run<BuiltinPolyfill>(src, polyfillIntDivMod());

    EXPECT_EQ(expect, str(got));
}

TEST_F(BuiltinPolyfillTest, IntMod_vec3_i32_ai) {
    auto* src = R"(
fn f() {
  let v = 10i;
  let x = vec3(v) % 20;
}
)";

    auto* expect = R"(
fn tint_mod(lhs : vec3<i32>, rhs : i32) -> vec3<i32> {
  let r = vec3<i32>(rhs);
  return (lhs % select(r, vec3(1), ((r == vec3(0)) | ((lhs == vec3(-2147483648)) & (r == vec3(-1))))));
}

fn f() {
  let v = 10i;
  let x = tint_mod(vec3(v), 20);
}
)";

    auto got = Run<BuiltinPolyfill>(src, polyfillIntDivMod());

    EXPECT_EQ(expect, str(got));
}

TEST_F(BuiltinPolyfillTest, IntDiv_vec3_i32_i32) {
    auto* src = R"(
fn f() {
  let v = 10i;
  let x = vec3<i32>(20i) / v;
}
)";

    auto* expect = R"(
fn tint_div(lhs : vec3<i32>, rhs : i32) -> vec3<i32> {
  let r = vec3<i32>(rhs);
  return (lhs / select(r, vec3(1), ((r == vec3(0)) | ((lhs == vec3(-2147483648)) & (r == vec3(-1))))));
}

fn f() {
  let v = 10i;
  let x = tint_div(vec3<i32>(20i), v);
}
)";

    auto got = Run<BuiltinPolyfill>(src, polyfillIntDivMod());

    EXPECT_EQ(expect, str(got));
}

TEST_F(BuiltinPolyfillTest, IntMod_vec3_i32_i32) {
    auto* src = R"(
fn f() {
  let v = 10i;
  let x = vec3<i32>(20i) % v;
}
)";

    auto* expect = R"(
fn tint_mod(lhs : vec3<i32>, rhs : i32) -> vec3<i32> {
  let r = vec3<i32>(rhs);
  return (lhs % select(r, vec3(1), ((r == vec3(0)) | ((lhs == vec3(-2147483648)) & (r == vec3(-1))))));
}

fn f() {
  let v = 10i;
  let x = tint_mod(vec3<i32>(20i), v);
}
)";

    auto got = Run<BuiltinPolyfill>(src, polyfillIntDivMod());

    EXPECT_EQ(expect, str(got));
}

TEST_F(BuiltinPolyfillTest, IntDiv_vec3_u32_u32) {
    auto* src = R"(
fn f() {
  let v = 10u;
  let x = vec3<u32>(20u) / v;
}
)";

    auto* expect = R"(
fn tint_div(lhs : vec3<u32>, rhs : u32) -> vec3<u32> {
  let r = vec3<u32>(rhs);
  return (lhs / select(r, vec3(1), (r == vec3(0))));
}

fn f() {
  let v = 10u;
  let x = tint_div(vec3<u32>(20u), v);
}
)";

    auto got = Run<BuiltinPolyfill>(src, polyfillIntDivMod());

    EXPECT_EQ(expect, str(got));
}

TEST_F(BuiltinPolyfillTest, IntMod_vec3_u32_u32) {
    auto* src = R"(
fn f() {
  let v = 10u;
  let x = vec3<u32>(20u) % v;
}
)";

    auto* expect = R"(
fn tint_mod(lhs : vec3<u32>, rhs : u32) -> vec3<u32> {
  let r = vec3<u32>(rhs);
  return (lhs % select(r, vec3(1), (r == vec3(0))));
}

fn f() {
  let v = 10u;
  let x = tint_mod(vec3<u32>(20u), v);
}
)";

    auto got = Run<BuiltinPolyfill>(src, polyfillIntDivMod());

    EXPECT_EQ(expect, str(got));
}

TEST_F(BuiltinPolyfillTest, IntDiv_ai_vec3_i32) {
    auto* src = R"(
fn f() {
  let v = 10i;
  let x = 20 / vec3(v);
}
)";

    auto* expect = R"(
fn tint_div(lhs : i32, rhs : vec3<i32>) -> vec3<i32> {
  let l = vec3<i32>(lhs);
  return (l / select(rhs, vec3(1), ((rhs == vec3(0)) | ((l == vec3(-2147483648)) & (rhs == vec3(-1))))));
}

fn f() {
  let v = 10i;
  let x = tint_div(20, vec3(v));
}
)";

    auto got = Run<BuiltinPolyfill>(src, polyfillIntDivMod());

    EXPECT_EQ(expect, str(got));
}

TEST_F(BuiltinPolyfillTest, IntMod_ai_vec3_i32) {
    auto* src = R"(
fn f() {
  let v = 10i;
  let x = 20 % vec3(v);
}
)";

    auto* expect = R"(
fn tint_mod(lhs : i32, rhs : vec3<i32>) -> vec3<i32> {
  let l = vec3<i32>(lhs);
  return (l % select(rhs, vec3(1), ((rhs == vec3(0)) | ((l == vec3(-2147483648)) & (rhs == vec3(-1))))));
}

fn f() {
  let v = 10i;
  let x = tint_mod(20, vec3(v));
}
)";

    auto got = Run<BuiltinPolyfill>(src, polyfillIntDivMod());

    EXPECT_EQ(expect, str(got));
}

TEST_F(BuiltinPolyfillTest, IntDiv_i32_vec3_i32) {
    auto* src = R"(
fn f() {
  let v = 10i;
  let x = 20i / vec3<i32>(v);
}
)";

    auto* expect = R"(
fn tint_div(lhs : i32, rhs : vec3<i32>) -> vec3<i32> {
  let l = vec3<i32>(lhs);
  return (l / select(rhs, vec3(1), ((rhs == vec3(0)) | ((l == vec3(-2147483648)) & (rhs == vec3(-1))))));
}

fn f() {
  let v = 10i;
  let x = tint_div(20i, vec3<i32>(v));
}
)";

    auto got = Run<BuiltinPolyfill>(src, polyfillIntDivMod());

    EXPECT_EQ(expect, str(got));
}

TEST_F(BuiltinPolyfillTest, IntMod_i32_vec3_i32) {
    auto* src = R"(
fn f() {
  let v = 10i;
  let x = 20i % vec3<i32>(v);
}
)";

    auto* expect = R"(
fn tint_mod(lhs : i32, rhs : vec3<i32>) -> vec3<i32> {
  let l = vec3<i32>(lhs);
  return (l % select(rhs, vec3(1), ((rhs == vec3(0)) | ((l == vec3(-2147483648)) & (rhs == vec3(-1))))));
}

fn f() {
  let v = 10i;
  let x = tint_mod(20i, vec3<i32>(v));
}
)";

    auto got = Run<BuiltinPolyfill>(src, polyfillIntDivMod());

    EXPECT_EQ(expect, str(got));
}

TEST_F(BuiltinPolyfillTest, IntDiv_u32_vec3_u32) {
    auto* src = R"(
fn f() {
  let v = 10u;
  let x = 20u / vec3<u32>(v);
}
)";

    auto* expect = R"(
fn tint_div(lhs : u32, rhs : vec3<u32>) -> vec3<u32> {
  let l = vec3<u32>(lhs);
  return (l / select(rhs, vec3(1), (rhs == vec3(0))));
}

fn f() {
  let v = 10u;
  let x = tint_div(20u, vec3<u32>(v));
}
)";

    auto got = Run<BuiltinPolyfill>(src, polyfillIntDivMod());

    EXPECT_EQ(expect, str(got));
}

TEST_F(BuiltinPolyfillTest, IntMod_u32_vec3_u32) {
    auto* src = R"(
fn f() {
  let v = 10u;
  let x = 20u % vec3<u32>(v);
}
)";

    auto* expect = R"(
fn tint_mod(lhs : u32, rhs : vec3<u32>) -> vec3<u32> {
  let l = vec3<u32>(lhs);
  return (l % select(rhs, vec3(1), (rhs == vec3(0))));
}

fn f() {
  let v = 10u;
  let x = tint_mod(20u, vec3<u32>(v));
}
)";

    auto got = Run<BuiltinPolyfill>(src, polyfillIntDivMod());

    EXPECT_EQ(expect, str(got));
}

TEST_F(BuiltinPolyfillTest, IntDiv_vec3_i32_vec3_i32) {
    auto* src = R"(
fn f() {
  let v = 10i;
  let x = vec3<i32>(20i) / vec3<i32>(v);
}
)";

    auto* expect = R"(
fn tint_div(lhs : vec3<i32>, rhs : vec3<i32>) -> vec3<i32> {
  return (lhs / select(rhs, vec3(1), ((rhs == vec3(0)) | ((lhs == vec3(-2147483648)) & (rhs == vec3(-1))))));
}

fn f() {
  let v = 10i;
  let x = tint_div(vec3<i32>(20i), vec3<i32>(v));
}
)";

    auto got = Run<BuiltinPolyfill>(src, polyfillIntDivMod());

    EXPECT_EQ(expect, str(got));
}

TEST_F(BuiltinPolyfillTest, IntMod_vec3_i32_vec3_i32) {
    auto* src = R"(
fn f() {
  let v = 10i;
  let x = vec3<i32>(20i) % vec3<i32>(v);
}
)";

    auto* expect = R"(
fn tint_mod(lhs : vec3<i32>, rhs : vec3<i32>) -> vec3<i32> {
  return (lhs % select(rhs, vec3(1), ((rhs == vec3(0)) | ((lhs == vec3(-2147483648)) & (rhs == vec3(-1))))));
}

fn f() {
  let v = 10i;
  let x = tint_mod(vec3<i32>(20i), vec3<i32>(v));
}
)";

    auto got = Run<BuiltinPolyfill>(src, polyfillIntDivMod());

    EXPECT_EQ(expect, str(got));
}

TEST_F(BuiltinPolyfillTest, IntDiv_vec3_u32_vec3_u32) {
    auto* src = R"(
fn f() {
  let v = 10u;
  let x = vec3<u32>(20u) / vec3<u32>(v);
}
)";

    auto* expect = R"(
fn tint_div(lhs : vec3<u32>, rhs : vec3<u32>) -> vec3<u32> {
  return (lhs / select(rhs, vec3(1), (rhs == vec3(0))));
}

fn f() {
  let v = 10u;
  let x = tint_div(vec3<u32>(20u), vec3<u32>(v));
}
)";

    auto got = Run<BuiltinPolyfill>(src, polyfillIntDivMod());

    EXPECT_EQ(expect, str(got));
}

TEST_F(BuiltinPolyfillTest, IntMod_vec3_u32_vec3_u32) {
    auto* src = R"(
fn f() {
  let v = 10u;
  let x = vec3<u32>(20u) % vec3<u32>(v);
}
)";

    auto* expect = R"(
fn tint_mod(lhs : vec3<u32>, rhs : vec3<u32>) -> vec3<u32> {
  return (lhs % select(rhs, vec3(1), (rhs == vec3(0))));
}

fn f() {
  let v = 10u;
  let x = tint_mod(vec3<u32>(20u), vec3<u32>(v));
}
)";

    auto got = Run<BuiltinPolyfill>(src, polyfillIntDivMod());

    EXPECT_EQ(expect, str(got));
}

////////////////////////////////////////////////////////////////////////////////
// saturate
////////////////////////////////////////////////////////////////////////////////
DataMap polyfillSaturate() {
    BuiltinPolyfill::Builtins builtins;
    builtins.saturate = true;
    DataMap data;
    data.Add<BuiltinPolyfill::Config>(builtins);
    return data;
}

TEST_F(BuiltinPolyfillTest, ShouldRunSaturate) {
    auto* src = R"(
fn f() {
  let v = 0.5f;
  saturate(v);
}
)";

    EXPECT_FALSE(ShouldRun<BuiltinPolyfill>(src));
    EXPECT_TRUE(ShouldRun<BuiltinPolyfill>(src, polyfillSaturate()));
}

TEST_F(BuiltinPolyfillTest, Saturate_ConstantExpression) {
    auto* src = R"(
fn f() {
  let r : f32 = saturate(0.5);
}
)";

    auto* expect = src;

    auto got = Run<BuiltinPolyfill>(src, polyfillSaturate());

    EXPECT_EQ(expect, str(got));
}

TEST_F(BuiltinPolyfillTest, Saturate_f32) {
    auto* src = R"(
fn f() {
  let v = 0.5f;
  let r : f32 = saturate(v);
}
)";

    auto* expect = R"(
fn tint_saturate(v : f32) -> f32 {
  return clamp(v, f32(0), f32(1));
}

fn f() {
  let v = 0.5f;
  let r : f32 = tint_saturate(v);
}
)";

    auto got = Run<BuiltinPolyfill>(src, polyfillSaturate());

    EXPECT_EQ(expect, str(got));
}

TEST_F(BuiltinPolyfillTest, Saturate_f16) {
    auto* src = R"(
enable f16;

fn f() {
  let v = 0.5h;
  let r : f16 = saturate(v);
}
)";

    auto* expect = R"(
enable f16;

fn tint_saturate(v : f16) -> f16 {
  return clamp(v, f16(0), f16(1));
}

fn f() {
  let v = 0.5h;
  let r : f16 = tint_saturate(v);
}
)";

    auto got = Run<BuiltinPolyfill>(src, polyfillSaturate());

    EXPECT_EQ(expect, str(got));
}

TEST_F(BuiltinPolyfillTest, Saturate_vec3_f32) {
    auto* src = R"(
fn f() {
  let v = 0.5f;
  let r : vec3<f32> = saturate(vec3<f32>(v));
}
)";

    auto* expect = R"(
fn tint_saturate(v : vec3<f32>) -> vec3<f32> {
  return clamp(v, vec3<f32>(0), vec3<f32>(1));
}

fn f() {
  let v = 0.5f;
  let r : vec3<f32> = tint_saturate(vec3<f32>(v));
}
)";

    auto got = Run<BuiltinPolyfill>(src, polyfillSaturate());

    EXPECT_EQ(expect, str(got));
}

TEST_F(BuiltinPolyfillTest, Saturate_vec3_f16) {
    auto* src = R"(
enable f16;

fn f() {
  let v = 0.5h;
  let r : vec3<f16> = saturate(vec3<f16>(v));
}
)";

    auto* expect = R"(
enable f16;

fn tint_saturate(v : vec3<f16>) -> vec3<f16> {
  return clamp(v, vec3<f16>(0), vec3<f16>(1));
}

fn f() {
  let v = 0.5h;
  let r : vec3<f16> = tint_saturate(vec3<f16>(v));
}
)";

    auto got = Run<BuiltinPolyfill>(src, polyfillSaturate());

    EXPECT_EQ(expect, str(got));
}

////////////////////////////////////////////////////////////////////////////////
// textureSampleBaseClampToEdge
////////////////////////////////////////////////////////////////////////////////
DataMap polyfillTextureSampleBaseClampToEdge_2d_f32() {
    BuiltinPolyfill::Builtins builtins;
    builtins.texture_sample_base_clamp_to_edge_2d_f32 = true;
    DataMap data;
    data.Add<BuiltinPolyfill::Config>(builtins);
    return data;
}

TEST_F(BuiltinPolyfillTest, ShouldRunTextureSampleBaseClampToEdge_2d_f32) {
    auto* src = R"(
@group(0) @binding(0) var t : texture_2d<f32>;
@group(0) @binding(1) var s : sampler;

fn f() {
  textureSampleBaseClampToEdge(t, s, vec2<f32>(0.5));
}
)";

    EXPECT_FALSE(ShouldRun<BuiltinPolyfill>(src));
    EXPECT_TRUE(ShouldRun<BuiltinPolyfill>(src, polyfillTextureSampleBaseClampToEdge_2d_f32()));
}

TEST_F(BuiltinPolyfillTest, ShouldRunTextureSampleBaseClampToEdge_external) {
    auto* src = R"(
@group(0) @binding(0) var t : texture_external;
@group(0) @binding(1) var s : sampler;

fn f() {
  textureSampleBaseClampToEdge(t, s, vec2<f32>(0.5));
}
)";

    EXPECT_FALSE(ShouldRun<BuiltinPolyfill>(src));
    EXPECT_FALSE(ShouldRun<BuiltinPolyfill>(src, polyfillTextureSampleBaseClampToEdge_2d_f32()));
}

TEST_F(BuiltinPolyfillTest, TextureSampleBaseClampToEdge_2d_f32_f32) {
    auto* src = R"(
@group(0) @binding(0) var t : texture_2d<f32>;
@group(0) @binding(1) var s : sampler;

fn f() {
  let r = textureSampleBaseClampToEdge(t, s, vec2<f32>(0.5));
}
)";

    auto* expect = R"(
fn tint_textureSampleBaseClampToEdge(t : texture_2d<f32>, s : sampler, coord : vec2<f32>) -> vec4<f32> {
  let dims = vec2<f32>(textureDimensions(t, 0));
  let half_texel = (vec2<f32>(0.5) / dims);
  let clamped = clamp(coord, half_texel, (1 - half_texel));
  return textureSampleLevel(t, s, clamped, 0);
}

@group(0) @binding(0) var t : texture_2d<f32>;

@group(0) @binding(1) var s : sampler;

fn f() {
  let r = tint_textureSampleBaseClampToEdge(t, s, vec2<f32>(0.5));
}
)";

    auto got = Run<BuiltinPolyfill>(src, polyfillTextureSampleBaseClampToEdge_2d_f32());

    EXPECT_EQ(expect, str(got));
}

////////////////////////////////////////////////////////////////////////////////
// quantizeToF16
////////////////////////////////////////////////////////////////////////////////
DataMap polyfillQuantizeToF16_2d_f32() {
    BuiltinPolyfill::Builtins builtins;
    builtins.quantize_to_vec_f16 = true;
    DataMap data;
    data.Add<BuiltinPolyfill::Config>(builtins);
    return data;
}

TEST_F(BuiltinPolyfillTest, ShouldRunQuantizeToF16_Scalar) {
    auto* src = R"(
fn f() {
  let v = 0.5;
  quantizeToF16(0.5);
}
)";

    EXPECT_FALSE(ShouldRun<BuiltinPolyfill>(src));
    EXPECT_FALSE(ShouldRun<BuiltinPolyfill>(src, polyfillQuantizeToF16_2d_f32()));
}

TEST_F(BuiltinPolyfillTest, ShouldRunQuantizeToF16_Vector) {
    auto* src = R"(
fn f() {
  let v = 0.5;
  quantizeToF16(vec2(v));
}
)";

    EXPECT_FALSE(ShouldRun<BuiltinPolyfill>(src));
    EXPECT_TRUE(ShouldRun<BuiltinPolyfill>(src, polyfillQuantizeToF16_2d_f32()));
}

TEST_F(BuiltinPolyfillTest, QuantizeToF16_Vec2) {
    auto* src = R"(
fn f() {
  let v = 0.5;
  quantizeToF16(vec2(v));
}
)";

    auto* expect = R"(
fn tint_quantizeToF16(v : vec2<f32>) -> vec2<f32> {
  return vec2<f32>(quantizeToF16(v[0u]), quantizeToF16(v[1u]));
}

fn f() {
  let v = 0.5;
  tint_quantizeToF16(vec2(v));
}
)";

    auto got = Run<BuiltinPolyfill>(src, polyfillQuantizeToF16_2d_f32());

    EXPECT_EQ(expect, str(got));
}

TEST_F(BuiltinPolyfillTest, QuantizeToF16_Vec3) {
    auto* src = R"(
fn f() {
  let v = 0.5;
  quantizeToF16(vec3(v));
}
)";

    auto* expect = R"(
fn tint_quantizeToF16(v : vec3<f32>) -> vec3<f32> {
  return vec3<f32>(quantizeToF16(v[0u]), quantizeToF16(v[1u]), quantizeToF16(v[2u]));
}

fn f() {
  let v = 0.5;
  tint_quantizeToF16(vec3(v));
}
)";

    auto got = Run<BuiltinPolyfill>(src, polyfillQuantizeToF16_2d_f32());

    EXPECT_EQ(expect, str(got));
}

TEST_F(BuiltinPolyfillTest, QuantizeToF16_Vec4) {
    auto* src = R"(
fn f() {
  let v = 0.5;
  quantizeToF16(vec4(v));
}
)";

    auto* expect = R"(
fn tint_quantizeToF16(v : vec4<f32>) -> vec4<f32> {
  return vec4<f32>(quantizeToF16(v[0u]), quantizeToF16(v[1u]), quantizeToF16(v[2u]), quantizeToF16(v[3u]));
}

fn f() {
  let v = 0.5;
  tint_quantizeToF16(vec4(v));
}
)";

    auto got = Run<BuiltinPolyfill>(src, polyfillQuantizeToF16_2d_f32());

    EXPECT_EQ(expect, str(got));
}

////////////////////////////////////////////////////////////////////////////////
// Polyfill combinations
////////////////////////////////////////////////////////////////////////////////

TEST_F(BuiltinPolyfillTest, BitshiftAndModulo) {
    auto* src = R"(
fn f(x : i32, y : u32, z : u32) {
    let l = x << (y % z);
}
)";

    auto* expect = R"(
fn tint_mod(lhs : u32, rhs : u32) -> u32 {
  return (lhs % select(rhs, 1, (rhs == 0)));
}

fn f(x : i32, y : u32, z : u32) {
  let l = (x << (tint_mod(y, z) & 31));
}
)";

    BuiltinPolyfill::Builtins builtins;
    builtins.bitshift_modulo = true;
    builtins.int_div_mod = true;
    DataMap data;
    data.Add<BuiltinPolyfill::Config>(builtins);

    auto got = Run<BuiltinPolyfill>(src, std::move(data));

    EXPECT_EQ(expect, str(got));
}

}  // namespace
}  // namespace tint::transform
