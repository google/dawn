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

#include "src/tint/writer/append_vector.h"
#include "src/tint/program_builder.h"
#include "src/tint/resolver/resolver.h"
#include "src/tint/sem/type_constructor.h"

#include "gtest/gtest.h"

namespace tint::writer {
namespace {

class AppendVectorTest : public ::testing::Test, public ProgramBuilder {};

// AppendVector(vec2<i32>(1, 2), 3) -> vec3<i32>(1, 2, 3)
TEST_F(AppendVectorTest, Vec2i32_i32) {
    auto* scalar_1 = Expr(1);
    auto* scalar_2 = Expr(2);
    auto* scalar_3 = Expr(3);
    auto* vec_12 = vec2<i32>(scalar_1, scalar_2);
    WrapInFunction(vec_12, scalar_3);

    resolver::Resolver resolver(this);
    ASSERT_TRUE(resolver.Resolve()) << resolver.error();

    auto* append = AppendVector(this, vec_12, scalar_3);

    auto* vec_123 = As<ast::CallExpression>(append->Declaration());
    ASSERT_NE(vec_123, nullptr);
    ASSERT_EQ(vec_123->args.size(), 3u);
    EXPECT_EQ(vec_123->args[0], scalar_1);
    EXPECT_EQ(vec_123->args[1], scalar_2);
    EXPECT_EQ(vec_123->args[2], scalar_3);

    auto* call = Sem().Get(vec_123);
    ASSERT_NE(call, nullptr);
    ASSERT_EQ(call->Arguments().size(), 3u);
    EXPECT_EQ(call->Arguments()[0], Sem().Get(scalar_1));
    EXPECT_EQ(call->Arguments()[1], Sem().Get(scalar_2));
    EXPECT_EQ(call->Arguments()[2], Sem().Get(scalar_3));

    auto* ctor = call->Target()->As<sem::TypeConstructor>();
    ASSERT_NE(ctor, nullptr);
    ASSERT_TRUE(ctor->ReturnType()->Is<sem::Vector>());
    EXPECT_EQ(ctor->ReturnType()->As<sem::Vector>()->Width(), 3u);
    EXPECT_TRUE(ctor->ReturnType()->As<sem::Vector>()->type()->Is<sem::I32>());
    EXPECT_EQ(ctor->ReturnType(), call->Type());

    ASSERT_EQ(ctor->Parameters().size(), 3u);
    EXPECT_TRUE(ctor->Parameters()[0]->Type()->Is<sem::I32>());
    EXPECT_TRUE(ctor->Parameters()[1]->Type()->Is<sem::I32>());
    EXPECT_TRUE(ctor->Parameters()[2]->Type()->Is<sem::I32>());
}

// AppendVector(vec2<i32>(1, 2), 3u) -> vec3<i32>(1, 2, i32(3u))
TEST_F(AppendVectorTest, Vec2i32_u32) {
    auto* scalar_1 = Expr(1);
    auto* scalar_2 = Expr(2);
    auto* scalar_3 = Expr(3u);
    auto* vec_12 = vec2<i32>(scalar_1, scalar_2);
    WrapInFunction(vec_12, scalar_3);

    resolver::Resolver resolver(this);
    ASSERT_TRUE(resolver.Resolve()) << resolver.error();

    auto* append = AppendVector(this, vec_12, scalar_3);

    auto* vec_123 = As<ast::CallExpression>(append->Declaration());
    ASSERT_NE(vec_123, nullptr);
    ASSERT_EQ(vec_123->args.size(), 3u);
    EXPECT_EQ(vec_123->args[0], scalar_1);
    EXPECT_EQ(vec_123->args[1], scalar_2);
    auto* u32_to_i32 = vec_123->args[2]->As<ast::CallExpression>();
    ASSERT_NE(u32_to_i32, nullptr);
    EXPECT_TRUE(u32_to_i32->target.type->Is<ast::I32>());
    ASSERT_EQ(u32_to_i32->args.size(), 1u);
    EXPECT_EQ(u32_to_i32->args[0], scalar_3);

    auto* call = Sem().Get(vec_123);
    ASSERT_NE(call, nullptr);
    ASSERT_EQ(call->Arguments().size(), 3u);
    EXPECT_EQ(call->Arguments()[0], Sem().Get(scalar_1));
    EXPECT_EQ(call->Arguments()[1], Sem().Get(scalar_2));
    EXPECT_EQ(call->Arguments()[2], Sem().Get(u32_to_i32));

    auto* ctor = call->Target()->As<sem::TypeConstructor>();
    ASSERT_NE(ctor, nullptr);
    ASSERT_TRUE(ctor->ReturnType()->Is<sem::Vector>());
    EXPECT_EQ(ctor->ReturnType()->As<sem::Vector>()->Width(), 3u);
    EXPECT_TRUE(ctor->ReturnType()->As<sem::Vector>()->type()->Is<sem::I32>());
    EXPECT_EQ(ctor->ReturnType(), call->Type());

    ASSERT_EQ(ctor->Parameters().size(), 3u);
    EXPECT_TRUE(ctor->Parameters()[0]->Type()->Is<sem::I32>());
    EXPECT_TRUE(ctor->Parameters()[1]->Type()->Is<sem::I32>());
    EXPECT_TRUE(ctor->Parameters()[2]->Type()->Is<sem::I32>());
}

// AppendVector(vec2<i32>(vec2<u32>(1u, 2u)), 3u) ->
//    vec3<i32>(vec2<i32>(vec2<u32>(1u, 2u)), i32(3u))
TEST_F(AppendVectorTest, Vec2i32FromVec2u32_u32) {
    auto* scalar_1 = Expr(1u);
    auto* scalar_2 = Expr(2u);
    auto* scalar_3 = Expr(3u);
    auto* uvec_12 = vec2<u32>(scalar_1, scalar_2);
    auto* vec_12 = vec2<i32>(uvec_12);
    WrapInFunction(vec_12, scalar_3);

    resolver::Resolver resolver(this);
    ASSERT_TRUE(resolver.Resolve()) << resolver.error();

    auto* append = AppendVector(this, vec_12, scalar_3);

    auto* vec_123 = As<ast::CallExpression>(append->Declaration());
    ASSERT_NE(vec_123, nullptr);
    ASSERT_EQ(vec_123->args.size(), 2u);
    auto* v2u32_to_v2i32 = vec_123->args[0]->As<ast::CallExpression>();
    ASSERT_NE(v2u32_to_v2i32, nullptr);
    ASSERT_TRUE(v2u32_to_v2i32->target.type->Is<ast::Vector>());
    EXPECT_EQ(v2u32_to_v2i32->target.type->As<ast::Vector>()->width, 2u);
    EXPECT_TRUE(v2u32_to_v2i32->target.type->As<ast::Vector>()->type->Is<ast::I32>());
    EXPECT_EQ(v2u32_to_v2i32->args.size(), 1u);
    EXPECT_EQ(v2u32_to_v2i32->args[0], uvec_12);

    auto* u32_to_i32 = vec_123->args[1]->As<ast::CallExpression>();
    ASSERT_NE(u32_to_i32, nullptr);
    EXPECT_TRUE(u32_to_i32->target.type->Is<ast::I32>());
    ASSERT_EQ(u32_to_i32->args.size(), 1u);
    EXPECT_EQ(u32_to_i32->args[0], scalar_3);

    auto* call = Sem().Get(vec_123);
    ASSERT_NE(call, nullptr);
    ASSERT_EQ(call->Arguments().size(), 2u);
    EXPECT_EQ(call->Arguments()[0], Sem().Get(vec_12));
    EXPECT_EQ(call->Arguments()[1], Sem().Get(u32_to_i32));

    auto* ctor = call->Target()->As<sem::TypeConstructor>();
    ASSERT_NE(ctor, nullptr);
    ASSERT_TRUE(ctor->ReturnType()->Is<sem::Vector>());
    EXPECT_EQ(ctor->ReturnType()->As<sem::Vector>()->Width(), 3u);
    EXPECT_TRUE(ctor->ReturnType()->As<sem::Vector>()->type()->Is<sem::I32>());
    EXPECT_EQ(ctor->ReturnType(), call->Type());

    ASSERT_EQ(ctor->Parameters().size(), 2u);
    EXPECT_TRUE(ctor->Parameters()[0]->Type()->Is<sem::Vector>());
    EXPECT_TRUE(ctor->Parameters()[1]->Type()->Is<sem::I32>());
}

// AppendVector(vec2<i32>(1, 2), 3.0f) -> vec3<i32>(1, 2, i32(3.0f))
TEST_F(AppendVectorTest, Vec2i32_f32) {
    auto* scalar_1 = Expr(1);
    auto* scalar_2 = Expr(2);
    auto* scalar_3 = Expr(3.0f);
    auto* vec_12 = vec2<i32>(scalar_1, scalar_2);
    WrapInFunction(vec_12, scalar_3);

    resolver::Resolver resolver(this);
    ASSERT_TRUE(resolver.Resolve()) << resolver.error();

    auto* append = AppendVector(this, vec_12, scalar_3);

    auto* vec_123 = As<ast::CallExpression>(append->Declaration());
    ASSERT_NE(vec_123, nullptr);
    ASSERT_EQ(vec_123->args.size(), 3u);
    EXPECT_EQ(vec_123->args[0], scalar_1);
    EXPECT_EQ(vec_123->args[1], scalar_2);
    auto* f32_to_i32 = vec_123->args[2]->As<ast::CallExpression>();
    ASSERT_NE(f32_to_i32, nullptr);
    EXPECT_TRUE(f32_to_i32->target.type->Is<ast::I32>());
    ASSERT_EQ(f32_to_i32->args.size(), 1u);
    EXPECT_EQ(f32_to_i32->args[0], scalar_3);

    auto* call = Sem().Get(vec_123);
    ASSERT_NE(call, nullptr);
    ASSERT_EQ(call->Arguments().size(), 3u);
    EXPECT_EQ(call->Arguments()[0], Sem().Get(scalar_1));
    EXPECT_EQ(call->Arguments()[1], Sem().Get(scalar_2));
    EXPECT_EQ(call->Arguments()[2], Sem().Get(f32_to_i32));

    auto* ctor = call->Target()->As<sem::TypeConstructor>();
    ASSERT_NE(ctor, nullptr);
    ASSERT_TRUE(ctor->ReturnType()->Is<sem::Vector>());
    EXPECT_EQ(ctor->ReturnType()->As<sem::Vector>()->Width(), 3u);
    EXPECT_TRUE(ctor->ReturnType()->As<sem::Vector>()->type()->Is<sem::I32>());
    EXPECT_EQ(ctor->ReturnType(), call->Type());

    ASSERT_EQ(ctor->Parameters().size(), 3u);
    EXPECT_TRUE(ctor->Parameters()[0]->Type()->Is<sem::I32>());
    EXPECT_TRUE(ctor->Parameters()[1]->Type()->Is<sem::I32>());
    EXPECT_TRUE(ctor->Parameters()[2]->Type()->Is<sem::I32>());
}

// AppendVector(vec3<i32>(1, 2, 3), 4) -> vec4<i32>(1, 2, 3, 4)
TEST_F(AppendVectorTest, Vec3i32_i32) {
    auto* scalar_1 = Expr(1);
    auto* scalar_2 = Expr(2);
    auto* scalar_3 = Expr(3);
    auto* scalar_4 = Expr(4);
    auto* vec_123 = vec3<i32>(scalar_1, scalar_2, scalar_3);
    WrapInFunction(vec_123, scalar_4);

    resolver::Resolver resolver(this);
    ASSERT_TRUE(resolver.Resolve()) << resolver.error();

    auto* append = AppendVector(this, vec_123, scalar_4);

    auto* vec_1234 = As<ast::CallExpression>(append->Declaration());
    ASSERT_NE(vec_1234, nullptr);
    ASSERT_EQ(vec_1234->args.size(), 4u);
    EXPECT_EQ(vec_1234->args[0], scalar_1);
    EXPECT_EQ(vec_1234->args[1], scalar_2);
    EXPECT_EQ(vec_1234->args[2], scalar_3);
    EXPECT_EQ(vec_1234->args[3], scalar_4);

    auto* call = Sem().Get(vec_1234);
    ASSERT_NE(call, nullptr);
    ASSERT_EQ(call->Arguments().size(), 4u);
    EXPECT_EQ(call->Arguments()[0], Sem().Get(scalar_1));
    EXPECT_EQ(call->Arguments()[1], Sem().Get(scalar_2));
    EXPECT_EQ(call->Arguments()[2], Sem().Get(scalar_3));
    EXPECT_EQ(call->Arguments()[3], Sem().Get(scalar_4));

    auto* ctor = call->Target()->As<sem::TypeConstructor>();
    ASSERT_NE(ctor, nullptr);
    ASSERT_TRUE(ctor->ReturnType()->Is<sem::Vector>());
    EXPECT_EQ(ctor->ReturnType()->As<sem::Vector>()->Width(), 4u);
    EXPECT_TRUE(ctor->ReturnType()->As<sem::Vector>()->type()->Is<sem::I32>());
    EXPECT_EQ(ctor->ReturnType(), call->Type());

    ASSERT_EQ(ctor->Parameters().size(), 4u);
    EXPECT_TRUE(ctor->Parameters()[0]->Type()->Is<sem::I32>());
    EXPECT_TRUE(ctor->Parameters()[1]->Type()->Is<sem::I32>());
    EXPECT_TRUE(ctor->Parameters()[2]->Type()->Is<sem::I32>());
    EXPECT_TRUE(ctor->Parameters()[3]->Type()->Is<sem::I32>());
}

// AppendVector(vec_12, 3) -> vec3<i32>(vec_12, 3)
TEST_F(AppendVectorTest, Vec2i32Var_i32) {
    Global("vec_12", ty.vec2<i32>(), ast::StorageClass::kPrivate);
    auto* vec_12 = Expr("vec_12");
    auto* scalar_3 = Expr(3);
    WrapInFunction(vec_12, scalar_3);

    resolver::Resolver resolver(this);
    ASSERT_TRUE(resolver.Resolve()) << resolver.error();

    auto* append = AppendVector(this, vec_12, scalar_3);

    auto* vec_123 = As<ast::CallExpression>(append->Declaration());
    ASSERT_NE(vec_123, nullptr);
    ASSERT_EQ(vec_123->args.size(), 2u);
    EXPECT_EQ(vec_123->args[0], vec_12);
    EXPECT_EQ(vec_123->args[1], scalar_3);

    auto* call = Sem().Get(vec_123);
    ASSERT_NE(call, nullptr);
    ASSERT_EQ(call->Arguments().size(), 2u);
    EXPECT_EQ(call->Arguments()[0], Sem().Get(vec_12));
    EXPECT_EQ(call->Arguments()[1], Sem().Get(scalar_3));

    auto* ctor = call->Target()->As<sem::TypeConstructor>();
    ASSERT_NE(ctor, nullptr);
    ASSERT_TRUE(ctor->ReturnType()->Is<sem::Vector>());
    EXPECT_EQ(ctor->ReturnType()->As<sem::Vector>()->Width(), 3u);
    EXPECT_TRUE(ctor->ReturnType()->As<sem::Vector>()->type()->Is<sem::I32>());
    EXPECT_EQ(ctor->ReturnType(), call->Type());

    ASSERT_EQ(ctor->Parameters().size(), 2u);
    EXPECT_TRUE(ctor->Parameters()[0]->Type()->Is<sem::Vector>());
    EXPECT_TRUE(ctor->Parameters()[1]->Type()->Is<sem::I32>());
}

// AppendVector(1, 2, scalar_3) -> vec3<i32>(1, 2, scalar_3)
TEST_F(AppendVectorTest, Vec2i32_i32Var) {
    Global("scalar_3", ty.i32(), ast::StorageClass::kPrivate);
    auto* scalar_1 = Expr(1);
    auto* scalar_2 = Expr(2);
    auto* scalar_3 = Expr("scalar_3");
    auto* vec_12 = vec2<i32>(scalar_1, scalar_2);
    WrapInFunction(vec_12, scalar_3);

    resolver::Resolver resolver(this);
    ASSERT_TRUE(resolver.Resolve()) << resolver.error();

    auto* append = AppendVector(this, vec_12, scalar_3);

    auto* vec_123 = As<ast::CallExpression>(append->Declaration());
    ASSERT_NE(vec_123, nullptr);
    ASSERT_EQ(vec_123->args.size(), 3u);
    EXPECT_EQ(vec_123->args[0], scalar_1);
    EXPECT_EQ(vec_123->args[1], scalar_2);
    EXPECT_EQ(vec_123->args[2], scalar_3);

    auto* call = Sem().Get(vec_123);
    ASSERT_NE(call, nullptr);
    ASSERT_EQ(call->Arguments().size(), 3u);
    EXPECT_EQ(call->Arguments()[0], Sem().Get(scalar_1));
    EXPECT_EQ(call->Arguments()[1], Sem().Get(scalar_2));
    EXPECT_EQ(call->Arguments()[2], Sem().Get(scalar_3));

    auto* ctor = call->Target()->As<sem::TypeConstructor>();
    ASSERT_NE(ctor, nullptr);
    ASSERT_TRUE(ctor->ReturnType()->Is<sem::Vector>());
    EXPECT_EQ(ctor->ReturnType()->As<sem::Vector>()->Width(), 3u);
    EXPECT_TRUE(ctor->ReturnType()->As<sem::Vector>()->type()->Is<sem::I32>());
    EXPECT_EQ(ctor->ReturnType(), call->Type());

    ASSERT_EQ(ctor->Parameters().size(), 3u);
    EXPECT_TRUE(ctor->Parameters()[0]->Type()->Is<sem::I32>());
    EXPECT_TRUE(ctor->Parameters()[1]->Type()->Is<sem::I32>());
    EXPECT_TRUE(ctor->Parameters()[2]->Type()->Is<sem::I32>());
}

// AppendVector(vec_12, scalar_3) -> vec3<i32>(vec_12, scalar_3)
TEST_F(AppendVectorTest, Vec2i32Var_i32Var) {
    Global("vec_12", ty.vec2<i32>(), ast::StorageClass::kPrivate);
    Global("scalar_3", ty.i32(), ast::StorageClass::kPrivate);
    auto* vec_12 = Expr("vec_12");
    auto* scalar_3 = Expr("scalar_3");
    WrapInFunction(vec_12, scalar_3);

    resolver::Resolver resolver(this);
    ASSERT_TRUE(resolver.Resolve()) << resolver.error();

    auto* append = AppendVector(this, vec_12, scalar_3);

    auto* vec_123 = As<ast::CallExpression>(append->Declaration());
    ASSERT_NE(vec_123, nullptr);
    ASSERT_EQ(vec_123->args.size(), 2u);
    EXPECT_EQ(vec_123->args[0], vec_12);
    EXPECT_EQ(vec_123->args[1], scalar_3);

    auto* call = Sem().Get(vec_123);
    ASSERT_NE(call, nullptr);
    ASSERT_EQ(call->Arguments().size(), 2u);
    EXPECT_EQ(call->Arguments()[0], Sem().Get(vec_12));
    EXPECT_EQ(call->Arguments()[1], Sem().Get(scalar_3));

    auto* ctor = call->Target()->As<sem::TypeConstructor>();
    ASSERT_NE(ctor, nullptr);
    ASSERT_TRUE(ctor->ReturnType()->Is<sem::Vector>());
    EXPECT_EQ(ctor->ReturnType()->As<sem::Vector>()->Width(), 3u);
    EXPECT_TRUE(ctor->ReturnType()->As<sem::Vector>()->type()->Is<sem::I32>());
    EXPECT_EQ(ctor->ReturnType(), call->Type());

    ASSERT_EQ(ctor->Parameters().size(), 2u);
    EXPECT_TRUE(ctor->Parameters()[0]->Type()->Is<sem::Vector>());
    EXPECT_TRUE(ctor->Parameters()[1]->Type()->Is<sem::I32>());
}

// AppendVector(vec_12, scalar_3) -> vec3<i32>(vec_12, i32(scalar_3))
TEST_F(AppendVectorTest, Vec2i32Var_f32Var) {
    Global("vec_12", ty.vec2<i32>(), ast::StorageClass::kPrivate);
    Global("scalar_3", ty.f32(), ast::StorageClass::kPrivate);
    auto* vec_12 = Expr("vec_12");
    auto* scalar_3 = Expr("scalar_3");
    WrapInFunction(vec_12, scalar_3);

    resolver::Resolver resolver(this);
    ASSERT_TRUE(resolver.Resolve()) << resolver.error();

    auto* append = AppendVector(this, vec_12, scalar_3);

    auto* vec_123 = As<ast::CallExpression>(append->Declaration());
    ASSERT_NE(vec_123, nullptr);
    ASSERT_EQ(vec_123->args.size(), 2u);
    EXPECT_EQ(vec_123->args[0], vec_12);
    auto* f32_to_i32 = vec_123->args[1]->As<ast::CallExpression>();
    ASSERT_NE(f32_to_i32, nullptr);
    EXPECT_TRUE(f32_to_i32->target.type->Is<ast::I32>());
    ASSERT_EQ(f32_to_i32->args.size(), 1u);
    EXPECT_EQ(f32_to_i32->args[0], scalar_3);

    auto* call = Sem().Get(vec_123);
    ASSERT_NE(call, nullptr);
    ASSERT_EQ(call->Arguments().size(), 2u);
    EXPECT_EQ(call->Arguments()[0], Sem().Get(vec_12));
    EXPECT_EQ(call->Arguments()[1], Sem().Get(f32_to_i32));

    auto* ctor = call->Target()->As<sem::TypeConstructor>();
    ASSERT_NE(ctor, nullptr);
    ASSERT_TRUE(ctor->ReturnType()->Is<sem::Vector>());
    EXPECT_EQ(ctor->ReturnType()->As<sem::Vector>()->Width(), 3u);
    EXPECT_TRUE(ctor->ReturnType()->As<sem::Vector>()->type()->Is<sem::I32>());
    EXPECT_EQ(ctor->ReturnType(), call->Type());

    ASSERT_EQ(ctor->Parameters().size(), 2u);
    EXPECT_TRUE(ctor->Parameters()[0]->Type()->Is<sem::Vector>());
    EXPECT_TRUE(ctor->Parameters()[1]->Type()->Is<sem::I32>());
}

// AppendVector(vec_12, scalar_3) -> vec3<bool>(vec_12, scalar_3)
TEST_F(AppendVectorTest, Vec2boolVar_boolVar) {
    Global("vec_12", ty.vec2<bool>(), ast::StorageClass::kPrivate);
    Global("scalar_3", ty.bool_(), ast::StorageClass::kPrivate);
    auto* vec_12 = Expr("vec_12");
    auto* scalar_3 = Expr("scalar_3");
    WrapInFunction(vec_12, scalar_3);

    resolver::Resolver resolver(this);
    ASSERT_TRUE(resolver.Resolve()) << resolver.error();

    auto* append = AppendVector(this, vec_12, scalar_3);

    auto* vec_123 = As<ast::CallExpression>(append->Declaration());
    ASSERT_NE(vec_123, nullptr);
    ASSERT_EQ(vec_123->args.size(), 2u);
    EXPECT_EQ(vec_123->args[0], vec_12);
    EXPECT_EQ(vec_123->args[1], scalar_3);

    auto* call = Sem().Get(vec_123);
    ASSERT_NE(call, nullptr);
    ASSERT_EQ(call->Arguments().size(), 2u);
    EXPECT_EQ(call->Arguments()[0], Sem().Get(vec_12));
    EXPECT_EQ(call->Arguments()[1], Sem().Get(scalar_3));

    auto* ctor = call->Target()->As<sem::TypeConstructor>();
    ASSERT_NE(ctor, nullptr);
    ASSERT_TRUE(ctor->ReturnType()->Is<sem::Vector>());
    EXPECT_EQ(ctor->ReturnType()->As<sem::Vector>()->Width(), 3u);
    EXPECT_TRUE(ctor->ReturnType()->As<sem::Vector>()->type()->Is<sem::Bool>());
    EXPECT_EQ(ctor->ReturnType(), call->Type());

    ASSERT_EQ(ctor->Parameters().size(), 2u);
    EXPECT_TRUE(ctor->Parameters()[0]->Type()->Is<sem::Vector>());
    EXPECT_TRUE(ctor->Parameters()[1]->Type()->Is<sem::Bool>());
}

// AppendVector(vec3<i32>(), 4) -> vec3<bool>(0, 0, 0, 4)
TEST_F(AppendVectorTest, ZeroVec3i32_i32) {
    auto* scalar = Expr(4);
    auto* vec000 = vec3<i32>();
    WrapInFunction(vec000, scalar);

    resolver::Resolver resolver(this);
    ASSERT_TRUE(resolver.Resolve()) << resolver.error();

    auto* append = AppendVector(this, vec000, scalar);

    auto* vec_0004 = As<ast::CallExpression>(append->Declaration());
    ASSERT_NE(vec_0004, nullptr);
    ASSERT_EQ(vec_0004->args.size(), 4u);
    for (size_t i = 0; i < 3; i++) {
        auto* literal = As<ast::SintLiteralExpression>(vec_0004->args[i]);
        ASSERT_NE(literal, nullptr);
        EXPECT_EQ(literal->value, 0);
    }
    EXPECT_EQ(vec_0004->args[3], scalar);

    auto* call = Sem().Get(vec_0004);
    ASSERT_NE(call, nullptr);
    ASSERT_EQ(call->Arguments().size(), 4u);
    EXPECT_EQ(call->Arguments()[0], Sem().Get(vec_0004->args[0]));
    EXPECT_EQ(call->Arguments()[1], Sem().Get(vec_0004->args[1]));
    EXPECT_EQ(call->Arguments()[2], Sem().Get(vec_0004->args[2]));
    EXPECT_EQ(call->Arguments()[3], Sem().Get(scalar));

    auto* ctor = call->Target()->As<sem::TypeConstructor>();
    ASSERT_NE(ctor, nullptr);
    ASSERT_TRUE(ctor->ReturnType()->Is<sem::Vector>());
    EXPECT_EQ(ctor->ReturnType()->As<sem::Vector>()->Width(), 4u);
    EXPECT_TRUE(ctor->ReturnType()->As<sem::Vector>()->type()->Is<sem::I32>());
    EXPECT_EQ(ctor->ReturnType(), call->Type());

    ASSERT_EQ(ctor->Parameters().size(), 4u);
    EXPECT_TRUE(ctor->Parameters()[0]->Type()->Is<sem::I32>());
    EXPECT_TRUE(ctor->Parameters()[1]->Type()->Is<sem::I32>());
    EXPECT_TRUE(ctor->Parameters()[2]->Type()->Is<sem::I32>());
    EXPECT_TRUE(ctor->Parameters()[3]->Type()->Is<sem::I32>());
}

}  // namespace
}  // namespace tint::writer
