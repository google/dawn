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

#include "src/tint/utils/map.h"

#include <unordered_map>

#include "gtest/gtest.h"

namespace tint::utils {
namespace {

TEST(Lookup, Test) {
    std::unordered_map<int, int> map;
    map.emplace(10, 1);
    EXPECT_EQ(Lookup(map, 10, 0), 1);    // exists, with if_missing
    EXPECT_EQ(Lookup(map, 10), 1);       // exists, without if_missing
    EXPECT_EQ(Lookup(map, 20, 50), 50);  // missing, with if_missing
    EXPECT_EQ(Lookup(map, 20), 0);       // missing, without if_missing
}

TEST(GetOrCreateTest, NewKey) {
    std::unordered_map<int, int> map;
    EXPECT_EQ(GetOrCreate(map, 1, [&] { return 2; }), 2);
    EXPECT_EQ(map.size(), 1u);
    EXPECT_EQ(map[1], 2);
}

TEST(GetOrCreateTest, ExistingKey) {
    std::unordered_map<int, int> map;
    map[1] = 2;
    bool called = false;
    EXPECT_EQ(GetOrCreate(map, 1,
                          [&] {
                              called = true;
                              return -2;
                          }),
              2);
    EXPECT_EQ(called, false);
    EXPECT_EQ(map.size(), 1u);
    EXPECT_EQ(map[1], 2);
}

}  // namespace
}  // namespace tint::utils
