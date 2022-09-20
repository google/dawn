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

#include "src/tint/utils/result.h"

#include <string>

#include "gmock/gmock.h"

namespace tint::utils {
namespace {

TEST(ResultTest, SuccessInt) {
    auto r = Result<int>(123);
    EXPECT_TRUE(r);
    EXPECT_FALSE(!r);
    EXPECT_EQ(r.Get(), 123);
}

TEST(ResultTest, SuccessStruct) {
    struct S {
        int value;
    };
    auto r = Result<S>({123});
    EXPECT_TRUE(r);
    EXPECT_FALSE(!r);
    EXPECT_EQ(r->value, 123);
}

TEST(ResultTest, Failure) {
    auto r = Result<int>(Failure);
    EXPECT_FALSE(r);
    EXPECT_TRUE(!r);
}

TEST(ResultTest, CustomFailure) {
    auto r = Result<int, std::string>("oh noes!");
    EXPECT_FALSE(r);
    EXPECT_TRUE(!r);
    EXPECT_EQ(r.Failure(), "oh noes!");
}

TEST(ResultTest, ValueCast) {
    struct X {};
    struct Y : X {};

    Y* y = nullptr;
    auto r_y = Result<Y*>{y};
    auto r_x = Result<X*>{r_y};

    (void)r_x;
    (void)r_y;
}

}  // namespace
}  // namespace tint::utils
