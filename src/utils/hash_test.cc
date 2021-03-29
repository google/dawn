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

#include "src/utils/hash.h"

#include <string>

#include "gtest/gtest.h"

namespace tint {
namespace utils {
namespace {

TEST(HashTests, Basic) {
  EXPECT_NE(Hash(123), Hash(321));
  EXPECT_NE(Hash(123, 456), Hash(123));
  EXPECT_NE(Hash(123, 456, false), Hash(123, 456));
  EXPECT_NE(Hash(std::string("hello")), Hash(std::string("world")));
}

TEST(HashTests, Order) {
  EXPECT_NE(Hash(123, 456), Hash(456, 123));
}

}  // namespace
}  // namespace utils
}  // namespace tint
