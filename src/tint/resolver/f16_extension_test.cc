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

#include "src/tint/resolver/resolver.h"
#include "src/tint/resolver/resolver_test_helper.h"

#include "gmock/gmock.h"

using namespace tint::number_suffixes;  // NOLINT

namespace tint::resolver {
namespace {

using ResolverF16ExtensionTest = ResolverTest;

TEST_F(ResolverF16ExtensionTest, TypeUsedWithExtension) {
    // enable f16;
    // var<private> v : f16;
    Enable(builtin::Extension::kF16);

    GlobalVar("v", ty.f16(), builtin::AddressSpace::kPrivate);

    EXPECT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverF16ExtensionTest, TypeUsedWithoutExtension) {
    // var<private> v : f16;
    GlobalVar("v", ty.f16(Source{{12, 34}}), builtin::AddressSpace::kPrivate);

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), "12:34 error: f16 type used without 'f16' extension enabled");
}

TEST_F(ResolverF16ExtensionTest, Vec2TypeUsedWithExtension) {
    // enable f16;
    // var<private> v : vec2<f16>;
    Enable(builtin::Extension::kF16);

    GlobalVar("v", ty.vec2<f16>(), builtin::AddressSpace::kPrivate);

    EXPECT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverF16ExtensionTest, Vec2TypeUsedWithoutExtension) {
    // var<private> v : vec2<f16>;
    GlobalVar("v", ty.vec2(ty.f16(Source{{12, 34}})), builtin::AddressSpace::kPrivate);

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), "12:34 error: f16 type used without 'f16' extension enabled");
}

TEST_F(ResolverF16ExtensionTest, Vec2TypeInitUsedWithExtension) {
    // enable f16;
    // var<private> v = vec2<f16>();
    Enable(builtin::Extension::kF16);

    GlobalVar("v", vec2<f16>(), builtin::AddressSpace::kPrivate);

    EXPECT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverF16ExtensionTest, Vec2TypeInitUsedWithoutExtension) {
    // var<private> v = vec2<f16>();
    GlobalVar("v", Call(ty.vec2(ty.f16(Source{{12, 34}}))), builtin::AddressSpace::kPrivate);

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), "12:34 error: f16 type used without 'f16' extension enabled");
}

TEST_F(ResolverF16ExtensionTest, Vec2TypeConvUsedWithExtension) {
    // enable f16;
    // var<private> v = vec2<f16>(vec2<f32>());
    Enable(builtin::Extension::kF16);

    GlobalVar("v", vec2<f16>(vec2<f32>()), builtin::AddressSpace::kPrivate);

    EXPECT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverF16ExtensionTest, Vec2TypeConvUsedWithoutExtension) {
    // var<private> v = vec2<f16>(vec2<f32>());
    GlobalVar("v", vec2(ty.f16(Source{{12, 34}}), vec2<f32>()), builtin::AddressSpace::kPrivate);

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), "12:34 error: f16 type used without 'f16' extension enabled");
}

TEST_F(ResolverF16ExtensionTest, F16LiteralUsedWithExtension) {
    // enable f16;
    // var<private> v = 16h;
    Enable(builtin::Extension::kF16);

    GlobalVar("v", Expr(16_h), builtin::AddressSpace::kPrivate);

    EXPECT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverF16ExtensionTest, F16LiteralUsedWithoutExtension) {
    // var<private> v = 16h;
    GlobalVar("v", Expr(Source{{12, 34}}, 16_h), builtin::AddressSpace::kPrivate);

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), "12:34 error: f16 type used without 'f16' extension enabled");
}

using ResolverF16ExtensionBuiltinTypeAliasTest = ResolverTestWithParam<const char*>;

TEST_P(ResolverF16ExtensionBuiltinTypeAliasTest, Vec2hTypeUsedWithExtension) {
    // enable f16;
    // var<private> v : vec2h;
    Enable(builtin::Extension::kF16);

    GlobalVar("v", ty(Source{{12, 34}}, GetParam()), builtin::AddressSpace::kPrivate);

    EXPECT_TRUE(r()->Resolve()) << r()->error();
}

TEST_P(ResolverF16ExtensionBuiltinTypeAliasTest, Vec2hTypeUsedWithoutExtension) {
    // var<private> v : vec2h;
    GlobalVar("v", ty(Source{{12, 34}}, GetParam()), builtin::AddressSpace::kPrivate);

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), "12:34 error: f16 type used without 'f16' extension enabled");
}

INSTANTIATE_TEST_SUITE_P(ResolverF16ExtensionBuiltinTypeAliasTest,
                         ResolverF16ExtensionBuiltinTypeAliasTest,
                         testing::Values("mat2x2h",
                                         "mat2x3h",
                                         "mat2x4h",
                                         "mat3x2h",
                                         "mat3x3h",
                                         "mat3x4h",
                                         "mat4x2h",
                                         "mat4x3h",
                                         "mat4x4h",
                                         "vec2h",
                                         "vec3h",
                                         "vec4h"));

}  // namespace
}  // namespace tint::resolver
