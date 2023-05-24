// Copyright 2023 The Tint Authors.
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

#include "src/tint/constant/manager.h"

#include "gtest/gtest.h"
#include "src/tint/constant/scalar.h"
#include "src/tint/type/abstract_float.h"
#include "src/tint/type/abstract_int.h"
#include "src/tint/type/bool.h"
#include "src/tint/type/f16.h"
#include "src/tint/type/f32.h"
#include "src/tint/type/i32.h"
#include "src/tint/type/manager.h"
#include "src/tint/type/u32.h"

namespace tint::constant {
namespace {

using namespace tint::number_suffixes;  // NOLINT

template <typename T>
size_t count(const T& range_loopable) {
    size_t n = 0;
    for (auto it : range_loopable) {
        (void)it;
        n++;
    }
    return n;
}

using ManagerTest = testing::Test;

TEST_F(ManagerTest, GetUnregistered) {
    constant::Manager cm;

    auto* c = cm.Get(1_i);
    static_assert(std::is_same_v<const Scalar<i32>*, decltype(c)>);
    ASSERT_NE(c, nullptr);
}

TEST_F(ManagerTest, GetSameConstantReturnsSamePtr) {
    constant::Manager cm;

    auto* c1 = cm.Get(1_i);
    static_assert(std::is_same_v<const Scalar<i32>*, decltype(c1)>);
    ASSERT_NE(c1, nullptr);

    auto* c2 = cm.Get(1_i);
    static_assert(std::is_same_v<const Scalar<i32>*, decltype(c2)>);
    EXPECT_EQ(c1, c2);
    EXPECT_EQ(c1->Type(), c2->Type());
}

TEST_F(ManagerTest, GetDifferentTypeReturnsDifferentPtr) {
    constant::Manager cm;

    auto* c1 = cm.Get(1_i);
    static_assert(std::is_same_v<const Scalar<i32>*, decltype(c1)>);
    ASSERT_NE(c1, nullptr);

    auto* c2 = cm.Get(1_u);
    static_assert(std::is_same_v<const Scalar<u32>*, decltype(c2)>);
    EXPECT_NE(static_cast<const Value*>(c1), static_cast<const Value*>(c2));
    EXPECT_NE(c1->Type(), c2->Type());
}

TEST_F(ManagerTest, GetDifferentValueReturnsDifferentPtr) {
    constant::Manager cm;

    auto* c1 = cm.Get(1_i);
    static_assert(std::is_same_v<const Scalar<i32>*, decltype(c1)>);
    ASSERT_NE(c1, nullptr);

    auto* c2 = cm.Get(2_i);
    static_assert(std::is_same_v<const Scalar<i32>*, decltype(c2)>);
    ASSERT_NE(c2, nullptr);
    EXPECT_NE(c1, c2);
    EXPECT_EQ(c1->Type(), c2->Type());
}

TEST_F(ManagerTest, Get_i32) {
    constant::Manager cm;

    auto* c = cm.Get(1_i);
    static_assert(std::is_same_v<const Scalar<i32>*, decltype(c)>);
    ASSERT_TRUE(Is<type::I32>(c->Type()));
    EXPECT_EQ(c->value, 1_i);
}

TEST_F(ManagerTest, Get_u32) {
    constant::Manager cm;

    auto* c = cm.Get(1_u);
    static_assert(std::is_same_v<const Scalar<u32>*, decltype(c)>);
    ASSERT_TRUE(Is<type::U32>(c->Type()));
    EXPECT_EQ(c->value, 1_u);
}

TEST_F(ManagerTest, Get_f32) {
    constant::Manager cm;

    auto* c = cm.Get(1_f);
    static_assert(std::is_same_v<const Scalar<f32>*, decltype(c)>);
    ASSERT_TRUE(Is<type::F32>(c->Type()));
    EXPECT_EQ(c->value, 1_f);
}

TEST_F(ManagerTest, Get_f16) {
    constant::Manager cm;

    auto* c = cm.Get(1_h);
    static_assert(std::is_same_v<const Scalar<f16>*, decltype(c)>);
    ASSERT_TRUE(Is<type::F16>(c->Type()));
    EXPECT_EQ(c->value, 1_h);
}

TEST_F(ManagerTest, Get_bool) {
    constant::Manager cm;

    auto* c = cm.Get(true);
    static_assert(std::is_same_v<const Scalar<bool>*, decltype(c)>);
    ASSERT_TRUE(Is<type::Bool>(c->Type()));
    EXPECT_EQ(c->value, true);
}

TEST_F(ManagerTest, Get_AFloat) {
    constant::Manager cm;

    auto* c = cm.Get(1._a);
    static_assert(std::is_same_v<const Scalar<AFloat>*, decltype(c)>);
    ASSERT_TRUE(Is<type::AbstractFloat>(c->Type()));
    EXPECT_EQ(c->value, 1._a);
}

TEST_F(ManagerTest, Get_AInt) {
    constant::Manager cm;

    auto* c = cm.Get(1_a);
    static_assert(std::is_same_v<const Scalar<AInt>*, decltype(c)>);
    ASSERT_TRUE(Is<type::AbstractInt>(c->Type()));
    EXPECT_EQ(c->value, 1_a);
}

TEST_F(ManagerTest, WrapDoesntAffectInner_Constant) {
    Manager inner;
    Manager outer = Manager::Wrap(inner);

    inner.Get(1_i);

    EXPECT_EQ(count(inner), 1u);
    EXPECT_EQ(count(outer), 0u);

    outer.Get(1_i);

    EXPECT_EQ(count(inner), 1u);
    EXPECT_EQ(count(outer), 1u);
}

TEST_F(ManagerTest, WrapDoesntAffectInner_Types) {
    Manager inner;
    Manager outer = Manager::Wrap(inner);

    inner.types.Get<type::I32>();

    EXPECT_EQ(count(inner.types), 1u);
    EXPECT_EQ(count(outer.types), 0u);

    outer.types.Get<type::U32>();

    EXPECT_EQ(count(inner.types), 1u);
    EXPECT_EQ(count(outer.types), 1u);
}

}  // namespace
}  // namespace tint::constant
