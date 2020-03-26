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

#include "src/ast/type/struct_type.h"

#include <utility>

#include "gtest/gtest.h"
#include "src/ast/type/i32_type.h"

namespace tint {
namespace ast {
namespace type {
namespace {

using StructTypeTest = testing::Test;

TEST_F(StructTypeTest, Creation) {
  auto impl = std::make_unique<Struct>();
  auto ptr = impl.get();
  StructType s{std::move(impl)};
  EXPECT_EQ(s.impl(), ptr);
}

TEST_F(StructTypeTest, Is) {
  auto impl = std::make_unique<Struct>();
  StructType s{std::move(impl)};
  EXPECT_FALSE(s.IsAlias());
  EXPECT_FALSE(s.IsArray());
  EXPECT_FALSE(s.IsBool());
  EXPECT_FALSE(s.IsF32());
  EXPECT_FALSE(s.IsI32());
  EXPECT_FALSE(s.IsMatrix());
  EXPECT_FALSE(s.IsPointer());
  EXPECT_TRUE(s.IsStruct());
  EXPECT_FALSE(s.IsU32());
  EXPECT_FALSE(s.IsVector());
}

TEST_F(StructTypeTest, TypeName) {
  auto impl = std::make_unique<Struct>();
  StructType s{std::move(impl)};
  s.set_name("my_struct");
  EXPECT_EQ(s.type_name(), "__struct_my_struct");
}

}  // namespace
}  // namespace type
}  // namespace ast
}  // namespace tint
