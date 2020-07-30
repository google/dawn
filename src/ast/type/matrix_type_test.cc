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

#include "src/ast/type/matrix_type.h"

#include "gtest/gtest.h"
#include "src/ast/type/i32_type.h"

namespace tint {
namespace ast {
namespace type {
namespace {

using MatrixTypeTest = testing::Test;

TEST_F(MatrixTypeTest, Creation) {
  I32Type i32;
  MatrixType m{&i32, 2, 4};
  EXPECT_EQ(m.type(), &i32);
  EXPECT_EQ(m.rows(), 2u);
  EXPECT_EQ(m.columns(), 4u);
}

TEST_F(MatrixTypeTest, Is) {
  I32Type i32;
  MatrixType m{&i32, 2, 3};
  EXPECT_FALSE(m.IsAlias());
  EXPECT_FALSE(m.IsArray());
  EXPECT_FALSE(m.IsBool());
  EXPECT_FALSE(m.IsF32());
  EXPECT_FALSE(m.IsI32());
  EXPECT_TRUE(m.IsMatrix());
  EXPECT_FALSE(m.IsPointer());
  EXPECT_FALSE(m.IsSampler());
  EXPECT_FALSE(m.IsStruct());
  EXPECT_FALSE(m.IsTexture());
  EXPECT_FALSE(m.IsU32());
  EXPECT_FALSE(m.IsVector());
}

TEST_F(MatrixTypeTest, TypeName) {
  I32Type i32;
  MatrixType m{&i32, 2, 3};
  EXPECT_EQ(m.type_name(), "__mat_2_3__i32");
}

}  // namespace
}  // namespace type
}  // namespace ast
}  // namespace tint
