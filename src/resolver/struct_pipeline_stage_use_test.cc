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

#include "src/resolver/resolver.h"

#include "gmock/gmock.h"
#include "src/ast/stage_decoration.h"
#include "src/resolver/resolver_test_helper.h"
#include "src/semantic/struct.h"

using ::testing::UnorderedElementsAre;

namespace tint {
namespace resolver {
namespace {

using ResolverPipelineStageUseTest = ResolverTest;

TEST_F(ResolverPipelineStageUseTest, UnusedStruct) {
  auto* s = Structure(
      "S", {Member("a", ty.f32(), {create<ast::LocationDecoration>(0)})});

  ASSERT_TRUE(r()->Resolve()) << r()->error();

  auto* sem = Sem().Get(s);
  ASSERT_NE(sem, nullptr);
  EXPECT_TRUE(sem->PipelineStageUses().empty());
}

TEST_F(ResolverPipelineStageUseTest, StructUsedAsNonEntryPointParam) {
  auto* s = Structure(
      "S", {Member("a", ty.f32(), {create<ast::LocationDecoration>(0)})});

  Func("foo", {Param("param", s)}, ty.void_(), {}, {});

  ASSERT_TRUE(r()->Resolve()) << r()->error();

  auto* sem = Sem().Get(s);
  ASSERT_NE(sem, nullptr);
  EXPECT_TRUE(sem->PipelineStageUses().empty());
}

TEST_F(ResolverPipelineStageUseTest, StructUsedAsNonEntryPointReturnType) {
  auto* s = Structure(
      "S", {Member("a", ty.f32(), {create<ast::LocationDecoration>(0)})});

  Func("foo", {}, s, {Return(Construct(s, Expr(0.f)))}, {});

  ASSERT_TRUE(r()->Resolve()) << r()->error();

  auto* sem = Sem().Get(s);
  ASSERT_NE(sem, nullptr);
  EXPECT_TRUE(sem->PipelineStageUses().empty());
}

TEST_F(ResolverPipelineStageUseTest, StructUsedAsVertexShaderParam) {
  auto* s = Structure(
      "S", {Member("a", ty.f32(), {create<ast::LocationDecoration>(0)})});

  Func("main", {Param("param", s)}, ty.void_(), {},
       {create<ast::StageDecoration>(ast::PipelineStage::kVertex)});

  ASSERT_TRUE(r()->Resolve()) << r()->error();

  auto* sem = Sem().Get(s);
  ASSERT_NE(sem, nullptr);
  EXPECT_THAT(sem->PipelineStageUses(),
              UnorderedElementsAre(semantic::PipelineStageUsage::kVertexInput));
}

TEST_F(ResolverPipelineStageUseTest, StructUsedAsVertexShaderReturnType) {
  auto* s = Structure(
      "S", {Member("a", ty.f32(), {create<ast::LocationDecoration>(0)})});

  Func("main", {}, s, {Return(Construct(s, Expr(0.f)))},
       {create<ast::StageDecoration>(ast::PipelineStage::kVertex)});

  ASSERT_TRUE(r()->Resolve()) << r()->error();

  auto* sem = Sem().Get(s);
  ASSERT_NE(sem, nullptr);
  EXPECT_THAT(
      sem->PipelineStageUses(),
      UnorderedElementsAre(semantic::PipelineStageUsage::kVertexOutput));
}

TEST_F(ResolverPipelineStageUseTest, StructUsedAsFragmentShaderParam) {
  auto* s = Structure(
      "S", {Member("a", ty.f32(), {create<ast::LocationDecoration>(0)})});

  Func("main", {Param("param", s)}, ty.void_(), {},
       {create<ast::StageDecoration>(ast::PipelineStage::kFragment)});

  ASSERT_TRUE(r()->Resolve()) << r()->error();

  auto* sem = Sem().Get(s);
  ASSERT_NE(sem, nullptr);
  EXPECT_THAT(
      sem->PipelineStageUses(),
      UnorderedElementsAre(semantic::PipelineStageUsage::kFragmentInput));
}

TEST_F(ResolverPipelineStageUseTest, StructUsedAsFragmentShaderReturnType) {
  auto* s = Structure(
      "S", {Member("a", ty.f32(), {create<ast::LocationDecoration>(0)})});

  Func("main", {}, s, {Return(Construct(s, Expr(0.f)))},
       {create<ast::StageDecoration>(ast::PipelineStage::kFragment)});

  ASSERT_TRUE(r()->Resolve()) << r()->error();

  auto* sem = Sem().Get(s);
  ASSERT_NE(sem, nullptr);
  EXPECT_THAT(
      sem->PipelineStageUses(),
      UnorderedElementsAre(semantic::PipelineStageUsage::kFragmentOutput));
}

TEST_F(ResolverPipelineStageUseTest, StructUsedAsComputeShaderParam) {
  auto* s = Structure("S", {Member("a", ty.u32(),
                                   {create<ast::BuiltinDecoration>(
                                       ast::Builtin::kLocalInvocationIndex)})});

  Func("main", {Param("param", s)}, ty.void_(), {},
       {create<ast::StageDecoration>(ast::PipelineStage::kCompute)});

  ASSERT_TRUE(r()->Resolve()) << r()->error();

  auto* sem = Sem().Get(s);
  ASSERT_NE(sem, nullptr);
  EXPECT_THAT(
      sem->PipelineStageUses(),
      UnorderedElementsAre(semantic::PipelineStageUsage::kComputeInput));
}

TEST_F(ResolverPipelineStageUseTest, StructUsedMultipleStages) {
  auto* s = Structure(
      "S", {Member("a", ty.f32(), {create<ast::LocationDecoration>(0)})});

  Func("vert_main", {Param("param", s)}, s, {Return(Construct(s, Expr(0.f)))},
       {create<ast::StageDecoration>(ast::PipelineStage::kVertex)});

  Func("frag_main", {Param("param", s)}, ty.void_(), {},
       {create<ast::StageDecoration>(ast::PipelineStage::kFragment)});

  ASSERT_TRUE(r()->Resolve()) << r()->error();

  auto* sem = Sem().Get(s);
  ASSERT_NE(sem, nullptr);
  EXPECT_THAT(
      sem->PipelineStageUses(),
      UnorderedElementsAre(semantic::PipelineStageUsage::kVertexInput,
                           semantic::PipelineStageUsage::kVertexOutput,
                           semantic::PipelineStageUsage::kFragmentInput));
}

TEST_F(ResolverPipelineStageUseTest, StructUsedAsShaderParamViaAlias) {
  auto* s = Structure(
      "S", {Member("a", ty.f32(), {create<ast::LocationDecoration>(0)})});
  auto* s_alias = ty.alias("S_alias", s);

  Func("main", {Param("param", s_alias)}, ty.void_(), {},
       {create<ast::StageDecoration>(ast::PipelineStage::kFragment)});

  ASSERT_TRUE(r()->Resolve()) << r()->error();

  auto* sem = Sem().Get(s);
  ASSERT_NE(sem, nullptr);
  EXPECT_THAT(
      sem->PipelineStageUses(),
      UnorderedElementsAre(semantic::PipelineStageUsage::kFragmentInput));
}

TEST_F(ResolverPipelineStageUseTest, StructUsedAsShaderReturnTypeViaAlias) {
  auto* s = Structure(
      "S", {Member("a", ty.f32(), {create<ast::LocationDecoration>(0)})});
  auto* s_alias = ty.alias("S_alias", s);

  Func("main", {}, s_alias, {Return(Construct(s_alias, Expr(0.f)))},
       {create<ast::StageDecoration>(ast::PipelineStage::kFragment)});

  ASSERT_TRUE(r()->Resolve()) << r()->error();

  auto* sem = Sem().Get(s);
  ASSERT_NE(sem, nullptr);
  EXPECT_THAT(
      sem->PipelineStageUses(),
      UnorderedElementsAre(semantic::PipelineStageUsage::kFragmentOutput));
}

}  // namespace
}  // namespace resolver
}  // namespace tint
