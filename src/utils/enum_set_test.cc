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

#include "src/utils/enum_set.h"

#include "gtest/gtest.h"

namespace tint {
namespace utils {
namespace {

enum class E { A, B, C };

TEST(EnumSetTest, ConstructEmpty) {
  EnumSet<E> set;
  EXPECT_FALSE(set.Contains(E::A));
  EXPECT_FALSE(set.Contains(E::B));
  EXPECT_FALSE(set.Contains(E::C));
}

TEST(EnumSetTest, ConstructWithSingle) {
  EnumSet<E> set(E::B);
  EXPECT_FALSE(set.Contains(E::A));
  EXPECT_TRUE(set.Contains(E::B));
  EXPECT_FALSE(set.Contains(E::C));
}

TEST(EnumSetTest, ConstructWithMultiple) {
  EnumSet<E> set(E::A, E::C);
  EXPECT_TRUE(set.Contains(E::A));
  EXPECT_FALSE(set.Contains(E::B));
  EXPECT_TRUE(set.Contains(E::C));
}

TEST(EnumSetTest, Add) {
  EnumSet<E> set;
  set.Add(E::B);
  EXPECT_FALSE(set.Contains(E::A));
  EXPECT_TRUE(set.Contains(E::B));
  EXPECT_FALSE(set.Contains(E::C));
}

TEST(EnumSetTest, Remove) {
  EnumSet<E> set(E::A, E::B);
  set.Remove(E::B);
  EXPECT_TRUE(set.Contains(E::A));
  EXPECT_FALSE(set.Contains(E::B));
  EXPECT_FALSE(set.Contains(E::C));
}

TEST(EnumSetTest, Equality) {
  EXPECT_TRUE(EnumSet<E>(E::A, E::B) == EnumSet<E>(E::A, E::B));
  EXPECT_FALSE(EnumSet<E>(E::A, E::B) == EnumSet<E>(E::A, E::C));
}

TEST(EnumSetTest, Inequality) {
  EXPECT_FALSE(EnumSet<E>(E::A, E::B) != EnumSet<E>(E::A, E::B));
  EXPECT_TRUE(EnumSet<E>(E::A, E::B) != EnumSet<E>(E::A, E::C));
}

TEST(EnumSetTest, Hash) {
  auto hash = [&](EnumSet<E> s) { return std::hash<EnumSet<E>>()(s); };
  EXPECT_EQ(hash(EnumSet<E>(E::A, E::B)), hash(EnumSet<E>(E::A, E::B)));
  EXPECT_NE(hash(EnumSet<E>(E::A, E::B)), hash(EnumSet<E>(E::A, E::C)));
}

TEST(EnumSetTest, Value) {
  EXPECT_EQ(EnumSet<E>().Value(), 0u);
  EXPECT_EQ(EnumSet<E>(E::A).Value(), 1u);
  EXPECT_EQ(EnumSet<E>(E::B).Value(), 2u);
  EXPECT_EQ(EnumSet<E>(E::C).Value(), 4u);
  EXPECT_EQ(EnumSet<E>(E::A, E::C).Value(), 5u);
}

}  // namespace
}  // namespace utils
}  // namespace tint
