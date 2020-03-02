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

#include "src/ast/struct_member_offset_decoration.h"

#include "gtest/gtest.h"

namespace tint {
namespace ast {

using StructMemberOffsetDecorationTest = testing::Test;

TEST_F(StructMemberOffsetDecorationTest, Creation) {
  StructMemberOffsetDecoration d{2};
  EXPECT_EQ(2, d.offset());
}

TEST_F(StructMemberOffsetDecorationTest, Is) {
  StructMemberOffsetDecoration d{2};
  EXPECT_TRUE(d.IsOffset());
}

}  // namespace ast
}  // namespace tint
