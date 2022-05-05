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
#include "src/tint/ast/call_statement.h"
#include "src/tint/ast/disable_validation_attribute.h"
#include "src/tint/ast/id_attribute.h"
#include "src/tint/ast/stage_attribute.h"
#include "src/tint/ast/workgroup_attribute.h"
#include "src/tint/inspector/test_inspector_builder.h"
#include "src/tint/inspector/test_inspector_runner.h"
#include "src/tint/program_builder.h"
#include "src/tint/sem/depth_texture.h"
#include "src/tint/sem/external_texture.h"
#include "src/tint/sem/multisampled_texture.h"
#include "src/tint/sem/sampled_texture.h"
#include "src/tint/sem/variable.h"
#include "tint/tint.h"

using namespace tint::number_suffixes;  // NOLINT

namespace tint::inspector {
namespace {

// All the tests that descend from InspectorBuilder are expected to define their
// test state via building up the AST through InspectorBuilder and then generate
// the program with ::Build.
// The returned Inspector from ::Build can then be used to test expecations.
//
// All the tests that descend from InspectorRunner are expected to define their
// test state via a WGSL shader, which will be parsed to generate a Program and
// Inspector in ::Initialize.
// The returned Inspector from ::Initialize can then be used to test
// expecations.

class InspectorGetEntryPointTest : public InspectorBuilder, public testing::Test {};

typedef std::tuple<inspector::ComponentType, inspector::CompositionType>
    InspectorGetEntryPointComponentAndCompositionTestParams;
class InspectorGetEntryPointComponentAndCompositionTest
    : public InspectorBuilder,
      public testing::TestWithParam<InspectorGetEntryPointComponentAndCompositionTestParams> {};
struct InspectorGetEntryPointInterpolateTestParams {
    ast::InterpolationType in_type;
    ast::InterpolationSampling in_sampling;
    inspector::InterpolationType out_type;
    inspector::InterpolationSampling out_sampling;
};
class InspectorGetEntryPointInterpolateTest
    : public InspectorBuilder,
      public testing::TestWithParam<InspectorGetEntryPointInterpolateTestParams> {};
class InspectorGetConstantIDsTest : public InspectorBuilder, public testing::Test {};
class InspectorGetConstantNameToIdMapTest : public InspectorBuilder, public testing::Test {};
class InspectorGetStorageSizeTest : public InspectorBuilder, public testing::Test {};
class InspectorGetResourceBindingsTest : public InspectorBuilder, public testing::Test {};
class InspectorGetUniformBufferResourceBindingsTest : public InspectorBuilder,
                                                      public testing::Test {};
class InspectorGetStorageBufferResourceBindingsTest : public InspectorBuilder,
                                                      public testing::Test {};
class InspectorGetReadOnlyStorageBufferResourceBindingsTest : public InspectorBuilder,
                                                              public testing::Test {};
class InspectorGetSamplerResourceBindingsTest : public InspectorBuilder, public testing::Test {};
class InspectorGetComparisonSamplerResourceBindingsTest : public InspectorBuilder,
                                                          public testing::Test {};
class InspectorGetSampledTextureResourceBindingsTest : public InspectorBuilder,
                                                       public testing::Test {};
class InspectorGetSampledArrayTextureResourceBindingsTest : public InspectorBuilder,
                                                            public testing::Test {};
struct GetSampledTextureTestParams {
    ast::TextureDimension type_dim;
    inspector::ResourceBinding::TextureDimension inspector_dim;
    inspector::ResourceBinding::SampledKind sampled_kind;
};
class InspectorGetSampledTextureResourceBindingsTestWithParam
    : public InspectorBuilder,
      public testing::TestWithParam<GetSampledTextureTestParams> {};
class InspectorGetSampledArrayTextureResourceBindingsTestWithParam
    : public InspectorBuilder,
      public testing::TestWithParam<GetSampledTextureTestParams> {};
class InspectorGetMultisampledTextureResourceBindingsTest : public InspectorBuilder,
                                                            public testing::Test {};
class InspectorGetMultisampledArrayTextureResourceBindingsTest : public InspectorBuilder,
                                                                 public testing::Test {};
typedef GetSampledTextureTestParams GetMultisampledTextureTestParams;
class InspectorGetMultisampledArrayTextureResourceBindingsTestWithParam
    : public InspectorBuilder,
      public testing::TestWithParam<GetMultisampledTextureTestParams> {};
class InspectorGetMultisampledTextureResourceBindingsTestWithParam
    : public InspectorBuilder,
      public testing::TestWithParam<GetMultisampledTextureTestParams> {};
class InspectorGetStorageTextureResourceBindingsTest : public InspectorBuilder,
                                                       public testing::Test {};
struct GetDepthTextureTestParams {
    ast::TextureDimension type_dim;
    inspector::ResourceBinding::TextureDimension inspector_dim;
};
class InspectorGetDepthTextureResourceBindingsTestWithParam
    : public InspectorBuilder,
      public testing::TestWithParam<GetDepthTextureTestParams> {};

class InspectorGetDepthMultisampledTextureResourceBindingsTest : public InspectorBuilder,
                                                                 public testing::Test {};

typedef std::tuple<ast::TextureDimension, ResourceBinding::TextureDimension> DimensionParams;
typedef std::tuple<ast::TexelFormat, ResourceBinding::TexelFormat, ResourceBinding::SampledKind>
    TexelFormatParams;
typedef std::tuple<DimensionParams, TexelFormatParams> GetStorageTextureTestParams;
class InspectorGetStorageTextureResourceBindingsTestWithParam
    : public InspectorBuilder,
      public testing::TestWithParam<GetStorageTextureTestParams> {};

class InspectorGetExternalTextureResourceBindingsTest : public InspectorBuilder,
                                                        public testing::Test {};

class InspectorGetSamplerTextureUsesTest : public InspectorRunner, public testing::Test {};

class InspectorGetWorkgroupStorageSizeTest : public InspectorBuilder, public testing::Test {};

class InspectorGetUsedExtensionNamesTest : public InspectorRunner, public testing::Test {};

class InspectorGetEnableDirectivesTest : public InspectorRunner, public testing::Test {};

// This is a catch all for shaders that have demonstrated regressions/crashes in
// the wild.
class InspectorRegressionTest : public InspectorRunner, public testing::Test {};

TEST_F(InspectorGetEntryPointTest, NoFunctions) {
    Inspector& inspector = Build();

    auto result = inspector.GetEntryPoints();
    ASSERT_FALSE(inspector.has_error()) << inspector.error();

    EXPECT_EQ(0u, result.size());
}

TEST_F(InspectorGetEntryPointTest, NoEntryPoints) {
    Inspector& inspector = Build();

    auto result = inspector.GetEntryPoints();
    ASSERT_FALSE(inspector.has_error()) << inspector.error();

    EXPECT_EQ(0u, result.size());
}

TEST_F(InspectorGetEntryPointTest, OneEntryPoint) {
    MakeEmptyBodyFunction("foo", ast::AttributeList{
                                     Stage(ast::PipelineStage::kFragment),
                                 });

    // TODO(dsinclair): Update to run the namer transform when available.

    Inspector& inspector = Build();

    auto result = inspector.GetEntryPoints();
    ASSERT_FALSE(inspector.has_error()) << inspector.error();

    ASSERT_EQ(1u, result.size());
    EXPECT_EQ("foo", result[0].name);
    EXPECT_EQ("foo", result[0].remapped_name);
    EXPECT_EQ(ast::PipelineStage::kFragment, result[0].stage);
}

TEST_F(InspectorGetEntryPointTest, MultipleEntryPoints) {
    MakeEmptyBodyFunction("foo", ast::AttributeList{
                                     Stage(ast::PipelineStage::kFragment),
                                 });

    MakeEmptyBodyFunction(
        "bar", ast::AttributeList{Stage(ast::PipelineStage::kCompute), WorkgroupSize(1_i)});

    // TODO(dsinclair): Update to run the namer transform when available.

    Inspector& inspector = Build();

    auto result = inspector.GetEntryPoints();
    ASSERT_FALSE(inspector.has_error()) << inspector.error();

    ASSERT_EQ(2u, result.size());
    EXPECT_EQ("foo", result[0].name);
    EXPECT_EQ("foo", result[0].remapped_name);
    EXPECT_EQ(ast::PipelineStage::kFragment, result[0].stage);
    EXPECT_EQ("bar", result[1].name);
    EXPECT_EQ("bar", result[1].remapped_name);
    EXPECT_EQ(ast::PipelineStage::kCompute, result[1].stage);
}

TEST_F(InspectorGetEntryPointTest, MixFunctionsAndEntryPoints) {
    MakeEmptyBodyFunction("func", {});

    MakeCallerBodyFunction(
        "foo", {"func"},
        ast::AttributeList{Stage(ast::PipelineStage::kCompute), WorkgroupSize(1_i)});

    MakeCallerBodyFunction("bar", {"func"},
                           ast::AttributeList{
                               Stage(ast::PipelineStage::kFragment),
                           });

    // TODO(dsinclair): Update to run the namer transform when available.

    Inspector& inspector = Build();

    auto result = inspector.GetEntryPoints();
    EXPECT_FALSE(inspector.has_error());

    ASSERT_EQ(2u, result.size());
    EXPECT_EQ("foo", result[0].name);
    EXPECT_EQ("foo", result[0].remapped_name);
    EXPECT_EQ(ast::PipelineStage::kCompute, result[0].stage);
    EXPECT_EQ("bar", result[1].name);
    EXPECT_EQ("bar", result[1].remapped_name);
    EXPECT_EQ(ast::PipelineStage::kFragment, result[1].stage);
}

TEST_F(InspectorGetEntryPointTest, DefaultWorkgroupSize) {
    MakeEmptyBodyFunction("foo", ast::AttributeList{Stage(ast::PipelineStage::kCompute),
                                                    WorkgroupSize(8_i, 2_i, 1_i)});

    Inspector& inspector = Build();

    auto result = inspector.GetEntryPoints();
    ASSERT_FALSE(inspector.has_error()) << inspector.error();

    ASSERT_EQ(1u, result.size());
    uint32_t x, y, z;
    std::tie(x, y, z) = result[0].workgroup_size();
    EXPECT_EQ(8u, x);
    EXPECT_EQ(2u, y);
    EXPECT_EQ(1u, z);
}

TEST_F(InspectorGetEntryPointTest, NonDefaultWorkgroupSize) {
    MakeEmptyBodyFunction("foo",
                          {Stage(ast::PipelineStage::kCompute), WorkgroupSize(8_i, 2_i, 1_i)});

    Inspector& inspector = Build();

    auto result = inspector.GetEntryPoints();
    ASSERT_FALSE(inspector.has_error()) << inspector.error();

    ASSERT_EQ(1u, result.size());
    uint32_t x, y, z;
    std::tie(x, y, z) = result[0].workgroup_size();
    EXPECT_EQ(8u, x);
    EXPECT_EQ(2u, y);
    EXPECT_EQ(1u, z);
}

TEST_F(InspectorGetEntryPointTest, NoInOutVariables) {
    MakeEmptyBodyFunction("func", {});

    MakeCallerBodyFunction("foo", {"func"},
                           ast::AttributeList{
                               Stage(ast::PipelineStage::kFragment),
                           });

    Inspector& inspector = Build();

    auto result = inspector.GetEntryPoints();
    ASSERT_FALSE(inspector.has_error()) << inspector.error();

    ASSERT_EQ(1u, result.size());
    EXPECT_EQ(0u, result[0].input_variables.size());
    EXPECT_EQ(0u, result[0].output_variables.size());
}

TEST_P(InspectorGetEntryPointComponentAndCompositionTest, Test) {
    ComponentType component;
    CompositionType composition;
    std::tie(component, composition) = GetParam();
    std::function<const ast::Type*()> tint_type = GetTypeFunction(component, composition);

    auto* in_var = Param("in_var", tint_type(), {Location(0u), Flat()});
    Func("foo", {in_var}, tint_type(), {Return("in_var")}, {Stage(ast::PipelineStage::kFragment)},
         {Location(0u)});
    Inspector& inspector = Build();

    auto result = inspector.GetEntryPoints();
    ASSERT_FALSE(inspector.has_error()) << inspector.error();

    ASSERT_EQ(1u, result.size());

    ASSERT_EQ(1u, result[0].input_variables.size());
    EXPECT_EQ("in_var", result[0].input_variables[0].name);
    EXPECT_TRUE(result[0].input_variables[0].has_location_attribute);
    EXPECT_EQ(0u, result[0].input_variables[0].location_attribute);
    EXPECT_EQ(component, result[0].input_variables[0].component_type);

    ASSERT_EQ(1u, result[0].output_variables.size());
    EXPECT_EQ("<retval>", result[0].output_variables[0].name);
    EXPECT_TRUE(result[0].output_variables[0].has_location_attribute);
    EXPECT_EQ(0u, result[0].output_variables[0].location_attribute);
    EXPECT_EQ(component, result[0].output_variables[0].component_type);
}
INSTANTIATE_TEST_SUITE_P(InspectorGetEntryPointTest,
                         InspectorGetEntryPointComponentAndCompositionTest,
                         testing::Combine(testing::Values(ComponentType::kFloat,
                                                          ComponentType::kSInt,
                                                          ComponentType::kUInt),
                                          testing::Values(CompositionType::kScalar,
                                                          CompositionType::kVec2,
                                                          CompositionType::kVec3,
                                                          CompositionType::kVec4)));

TEST_F(InspectorGetEntryPointTest, MultipleInOutVariables) {
    auto* in_var0 = Param("in_var0", ty.u32(), {Location(0u), Flat()});
    auto* in_var1 = Param("in_var1", ty.u32(), {Location(1u), Flat()});
    auto* in_var4 = Param("in_var4", ty.u32(), {Location(4u), Flat()});
    Func("foo", {in_var0, in_var1, in_var4}, ty.u32(), {Return("in_var0")},
         {Stage(ast::PipelineStage::kFragment)}, {Location(0u)});
    Inspector& inspector = Build();

    auto result = inspector.GetEntryPoints();
    ASSERT_FALSE(inspector.has_error()) << inspector.error();

    ASSERT_EQ(1u, result.size());

    ASSERT_EQ(3u, result[0].input_variables.size());
    EXPECT_EQ("in_var0", result[0].input_variables[0].name);
    EXPECT_TRUE(result[0].input_variables[0].has_location_attribute);
    EXPECT_EQ(0u, result[0].input_variables[0].location_attribute);
    EXPECT_EQ(InterpolationType::kFlat, result[0].input_variables[0].interpolation_type);
    EXPECT_EQ(ComponentType::kUInt, result[0].input_variables[0].component_type);
    EXPECT_EQ("in_var1", result[0].input_variables[1].name);
    EXPECT_TRUE(result[0].input_variables[1].has_location_attribute);
    EXPECT_EQ(1u, result[0].input_variables[1].location_attribute);
    EXPECT_EQ(InterpolationType::kFlat, result[0].input_variables[1].interpolation_type);
    EXPECT_EQ(ComponentType::kUInt, result[0].input_variables[1].component_type);
    EXPECT_EQ("in_var4", result[0].input_variables[2].name);
    EXPECT_TRUE(result[0].input_variables[2].has_location_attribute);
    EXPECT_EQ(4u, result[0].input_variables[2].location_attribute);
    EXPECT_EQ(InterpolationType::kFlat, result[0].input_variables[2].interpolation_type);
    EXPECT_EQ(ComponentType::kUInt, result[0].input_variables[2].component_type);

    ASSERT_EQ(1u, result[0].output_variables.size());
    EXPECT_EQ("<retval>", result[0].output_variables[0].name);
    EXPECT_TRUE(result[0].output_variables[0].has_location_attribute);
    EXPECT_EQ(0u, result[0].output_variables[0].location_attribute);
    EXPECT_EQ(ComponentType::kUInt, result[0].output_variables[0].component_type);
}

TEST_F(InspectorGetEntryPointTest, MultipleEntryPointsInOutVariables) {
    auto* in_var_foo = Param("in_var_foo", ty.u32(), {Location(0u), Flat()});
    Func("foo", {in_var_foo}, ty.u32(), {Return("in_var_foo")},
         {Stage(ast::PipelineStage::kFragment)}, {Location(0u)});

    auto* in_var_bar = Param("in_var_bar", ty.u32(), {Location(0u), Flat()});
    Func("bar", {in_var_bar}, ty.u32(), {Return("in_var_bar")},
         {Stage(ast::PipelineStage::kFragment)}, {Location(1u)});

    Inspector& inspector = Build();

    auto result = inspector.GetEntryPoints();
    ASSERT_FALSE(inspector.has_error()) << inspector.error();

    ASSERT_EQ(2u, result.size());

    ASSERT_EQ(1u, result[0].input_variables.size());
    EXPECT_EQ("in_var_foo", result[0].input_variables[0].name);
    EXPECT_TRUE(result[0].input_variables[0].has_location_attribute);
    EXPECT_EQ(0u, result[0].input_variables[0].location_attribute);
    EXPECT_EQ(InterpolationType::kFlat, result[0].input_variables[0].interpolation_type);
    EXPECT_EQ(ComponentType::kUInt, result[0].input_variables[0].component_type);

    ASSERT_EQ(1u, result[0].output_variables.size());
    EXPECT_EQ("<retval>", result[0].output_variables[0].name);
    EXPECT_TRUE(result[0].output_variables[0].has_location_attribute);
    EXPECT_EQ(0u, result[0].output_variables[0].location_attribute);
    EXPECT_EQ(ComponentType::kUInt, result[0].output_variables[0].component_type);

    ASSERT_EQ(1u, result[1].input_variables.size());
    EXPECT_EQ("in_var_bar", result[1].input_variables[0].name);
    EXPECT_TRUE(result[1].input_variables[0].has_location_attribute);
    EXPECT_EQ(0u, result[1].input_variables[0].location_attribute);
    EXPECT_EQ(InterpolationType::kFlat, result[1].input_variables[0].interpolation_type);
    EXPECT_EQ(ComponentType::kUInt, result[1].input_variables[0].component_type);

    ASSERT_EQ(1u, result[1].output_variables.size());
    EXPECT_EQ("<retval>", result[1].output_variables[0].name);
    EXPECT_TRUE(result[1].output_variables[0].has_location_attribute);
    EXPECT_EQ(1u, result[1].output_variables[0].location_attribute);
    EXPECT_EQ(ComponentType::kUInt, result[1].output_variables[0].component_type);
}

TEST_F(InspectorGetEntryPointTest, BuiltInsNotStageVariables) {
    auto* in_var0 = Param("in_var0", ty.u32(), {Builtin(ast::Builtin::kSampleIndex)});
    auto* in_var1 = Param("in_var1", ty.f32(), {Location(0u)});
    Func("foo", {in_var0, in_var1}, ty.f32(), {Return("in_var1")},
         {Stage(ast::PipelineStage::kFragment)}, {Builtin(ast::Builtin::kFragDepth)});
    Inspector& inspector = Build();

    auto result = inspector.GetEntryPoints();
    ASSERT_FALSE(inspector.has_error()) << inspector.error();

    ASSERT_EQ(1u, result.size());

    ASSERT_EQ(1u, result[0].input_variables.size());
    EXPECT_EQ("in_var1", result[0].input_variables[0].name);
    EXPECT_TRUE(result[0].input_variables[0].has_location_attribute);
    EXPECT_EQ(0u, result[0].input_variables[0].location_attribute);
    EXPECT_EQ(ComponentType::kFloat, result[0].input_variables[0].component_type);

    ASSERT_EQ(0u, result[0].output_variables.size());
}

TEST_F(InspectorGetEntryPointTest, InOutStruct) {
    auto* interface = MakeInOutStruct("interface", {{"a", 0u}, {"b", 1u}});
    Func("foo", {Param("param", ty.Of(interface))}, ty.Of(interface), {Return("param")},
         {Stage(ast::PipelineStage::kFragment)});
    Inspector& inspector = Build();

    auto result = inspector.GetEntryPoints();
    ASSERT_FALSE(inspector.has_error()) << inspector.error();

    ASSERT_EQ(1u, result.size());

    ASSERT_EQ(2u, result[0].input_variables.size());
    EXPECT_EQ("param.a", result[0].input_variables[0].name);
    EXPECT_TRUE(result[0].input_variables[0].has_location_attribute);
    EXPECT_EQ(0u, result[0].input_variables[0].location_attribute);
    EXPECT_EQ(ComponentType::kUInt, result[0].input_variables[0].component_type);
    EXPECT_EQ("param.b", result[0].input_variables[1].name);
    EXPECT_TRUE(result[0].input_variables[1].has_location_attribute);
    EXPECT_EQ(1u, result[0].input_variables[1].location_attribute);
    EXPECT_EQ(ComponentType::kUInt, result[0].input_variables[1].component_type);

    ASSERT_EQ(2u, result[0].output_variables.size());
    EXPECT_EQ("<retval>.a", result[0].output_variables[0].name);
    EXPECT_TRUE(result[0].output_variables[0].has_location_attribute);
    EXPECT_EQ(0u, result[0].output_variables[0].location_attribute);
    EXPECT_EQ(ComponentType::kUInt, result[0].output_variables[0].component_type);
    EXPECT_EQ("<retval>.b", result[0].output_variables[1].name);
    EXPECT_TRUE(result[0].output_variables[1].has_location_attribute);
    EXPECT_EQ(1u, result[0].output_variables[1].location_attribute);
    EXPECT_EQ(ComponentType::kUInt, result[0].output_variables[1].component_type);
}

TEST_F(InspectorGetEntryPointTest, MultipleEntryPointsInOutSharedStruct) {
    auto* interface = MakeInOutStruct("interface", {{"a", 0u}, {"b", 1u}});
    Func("foo", {}, ty.Of(interface), {Return(Construct(ty.Of(interface)))},
         {Stage(ast::PipelineStage::kFragment)});
    Func("bar", {Param("param", ty.Of(interface))}, ty.void_(), {},
         {Stage(ast::PipelineStage::kFragment)});
    Inspector& inspector = Build();

    auto result = inspector.GetEntryPoints();
    ASSERT_FALSE(inspector.has_error()) << inspector.error();

    ASSERT_EQ(2u, result.size());

    ASSERT_EQ(0u, result[0].input_variables.size());

    ASSERT_EQ(2u, result[0].output_variables.size());
    EXPECT_EQ("<retval>.a", result[0].output_variables[0].name);
    EXPECT_TRUE(result[0].output_variables[0].has_location_attribute);
    EXPECT_EQ(0u, result[0].output_variables[0].location_attribute);
    EXPECT_EQ(ComponentType::kUInt, result[0].output_variables[0].component_type);
    EXPECT_EQ("<retval>.b", result[0].output_variables[1].name);
    EXPECT_TRUE(result[0].output_variables[1].has_location_attribute);
    EXPECT_EQ(1u, result[0].output_variables[1].location_attribute);
    EXPECT_EQ(ComponentType::kUInt, result[0].output_variables[1].component_type);

    ASSERT_EQ(2u, result[1].input_variables.size());
    EXPECT_EQ("param.a", result[1].input_variables[0].name);
    EXPECT_TRUE(result[1].input_variables[0].has_location_attribute);
    EXPECT_EQ(0u, result[1].input_variables[0].location_attribute);
    EXPECT_EQ(ComponentType::kUInt, result[1].input_variables[0].component_type);
    EXPECT_EQ("param.b", result[1].input_variables[1].name);
    EXPECT_TRUE(result[1].input_variables[1].has_location_attribute);
    EXPECT_EQ(1u, result[1].input_variables[1].location_attribute);
    EXPECT_EQ(ComponentType::kUInt, result[1].input_variables[1].component_type);

    ASSERT_EQ(0u, result[1].output_variables.size());
}

TEST_F(InspectorGetEntryPointTest, MixInOutVariablesAndStruct) {
    auto* struct_a = MakeInOutStruct("struct_a", {{"a", 0u}, {"b", 1u}});
    auto* struct_b = MakeInOutStruct("struct_b", {{"a", 2u}});
    Func("foo",
         {Param("param_a", ty.Of(struct_a)), Param("param_b", ty.Of(struct_b)),
          Param("param_c", ty.f32(), {Location(3u)}), Param("param_d", ty.f32(), {Location(4u)})},
         ty.Of(struct_a), {Return("param_a")}, {Stage(ast::PipelineStage::kFragment)});
    Inspector& inspector = Build();

    auto result = inspector.GetEntryPoints();
    ASSERT_FALSE(inspector.has_error()) << inspector.error();

    ASSERT_EQ(1u, result.size());

    ASSERT_EQ(5u, result[0].input_variables.size());
    EXPECT_EQ("param_a.a", result[0].input_variables[0].name);
    EXPECT_TRUE(result[0].input_variables[0].has_location_attribute);
    EXPECT_EQ(0u, result[0].input_variables[0].location_attribute);
    EXPECT_EQ(ComponentType::kUInt, result[0].input_variables[0].component_type);
    EXPECT_EQ("param_a.b", result[0].input_variables[1].name);
    EXPECT_TRUE(result[0].input_variables[1].has_location_attribute);
    EXPECT_EQ(1u, result[0].input_variables[1].location_attribute);
    EXPECT_EQ(ComponentType::kUInt, result[0].input_variables[1].component_type);
    EXPECT_EQ("param_b.a", result[0].input_variables[2].name);
    EXPECT_TRUE(result[0].input_variables[2].has_location_attribute);
    EXPECT_EQ(2u, result[0].input_variables[2].location_attribute);
    EXPECT_EQ(ComponentType::kUInt, result[0].input_variables[2].component_type);
    EXPECT_EQ("param_c", result[0].input_variables[3].name);
    EXPECT_TRUE(result[0].input_variables[3].has_location_attribute);
    EXPECT_EQ(3u, result[0].input_variables[3].location_attribute);
    EXPECT_EQ(ComponentType::kFloat, result[0].input_variables[3].component_type);
    EXPECT_EQ("param_d", result[0].input_variables[4].name);
    EXPECT_TRUE(result[0].input_variables[4].has_location_attribute);
    EXPECT_EQ(4u, result[0].input_variables[4].location_attribute);
    EXPECT_EQ(ComponentType::kFloat, result[0].input_variables[4].component_type);

    ASSERT_EQ(2u, result[0].output_variables.size());
    EXPECT_EQ("<retval>.a", result[0].output_variables[0].name);
    EXPECT_TRUE(result[0].output_variables[0].has_location_attribute);
    EXPECT_EQ(0u, result[0].output_variables[0].location_attribute);
    EXPECT_EQ(ComponentType::kUInt, result[0].output_variables[0].component_type);
    EXPECT_EQ("<retval>.b", result[0].output_variables[1].name);
    EXPECT_TRUE(result[0].output_variables[1].has_location_attribute);
    EXPECT_EQ(1u, result[0].output_variables[1].location_attribute);
    EXPECT_EQ(ComponentType::kUInt, result[0].output_variables[1].component_type);
}

TEST_F(InspectorGetEntryPointTest, OverridableConstantUnreferenced) {
    AddOverridableConstantWithoutID("foo", ty.f32(), nullptr);
    MakeEmptyBodyFunction("ep_func", {Stage(ast::PipelineStage::kCompute), WorkgroupSize(1_i)});

    Inspector& inspector = Build();

    auto result = inspector.GetEntryPoints();

    ASSERT_EQ(1u, result.size());
    EXPECT_EQ(0u, result[0].overridable_constants.size());
}

TEST_F(InspectorGetEntryPointTest, OverridableConstantReferencedByEntryPoint) {
    AddOverridableConstantWithoutID("foo", ty.f32(), nullptr);
    MakePlainGlobalReferenceBodyFunction("ep_func", "foo", ty.f32(),
                                         {Stage(ast::PipelineStage::kCompute), WorkgroupSize(1_i)});

    Inspector& inspector = Build();

    auto result = inspector.GetEntryPoints();

    ASSERT_EQ(1u, result.size());
    ASSERT_EQ(1u, result[0].overridable_constants.size());
    EXPECT_EQ("foo", result[0].overridable_constants[0].name);
}

TEST_F(InspectorGetEntryPointTest, OverridableConstantReferencedByCallee) {
    AddOverridableConstantWithoutID("foo", ty.f32(), nullptr);
    MakePlainGlobalReferenceBodyFunction("callee_func", "foo", ty.f32(), {});
    MakeCallerBodyFunction("ep_func", {"callee_func"},
                           {Stage(ast::PipelineStage::kCompute), WorkgroupSize(1_i)});

    Inspector& inspector = Build();

    auto result = inspector.GetEntryPoints();

    ASSERT_EQ(1u, result.size());
    ASSERT_EQ(1u, result[0].overridable_constants.size());
    EXPECT_EQ("foo", result[0].overridable_constants[0].name);
}

TEST_F(InspectorGetEntryPointTest, OverridableConstantSomeReferenced) {
    AddOverridableConstantWithID("foo", 1, ty.f32(), nullptr);
    AddOverridableConstantWithID("bar", 2, ty.f32(), nullptr);
    MakePlainGlobalReferenceBodyFunction("callee_func", "foo", ty.f32(), {});
    MakeCallerBodyFunction("ep_func", {"callee_func"},
                           {Stage(ast::PipelineStage::kCompute), WorkgroupSize(1_i)});

    Inspector& inspector = Build();

    auto result = inspector.GetEntryPoints();

    ASSERT_EQ(1u, result.size());
    ASSERT_EQ(1u, result[0].overridable_constants.size());
    EXPECT_EQ("foo", result[0].overridable_constants[0].name);
    EXPECT_EQ(1, result[0].overridable_constants[0].numeric_id);
}

TEST_F(InspectorGetEntryPointTest, OverridableConstantTypes) {
    AddOverridableConstantWithoutID("bool_var", ty.bool_(), nullptr);
    AddOverridableConstantWithoutID("float_var", ty.f32(), nullptr);
    AddOverridableConstantWithoutID("u32_var", ty.u32(), nullptr);
    AddOverridableConstantWithoutID("i32_var", ty.i32(), nullptr);

    MakePlainGlobalReferenceBodyFunction("bool_func", "bool_var", ty.bool_(), {});
    MakePlainGlobalReferenceBodyFunction("float_func", "float_var", ty.f32(), {});
    MakePlainGlobalReferenceBodyFunction("u32_func", "u32_var", ty.u32(), {});
    MakePlainGlobalReferenceBodyFunction("i32_func", "i32_var", ty.i32(), {});

    MakeCallerBodyFunction("ep_func", {"bool_func", "float_func", "u32_func", "i32_func"},
                           {Stage(ast::PipelineStage::kCompute), WorkgroupSize(1_i)});

    Inspector& inspector = Build();

    auto result = inspector.GetEntryPoints();

    ASSERT_EQ(1u, result.size());
    ASSERT_EQ(4u, result[0].overridable_constants.size());
    EXPECT_EQ("bool_var", result[0].overridable_constants[0].name);
    EXPECT_EQ(inspector::OverridableConstant::Type::kBool, result[0].overridable_constants[0].type);
    EXPECT_EQ("float_var", result[0].overridable_constants[1].name);
    EXPECT_EQ(inspector::OverridableConstant::Type::kFloat32,
              result[0].overridable_constants[1].type);
    EXPECT_EQ("u32_var", result[0].overridable_constants[2].name);
    EXPECT_EQ(inspector::OverridableConstant::Type::kUint32,
              result[0].overridable_constants[2].type);
    EXPECT_EQ("i32_var", result[0].overridable_constants[3].name);
    EXPECT_EQ(inspector::OverridableConstant::Type::kInt32,
              result[0].overridable_constants[3].type);
}

TEST_F(InspectorGetEntryPointTest, OverridableConstantInitialized) {
    AddOverridableConstantWithoutID("foo", ty.f32(), Expr(0.0f));
    MakePlainGlobalReferenceBodyFunction("ep_func", "foo", ty.f32(),
                                         {Stage(ast::PipelineStage::kCompute), WorkgroupSize(1_i)});

    Inspector& inspector = Build();

    auto result = inspector.GetEntryPoints();

    ASSERT_EQ(1u, result.size());
    ASSERT_EQ(1u, result[0].overridable_constants.size());
    EXPECT_EQ("foo", result[0].overridable_constants[0].name);
    EXPECT_TRUE(result[0].overridable_constants[0].is_initialized);
}

TEST_F(InspectorGetEntryPointTest, OverridableConstantUninitialized) {
    AddOverridableConstantWithoutID("foo", ty.f32(), nullptr);
    MakePlainGlobalReferenceBodyFunction("ep_func", "foo", ty.f32(),
                                         {Stage(ast::PipelineStage::kCompute), WorkgroupSize(1_i)});

    Inspector& inspector = Build();

    auto result = inspector.GetEntryPoints();

    ASSERT_EQ(1u, result.size());
    ASSERT_EQ(1u, result[0].overridable_constants.size());
    EXPECT_EQ("foo", result[0].overridable_constants[0].name);

    EXPECT_FALSE(result[0].overridable_constants[0].is_initialized);
}

TEST_F(InspectorGetEntryPointTest, OverridableConstantNumericIDSpecified) {
    AddOverridableConstantWithoutID("foo_no_id", ty.f32(), nullptr);
    AddOverridableConstantWithID("foo_id", 1234, ty.f32(), nullptr);

    MakePlainGlobalReferenceBodyFunction("no_id_func", "foo_no_id", ty.f32(), {});
    MakePlainGlobalReferenceBodyFunction("id_func", "foo_id", ty.f32(), {});

    MakeCallerBodyFunction("ep_func", {"no_id_func", "id_func"},
                           {Stage(ast::PipelineStage::kCompute), WorkgroupSize(1_i)});

    Inspector& inspector = Build();

    auto result = inspector.GetEntryPoints();

    ASSERT_EQ(1u, result.size());
    ASSERT_EQ(2u, result[0].overridable_constants.size());
    EXPECT_EQ("foo_no_id", result[0].overridable_constants[0].name);
    EXPECT_EQ("foo_id", result[0].overridable_constants[1].name);
    EXPECT_EQ(1234, result[0].overridable_constants[1].numeric_id);

    EXPECT_FALSE(result[0].overridable_constants[0].is_numeric_id_specified);
    EXPECT_TRUE(result[0].overridable_constants[1].is_numeric_id_specified);
}

TEST_F(InspectorGetEntryPointTest, NonOverridableConstantSkipped) {
    auto* foo_struct_type = MakeUniformBufferType("foo_type", {ty.i32()});
    AddUniformBuffer("foo_ub", ty.Of(foo_struct_type), 0, 0);
    MakeStructVariableReferenceBodyFunction("ub_func", "foo_ub", {{0, ty.i32()}});
    MakeCallerBodyFunction("ep_func", {"ub_func"}, {Stage(ast::PipelineStage::kFragment)});

    Inspector& inspector = Build();

    auto result = inspector.GetEntryPoints();

    ASSERT_EQ(1u, result.size());
    EXPECT_EQ(0u, result[0].overridable_constants.size());
}

TEST_F(InspectorGetEntryPointTest, BuiltinNotReferenced) {
    MakeEmptyBodyFunction("ep_func", {Stage(ast::PipelineStage::kFragment)});

    Inspector& inspector = Build();

    auto result = inspector.GetEntryPoints();

    ASSERT_EQ(1u, result.size());
    EXPECT_FALSE(result[0].input_sample_mask_used);
    EXPECT_FALSE(result[0].output_sample_mask_used);
    EXPECT_FALSE(result[0].input_position_used);
    EXPECT_FALSE(result[0].front_facing_used);
    EXPECT_FALSE(result[0].sample_index_used);
    EXPECT_FALSE(result[0].num_workgroups_used);
}

TEST_F(InspectorGetEntryPointTest, InputSampleMaskSimpleReferenced) {
    auto* in_var = Param("in_var", ty.u32(), {Builtin(ast::Builtin::kSampleMask)});
    Func("ep_func", {in_var}, ty.void_(), {Return()}, {Stage(ast::PipelineStage::kFragment)}, {});

    Inspector& inspector = Build();

    auto result = inspector.GetEntryPoints();

    ASSERT_EQ(1u, result.size());
    EXPECT_TRUE(result[0].input_sample_mask_used);
}

TEST_F(InspectorGetEntryPointTest, InputSampleMaskStructReferenced) {
    ast::StructMemberList members;
    members.push_back(Member("inner_position", ty.u32(), {Builtin(ast::Builtin::kSampleMask)}));
    Structure("in_struct", members);
    auto* in_var = Param("in_var", ty.type_name("in_struct"), {});

    Func("ep_func", {in_var}, ty.void_(), {Return()}, {Stage(ast::PipelineStage::kFragment)}, {});

    Inspector& inspector = Build();

    auto result = inspector.GetEntryPoints();

    ASSERT_EQ(1u, result.size());
    EXPECT_TRUE(result[0].input_sample_mask_used);
}

TEST_F(InspectorGetEntryPointTest, OutputSampleMaskSimpleReferenced) {
    auto* in_var = Param("in_var", ty.u32(), {Builtin(ast::Builtin::kSampleMask)});
    Func("ep_func", {in_var}, ty.u32(), {Return("in_var")}, {Stage(ast::PipelineStage::kFragment)},
         {Builtin(ast::Builtin::kSampleMask)});

    Inspector& inspector = Build();

    auto result = inspector.GetEntryPoints();

    ASSERT_EQ(1u, result.size());
    EXPECT_TRUE(result[0].output_sample_mask_used);
}

TEST_F(InspectorGetEntryPointTest, OutputSampleMaskStructReferenced) {
    ast::StructMemberList members;
    members.push_back(Member("inner_sample_mask", ty.u32(), {Builtin(ast::Builtin::kSampleMask)}));
    Structure("out_struct", members);

    Func("ep_func", {}, ty.type_name("out_struct"),
         {Decl(Var("out_var", ty.type_name("out_struct"))), Return("out_var")},
         {Stage(ast::PipelineStage::kFragment)}, {});

    Inspector& inspector = Build();

    auto result = inspector.GetEntryPoints();

    ASSERT_EQ(1u, result.size());
    EXPECT_TRUE(result[0].output_sample_mask_used);
}

TEST_F(InspectorGetEntryPointTest, InputPositionSimpleReferenced) {
    auto* in_var = Param("in_var", ty.vec4<f32>(), {Builtin(ast::Builtin::kPosition)});
    Func("ep_func", {in_var}, ty.void_(), {Return()}, {Stage(ast::PipelineStage::kFragment)}, {});

    Inspector& inspector = Build();

    auto result = inspector.GetEntryPoints();

    ASSERT_EQ(1u, result.size());
    EXPECT_TRUE(result[0].input_position_used);
}

TEST_F(InspectorGetEntryPointTest, InputPositionStructReferenced) {
    ast::StructMemberList members;
    members.push_back(Member("inner_position", ty.vec4<f32>(), {Builtin(ast::Builtin::kPosition)}));
    Structure("in_struct", members);
    auto* in_var = Param("in_var", ty.type_name("in_struct"), {});

    Func("ep_func", {in_var}, ty.void_(), {Return()}, {Stage(ast::PipelineStage::kFragment)}, {});

    Inspector& inspector = Build();

    auto result = inspector.GetEntryPoints();

    ASSERT_EQ(1u, result.size());
    EXPECT_TRUE(result[0].input_position_used);
}

TEST_F(InspectorGetEntryPointTest, FrontFacingSimpleReferenced) {
    auto* in_var = Param("in_var", ty.bool_(), {Builtin(ast::Builtin::kFrontFacing)});
    Func("ep_func", {in_var}, ty.void_(), {Return()}, {Stage(ast::PipelineStage::kFragment)}, {});

    Inspector& inspector = Build();

    auto result = inspector.GetEntryPoints();

    ASSERT_EQ(1u, result.size());
    EXPECT_TRUE(result[0].front_facing_used);
}

TEST_F(InspectorGetEntryPointTest, FrontFacingStructReferenced) {
    ast::StructMemberList members;
    members.push_back(Member("inner_position", ty.bool_(), {Builtin(ast::Builtin::kFrontFacing)}));
    Structure("in_struct", members);
    auto* in_var = Param("in_var", ty.type_name("in_struct"), {});

    Func("ep_func", {in_var}, ty.void_(), {Return()}, {Stage(ast::PipelineStage::kFragment)}, {});

    Inspector& inspector = Build();

    auto result = inspector.GetEntryPoints();

    ASSERT_EQ(1u, result.size());
    EXPECT_TRUE(result[0].front_facing_used);
}

TEST_F(InspectorGetEntryPointTest, SampleIndexSimpleReferenced) {
    auto* in_var = Param("in_var", ty.u32(), {Builtin(ast::Builtin::kSampleIndex)});
    Func("ep_func", {in_var}, ty.void_(), {Return()}, {Stage(ast::PipelineStage::kFragment)}, {});

    Inspector& inspector = Build();

    auto result = inspector.GetEntryPoints();

    ASSERT_EQ(1u, result.size());
    EXPECT_TRUE(result[0].sample_index_used);
}

TEST_F(InspectorGetEntryPointTest, SampleIndexStructReferenced) {
    ast::StructMemberList members;
    members.push_back(Member("inner_position", ty.u32(), {Builtin(ast::Builtin::kSampleIndex)}));
    Structure("in_struct", members);
    auto* in_var = Param("in_var", ty.type_name("in_struct"), {});

    Func("ep_func", {in_var}, ty.void_(), {Return()}, {Stage(ast::PipelineStage::kFragment)}, {});

    Inspector& inspector = Build();

    auto result = inspector.GetEntryPoints();

    ASSERT_EQ(1u, result.size());
    EXPECT_TRUE(result[0].sample_index_used);
}

TEST_F(InspectorGetEntryPointTest, NumWorkgroupsSimpleReferenced) {
    auto* in_var = Param("in_var", ty.vec3<u32>(), {Builtin(ast::Builtin::kNumWorkgroups)});
    Func("ep_func", {in_var}, ty.void_(), {Return()},
         {Stage(ast::PipelineStage::kCompute), WorkgroupSize(1_i)}, {});

    Inspector& inspector = Build();

    auto result = inspector.GetEntryPoints();

    ASSERT_EQ(1u, result.size());
    EXPECT_TRUE(result[0].num_workgroups_used);
}

TEST_F(InspectorGetEntryPointTest, NumWorkgroupsStructReferenced) {
    ast::StructMemberList members;
    members.push_back(
        Member("inner_position", ty.vec3<u32>(), {Builtin(ast::Builtin::kNumWorkgroups)}));
    Structure("in_struct", members);
    auto* in_var = Param("in_var", ty.type_name("in_struct"), {});

    Func("ep_func", {in_var}, ty.void_(), {Return()},
         {Stage(ast::PipelineStage::kCompute), WorkgroupSize(1_i)}, {});

    Inspector& inspector = Build();

    auto result = inspector.GetEntryPoints();

    ASSERT_EQ(1u, result.size());
    EXPECT_TRUE(result[0].num_workgroups_used);
}

TEST_F(InspectorGetEntryPointTest, ImplicitInterpolate) {
    ast::StructMemberList members;
    members.push_back(Member("struct_inner", ty.f32(), {Location(0)}));
    Structure("in_struct", members);
    auto* in_var = Param("in_var", ty.type_name("in_struct"), {});

    Func("ep_func", {in_var}, ty.void_(), {Return()}, {Stage(ast::PipelineStage::kFragment)}, {});

    Inspector& inspector = Build();

    auto result = inspector.GetEntryPoints();

    ASSERT_EQ(1u, result.size());
    ASSERT_EQ(1u, result[0].input_variables.size());
    EXPECT_EQ(InterpolationType::kPerspective, result[0].input_variables[0].interpolation_type);
    EXPECT_EQ(InterpolationSampling::kCenter, result[0].input_variables[0].interpolation_sampling);
}

TEST_P(InspectorGetEntryPointInterpolateTest, Test) {
    auto& params = GetParam();
    ast::StructMemberList members;
    members.push_back(Member("struct_inner", ty.f32(),
                             {Interpolate(params.in_type, params.in_sampling), Location(0)}));
    Structure("in_struct", members);
    auto* in_var = Param("in_var", ty.type_name("in_struct"), {});

    Func("ep_func", {in_var}, ty.void_(), {Return()}, {Stage(ast::PipelineStage::kFragment)}, {});

    Inspector& inspector = Build();

    auto result = inspector.GetEntryPoints();

    ASSERT_EQ(1u, result.size());
    ASSERT_EQ(1u, result[0].input_variables.size());
    EXPECT_EQ(params.out_type, result[0].input_variables[0].interpolation_type);
    EXPECT_EQ(params.out_sampling, result[0].input_variables[0].interpolation_sampling);
}

INSTANTIATE_TEST_SUITE_P(
    InspectorGetEntryPointTest,
    InspectorGetEntryPointInterpolateTest,
    testing::Values(
        InspectorGetEntryPointInterpolateTestParams{
            ast::InterpolationType::kPerspective, ast::InterpolationSampling::kCenter,
            InterpolationType::kPerspective, InterpolationSampling::kCenter},
        InspectorGetEntryPointInterpolateTestParams{
            ast::InterpolationType::kPerspective, ast::InterpolationSampling::kCentroid,
            InterpolationType::kPerspective, InterpolationSampling::kCentroid},
        InspectorGetEntryPointInterpolateTestParams{
            ast::InterpolationType::kPerspective, ast::InterpolationSampling::kSample,
            InterpolationType::kPerspective, InterpolationSampling::kSample},
        InspectorGetEntryPointInterpolateTestParams{
            ast::InterpolationType::kPerspective, ast::InterpolationSampling::kNone,
            InterpolationType::kPerspective, InterpolationSampling::kCenter},
        InspectorGetEntryPointInterpolateTestParams{
            ast::InterpolationType::kLinear, ast::InterpolationSampling::kCenter,
            InterpolationType::kLinear, InterpolationSampling::kCenter},
        InspectorGetEntryPointInterpolateTestParams{
            ast::InterpolationType::kLinear, ast::InterpolationSampling::kCentroid,
            InterpolationType::kLinear, InterpolationSampling::kCentroid},
        InspectorGetEntryPointInterpolateTestParams{
            ast::InterpolationType::kLinear, ast::InterpolationSampling::kSample,
            InterpolationType::kLinear, InterpolationSampling::kSample},
        InspectorGetEntryPointInterpolateTestParams{
            ast::InterpolationType::kLinear, ast::InterpolationSampling::kNone,
            InterpolationType::kLinear, InterpolationSampling::kCenter},
        InspectorGetEntryPointInterpolateTestParams{
            ast::InterpolationType::kFlat, ast::InterpolationSampling::kNone,
            InterpolationType::kFlat, InterpolationSampling::kNone}));

TEST_F(InspectorGetConstantIDsTest, Bool) {
    AddOverridableConstantWithID("foo", 1, ty.bool_(), nullptr);
    AddOverridableConstantWithID("bar", 20, ty.bool_(), Expr(true));
    AddOverridableConstantWithID("baz", 300, ty.bool_(), Expr(false));

    Inspector& inspector = Build();

    auto result = inspector.GetConstantIDs();
    ASSERT_EQ(3u, result.size());

    ASSERT_TRUE(result.find(1) != result.end());
    EXPECT_TRUE(result[1].IsNull());

    ASSERT_TRUE(result.find(20) != result.end());
    EXPECT_TRUE(result[20].IsBool());
    EXPECT_TRUE(result[20].AsBool());

    ASSERT_TRUE(result.find(300) != result.end());
    EXPECT_TRUE(result[300].IsBool());
    EXPECT_FALSE(result[300].AsBool());
}

TEST_F(InspectorGetConstantIDsTest, U32) {
    AddOverridableConstantWithID("foo", 1, ty.u32(), nullptr);
    AddOverridableConstantWithID("bar", 20, ty.u32(), Expr(42_u));

    Inspector& inspector = Build();

    auto result = inspector.GetConstantIDs();
    ASSERT_EQ(2u, result.size());

    ASSERT_TRUE(result.find(1) != result.end());
    EXPECT_TRUE(result[1].IsNull());

    ASSERT_TRUE(result.find(20) != result.end());
    EXPECT_TRUE(result[20].IsU32());
    EXPECT_EQ(42u, result[20].AsU32());
}

TEST_F(InspectorGetConstantIDsTest, I32) {
    AddOverridableConstantWithID("foo", 1, ty.i32(), nullptr);
    AddOverridableConstantWithID("bar", 20, ty.i32(), Expr(i32(-42)));
    AddOverridableConstantWithID("baz", 300, ty.i32(), Expr(42_i));

    Inspector& inspector = Build();

    auto result = inspector.GetConstantIDs();
    ASSERT_EQ(3u, result.size());

    ASSERT_TRUE(result.find(1) != result.end());
    EXPECT_TRUE(result[1].IsNull());

    ASSERT_TRUE(result.find(20) != result.end());
    EXPECT_TRUE(result[20].IsI32());
    EXPECT_EQ(-42, result[20].AsI32());

    ASSERT_TRUE(result.find(300) != result.end());
    EXPECT_TRUE(result[300].IsI32());
    EXPECT_EQ(42, result[300].AsI32());
}

TEST_F(InspectorGetConstantIDsTest, Float) {
    AddOverridableConstantWithID("foo", 1, ty.f32(), nullptr);
    AddOverridableConstantWithID("bar", 20, ty.f32(), Expr(0.0f));
    AddOverridableConstantWithID("baz", 300, ty.f32(), Expr(-10.0f));
    AddOverridableConstantWithID("x", 4000, ty.f32(), Expr(15.0f));

    Inspector& inspector = Build();

    auto result = inspector.GetConstantIDs();
    ASSERT_EQ(4u, result.size());

    ASSERT_TRUE(result.find(1) != result.end());
    EXPECT_TRUE(result[1].IsNull());

    ASSERT_TRUE(result.find(20) != result.end());
    EXPECT_TRUE(result[20].IsFloat());
    EXPECT_FLOAT_EQ(0.0, result[20].AsFloat());

    ASSERT_TRUE(result.find(300) != result.end());
    EXPECT_TRUE(result[300].IsFloat());
    EXPECT_FLOAT_EQ(-10.0, result[300].AsFloat());

    ASSERT_TRUE(result.find(4000) != result.end());
    EXPECT_TRUE(result[4000].IsFloat());
    EXPECT_FLOAT_EQ(15.0, result[4000].AsFloat());
}

TEST_F(InspectorGetConstantNameToIdMapTest, WithAndWithoutIds) {
    AddOverridableConstantWithID("v1", 1, ty.f32(), nullptr);
    AddOverridableConstantWithID("v20", 20, ty.f32(), nullptr);
    AddOverridableConstantWithID("v300", 300, ty.f32(), nullptr);
    auto* a = AddOverridableConstantWithoutID("a", ty.f32(), nullptr);
    auto* b = AddOverridableConstantWithoutID("b", ty.f32(), nullptr);
    auto* c = AddOverridableConstantWithoutID("c", ty.f32(), nullptr);

    Inspector& inspector = Build();

    auto result = inspector.GetConstantNameToIdMap();
    ASSERT_EQ(6u, result.size());

    ASSERT_TRUE(result.count("v1"));
    EXPECT_EQ(result["v1"], 1u);

    ASSERT_TRUE(result.count("v20"));
    EXPECT_EQ(result["v20"], 20u);

    ASSERT_TRUE(result.count("v300"));
    EXPECT_EQ(result["v300"], 300u);

    ASSERT_TRUE(result.count("a"));
    ASSERT_TRUE(program_->Sem().Get<sem::GlobalVariable>(a));
    EXPECT_EQ(result["a"], program_->Sem().Get<sem::GlobalVariable>(a)->ConstantId());

    ASSERT_TRUE(result.count("b"));
    ASSERT_TRUE(program_->Sem().Get<sem::GlobalVariable>(b));
    EXPECT_EQ(result["b"], program_->Sem().Get<sem::GlobalVariable>(b)->ConstantId());

    ASSERT_TRUE(result.count("c"));
    ASSERT_TRUE(program_->Sem().Get<sem::GlobalVariable>(c));
    EXPECT_EQ(result["c"], program_->Sem().Get<sem::GlobalVariable>(c)->ConstantId());
}

TEST_F(InspectorGetStorageSizeTest, Empty) {
    MakeEmptyBodyFunction(
        "ep_func", ast::AttributeList{Stage(ast::PipelineStage::kCompute), WorkgroupSize(1_i)});
    Inspector& inspector = Build();
    EXPECT_EQ(0u, inspector.GetStorageSize("ep_func"));
}

TEST_F(InspectorGetStorageSizeTest, Simple_NonStruct) {
    AddUniformBuffer("ub_var", ty.i32(), 0, 0);
    AddStorageBuffer("sb_var", ty.i32(), ast::Access::kReadWrite, 1, 0);
    AddStorageBuffer("rosb_var", ty.i32(), ast::Access::kRead, 1, 1);
    Func("ep_func", {}, ty.void_(),
         {
             Decl(Let("ub", nullptr, Expr("ub_var"))),
             Decl(Let("sb", nullptr, Expr("sb_var"))),
             Decl(Let("rosb", nullptr, Expr("rosb_var"))),
         },
         {Stage(ast::PipelineStage::kCompute), WorkgroupSize(1_i)});

    Inspector& inspector = Build();

    EXPECT_EQ(12u, inspector.GetStorageSize("ep_func"));
}

TEST_F(InspectorGetStorageSizeTest, Simple_Struct) {
    auto* ub_struct_type = MakeUniformBufferType("ub_type", {ty.i32(), ty.i32()});
    AddUniformBuffer("ub_var", ty.Of(ub_struct_type), 0, 0);
    MakeStructVariableReferenceBodyFunction("ub_func", "ub_var", {{0, ty.i32()}});

    auto sb = MakeStorageBufferTypes("sb_type", {ty.i32()});
    AddStorageBuffer("sb_var", sb(), ast::Access::kReadWrite, 1, 0);
    MakeStructVariableReferenceBodyFunction("sb_func", "sb_var", {{0, ty.i32()}});

    auto ro_sb = MakeStorageBufferTypes("rosb_type", {ty.i32()});
    AddStorageBuffer("rosb_var", ro_sb(), ast::Access::kRead, 1, 1);
    MakeStructVariableReferenceBodyFunction("rosb_func", "rosb_var", {{0, ty.i32()}});

    MakeCallerBodyFunction("ep_func", {"ub_func", "sb_func", "rosb_func"},
                           ast::AttributeList{
                               Stage(ast::PipelineStage::kCompute),
                               WorkgroupSize(1_i),
                           });

    Inspector& inspector = Build();

    EXPECT_EQ(16u, inspector.GetStorageSize("ep_func"));
}

TEST_F(InspectorGetStorageSizeTest, NonStructVec3) {
    AddUniformBuffer("ub_var", ty.vec3<f32>(), 0, 0);
    Func("ep_func", {}, ty.void_(),
         {
             Decl(Let("ub", nullptr, Expr("ub_var"))),
         },
         {Stage(ast::PipelineStage::kCompute), WorkgroupSize(1_i)});

    Inspector& inspector = Build();

    EXPECT_EQ(12u, inspector.GetStorageSize("ep_func"));
}

TEST_F(InspectorGetStorageSizeTest, StructVec3) {
    auto* ub_struct_type = MakeUniformBufferType("ub_type", {ty.vec3<f32>()});
    AddUniformBuffer("ub_var", ty.Of(ub_struct_type), 0, 0);
    Func("ep_func", {}, ty.void_(),
         {
             Decl(Let("ub", nullptr, Expr("ub_var"))),
         },
         {Stage(ast::PipelineStage::kCompute), WorkgroupSize(1_i)});

    Inspector& inspector = Build();

    EXPECT_EQ(16u, inspector.GetStorageSize("ep_func"));
}

TEST_F(InspectorGetResourceBindingsTest, Empty) {
    MakeCallerBodyFunction("ep_func", {},
                           ast::AttributeList{
                               Stage(ast::PipelineStage::kFragment),
                           });

    Inspector& inspector = Build();

    auto result = inspector.GetResourceBindings("ep_func");
    ASSERT_FALSE(inspector.has_error()) << inspector.error();
    ASSERT_EQ(0u, result.size());
}

TEST_F(InspectorGetResourceBindingsTest, Simple) {
    auto* ub_struct_type = MakeUniformBufferType("ub_type", {ty.i32()});
    AddUniformBuffer("ub_var", ty.Of(ub_struct_type), 0, 0);
    MakeStructVariableReferenceBodyFunction("ub_func", "ub_var", {{0, ty.i32()}});

    auto sb = MakeStorageBufferTypes("sb_type", {ty.i32()});
    AddStorageBuffer("sb_var", sb(), ast::Access::kReadWrite, 1, 0);
    MakeStructVariableReferenceBodyFunction("sb_func", "sb_var", {{0, ty.i32()}});

    auto ro_sb = MakeStorageBufferTypes("rosb_type", {ty.i32()});
    AddStorageBuffer("rosb_var", ro_sb(), ast::Access::kRead, 1, 1);
    MakeStructVariableReferenceBodyFunction("rosb_func", "rosb_var", {{0, ty.i32()}});

    auto* s_texture_type = ty.sampled_texture(ast::TextureDimension::k1d, ty.f32());
    AddResource("s_texture", s_texture_type, 2, 0);
    AddSampler("s_var", 3, 0);
    AddGlobalVariable("s_coords", ty.f32());
    MakeSamplerReferenceBodyFunction("s_func", "s_texture", "s_var", "s_coords", ty.f32(), {});

    auto* cs_depth_texture_type = ty.depth_texture(ast::TextureDimension::k2d);
    AddResource("cs_texture", cs_depth_texture_type, 3, 1);
    AddComparisonSampler("cs_var", 3, 2);
    AddGlobalVariable("cs_coords", ty.vec2<f32>());
    AddGlobalVariable("cs_depth", ty.f32());
    MakeComparisonSamplerReferenceBodyFunction("cs_func", "cs_texture", "cs_var", "cs_coords",
                                               "cs_depth", ty.f32(), {});

    auto* depth_ms_texture_type = ty.depth_multisampled_texture(ast::TextureDimension::k2d);
    AddResource("depth_ms_texture", depth_ms_texture_type, 3, 3);
    Func("depth_ms_func", {}, ty.void_(), {Ignore("depth_ms_texture")});

    auto* st_type = MakeStorageTextureTypes(ast::TextureDimension::k2d, ast::TexelFormat::kR32Uint);
    AddStorageTexture("st_var", st_type, 4, 0);
    MakeStorageTextureBodyFunction("st_func", "st_var", ty.vec2<i32>(), {});

    MakeCallerBodyFunction(
        "ep_func",
        {"ub_func", "sb_func", "rosb_func", "s_func", "cs_func", "depth_ms_func", "st_func"},
        ast::AttributeList{
            Stage(ast::PipelineStage::kFragment),
        });

    Inspector& inspector = Build();

    auto result = inspector.GetResourceBindings("ep_func");
    ASSERT_FALSE(inspector.has_error()) << inspector.error();
    ASSERT_EQ(9u, result.size());

    EXPECT_EQ(ResourceBinding::ResourceType::kUniformBuffer, result[0].resource_type);
    EXPECT_EQ(0u, result[0].bind_group);
    EXPECT_EQ(0u, result[0].binding);

    EXPECT_EQ(ResourceBinding::ResourceType::kStorageBuffer, result[1].resource_type);
    EXPECT_EQ(1u, result[1].bind_group);
    EXPECT_EQ(0u, result[1].binding);

    EXPECT_EQ(ResourceBinding::ResourceType::kReadOnlyStorageBuffer, result[2].resource_type);
    EXPECT_EQ(1u, result[2].bind_group);
    EXPECT_EQ(1u, result[2].binding);

    EXPECT_EQ(ResourceBinding::ResourceType::kSampler, result[3].resource_type);
    EXPECT_EQ(3u, result[3].bind_group);
    EXPECT_EQ(0u, result[3].binding);

    EXPECT_EQ(ResourceBinding::ResourceType::kComparisonSampler, result[4].resource_type);
    EXPECT_EQ(3u, result[4].bind_group);
    EXPECT_EQ(2u, result[4].binding);

    EXPECT_EQ(ResourceBinding::ResourceType::kSampledTexture, result[5].resource_type);
    EXPECT_EQ(2u, result[5].bind_group);
    EXPECT_EQ(0u, result[5].binding);

    EXPECT_EQ(ResourceBinding::ResourceType::kWriteOnlyStorageTexture, result[6].resource_type);
    EXPECT_EQ(4u, result[6].bind_group);
    EXPECT_EQ(0u, result[6].binding);

    EXPECT_EQ(ResourceBinding::ResourceType::kDepthTexture, result[7].resource_type);
    EXPECT_EQ(3u, result[7].bind_group);
    EXPECT_EQ(1u, result[7].binding);

    EXPECT_EQ(ResourceBinding::ResourceType::kDepthMultisampledTexture, result[8].resource_type);
    EXPECT_EQ(3u, result[8].bind_group);
    EXPECT_EQ(3u, result[8].binding);
}

TEST_F(InspectorGetUniformBufferResourceBindingsTest, MissingEntryPoint) {
    Inspector& inspector = Build();

    auto result = inspector.GetUniformBufferResourceBindings("ep_func");
    ASSERT_TRUE(inspector.has_error());
    std::string error = inspector.error();
    EXPECT_TRUE(error.find("not found") != std::string::npos);
}

TEST_F(InspectorGetUniformBufferResourceBindingsTest, NonEntryPointFunc) {
    auto* foo_struct_type = MakeUniformBufferType("foo_type", {ty.i32()});
    AddUniformBuffer("foo_ub", ty.Of(foo_struct_type), 0, 0);

    MakeStructVariableReferenceBodyFunction("ub_func", "foo_ub", {{0, ty.i32()}});

    MakeCallerBodyFunction("ep_func", {"ub_func"},
                           ast::AttributeList{
                               Stage(ast::PipelineStage::kFragment),
                           });

    Inspector& inspector = Build();

    auto result = inspector.GetUniformBufferResourceBindings("ub_func");
    std::string error = inspector.error();
    EXPECT_TRUE(error.find("not an entry point") != std::string::npos);
}

TEST_F(InspectorGetUniformBufferResourceBindingsTest, Simple_NonStruct) {
    AddUniformBuffer("foo_ub", ty.i32(), 0, 0);
    MakePlainGlobalReferenceBodyFunction("ub_func", "foo_ub", ty.i32(), {});

    MakeCallerBodyFunction("ep_func", {"ub_func"},
                           ast::AttributeList{
                               Stage(ast::PipelineStage::kFragment),
                           });

    Inspector& inspector = Build();

    auto result = inspector.GetUniformBufferResourceBindings("ep_func");
    ASSERT_FALSE(inspector.has_error()) << inspector.error();
    ASSERT_EQ(1u, result.size());

    EXPECT_EQ(ResourceBinding::ResourceType::kUniformBuffer, result[0].resource_type);
    EXPECT_EQ(0u, result[0].bind_group);
    EXPECT_EQ(0u, result[0].binding);
    EXPECT_EQ(4u, result[0].size);
    EXPECT_EQ(4u, result[0].size_no_padding);
}

TEST_F(InspectorGetUniformBufferResourceBindingsTest, Simple_Struct) {
    auto* foo_struct_type = MakeUniformBufferType("foo_type", {ty.i32()});
    AddUniformBuffer("foo_ub", ty.Of(foo_struct_type), 0, 0);

    MakeStructVariableReferenceBodyFunction("ub_func", "foo_ub", {{0, ty.i32()}});

    MakeCallerBodyFunction("ep_func", {"ub_func"},
                           ast::AttributeList{
                               Stage(ast::PipelineStage::kFragment),
                           });

    Inspector& inspector = Build();

    auto result = inspector.GetUniformBufferResourceBindings("ep_func");
    ASSERT_FALSE(inspector.has_error()) << inspector.error();
    ASSERT_EQ(1u, result.size());

    EXPECT_EQ(ResourceBinding::ResourceType::kUniformBuffer, result[0].resource_type);
    EXPECT_EQ(0u, result[0].bind_group);
    EXPECT_EQ(0u, result[0].binding);
    EXPECT_EQ(4u, result[0].size);
    EXPECT_EQ(4u, result[0].size_no_padding);
}

TEST_F(InspectorGetUniformBufferResourceBindingsTest, MultipleMembers) {
    auto* foo_struct_type = MakeUniformBufferType("foo_type", {ty.i32(), ty.u32(), ty.f32()});
    AddUniformBuffer("foo_ub", ty.Of(foo_struct_type), 0, 0);

    MakeStructVariableReferenceBodyFunction("ub_func", "foo_ub",
                                            {{0, ty.i32()}, {1, ty.u32()}, {2, ty.f32()}});

    MakeCallerBodyFunction("ep_func", {"ub_func"},
                           ast::AttributeList{
                               Stage(ast::PipelineStage::kFragment),
                           });

    Inspector& inspector = Build();

    auto result = inspector.GetUniformBufferResourceBindings("ep_func");
    ASSERT_FALSE(inspector.has_error()) << inspector.error();
    ASSERT_EQ(1u, result.size());

    EXPECT_EQ(ResourceBinding::ResourceType::kUniformBuffer, result[0].resource_type);
    EXPECT_EQ(0u, result[0].bind_group);
    EXPECT_EQ(0u, result[0].binding);
    EXPECT_EQ(12u, result[0].size);
    EXPECT_EQ(12u, result[0].size_no_padding);
}

TEST_F(InspectorGetUniformBufferResourceBindingsTest, ContainingPadding) {
    auto* foo_struct_type = MakeUniformBufferType("foo_type", {ty.vec3<f32>()});
    AddUniformBuffer("foo_ub", ty.Of(foo_struct_type), 0, 0);

    MakeStructVariableReferenceBodyFunction("ub_func", "foo_ub", {{0, ty.vec3<f32>()}});

    MakeCallerBodyFunction("ep_func", {"ub_func"},
                           ast::AttributeList{
                               Stage(ast::PipelineStage::kFragment),
                           });

    Inspector& inspector = Build();

    auto result = inspector.GetUniformBufferResourceBindings("ep_func");
    ASSERT_FALSE(inspector.has_error()) << inspector.error();
    ASSERT_EQ(1u, result.size());

    EXPECT_EQ(ResourceBinding::ResourceType::kUniformBuffer, result[0].resource_type);
    EXPECT_EQ(0u, result[0].bind_group);
    EXPECT_EQ(0u, result[0].binding);
    EXPECT_EQ(16u, result[0].size);
    EXPECT_EQ(12u, result[0].size_no_padding);
}

TEST_F(InspectorGetUniformBufferResourceBindingsTest, NonStructVec3) {
    AddUniformBuffer("foo_ub", ty.vec3<f32>(), 0, 0);
    MakePlainGlobalReferenceBodyFunction("ub_func", "foo_ub", ty.vec3<f32>(), {});

    MakeCallerBodyFunction("ep_func", {"ub_func"},
                           ast::AttributeList{
                               Stage(ast::PipelineStage::kFragment),
                           });

    Inspector& inspector = Build();

    auto result = inspector.GetUniformBufferResourceBindings("ep_func");
    ASSERT_FALSE(inspector.has_error()) << inspector.error();
    ASSERT_EQ(1u, result.size());

    EXPECT_EQ(ResourceBinding::ResourceType::kUniformBuffer, result[0].resource_type);
    EXPECT_EQ(0u, result[0].bind_group);
    EXPECT_EQ(0u, result[0].binding);
    EXPECT_EQ(12u, result[0].size);
    EXPECT_EQ(12u, result[0].size_no_padding);
}

TEST_F(InspectorGetUniformBufferResourceBindingsTest, MultipleUniformBuffers) {
    auto* ub_struct_type = MakeUniformBufferType("ub_type", {ty.i32(), ty.u32(), ty.f32()});
    AddUniformBuffer("ub_foo", ty.Of(ub_struct_type), 0, 0);
    AddUniformBuffer("ub_bar", ty.Of(ub_struct_type), 0, 1);
    AddUniformBuffer("ub_baz", ty.Of(ub_struct_type), 2, 0);

    auto AddReferenceFunc = [this](const std::string& func_name, const std::string& var_name) {
        MakeStructVariableReferenceBodyFunction(func_name, var_name,
                                                {{0, ty.i32()}, {1, ty.u32()}, {2, ty.f32()}});
    };
    AddReferenceFunc("ub_foo_func", "ub_foo");
    AddReferenceFunc("ub_bar_func", "ub_bar");
    AddReferenceFunc("ub_baz_func", "ub_baz");

    auto FuncCall = [&](const std::string& callee) {
        return create<ast::CallStatement>(Call(callee));
    };

    Func("ep_func", ast::VariableList(), ty.void_(),
         ast::StatementList{FuncCall("ub_foo_func"), FuncCall("ub_bar_func"),
                            FuncCall("ub_baz_func"), Return()},
         ast::AttributeList{
             Stage(ast::PipelineStage::kFragment),
         });

    Inspector& inspector = Build();

    auto result = inspector.GetUniformBufferResourceBindings("ep_func");
    ASSERT_FALSE(inspector.has_error()) << inspector.error();
    ASSERT_EQ(3u, result.size());

    EXPECT_EQ(ResourceBinding::ResourceType::kUniformBuffer, result[0].resource_type);
    EXPECT_EQ(0u, result[0].bind_group);
    EXPECT_EQ(0u, result[0].binding);
    EXPECT_EQ(12u, result[0].size);
    EXPECT_EQ(12u, result[0].size_no_padding);

    EXPECT_EQ(ResourceBinding::ResourceType::kUniformBuffer, result[1].resource_type);
    EXPECT_EQ(0u, result[1].bind_group);
    EXPECT_EQ(1u, result[1].binding);
    EXPECT_EQ(12u, result[1].size);
    EXPECT_EQ(12u, result[1].size_no_padding);

    EXPECT_EQ(ResourceBinding::ResourceType::kUniformBuffer, result[2].resource_type);
    EXPECT_EQ(2u, result[2].bind_group);
    EXPECT_EQ(0u, result[2].binding);
    EXPECT_EQ(12u, result[2].size);
    EXPECT_EQ(12u, result[2].size_no_padding);
}

TEST_F(InspectorGetUniformBufferResourceBindingsTest, ContainingArray) {
    // Manually create uniform buffer to make sure it had a valid layout (array
    // with elem stride of 16, and that is 16-byte aligned within the struct)
    auto* foo_struct_type = Structure(
        "foo_type", {Member("0i32", ty.i32()),
                     Member("b", ty.array(ty.u32(), 4_u, /*stride*/ 16), {MemberAlign(16)})});

    AddUniformBuffer("foo_ub", ty.Of(foo_struct_type), 0, 0);

    MakeStructVariableReferenceBodyFunction("ub_func", "foo_ub", {{0, ty.i32()}});

    MakeCallerBodyFunction("ep_func", {"ub_func"},
                           ast::AttributeList{
                               Stage(ast::PipelineStage::kFragment),
                           });

    Inspector& inspector = Build();

    auto result = inspector.GetUniformBufferResourceBindings("ep_func");
    ASSERT_FALSE(inspector.has_error()) << inspector.error();
    ASSERT_EQ(1u, result.size());

    EXPECT_EQ(ResourceBinding::ResourceType::kUniformBuffer, result[0].resource_type);
    EXPECT_EQ(0u, result[0].bind_group);
    EXPECT_EQ(0u, result[0].binding);
    EXPECT_EQ(80u, result[0].size);
    EXPECT_EQ(80u, result[0].size_no_padding);
}

TEST_F(InspectorGetStorageBufferResourceBindingsTest, Simple_NonStruct) {
    AddStorageBuffer("foo_sb", ty.i32(), ast::Access::kReadWrite, 0, 0);
    MakePlainGlobalReferenceBodyFunction("sb_func", "foo_sb", ty.i32(), {});

    MakeCallerBodyFunction("ep_func", {"sb_func"},
                           ast::AttributeList{
                               Stage(ast::PipelineStage::kFragment),
                           });

    Inspector& inspector = Build();

    auto result = inspector.GetStorageBufferResourceBindings("ep_func");
    ASSERT_FALSE(inspector.has_error()) << inspector.error();
    ASSERT_EQ(1u, result.size());

    EXPECT_EQ(ResourceBinding::ResourceType::kStorageBuffer, result[0].resource_type);
    EXPECT_EQ(0u, result[0].bind_group);
    EXPECT_EQ(0u, result[0].binding);
    EXPECT_EQ(4u, result[0].size);
    EXPECT_EQ(4u, result[0].size_no_padding);
}

TEST_F(InspectorGetStorageBufferResourceBindingsTest, Simple_Struct) {
    auto foo_struct_type = MakeStorageBufferTypes("foo_type", {ty.i32()});
    AddStorageBuffer("foo_sb", foo_struct_type(), ast::Access::kReadWrite, 0, 0);

    MakeStructVariableReferenceBodyFunction("sb_func", "foo_sb", {{0, ty.i32()}});

    MakeCallerBodyFunction("ep_func", {"sb_func"},
                           ast::AttributeList{
                               Stage(ast::PipelineStage::kFragment),
                           });

    Inspector& inspector = Build();

    auto result = inspector.GetStorageBufferResourceBindings("ep_func");
    ASSERT_FALSE(inspector.has_error()) << inspector.error();
    ASSERT_EQ(1u, result.size());

    EXPECT_EQ(ResourceBinding::ResourceType::kStorageBuffer, result[0].resource_type);
    EXPECT_EQ(0u, result[0].bind_group);
    EXPECT_EQ(0u, result[0].binding);
    EXPECT_EQ(4u, result[0].size);
    EXPECT_EQ(4u, result[0].size_no_padding);
}

TEST_F(InspectorGetStorageBufferResourceBindingsTest, MultipleMembers) {
    auto foo_struct_type = MakeStorageBufferTypes("foo_type", {
                                                                  ty.i32(),
                                                                  ty.u32(),
                                                                  ty.f32(),
                                                              });
    AddStorageBuffer("foo_sb", foo_struct_type(), ast::Access::kReadWrite, 0, 0);

    MakeStructVariableReferenceBodyFunction("sb_func", "foo_sb",
                                            {{0, ty.i32()}, {1, ty.u32()}, {2, ty.f32()}});

    MakeCallerBodyFunction("ep_func", {"sb_func"},
                           ast::AttributeList{
                               Stage(ast::PipelineStage::kFragment),
                           });

    Inspector& inspector = Build();

    auto result = inspector.GetStorageBufferResourceBindings("ep_func");
    ASSERT_FALSE(inspector.has_error()) << inspector.error();
    ASSERT_EQ(1u, result.size());

    EXPECT_EQ(ResourceBinding::ResourceType::kStorageBuffer, result[0].resource_type);
    EXPECT_EQ(0u, result[0].bind_group);
    EXPECT_EQ(0u, result[0].binding);
    EXPECT_EQ(12u, result[0].size);
    EXPECT_EQ(12u, result[0].size_no_padding);
}

TEST_F(InspectorGetStorageBufferResourceBindingsTest, MultipleStorageBuffers) {
    auto sb_struct_type = MakeStorageBufferTypes("sb_type", {
                                                                ty.i32(),
                                                                ty.u32(),
                                                                ty.f32(),
                                                            });
    AddStorageBuffer("sb_foo", sb_struct_type(), ast::Access::kReadWrite, 0, 0);
    AddStorageBuffer("sb_bar", sb_struct_type(), ast::Access::kReadWrite, 0, 1);
    AddStorageBuffer("sb_baz", sb_struct_type(), ast::Access::kReadWrite, 2, 0);

    auto AddReferenceFunc = [this](const std::string& func_name, const std::string& var_name) {
        MakeStructVariableReferenceBodyFunction(func_name, var_name,
                                                {{0, ty.i32()}, {1, ty.u32()}, {2, ty.f32()}});
    };
    AddReferenceFunc("sb_foo_func", "sb_foo");
    AddReferenceFunc("sb_bar_func", "sb_bar");
    AddReferenceFunc("sb_baz_func", "sb_baz");

    auto FuncCall = [&](const std::string& callee) {
        return create<ast::CallStatement>(Call(callee));
    };

    Func("ep_func", ast::VariableList(), ty.void_(),
         ast::StatementList{
             FuncCall("sb_foo_func"),
             FuncCall("sb_bar_func"),
             FuncCall("sb_baz_func"),
             Return(),
         },
         ast::AttributeList{
             Stage(ast::PipelineStage::kFragment),
         });

    Inspector& inspector = Build();

    auto result = inspector.GetStorageBufferResourceBindings("ep_func");
    ASSERT_FALSE(inspector.has_error()) << inspector.error();
    ASSERT_EQ(3u, result.size());

    EXPECT_EQ(ResourceBinding::ResourceType::kStorageBuffer, result[0].resource_type);
    EXPECT_EQ(0u, result[0].bind_group);
    EXPECT_EQ(0u, result[0].binding);
    EXPECT_EQ(12u, result[0].size);
    EXPECT_EQ(12u, result[0].size_no_padding);

    EXPECT_EQ(ResourceBinding::ResourceType::kStorageBuffer, result[1].resource_type);
    EXPECT_EQ(0u, result[1].bind_group);
    EXPECT_EQ(1u, result[1].binding);
    EXPECT_EQ(12u, result[1].size);
    EXPECT_EQ(12u, result[1].size_no_padding);

    EXPECT_EQ(ResourceBinding::ResourceType::kStorageBuffer, result[2].resource_type);
    EXPECT_EQ(2u, result[2].bind_group);
    EXPECT_EQ(0u, result[2].binding);
    EXPECT_EQ(12u, result[2].size);
    EXPECT_EQ(12u, result[2].size_no_padding);
}

TEST_F(InspectorGetStorageBufferResourceBindingsTest, ContainingArray) {
    auto foo_struct_type = MakeStorageBufferTypes("foo_type", {ty.i32(), ty.array<u32, 4>()});
    AddStorageBuffer("foo_sb", foo_struct_type(), ast::Access::kReadWrite, 0, 0);

    MakeStructVariableReferenceBodyFunction("sb_func", "foo_sb", {{0, ty.i32()}});

    MakeCallerBodyFunction("ep_func", {"sb_func"},
                           ast::AttributeList{
                               Stage(ast::PipelineStage::kFragment),
                           });

    Inspector& inspector = Build();

    auto result = inspector.GetStorageBufferResourceBindings("ep_func");
    ASSERT_FALSE(inspector.has_error()) << inspector.error();
    ASSERT_EQ(1u, result.size());

    EXPECT_EQ(ResourceBinding::ResourceType::kStorageBuffer, result[0].resource_type);
    EXPECT_EQ(0u, result[0].bind_group);
    EXPECT_EQ(0u, result[0].binding);
    EXPECT_EQ(20u, result[0].size);
    EXPECT_EQ(20u, result[0].size_no_padding);
}

TEST_F(InspectorGetStorageBufferResourceBindingsTest, ContainingRuntimeArray) {
    auto foo_struct_type = MakeStorageBufferTypes("foo_type", {
                                                                  ty.i32(),
                                                                  ty.array<u32>(),
                                                              });
    AddStorageBuffer("foo_sb", foo_struct_type(), ast::Access::kReadWrite, 0, 0);

    MakeStructVariableReferenceBodyFunction("sb_func", "foo_sb", {{0, ty.i32()}});

    MakeCallerBodyFunction("ep_func", {"sb_func"},
                           ast::AttributeList{
                               Stage(ast::PipelineStage::kFragment),
                           });

    Inspector& inspector = Build();

    auto result = inspector.GetStorageBufferResourceBindings("ep_func");
    ASSERT_FALSE(inspector.has_error()) << inspector.error();
    ASSERT_EQ(1u, result.size());

    EXPECT_EQ(ResourceBinding::ResourceType::kStorageBuffer, result[0].resource_type);
    EXPECT_EQ(0u, result[0].bind_group);
    EXPECT_EQ(0u, result[0].binding);
    EXPECT_EQ(8u, result[0].size);
    EXPECT_EQ(8u, result[0].size_no_padding);
}

TEST_F(InspectorGetStorageBufferResourceBindingsTest, ContainingPadding) {
    auto foo_struct_type = MakeStorageBufferTypes("foo_type", {ty.vec3<f32>()});
    AddStorageBuffer("foo_sb", foo_struct_type(), ast::Access::kReadWrite, 0, 0);

    MakeStructVariableReferenceBodyFunction("sb_func", "foo_sb", {{0, ty.vec3<f32>()}});

    MakeCallerBodyFunction("ep_func", {"sb_func"},
                           ast::AttributeList{
                               Stage(ast::PipelineStage::kFragment),
                           });

    Inspector& inspector = Build();

    auto result = inspector.GetStorageBufferResourceBindings("ep_func");
    ASSERT_FALSE(inspector.has_error()) << inspector.error();
    ASSERT_EQ(1u, result.size());

    EXPECT_EQ(ResourceBinding::ResourceType::kStorageBuffer, result[0].resource_type);
    EXPECT_EQ(0u, result[0].bind_group);
    EXPECT_EQ(0u, result[0].binding);
    EXPECT_EQ(16u, result[0].size);
    EXPECT_EQ(12u, result[0].size_no_padding);
}

TEST_F(InspectorGetStorageBufferResourceBindingsTest, NonStructVec3) {
    AddStorageBuffer("foo_ub", ty.vec3<f32>(), ast::Access::kReadWrite, 0, 0);
    MakePlainGlobalReferenceBodyFunction("ub_func", "foo_ub", ty.vec3<f32>(), {});

    MakeCallerBodyFunction("ep_func", {"ub_func"},
                           ast::AttributeList{
                               Stage(ast::PipelineStage::kFragment),
                           });

    Inspector& inspector = Build();

    auto result = inspector.GetStorageBufferResourceBindings("ep_func");
    ASSERT_FALSE(inspector.has_error()) << inspector.error();
    ASSERT_EQ(1u, result.size());

    EXPECT_EQ(ResourceBinding::ResourceType::kStorageBuffer, result[0].resource_type);
    EXPECT_EQ(0u, result[0].bind_group);
    EXPECT_EQ(0u, result[0].binding);
    EXPECT_EQ(12u, result[0].size);
    EXPECT_EQ(12u, result[0].size_no_padding);
}

TEST_F(InspectorGetStorageBufferResourceBindingsTest, SkipReadOnly) {
    auto foo_struct_type = MakeStorageBufferTypes("foo_type", {ty.i32()});
    AddStorageBuffer("foo_sb", foo_struct_type(), ast::Access::kRead, 0, 0);

    MakeStructVariableReferenceBodyFunction("sb_func", "foo_sb", {{0, ty.i32()}});

    MakeCallerBodyFunction("ep_func", {"sb_func"},
                           ast::AttributeList{
                               Stage(ast::PipelineStage::kFragment),
                           });

    Inspector& inspector = Build();

    auto result = inspector.GetStorageBufferResourceBindings("ep_func");
    ASSERT_FALSE(inspector.has_error()) << inspector.error();
    ASSERT_EQ(0u, result.size());
}

TEST_F(InspectorGetReadOnlyStorageBufferResourceBindingsTest, Simple) {
    auto foo_struct_type = MakeStorageBufferTypes("foo_type", {ty.i32()});
    AddStorageBuffer("foo_sb", foo_struct_type(), ast::Access::kRead, 0, 0);

    MakeStructVariableReferenceBodyFunction("sb_func", "foo_sb", {{0, ty.i32()}});

    MakeCallerBodyFunction("ep_func", {"sb_func"},
                           ast::AttributeList{
                               Stage(ast::PipelineStage::kFragment),
                           });

    Inspector& inspector = Build();

    auto result = inspector.GetReadOnlyStorageBufferResourceBindings("ep_func");
    ASSERT_FALSE(inspector.has_error()) << inspector.error();
    ASSERT_EQ(1u, result.size());

    EXPECT_EQ(ResourceBinding::ResourceType::kReadOnlyStorageBuffer, result[0].resource_type);
    EXPECT_EQ(0u, result[0].bind_group);
    EXPECT_EQ(0u, result[0].binding);
    EXPECT_EQ(4u, result[0].size);
    EXPECT_EQ(4u, result[0].size_no_padding);
}

TEST_F(InspectorGetReadOnlyStorageBufferResourceBindingsTest, MultipleStorageBuffers) {
    auto sb_struct_type = MakeStorageBufferTypes("sb_type", {
                                                                ty.i32(),
                                                                ty.u32(),
                                                                ty.f32(),
                                                            });
    AddStorageBuffer("sb_foo", sb_struct_type(), ast::Access::kRead, 0, 0);
    AddStorageBuffer("sb_bar", sb_struct_type(), ast::Access::kRead, 0, 1);
    AddStorageBuffer("sb_baz", sb_struct_type(), ast::Access::kRead, 2, 0);

    auto AddReferenceFunc = [this](const std::string& func_name, const std::string& var_name) {
        MakeStructVariableReferenceBodyFunction(func_name, var_name,
                                                {{0, ty.i32()}, {1, ty.u32()}, {2, ty.f32()}});
    };
    AddReferenceFunc("sb_foo_func", "sb_foo");
    AddReferenceFunc("sb_bar_func", "sb_bar");
    AddReferenceFunc("sb_baz_func", "sb_baz");

    auto FuncCall = [&](const std::string& callee) {
        return create<ast::CallStatement>(Call(callee));
    };

    Func("ep_func", ast::VariableList(), ty.void_(),
         ast::StatementList{
             FuncCall("sb_foo_func"),
             FuncCall("sb_bar_func"),
             FuncCall("sb_baz_func"),
             Return(),
         },
         ast::AttributeList{
             Stage(ast::PipelineStage::kFragment),
         });

    Inspector& inspector = Build();

    auto result = inspector.GetReadOnlyStorageBufferResourceBindings("ep_func");
    ASSERT_FALSE(inspector.has_error()) << inspector.error();
    ASSERT_EQ(3u, result.size());

    EXPECT_EQ(ResourceBinding::ResourceType::kReadOnlyStorageBuffer, result[0].resource_type);
    EXPECT_EQ(0u, result[0].bind_group);
    EXPECT_EQ(0u, result[0].binding);
    EXPECT_EQ(12u, result[0].size);
    EXPECT_EQ(12u, result[0].size_no_padding);

    EXPECT_EQ(ResourceBinding::ResourceType::kReadOnlyStorageBuffer, result[1].resource_type);
    EXPECT_EQ(0u, result[1].bind_group);
    EXPECT_EQ(1u, result[1].binding);
    EXPECT_EQ(12u, result[1].size);
    EXPECT_EQ(12u, result[1].size_no_padding);

    EXPECT_EQ(ResourceBinding::ResourceType::kReadOnlyStorageBuffer, result[2].resource_type);
    EXPECT_EQ(2u, result[2].bind_group);
    EXPECT_EQ(0u, result[2].binding);
    EXPECT_EQ(12u, result[2].size);
    EXPECT_EQ(12u, result[2].size_no_padding);
}

TEST_F(InspectorGetReadOnlyStorageBufferResourceBindingsTest, ContainingArray) {
    auto foo_struct_type = MakeStorageBufferTypes("foo_type", {
                                                                  ty.i32(),
                                                                  ty.array<u32, 4>(),
                                                              });
    AddStorageBuffer("foo_sb", foo_struct_type(), ast::Access::kRead, 0, 0);

    MakeStructVariableReferenceBodyFunction("sb_func", "foo_sb", {{0, ty.i32()}});

    MakeCallerBodyFunction("ep_func", {"sb_func"},
                           ast::AttributeList{
                               Stage(ast::PipelineStage::kFragment),
                           });

    Inspector& inspector = Build();

    auto result = inspector.GetReadOnlyStorageBufferResourceBindings("ep_func");
    ASSERT_FALSE(inspector.has_error()) << inspector.error();
    ASSERT_EQ(1u, result.size());

    EXPECT_EQ(ResourceBinding::ResourceType::kReadOnlyStorageBuffer, result[0].resource_type);
    EXPECT_EQ(0u, result[0].bind_group);
    EXPECT_EQ(0u, result[0].binding);
    EXPECT_EQ(20u, result[0].size);
    EXPECT_EQ(20u, result[0].size_no_padding);
}

TEST_F(InspectorGetReadOnlyStorageBufferResourceBindingsTest, ContainingRuntimeArray) {
    auto foo_struct_type = MakeStorageBufferTypes("foo_type", {
                                                                  ty.i32(),
                                                                  ty.array<u32>(),
                                                              });
    AddStorageBuffer("foo_sb", foo_struct_type(), ast::Access::kRead, 0, 0);

    MakeStructVariableReferenceBodyFunction("sb_func", "foo_sb", {{0, ty.i32()}});

    MakeCallerBodyFunction("ep_func", {"sb_func"},
                           ast::AttributeList{
                               Stage(ast::PipelineStage::kFragment),
                           });

    Inspector& inspector = Build();

    auto result = inspector.GetReadOnlyStorageBufferResourceBindings("ep_func");
    ASSERT_FALSE(inspector.has_error()) << inspector.error();
    ASSERT_EQ(1u, result.size());

    EXPECT_EQ(ResourceBinding::ResourceType::kReadOnlyStorageBuffer, result[0].resource_type);
    EXPECT_EQ(0u, result[0].bind_group);
    EXPECT_EQ(0u, result[0].binding);
    EXPECT_EQ(8u, result[0].size);
    EXPECT_EQ(8u, result[0].size_no_padding);
}

TEST_F(InspectorGetReadOnlyStorageBufferResourceBindingsTest, SkipNonReadOnly) {
    auto foo_struct_type = MakeStorageBufferTypes("foo_type", {ty.i32()});
    AddStorageBuffer("foo_sb", foo_struct_type(), ast::Access::kReadWrite, 0, 0);

    MakeStructVariableReferenceBodyFunction("sb_func", "foo_sb", {{0, ty.i32()}});

    MakeCallerBodyFunction("ep_func", {"sb_func"},
                           ast::AttributeList{
                               Stage(ast::PipelineStage::kFragment),
                           });

    Inspector& inspector = Build();

    auto result = inspector.GetReadOnlyStorageBufferResourceBindings("ep_func");
    ASSERT_FALSE(inspector.has_error()) << inspector.error();
    ASSERT_EQ(0u, result.size());
}

TEST_F(InspectorGetSamplerResourceBindingsTest, Simple) {
    auto* sampled_texture_type = ty.sampled_texture(ast::TextureDimension::k1d, ty.f32());
    AddResource("foo_texture", sampled_texture_type, 0, 0);
    AddSampler("foo_sampler", 0, 1);
    AddGlobalVariable("foo_coords", ty.f32());

    MakeSamplerReferenceBodyFunction("ep", "foo_texture", "foo_sampler", "foo_coords", ty.f32(),
                                     ast::AttributeList{
                                         Stage(ast::PipelineStage::kFragment),
                                     });

    Inspector& inspector = Build();

    auto result = inspector.GetSamplerResourceBindings("ep");
    ASSERT_FALSE(inspector.has_error()) << inspector.error();

    EXPECT_EQ(ResourceBinding::ResourceType::kSampler, result[0].resource_type);
    ASSERT_EQ(1u, result.size());
    EXPECT_EQ(0u, result[0].bind_group);
    EXPECT_EQ(1u, result[0].binding);
}

TEST_F(InspectorGetSamplerResourceBindingsTest, NoSampler) {
    MakeEmptyBodyFunction("ep_func", ast::AttributeList{
                                         Stage(ast::PipelineStage::kFragment),
                                     });

    Inspector& inspector = Build();

    auto result = inspector.GetSamplerResourceBindings("ep_func");
    ASSERT_FALSE(inspector.has_error()) << inspector.error();

    ASSERT_EQ(0u, result.size());
}

TEST_F(InspectorGetSamplerResourceBindingsTest, InFunction) {
    auto* sampled_texture_type = ty.sampled_texture(ast::TextureDimension::k1d, ty.f32());
    AddResource("foo_texture", sampled_texture_type, 0, 0);
    AddSampler("foo_sampler", 0, 1);
    AddGlobalVariable("foo_coords", ty.f32());

    MakeSamplerReferenceBodyFunction("foo_func", "foo_texture", "foo_sampler", "foo_coords",
                                     ty.f32(), {});

    MakeCallerBodyFunction("ep_func", {"foo_func"},
                           ast::AttributeList{
                               Stage(ast::PipelineStage::kFragment),
                           });

    Inspector& inspector = Build();

    auto result = inspector.GetSamplerResourceBindings("ep_func");
    ASSERT_FALSE(inspector.has_error()) << inspector.error();

    EXPECT_EQ(ResourceBinding::ResourceType::kSampler, result[0].resource_type);
    ASSERT_EQ(1u, result.size());
    EXPECT_EQ(0u, result[0].bind_group);
    EXPECT_EQ(1u, result[0].binding);
}

TEST_F(InspectorGetSamplerResourceBindingsTest, UnknownEntryPoint) {
    auto* sampled_texture_type = ty.sampled_texture(ast::TextureDimension::k1d, ty.f32());
    AddResource("foo_texture", sampled_texture_type, 0, 0);
    AddSampler("foo_sampler", 0, 1);
    AddGlobalVariable("foo_coords", ty.f32());

    MakeSamplerReferenceBodyFunction("ep", "foo_texture", "foo_sampler", "foo_coords", ty.f32(),
                                     ast::AttributeList{
                                         Stage(ast::PipelineStage::kFragment),
                                     });

    Inspector& inspector = Build();

    auto result = inspector.GetSamplerResourceBindings("foo");
    ASSERT_TRUE(inspector.has_error()) << inspector.error();
}

TEST_F(InspectorGetSamplerResourceBindingsTest, SkipsComparisonSamplers) {
    auto* depth_texture_type = ty.depth_texture(ast::TextureDimension::k2d);
    AddResource("foo_texture", depth_texture_type, 0, 0);
    AddComparisonSampler("foo_sampler", 0, 1);
    AddGlobalVariable("foo_coords", ty.vec2<f32>());
    AddGlobalVariable("foo_depth", ty.f32());

    MakeComparisonSamplerReferenceBodyFunction("ep", "foo_texture", "foo_sampler", "foo_coords",
                                               "foo_depth", ty.f32(),
                                               ast::AttributeList{
                                                   Stage(ast::PipelineStage::kFragment),
                                               });

    Inspector& inspector = Build();

    auto result = inspector.GetSamplerResourceBindings("ep");
    ASSERT_FALSE(inspector.has_error()) << inspector.error();

    ASSERT_EQ(0u, result.size());
}

TEST_F(InspectorGetComparisonSamplerResourceBindingsTest, Simple) {
    auto* depth_texture_type = ty.depth_texture(ast::TextureDimension::k2d);
    AddResource("foo_texture", depth_texture_type, 0, 0);
    AddComparisonSampler("foo_sampler", 0, 1);
    AddGlobalVariable("foo_coords", ty.vec2<f32>());
    AddGlobalVariable("foo_depth", ty.f32());

    MakeComparisonSamplerReferenceBodyFunction("ep", "foo_texture", "foo_sampler", "foo_coords",
                                               "foo_depth", ty.f32(),
                                               ast::AttributeList{
                                                   Stage(ast::PipelineStage::kFragment),
                                               });

    Inspector& inspector = Build();

    auto result = inspector.GetComparisonSamplerResourceBindings("ep");
    ASSERT_FALSE(inspector.has_error()) << inspector.error();

    EXPECT_EQ(ResourceBinding::ResourceType::kComparisonSampler, result[0].resource_type);
    ASSERT_EQ(1u, result.size());
    EXPECT_EQ(0u, result[0].bind_group);
    EXPECT_EQ(1u, result[0].binding);
}

TEST_F(InspectorGetComparisonSamplerResourceBindingsTest, NoSampler) {
    MakeEmptyBodyFunction("ep_func", ast::AttributeList{
                                         Stage(ast::PipelineStage::kFragment),
                                     });

    Inspector& inspector = Build();

    auto result = inspector.GetComparisonSamplerResourceBindings("ep_func");
    ASSERT_FALSE(inspector.has_error()) << inspector.error();

    ASSERT_EQ(0u, result.size());
}

TEST_F(InspectorGetComparisonSamplerResourceBindingsTest, InFunction) {
    auto* depth_texture_type = ty.depth_texture(ast::TextureDimension::k2d);
    AddResource("foo_texture", depth_texture_type, 0, 0);
    AddComparisonSampler("foo_sampler", 0, 1);
    AddGlobalVariable("foo_coords", ty.vec2<f32>());
    AddGlobalVariable("foo_depth", ty.f32());

    MakeComparisonSamplerReferenceBodyFunction("foo_func", "foo_texture", "foo_sampler",
                                               "foo_coords", "foo_depth", ty.f32(), {});

    MakeCallerBodyFunction("ep_func", {"foo_func"},
                           ast::AttributeList{
                               Stage(ast::PipelineStage::kFragment),
                           });

    Inspector& inspector = Build();

    auto result = inspector.GetComparisonSamplerResourceBindings("ep_func");
    ASSERT_FALSE(inspector.has_error()) << inspector.error();

    EXPECT_EQ(ResourceBinding::ResourceType::kComparisonSampler, result[0].resource_type);
    ASSERT_EQ(1u, result.size());
    EXPECT_EQ(0u, result[0].bind_group);
    EXPECT_EQ(1u, result[0].binding);
}

TEST_F(InspectorGetComparisonSamplerResourceBindingsTest, UnknownEntryPoint) {
    auto* depth_texture_type = ty.depth_texture(ast::TextureDimension::k2d);
    AddResource("foo_texture", depth_texture_type, 0, 0);
    AddComparisonSampler("foo_sampler", 0, 1);
    AddGlobalVariable("foo_coords", ty.vec2<f32>());
    AddGlobalVariable("foo_depth", ty.f32());

    MakeComparisonSamplerReferenceBodyFunction("ep", "foo_texture", "foo_sampler", "foo_coords",
                                               "foo_depth", ty.f32(),
                                               ast::AttributeList{
                                                   Stage(ast::PipelineStage::kFragment),
                                               });

    Inspector& inspector = Build();

    auto result = inspector.GetSamplerResourceBindings("foo");
    ASSERT_TRUE(inspector.has_error()) << inspector.error();
}

TEST_F(InspectorGetComparisonSamplerResourceBindingsTest, SkipsSamplers) {
    auto* sampled_texture_type = ty.sampled_texture(ast::TextureDimension::k1d, ty.f32());
    AddResource("foo_texture", sampled_texture_type, 0, 0);
    AddSampler("foo_sampler", 0, 1);
    AddGlobalVariable("foo_coords", ty.f32());

    MakeSamplerReferenceBodyFunction("ep", "foo_texture", "foo_sampler", "foo_coords", ty.f32(),
                                     ast::AttributeList{
                                         Stage(ast::PipelineStage::kFragment),
                                     });

    Inspector& inspector = Build();

    auto result = inspector.GetComparisonSamplerResourceBindings("ep");
    ASSERT_FALSE(inspector.has_error()) << inspector.error();

    ASSERT_EQ(0u, result.size());
}

TEST_F(InspectorGetSampledTextureResourceBindingsTest, Empty) {
    MakeEmptyBodyFunction("foo", ast::AttributeList{
                                     Stage(ast::PipelineStage::kFragment),
                                 });

    Inspector& inspector = Build();

    auto result = inspector.GetSampledTextureResourceBindings("foo");
    ASSERT_FALSE(inspector.has_error()) << inspector.error();

    EXPECT_EQ(0u, result.size());
}

TEST_P(InspectorGetSampledTextureResourceBindingsTestWithParam, textureSample) {
    auto* sampled_texture_type =
        ty.sampled_texture(GetParam().type_dim, GetBaseType(GetParam().sampled_kind));
    AddResource("foo_texture", sampled_texture_type, 0, 0);
    AddSampler("foo_sampler", 0, 1);
    auto* coord_type = GetCoordsType(GetParam().type_dim, ty.f32());
    AddGlobalVariable("foo_coords", coord_type);

    MakeSamplerReferenceBodyFunction("ep", "foo_texture", "foo_sampler", "foo_coords",
                                     GetBaseType(GetParam().sampled_kind),
                                     ast::AttributeList{
                                         Stage(ast::PipelineStage::kFragment),
                                     });

    Inspector& inspector = Build();

    auto result = inspector.GetSampledTextureResourceBindings("ep");
    ASSERT_FALSE(inspector.has_error()) << inspector.error();

    EXPECT_EQ(ResourceBinding::ResourceType::kSampledTexture, result[0].resource_type);
    ASSERT_EQ(1u, result.size());
    EXPECT_EQ(0u, result[0].bind_group);
    EXPECT_EQ(0u, result[0].binding);
    EXPECT_EQ(GetParam().inspector_dim, result[0].dim);
    EXPECT_EQ(GetParam().sampled_kind, result[0].sampled_kind);

    // Prove that sampled and multi-sampled bindings are accounted
    // for separately.
    auto multisampled_result = inspector.GetMultisampledTextureResourceBindings("ep");
    ASSERT_FALSE(inspector.has_error()) << inspector.error();
    ASSERT_TRUE(multisampled_result.empty());
}

INSTANTIATE_TEST_SUITE_P(
    InspectorGetSampledTextureResourceBindingsTest,
    InspectorGetSampledTextureResourceBindingsTestWithParam,
    testing::Values(GetSampledTextureTestParams{ast::TextureDimension::k1d,
                                                inspector::ResourceBinding::TextureDimension::k1d,
                                                inspector::ResourceBinding::SampledKind::kFloat},
                    GetSampledTextureTestParams{ast::TextureDimension::k2d,
                                                inspector::ResourceBinding::TextureDimension::k2d,
                                                inspector::ResourceBinding::SampledKind::kFloat},
                    GetSampledTextureTestParams{ast::TextureDimension::k3d,
                                                inspector::ResourceBinding::TextureDimension::k3d,
                                                inspector::ResourceBinding::SampledKind::kFloat},
                    GetSampledTextureTestParams{ast::TextureDimension::kCube,
                                                inspector::ResourceBinding::TextureDimension::kCube,
                                                inspector::ResourceBinding::SampledKind::kFloat}));

TEST_P(InspectorGetSampledArrayTextureResourceBindingsTestWithParam, textureSample) {
    auto* sampled_texture_type =
        ty.sampled_texture(GetParam().type_dim, GetBaseType(GetParam().sampled_kind));
    AddResource("foo_texture", sampled_texture_type, 0, 0);
    AddSampler("foo_sampler", 0, 1);
    auto* coord_type = GetCoordsType(GetParam().type_dim, ty.f32());
    AddGlobalVariable("foo_coords", coord_type);
    AddGlobalVariable("foo_array_index", ty.i32());

    MakeSamplerReferenceBodyFunction("ep", "foo_texture", "foo_sampler", "foo_coords",
                                     "foo_array_index", GetBaseType(GetParam().sampled_kind),
                                     ast::AttributeList{
                                         Stage(ast::PipelineStage::kFragment),
                                     });

    Inspector& inspector = Build();

    auto result = inspector.GetSampledTextureResourceBindings("ep");
    ASSERT_FALSE(inspector.has_error()) << inspector.error();
    ASSERT_EQ(1u, result.size());

    EXPECT_EQ(ResourceBinding::ResourceType::kSampledTexture, result[0].resource_type);
    EXPECT_EQ(0u, result[0].bind_group);
    EXPECT_EQ(0u, result[0].binding);
    EXPECT_EQ(GetParam().inspector_dim, result[0].dim);
    EXPECT_EQ(GetParam().sampled_kind, result[0].sampled_kind);
}

INSTANTIATE_TEST_SUITE_P(
    InspectorGetSampledArrayTextureResourceBindingsTest,
    InspectorGetSampledArrayTextureResourceBindingsTestWithParam,
    testing::Values(
        GetSampledTextureTestParams{ast::TextureDimension::k2dArray,
                                    inspector::ResourceBinding::TextureDimension::k2dArray,
                                    inspector::ResourceBinding::SampledKind::kFloat},
        GetSampledTextureTestParams{ast::TextureDimension::kCubeArray,
                                    inspector::ResourceBinding::TextureDimension::kCubeArray,
                                    inspector::ResourceBinding::SampledKind::kFloat}));

TEST_P(InspectorGetMultisampledTextureResourceBindingsTestWithParam, textureLoad) {
    auto* multisampled_texture_type =
        ty.multisampled_texture(GetParam().type_dim, GetBaseType(GetParam().sampled_kind));
    AddResource("foo_texture", multisampled_texture_type, 0, 0);
    auto* coord_type = GetCoordsType(GetParam().type_dim, ty.i32());
    AddGlobalVariable("foo_coords", coord_type);
    AddGlobalVariable("foo_sample_index", ty.i32());

    Func("ep", ast::VariableList(), ty.void_(),
         ast::StatementList{
             CallStmt(Call("textureLoad", "foo_texture", "foo_coords", "foo_sample_index")),
         },
         ast::AttributeList{
             Stage(ast::PipelineStage::kFragment),
         });

    Inspector& inspector = Build();

    auto result = inspector.GetMultisampledTextureResourceBindings("ep");
    ASSERT_FALSE(inspector.has_error()) << inspector.error();

    ASSERT_EQ(1u, result.size());
    EXPECT_EQ(ResourceBinding::ResourceType::kMultisampledTexture, result[0].resource_type);
    EXPECT_EQ(0u, result[0].bind_group);
    EXPECT_EQ(0u, result[0].binding);
    EXPECT_EQ(GetParam().inspector_dim, result[0].dim);
    EXPECT_EQ(GetParam().sampled_kind, result[0].sampled_kind);

    // Prove that sampled and multi-sampled bindings are accounted
    // for separately.
    auto single_sampled_result = inspector.GetSampledTextureResourceBindings("ep");
    ASSERT_FALSE(inspector.has_error()) << inspector.error();
    ASSERT_TRUE(single_sampled_result.empty());
}

INSTANTIATE_TEST_SUITE_P(
    InspectorGetMultisampledTextureResourceBindingsTest,
    InspectorGetMultisampledTextureResourceBindingsTestWithParam,
    testing::Values(
        GetMultisampledTextureTestParams{ast::TextureDimension::k2d,
                                         inspector::ResourceBinding::TextureDimension::k2d,
                                         inspector::ResourceBinding::SampledKind::kFloat},
        GetMultisampledTextureTestParams{ast::TextureDimension::k2d,
                                         inspector::ResourceBinding::TextureDimension::k2d,
                                         inspector::ResourceBinding::SampledKind::kSInt},
        GetMultisampledTextureTestParams{ast::TextureDimension::k2d,
                                         inspector::ResourceBinding::TextureDimension::k2d,
                                         inspector::ResourceBinding::SampledKind::kUInt}));

TEST_F(InspectorGetMultisampledArrayTextureResourceBindingsTest, Empty) {
    MakeEmptyBodyFunction("foo", ast::AttributeList{
                                     Stage(ast::PipelineStage::kFragment),
                                 });

    Inspector& inspector = Build();

    auto result = inspector.GetSampledTextureResourceBindings("foo");
    ASSERT_FALSE(inspector.has_error()) << inspector.error();

    EXPECT_EQ(0u, result.size());
}

TEST_P(InspectorGetMultisampledArrayTextureResourceBindingsTestWithParam, DISABLED_textureSample) {
    auto* multisampled_texture_type =
        ty.multisampled_texture(GetParam().type_dim, GetBaseType(GetParam().sampled_kind));
    AddResource("foo_texture", multisampled_texture_type, 0, 0);
    AddSampler("foo_sampler", 0, 1);
    auto* coord_type = GetCoordsType(GetParam().type_dim, ty.f32());
    AddGlobalVariable("foo_coords", coord_type);
    AddGlobalVariable("foo_array_index", ty.i32());

    MakeSamplerReferenceBodyFunction("ep", "foo_texture", "foo_sampler", "foo_coords",
                                     "foo_array_index", GetBaseType(GetParam().sampled_kind),
                                     ast::AttributeList{
                                         Stage(ast::PipelineStage::kFragment),
                                     });

    Inspector& inspector = Build();

    auto result = inspector.GetMultisampledTextureResourceBindings("ep");
    ASSERT_FALSE(inspector.has_error()) << inspector.error();
    ASSERT_EQ(1u, result.size());

    EXPECT_EQ(ResourceBinding::ResourceType::kMultisampledTexture, result[0].resource_type);
    EXPECT_EQ(0u, result[0].bind_group);
    EXPECT_EQ(0u, result[0].binding);
    EXPECT_EQ(GetParam().inspector_dim, result[0].dim);
    EXPECT_EQ(GetParam().sampled_kind, result[0].sampled_kind);
}

INSTANTIATE_TEST_SUITE_P(
    InspectorGetMultisampledArrayTextureResourceBindingsTest,
    InspectorGetMultisampledArrayTextureResourceBindingsTestWithParam,
    testing::Values(
        GetMultisampledTextureTestParams{ast::TextureDimension::k2dArray,
                                         inspector::ResourceBinding::TextureDimension::k2dArray,
                                         inspector::ResourceBinding::SampledKind::kFloat},
        GetMultisampledTextureTestParams{ast::TextureDimension::k2dArray,
                                         inspector::ResourceBinding::TextureDimension::k2dArray,
                                         inspector::ResourceBinding::SampledKind::kSInt},
        GetMultisampledTextureTestParams{ast::TextureDimension::k2dArray,
                                         inspector::ResourceBinding::TextureDimension::k2dArray,
                                         inspector::ResourceBinding::SampledKind::kUInt}));

TEST_F(InspectorGetStorageTextureResourceBindingsTest, Empty) {
    MakeEmptyBodyFunction("ep", ast::AttributeList{
                                    Stage(ast::PipelineStage::kFragment),
                                });

    Inspector& inspector = Build();

    auto result = inspector.GetWriteOnlyStorageTextureResourceBindings("ep");
    ASSERT_FALSE(inspector.has_error()) << inspector.error();
    EXPECT_EQ(0u, result.size());
}

TEST_P(InspectorGetStorageTextureResourceBindingsTestWithParam, Simple) {
    DimensionParams dim_params;
    TexelFormatParams format_params;
    std::tie(dim_params, format_params) = GetParam();

    ast::TextureDimension dim;
    ResourceBinding::TextureDimension expected_dim;
    std::tie(dim, expected_dim) = dim_params;

    ast::TexelFormat format;
    ResourceBinding::TexelFormat expected_format;
    ResourceBinding::SampledKind expected_kind;
    std::tie(format, expected_format, expected_kind) = format_params;

    auto* st_type = MakeStorageTextureTypes(dim, format);
    AddStorageTexture("st_var", st_type, 0, 0);

    const ast::Type* dim_type = nullptr;
    switch (dim) {
        case ast::TextureDimension::k1d:
            dim_type = ty.i32();
            break;
        case ast::TextureDimension::k2d:
        case ast::TextureDimension::k2dArray:
            dim_type = ty.vec2<i32>();
            break;
        case ast::TextureDimension::k3d:
            dim_type = ty.vec3<i32>();
            break;
        default:
            break;
    }

    ASSERT_FALSE(dim_type == nullptr);

    MakeStorageTextureBodyFunction("ep", "st_var", dim_type,
                                   ast::AttributeList{Stage(ast::PipelineStage::kFragment)});

    Inspector& inspector = Build();

    auto result = inspector.GetWriteOnlyStorageTextureResourceBindings("ep");
    ASSERT_FALSE(inspector.has_error()) << inspector.error();
    ASSERT_EQ(1u, result.size());

    EXPECT_EQ(ResourceBinding::ResourceType::kWriteOnlyStorageTexture, result[0].resource_type);
    EXPECT_EQ(0u, result[0].bind_group);
    EXPECT_EQ(0u, result[0].binding);
    EXPECT_EQ(expected_dim, result[0].dim);
    EXPECT_EQ(expected_format, result[0].image_format);
    EXPECT_EQ(expected_kind, result[0].sampled_kind);
}

INSTANTIATE_TEST_SUITE_P(
    InspectorGetStorageTextureResourceBindingsTest,
    InspectorGetStorageTextureResourceBindingsTestWithParam,
    testing::Combine(testing::Values(std::make_tuple(ast::TextureDimension::k1d,
                                                     ResourceBinding::TextureDimension::k1d),
                                     std::make_tuple(ast::TextureDimension::k2d,
                                                     ResourceBinding::TextureDimension::k2d),
                                     std::make_tuple(ast::TextureDimension::k2dArray,
                                                     ResourceBinding::TextureDimension::k2dArray),
                                     std::make_tuple(ast::TextureDimension::k3d,
                                                     ResourceBinding::TextureDimension::k3d)),
                     testing::Values(std::make_tuple(ast::TexelFormat::kR32Float,
                                                     ResourceBinding::TexelFormat::kR32Float,
                                                     ResourceBinding::SampledKind::kFloat),
                                     std::make_tuple(ast::TexelFormat::kR32Sint,
                                                     ResourceBinding::TexelFormat::kR32Sint,
                                                     ResourceBinding::SampledKind::kSInt),
                                     std::make_tuple(ast::TexelFormat::kR32Uint,
                                                     ResourceBinding::TexelFormat::kR32Uint,
                                                     ResourceBinding::SampledKind::kUInt),
                                     std::make_tuple(ast::TexelFormat::kRg32Float,
                                                     ResourceBinding::TexelFormat::kRg32Float,
                                                     ResourceBinding::SampledKind::kFloat),
                                     std::make_tuple(ast::TexelFormat::kRg32Sint,
                                                     ResourceBinding::TexelFormat::kRg32Sint,
                                                     ResourceBinding::SampledKind::kSInt),
                                     std::make_tuple(ast::TexelFormat::kRg32Uint,
                                                     ResourceBinding::TexelFormat::kRg32Uint,
                                                     ResourceBinding::SampledKind::kUInt),
                                     std::make_tuple(ast::TexelFormat::kRgba16Float,
                                                     ResourceBinding::TexelFormat::kRgba16Float,
                                                     ResourceBinding::SampledKind::kFloat),
                                     std::make_tuple(ast::TexelFormat::kRgba16Sint,
                                                     ResourceBinding::TexelFormat::kRgba16Sint,
                                                     ResourceBinding::SampledKind::kSInt),
                                     std::make_tuple(ast::TexelFormat::kRgba16Uint,
                                                     ResourceBinding::TexelFormat::kRgba16Uint,
                                                     ResourceBinding::SampledKind::kUInt),
                                     std::make_tuple(ast::TexelFormat::kRgba32Float,
                                                     ResourceBinding::TexelFormat::kRgba32Float,
                                                     ResourceBinding::SampledKind::kFloat),
                                     std::make_tuple(ast::TexelFormat::kRgba32Sint,
                                                     ResourceBinding::TexelFormat::kRgba32Sint,
                                                     ResourceBinding::SampledKind::kSInt),
                                     std::make_tuple(ast::TexelFormat::kRgba32Uint,
                                                     ResourceBinding::TexelFormat::kRgba32Uint,
                                                     ResourceBinding::SampledKind::kUInt),
                                     std::make_tuple(ast::TexelFormat::kRgba8Sint,
                                                     ResourceBinding::TexelFormat::kRgba8Sint,
                                                     ResourceBinding::SampledKind::kSInt),
                                     std::make_tuple(ast::TexelFormat::kRgba8Snorm,
                                                     ResourceBinding::TexelFormat::kRgba8Snorm,
                                                     ResourceBinding::SampledKind::kFloat),
                                     std::make_tuple(ast::TexelFormat::kRgba8Uint,
                                                     ResourceBinding::TexelFormat::kRgba8Uint,
                                                     ResourceBinding::SampledKind::kUInt),
                                     std::make_tuple(ast::TexelFormat::kRgba8Unorm,
                                                     ResourceBinding::TexelFormat::kRgba8Unorm,
                                                     ResourceBinding::SampledKind::kFloat))));

TEST_P(InspectorGetDepthTextureResourceBindingsTestWithParam, textureDimensions) {
    auto* depth_texture_type = ty.depth_texture(GetParam().type_dim);
    AddResource("dt", depth_texture_type, 0, 0);

    Func("ep", ast::VariableList(), ty.void_(),
         ast::StatementList{
             CallStmt(Call("textureDimensions", "dt")),
         },
         ast::AttributeList{
             Stage(ast::PipelineStage::kFragment),
         });

    Inspector& inspector = Build();

    auto result = inspector.GetDepthTextureResourceBindings("ep");
    ASSERT_FALSE(inspector.has_error()) << inspector.error();

    EXPECT_EQ(ResourceBinding::ResourceType::kDepthTexture, result[0].resource_type);
    ASSERT_EQ(1u, result.size());
    EXPECT_EQ(0u, result[0].bind_group);
    EXPECT_EQ(0u, result[0].binding);
    EXPECT_EQ(GetParam().inspector_dim, result[0].dim);
}

INSTANTIATE_TEST_SUITE_P(
    InspectorGetDepthTextureResourceBindingsTest,
    InspectorGetDepthTextureResourceBindingsTestWithParam,
    testing::Values(
        GetDepthTextureTestParams{ast::TextureDimension::k2d,
                                  inspector::ResourceBinding::TextureDimension::k2d},
        GetDepthTextureTestParams{ast::TextureDimension::k2dArray,
                                  inspector::ResourceBinding::TextureDimension::k2dArray},
        GetDepthTextureTestParams{ast::TextureDimension::kCube,
                                  inspector::ResourceBinding::TextureDimension::kCube},
        GetDepthTextureTestParams{ast::TextureDimension::kCubeArray,
                                  inspector::ResourceBinding::TextureDimension::kCubeArray}));

TEST_F(InspectorGetDepthMultisampledTextureResourceBindingsTest, textureDimensions) {
    auto* depth_ms_texture_type = ty.depth_multisampled_texture(ast::TextureDimension::k2d);
    AddResource("tex", depth_ms_texture_type, 0, 0);

    Func("ep", ast::VariableList(), ty.void_(),
         ast::StatementList{
             CallStmt(Call("textureDimensions", "tex")),
         },
         ast::AttributeList{
             Stage(ast::PipelineStage::kFragment),
         });

    Inspector& inspector = Build();

    auto result = inspector.GetDepthMultisampledTextureResourceBindings("ep");
    ASSERT_FALSE(inspector.has_error()) << inspector.error();

    EXPECT_EQ(ResourceBinding::ResourceType::kDepthMultisampledTexture, result[0].resource_type);
    ASSERT_EQ(1u, result.size());
    EXPECT_EQ(0u, result[0].bind_group);
    EXPECT_EQ(0u, result[0].binding);
    EXPECT_EQ(ResourceBinding::TextureDimension::k2d, result[0].dim);
}

TEST_F(InspectorGetExternalTextureResourceBindingsTest, Simple) {
    auto* external_texture_type = ty.external_texture();
    AddResource("et", external_texture_type, 0, 0);

    Func("ep", ast::VariableList(), ty.void_(),
         ast::StatementList{
             CallStmt(Call("textureDimensions", "et")),
         },
         ast::AttributeList{
             Stage(ast::PipelineStage::kFragment),
         });

    Inspector& inspector = Build();

    auto result = inspector.GetExternalTextureResourceBindings("ep");
    ASSERT_FALSE(inspector.has_error()) << inspector.error();
    EXPECT_EQ(ResourceBinding::ResourceType::kExternalTexture, result[0].resource_type);

    ASSERT_EQ(1u, result.size());
    EXPECT_EQ(0u, result[0].bind_group);
    EXPECT_EQ(0u, result[0].binding);
}

TEST_F(InspectorGetSamplerTextureUsesTest, None) {
    std::string shader = R"(
@stage(fragment)
fn main() {
})";

    Inspector& inspector = Initialize(shader);
    auto result = inspector.GetSamplerTextureUses("main");
    ASSERT_FALSE(inspector.has_error()) << inspector.error();

    ASSERT_EQ(0u, result.size());
}

TEST_F(InspectorGetSamplerTextureUsesTest, Simple) {
    std::string shader = R"(
@group(0) @binding(1) var mySampler: sampler;
@group(0) @binding(2) var myTexture: texture_2d<f32>;

@stage(fragment)
fn main(@location(0) fragUV: vec2<f32>,
        @location(1) fragPosition: vec4<f32>) -> @location(0) vec4<f32> {
  return textureSample(myTexture, mySampler, fragUV) * fragPosition;
})";

    Inspector& inspector = Initialize(shader);
    auto result = inspector.GetSamplerTextureUses("main");
    ASSERT_FALSE(inspector.has_error()) << inspector.error();

    ASSERT_EQ(1u, result.size());

    EXPECT_EQ(0u, result[0].sampler_binding_point.group);
    EXPECT_EQ(1u, result[0].sampler_binding_point.binding);
    EXPECT_EQ(0u, result[0].texture_binding_point.group);
    EXPECT_EQ(2u, result[0].texture_binding_point.binding);
}

TEST_F(InspectorGetSamplerTextureUsesTest, UnknownEntryPoint) {
    std::string shader = R"(
@group(0) @binding(1) var mySampler: sampler;
@group(0) @binding(2) var myTexture: texture_2d<f32>;

@stage(fragment)
fn main(@location(0) fragUV: vec2<f32>,
        @location(1) fragPosition: vec4<f32>) -> @location(0) vec4<f32> {
  return textureSample(myTexture, mySampler, fragUV) * fragPosition;
})";

    Inspector& inspector = Initialize(shader);
    auto result = inspector.GetSamplerTextureUses("foo");
    ASSERT_TRUE(inspector.has_error()) << inspector.error();
}

TEST_F(InspectorGetSamplerTextureUsesTest, MultipleCalls) {
    std::string shader = R"(
@group(0) @binding(1) var mySampler: sampler;
@group(0) @binding(2) var myTexture: texture_2d<f32>;

@stage(fragment)
fn main(@location(0) fragUV: vec2<f32>,
        @location(1) fragPosition: vec4<f32>) -> @location(0) vec4<f32> {
  return textureSample(myTexture, mySampler, fragUV) * fragPosition;
})";

    Inspector& inspector = Initialize(shader);
    auto result_0 = inspector.GetSamplerTextureUses("main");
    ASSERT_FALSE(inspector.has_error()) << inspector.error();

    auto result_1 = inspector.GetSamplerTextureUses("main");
    ASSERT_FALSE(inspector.has_error()) << inspector.error();

    EXPECT_EQ(result_0, result_1);
}

TEST_F(InspectorGetSamplerTextureUsesTest, BothIndirect) {
    std::string shader = R"(
@group(0) @binding(1) var mySampler: sampler;
@group(0) @binding(2) var myTexture: texture_2d<f32>;

fn doSample(t: texture_2d<f32>, s: sampler, uv: vec2<f32>) -> vec4<f32> {
  return textureSample(t, s, uv);
}

@stage(fragment)
fn main(@location(0) fragUV: vec2<f32>,
        @location(1) fragPosition: vec4<f32>) -> @location(0) vec4<f32> {
  return doSample(myTexture, mySampler, fragUV) * fragPosition;
})";

    Inspector& inspector = Initialize(shader);
    auto result = inspector.GetSamplerTextureUses("main");
    ASSERT_FALSE(inspector.has_error()) << inspector.error();

    ASSERT_EQ(1u, result.size());

    EXPECT_EQ(0u, result[0].sampler_binding_point.group);
    EXPECT_EQ(1u, result[0].sampler_binding_point.binding);
    EXPECT_EQ(0u, result[0].texture_binding_point.group);
    EXPECT_EQ(2u, result[0].texture_binding_point.binding);
}

TEST_F(InspectorGetSamplerTextureUsesTest, SamplerIndirect) {
    std::string shader = R"(
@group(0) @binding(1) var mySampler: sampler;
@group(0) @binding(2) var myTexture: texture_2d<f32>;

fn doSample(s: sampler, uv: vec2<f32>) -> vec4<f32> {
  return textureSample(myTexture, s, uv);
}

@stage(fragment)
fn main(@location(0) fragUV: vec2<f32>,
        @location(1) fragPosition: vec4<f32>) -> @location(0) vec4<f32> {
  return doSample(mySampler, fragUV) * fragPosition;
})";

    Inspector& inspector = Initialize(shader);
    auto result = inspector.GetSamplerTextureUses("main");
    ASSERT_FALSE(inspector.has_error()) << inspector.error();

    ASSERT_EQ(1u, result.size());

    EXPECT_EQ(0u, result[0].sampler_binding_point.group);
    EXPECT_EQ(1u, result[0].sampler_binding_point.binding);
    EXPECT_EQ(0u, result[0].texture_binding_point.group);
    EXPECT_EQ(2u, result[0].texture_binding_point.binding);
}

TEST_F(InspectorGetSamplerTextureUsesTest, TextureIndirect) {
    std::string shader = R"(
@group(0) @binding(1) var mySampler: sampler;
@group(0) @binding(2) var myTexture: texture_2d<f32>;

fn doSample(t: texture_2d<f32>, uv: vec2<f32>) -> vec4<f32> {
  return textureSample(t, mySampler, uv);
}

@stage(fragment)
fn main(@location(0) fragUV: vec2<f32>,
        @location(1) fragPosition: vec4<f32>) -> @location(0) vec4<f32> {
  return doSample(myTexture, fragUV) * fragPosition;
})";

    Inspector& inspector = Initialize(shader);
    auto result = inspector.GetSamplerTextureUses("main");
    ASSERT_FALSE(inspector.has_error()) << inspector.error();

    ASSERT_EQ(1u, result.size());

    EXPECT_EQ(0u, result[0].sampler_binding_point.group);
    EXPECT_EQ(1u, result[0].sampler_binding_point.binding);
    EXPECT_EQ(0u, result[0].texture_binding_point.group);
    EXPECT_EQ(2u, result[0].texture_binding_point.binding);
}

TEST_F(InspectorGetSamplerTextureUsesTest, NeitherIndirect) {
    std::string shader = R"(
@group(0) @binding(1) var mySampler: sampler;
@group(0) @binding(2) var myTexture: texture_2d<f32>;

fn doSample(uv: vec2<f32>) -> vec4<f32> {
  return textureSample(myTexture, mySampler, uv);
}

@stage(fragment)
fn main(@location(0) fragUV: vec2<f32>,
        @location(1) fragPosition: vec4<f32>) -> @location(0) vec4<f32> {
  return doSample(fragUV) * fragPosition;
})";

    Inspector& inspector = Initialize(shader);
    auto result = inspector.GetSamplerTextureUses("main");
    ASSERT_FALSE(inspector.has_error()) << inspector.error();

    ASSERT_EQ(1u, result.size());

    EXPECT_EQ(0u, result[0].sampler_binding_point.group);
    EXPECT_EQ(1u, result[0].sampler_binding_point.binding);
    EXPECT_EQ(0u, result[0].texture_binding_point.group);
    EXPECT_EQ(2u, result[0].texture_binding_point.binding);
}

TEST_F(InspectorGetSamplerTextureUsesTest, Complex) {
    std::string shader = R"(
@group(0) @binding(1) var mySampler: sampler;
@group(0) @binding(2) var myTexture: texture_2d<f32>;


fn doSample(t: texture_2d<f32>, s: sampler, uv: vec2<f32>) -> vec4<f32> {
  return textureSample(t, s, uv);
}

fn X(t: texture_2d<f32>, s: sampler, uv: vec2<f32>) -> vec4<f32> {
  return doSample(t, s, uv);
}

fn Y(t: texture_2d<f32>, s: sampler, uv: vec2<f32>) -> vec4<f32> {
  return doSample(t, s, uv);
}

fn Z(t: texture_2d<f32>, s: sampler, uv: vec2<f32>) -> vec4<f32> {
  return X(t, s, uv) + Y(t, s, uv);
}

@stage(fragment)
fn via_call(@location(0) fragUV: vec2<f32>,
        @location(1) fragPosition: vec4<f32>) -> @location(0) vec4<f32> {
  return Z(myTexture, mySampler, fragUV) * fragPosition;
}

@stage(fragment)
fn via_ptr(@location(0) fragUV: vec2<f32>,
        @location(1) fragPosition: vec4<f32>) -> @location(0) vec4<f32> {
  return textureSample(myTexture, mySampler, fragUV) + fragPosition;
}

@stage(fragment)
fn direct(@location(0) fragUV: vec2<f32>,
        @location(1) fragPosition: vec4<f32>) -> @location(0) vec4<f32> {
  return textureSample(myTexture, mySampler, fragUV) + fragPosition;
})";

    Inspector& inspector = Initialize(shader);

    {
        auto result = inspector.GetSamplerTextureUses("via_call");
        ASSERT_FALSE(inspector.has_error()) << inspector.error();

        ASSERT_EQ(1u, result.size());

        EXPECT_EQ(0u, result[0].sampler_binding_point.group);
        EXPECT_EQ(1u, result[0].sampler_binding_point.binding);
        EXPECT_EQ(0u, result[0].texture_binding_point.group);
        EXPECT_EQ(2u, result[0].texture_binding_point.binding);
    }

    {
        auto result = inspector.GetSamplerTextureUses("via_ptr");
        ASSERT_FALSE(inspector.has_error()) << inspector.error();

        ASSERT_EQ(1u, result.size());

        EXPECT_EQ(0u, result[0].sampler_binding_point.group);
        EXPECT_EQ(1u, result[0].sampler_binding_point.binding);
        EXPECT_EQ(0u, result[0].texture_binding_point.group);
        EXPECT_EQ(2u, result[0].texture_binding_point.binding);
    }

    {
        auto result = inspector.GetSamplerTextureUses("direct");
        ASSERT_FALSE(inspector.has_error()) << inspector.error();

        ASSERT_EQ(1u, result.size());

        EXPECT_EQ(0u, result[0].sampler_binding_point.group);
        EXPECT_EQ(1u, result[0].sampler_binding_point.binding);
        EXPECT_EQ(0u, result[0].texture_binding_point.group);
        EXPECT_EQ(2u, result[0].texture_binding_point.binding);
    }
}

TEST_F(InspectorGetWorkgroupStorageSizeTest, Empty) {
    MakeEmptyBodyFunction(
        "ep_func", ast::AttributeList{Stage(ast::PipelineStage::kCompute), WorkgroupSize(1_i)});
    Inspector& inspector = Build();
    EXPECT_EQ(0u, inspector.GetWorkgroupStorageSize("ep_func"));
}

TEST_F(InspectorGetWorkgroupStorageSizeTest, Simple) {
    AddWorkgroupStorage("wg_f32", ty.f32());
    MakePlainGlobalReferenceBodyFunction("f32_func", "wg_f32", ty.f32(), {});

    MakeCallerBodyFunction("ep_func", {"f32_func"},
                           ast::AttributeList{
                               Stage(ast::PipelineStage::kCompute),
                               WorkgroupSize(1_i),
                           });

    Inspector& inspector = Build();
    EXPECT_EQ(4u, inspector.GetWorkgroupStorageSize("ep_func"));
}

TEST_F(InspectorGetWorkgroupStorageSizeTest, CompoundTypes) {
    // This struct should occupy 68 bytes. 4 from the i32 field, and another 64
    // from the 4-element array with 16-byte stride.
    auto* wg_struct_type =
        MakeStructType("WgStruct", {ty.i32(), ty.array(ty.i32(), 4_u, /*stride=*/16)});
    AddWorkgroupStorage("wg_struct_var", ty.Of(wg_struct_type));
    MakeStructVariableReferenceBodyFunction("wg_struct_func", "wg_struct_var", {{0, ty.i32()}});

    // Plus another 4 bytes from this other workgroup-class f32.
    AddWorkgroupStorage("wg_f32", ty.f32());
    MakePlainGlobalReferenceBodyFunction("f32_func", "wg_f32", ty.f32(), {});

    MakeCallerBodyFunction("ep_func", {"wg_struct_func", "f32_func"},
                           ast::AttributeList{
                               Stage(ast::PipelineStage::kCompute),
                               WorkgroupSize(1_i),
                           });

    Inspector& inspector = Build();
    EXPECT_EQ(72u, inspector.GetWorkgroupStorageSize("ep_func"));
}

TEST_F(InspectorGetWorkgroupStorageSizeTest, AlignmentPadding) {
    // vec3<f32> has an alignment of 16 but a size of 12. We leverage this to test
    // that our padded size calculation for workgroup storage is accurate.
    AddWorkgroupStorage("wg_vec3", ty.vec3<f32>());
    MakePlainGlobalReferenceBodyFunction("wg_func", "wg_vec3", ty.vec3<f32>(), {});

    MakeCallerBodyFunction("ep_func", {"wg_func"},
                           ast::AttributeList{
                               Stage(ast::PipelineStage::kCompute),
                               WorkgroupSize(1_i),
                           });

    Inspector& inspector = Build();
    EXPECT_EQ(16u, inspector.GetWorkgroupStorageSize("ep_func"));
}

TEST_F(InspectorGetWorkgroupStorageSizeTest, StructAlignment) {
    // Per WGSL spec, a struct's size is the offset its last member plus the size
    // of its last member, rounded up to the alignment of its largest member. So
    // here the struct is expected to occupy 1024 bytes of workgroup storage.
    const auto* wg_struct_type = MakeStructTypeFromMembers(
        "WgStruct",
        {MakeStructMember(0, ty.f32(), {create<ast::StructMemberAlignAttribute>(1024)})});

    AddWorkgroupStorage("wg_struct_var", ty.Of(wg_struct_type));
    MakeStructVariableReferenceBodyFunction("wg_struct_func", "wg_struct_var", {{0, ty.f32()}});

    MakeCallerBodyFunction("ep_func", {"wg_struct_func"},
                           ast::AttributeList{
                               Stage(ast::PipelineStage::kCompute),
                               WorkgroupSize(1_i),
                           });

    Inspector& inspector = Build();
    EXPECT_EQ(1024u, inspector.GetWorkgroupStorageSize("ep_func"));
}

// Test calling GetUsedExtensionNames on a empty shader.
TEST_F(InspectorGetUsedExtensionNamesTest, Empty) {
    std::string shader = "";

    Inspector& inspector = Initialize(shader);

    auto result = inspector.GetUsedExtensionNames();
    EXPECT_EQ(result.size(), 0u);
}

// Test calling GetUsedExtensionNames on a shader with no extension.
TEST_F(InspectorGetUsedExtensionNamesTest, None) {
    std::string shader = R"(
@stage(fragment)
fn main() {
})";

    Inspector& inspector = Initialize(shader);

    auto result = inspector.GetUsedExtensionNames();
    EXPECT_EQ(result.size(), 0u);
}

// Test calling GetUsedExtensionNames on a shader with valid extension.
TEST_F(InspectorGetUsedExtensionNamesTest, Simple) {
    std::string shader = R"(
enable InternalExtensionForTesting;

@stage(fragment)
fn main() {
})";

    Inspector& inspector = Initialize(shader);

    auto result = inspector.GetUsedExtensionNames();
    EXPECT_EQ(result.size(), 1u);
    EXPECT_EQ(result[0], "InternalExtensionForTesting");
}

// Test calling GetUsedExtensionNames on a shader with a extension enabled for
// multiple times.
TEST_F(InspectorGetUsedExtensionNamesTest, Duplicated) {
    std::string shader = R"(
enable InternalExtensionForTesting;
enable InternalExtensionForTesting;

@stage(fragment)
fn main() {
})";

    Inspector& inspector = Initialize(shader);

    auto result = inspector.GetUsedExtensionNames();
    EXPECT_EQ(result.size(), 1u);
    EXPECT_EQ(result[0], "InternalExtensionForTesting");
}

// Test calling GetEnableDirectives on a empty shader.
TEST_F(InspectorGetEnableDirectivesTest, Empty) {
    std::string shader = "";

    Inspector& inspector = Initialize(shader);

    auto result = inspector.GetEnableDirectives();
    EXPECT_EQ(result.size(), 0u);
}

// Test calling GetEnableDirectives on a shader with no extension.
TEST_F(InspectorGetEnableDirectivesTest, None) {
    std::string shader = R"(
@stage(fragment)
fn main() {
})";

    Inspector& inspector = Initialize(shader);

    auto result = inspector.GetEnableDirectives();
    EXPECT_EQ(result.size(), 0u);
}

// Test calling GetEnableDirectives on a shader with valid extension.
TEST_F(InspectorGetEnableDirectivesTest, Simple) {
    std::string shader = R"(
enable InternalExtensionForTesting;

@stage(fragment)
fn main() {
})";

    Inspector& inspector = Initialize(shader);

    auto result = inspector.GetEnableDirectives();
    EXPECT_EQ(result.size(), 1u);
    EXPECT_EQ(result[0].first, "InternalExtensionForTesting");
    EXPECT_EQ(result[0].second.range, (Source::Range{{2, 8}, {2, 35}}));
}

// Test calling GetEnableDirectives on a shader with a extension enabled for
// multiple times.
TEST_F(InspectorGetEnableDirectivesTest, Duplicated) {
    std::string shader = R"(
enable InternalExtensionForTesting;

enable InternalExtensionForTesting;
@stage(fragment)
fn main() {
})";

    Inspector& inspector = Initialize(shader);

    auto result = inspector.GetEnableDirectives();
    EXPECT_EQ(result.size(), 2u);
    EXPECT_EQ(result[0].first, "InternalExtensionForTesting");
    EXPECT_EQ(result[0].second.range, (Source::Range{{2, 8}, {2, 35}}));
    EXPECT_EQ(result[1].first, "InternalExtensionForTesting");
    EXPECT_EQ(result[1].second.range, (Source::Range{{4, 8}, {4, 35}}));
}

// Crash was occuring in ::GenerateSamplerTargets, when
// ::GetSamplerTextureUses was called.
TEST_F(InspectorRegressionTest, tint967) {
    std::string shader = R"(
@group(0) @binding(1) var mySampler: sampler;
@group(0) @binding(2) var myTexture: texture_2d<f32>;

fn doSample(t: texture_2d<f32>, s: sampler, uv: vec2<f32>) -> vec4<f32> {
  return textureSample(t, s, uv);
}

@stage(fragment)
fn main(@location(0) fragUV: vec2<f32>,
        @location(1) fragPosition: vec4<f32>) -> @location(0) vec4<f32> {
  return doSample(myTexture, mySampler, fragUV) * fragPosition;
})";

    Inspector& inspector = Initialize(shader);
    auto result = inspector.GetSamplerTextureUses("main");
}

}  // namespace
}  // namespace tint::inspector
