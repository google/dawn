// Copyright 2020 The Tint Authors.
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

#include "src/namer.h"

#include "gtest/gtest.h"

namespace tint {
namespace {

using NamerTest = testing::Test;

TEST_F(NamerTest, ReturnsName) {
  Namer n;
  EXPECT_EQ("tint_6d795f6e616d65", n.NameFor("my_name"));
}

TEST_F(NamerTest, ReturnsSameValueForSameName) {
  Namer n;
  EXPECT_EQ("tint_6e616d6531", n.NameFor("name1"));
  EXPECT_EQ("tint_6e616d6532", n.NameFor("name2"));
  EXPECT_EQ("tint_6e616d6531", n.NameFor("name1"));
}

TEST_F(NamerTest, IsMapped) {
  Namer n;
  EXPECT_FALSE(n.IsMapped("my_name"));
  EXPECT_EQ("tint_6d795f6e616d65", n.NameFor("my_name"));
  EXPECT_TRUE(n.IsMapped("my_name"));
}

}  // namespace
}  // namespace tint
