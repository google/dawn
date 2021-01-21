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

#include "src/type/sampler_type.h"

#include "src/type/access_control_type.h"
#include "src/type/array_type.h"
#include "src/type/bool_type.h"
#include "src/type/f32_type.h"
#include "src/type/i32_type.h"
#include "src/type/matrix_type.h"
#include "src/type/pointer_type.h"
#include "src/type/struct_type.h"
#include "src/type/test_helper.h"
#include "src/type/texture_type.h"
#include "src/type/u32_type.h"
#include "src/type/vector_type.h"

namespace tint {
namespace type {
namespace {

using SamplerTest = TestHelper;

TEST_F(SamplerTest, Creation) {
  Sampler s{SamplerKind::kSampler};
  EXPECT_EQ(s.kind(), SamplerKind::kSampler);
}

TEST_F(SamplerTest, Creation_ComparisonSampler) {
  Sampler s{SamplerKind::kComparisonSampler};
  EXPECT_EQ(s.kind(), SamplerKind::kComparisonSampler);
  EXPECT_TRUE(s.IsComparison());
}

TEST_F(SamplerTest, Is) {
  Sampler s{SamplerKind::kSampler};
  Type* ty = &s;
  EXPECT_FALSE(ty->Is<AccessControl>());
  EXPECT_FALSE(ty->Is<Alias>());
  EXPECT_FALSE(ty->Is<Array>());
  EXPECT_FALSE(ty->Is<Bool>());
  EXPECT_FALSE(ty->Is<F32>());
  EXPECT_FALSE(ty->Is<I32>());
  EXPECT_FALSE(ty->Is<Matrix>());
  EXPECT_FALSE(ty->Is<Pointer>());
  EXPECT_TRUE(ty->Is<Sampler>());
  EXPECT_FALSE(ty->Is<Struct>());
  EXPECT_FALSE(ty->Is<Texture>());
  EXPECT_FALSE(ty->Is<U32>());
  EXPECT_FALSE(ty->Is<Vector>());
}

TEST_F(SamplerTest, TypeName_Sampler) {
  Sampler s{SamplerKind::kSampler};
  EXPECT_EQ(s.type_name(), "__sampler_sampler");
}

TEST_F(SamplerTest, TypeName_Comparison) {
  Sampler s{SamplerKind::kComparisonSampler};
  EXPECT_EQ(s.type_name(), "__sampler_comparison");
}

TEST_F(SamplerTest, MinBufferBindingSize) {
  Sampler s{SamplerKind::kSampler};
  EXPECT_EQ(0u, s.MinBufferBindingSize(MemoryLayout::kUniformBuffer));
}

}  // namespace
}  // namespace type
}  // namespace tint
