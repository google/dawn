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

#include "gtest/gtest.h"
#include "src/ast/struct_member_offset_decoration.h"
#include "src/reader/wgsl/parser_impl.h"
#include "src/reader/wgsl/parser_impl_test_helper.h"

namespace tint {
namespace reader {
namespace wgsl {
namespace {

TEST_F(ParserImplTest, StructMemberDecoration_Offset) {
  auto* p = parser("offset 4");
  auto deco = p->struct_member_decoration();
  ASSERT_NE(deco, nullptr);
  ASSERT_FALSE(p->has_error());
  ASSERT_TRUE(deco->IsOffset());

  auto* o = deco->AsOffset();
  EXPECT_EQ(o->offset(), 4u);
}

TEST_F(ParserImplTest, StructMemberDecoration_Offset_MissingValue) {
  auto* p = parser("offset");
  auto deco = p->struct_member_decoration();
  ASSERT_EQ(deco, nullptr);
  ASSERT_TRUE(p->has_error());
  EXPECT_EQ(p->error(), "1:7: invalid value for offset decoration");
}

TEST_F(ParserImplTest, StructMemberDecoration_Offset_MissingInvalid) {
  auto* p = parser("offset nan");
  auto deco = p->struct_member_decoration();
  ASSERT_EQ(deco, nullptr);
  ASSERT_TRUE(p->has_error());
  EXPECT_EQ(p->error(), "1:8: invalid value for offset decoration");
}

}  // namespace
}  // namespace wgsl
}  // namespace reader
}  // namespace tint
