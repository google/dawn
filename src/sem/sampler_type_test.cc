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

#include "src/sem/test_helper.h"
#include "src/sem/texture_type.h"

namespace tint {
namespace sem {
namespace {

using SamplerTest = TestHelper;

TEST_F(SamplerTest, Creation) {
  Sampler s{ast::SamplerKind::kSampler};
  EXPECT_EQ(s.kind(), ast::SamplerKind::kSampler);
}

TEST_F(SamplerTest, Creation_ComparisonSampler) {
  Sampler s{ast::SamplerKind::kComparisonSampler};
  EXPECT_EQ(s.kind(), ast::SamplerKind::kComparisonSampler);
  EXPECT_TRUE(s.IsComparison());
}

TEST_F(SamplerTest, TypeName_Sampler) {
  Sampler s{ast::SamplerKind::kSampler};
  EXPECT_EQ(s.type_name(), "__sampler_sampler");
}

TEST_F(SamplerTest, TypeName_Comparison) {
  Sampler s{ast::SamplerKind::kComparisonSampler};
  EXPECT_EQ(s.type_name(), "__sampler_comparison");
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
}  // namespace sem
}  // namespace tint
