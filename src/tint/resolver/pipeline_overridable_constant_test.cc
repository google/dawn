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

#include "src/tint/resolver/resolver_test_helper.h"

using namespace tint::number_suffixes;  // NOLINT

namespace tint::resolver {
namespace {

class ResolverPipelineOverridableConstantTest : public ResolverTest {
  protected:
    /// Verify that the AST node `var` was resolved to an overridable constant
    /// with an ID equal to `id`.
    /// @param var the overridable constant AST node
    /// @param id the expected constant ID
    void ExpectConstantId(const ast::Variable* var, uint16_t id) {
        auto* sem = Sem().Get<sem::GlobalVariable>(var);
        ASSERT_NE(sem, nullptr);
        EXPECT_EQ(sem->Declaration(), var);
        EXPECT_TRUE(sem->IsOverridable());
        EXPECT_EQ(sem->ConstantId(), id);
        EXPECT_FALSE(sem->ConstantValue());
    }
};

TEST_F(ResolverPipelineOverridableConstantTest, NonOverridable) {
    auto* a = GlobalConst("a", ty.f32(), Expr(1_f));

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    auto* sem_a = Sem().Get<sem::GlobalVariable>(a);
    ASSERT_NE(sem_a, nullptr);
    EXPECT_EQ(sem_a->Declaration(), a);
    EXPECT_FALSE(sem_a->IsOverridable());
    EXPECT_TRUE(sem_a->ConstantValue());
}

TEST_F(ResolverPipelineOverridableConstantTest, WithId) {
    auto* a = Override("a", ty.f32(), Expr(1_f), {Id(7u)});

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    ExpectConstantId(a, 7u);
}

TEST_F(ResolverPipelineOverridableConstantTest, WithoutId) {
    auto* a = Override("a", ty.f32(), Expr(1_f));

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    ExpectConstantId(a, 0u);
}

TEST_F(ResolverPipelineOverridableConstantTest, WithAndWithoutIds) {
    std::vector<ast::Variable*> variables;
    auto* a = Override("a", ty.f32(), Expr(1_f));
    auto* b = Override("b", ty.f32(), Expr(1_f));
    auto* c = Override("c", ty.f32(), Expr(1_f), {Id(2u)});
    auto* d = Override("d", ty.f32(), Expr(1_f), {Id(4u)});
    auto* e = Override("e", ty.f32(), Expr(1_f));
    auto* f = Override("f", ty.f32(), Expr(1_f), {Id(1u)});

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    // Verify that constant id allocation order is deterministic.
    ExpectConstantId(a, 0u);
    ExpectConstantId(b, 3u);
    ExpectConstantId(c, 2u);
    ExpectConstantId(d, 4u);
    ExpectConstantId(e, 5u);
    ExpectConstantId(f, 1u);
}

TEST_F(ResolverPipelineOverridableConstantTest, DuplicateIds) {
    Override("a", ty.f32(), Expr(1_f), {Id(Source{{12, 34}}, 7u)});
    Override("b", ty.f32(), Expr(1_f), {Id(Source{{56, 78}}, 7u)});

    EXPECT_FALSE(r()->Resolve());

    EXPECT_EQ(r()->error(), R"(56:78 error: pipeline constant IDs must be unique
12:34 note: a pipeline constant with an ID of 7 was previously declared here:)");
}

TEST_F(ResolverPipelineOverridableConstantTest, IdTooLarge) {
    Override("a", ty.f32(), Expr(1_f), {Id(Source{{12, 34}}, 65536u)});

    EXPECT_FALSE(r()->Resolve());

    EXPECT_EQ(r()->error(), "12:34 error: pipeline constant IDs must be between 0 and 65535");
}

}  // namespace
}  // namespace tint::resolver
