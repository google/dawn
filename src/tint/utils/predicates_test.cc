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

#include "src/tint/utils/predicates.h"

#include "gtest/gtest.h"

namespace tint::utils {
namespace {

TEST(PredicatesTest, Eq) {
    auto pred = Eq(3);
    EXPECT_FALSE(pred(1));
    EXPECT_FALSE(pred(2));
    EXPECT_TRUE(pred(3));
    EXPECT_FALSE(pred(4));
    EXPECT_FALSE(pred(5));
}

TEST(PredicatesTest, Ne) {
    auto pred = Ne(3);
    EXPECT_TRUE(pred(1));
    EXPECT_TRUE(pred(2));
    EXPECT_FALSE(pred(3));
    EXPECT_TRUE(pred(4));
    EXPECT_TRUE(pred(5));
}

TEST(PredicatesTest, Gt) {
    auto pred = Gt(3);
    EXPECT_FALSE(pred(1));
    EXPECT_FALSE(pred(2));
    EXPECT_FALSE(pred(3));
    EXPECT_TRUE(pred(4));
    EXPECT_TRUE(pred(5));
}

TEST(PredicatesTest, Lt) {
    auto pred = Lt(3);
    EXPECT_TRUE(pred(1));
    EXPECT_TRUE(pred(2));
    EXPECT_FALSE(pred(3));
    EXPECT_FALSE(pred(4));
    EXPECT_FALSE(pred(5));
}

TEST(PredicatesTest, Ge) {
    auto pred = Ge(3);
    EXPECT_FALSE(pred(1));
    EXPECT_FALSE(pred(2));
    EXPECT_TRUE(pred(3));
    EXPECT_TRUE(pred(4));
    EXPECT_TRUE(pred(5));
}

TEST(PredicatesTest, Le) {
    auto pred = Le(3);
    EXPECT_TRUE(pred(1));
    EXPECT_TRUE(pred(2));
    EXPECT_TRUE(pred(3));
    EXPECT_FALSE(pred(4));
    EXPECT_FALSE(pred(5));
}

TEST(PredicatesTest, IsNull) {
    int i = 1;
    EXPECT_TRUE(IsNull(nullptr));
    EXPECT_FALSE(IsNull(&i));
}

}  // namespace
}  // namespace tint::utils
