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

#include "src/tint/utils/bitcast.h"

#include <stdint.h>

#include "gtest/gtest.h"

namespace tint::utils {
namespace {

TEST(Bitcast, Integer) {
    uint32_t a = 123;
    int32_t b = Bitcast<int32_t>(a);
    EXPECT_EQ(a, static_cast<uint32_t>(b));
}

TEST(Bitcast, Pointer) {
    uint32_t a = 123;
    void* b = Bitcast<void*>(&a);
    EXPECT_EQ(&a, static_cast<uint32_t*>(b));
}

}  // namespace
}  // namespace tint::utils
