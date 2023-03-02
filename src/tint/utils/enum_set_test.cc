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

#include "src/tint/utils/enum_set.h"

#include <vector>

#include "gmock/gmock.h"
#include "src/tint/utils/string_stream.h"

namespace tint::utils {
namespace {

using ::testing::ElementsAre;

enum class E { A = 0, B = 3, C = 7 };

utils::StringStream& operator<<(utils::StringStream& out, E e) {
    switch (e) {
        case E::A:
            return out << "A";
        case E::B:
            return out << "B";
        case E::C:
            return out << "C";
    }
    return out << "E(" << static_cast<uint32_t>(e) << ")";
}

TEST(EnumSetTest, ConstructEmpty) {
    EnumSet<E> set;
    EXPECT_FALSE(set.Contains(E::A));
    EXPECT_FALSE(set.Contains(E::B));
    EXPECT_FALSE(set.Contains(E::C));
    EXPECT_TRUE(set.Empty());
}

TEST(EnumSetTest, ConstructWithSingle) {
    EnumSet<E> set(E::B);
    EXPECT_FALSE(set.Contains(E::A));
    EXPECT_TRUE(set.Contains(E::B));
    EXPECT_FALSE(set.Contains(E::C));
    EXPECT_FALSE(set.Empty());
}

TEST(EnumSetTest, ConstructWithMultiple) {
    EnumSet<E> set(E::A, E::C);
    EXPECT_TRUE(set.Contains(E::A));
    EXPECT_FALSE(set.Contains(E::B));
    EXPECT_TRUE(set.Contains(E::C));
    EXPECT_FALSE(set.Empty());
}

TEST(EnumSetTest, AssignSet) {
    EnumSet<E> set;
    set = EnumSet<E>(E::A, E::C);
    EXPECT_TRUE(set.Contains(E::A));
    EXPECT_FALSE(set.Contains(E::B));
    EXPECT_TRUE(set.Contains(E::C));
}

TEST(EnumSetTest, AssignEnum) {
    EnumSet<E> set(E::A);
    set = E::B;
    EXPECT_FALSE(set.Contains(E::A));
    EXPECT_TRUE(set.Contains(E::B));
    EXPECT_FALSE(set.Contains(E::C));
}

TEST(EnumSetTest, AddEnum) {
    EnumSet<E> set;
    set.Add(E::B);
    EXPECT_FALSE(set.Contains(E::A));
    EXPECT_TRUE(set.Contains(E::B));
    EXPECT_FALSE(set.Contains(E::C));
}

TEST(EnumSetTest, RemoveEnum) {
    EnumSet<E> set(E::A, E::B);
    set.Remove(E::B);
    EXPECT_TRUE(set.Contains(E::A));
    EXPECT_FALSE(set.Contains(E::B));
    EXPECT_FALSE(set.Contains(E::C));
}

TEST(EnumSetTest, AddEnums) {
    EnumSet<E> set;
    set.Add(E::B, E::C);
    EXPECT_FALSE(set.Contains(E::A));
    EXPECT_TRUE(set.Contains(E::B));
    EXPECT_TRUE(set.Contains(E::C));
}

TEST(EnumSetTest, RemoveEnums) {
    EnumSet<E> set(E::A, E::B);
    set.Remove(E::C, E::B);
    EXPECT_TRUE(set.Contains(E::A));
    EXPECT_FALSE(set.Contains(E::B));
    EXPECT_FALSE(set.Contains(E::C));
}

TEST(EnumSetTest, AddEnumSet) {
    EnumSet<E> set;
    set.Add(EnumSet<E>{E::B, E::C});
    EXPECT_FALSE(set.Contains(E::A));
    EXPECT_TRUE(set.Contains(E::B));
    EXPECT_TRUE(set.Contains(E::C));
}

TEST(EnumSetTest, RemoveEnumSet) {
    EnumSet<E> set(E::A, E::B);
    set.Remove(EnumSet<E>{E::B, E::C});
    EXPECT_TRUE(set.Contains(E::A));
    EXPECT_FALSE(set.Contains(E::B));
    EXPECT_FALSE(set.Contains(E::C));
}

TEST(EnumSetTest, OperatorPlusEnum) {
    EnumSet<E> set = EnumSet<E>{E::B} + E::C;
    EXPECT_FALSE(set.Contains(E::A));
    EXPECT_TRUE(set.Contains(E::B));
    EXPECT_TRUE(set.Contains(E::C));
}

TEST(EnumSetTest, OperatorMinusEnum) {
    EnumSet<E> set = EnumSet<E>{E::A, E::B} - E::B;
    EXPECT_TRUE(set.Contains(E::A));
    EXPECT_FALSE(set.Contains(E::B));
    EXPECT_FALSE(set.Contains(E::C));
}

TEST(EnumSetTest, OperatorPlusSet) {
    EnumSet<E> set = EnumSet<E>{E::B} + EnumSet<E>{E::B, E::C};
    EXPECT_FALSE(set.Contains(E::A));
    EXPECT_TRUE(set.Contains(E::B));
    EXPECT_TRUE(set.Contains(E::C));
}

TEST(EnumSetTest, OperatorMinusSet) {
    EnumSet<E> set = EnumSet<E>{E::A, E::B} - EnumSet<E>{E::B, E::C};
    EXPECT_TRUE(set.Contains(E::A));
    EXPECT_FALSE(set.Contains(E::B));
    EXPECT_FALSE(set.Contains(E::C));
}

TEST(EnumSetTest, OperatorAnd) {
    EnumSet<E> set = EnumSet<E>{E::A, E::B} & EnumSet<E>{E::B, E::C};
    EXPECT_FALSE(set.Contains(E::A));
    EXPECT_TRUE(set.Contains(E::B));
    EXPECT_FALSE(set.Contains(E::C));
}

TEST(EnumSetTest, EqualitySet) {
    EXPECT_TRUE(EnumSet<E>(E::A, E::B) == EnumSet<E>(E::A, E::B));
    EXPECT_FALSE(EnumSet<E>(E::A, E::B) == EnumSet<E>(E::A, E::C));
}

TEST(EnumSetTest, InequalitySet) {
    EXPECT_FALSE(EnumSet<E>(E::A, E::B) != EnumSet<E>(E::A, E::B));
    EXPECT_TRUE(EnumSet<E>(E::A, E::B) != EnumSet<E>(E::A, E::C));
}

TEST(EnumSetTest, EqualityEnum) {
    EXPECT_TRUE(EnumSet<E>(E::A) == E::A);
    EXPECT_FALSE(EnumSet<E>(E::B) == E::A);
    EXPECT_FALSE(EnumSet<E>(E::B) == E::C);
    EXPECT_FALSE(EnumSet<E>(E::A, E::B) == E::A);
    EXPECT_FALSE(EnumSet<E>(E::A, E::B) == E::B);
    EXPECT_FALSE(EnumSet<E>(E::A, E::B) == E::C);
}

TEST(EnumSetTest, InequalityEnum) {
    EXPECT_FALSE(EnumSet<E>(E::A) != E::A);
    EXPECT_TRUE(EnumSet<E>(E::B) != E::A);
    EXPECT_TRUE(EnumSet<E>(E::B) != E::C);
    EXPECT_TRUE(EnumSet<E>(E::A, E::B) != E::A);
    EXPECT_TRUE(EnumSet<E>(E::A, E::B) != E::B);
    EXPECT_TRUE(EnumSet<E>(E::A, E::B) != E::C);
}

TEST(EnumSetTest, Hash) {
    auto hash = [&](EnumSet<E> s) { return std::hash<EnumSet<E>>()(s); };
    EXPECT_EQ(hash(EnumSet<E>(E::A, E::B)), hash(EnumSet<E>(E::A, E::B)));
}

TEST(EnumSetTest, Value) {
    EXPECT_EQ(EnumSet<E>().Value(), 0u);
    EXPECT_EQ(EnumSet<E>(E::A).Value(), 1u);
    EXPECT_EQ(EnumSet<E>(E::B).Value(), 8u);
    EXPECT_EQ(EnumSet<E>(E::C).Value(), 128u);
    EXPECT_EQ(EnumSet<E>(E::A, E::C).Value(), 129u);
}

TEST(EnumSetTest, Iterator) {
    auto set = EnumSet<E>(E::C, E::A);

    auto it = set.begin();
    EXPECT_EQ(*it, E::A);
    EXPECT_NE(it, set.end());
    ++it;
    EXPECT_EQ(*it, E::C);
    EXPECT_NE(it, set.end());
    ++it;
    EXPECT_EQ(it, set.end());
}

TEST(EnumSetTest, IteratorEmpty) {
    auto set = EnumSet<E>();
    EXPECT_EQ(set.begin(), set.end());
}

TEST(EnumSetTest, Loop) {
    auto set = EnumSet<E>(E::C, E::A);

    std::vector<E> seen;
    for (auto e : set) {
        seen.emplace_back(e);
    }

    EXPECT_THAT(seen, ElementsAre(E::A, E::C));
}

TEST(EnumSetTest, Ostream) {
    utils::StringStream ss;
    ss << EnumSet<E>(E::A, E::C);
    EXPECT_EQ(ss.str(), "{A, C}");
}

}  // namespace
}  // namespace tint::utils
