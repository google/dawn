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
#include "src/reader/wgsl/parser_impl.h"
#include "src/reader/wgsl/parser_impl_test_helper.h"

namespace tint {
namespace reader {
namespace wgsl {
namespace {

TEST_F(ParserImplTest, StructDecorationDecl_Parses) {
  auto* p = parser("[[block]]");
  auto d = p->struct_decoration_decl();
  ASSERT_FALSE(p->has_error());
  EXPECT_EQ(d, ast::StructDecoration::kBlock);
}

TEST_F(ParserImplTest, StructDecorationDecl_MissingAttrRight) {
  auto* p = parser("[[block");
  p->struct_decoration_decl();
  ASSERT_TRUE(p->has_error());
  EXPECT_EQ(p->error(), "1:8: missing ]] for struct decoration");
}

// Note, this isn't an error because it could be an array decoration
TEST_F(ParserImplTest, StructDecorationDecl_InvalidDecoration) {
  auto* p = parser("[[invalid]]");
  p->struct_decoration_decl();
  ASSERT_FALSE(p->has_error());
}

}  // namespace
}  // namespace wgsl
}  // namespace reader
}  // namespace tint
