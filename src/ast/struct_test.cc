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

#include "src/ast/struct.h"

#include <memory>
#include <sstream>
#include <utility>

#include "gtest/gtest.h"
#include "src/ast/struct_decoration.h"
#include "src/ast/struct_member.h"
#include "src/ast/type/i32_type.h"

namespace tint {
namespace ast {
namespace {

using StructTest = testing::Test;

TEST_F(StructTest, Creation) {
  type::I32Type i32;
  StructMemberList members;
  members.push_back(
      std::make_unique<StructMember>("a", &i32, StructMemberDecorationList()));

  Struct s{StructDecoration::kNone, std::move(members)};
  EXPECT_EQ(s.members().size(), 1u);
  EXPECT_EQ(s.decoration(), StructDecoration::kNone);
  EXPECT_EQ(s.line(), 0u);
  EXPECT_EQ(s.column(), 0u);
}

TEST_F(StructTest, CreationWithSource) {
  type::I32Type i32;
  Source source{27, 4};
  StructMemberList members;
  members.emplace_back(
      std::make_unique<StructMember>("a", &i32, StructMemberDecorationList()));

  Struct s{source, StructDecoration::kNone, std::move(members)};
  EXPECT_EQ(s.members().size(), 1u);
  EXPECT_EQ(s.decoration(), StructDecoration::kNone);
  EXPECT_EQ(s.line(), 27u);
  EXPECT_EQ(s.column(), 4u);
}

TEST_F(StructTest, IsValid) {
  Struct s;
  EXPECT_TRUE(s.IsValid());
}

TEST_F(StructTest, IsValid_Null_StructMember) {
  type::I32Type i32;
  StructMemberList members;
  members.push_back(
      std::make_unique<StructMember>("a", &i32, StructMemberDecorationList()));
  members.push_back(nullptr);

  Struct s{StructDecoration::kNone, std::move(members)};
  EXPECT_FALSE(s.IsValid());
}

TEST_F(StructTest, IsValid_Invalid_StructMember) {
  type::I32Type i32;
  StructMemberList members;
  members.push_back(
      std::make_unique<StructMember>("", &i32, StructMemberDecorationList()));

  Struct s{StructDecoration::kNone, std::move(members)};
  EXPECT_FALSE(s.IsValid());
}

TEST_F(StructTest, ToStr) {
  type::I32Type i32;
  Source source{27, 4};
  StructMemberList members;
  members.emplace_back(
      std::make_unique<StructMember>("a", &i32, StructMemberDecorationList()));

  Struct s{source, StructDecoration::kNone, std::move(members)};

  std::ostringstream out;
  s.to_str(out, 2);
  EXPECT_EQ(out.str(), R"(  Struct{
    StructMember{a: __i32}
  }
)");
}

}  // namespace
}  // namespace ast
}  // namespace tint
