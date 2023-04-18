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

#include "src/tint/type/sampler.h"
#include "src/tint/type/test_helper.h"
#include "src/tint/type/texture.h"

namespace tint::type {
namespace {

using SamplerTest = TestHelper;

TEST_F(SamplerTest, Creation) {
    auto* a = create<Sampler>(SamplerKind::kSampler);
    auto* b = create<Sampler>(SamplerKind::kSampler);
    auto* c = create<Sampler>(SamplerKind::kComparisonSampler);

    EXPECT_EQ(a->kind(), SamplerKind::kSampler);
    EXPECT_EQ(c->kind(), SamplerKind::kComparisonSampler);

    EXPECT_FALSE(a->IsComparison());
    EXPECT_TRUE(c->IsComparison());

    EXPECT_EQ(a, b);
    EXPECT_NE(a, c);
}

TEST_F(SamplerTest, Hash) {
    auto* a = create<Sampler>(SamplerKind::kSampler);
    auto* b = create<Sampler>(SamplerKind::kSampler);

    EXPECT_EQ(a->unique_hash, b->unique_hash);
}

TEST_F(SamplerTest, Equals) {
    auto* a = create<Sampler>(SamplerKind::kSampler);
    auto* b = create<Sampler>(SamplerKind::kSampler);
    auto* c = create<Sampler>(SamplerKind::kComparisonSampler);

    EXPECT_TRUE(a->Equals(*b));
    EXPECT_FALSE(a->Equals(*c));
    EXPECT_FALSE(a->Equals(Void{}));
}

TEST_F(SamplerTest, FriendlyNameSampler) {
    Sampler s{SamplerKind::kSampler};
    EXPECT_EQ(s.FriendlyName(), "sampler");
}

TEST_F(SamplerTest, FriendlyNameComparisonSampler) {
    Sampler s{SamplerKind::kComparisonSampler};
    EXPECT_EQ(s.FriendlyName(), "sampler_comparison");
}

TEST_F(SamplerTest, Clone) {
    auto* a = create<Sampler>(SamplerKind::kSampler);

    type::Manager mgr;
    type::CloneContext ctx{{nullptr}, {nullptr, &mgr}};

    auto* mt = a->Clone(ctx);
    EXPECT_EQ(mt->kind(), SamplerKind::kSampler);
}

}  // namespace
}  // namespace tint::type
