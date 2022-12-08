// Copyright 2022 The Tint Authors.
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

#include "src/tint/type/type_manager.h"

#include "gtest/gtest.h"
#include "src/tint/sem/i32.h"
#include "src/tint/sem/u32.h"

namespace tint::type {
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
    TypeManager tm;
    auto* t = tm.Get<sem::I32>();
    ASSERT_NE(t, nullptr);
    EXPECT_TRUE(t->Is<sem::I32>());
}

TEST_F(TypeManagerTest, GetSameTypeReturnsSamePtr) {
    TypeManager tm;
    auto* t = tm.Get<sem::I32>();
    ASSERT_NE(t, nullptr);
    EXPECT_TRUE(t->Is<sem::I32>());

    auto* t2 = tm.Get<sem::I32>();
    EXPECT_EQ(t, t2);
}

TEST_F(TypeManagerTest, GetDifferentTypeReturnsDifferentPtr) {
    TypeManager tm;
    type::Type* t = tm.Get<sem::I32>();
    ASSERT_NE(t, nullptr);
    EXPECT_TRUE(t->Is<sem::I32>());

    type::Type* t2 = tm.Get<sem::U32>();
    ASSERT_NE(t2, nullptr);
    EXPECT_NE(t, t2);
    EXPECT_TRUE(t2->Is<sem::U32>());
}

TEST_F(TypeManagerTest, Find) {
    TypeManager tm;
    auto* created = tm.Get<sem::I32>();

    EXPECT_EQ(tm.Find<sem::U32>(), nullptr);
    EXPECT_EQ(tm.Find<sem::I32>(), created);
}

TEST_F(TypeManagerTest, WrapDoesntAffectInner) {
    TypeManager inner;
    TypeManager outer = TypeManager::Wrap(inner);

    inner.Get<sem::I32>();

    EXPECT_EQ(count(inner), 1u);
    EXPECT_EQ(count(outer), 0u);

    outer.Get<sem::U32>();

    EXPECT_EQ(count(inner), 1u);
    EXPECT_EQ(count(outer), 1u);
}

}  // namespace
}  // namespace tint::type
