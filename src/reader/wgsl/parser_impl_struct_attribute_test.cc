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

#include "src/ast/struct_block_attribute.h"
#include "src/reader/wgsl/parser_impl_test_helper.h"

namespace tint {
namespace reader {
namespace wgsl {
namespace {

struct AttributeData {
  const char* input;
  bool is_block;
};
inline std::ostream& operator<<(std::ostream& out, AttributeData data) {
  out << std::string(data.input);
  return out;
}

class AttributeTest : public ParserImplTestWithParam<AttributeData> {};

TEST_P(AttributeTest, Parses) {
  auto params = GetParam();
  auto p = parser(params.input);

  auto attr = p->attribute();
  ASSERT_FALSE(p->has_error());
  EXPECT_TRUE(attr.matched);
  EXPECT_FALSE(attr.errored);
  ASSERT_NE(attr.value, nullptr);
  auto* struct_attr = attr.value->As<ast::Attribute>();
  ASSERT_NE(struct_attr, nullptr);
  EXPECT_EQ(struct_attr->Is<ast::StructBlockAttribute>(), params.is_block);
}
INSTANTIATE_TEST_SUITE_P(ParserImplTest,
                         AttributeTest,
                         testing::Values(AttributeData{"block", true}));

TEST_F(ParserImplTest, Attribute_NoMatch) {
  auto p = parser("not-a-stage");
  auto attr = p->attribute();
  EXPECT_FALSE(attr.matched);
  EXPECT_FALSE(attr.errored);
  ASSERT_EQ(attr.value, nullptr);
}

}  // namespace
}  // namespace wgsl
}  // namespace reader
}  // namespace tint
