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

using Namer_HashingNamer_Test = testing::Test;

TEST_F(Namer_HashingNamer_Test, ReturnsName) {
  HashingNamer n;
  EXPECT_EQ("tint_6d795f6e616d65", n.NameFor("my_name"));
}

TEST_F(Namer_HashingNamer_Test, ReturnsSameValueForSameName) {
  HashingNamer n;
  EXPECT_EQ("tint_6e616d6531", n.NameFor("name1"));
  EXPECT_EQ("tint_6e616d6532", n.NameFor("name2"));
  EXPECT_EQ("tint_6e616d6531", n.NameFor("name1"));
}

TEST_F(Namer_HashingNamer_Test, IsMapped) {
  HashingNamer n;
  EXPECT_FALSE(n.IsMapped("my_name"));
  EXPECT_EQ("tint_6d795f6e616d65", n.NameFor("my_name"));
  EXPECT_TRUE(n.IsMapped("my_name"));
}

using Namer_NoopNamer_Test = testing::Test;

TEST_F(Namer_NoopNamer_Test, ReturnsName) {
  NoopNamer n;
  EXPECT_EQ("my_name", n.NameFor("my_name"));
}

TEST_F(Namer_NoopNamer_Test, ReturnsSameValueForSameName) {
  NoopNamer n;
  EXPECT_EQ("name1", n.NameFor("name1"));
  EXPECT_EQ("name2", n.NameFor("name2"));
  EXPECT_EQ("name1", n.NameFor("name1"));
}

TEST_F(Namer_NoopNamer_Test, IsMapped) {
  NoopNamer n;
  EXPECT_FALSE(n.IsMapped("my_name"));
  EXPECT_EQ("my_name", n.NameFor("my_name"));
  EXPECT_TRUE(n.IsMapped("my_name"));
}

}  // namespace
}  // namespace tint
