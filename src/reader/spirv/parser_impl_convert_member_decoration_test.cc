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

#include "gmock/gmock.h"
#include "src/reader/spirv/parser_impl_test_helper.h"

namespace tint {
namespace reader {
namespace spirv {
namespace {

using ::testing::Eq;

TEST_F(SpvParserTest, ConvertMemberDecoration_Empty) {
  auto p = parser(std::vector<uint32_t>{});

  auto* result = p->ConvertMemberDecoration(1, 1, {});
  EXPECT_EQ(result, nullptr);
  EXPECT_THAT(p->error(), Eq("malformed SPIR-V decoration: it's empty"));
}

TEST_F(SpvParserTest, ConvertMemberDecoration_OffsetWithoutOperand) {
  auto p = parser(std::vector<uint32_t>{});

  auto* result = p->ConvertMemberDecoration(12, 13, {SpvDecorationOffset});
  EXPECT_EQ(result, nullptr);
  EXPECT_THAT(p->error(), Eq("malformed Offset decoration: expected 1 literal "
                             "operand, has 0: member 13 of SPIR-V type 12"));
}

TEST_F(SpvParserTest, ConvertMemberDecoration_OffsetWithTooManyOperands) {
  auto p = parser(std::vector<uint32_t>{});

  auto* result =
      p->ConvertMemberDecoration(12, 13, {SpvDecorationOffset, 3, 4});
  EXPECT_EQ(result, nullptr);
  EXPECT_THAT(p->error(), Eq("malformed Offset decoration: expected 1 literal "
                             "operand, has 2: member 13 of SPIR-V type 12"));
}

TEST_F(SpvParserTest, ConvertMemberDecoration_Offset) {
  auto p = parser(std::vector<uint32_t>{});

  auto* result = p->ConvertMemberDecoration(1, 1, {SpvDecorationOffset, 8});
  ASSERT_NE(result, nullptr);
  EXPECT_TRUE(result->Is<ast::StructMemberOffsetDecoration>());
  auto* offset_deco = result->As<ast::StructMemberOffsetDecoration>();
  ASSERT_NE(offset_deco, nullptr);
  EXPECT_EQ(offset_deco->offset(), 8u);
  EXPECT_TRUE(p->error().empty());
}

TEST_F(SpvParserTest, ConvertMemberDecoration_UnhandledDecoration) {
  auto p = parser(std::vector<uint32_t>{});

  auto* result = p->ConvertMemberDecoration(12, 13, {12345678});
  EXPECT_EQ(result, nullptr);
  EXPECT_THAT(p->error(), Eq("unhandled member decoration: 12345678 on member "
                             "13 of SPIR-V type 12"));
}

}  // namespace
}  // namespace spirv
}  // namespace reader
}  // namespace tint
