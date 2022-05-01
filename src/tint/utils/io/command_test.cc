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

#include "src/tint/utils/io/command.h"

#include "gtest/gtest.h"

namespace tint::utils {
namespace {

#ifdef _WIN32

TEST(CommandTest, Echo) {
    auto cmd = Command::LookPath("cmd");
    if (!cmd.Found()) {
        GTEST_SKIP() << "cmd not found on PATH";
    }

    auto res = cmd("/C", "echo", "hello world");
    EXPECT_EQ(res.error_code, 0);
    EXPECT_EQ(res.out, "hello world\r\n");
    EXPECT_EQ(res.err, "");
}

#else

TEST(CommandTest, Echo) {
    auto cmd = Command::LookPath("echo");
    if (!cmd.Found()) {
        GTEST_SKIP() << "echo not found on PATH";
    }

    auto res = cmd("hello world");
    EXPECT_EQ(res.error_code, 0);
    EXPECT_EQ(res.out, "hello world\n");
    EXPECT_EQ(res.err, "");
}

TEST(CommandTest, Cat) {
    auto cmd = Command::LookPath("cat");
    if (!cmd.Found()) {
        GTEST_SKIP() << "cat not found on PATH";
    }

    cmd.SetInput("hello world");
    auto res = cmd();
    EXPECT_EQ(res.error_code, 0);
    EXPECT_EQ(res.out, "hello world");
    EXPECT_EQ(res.err, "");
}

TEST(CommandTest, True) {
    auto cmd = Command::LookPath("true");
    if (!cmd.Found()) {
        GTEST_SKIP() << "true not found on PATH";
    }

    auto res = cmd();
    EXPECT_EQ(res.error_code, 0);
    EXPECT_EQ(res.out, "");
    EXPECT_EQ(res.err, "");
}

TEST(CommandTest, False) {
    auto cmd = Command::LookPath("false");
    if (!cmd.Found()) {
        GTEST_SKIP() << "false not found on PATH";
    }

    auto res = cmd();
    EXPECT_NE(res.error_code, 0);
    EXPECT_EQ(res.out, "");
    EXPECT_EQ(res.err, "");
}

#endif

}  // namespace
}  // namespace tint::utils
