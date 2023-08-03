// Copyright 2023 The Tint Authors.
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

#include "src/tint/lang/msl/writer/printer/helper_test.h"

namespace tint::msl::writer {
namespace {

using namespace tint::builtin::fluent_types;  // NOLINT
using namespace tint::number_suffixes;        // NOLINT

TEST_F(MslPrinterTest, LetU32) {
    auto* func = b.Function("foo", ty.void_());
    b.Append(func->Block(), [&] {
        b.Let("l", 42_u);
        b.Return(func);
    });

    ASSERT_TRUE(generator_.Generate()) << generator_.Diagnostics().str();
    EXPECT_EQ(generator_.Result(), MetalHeader() + R"(
void foo() {
  uint const l = 42u;
}
)");
}

TEST_F(MslPrinterTest, LetDuplicate) {
    auto* func = b.Function("foo", ty.void_());
    b.Append(func->Block(), [&] {
        b.Let("l1", 42_u);
        b.Let("l2", 42_u);
        b.Return(func);
    });

    ASSERT_TRUE(generator_.Generate()) << generator_.Diagnostics().str();
    EXPECT_EQ(generator_.Result(), MetalHeader() + R"(
void foo() {
  uint const l1 = 42u;
  uint const l2 = 42u;
}
)");
}

TEST_F(MslPrinterTest, LetF32) {
    auto* func = b.Function("foo", ty.void_());
    b.Append(func->Block(), [&] {
        b.Let("l", 42.0_f);
        b.Return(func);
    });

    ASSERT_TRUE(generator_.Generate()) << generator_.Diagnostics().str();
    EXPECT_EQ(generator_.Result(), MetalHeader() + R"(
void foo() {
  float const l = 42.0f;
}
)");
}

TEST_F(MslPrinterTest, LetI32) {
    auto* func = b.Function("foo", ty.void_());
    b.Append(func->Block(), [&] {
        b.Let("l", 42_i);
        b.Return(func);
    });

    ASSERT_TRUE(generator_.Generate()) << generator_.Diagnostics().str();
    EXPECT_EQ(generator_.Result(), MetalHeader() + R"(
void foo() {
  int const l = 42;
}
)");
}

TEST_F(MslPrinterTest, LetF16) {
    // Enable F16?
    auto* func = b.Function("foo", ty.void_());
    b.Append(func->Block(), [&] {
        b.Let("l", 42_h);
        b.Return(func);
    });

    ASSERT_TRUE(generator_.Generate()) << generator_.Diagnostics().str();
    EXPECT_EQ(generator_.Result(), MetalHeader() + R"(
void foo() {
  half const l = 42.0h;
}
)");
}

TEST_F(MslPrinterTest, LetVec3F32) {
    auto* func = b.Function("foo", ty.void_());
    b.Append(func->Block(), [&] {
        b.Let("l", b.Composite(ty.vec3<f32>(), 1_f, 2_f, 3_f));
        b.Return(func);
    });

    ASSERT_TRUE(generator_.Generate()) << generator_.Diagnostics().str();
    EXPECT_EQ(generator_.Result(), MetalHeader() + R"(
void foo() {
  float3 const l = float3(1.0f, 2.0f, 3.0f);
}
)");
}

TEST_F(MslPrinterTest, LetVec3F16) {
    // Enable f16?
    auto* func = b.Function("foo", ty.void_());
    b.Append(func->Block(), [&] {
        b.Let("l", b.Composite(ty.vec3<f16>(), 1_h, 2_h, 3_h));
        b.Return(func);
    });

    ASSERT_TRUE(generator_.Generate()) << generator_.Diagnostics().str();
    EXPECT_EQ(generator_.Result(), MetalHeader() + R"(
void foo() {
  half3 const l = half3(1.0h, 2.0h, 3.0h);
}
)");
}

TEST_F(MslPrinterTest, LetMat2x3F32) {
    auto* func = b.Function("foo", ty.void_());
    b.Append(func->Block(), [&] {
        b.Let("l", b.Composite(ty.mat2x3<f32>(),  //
                               b.Composite(ty.vec3<f32>(), 1_f, 2_f, 3_f),
                               b.Composite(ty.vec3<f32>(), 4_f, 5_f, 6_f)));
        b.Return(func);
    });

    ASSERT_TRUE(generator_.Generate()) << generator_.Diagnostics().str();
    EXPECT_EQ(generator_.Result(), MetalHeader() + R"(
void foo() {
  float2x3 const l = float2x3(float3(1.0f, 2.0f, 3.0f), float3(4.0f, 5.0f, 6.0f));
}
)");
}

TEST_F(MslPrinterTest, LetMat2x3F16) {
    // Enable f16?
    auto* func = b.Function("foo", ty.void_());
    b.Append(func->Block(), [&] {
        b.Let("l", b.Composite(ty.mat2x3<f16>(),  //
                               b.Composite(ty.vec3<f16>(), 1_h, 2_h, 3_h),
                               b.Composite(ty.vec3<f16>(), 4_h, 5_h, 6_h)));
        b.Return(func);
    });

    ASSERT_TRUE(generator_.Generate()) << generator_.Diagnostics().str();
    EXPECT_EQ(generator_.Result(), MetalHeader() + R"(
void foo() {
  half2x3 const l = half2x3(half3(1.0h, 2.0h, 3.0h), half3(4.0h, 5.0h, 6.0h));
}
)");
}

TEST_F(MslPrinterTest, LetArrF32) {
    auto* func = b.Function("foo", ty.void_());
    b.Append(func->Block(), [&] {
        b.Let("l", b.Composite(ty.array<f32, 3>(), 1_f, 2_f, 3_f));
        b.Return(func);
    });

    ASSERT_TRUE(generator_.Generate()) << generator_.Diagnostics().str();
    EXPECT_EQ(generator_.Result(), MetalHeader() + MetalArray() + R"(
void foo() {
  tint_array<float, 3> const l = tint_array<float, 3>{1.0f, 2.0f, 3.0f};
}
)");
}

TEST_F(MslPrinterTest, LetArrVec2Bool) {
    auto* func = b.Function("foo", ty.void_());
    b.Append(func->Block(), [&] {
        b.Let("l", b.Composite(ty.array<vec2<bool>, 3>(),  //
                               b.Composite(ty.vec2<bool>(), true, false),
                               b.Composite(ty.vec2<bool>(), false, true),
                               b.Composite(ty.vec2<bool>(), true, false)));
        b.Return(func);
    });

    ASSERT_TRUE(generator_.Generate()) << generator_.Diagnostics().str();
    EXPECT_EQ(generator_.Result(), MetalHeader() + MetalArray() + R"(
void foo() {
  tint_array<bool2, 3> const l = tint_array<bool2, 3>{bool2(true, false), bool2(false, true), bool2(true, false)};
}
)");
}

}  // namespace
}  // namespace tint::msl::writer
