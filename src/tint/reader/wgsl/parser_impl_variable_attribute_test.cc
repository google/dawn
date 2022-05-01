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

#include "src/tint/reader/wgsl/parser_impl_test_helper.h"

namespace tint::reader::wgsl {
namespace {

TEST_F(ParserImplTest, Attribute_Location) {
    auto p = parser("location(4)");
    auto attr = p->attribute();
    EXPECT_TRUE(attr.matched);
    EXPECT_FALSE(attr.errored);
    ASSERT_NE(attr.value, nullptr);
    auto* var_attr = attr.value->As<ast::Attribute>();
    ASSERT_NE(var_attr, nullptr);
    ASSERT_FALSE(p->has_error());
    ASSERT_TRUE(var_attr->Is<ast::LocationAttribute>());

    auto* loc = var_attr->As<ast::LocationAttribute>();
    EXPECT_EQ(loc->value, 4u);
}

TEST_F(ParserImplTest, Attribute_Location_MissingLeftParen) {
    auto p = parser("location 4)");
    auto attr = p->attribute();
    EXPECT_FALSE(attr.matched);
    EXPECT_TRUE(attr.errored);
    EXPECT_EQ(attr.value, nullptr);
    EXPECT_TRUE(p->has_error());
    EXPECT_EQ(p->error(), "1:10: expected '(' for location attribute");
}

TEST_F(ParserImplTest, Attribute_Location_MissingRightParen) {
    auto p = parser("location(4");
    auto attr = p->attribute();
    EXPECT_FALSE(attr.matched);
    EXPECT_TRUE(attr.errored);
    EXPECT_EQ(attr.value, nullptr);
    EXPECT_TRUE(p->has_error());
    EXPECT_EQ(p->error(), "1:11: expected ')' for location attribute");
}

TEST_F(ParserImplTest, Attribute_Location_MissingValue) {
    auto p = parser("location()");
    auto attr = p->attribute();
    EXPECT_FALSE(attr.matched);
    EXPECT_TRUE(attr.errored);
    EXPECT_EQ(attr.value, nullptr);
    EXPECT_TRUE(p->has_error());
    EXPECT_EQ(p->error(), "1:10: expected signed integer literal for location attribute");
}

TEST_F(ParserImplTest, Attribute_Location_MissingInvalid) {
    auto p = parser("location(nan)");
    auto attr = p->attribute();
    EXPECT_FALSE(attr.matched);
    EXPECT_TRUE(attr.errored);
    EXPECT_EQ(attr.value, nullptr);
    EXPECT_TRUE(p->has_error());
    EXPECT_EQ(p->error(), "1:10: expected signed integer literal for location attribute");
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

TEST_P(BuiltinTest, Attribute_Builtin) {
    auto params = GetParam();
    auto p = parser(std::string("builtin(") + params.input + ")");

    auto attr = p->attribute();
    EXPECT_TRUE(attr.matched);
    EXPECT_FALSE(attr.errored);
    ASSERT_NE(attr.value, nullptr);
    auto* var_attr = attr.value->As<ast::Attribute>();
    ASSERT_FALSE(p->has_error()) << p->error();
    ASSERT_NE(var_attr, nullptr);
    ASSERT_TRUE(var_attr->Is<ast::BuiltinAttribute>());

    auto* builtin = var_attr->As<ast::BuiltinAttribute>();
    EXPECT_EQ(builtin->builtin, params.result);
}
INSTANTIATE_TEST_SUITE_P(
    ParserImplTest,
    BuiltinTest,
    testing::Values(BuiltinData{"position", ast::Builtin::kPosition},
                    BuiltinData{"vertex_index", ast::Builtin::kVertexIndex},
                    BuiltinData{"instance_index", ast::Builtin::kInstanceIndex},
                    BuiltinData{"front_facing", ast::Builtin::kFrontFacing},
                    BuiltinData{"frag_depth", ast::Builtin::kFragDepth},
                    BuiltinData{"local_invocation_id", ast::Builtin::kLocalInvocationId},
                    BuiltinData{"local_invocation_idx", ast::Builtin::kLocalInvocationIndex},
                    BuiltinData{"local_invocation_index", ast::Builtin::kLocalInvocationIndex},
                    BuiltinData{"global_invocation_id", ast::Builtin::kGlobalInvocationId},
                    BuiltinData{"workgroup_id", ast::Builtin::kWorkgroupId},
                    BuiltinData{"num_workgroups", ast::Builtin::kNumWorkgroups},
                    BuiltinData{"sample_index", ast::Builtin::kSampleIndex},
                    BuiltinData{"sample_mask", ast::Builtin::kSampleMask}));

TEST_F(ParserImplTest, Attribute_Builtin_MissingLeftParen) {
    auto p = parser("builtin position)");
    auto attr = p->attribute();
    EXPECT_FALSE(attr.matched);
    EXPECT_TRUE(attr.errored);
    EXPECT_EQ(attr.value, nullptr);
    EXPECT_TRUE(p->has_error());
    EXPECT_EQ(p->error(), "1:9: expected '(' for builtin attribute");
}

TEST_F(ParserImplTest, Attribute_Builtin_MissingRightParen) {
    auto p = parser("builtin(position");
    auto attr = p->attribute();
    EXPECT_FALSE(attr.matched);
    EXPECT_TRUE(attr.errored);
    EXPECT_EQ(attr.value, nullptr);
    EXPECT_TRUE(p->has_error());
    EXPECT_EQ(p->error(), "1:17: expected ')' for builtin attribute");
}

TEST_F(ParserImplTest, Attribute_Builtin_MissingValue) {
    auto p = parser("builtin()");
    auto attr = p->attribute();
    EXPECT_FALSE(attr.matched);
    EXPECT_TRUE(attr.errored);
    EXPECT_EQ(attr.value, nullptr);
    EXPECT_TRUE(p->has_error());
    EXPECT_EQ(p->error(), "1:9: expected identifier for builtin");
}

TEST_F(ParserImplTest, Attribute_Builtin_InvalidValue) {
    auto p = parser("builtin(other_thingy)");
    auto attr = p->attribute();
    EXPECT_FALSE(attr.matched);
    EXPECT_TRUE(attr.errored);
    EXPECT_EQ(attr.value, nullptr);
    EXPECT_TRUE(p->has_error());
    EXPECT_EQ(p->error(), "1:9: invalid value for builtin attribute");
}

TEST_F(ParserImplTest, Attribute_Builtin_MissingInvalid) {
    auto p = parser("builtin(3)");
    auto attr = p->attribute();
    EXPECT_FALSE(attr.matched);
    EXPECT_TRUE(attr.errored);
    EXPECT_EQ(attr.value, nullptr);
    EXPECT_TRUE(p->has_error());
    EXPECT_EQ(p->error(), "1:9: expected identifier for builtin");
}

TEST_F(ParserImplTest, Attribute_Interpolate_Flat) {
    auto p = parser("interpolate(flat)");
    auto attr = p->attribute();
    EXPECT_TRUE(attr.matched);
    EXPECT_FALSE(attr.errored);
    ASSERT_NE(attr.value, nullptr);
    auto* var_attr = attr.value->As<ast::Attribute>();
    ASSERT_NE(var_attr, nullptr);
    ASSERT_FALSE(p->has_error());
    ASSERT_TRUE(var_attr->Is<ast::InterpolateAttribute>());

    auto* interp = var_attr->As<ast::InterpolateAttribute>();
    EXPECT_EQ(interp->type, ast::InterpolationType::kFlat);
    EXPECT_EQ(interp->sampling, ast::InterpolationSampling::kNone);
}

TEST_F(ParserImplTest, Attribute_Interpolate_Perspective_Center) {
    auto p = parser("interpolate(perspective, center)");
    auto attr = p->attribute();
    EXPECT_TRUE(attr.matched);
    EXPECT_FALSE(attr.errored);
    ASSERT_NE(attr.value, nullptr);
    auto* var_attr = attr.value->As<ast::Attribute>();
    ASSERT_NE(var_attr, nullptr);
    ASSERT_FALSE(p->has_error());
    ASSERT_TRUE(var_attr->Is<ast::InterpolateAttribute>());

    auto* interp = var_attr->As<ast::InterpolateAttribute>();
    EXPECT_EQ(interp->type, ast::InterpolationType::kPerspective);
    EXPECT_EQ(interp->sampling, ast::InterpolationSampling::kCenter);
}

TEST_F(ParserImplTest, Attribute_Interpolate_Perspective_Centroid) {
    auto p = parser("interpolate(perspective, centroid)");
    auto attr = p->attribute();
    EXPECT_TRUE(attr.matched);
    EXPECT_FALSE(attr.errored);
    ASSERT_NE(attr.value, nullptr);
    auto* var_attr = attr.value->As<ast::Attribute>();
    ASSERT_NE(var_attr, nullptr);
    ASSERT_FALSE(p->has_error());
    ASSERT_TRUE(var_attr->Is<ast::InterpolateAttribute>());

    auto* interp = var_attr->As<ast::InterpolateAttribute>();
    EXPECT_EQ(interp->type, ast::InterpolationType::kPerspective);
    EXPECT_EQ(interp->sampling, ast::InterpolationSampling::kCentroid);
}

TEST_F(ParserImplTest, Attribute_Interpolate_Linear_Sample) {
    auto p = parser("interpolate(linear, sample)");
    auto attr = p->attribute();
    EXPECT_TRUE(attr.matched);
    EXPECT_FALSE(attr.errored);
    ASSERT_NE(attr.value, nullptr);
    auto* var_attr = attr.value->As<ast::Attribute>();
    ASSERT_NE(var_attr, nullptr);
    ASSERT_FALSE(p->has_error());
    ASSERT_TRUE(var_attr->Is<ast::InterpolateAttribute>());

    auto* interp = var_attr->As<ast::InterpolateAttribute>();
    EXPECT_EQ(interp->type, ast::InterpolationType::kLinear);
    EXPECT_EQ(interp->sampling, ast::InterpolationSampling::kSample);
}

TEST_F(ParserImplTest, Attribute_Interpolate_MissingLeftParen) {
    auto p = parser("interpolate flat)");
    auto attr = p->attribute();
    EXPECT_FALSE(attr.matched);
    EXPECT_TRUE(attr.errored);
    EXPECT_EQ(attr.value, nullptr);
    EXPECT_TRUE(p->has_error());
    EXPECT_EQ(p->error(), "1:13: expected '(' for interpolate attribute");
}

TEST_F(ParserImplTest, Attribute_Interpolate_MissingRightParen) {
    auto p = parser("interpolate(flat");
    auto attr = p->attribute();
    EXPECT_FALSE(attr.matched);
    EXPECT_TRUE(attr.errored);
    EXPECT_EQ(attr.value, nullptr);
    EXPECT_TRUE(p->has_error());
    EXPECT_EQ(p->error(), "1:17: expected ')' for interpolate attribute");
}

TEST_F(ParserImplTest, Attribute_Interpolate_MissingFirstValue) {
    auto p = parser("interpolate()");
    auto attr = p->attribute();
    EXPECT_FALSE(attr.matched);
    EXPECT_TRUE(attr.errored);
    EXPECT_EQ(attr.value, nullptr);
    EXPECT_TRUE(p->has_error());
    EXPECT_EQ(p->error(), "1:13: invalid interpolation type");
}

TEST_F(ParserImplTest, Attribute_Interpolate_InvalidFirstValue) {
    auto p = parser("interpolate(other_thingy)");
    auto attr = p->attribute();
    EXPECT_FALSE(attr.matched);
    EXPECT_TRUE(attr.errored);
    EXPECT_EQ(attr.value, nullptr);
    EXPECT_TRUE(p->has_error());
    EXPECT_EQ(p->error(), "1:13: invalid interpolation type");
}

TEST_F(ParserImplTest, Attribute_Interpolate_MissingSecondValue) {
    auto p = parser("interpolate(perspective,)");
    auto attr = p->attribute();
    EXPECT_FALSE(attr.matched);
    EXPECT_TRUE(attr.errored);
    EXPECT_EQ(attr.value, nullptr);
    EXPECT_TRUE(p->has_error());
    EXPECT_EQ(p->error(), "1:25: invalid interpolation sampling");
}

TEST_F(ParserImplTest, Attribute_Interpolate_InvalidSecondValue) {
    auto p = parser("interpolate(perspective, nope)");
    auto attr = p->attribute();
    EXPECT_FALSE(attr.matched);
    EXPECT_TRUE(attr.errored);
    EXPECT_EQ(attr.value, nullptr);
    EXPECT_TRUE(p->has_error());
    EXPECT_EQ(p->error(), "1:26: invalid interpolation sampling");
}

TEST_F(ParserImplTest, Attribute_Binding) {
    auto p = parser("binding(4)");
    auto attr = p->attribute();
    EXPECT_TRUE(attr.matched);
    EXPECT_FALSE(attr.errored);
    ASSERT_NE(attr.value, nullptr);
    auto* var_attr = attr.value->As<ast::Attribute>();
    ASSERT_NE(var_attr, nullptr);
    ASSERT_FALSE(p->has_error());
    ASSERT_TRUE(var_attr->Is<ast::BindingAttribute>());

    auto* binding = var_attr->As<ast::BindingAttribute>();
    EXPECT_EQ(binding->value, 4u);
}

TEST_F(ParserImplTest, Attribute_Binding_MissingLeftParen) {
    auto p = parser("binding 4)");
    auto attr = p->attribute();
    EXPECT_FALSE(attr.matched);
    EXPECT_TRUE(attr.errored);
    EXPECT_EQ(attr.value, nullptr);
    EXPECT_TRUE(p->has_error());
    EXPECT_EQ(p->error(), "1:9: expected '(' for binding attribute");
}

TEST_F(ParserImplTest, Attribute_Binding_MissingRightParen) {
    auto p = parser("binding(4");
    auto attr = p->attribute();
    EXPECT_FALSE(attr.matched);
    EXPECT_TRUE(attr.errored);
    EXPECT_EQ(attr.value, nullptr);
    EXPECT_TRUE(p->has_error());
    EXPECT_EQ(p->error(), "1:10: expected ')' for binding attribute");
}

TEST_F(ParserImplTest, Attribute_Binding_MissingValue) {
    auto p = parser("binding()");
    auto attr = p->attribute();
    EXPECT_FALSE(attr.matched);
    EXPECT_TRUE(attr.errored);
    EXPECT_EQ(attr.value, nullptr);
    EXPECT_TRUE(p->has_error());
    EXPECT_EQ(p->error(), "1:9: expected signed integer literal for binding attribute");
}

TEST_F(ParserImplTest, Attribute_Binding_MissingInvalid) {
    auto p = parser("binding(nan)");
    auto attr = p->attribute();
    EXPECT_FALSE(attr.matched);
    EXPECT_TRUE(attr.errored);
    EXPECT_EQ(attr.value, nullptr);
    EXPECT_TRUE(p->has_error());
    EXPECT_EQ(p->error(), "1:9: expected signed integer literal for binding attribute");
}

TEST_F(ParserImplTest, Attribute_group) {
    auto p = parser("group(4)");
    auto attr = p->attribute();
    EXPECT_TRUE(attr.matched);
    EXPECT_FALSE(attr.errored);
    ASSERT_NE(attr.value, nullptr);
    auto* var_attr = attr.value->As<ast::Attribute>();
    ASSERT_FALSE(p->has_error());
    ASSERT_NE(var_attr, nullptr);
    ASSERT_TRUE(var_attr->Is<ast::GroupAttribute>());

    auto* group = var_attr->As<ast::GroupAttribute>();
    EXPECT_EQ(group->value, 4u);
}

TEST_F(ParserImplTest, Attribute_Group_MissingLeftParen) {
    auto p = parser("group 2)");
    auto attr = p->attribute();
    EXPECT_FALSE(attr.matched);
    EXPECT_TRUE(attr.errored);
    EXPECT_EQ(attr.value, nullptr);
    EXPECT_TRUE(p->has_error());
    EXPECT_EQ(p->error(), "1:7: expected '(' for group attribute");
}

TEST_F(ParserImplTest, Attribute_Group_MissingRightParen) {
    auto p = parser("group(2");
    auto attr = p->attribute();
    EXPECT_FALSE(attr.matched);
    EXPECT_TRUE(attr.errored);
    EXPECT_EQ(attr.value, nullptr);
    EXPECT_TRUE(p->has_error());
    EXPECT_EQ(p->error(), "1:8: expected ')' for group attribute");
}

TEST_F(ParserImplTest, Attribute_Group_MissingValue) {
    auto p = parser("group()");
    auto attr = p->attribute();
    EXPECT_FALSE(attr.matched);
    EXPECT_TRUE(attr.errored);
    EXPECT_EQ(attr.value, nullptr);
    EXPECT_TRUE(p->has_error());
    EXPECT_EQ(p->error(), "1:7: expected signed integer literal for group attribute");
}

TEST_F(ParserImplTest, Attribute_Group_MissingInvalid) {
    auto p = parser("group(nan)");
    auto attr = p->attribute();
    EXPECT_FALSE(attr.matched);
    EXPECT_TRUE(attr.errored);
    EXPECT_EQ(attr.value, nullptr);
    EXPECT_TRUE(p->has_error());
    EXPECT_EQ(p->error(), "1:7: expected signed integer literal for group attribute");
}

}  // namespace
}  // namespace tint::reader::wgsl
