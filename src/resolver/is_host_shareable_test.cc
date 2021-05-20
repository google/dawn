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

namespace tint {
namespace resolver {
namespace {

using ResolverIsHostShareable = ResolverTest;

TEST_F(ResolverIsHostShareable, Void) {
  EXPECT_FALSE(r()->IsHostShareable(create<sem::Void>()));
}

TEST_F(ResolverIsHostShareable, Bool) {
  EXPECT_FALSE(r()->IsHostShareable(create<sem::Bool>()));
}

TEST_F(ResolverIsHostShareable, NumericScalar) {
  EXPECT_TRUE(r()->IsHostShareable(create<sem::I32>()));
  EXPECT_TRUE(r()->IsHostShareable(create<sem::U32>()));
  EXPECT_TRUE(r()->IsHostShareable(create<sem::F32>()));
}

TEST_F(ResolverIsHostShareable, NumericVector) {
  EXPECT_TRUE(r()->IsHostShareable(create<sem::Vector>(create<sem::I32>(), 2)));
  EXPECT_TRUE(r()->IsHostShareable(create<sem::Vector>(create<sem::I32>(), 3)));
  EXPECT_TRUE(r()->IsHostShareable(create<sem::Vector>(create<sem::I32>(), 4)));
  EXPECT_TRUE(r()->IsHostShareable(create<sem::Vector>(create<sem::U32>(), 2)));
  EXPECT_TRUE(r()->IsHostShareable(create<sem::Vector>(create<sem::U32>(), 3)));
  EXPECT_TRUE(r()->IsHostShareable(create<sem::Vector>(create<sem::U32>(), 4)));
  EXPECT_TRUE(r()->IsHostShareable(create<sem::Vector>(create<sem::F32>(), 2)));
  EXPECT_TRUE(r()->IsHostShareable(create<sem::Vector>(create<sem::F32>(), 3)));
  EXPECT_TRUE(r()->IsHostShareable(create<sem::Vector>(create<sem::F32>(), 4)));
}

TEST_F(ResolverIsHostShareable, BoolVector) {
  EXPECT_FALSE(
      r()->IsHostShareable(create<sem::Vector>(create<sem::Bool>(), 2)));
  EXPECT_FALSE(
      r()->IsHostShareable(create<sem::Vector>(create<sem::Bool>(), 3)));
  EXPECT_FALSE(
      r()->IsHostShareable(create<sem::Vector>(create<sem::Bool>(), 4)));
  EXPECT_FALSE(
      r()->IsHostShareable(create<sem::Vector>(create<sem::Bool>(), 2)));
  EXPECT_FALSE(
      r()->IsHostShareable(create<sem::Vector>(create<sem::Bool>(), 3)));
  EXPECT_FALSE(
      r()->IsHostShareable(create<sem::Vector>(create<sem::Bool>(), 4)));
  EXPECT_FALSE(
      r()->IsHostShareable(create<sem::Vector>(create<sem::Bool>(), 2)));
  EXPECT_FALSE(
      r()->IsHostShareable(create<sem::Vector>(create<sem::Bool>(), 3)));
  EXPECT_FALSE(
      r()->IsHostShareable(create<sem::Vector>(create<sem::Bool>(), 4)));
}

TEST_F(ResolverIsHostShareable, Matrix) {
  auto* vec2 = create<sem::Vector>(create<sem::F32>(), 2);
  auto* vec3 = create<sem::Vector>(create<sem::F32>(), 3);
  auto* vec4 = create<sem::Vector>(create<sem::F32>(), 4);

  EXPECT_TRUE(r()->IsHostShareable(create<sem::Matrix>(vec2, 2)));
  EXPECT_TRUE(r()->IsHostShareable(create<sem::Matrix>(vec2, 3)));
  EXPECT_TRUE(r()->IsHostShareable(create<sem::Matrix>(vec2, 4)));
  EXPECT_TRUE(r()->IsHostShareable(create<sem::Matrix>(vec3, 2)));
  EXPECT_TRUE(r()->IsHostShareable(create<sem::Matrix>(vec3, 3)));
  EXPECT_TRUE(r()->IsHostShareable(create<sem::Matrix>(vec3, 4)));
  EXPECT_TRUE(r()->IsHostShareable(create<sem::Matrix>(vec4, 2)));
  EXPECT_TRUE(r()->IsHostShareable(create<sem::Matrix>(vec4, 3)));
  EXPECT_TRUE(r()->IsHostShareable(create<sem::Matrix>(vec4, 4)));
}

TEST_F(ResolverIsHostShareable, Pointer) {
  auto* ptr =
      create<sem::Pointer>(create<sem::I32>(), ast::StorageClass::kPrivate);
  EXPECT_FALSE(r()->IsHostShareable(ptr));
}

TEST_F(ResolverIsHostShareable, ArraySizedOfHostShareable) {
  auto* arr = create<sem::Array>(create<sem::I32>(), 5, 4, 20, 4, true);
  EXPECT_TRUE(r()->IsHostShareable(arr));
}

TEST_F(ResolverIsHostShareable, ArrayUnsizedOfHostShareable) {
  auto* arr = create<sem::Array>(create<sem::I32>(), 0, 4, 4, 4, true);
  EXPECT_TRUE(r()->IsHostShareable(arr));
}

// Note: Structure tests covered in host_shareable_validation_test.cc

}  // namespace
}  // namespace resolver
}  // namespace tint
