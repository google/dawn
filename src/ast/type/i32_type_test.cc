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

#include "src/ast/type/i32_type.h"

#include "gtest/gtest.h"

namespace tint {
namespace ast {
namespace type {
namespace {

using I32TypeTest = testing::Test;

TEST_F(I32TypeTest, Is) {
  I32Type i;
  EXPECT_FALSE(i.IsAlias());
  EXPECT_FALSE(i.IsArray());
  EXPECT_FALSE(i.IsBool());
  EXPECT_FALSE(i.IsF32());
  EXPECT_TRUE(i.IsI32());
  EXPECT_FALSE(i.IsMatrix());
  EXPECT_FALSE(i.IsPointer());
  EXPECT_FALSE(i.IsSampler());
  EXPECT_FALSE(i.IsStruct());
  EXPECT_FALSE(i.IsTexture());
  EXPECT_FALSE(i.IsU32());
  EXPECT_FALSE(i.IsVector());
}

TEST_F(I32TypeTest, TypeName) {
  I32Type i;
  EXPECT_EQ(i.type_name(), "__i32");
}

}  // namespace
}  // namespace type
}  // namespace ast
}  // namespace tint
