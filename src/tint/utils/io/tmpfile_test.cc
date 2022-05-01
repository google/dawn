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

#include "src/tint/utils/io/tmpfile.h"

#include <fstream>

#include "gtest/gtest.h"

namespace tint::utils {
namespace {

TEST(TmpFileTest, WriteReadAppendDelete) {
    std::string path;
    {
        TmpFile tmp;
        if (!tmp) {
            GTEST_SKIP() << "Unable to create a temporary file";
        }

        path = tmp.Path();

        // Write a string to the temporary file
        tmp << "hello world\n";

        // Check the content of the file
        {
            std::ifstream file(path);
            ASSERT_TRUE(file);
            std::string line;
            EXPECT_TRUE(std::getline(file, line));
            EXPECT_EQ(line, "hello world");
            EXPECT_FALSE(std::getline(file, line));
        }

        // Write some more content to the file
        tmp << 42;

        // Check the content of the file again
        {
            std::ifstream file(path);
            ASSERT_TRUE(file);
            std::string line;
            EXPECT_TRUE(std::getline(file, line));
            EXPECT_EQ(line, "hello world");
            EXPECT_TRUE(std::getline(file, line));
            EXPECT_EQ(line, "42");
            EXPECT_FALSE(std::getline(file, line));
        }
    }

    // Check the file has been deleted when it fell out of scope
    std::ifstream file(path);
    ASSERT_FALSE(file);
}

TEST(TmpFileTest, FileExtension) {
    const std::string kExt = ".foo";
    std::string path;
    {
        TmpFile tmp(kExt);
        if (!tmp) {
            GTEST_SKIP() << "Unable create a temporary file";
        }
        path = tmp.Path();
    }

    ASSERT_GT(path.length(), kExt.length());
    EXPECT_EQ(kExt, path.substr(path.length() - kExt.length()));

    // Check the file has been deleted when it fell out of scope
    std::ifstream file(path);
    ASSERT_FALSE(file);
}

}  // namespace
}  // namespace tint::utils
