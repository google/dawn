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

#include "src/type_manager.h"

#include "gtest/gtest.h"
#include "src/ast/type/i32_type.h"
#include "src/ast/type/u32_type.h"

namespace tint {
namespace {

using TypeManagerTest = testing::Test;

TEST_F(TypeManagerTest, GetUnregistered) {
  TypeManager tm;
  auto* t = tm.Get(std::make_unique<ast::type::I32Type>());
  ASSERT_NE(t, nullptr);
  EXPECT_TRUE(t->IsI32());
}

TEST_F(TypeManagerTest, GetSameTypeReturnsSamePtr) {
  TypeManager tm;
  auto* t = tm.Get(std::make_unique<ast::type::I32Type>());
  ASSERT_NE(t, nullptr);
  EXPECT_TRUE(t->IsI32());

  auto* t2 = tm.Get(std::make_unique<ast::type::I32Type>());
  EXPECT_EQ(t, t2);
}

TEST_F(TypeManagerTest, GetDifferentTypeReturnsDifferentPtr) {
  TypeManager tm;
  auto* t = tm.Get(std::make_unique<ast::type::I32Type>());
  ASSERT_NE(t, nullptr);
  EXPECT_TRUE(t->IsI32());

  auto* t2 = tm.Get(std::make_unique<ast::type::U32Type>());
  ASSERT_NE(t2, nullptr);
  EXPECT_NE(t, t2);
  EXPECT_TRUE(t2->IsU32());
}

TEST_F(TypeManagerTest, ResetClearsPreviousData) {
  TypeManager tm;
  auto* t = tm.Get(std::make_unique<ast::type::I32Type>());
  ASSERT_NE(t, nullptr);

  EXPECT_FALSE(tm.types().empty());
  tm.Reset();
  EXPECT_TRUE(tm.types().empty());

  auto* t2 = tm.Get(std::make_unique<ast::type::I32Type>());
  ASSERT_NE(t2, nullptr);
}

}  // namespace
}  // namespace tint
