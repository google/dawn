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

#include "src/sem/type_manager.h"

#include "gtest/gtest.h"
#include "src/sem/i32_type.h"
#include "src/sem/u32_type.h"

namespace tint {
namespace sem {
namespace {

template <typename T>
size_t count(const T& range_loopable) {
  size_t n = 0;
  for (auto it : range_loopable) {
    (void)it;
    n++;
  }
  return n;
}

using TypeManagerTest = testing::Test;

TEST_F(TypeManagerTest, GetUnregistered) {
  Manager tm;
  auto* t = tm.Get<I32>();
  ASSERT_NE(t, nullptr);
  EXPECT_TRUE(t->Is<I32>());
}

TEST_F(TypeManagerTest, GetSameTypeReturnsSamePtr) {
  Manager tm;
  auto* t = tm.Get<I32>();
  ASSERT_NE(t, nullptr);
  EXPECT_TRUE(t->Is<I32>());

  auto* t2 = tm.Get<I32>();
  EXPECT_EQ(t, t2);
}

TEST_F(TypeManagerTest, GetDifferentTypeReturnsDifferentPtr) {
  Manager tm;
  Type* t = tm.Get<I32>();
  ASSERT_NE(t, nullptr);
  EXPECT_TRUE(t->Is<I32>());

  Type* t2 = tm.Get<U32>();
  ASSERT_NE(t2, nullptr);
  EXPECT_NE(t, t2);
  EXPECT_TRUE(t2->Is<U32>());
}

TEST_F(TypeManagerTest, WrapDoesntAffectInner) {
  Manager inner;
  Manager outer = Manager::Wrap(inner);

  inner.Get<I32>();

  EXPECT_EQ(count(inner), 1u);
  EXPECT_EQ(count(outer), 0u);

  outer.Get<U32>();

  EXPECT_EQ(count(inner), 1u);
  EXPECT_EQ(count(outer), 1u);
}

}  // namespace
}  // namespace sem
}  // namespace tint
