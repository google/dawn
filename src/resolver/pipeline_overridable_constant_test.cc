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
#include "src/resolver/resolver_test_helper.h"

using ::testing::UnorderedElementsAre;

namespace tint {
namespace resolver {
namespace {

using ResolverPipelineOverridableConstantTest = ResolverTest;

TEST_F(ResolverPipelineOverridableConstantTest, NonOverridable) {
  auto* a = GlobalConst("a", ty.f32(), Construct(ty.f32()));

  EXPECT_TRUE(r()->Resolve()) << r()->error();

  auto* sem_a = Sem().Get(a);
  ASSERT_NE(sem_a, nullptr);
  EXPECT_EQ(sem_a->Declaration(), a);
  EXPECT_FALSE(sem_a->IsPipelineConstant());
}

TEST_F(ResolverPipelineOverridableConstantTest, WithId) {
  auto* a = GlobalConst("a", ty.f32(), Construct(ty.f32()), {Override(7u)});

  EXPECT_TRUE(r()->Resolve()) << r()->error();

  auto* sem_a = Sem().Get(a);
  ASSERT_NE(sem_a, nullptr);
  EXPECT_EQ(sem_a->Declaration(), a);
  EXPECT_TRUE(sem_a->IsPipelineConstant());
  EXPECT_EQ(sem_a->ConstantId(), 7u);
}

TEST_F(ResolverPipelineOverridableConstantTest, WithoutId) {
  auto* a = GlobalConst("a", ty.f32(), Construct(ty.f32()), {Override()});

  EXPECT_TRUE(r()->Resolve()) << r()->error();

  auto* sem_a = Sem().Get(a);
  ASSERT_NE(sem_a, nullptr);
  EXPECT_EQ(sem_a->Declaration(), a);
  EXPECT_TRUE(sem_a->IsPipelineConstant());
  EXPECT_EQ(sem_a->ConstantId(), 0u);
}

TEST_F(ResolverPipelineOverridableConstantTest, WithAndWithoutIds) {
  std::vector<ast::Variable*> variables;
  variables.push_back(
      GlobalConst("a", ty.f32(), Construct(ty.f32()), {Override()}));
  variables.push_back(
      GlobalConst("b", ty.f32(), Construct(ty.f32()), {Override()}));
  variables.push_back(
      GlobalConst("c", ty.f32(), Construct(ty.f32()), {Override(2u)}));
  variables.push_back(
      GlobalConst("d", ty.f32(), Construct(ty.f32()), {Override(4u)}));
  variables.push_back(
      GlobalConst("e", ty.f32(), Construct(ty.f32()), {Override()}));
  variables.push_back(
      GlobalConst("f", ty.f32(), Construct(ty.f32()), {Override(1u)}));

  EXPECT_TRUE(r()->Resolve()) << r()->error();

  std::vector<uint16_t> constant_ids;
  for (auto* var : variables) {
    auto* sem = Sem().Get(var);
    ASSERT_NE(sem, nullptr);
    constant_ids.push_back(static_cast<uint16_t>(sem->ConstantId()));
  }
  EXPECT_THAT(constant_ids, UnorderedElementsAre(0u, 3u, 2u, 4u, 5u, 1u));
}

TEST_F(ResolverPipelineOverridableConstantTest, DuplicateIds) {
  GlobalConst("a", ty.f32(), Construct(ty.f32()),
              {Override(Source{{12, 34}}, 7u)});
  GlobalConst("b", ty.f32(), Construct(ty.f32()),
              {Override(Source{{56, 78}}, 7u)});

  EXPECT_FALSE(r()->Resolve());

  EXPECT_EQ(r()->error(), R"(56:78 error: pipeline constant IDs must be unique
12:34 note: a pipeline constant with an ID of 7 was previously declared here:)");
}

TEST_F(ResolverPipelineOverridableConstantTest, IdTooLarge) {
  GlobalConst("a", ty.f32(), Construct(ty.f32()),
              {Override(Source{{12, 34}}, 65536u)});

  EXPECT_FALSE(r()->Resolve());

  EXPECT_EQ(r()->error(),
            "12:34 error: pipeline constant IDs must be between 0 and 65535");
}

}  // namespace
}  // namespace resolver
}  // namespace tint
