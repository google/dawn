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

#include "src/tint/resolver/resolver.h"

#include "gmock/gmock.h"
#include "src/tint/ast/stage_attribute.h"
#include "src/tint/builtin/builtin_value.h"
#include "src/tint/resolver/resolver_test_helper.h"
#include "src/tint/sem/struct.h"

using ::testing::UnorderedElementsAre;

using namespace tint::number_suffixes;  // NOLINT

namespace tint::resolver {
namespace {

using ResolverPipelineStageUseTest = ResolverTest;

TEST_F(ResolverPipelineStageUseTest, UnusedStruct) {
    auto* s = Structure("S", utils::Vector{Member("a", ty.f32(), utils::Vector{Location(0_a)})});

    ASSERT_TRUE(r()->Resolve()) << r()->error();

    auto* sem = TypeOf(s)->As<type::Struct>();
    ASSERT_NE(sem, nullptr);
    EXPECT_TRUE(sem->PipelineStageUses().empty());
}

TEST_F(ResolverPipelineStageUseTest, StructUsedAsNonEntryPointParam) {
    auto* s = Structure("S", utils::Vector{Member("a", ty.f32(), utils::Vector{Location(0_a)})});

    Func("foo", utils::Vector{Param("param", ty.Of(s))}, ty.void_(), utils::Empty, utils::Empty);

    ASSERT_TRUE(r()->Resolve()) << r()->error();

    auto* sem = TypeOf(s)->As<type::Struct>();
    ASSERT_NE(sem, nullptr);
    EXPECT_TRUE(sem->PipelineStageUses().empty());
}

TEST_F(ResolverPipelineStageUseTest, StructUsedAsNonEntryPointReturnType) {
    auto* s = Structure("S", utils::Vector{Member("a", ty.f32(), utils::Vector{Location(0_a)})});

    Func("foo", utils::Empty, ty.Of(s), utils::Vector{Return(Call(ty.Of(s), Expr(0_f)))},
         utils::Empty);

    ASSERT_TRUE(r()->Resolve()) << r()->error();

    auto* sem = TypeOf(s)->As<type::Struct>();
    ASSERT_NE(sem, nullptr);
    EXPECT_TRUE(sem->PipelineStageUses().empty());
}

TEST_F(ResolverPipelineStageUseTest, StructUsedAsVertexShaderParam) {
    auto* s = Structure("S", utils::Vector{Member("a", ty.f32(), utils::Vector{Location(0_a)})});

    Func("main", utils::Vector{Param("param", ty.Of(s))}, ty.vec4<f32>(),
         utils::Vector{Return(Call(ty.vec4<f32>()))},
         utils::Vector{Stage(ast::PipelineStage::kVertex)},
         utils::Vector{Builtin(builtin::BuiltinValue::kPosition)});

    ASSERT_TRUE(r()->Resolve()) << r()->error();

    auto* sem = TypeOf(s)->As<type::Struct>();
    ASSERT_NE(sem, nullptr);
    EXPECT_THAT(sem->PipelineStageUses(),
                UnorderedElementsAre(type::PipelineStageUsage::kVertexInput));
}

TEST_F(ResolverPipelineStageUseTest, StructUsedAsVertexShaderReturnType) {
    auto* s = Structure(
        "S", utils::Vector{Member("a", ty.vec4<f32>(),
                                  utils::Vector{Builtin(builtin::BuiltinValue::kPosition)})});

    Func("main", utils::Empty, ty.Of(s), utils::Vector{Return(Call(ty.Of(s)))},
         utils::Vector{Stage(ast::PipelineStage::kVertex)});

    ASSERT_TRUE(r()->Resolve()) << r()->error();

    auto* sem = TypeOf(s)->As<type::Struct>();
    ASSERT_NE(sem, nullptr);
    EXPECT_THAT(sem->PipelineStageUses(),
                UnorderedElementsAre(type::PipelineStageUsage::kVertexOutput));
}

TEST_F(ResolverPipelineStageUseTest, StructUsedAsFragmentShaderParam) {
    auto* s = Structure("S", utils::Vector{Member("a", ty.f32(), utils::Vector{Location(0_a)})});

    Func("main", utils::Vector{Param("param", ty.Of(s))}, ty.void_(), utils::Empty,
         utils::Vector{Stage(ast::PipelineStage::kFragment)});

    ASSERT_TRUE(r()->Resolve()) << r()->error();

    auto* sem = TypeOf(s)->As<type::Struct>();
    ASSERT_NE(sem, nullptr);
    EXPECT_THAT(sem->PipelineStageUses(),
                UnorderedElementsAre(type::PipelineStageUsage::kFragmentInput));
}

TEST_F(ResolverPipelineStageUseTest, StructUsedAsFragmentShaderReturnType) {
    auto* s = Structure("S", utils::Vector{Member("a", ty.f32(), utils::Vector{Location(0_a)})});

    Func("main", utils::Empty, ty.Of(s), utils::Vector{Return(Call(ty.Of(s), Expr(0_f)))},
         utils::Vector{Stage(ast::PipelineStage::kFragment)});

    ASSERT_TRUE(r()->Resolve()) << r()->error();

    auto* sem = TypeOf(s)->As<type::Struct>();
    ASSERT_NE(sem, nullptr);
    EXPECT_THAT(sem->PipelineStageUses(),
                UnorderedElementsAre(type::PipelineStageUsage::kFragmentOutput));
}

TEST_F(ResolverPipelineStageUseTest, StructUsedAsComputeShaderParam) {
    auto* s = Structure(
        "S",
        utils::Vector{Member(
            "a", ty.u32(), utils::Vector{Builtin(builtin::BuiltinValue::kLocalInvocationIndex)})});

    Func("main", utils::Vector{Param("param", ty.Of(s))}, ty.void_(), utils::Empty,
         utils::Vector{Stage(ast::PipelineStage::kCompute), WorkgroupSize(1_i)});

    ASSERT_TRUE(r()->Resolve()) << r()->error();

    auto* sem = TypeOf(s)->As<type::Struct>();
    ASSERT_NE(sem, nullptr);
    EXPECT_THAT(sem->PipelineStageUses(),
                UnorderedElementsAre(type::PipelineStageUsage::kComputeInput));
}

TEST_F(ResolverPipelineStageUseTest, StructUsedMultipleStages) {
    auto* s = Structure(
        "S", utils::Vector{Member("a", ty.vec4<f32>(),
                                  utils::Vector{Builtin(builtin::BuiltinValue::kPosition)})});

    Func("vert_main", utils::Empty, ty.Of(s), utils::Vector{Return(Call(ty.Of(s)))},
         utils::Vector{Stage(ast::PipelineStage::kVertex)});

    Func("frag_main", utils::Vector{Param("param", ty.Of(s))}, ty.void_(), utils::Empty,
         utils::Vector{Stage(ast::PipelineStage::kFragment)});

    ASSERT_TRUE(r()->Resolve()) << r()->error();

    auto* sem = TypeOf(s)->As<type::Struct>();
    ASSERT_NE(sem, nullptr);
    EXPECT_THAT(sem->PipelineStageUses(),
                UnorderedElementsAre(type::PipelineStageUsage::kVertexOutput,
                                     type::PipelineStageUsage::kFragmentInput));
}

TEST_F(ResolverPipelineStageUseTest, StructUsedAsShaderParamViaAlias) {
    auto* s = Structure("S", utils::Vector{Member("a", ty.f32(), utils::Vector{Location(0_a)})});
    auto* s_alias = Alias("S_alias", ty.Of(s));

    Func("main", utils::Vector{Param("param", ty.Of(s_alias))}, ty.void_(), utils::Empty,
         utils::Vector{Stage(ast::PipelineStage::kFragment)});

    ASSERT_TRUE(r()->Resolve()) << r()->error();

    auto* sem = TypeOf(s)->As<type::Struct>();
    ASSERT_NE(sem, nullptr);
    EXPECT_THAT(sem->PipelineStageUses(),
                UnorderedElementsAre(type::PipelineStageUsage::kFragmentInput));
}

TEST_F(ResolverPipelineStageUseTest, StructUsedAsShaderParamLocationSet) {
    auto* s = Structure("S", utils::Vector{Member("a", ty.f32(), utils::Vector{Location(3_a)})});

    Func("main", utils::Vector{Param("param", ty.Of(s))}, ty.void_(), utils::Empty,
         utils::Vector{Stage(ast::PipelineStage::kFragment)});

    ASSERT_TRUE(r()->Resolve()) << r()->error();

    auto* sem = TypeOf(s)->As<type::Struct>();
    ASSERT_NE(sem, nullptr);
    ASSERT_EQ(1u, sem->Members().Length());
    EXPECT_EQ(3u, sem->Members()[0]->Attributes().location);
}

TEST_F(ResolverPipelineStageUseTest, StructUsedAsShaderReturnTypeViaAlias) {
    auto* s = Structure("S", utils::Vector{Member("a", ty.f32(), utils::Vector{Location(0_a)})});
    auto* s_alias = Alias("S_alias", ty.Of(s));

    Func("main", utils::Empty, ty.Of(s_alias),
         utils::Vector{Return(Call(ty.Of(s_alias), Expr(0_f)))},
         utils::Vector{Stage(ast::PipelineStage::kFragment)});

    ASSERT_TRUE(r()->Resolve()) << r()->error();

    auto* sem = TypeOf(s)->As<type::Struct>();
    ASSERT_NE(sem, nullptr);
    EXPECT_THAT(sem->PipelineStageUses(),
                UnorderedElementsAre(type::PipelineStageUsage::kFragmentOutput));
}

TEST_F(ResolverPipelineStageUseTest, StructUsedAsShaderReturnTypeLocationSet) {
    auto* s = Structure("S", utils::Vector{Member("a", ty.f32(), utils::Vector{Location(3_a)})});

    Func("main", utils::Empty, ty.Of(s), utils::Vector{Return(Call(ty.Of(s), Expr(0_f)))},
         utils::Vector{Stage(ast::PipelineStage::kFragment)});

    ASSERT_TRUE(r()->Resolve()) << r()->error();

    auto* sem = TypeOf(s)->As<type::Struct>();
    ASSERT_NE(sem, nullptr);
    ASSERT_EQ(1u, sem->Members().Length());
    EXPECT_EQ(3u, sem->Members()[0]->Attributes().location);
}

}  // namespace
}  // namespace tint::resolver
