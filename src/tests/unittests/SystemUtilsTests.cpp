// Copyright 2019 The Dawn Authors
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

#include <gmock/gmock-matchers.h>
#include <gtest/gtest.h>

#include "common/Assert.h"
#include "common/SystemUtils.h"

using ::testing::_;
using ::testing::Pair;

// Tests for GetEnvironmentVar
TEST(SystemUtilsTests, GetEnvironmentVar) {
    // Test nonexistent environment variable
    EXPECT_THAT(GetEnvironmentVar("NonexistentEnvironmentVar"), Pair("", false));
}

// Tests for SetEnvironmentVar
TEST(SystemUtilsTests, SetEnvironmentVar) {
    // Test new environment variable
    EXPECT_TRUE(SetEnvironmentVar("EnvironmentVarForTest", "NewEnvironmentVarValue"));
    EXPECT_THAT(GetEnvironmentVar("EnvironmentVarForTest"), Pair("NewEnvironmentVarValue", true));
    // Test override environment variable
    EXPECT_TRUE(SetEnvironmentVar("EnvironmentVarForTest", "OverrideEnvironmentVarValue"));
    EXPECT_THAT(GetEnvironmentVar("EnvironmentVarForTest"),
                Pair("OverrideEnvironmentVarValue", true));
}

// Tests for GetExecutableDirectory
TEST(SystemUtilsTests, GetExecutableDirectory) {
    // Test returned value is non-empty string
    EXPECT_NE(GetExecutableDirectory(), "");
    // Test last charecter in path
    EXPECT_EQ(GetExecutableDirectory().back(), *GetPathSeparator());
}

// Tests for ScopedEnvironmentVar
TEST(SystemUtilsTests, ScopedEnvironmentVar) {
    SetEnvironmentVar("ScopedEnvironmentVarForTest", "original");

    // Test empty environment variable doesn't crash
    { ScopedEnvironmentVar var; }

    // Test setting empty environment variable
    {
        ScopedEnvironmentVar var;
        var.Set("ScopedEnvironmentVarForTest", "NewEnvironmentVarValue");
        EXPECT_THAT(GetEnvironmentVar("ScopedEnvironmentVarForTest"),
                    Pair("NewEnvironmentVarValue", true));
    }
    EXPECT_THAT(GetEnvironmentVar("ScopedEnvironmentVarForTest"), Pair("original", true));

    // Test that the environment variable can be set, and it is unset at the end of the scope.
    {
        ScopedEnvironmentVar var("ScopedEnvironmentVarForTest", "NewEnvironmentVarValue");
        EXPECT_THAT(GetEnvironmentVar("ScopedEnvironmentVarForTest"),
                    Pair("NewEnvironmentVarValue", true));
    }
    EXPECT_THAT(GetEnvironmentVar("ScopedEnvironmentVarForTest"), Pair("original", true));

    // Test nested scopes
    {
        ScopedEnvironmentVar outer("ScopedEnvironmentVarForTest", "outer");
        EXPECT_THAT(GetEnvironmentVar("ScopedEnvironmentVarForTest"), Pair("outer", true));
        {
            ScopedEnvironmentVar inner("ScopedEnvironmentVarForTest", "inner");
            EXPECT_THAT(GetEnvironmentVar("ScopedEnvironmentVarForTest"), Pair("inner", true));
        }
        EXPECT_THAT(GetEnvironmentVar("ScopedEnvironmentVarForTest"), Pair("outer", true));
    }
    EXPECT_THAT(GetEnvironmentVar("ScopedEnvironmentVarForTest"), Pair("original", true));

    // Test redundantly setting scoped variables
    {
        ScopedEnvironmentVar var1("ScopedEnvironmentVarForTest", "var1");
        EXPECT_THAT(GetEnvironmentVar("ScopedEnvironmentVarForTest"), Pair("var1", true));

        ScopedEnvironmentVar var2("ScopedEnvironmentVarForTest", "var2");
        EXPECT_THAT(GetEnvironmentVar("ScopedEnvironmentVarForTest"), Pair("var2", true));
    }
    EXPECT_THAT(GetEnvironmentVar("ScopedEnvironmentVarForTest"), Pair("original", true));
}

// Test that restoring a scoped environment variable to the empty string.
TEST(SystemUtilsTests, ScopedEnvironmentVarRestoresEmptyString) {
    ScopedEnvironmentVar empty("ScopedEnvironmentVarForTest", "");
    {
        ScopedEnvironmentVar var1("ScopedEnvironmentVarForTest", "var1");
        EXPECT_THAT(GetEnvironmentVar("ScopedEnvironmentVarForTest"), Pair("var1", true));
    }
    EXPECT_THAT(GetEnvironmentVar("ScopedEnvironmentVarForTest"), Pair("", true));
}

// Test that restoring a scoped environment variable to not set (distuishable from empty string)
// works.
TEST(SystemUtilsTests, ScopedEnvironmentVarRestoresNotSet) {
    ScopedEnvironmentVar null("ScopedEnvironmentVarForTest", nullptr);
    {
        ScopedEnvironmentVar var1("ScopedEnvironmentVarForTest", "var1");
        EXPECT_THAT(GetEnvironmentVar("ScopedEnvironmentVarForTest"), Pair("var1", true));
    }
    EXPECT_THAT(GetEnvironmentVar("ScopedEnvironmentVarForTest"), Pair("", false));
}
