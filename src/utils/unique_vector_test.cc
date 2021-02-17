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

#include "src/utils/unique_vector.h"

#include "gtest/gtest.h"

namespace tint {
namespace {

TEST(UniqueVectorTest, Empty) {
  UniqueVector<int> unique_vec;
  EXPECT_EQ(unique_vec.size(), 0u);
  EXPECT_EQ(unique_vec.begin(), unique_vec.end());
}

TEST(UniqueVectorTest, AddUnique) {
  UniqueVector<int> unique_vec;
  unique_vec.add(0);
  unique_vec.add(1);
  unique_vec.add(2);
  EXPECT_EQ(unique_vec.size(), 3u);
  int i = 0;
  for (auto n : unique_vec) {
    EXPECT_EQ(n, i);
    i++;
  }
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
  int i = 0;
  for (auto n : unique_vec) {
    EXPECT_EQ(n, i);
    i++;
  }
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
  int i = 0;
  for (auto n : vec) {
    EXPECT_EQ(n, i);
    i++;
  }
}

}  // namespace
}  // namespace tint
