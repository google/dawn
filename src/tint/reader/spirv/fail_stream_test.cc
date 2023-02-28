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

#include "src/tint/reader/spirv/fail_stream.h"

#include "gmock/gmock.h"
#include "src/tint/utils/string_stream.h"

namespace tint::reader::spirv {
namespace {

using ::testing::Eq;

using FailStreamTest = ::testing::Test;

TEST_F(FailStreamTest, ConversionToBoolIsSameAsStatusMethod) {
    bool flag = true;
    FailStream fs(&flag, nullptr);

    EXPECT_TRUE(fs.status());
    EXPECT_TRUE(bool(fs));  // NOLINT
    flag = false;
    EXPECT_FALSE(fs.status());
    EXPECT_FALSE(bool(fs));  // NOLINT
    flag = true;
    EXPECT_TRUE(fs.status());
    EXPECT_TRUE(bool(fs));  // NOLINT
}

TEST_F(FailStreamTest, FailMethodChangesStatusToFalse) {
    bool flag = true;
    FailStream fs(&flag, nullptr);
    EXPECT_TRUE(flag);
    EXPECT_TRUE(bool(fs));  // NOLINT
    fs.Fail();
    EXPECT_FALSE(flag);
    EXPECT_FALSE(bool(fs));  // NOLINT
}

TEST_F(FailStreamTest, FailMethodReturnsSelf) {
    bool flag = true;
    FailStream fs(&flag, nullptr);
    FailStream& result = fs.Fail();
    EXPECT_THAT(&result, Eq(&fs));
}

TEST_F(FailStreamTest, ShiftOperatorAccumulatesValues) {
    bool flag = true;
    utils::StringStream ss;
    FailStream fs(&flag, &ss);

    ss << "prefix ";
    fs << "cat " << 42;

    EXPECT_THAT(ss.str(), Eq("prefix cat 42"));
}

}  // namespace
}  // namespace tint::reader::spirv
