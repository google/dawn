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

#include "src/ast/type/sampler_type.h"

#include "gtest/gtest.h"

namespace tint {
namespace ast {
namespace type {
namespace {

using SamplerTypeTest = testing::Test;

TEST_F(SamplerTypeTest, Creation) {
  SamplerType s{SamplerKind::kSampler};
  EXPECT_EQ(s.kind(), SamplerKind::kSampler);
}

TEST_F(SamplerTypeTest, Creation_ComparisonSampler) {
  SamplerType s{SamplerKind::kComparisonSampler};
  EXPECT_EQ(s.kind(), SamplerKind::kComparisonSampler);
  EXPECT_TRUE(s.IsComparison());
}

TEST_F(SamplerTypeTest, Is) {
  SamplerType s{SamplerKind::kSampler};
  EXPECT_FALSE(s.IsAlias());
  EXPECT_FALSE(s.IsArray());
  EXPECT_FALSE(s.IsBool());
  EXPECT_FALSE(s.IsF32());
  EXPECT_FALSE(s.IsI32());
  EXPECT_FALSE(s.IsMatrix());
  EXPECT_FALSE(s.IsPointer());
  EXPECT_TRUE(s.IsSampler());
  EXPECT_FALSE(s.IsStruct());
  EXPECT_FALSE(s.IsTexture());
  EXPECT_FALSE(s.IsU32());
  EXPECT_FALSE(s.IsVector());
}

TEST_F(SamplerTypeTest, TypeName_Sampler) {
  SamplerType s{SamplerKind::kSampler};
  EXPECT_EQ(s.type_name(), "__sampler_sampler");
}

TEST_F(SamplerTypeTest, TypeName_Comparison) {
  SamplerType s{SamplerKind::kComparisonSampler};
  EXPECT_EQ(s.type_name(), "__sampler_comparison");
}

}  // namespace
}  // namespace type
}  // namespace ast
}  // namespace tint
