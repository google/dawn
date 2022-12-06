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

#include <vector>

#include "src/tint/utils/reverse.h"

#include "gtest/gtest.h"

namespace tint::utils {
namespace {

TEST(UniqueVectorTest, Empty) {
    UniqueVector<int, 4> unique_vec;
    EXPECT_EQ(unique_vec.Length(), 0u);
    EXPECT_EQ(unique_vec.IsEmpty(), true);
    EXPECT_EQ(unique_vec.begin(), unique_vec.end());
}

TEST(UniqueVectorTest, MoveConstructor) {
    UniqueVector<int, 4> unique_vec(std::vector<int>{0, 3, 2, 1, 2});
    EXPECT_EQ(unique_vec.Length(), 4u);
    EXPECT_EQ(unique_vec.IsEmpty(), false);
    EXPECT_EQ(unique_vec[0], 0);
    EXPECT_EQ(unique_vec[1], 3);
    EXPECT_EQ(unique_vec[2], 2);
    EXPECT_EQ(unique_vec[3], 1);
}

TEST(UniqueVectorTest, AddUnique) {
    UniqueVector<int, 4> unique_vec;
    unique_vec.Add(0);
    unique_vec.Add(1);
    unique_vec.Add(2);
    EXPECT_EQ(unique_vec.Length(), 3u);
    EXPECT_EQ(unique_vec.IsEmpty(), false);
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
    UniqueVector<int, 4> unique_vec;
    unique_vec.Add(0);
    unique_vec.Add(0);
    unique_vec.Add(0);
    unique_vec.Add(1);
    unique_vec.Add(1);
    unique_vec.Add(2);
    EXPECT_EQ(unique_vec.Length(), 3u);
    EXPECT_EQ(unique_vec.IsEmpty(), false);
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
    UniqueVector<int, 4> unique_vec;
    unique_vec.Add(0);
    unique_vec.Add(0);
    unique_vec.Add(0);
    unique_vec.Add(1);
    unique_vec.Add(1);
    unique_vec.Add(2);

    utils::VectorRef<int> ref = unique_vec;
    EXPECT_EQ(ref.Length(), 3u);
    EXPECT_EQ(unique_vec.IsEmpty(), false);
    int i = 0;
    for (auto n : ref) {
        EXPECT_EQ(n, i);
        i++;
    }
    for (auto n : Reverse(unique_vec)) {
        i--;
        EXPECT_EQ(n, i);
    }
}

TEST(UniqueVectorTest, PopBack) {
    UniqueVector<int, 4> unique_vec;
    unique_vec.Add(0);
    unique_vec.Add(2);
    unique_vec.Add(1);

    EXPECT_EQ(unique_vec.Pop(), 1);
    EXPECT_EQ(unique_vec.Length(), 2u);
    EXPECT_EQ(unique_vec.IsEmpty(), false);
    EXPECT_EQ(unique_vec[0], 0);
    EXPECT_EQ(unique_vec[1], 2);

    EXPECT_EQ(unique_vec.Pop(), 2);
    EXPECT_EQ(unique_vec.Length(), 1u);
    EXPECT_EQ(unique_vec.IsEmpty(), false);
    EXPECT_EQ(unique_vec[0], 0);

    unique_vec.Add(1);

    EXPECT_EQ(unique_vec.Length(), 2u);
    EXPECT_EQ(unique_vec.IsEmpty(), false);
    EXPECT_EQ(unique_vec[0], 0);
    EXPECT_EQ(unique_vec[1], 1);

    EXPECT_EQ(unique_vec.Pop(), 1);
    EXPECT_EQ(unique_vec.Length(), 1u);
    EXPECT_EQ(unique_vec.IsEmpty(), false);
    EXPECT_EQ(unique_vec[0], 0);

    EXPECT_EQ(unique_vec.Pop(), 0);
    EXPECT_EQ(unique_vec.Length(), 0u);
    EXPECT_EQ(unique_vec.IsEmpty(), true);
}

TEST(UniqueVectorTest, Data) {
    UniqueVector<int, 4> unique_vec;
    EXPECT_EQ(unique_vec.Data(), nullptr);

    unique_vec.Add(42);
    EXPECT_EQ(unique_vec.Data(), &unique_vec[0]);
    EXPECT_EQ(*unique_vec.Data(), 42);
}

}  // namespace
}  // namespace tint::utils
