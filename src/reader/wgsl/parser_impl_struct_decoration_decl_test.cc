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

#include "src/ast/struct_block_decoration.h"
#include "src/reader/wgsl/parser_impl_test_helper.h"

namespace tint {
namespace reader {
namespace wgsl {
namespace {

TEST_F(ParserImplTest, DecorationDecl_Parses) {
  auto p = parser("[[block]]");
  auto decos = p->decoration_list();
  EXPECT_FALSE(p->has_error());
  EXPECT_FALSE(decos.errored);
  EXPECT_TRUE(decos.matched);
  ASSERT_EQ(decos.value.size(), 1u);
  auto* struct_deco = decos.value[0]->As<ast::Decoration>();
  EXPECT_TRUE(struct_deco->Is<ast::StructBlockDecoration>());
}

TEST_F(ParserImplTest, DecorationDecl_MissingAttrRight) {
  auto p = parser("[[block");
  auto decos = p->decoration_list();
  EXPECT_TRUE(p->has_error());
  EXPECT_TRUE(decos.errored);
  EXPECT_FALSE(decos.matched);
  EXPECT_TRUE(decos.value.empty());
  EXPECT_EQ(p->error(), "1:8: expected ']]' for decoration list");
}

TEST_F(ParserImplTest, DecorationDecl_InvalidDecoration) {
  auto p = parser("[[invalid]]");
  auto decos = p->decoration_list();
  EXPECT_TRUE(p->has_error());
  EXPECT_TRUE(decos.errored);
  EXPECT_FALSE(decos.matched);
  EXPECT_TRUE(decos.value.empty());
}

}  // namespace
}  // namespace wgsl
}  // namespace reader
}  // namespace tint
