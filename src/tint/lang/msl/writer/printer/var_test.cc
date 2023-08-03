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

TEST_F(MslPrinterTest, VarF32) {
    auto* func = b.Function("foo", ty.void_());
    b.Append(func->Block(), [&] {
        b.Var("a", ty.ptr<builtin::AddressSpace::kFunction, f32>());
        b.Return(func);
    });

    ASSERT_TRUE(generator_.Generate()) << generator_.Diagnostics().str();
    EXPECT_EQ(generator_.Result(), MetalHeader() + R"(
void foo() {
  float a = 0.0f;
}
)");
}

TEST_F(MslPrinterTest, VarI32) {
    auto* func = b.Function("foo", ty.void_());
    b.Append(func->Block(), [&] {
        auto* v = b.Var("a", ty.ptr<builtin::AddressSpace::kFunction, i32>());
        v->SetInitializer(b.Constant(1_i));
        b.Return(func);
    });

    ASSERT_TRUE(generator_.Generate()) << generator_.Diagnostics().str();
    EXPECT_EQ(generator_.Result(), MetalHeader() + R"(
void foo() {
  int a = 1;
}
)");
}

TEST_F(MslPrinterTest, VarU32) {
    auto* func = b.Function("foo", ty.void_());
    b.Append(func->Block(), [&] {
        auto* v = b.Var("a", ty.ptr<builtin::AddressSpace::kFunction, u32>());
        v->SetInitializer(b.Constant(1_u));
        b.Return(func);
    });

    ASSERT_TRUE(generator_.Generate()) << generator_.Diagnostics().str();
    EXPECT_EQ(generator_.Result(), MetalHeader() + R"(
void foo() {
  uint a = 1u;
}
)");
}

TEST_F(MslPrinterTest, VarArrayF32) {
    auto* func = b.Function("foo", ty.void_());
    b.Append(func->Block(), [&] {
        b.Var("a", ty.ptr<builtin::AddressSpace::kFunction, array<f32, 5>>());
        b.Return(func);
    });

    ASSERT_TRUE(generator_.Generate()) << generator_.Diagnostics().str();
    EXPECT_EQ(generator_.Result(), MetalHeader() + MetalArray() + R"(
void foo() {
  tint_array<float, 5> a = {};
}
)");
}

TEST_F(MslPrinterTest, VarStruct) {
    auto* s = ty.Struct(mod.symbols.New("MyStruct"), {{mod.symbols.Register("a"), ty.f32()},  //
                                                      {mod.symbols.Register("b"), ty.vec4<i32>()}});

    auto* func = b.Function("foo", ty.void_());
    b.Append(func->Block(), [&] {
        b.Var("a", ty.ptr(builtin::AddressSpace::kFunction, s));
        b.Return(func);
    });

    ASSERT_TRUE(generator_.Generate()) << generator_.Diagnostics().str();
    EXPECT_EQ(generator_.Result(), MetalHeader() + R"(struct MyStruct {
  float a;
  int4 b;
};

void foo() {
  MyStruct a = {};
}
)");
}

TEST_F(MslPrinterTest, VarVecF32) {
    auto* func = b.Function("foo", ty.void_());
    b.Append(func->Block(), [&] {
        b.Var("a", ty.ptr<builtin::AddressSpace::kFunction, vec2<f32>>());
        b.Return(func);
    });

    ASSERT_TRUE(generator_.Generate()) << generator_.Diagnostics().str();
    EXPECT_EQ(generator_.Result(), MetalHeader() + R"(
void foo() {
  float2 a = 0.0f;
}
)");
}

TEST_F(MslPrinterTest, VarVecF16) {
    // Enable f16?
    auto* func = b.Function("foo", ty.void_());
    b.Append(func->Block(), [&] {
        b.Var("a", ty.ptr<builtin::AddressSpace::kFunction, vec2<f16>>());
        b.Return(func);
    });

    ASSERT_TRUE(generator_.Generate()) << generator_.Diagnostics().str();
    EXPECT_EQ(generator_.Result(), MetalHeader() + R"(
void foo() {
  half2 a = 0.0h;
}
)");
}

TEST_F(MslPrinterTest, VarMatF32) {
    auto* func = b.Function("foo", ty.void_());
    b.Append(func->Block(), [&] {
        b.Var("a", ty.ptr<builtin::AddressSpace::kFunction, mat3x2<f32>>());
        b.Return(func);
    });

    ASSERT_TRUE(generator_.Generate()) << generator_.Diagnostics().str();
    EXPECT_EQ(generator_.Result(), MetalHeader() + R"(
void foo() {
  float3x2 a = float3x2(0.0f);
}
)");
}

TEST_F(MslPrinterTest, VarMatF16) {
    // Enable f16?
    auto* func = b.Function("foo", ty.void_());
    b.Append(func->Block(), [&] {
        b.Var("a", ty.ptr<builtin::AddressSpace::kFunction, mat3x2<f16>>());
        b.Return(func);
    });

    ASSERT_TRUE(generator_.Generate()) << generator_.Diagnostics().str();
    EXPECT_EQ(generator_.Result(), MetalHeader() + R"(
void foo() {
  half3x2 a = half3x2(0.0h);
}
)");
}

TEST_F(MslPrinterTest, VarVecF32SplatZero) {
    auto* func = b.Function("foo", ty.void_());
    b.Append(func->Block(), [&] {
        auto* v = b.Var("a", ty.ptr<builtin::AddressSpace::kFunction, vec3<f32>>());
        v->SetInitializer(b.Splat(ty.vec3<f32>(), 0_f, 3));
        b.Return(func);
    });

    ASSERT_TRUE(generator_.Generate()) << generator_.Diagnostics().str();
    EXPECT_EQ(generator_.Result(), MetalHeader() + R"(
void foo() {
  float3 a = float3(0.0f);
}
)");
}

TEST_F(MslPrinterTest, VarVecF16SplatZero) {
    // Enable f16
    auto* func = b.Function("foo", ty.void_());
    b.Append(func->Block(), [&] {
        auto* v = b.Var("a", ty.ptr<builtin::AddressSpace::kFunction, vec3<f16>>());
        v->SetInitializer(b.Splat(ty.vec3<f16>(), 0_h, 3));
        b.Return(func);
    });

    ASSERT_TRUE(generator_.Generate()) << generator_.Diagnostics().str();
    EXPECT_EQ(generator_.Result(), MetalHeader() + R"(
void foo() {
  half3 a = half3(0.0h);
}
)");
}

TEST_F(MslPrinterTest, VarMatF32SplatZero) {
    auto* func = b.Function("foo", ty.void_());
    b.Append(func->Block(), [&] {
        auto* v = b.Var("a", ty.ptr<builtin::AddressSpace::kFunction, mat2x3<f32>>());
        v->SetInitializer(b.Composite(ty.mat2x3<f32>(), b.Splat(ty.vec3<f32>(), 0_f, 3),
                                      b.Splat(ty.vec3<f32>(), 0_f, 3)));
        b.Return(func);
    });

    ASSERT_TRUE(generator_.Generate()) << generator_.Diagnostics().str();
    EXPECT_EQ(generator_.Result(), MetalHeader() + R"(
void foo() {
  float2x3 a = float2x3(float3(0.0f), float3(0.0f));
}
)");
}

TEST_F(MslPrinterTest, VarMatF16SplatZero) {
    // Enable f16?
    auto* func = b.Function("foo", ty.void_());
    b.Append(func->Block(), [&] {
        auto* v = b.Var("a", ty.ptr<builtin::AddressSpace::kFunction, mat2x3<f16>>());
        v->SetInitializer(b.Composite(ty.mat2x3<f16>(), b.Splat(ty.vec3<f16>(), 0_h, 3),
                                      b.Splat(ty.vec3<f16>(), 0_h, 3)));
        b.Return(func);
    });

    ASSERT_TRUE(generator_.Generate()) << generator_.Diagnostics().str();
    EXPECT_EQ(generator_.Result(), MetalHeader() + R"(
void foo() {
  half2x3 a = half2x3(half3(0.0h), half3(0.0h));
}
)");
}

// TODO(dsinclair): Requires ModuleScopeVarToEntryPointParam transform
TEST_F(MslPrinterTest, DISABLED_VarGlobalPrivate) {
    ir::Var* v = nullptr;
    b.Append(b.RootBlock(),
             [&] { v = b.Var("v", ty.ptr<builtin::AddressSpace::kPrivate, f32>()); });

    auto* func = b.Function("foo", ty.void_());
    b.Append(func->Block(), [&] {
        auto* ld = b.Load(v->Result());
        auto* a = b.Var("a", ty.ptr<builtin::AddressSpace::kFunction, f32>());
        a->SetInitializer(ld->Result());
        b.Return(func);
    });

    ASSERT_TRUE(generator_.Generate()) << generator_.Diagnostics().str();
    EXPECT_EQ(generator_.Result(), MetalHeader() + R"(
struct tint_private_vars_struct {
  float a;
};

void foo() {
    thread tint_private_vars_struct tint_private_vars = {};
    float const a = tint_private_vars.a;
    return;
}
)");
}

TEST_F(MslPrinterTest, VarGlobalWorkgroup) {
    ir::Var* v = nullptr;
    b.Append(b.RootBlock(),
             [&] { v = b.Var("v", ty.ptr<builtin::AddressSpace::kWorkgroup, f32>()); });

    auto* func = b.Function("foo", ty.void_());
    b.Append(func->Block(), [&] {
        auto* ld = b.Load(v->Result());
        auto* a = b.Var("a", ty.ptr<builtin::AddressSpace::kFunction, f32>());
        a->SetInitializer(ld->Result());
        b.Return(func);
    });

    ASSERT_TRUE(generator_.Generate()) << generator_.Diagnostics().str();
    EXPECT_EQ(generator_.Result(), MetalHeader() + R"(
threadgroup float v;
void foo() {
  float a = v;
}
)");
}

}  // namespace
}  // namespace tint::msl::writer
