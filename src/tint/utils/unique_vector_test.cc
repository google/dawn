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

#include "src/tint/utils/unique_vector.h"
#include "src/tint/utils/reverse.h"

#include "gtest/gtest.h"

namespace tint::utils {
namespace {

TEST(UniqueVectorTest, Empty) {
    UniqueVector<int> unique_vec;
    EXPECT_EQ(unique_vec.size(), 0u);
    EXPECT_EQ(unique_vec.empty(), true);
    EXPECT_EQ(unique_vec.begin(), unique_vec.end());
}

TEST(UniqueVectorTest, MoveConstructor) {
    UniqueVector<int> unique_vec(std::vector<int>{0, 3, 2, 1, 2});
    EXPECT_EQ(unique_vec.size(), 4u);
    EXPECT_EQ(unique_vec.empty(), false);
    EXPECT_EQ(unique_vec[0], 0);
    EXPECT_EQ(unique_vec[1], 3);
    EXPECT_EQ(unique_vec[2], 2);
    EXPECT_EQ(unique_vec[3], 1);
}

TEST(UniqueVectorTest, AddUnique) {
    UniqueVector<int> unique_vec;
    unique_vec.add(0);
    unique_vec.add(1);
    unique_vec.add(2);
    EXPECT_EQ(unique_vec.size(), 3u);
    EXPECT_EQ(unique_vec.empty(), false);
    int i = 0;
    for (auto n : unique_vec) {
        EXPECT_EQ(n, i);
        i++;
    }
    for (auto n : Reverse(unique_vec)) {
        i--;
        EXPECT_EQ(n, i);
    }
    EXPECT_EQ(unique_vec[0], 0);
    EXPECT_EQ(unique_vec[1], 1);
    EXPECT_EQ(unique_vec[2], 2);
}

TEST(UniqueVectorTest, AddDuplicates) {
    UniqueVector<int> unique_vec;
    unique_vec.add(0);
    unique_vec.add(0);
    unique_vec.add(0);
    unique_vec.add(1);
    unique_vec.add(1);
    unique_vec.add(2);
    EXPECT_EQ(unique_vec.size(), 3u);
    EXPECT_EQ(unique_vec.empty(), false);
    int i = 0;
    for (auto n : unique_vec) {
        EXPECT_EQ(n, i);
        i++;
    }
    for (auto n : Reverse(unique_vec)) {
        i--;
        EXPECT_EQ(n, i);
    }
    EXPECT_EQ(unique_vec[0], 0);
    EXPECT_EQ(unique_vec[1], 1);
    EXPECT_EQ(unique_vec[2], 2);
}

TEST(UniqueVectorTest, AsVector) {
    UniqueVector<int> unique_vec;
    unique_vec.add(0);
    unique_vec.add(0);
    unique_vec.add(0);
    unique_vec.add(1);
    unique_vec.add(1);
    unique_vec.add(2);

    const std::vector<int>& vec = unique_vec;
    EXPECT_EQ(vec.size(), 3u);
    EXPECT_EQ(unique_vec.empty(), false);
    int i = 0;
    for (auto n : vec) {
        EXPECT_EQ(n, i);
        i++;
    }
    for (auto n : Reverse(unique_vec)) {
        i--;
        EXPECT_EQ(n, i);
    }
}

TEST(UniqueVectorTest, PopBack) {
    UniqueVector<int> unique_vec;
    unique_vec.add(0);
    unique_vec.add(2);
    unique_vec.add(1);

    EXPECT_EQ(unique_vec.pop_back(), 1);
    EXPECT_EQ(unique_vec.size(), 2u);
    EXPECT_EQ(unique_vec.empty(), false);
    EXPECT_EQ(unique_vec[0], 0);
    EXPECT_EQ(unique_vec[1], 2);

    EXPECT_EQ(unique_vec.pop_back(), 2);
    EXPECT_EQ(unique_vec.size(), 1u);
    EXPECT_EQ(unique_vec.empty(), false);
    EXPECT_EQ(unique_vec[0], 0);

    unique_vec.add(1);

    EXPECT_EQ(unique_vec.size(), 2u);
    EXPECT_EQ(unique_vec.empty(), false);
    EXPECT_EQ(unique_vec[0], 0);
    EXPECT_EQ(unique_vec[1], 1);

    EXPECT_EQ(unique_vec.pop_back(), 1);
    EXPECT_EQ(unique_vec.size(), 1u);
    EXPECT_EQ(unique_vec.empty(), false);
    EXPECT_EQ(unique_vec[0], 0);

    EXPECT_EQ(unique_vec.pop_back(), 0);
    EXPECT_EQ(unique_vec.size(), 0u);
    EXPECT_EQ(unique_vec.empty(), true);
}

}  // namespace
}  // namespace tint::utils
