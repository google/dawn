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
#include "src/ast/workgroup_decoration.h"
#include "src/reader/wgsl/parser_impl.h"
#include "src/reader/wgsl/parser_impl_test_helper.h"

namespace tint {
namespace reader {
namespace wgsl {
namespace {

TEST_F(ParserImplTest, FunctionDecorationList_Parses) {
  auto* p = parser("[[workgroup_size(2), workgroup_size(3, 4, 5)]]");
  ast::FunctionDecorationList decos;
  ASSERT_TRUE(p->function_decoration_decl(decos));
  ASSERT_FALSE(p->has_error()) << p->error();
  ASSERT_EQ(decos.size(), 2u);

  uint32_t x = 0;
  uint32_t y = 0;
  uint32_t z = 0;
  ASSERT_TRUE(decos[0]->IsWorkgroup());
  std::tie(x, y, z) = decos[0]->AsWorkgroup()->values();
  EXPECT_EQ(x, 2u);

  ASSERT_TRUE(decos[1]->IsWorkgroup());
  std::tie(x, y, z) = decos[1]->AsWorkgroup()->values();
  EXPECT_EQ(x, 3u);
  EXPECT_EQ(y, 4u);
  EXPECT_EQ(z, 5u);
}

TEST_F(ParserImplTest, FunctionDecorationList_Empty) {
  auto* p = parser("[[]]");
  ast::FunctionDecorationList decos;
  ASSERT_FALSE(p->function_decoration_decl(decos));
  ASSERT_TRUE(p->has_error());
  ASSERT_EQ(p->error(),
            "1:3: missing decorations for function decoration block");
}

TEST_F(ParserImplTest, FunctionDecorationList_Invalid) {
  auto* p = parser("[[invalid]]");
  ast::FunctionDecorationList decos;
  ASSERT_TRUE(p->function_decoration_decl(decos));
  ASSERT_FALSE(p->has_error());
  ASSERT_TRUE(decos.empty());
}

TEST_F(ParserImplTest, FunctionDecorationList_ExtraComma) {
  auto* p = parser("[[workgroup_size(2), ]]");
  ast::FunctionDecorationList decos;
  ASSERT_FALSE(p->function_decoration_decl(decos));
  ASSERT_TRUE(p->has_error());
  ASSERT_EQ(p->error(), "1:22: expected decoration but none found");
}

TEST_F(ParserImplTest, FunctionDecorationList_MissingComma) {
  auto* p = parser("[[workgroup_size(2) workgroup_size(2)]]");
  ast::FunctionDecorationList decos;
  ASSERT_FALSE(p->function_decoration_decl(decos));
  ASSERT_TRUE(p->has_error());
  ASSERT_EQ(p->error(), "1:21: missing ]] for function decorations");
}

TEST_F(ParserImplTest, FunctionDecorationList_BadDecoration) {
  auto* p = parser("[[workgroup_size()]]");
  ast::FunctionDecorationList decos;
  ASSERT_FALSE(p->function_decoration_decl(decos));
  ASSERT_TRUE(p->has_error());
  ASSERT_EQ(p->error(), "1:18: missing x value for workgroup_size");
}

TEST_F(ParserImplTest, FunctionDecorationList_MissingRightAttr) {
  auto* p = parser("[[workgroup_size(2), workgroup_size(3, 4, 5)");
  ast::FunctionDecorationList decos;
  ASSERT_FALSE(p->function_decoration_decl(decos));
  ASSERT_TRUE(p->has_error());
  ASSERT_EQ(p->error(), "1:45: missing ]] for function decorations");
}

}  // namespace
}  // namespace wgsl
}  // namespace reader
}  // namespace tint
