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

#include "src/tint/utils/scoped_assignment.h"

#include "gtest/gtest.h"

namespace tint::utils {
namespace {

TEST(ScopedAssignmentTest, Scopes) {
    int i = 0;
    EXPECT_EQ(i, 0);
    {
        EXPECT_EQ(i, 0);
        TINT_SCOPED_ASSIGNMENT(i, 1);
        EXPECT_EQ(i, 1);
        {
            EXPECT_EQ(i, 1);
            TINT_SCOPED_ASSIGNMENT(i, 2);
            EXPECT_EQ(i, 2);
        }
        {
            EXPECT_EQ(i, 1);
            TINT_SCOPED_ASSIGNMENT(i, 3);
            EXPECT_EQ(i, 3);
        }
        EXPECT_EQ(i, 1);
    }
    EXPECT_EQ(i, 0);
}

}  // namespace
}  // namespace tint::utils
