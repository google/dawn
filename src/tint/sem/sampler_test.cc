// Copyright 2020 The Tint Authors.
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

#include "src/tint/sem/sampler.h"
#include "src/tint/sem/test_helper.h"
#include "src/tint/sem/texture.h"

namespace tint::sem {
namespace {

using SamplerTest = TestHelper;

TEST_F(SamplerTest, Creation) {
    auto* a = create<Sampler>(ast::SamplerKind::kSampler);
    auto* b = create<Sampler>(ast::SamplerKind::kSampler);
    auto* c = create<Sampler>(ast::SamplerKind::kComparisonSampler);

    EXPECT_EQ(a->kind(), ast::SamplerKind::kSampler);
    EXPECT_EQ(c->kind(), ast::SamplerKind::kComparisonSampler);

    EXPECT_FALSE(a->IsComparison());
    EXPECT_TRUE(c->IsComparison());

    EXPECT_EQ(a, b);
    EXPECT_NE(a, c);
}

TEST_F(SamplerTest, Hash) {
    auto* a = create<Sampler>(ast::SamplerKind::kSampler);
    auto* b = create<Sampler>(ast::SamplerKind::kSampler);
    auto* c = create<Sampler>(ast::SamplerKind::kComparisonSampler);

    EXPECT_EQ(a->Hash(), b->Hash());
    EXPECT_NE(a->Hash(), c->Hash());
}

TEST_F(SamplerTest, Equals) {
    auto* a = create<Sampler>(ast::SamplerKind::kSampler);
    auto* b = create<Sampler>(ast::SamplerKind::kSampler);
    auto* c = create<Sampler>(ast::SamplerKind::kComparisonSampler);

    EXPECT_TRUE(a->Equals(*b));
    EXPECT_FALSE(a->Equals(*c));
    EXPECT_FALSE(a->Equals(Void{}));
}

TEST_F(SamplerTest, FriendlyNameSampler) {
    Sampler s{ast::SamplerKind::kSampler};
    EXPECT_EQ(s.FriendlyName(Symbols()), "sampler");
}

TEST_F(SamplerTest, FriendlyNameComparisonSampler) {
    Sampler s{ast::SamplerKind::kComparisonSampler};
    EXPECT_EQ(s.FriendlyName(Symbols()), "sampler_comparison");
}

}  // namespace
}  // namespace tint::sem
