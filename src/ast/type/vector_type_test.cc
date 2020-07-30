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

#include "src/ast/type/vector_type.h"

#include "gtest/gtest.h"
#include "src/ast/type/i32_type.h"

namespace tint {
namespace ast {
namespace type {
namespace {

using VectorTypeTest = testing::Test;

TEST_F(VectorTypeTest, Creation) {
  I32Type i32;
  VectorType v{&i32, 2};
  EXPECT_EQ(v.type(), &i32);
  EXPECT_EQ(v.size(), 2u);
}

TEST_F(VectorTypeTest, Is) {
  I32Type i32;
  VectorType v{&i32, 4};
  EXPECT_FALSE(v.IsAlias());
  EXPECT_FALSE(v.IsArray());
  EXPECT_FALSE(v.IsBool());
  EXPECT_FALSE(v.IsF32());
  EXPECT_FALSE(v.IsI32());
  EXPECT_FALSE(v.IsMatrix());
  EXPECT_FALSE(v.IsPointer());
  EXPECT_FALSE(v.IsSampler());
  EXPECT_FALSE(v.IsStruct());
  EXPECT_FALSE(v.IsTexture());
  EXPECT_FALSE(v.IsU32());
  EXPECT_TRUE(v.IsVector());
}

TEST_F(VectorTypeTest, TypeName) {
  I32Type i32;
  VectorType v{&i32, 3};
  EXPECT_EQ(v.type_name(), "__vec_3__i32");
}

}  // namespace
}  // namespace type
}  // namespace ast
}  // namespace tint
