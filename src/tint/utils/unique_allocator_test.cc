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

#include "src/tint/utils/unique_allocator.h"

#include <string>

#include "gtest/gtest.h"

namespace tint::utils {
namespace {

TEST(UniqueAllocator, Int) {
    UniqueAllocator<int> a;
    EXPECT_NE(a.Get(0), a.Get(1));
    EXPECT_NE(a.Get(1), a.Get(2));
    EXPECT_EQ(a.Get(0), a.Get(0));
    EXPECT_EQ(a.Get(1), a.Get(1));
    EXPECT_EQ(a.Get(2), a.Get(2));
}

TEST(UniqueAllocator, Float) {
    UniqueAllocator<float> a;
    EXPECT_NE(a.Get(0.1f), a.Get(1.1f));
    EXPECT_NE(a.Get(1.1f), a.Get(2.1f));
    EXPECT_EQ(a.Get(0.1f), a.Get(0.1f));
    EXPECT_EQ(a.Get(1.1f), a.Get(1.1f));
    EXPECT_EQ(a.Get(2.1f), a.Get(2.1f));
}

TEST(UniqueAllocator, String) {
    UniqueAllocator<std::string> a;
    EXPECT_NE(a.Get("x"), a.Get("y"));
    EXPECT_NE(a.Get("z"), a.Get("w"));
    EXPECT_EQ(a.Get("x"), a.Get("x"));
    EXPECT_EQ(a.Get("y"), a.Get("y"));
    EXPECT_EQ(a.Get("z"), a.Get("z"));
}

}  // namespace
}  // namespace tint::utils
