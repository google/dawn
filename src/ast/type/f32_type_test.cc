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

#include "src/ast/type/f32_type.h"

#include "gtest/gtest.h"

namespace tint {
namespace ast {
namespace type {
namespace {

using F32TypeTest = testing::Test;

TEST_F(F32TypeTest, Is) {
  F32Type f;
  EXPECT_FALSE(f.IsAlias());
  EXPECT_FALSE(f.IsArray());
  EXPECT_FALSE(f.IsBool());
  EXPECT_TRUE(f.IsF32());
  EXPECT_FALSE(f.IsI32());
  EXPECT_FALSE(f.IsMatrix());
  EXPECT_FALSE(f.IsPointer());
  EXPECT_FALSE(f.IsSampler());
  EXPECT_FALSE(f.IsStruct());
  EXPECT_FALSE(f.IsTexture());
  EXPECT_FALSE(f.IsU32());
  EXPECT_FALSE(f.IsVector());
}

TEST_F(F32TypeTest, TypeName) {
  F32Type f;
  EXPECT_EQ(f.type_name(), "__f32");
}

}  // namespace
}  // namespace type
}  // namespace ast
}  // namespace tint
