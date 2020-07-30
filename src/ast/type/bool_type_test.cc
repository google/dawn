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

#include "src/ast/type/bool_type.h"

#include "gtest/gtest.h"

namespace tint {
namespace ast {
namespace type {
namespace {

using BoolTypeTest = testing::Test;

TEST_F(BoolTypeTest, Is) {
  BoolType b;
  EXPECT_FALSE(b.IsAlias());
  EXPECT_FALSE(b.IsArray());
  EXPECT_TRUE(b.IsBool());
  EXPECT_FALSE(b.IsF32());
  EXPECT_FALSE(b.IsI32());
  EXPECT_FALSE(b.IsMatrix());
  EXPECT_FALSE(b.IsPointer());
  EXPECT_FALSE(b.IsSampler());
  EXPECT_FALSE(b.IsStruct());
  EXPECT_FALSE(b.IsTexture());
  EXPECT_FALSE(b.IsU32());
  EXPECT_FALSE(b.IsVector());
}

TEST_F(BoolTypeTest, TypeName) {
  BoolType b;
  EXPECT_EQ(b.type_name(), "__bool");
}

}  // namespace
}  // namespace type
}  // namespace ast
}  // namespace tint
