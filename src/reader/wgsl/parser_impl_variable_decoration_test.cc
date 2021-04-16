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

#include "src/reader/wgsl/parser_impl_test_helper.h"

namespace tint {
namespace reader {
namespace wgsl {
namespace {

TEST_F(ParserImplTest, Decoration_Location) {
  auto p = parser("location(4)");
  auto deco = p->decoration();
  EXPECT_TRUE(deco.matched);
  EXPECT_FALSE(deco.errored);
  ASSERT_NE(deco.value, nullptr);
  auto* var_deco = deco.value->As<ast::Decoration>();
  ASSERT_NE(var_deco, nullptr);
  ASSERT_FALSE(p->has_error());
  ASSERT_TRUE(var_deco->Is<ast::LocationDecoration>());

  auto* loc = var_deco->As<ast::LocationDecoration>();
  EXPECT_EQ(loc->value(), 4u);
}

TEST_F(ParserImplTest, Decoration_Location_MissingLeftParen) {
  auto p = parser("location 4)");
  auto deco = p->decoration();
  EXPECT_FALSE(deco.matched);
  EXPECT_TRUE(deco.errored);
  EXPECT_EQ(deco.value, nullptr);
  EXPECT_TRUE(p->has_error());
  EXPECT_EQ(p->error(), "1:10: expected '(' for location decoration");
}

TEST_F(ParserImplTest, Decoration_Location_MissingRightParen) {
  auto p = parser("location(4");
  auto deco = p->decoration();
  EXPECT_FALSE(deco.matched);
  EXPECT_TRUE(deco.errored);
  EXPECT_EQ(deco.value, nullptr);
  EXPECT_TRUE(p->has_error());
  EXPECT_EQ(p->error(), "1:11: expected ')' for location decoration");
}

TEST_F(ParserImplTest, Decoration_Location_MissingValue) {
  auto p = parser("location()");
  auto deco = p->decoration();
  EXPECT_FALSE(deco.matched);
  EXPECT_TRUE(deco.errored);
  EXPECT_EQ(deco.value, nullptr);
  EXPECT_TRUE(p->has_error());
  EXPECT_EQ(p->error(),
            "1:10: expected signed integer literal for location decoration");
}

TEST_F(ParserImplTest, Decoration_Location_MissingInvalid) {
  auto p = parser("location(nan)");
  auto deco = p->decoration();
  EXPECT_FALSE(deco.matched);
  EXPECT_TRUE(deco.errored);
  EXPECT_EQ(deco.value, nullptr);
  EXPECT_TRUE(p->has_error());
  EXPECT_EQ(p->error(),
            "1:10: expected signed integer literal for location decoration");
}

struct BuiltinData {
  const char* input;
  ast::Builtin result;
};
inline std::ostream& operator<<(std::ostream& out, BuiltinData data) {
  out << std::string(data.input);
  return out;
}

class BuiltinTest : public ParserImplTestWithParam<BuiltinData> {};

TEST_P(BuiltinTest, Decoration_Builtin) {
  auto params = GetParam();
  auto p = parser(std::string("builtin(") + params.input + ")");

  auto deco = p->decoration();
  EXPECT_TRUE(deco.matched);
  EXPECT_FALSE(deco.errored);
  ASSERT_NE(deco.value, nullptr);
  auto* var_deco = deco.value->As<ast::Decoration>();
  ASSERT_FALSE(p->has_error()) << p->error();
  ASSERT_NE(var_deco, nullptr);
  ASSERT_TRUE(var_deco->Is<ast::BuiltinDecoration>());

  auto* builtin = var_deco->As<ast::BuiltinDecoration>();
  EXPECT_EQ(builtin->value(), params.result);
}
INSTANTIATE_TEST_SUITE_P(
    ParserImplTest,
    BuiltinTest,
    testing::Values(
        BuiltinData{"position", ast::Builtin::kPosition},
        BuiltinData{"vertex_idx", ast::Builtin::kVertexIndex},
        BuiltinData{"vertex_index", ast::Builtin::kVertexIndex},
        BuiltinData{"instance_idx", ast::Builtin::kInstanceIndex},
        BuiltinData{"instance_index", ast::Builtin::kInstanceIndex},
        BuiltinData{"front_facing", ast::Builtin::kFrontFacing},
        BuiltinData{"frag_coord", ast::Builtin::kFragCoord},
        BuiltinData{"frag_depth", ast::Builtin::kFragDepth},
        BuiltinData{"local_invocation_id", ast::Builtin::kLocalInvocationId},
        BuiltinData{"local_invocation_idx",
                    ast::Builtin::kLocalInvocationIndex},
        BuiltinData{"local_invocation_index",
                    ast::Builtin::kLocalInvocationIndex},
        BuiltinData{"global_invocation_id", ast::Builtin::kGlobalInvocationId},
        BuiltinData{"workgroup_id", ast::Builtin::kWorkgroupId},
        BuiltinData{"sample_index", ast::Builtin::kSampleIndex},
        BuiltinData{"sample_mask", ast::Builtin::kSampleMask},
        BuiltinData{"sample_mask_in", ast::Builtin::kSampleMaskIn},
        BuiltinData{"sample_mask_out", ast::Builtin::kSampleMaskOut}));

TEST_F(ParserImplTest, Decoration_Builtin_MissingLeftParen) {
  auto p = parser("builtin position)");
  auto deco = p->decoration();
  EXPECT_FALSE(deco.matched);
  EXPECT_TRUE(deco.errored);
  EXPECT_EQ(deco.value, nullptr);
  EXPECT_TRUE(p->has_error());
  EXPECT_EQ(p->error(), "1:9: expected '(' for builtin decoration");
}

TEST_F(ParserImplTest, Decoration_Builtin_MissingRightParen) {
  auto p = parser("builtin(position");
  auto deco = p->decoration();
  EXPECT_FALSE(deco.matched);
  EXPECT_TRUE(deco.errored);
  EXPECT_EQ(deco.value, nullptr);
  EXPECT_TRUE(p->has_error());
  EXPECT_EQ(p->error(), "1:17: expected ')' for builtin decoration");
}

TEST_F(ParserImplTest, Decoration_Builtin_MissingValue) {
  auto p = parser("builtin()");
  auto deco = p->decoration();
  EXPECT_FALSE(deco.matched);
  EXPECT_TRUE(deco.errored);
  EXPECT_EQ(deco.value, nullptr);
  EXPECT_TRUE(p->has_error());
  EXPECT_EQ(p->error(), "1:9: expected identifier for builtin");
}

TEST_F(ParserImplTest, Decoration_Builtin_InvalidValue) {
  auto p = parser("builtin(other_thingy)");
  auto deco = p->decoration();
  EXPECT_FALSE(deco.matched);
  EXPECT_TRUE(deco.errored);
  EXPECT_EQ(deco.value, nullptr);
  EXPECT_TRUE(p->has_error());
  EXPECT_EQ(p->error(), "1:9: invalid value for builtin decoration");
}

TEST_F(ParserImplTest, Decoration_Builtin_MissingInvalid) {
  auto p = parser("builtin(3)");
  auto deco = p->decoration();
  EXPECT_FALSE(deco.matched);
  EXPECT_TRUE(deco.errored);
  EXPECT_EQ(deco.value, nullptr);
  EXPECT_TRUE(p->has_error());
  EXPECT_EQ(p->error(), "1:9: expected identifier for builtin");
}

TEST_F(ParserImplTest, Decoration_Binding) {
  auto p = parser("binding(4)");
  auto deco = p->decoration();
  EXPECT_TRUE(deco.matched);
  EXPECT_FALSE(deco.errored);
  ASSERT_NE(deco.value, nullptr);
  auto* var_deco = deco.value->As<ast::Decoration>();
  ASSERT_NE(var_deco, nullptr);
  ASSERT_FALSE(p->has_error());
  ASSERT_TRUE(var_deco->Is<ast::BindingDecoration>());

  auto* binding = var_deco->As<ast::BindingDecoration>();
  EXPECT_EQ(binding->value(), 4u);
}

TEST_F(ParserImplTest, Decoration_Binding_MissingLeftParen) {
  auto p = parser("binding 4)");
  auto deco = p->decoration();
  EXPECT_FALSE(deco.matched);
  EXPECT_TRUE(deco.errored);
  EXPECT_EQ(deco.value, nullptr);
  EXPECT_TRUE(p->has_error());
  EXPECT_EQ(p->error(), "1:9: expected '(' for binding decoration");
}

TEST_F(ParserImplTest, Decoration_Binding_MissingRightParen) {
  auto p = parser("binding(4");
  auto deco = p->decoration();
  EXPECT_FALSE(deco.matched);
  EXPECT_TRUE(deco.errored);
  EXPECT_EQ(deco.value, nullptr);
  EXPECT_TRUE(p->has_error());
  EXPECT_EQ(p->error(), "1:10: expected ')' for binding decoration");
}

TEST_F(ParserImplTest, Decoration_Binding_MissingValue) {
  auto p = parser("binding()");
  auto deco = p->decoration();
  EXPECT_FALSE(deco.matched);
  EXPECT_TRUE(deco.errored);
  EXPECT_EQ(deco.value, nullptr);
  EXPECT_TRUE(p->has_error());
  EXPECT_EQ(p->error(),
            "1:9: expected signed integer literal for binding decoration");
}

TEST_F(ParserImplTest, Decoration_Binding_MissingInvalid) {
  auto p = parser("binding(nan)");
  auto deco = p->decoration();
  EXPECT_FALSE(deco.matched);
  EXPECT_TRUE(deco.errored);
  EXPECT_EQ(deco.value, nullptr);
  EXPECT_TRUE(p->has_error());
  EXPECT_EQ(p->error(),
            "1:9: expected signed integer literal for binding decoration");
}

// DEPRECATED
TEST_F(ParserImplTest, Decoration_set) {
  auto p = parser("set(4)");
  auto deco = p->decoration();
  EXPECT_TRUE(deco.matched);
  EXPECT_FALSE(deco.errored);
  ASSERT_NE(deco.value, nullptr);
  auto* var_deco = deco.value->As<ast::Decoration>();
  ASSERT_FALSE(p->has_error());
  ASSERT_NE(var_deco, nullptr);
  ASSERT_TRUE(var_deco->Is<ast::GroupDecoration>());

  auto* group = var_deco->As<ast::GroupDecoration>();
  EXPECT_EQ(group->value(), 4u);
}

TEST_F(ParserImplTest, Decoration_group) {
  auto p = parser("group(4)");
  auto deco = p->decoration();
  EXPECT_TRUE(deco.matched);
  EXPECT_FALSE(deco.errored);
  ASSERT_NE(deco.value, nullptr);
  auto* var_deco = deco.value->As<ast::Decoration>();
  ASSERT_FALSE(p->has_error());
  ASSERT_NE(var_deco, nullptr);
  ASSERT_TRUE(var_deco->Is<ast::GroupDecoration>());

  auto* group = var_deco->As<ast::GroupDecoration>();
  EXPECT_EQ(group->value(), 4u);
}

TEST_F(ParserImplTest, Decoration_Group_MissingLeftParen) {
  auto p = parser("group 2)");
  auto deco = p->decoration();
  EXPECT_FALSE(deco.matched);
  EXPECT_TRUE(deco.errored);
  EXPECT_EQ(deco.value, nullptr);
  EXPECT_TRUE(p->has_error());
  EXPECT_EQ(p->error(), "1:7: expected '(' for group decoration");
}

TEST_F(ParserImplTest, Decoration_Group_MissingRightParen) {
  auto p = parser("group(2");
  auto deco = p->decoration();
  EXPECT_FALSE(deco.matched);
  EXPECT_TRUE(deco.errored);
  EXPECT_EQ(deco.value, nullptr);
  EXPECT_TRUE(p->has_error());
  EXPECT_EQ(p->error(), "1:8: expected ')' for group decoration");
}

TEST_F(ParserImplTest, Decoration_Group_MissingValue) {
  auto p = parser("group()");
  auto deco = p->decoration();
  EXPECT_FALSE(deco.matched);
  EXPECT_TRUE(deco.errored);
  EXPECT_EQ(deco.value, nullptr);
  EXPECT_TRUE(p->has_error());
  EXPECT_EQ(p->error(),
            "1:7: expected signed integer literal for group decoration");
}

TEST_F(ParserImplTest, Decoration_Group_MissingInvalid) {
  auto p = parser("group(nan)");
  auto deco = p->decoration();
  EXPECT_FALSE(deco.matched);
  EXPECT_TRUE(deco.errored);
  EXPECT_EQ(deco.value, nullptr);
  EXPECT_TRUE(p->has_error());
  EXPECT_EQ(p->error(),
            "1:7: expected signed integer literal for group decoration");
}

TEST_F(ParserImplTest, Decoration_FragCoord_Deprecated) {
  auto p = parser("builtin(frag_coord)");
  auto deco = p->decoration();
  EXPECT_TRUE(deco.matched);
  EXPECT_FALSE(deco.errored);
  ASSERT_NE(deco.value, nullptr);
  auto* var_deco = deco.value->As<ast::Decoration>();
  ASSERT_NE(var_deco, nullptr);
  ASSERT_FALSE(p->has_error());
  ASSERT_TRUE(var_deco->Is<ast::BuiltinDecoration>());

  auto* builtin = var_deco->As<ast::BuiltinDecoration>();
  EXPECT_EQ(builtin->value(), ast::Builtin::kFragCoord);

  EXPECT_EQ(
      p->builder().Diagnostics().str(),
      R"(test.wgsl:1:9 warning: use of deprecated language feature: use 'position' instead of 'frag_coord'
builtin(frag_coord)
        ^^^^^^^^^^
)");
}

TEST_F(ParserImplTest, Decoration_SampleMaskIn_Deprecated) {
  auto p = parser("builtin(sample_mask_in)");
  auto deco = p->decoration();
  EXPECT_TRUE(deco.matched);
  EXPECT_FALSE(deco.errored);
  ASSERT_NE(deco.value, nullptr);
  auto* var_deco = deco.value->As<ast::Decoration>();
  ASSERT_NE(var_deco, nullptr);
  ASSERT_FALSE(p->has_error());
  ASSERT_TRUE(var_deco->Is<ast::BuiltinDecoration>());

  auto* builtin = var_deco->As<ast::BuiltinDecoration>();
  EXPECT_EQ(builtin->value(), ast::Builtin::kSampleMaskIn);

  EXPECT_EQ(
      p->builder().Diagnostics().str(),
      R"(test.wgsl:1:9 warning: use of deprecated language feature: use 'sample_mask' instead of 'sample_mask_in'
builtin(sample_mask_in)
        ^^^^^^^^^^^^^^
)");
}

TEST_F(ParserImplTest, Decoration_SampleMaskOut_Deprecated) {
  auto p = parser("builtin(sample_mask_out)");
  auto deco = p->decoration();
  EXPECT_TRUE(deco.matched);
  EXPECT_FALSE(deco.errored);
  ASSERT_NE(deco.value, nullptr);
  auto* var_deco = deco.value->As<ast::Decoration>();
  ASSERT_NE(var_deco, nullptr);
  ASSERT_FALSE(p->has_error());
  ASSERT_TRUE(var_deco->Is<ast::BuiltinDecoration>());

  auto* builtin = var_deco->As<ast::BuiltinDecoration>();
  EXPECT_EQ(builtin->value(), ast::Builtin::kSampleMaskOut);

  EXPECT_EQ(
      p->builder().Diagnostics().str(),
      R"(test.wgsl:1:9 warning: use of deprecated language feature: use 'sample_mask' instead of 'sample_mask_out'
builtin(sample_mask_out)
        ^^^^^^^^^^^^^^^
)");
}

}  // namespace
}  // namespace wgsl
}  // namespace reader
}  // namespace tint
