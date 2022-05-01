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

#include "src/tint/utils/crc32.h"

#include "gtest/gtest.h"

namespace tint::utils {
namespace {

TEST(CRC32Test, Compiletime) {
    static_assert(CRC32("") == 0x00000000u);
    static_assert(CRC32("hello world") == 0x0d4a1185u);
    static_assert(CRC32("123456789") == 0xcbf43926u);
}

TEST(CRC32Test, Runtime) {
    EXPECT_EQ(CRC32(""), 0x00000000u);
    EXPECT_EQ(CRC32("hello world"), 0x0d4a1185u);
    EXPECT_EQ(CRC32("123456789"), 0xcbf43926u);
}

}  // namespace
}  // namespace tint::utils
