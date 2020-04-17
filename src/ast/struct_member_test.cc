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

#include "src/ast/struct_member.h"

#include <sstream>
#include <utility>

#include "gtest/gtest.h"
#include "src/ast/struct_member_offset_decoration.h"
#include "src/ast/type/i32_type.h"

namespace tint {
namespace ast {
namespace {

using StructMemberTest = testing::Test;

TEST_F(StructMemberTest, Creation) {
  type::I32Type i32;
  StructMemberDecorationList decorations;
  decorations.emplace_back(std::make_unique<StructMemberOffsetDecoration>(4));

  StructMember st{"a", &i32, std::move(decorations)};
  EXPECT_EQ(st.name(), "a");
  EXPECT_EQ(st.type(), &i32);
  EXPECT_EQ(st.decorations().size(), 1u);
  EXPECT_TRUE(st.decorations()[0]->IsOffset());
  EXPECT_EQ(st.line(), 0u);
  EXPECT_EQ(st.column(), 0u);
}

TEST_F(StructMemberTest, CreationWithSource) {
  type::I32Type i32;
  Source s{27, 4};

  StructMember st{s, "a", &i32, {}};
  EXPECT_EQ(st.name(), "a");
  EXPECT_EQ(st.type(), &i32);
  EXPECT_EQ(st.decorations().size(), 0u);
  EXPECT_EQ(st.line(), 27u);
  EXPECT_EQ(st.column(), 4u);
}

TEST_F(StructMemberTest, IsValid) {
  type::I32Type i32;
  StructMember st{"a", &i32, {}};
  EXPECT_TRUE(st.IsValid());
}

TEST_F(StructMemberTest, IsValid_EmptyName) {
  type::I32Type i32;
  StructMember st{"", &i32, {}};
  EXPECT_FALSE(st.IsValid());
}

TEST_F(StructMemberTest, IsValid_NullType) {
  StructMember st{"a", nullptr, {}};
  EXPECT_FALSE(st.IsValid());
}

TEST_F(StructMemberTest, IsValid_Null_Decoration) {
  type::I32Type i32;
  StructMemberDecorationList decorations;
  decorations.emplace_back(std::make_unique<StructMemberOffsetDecoration>(4));
  decorations.push_back(nullptr);

  StructMember st{"a", &i32, std::move(decorations)};
  EXPECT_FALSE(st.IsValid());
}

TEST_F(StructMemberTest, ToStr) {
  type::I32Type i32;
  StructMemberDecorationList decorations;
  decorations.emplace_back(std::make_unique<StructMemberOffsetDecoration>(4));

  StructMember st{"a", &i32, std::move(decorations)};
  std::ostringstream out;
  st.to_str(out, 2);
  EXPECT_EQ(out.str(), "  StructMember{[[ offset 4 ]] a: __i32}\n");
}

TEST_F(StructMemberTest, ToStrNoDecorations) {
  type::I32Type i32;
  StructMember st{"a", &i32, {}};
  std::ostringstream out;
  st.to_str(out, 2);
  EXPECT_EQ(out.str(), "  StructMember{a: __i32}\n");
}

}  // namespace
}  // namespace ast
}  // namespace tint
