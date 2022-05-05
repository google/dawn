// Copyright 2021 The Tint Authors.
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

#include "src/tint/ast/disable_validation_attribute.h"
#include "src/tint/resolver/resolver.h"
#include "src/tint/resolver/resolver_test_helper.h"
#include "src/tint/transform/add_spirv_block_attribute.h"

#include "gmock/gmock.h"

using namespace tint::number_suffixes;  // NOLINT

namespace tint::resolver {

// Helpers and typedefs
template <typename T>
using DataType = builder::DataType<T>;
template <typename T>
using vec2 = builder::vec2<T>;
template <typename T>
using vec3 = builder::vec3<T>;
template <typename T>
using vec4 = builder::vec4<T>;
template <typename T>
using mat2x2 = builder::mat2x2<T>;
template <typename T>
using mat3x3 = builder::mat3x3<T>;
template <typename T>
using mat4x4 = builder::mat4x4<T>;
template <typename T, int ID = 0>
using alias = builder::alias<T, ID>;
template <typename T>
using alias1 = builder::alias1<T>;
template <typename T>
using alias2 = builder::alias2<T>;
template <typename T>
using alias3 = builder::alias3<T>;

namespace AttributeTests {
namespace {
enum class AttributeKind {
    kAlign,
    kBinding,
    kBuiltin,
    kGroup,
    kId,
    kInterpolate,
    kInvariant,
    kLocation,
    kOffset,
    kSize,
    kStage,
    kStride,
    kWorkgroup,

    kBindingAndGroup,
};

static bool IsBindingAttribute(AttributeKind kind) {
    switch (kind) {
        case AttributeKind::kBinding:
        case AttributeKind::kGroup:
        case AttributeKind::kBindingAndGroup:
            return true;
        default:
            return false;
    }
}

struct TestParams {
    AttributeKind kind;
    bool should_pass;
};
struct TestWithParams : ResolverTestWithParam<TestParams> {};

static ast::AttributeList createAttributes(const Source& source,
                                           ProgramBuilder& builder,
                                           AttributeKind kind) {
    switch (kind) {
        case AttributeKind::kAlign:
            return {builder.create<ast::StructMemberAlignAttribute>(source, 4u)};
        case AttributeKind::kBinding:
            return {builder.create<ast::BindingAttribute>(source, 1u)};
        case AttributeKind::kBuiltin:
            return {builder.Builtin(source, ast::Builtin::kPosition)};
        case AttributeKind::kGroup:
            return {builder.create<ast::GroupAttribute>(source, 1u)};
        case AttributeKind::kId:
            return {builder.create<ast::IdAttribute>(source, 0u)};
        case AttributeKind::kInterpolate:
            return {builder.Interpolate(source, ast::InterpolationType::kLinear,
                                        ast::InterpolationSampling::kCenter)};
        case AttributeKind::kInvariant:
            return {builder.Invariant(source)};
        case AttributeKind::kLocation:
            return {builder.Location(source, 1)};
        case AttributeKind::kOffset:
            return {builder.create<ast::StructMemberOffsetAttribute>(source, 4u)};
        case AttributeKind::kSize:
            return {builder.create<ast::StructMemberSizeAttribute>(source, 16u)};
        case AttributeKind::kStage:
            return {builder.Stage(source, ast::PipelineStage::kCompute)};
        case AttributeKind::kStride:
            return {builder.create<ast::StrideAttribute>(source, 4u)};
        case AttributeKind::kWorkgroup:
            return {builder.create<ast::WorkgroupAttribute>(source, builder.Expr(1_i))};
        case AttributeKind::kBindingAndGroup:
            return {builder.create<ast::BindingAttribute>(source, 1_u),
                    builder.create<ast::GroupAttribute>(source, 1_u)};
    }
    return {};
}

namespace FunctionInputAndOutputTests {
using FunctionParameterAttributeTest = TestWithParams;
TEST_P(FunctionParameterAttributeTest, IsValid) {
    auto& params = GetParam();

    Func("main",
         ast::VariableList{Param("a", ty.vec4<f32>(), createAttributes({}, *this, params.kind))},
         ty.void_(), {});

    if (params.should_pass) {
        EXPECT_TRUE(r()->Resolve()) << r()->error();
    } else {
        EXPECT_FALSE(r()->Resolve());
        EXPECT_EQ(r()->error(),
                  "error: attribute is not valid for non-entry point function "
                  "parameters");
    }
}
INSTANTIATE_TEST_SUITE_P(ResolverAttributeValidationTest,
                         FunctionParameterAttributeTest,
                         testing::Values(TestParams{AttributeKind::kAlign, false},
                                         TestParams{AttributeKind::kBinding, false},
                                         TestParams{AttributeKind::kBuiltin, false},
                                         TestParams{AttributeKind::kGroup, false},
                                         TestParams{AttributeKind::kId, false},
                                         TestParams{AttributeKind::kInterpolate, false},
                                         TestParams{AttributeKind::kInvariant, false},
                                         TestParams{AttributeKind::kLocation, false},
                                         TestParams{AttributeKind::kOffset, false},
                                         TestParams{AttributeKind::kSize, false},
                                         TestParams{AttributeKind::kStage, false},
                                         TestParams{AttributeKind::kStride, false},
                                         TestParams{AttributeKind::kWorkgroup, false},
                                         TestParams{AttributeKind::kBindingAndGroup, false}));

using FunctionReturnTypeAttributeTest = TestWithParams;
TEST_P(FunctionReturnTypeAttributeTest, IsValid) {
    auto& params = GetParam();

    Func("main", ast::VariableList{}, ty.f32(), ast::StatementList{Return(1.f)}, {},
         createAttributes({}, *this, params.kind));

    if (params.should_pass) {
        EXPECT_TRUE(r()->Resolve()) << r()->error();
    } else {
        EXPECT_FALSE(r()->Resolve());
        EXPECT_EQ(r()->error(),
                  "error: attribute is not valid for non-entry point function "
                  "return types");
    }
}
INSTANTIATE_TEST_SUITE_P(ResolverAttributeValidationTest,
                         FunctionReturnTypeAttributeTest,
                         testing::Values(TestParams{AttributeKind::kAlign, false},
                                         TestParams{AttributeKind::kBinding, false},
                                         TestParams{AttributeKind::kBuiltin, false},
                                         TestParams{AttributeKind::kGroup, false},
                                         TestParams{AttributeKind::kId, false},
                                         TestParams{AttributeKind::kInterpolate, false},
                                         TestParams{AttributeKind::kInvariant, false},
                                         TestParams{AttributeKind::kLocation, false},
                                         TestParams{AttributeKind::kOffset, false},
                                         TestParams{AttributeKind::kSize, false},
                                         TestParams{AttributeKind::kStage, false},
                                         TestParams{AttributeKind::kStride, false},
                                         TestParams{AttributeKind::kWorkgroup, false},
                                         TestParams{AttributeKind::kBindingAndGroup, false}));
}  // namespace FunctionInputAndOutputTests

namespace EntryPointInputAndOutputTests {
using ComputeShaderParameterAttributeTest = TestWithParams;
TEST_P(ComputeShaderParameterAttributeTest, IsValid) {
    auto& params = GetParam();
    auto* p = Param("a", ty.vec4<f32>(), createAttributes(Source{{12, 34}}, *this, params.kind));
    Func("main", ast::VariableList{p}, ty.void_(), {},
         {Stage(ast::PipelineStage::kCompute), WorkgroupSize(1_i)});

    if (params.should_pass) {
        EXPECT_TRUE(r()->Resolve()) << r()->error();
    } else {
        EXPECT_FALSE(r()->Resolve());
        if (params.kind == AttributeKind::kBuiltin) {
            EXPECT_EQ(r()->error(),
                      "12:34 error: builtin(position) cannot be used in input of "
                      "compute pipeline stage");
        } else if (params.kind == AttributeKind::kInterpolate ||
                   params.kind == AttributeKind::kLocation ||
                   params.kind == AttributeKind::kInvariant) {
            EXPECT_EQ(r()->error(),
                      "12:34 error: attribute is not valid for compute shader inputs");
        } else {
            EXPECT_EQ(r()->error(), "12:34 error: attribute is not valid for function parameters");
        }
    }
}
INSTANTIATE_TEST_SUITE_P(ResolverAttributeValidationTest,
                         ComputeShaderParameterAttributeTest,
                         testing::Values(TestParams{AttributeKind::kAlign, false},
                                         TestParams{AttributeKind::kBinding, false},
                                         TestParams{AttributeKind::kBuiltin, false},
                                         TestParams{AttributeKind::kGroup, false},
                                         TestParams{AttributeKind::kId, false},
                                         TestParams{AttributeKind::kInterpolate, false},
                                         TestParams{AttributeKind::kInvariant, false},
                                         TestParams{AttributeKind::kLocation, false},
                                         TestParams{AttributeKind::kOffset, false},
                                         TestParams{AttributeKind::kSize, false},
                                         TestParams{AttributeKind::kStage, false},
                                         TestParams{AttributeKind::kStride, false},
                                         TestParams{AttributeKind::kWorkgroup, false},
                                         TestParams{AttributeKind::kBindingAndGroup, false}));

using FragmentShaderParameterAttributeTest = TestWithParams;
TEST_P(FragmentShaderParameterAttributeTest, IsValid) {
    auto& params = GetParam();
    auto attrs = createAttributes(Source{{12, 34}}, *this, params.kind);
    if (params.kind != AttributeKind::kBuiltin && params.kind != AttributeKind::kLocation) {
        attrs.push_back(Builtin(Source{{34, 56}}, ast::Builtin::kPosition));
    }
    auto* p = Param("a", ty.vec4<f32>(), attrs);
    Func("frag_main", {p}, ty.void_(), {}, {Stage(ast::PipelineStage::kFragment)});

    if (params.should_pass) {
        EXPECT_TRUE(r()->Resolve()) << r()->error();
    } else {
        EXPECT_FALSE(r()->Resolve());
        EXPECT_EQ(r()->error(), "12:34 error: attribute is not valid for function parameters");
    }
}
INSTANTIATE_TEST_SUITE_P(ResolverAttributeValidationTest,
                         FragmentShaderParameterAttributeTest,
                         testing::Values(TestParams{AttributeKind::kAlign, false},
                                         TestParams{AttributeKind::kBinding, false},
                                         TestParams{AttributeKind::kBuiltin, true},
                                         TestParams{AttributeKind::kGroup, false},
                                         TestParams{AttributeKind::kId, false},
                                         // kInterpolate tested separately (requires @location)
                                         TestParams{AttributeKind::kInvariant, true},
                                         TestParams{AttributeKind::kLocation, true},
                                         TestParams{AttributeKind::kOffset, false},
                                         TestParams{AttributeKind::kSize, false},
                                         TestParams{AttributeKind::kStage, false},
                                         TestParams{AttributeKind::kStride, false},
                                         TestParams{AttributeKind::kWorkgroup, false},
                                         TestParams{AttributeKind::kBindingAndGroup, false}));

using VertexShaderParameterAttributeTest = TestWithParams;
TEST_P(VertexShaderParameterAttributeTest, IsValid) {
    auto& params = GetParam();
    auto attrs = createAttributes(Source{{12, 34}}, *this, params.kind);
    if (params.kind != AttributeKind::kLocation) {
        attrs.push_back(Location(Source{{34, 56}}, 2));
    }
    auto* p = Param("a", ty.vec4<f32>(), attrs);
    Func("vertex_main", ast::VariableList{p}, ty.vec4<f32>(), {Return(Construct(ty.vec4<f32>()))},
         {Stage(ast::PipelineStage::kVertex)}, {Builtin(ast::Builtin::kPosition)});

    if (params.should_pass) {
        EXPECT_TRUE(r()->Resolve()) << r()->error();
    } else {
        EXPECT_FALSE(r()->Resolve());
        if (params.kind == AttributeKind::kBuiltin) {
            EXPECT_EQ(r()->error(),
                      "12:34 error: builtin(position) cannot be used in input of "
                      "vertex pipeline stage");
        } else if (params.kind == AttributeKind::kInvariant) {
            EXPECT_EQ(r()->error(),
                      "12:34 error: invariant attribute must only be applied to a "
                      "position builtin");
        } else {
            EXPECT_EQ(r()->error(), "12:34 error: attribute is not valid for function parameters");
        }
    }
}
INSTANTIATE_TEST_SUITE_P(ResolverAttributeValidationTest,
                         VertexShaderParameterAttributeTest,
                         testing::Values(TestParams{AttributeKind::kAlign, false},
                                         TestParams{AttributeKind::kBinding, false},
                                         TestParams{AttributeKind::kBuiltin, false},
                                         TestParams{AttributeKind::kGroup, false},
                                         TestParams{AttributeKind::kId, false},
                                         TestParams{AttributeKind::kInterpolate, true},
                                         TestParams{AttributeKind::kInvariant, false},
                                         TestParams{AttributeKind::kLocation, true},
                                         TestParams{AttributeKind::kOffset, false},
                                         TestParams{AttributeKind::kSize, false},
                                         TestParams{AttributeKind::kStage, false},
                                         TestParams{AttributeKind::kStride, false},
                                         TestParams{AttributeKind::kWorkgroup, false},
                                         TestParams{AttributeKind::kBindingAndGroup, false}));

using ComputeShaderReturnTypeAttributeTest = TestWithParams;
TEST_P(ComputeShaderReturnTypeAttributeTest, IsValid) {
    auto& params = GetParam();
    Func("main", ast::VariableList{}, ty.vec4<f32>(), {Return(Construct(ty.vec4<f32>(), 1.f))},
         {Stage(ast::PipelineStage::kCompute), WorkgroupSize(1_i)},
         createAttributes(Source{{12, 34}}, *this, params.kind));

    if (params.should_pass) {
        EXPECT_TRUE(r()->Resolve()) << r()->error();
    } else {
        EXPECT_FALSE(r()->Resolve());
        if (params.kind == AttributeKind::kBuiltin) {
            EXPECT_EQ(r()->error(),
                      "12:34 error: builtin(position) cannot be used in output of "
                      "compute pipeline stage");
        } else if (params.kind == AttributeKind::kInterpolate ||
                   params.kind == AttributeKind::kLocation ||
                   params.kind == AttributeKind::kInvariant) {
            EXPECT_EQ(r()->error(),
                      "12:34 error: attribute is not valid for compute shader output");
        } else {
            EXPECT_EQ(r()->error(),
                      "12:34 error: attribute is not valid for entry point return "
                      "types");
        }
    }
}
INSTANTIATE_TEST_SUITE_P(ResolverAttributeValidationTest,
                         ComputeShaderReturnTypeAttributeTest,
                         testing::Values(TestParams{AttributeKind::kAlign, false},
                                         TestParams{AttributeKind::kBinding, false},
                                         TestParams{AttributeKind::kBuiltin, false},
                                         TestParams{AttributeKind::kGroup, false},
                                         TestParams{AttributeKind::kId, false},
                                         TestParams{AttributeKind::kInterpolate, false},
                                         TestParams{AttributeKind::kInvariant, false},
                                         TestParams{AttributeKind::kLocation, false},
                                         TestParams{AttributeKind::kOffset, false},
                                         TestParams{AttributeKind::kSize, false},
                                         TestParams{AttributeKind::kStage, false},
                                         TestParams{AttributeKind::kStride, false},
                                         TestParams{AttributeKind::kWorkgroup, false},
                                         TestParams{AttributeKind::kBindingAndGroup, false}));

using FragmentShaderReturnTypeAttributeTest = TestWithParams;
TEST_P(FragmentShaderReturnTypeAttributeTest, IsValid) {
    auto& params = GetParam();
    auto attrs = createAttributes(Source{{12, 34}}, *this, params.kind);
    attrs.push_back(Location(Source{{34, 56}}, 2));
    Func("frag_main", {}, ty.vec4<f32>(), {Return(Construct(ty.vec4<f32>()))},
         {Stage(ast::PipelineStage::kFragment)}, attrs);

    if (params.should_pass) {
        EXPECT_TRUE(r()->Resolve()) << r()->error();
    } else {
        EXPECT_FALSE(r()->Resolve());
        if (params.kind == AttributeKind::kBuiltin) {
            EXPECT_EQ(r()->error(),
                      "12:34 error: builtin(position) cannot be used in output of "
                      "fragment pipeline stage");
        } else if (params.kind == AttributeKind::kInvariant) {
            EXPECT_EQ(r()->error(),
                      "12:34 error: invariant attribute must only be applied to a "
                      "position builtin");
        } else if (params.kind == AttributeKind::kLocation) {
            EXPECT_EQ(r()->error(),
                      "34:56 error: duplicate location attribute\n"
                      "12:34 note: first attribute declared here");
        } else {
            EXPECT_EQ(r()->error(),
                      "12:34 error: attribute is not valid for entry point return "
                      "types");
        }
    }
}
INSTANTIATE_TEST_SUITE_P(ResolverAttributeValidationTest,
                         FragmentShaderReturnTypeAttributeTest,
                         testing::Values(TestParams{AttributeKind::kAlign, false},
                                         TestParams{AttributeKind::kBinding, false},
                                         TestParams{AttributeKind::kBuiltin, false},
                                         TestParams{AttributeKind::kGroup, false},
                                         TestParams{AttributeKind::kId, false},
                                         TestParams{AttributeKind::kInterpolate, true},
                                         TestParams{AttributeKind::kInvariant, false},
                                         TestParams{AttributeKind::kLocation, false},
                                         TestParams{AttributeKind::kOffset, false},
                                         TestParams{AttributeKind::kSize, false},
                                         TestParams{AttributeKind::kStage, false},
                                         TestParams{AttributeKind::kStride, false},
                                         TestParams{AttributeKind::kWorkgroup, false},
                                         TestParams{AttributeKind::kBindingAndGroup, false}));

using VertexShaderReturnTypeAttributeTest = TestWithParams;
TEST_P(VertexShaderReturnTypeAttributeTest, IsValid) {
    auto& params = GetParam();
    auto attrs = createAttributes(Source{{12, 34}}, *this, params.kind);
    // a vertex shader must include the 'position' builtin in its return type
    if (params.kind != AttributeKind::kBuiltin) {
        attrs.push_back(Builtin(Source{{34, 56}}, ast::Builtin::kPosition));
    }
    Func("vertex_main", ast::VariableList{}, ty.vec4<f32>(), {Return(Construct(ty.vec4<f32>()))},
         {Stage(ast::PipelineStage::kVertex)}, attrs);

    if (params.should_pass) {
        EXPECT_TRUE(r()->Resolve()) << r()->error();
    } else {
        EXPECT_FALSE(r()->Resolve());
        if (params.kind == AttributeKind::kLocation) {
            EXPECT_EQ(r()->error(),
                      "34:56 error: multiple entry point IO attributes\n"
                      "12:34 note: previously consumed location(1)");
        } else {
            EXPECT_EQ(r()->error(),
                      "12:34 error: attribute is not valid for entry point return "
                      "types");
        }
    }
}
INSTANTIATE_TEST_SUITE_P(ResolverAttributeValidationTest,
                         VertexShaderReturnTypeAttributeTest,
                         testing::Values(TestParams{AttributeKind::kAlign, false},
                                         TestParams{AttributeKind::kBinding, false},
                                         TestParams{AttributeKind::kBuiltin, true},
                                         TestParams{AttributeKind::kGroup, false},
                                         TestParams{AttributeKind::kId, false},
                                         // kInterpolate tested separately (requires @location)
                                         TestParams{AttributeKind::kInvariant, true},
                                         TestParams{AttributeKind::kLocation, false},
                                         TestParams{AttributeKind::kOffset, false},
                                         TestParams{AttributeKind::kSize, false},
                                         TestParams{AttributeKind::kStage, false},
                                         TestParams{AttributeKind::kStride, false},
                                         TestParams{AttributeKind::kWorkgroup, false},
                                         TestParams{AttributeKind::kBindingAndGroup, false}));

using EntryPointParameterAttributeTest = TestWithParams;
TEST_F(EntryPointParameterAttributeTest, DuplicateAttribute) {
    Func("main", ast::VariableList{}, ty.f32(), ast::StatementList{Return(1.f)},
         {Stage(ast::PipelineStage::kFragment)},
         {
             Location(Source{{12, 34}}, 2),
             Location(Source{{56, 78}}, 3),
         });

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(),
              R"(56:78 error: duplicate location attribute
12:34 note: first attribute declared here)");
}

TEST_F(EntryPointParameterAttributeTest, DuplicateInternalAttribute) {
    auto* s = Param("s", ty.sampler(ast::SamplerKind::kSampler),
                    ast::AttributeList{
                        create<ast::BindingAttribute>(0),
                        create<ast::GroupAttribute>(0),
                        Disable(ast::DisabledValidation::kBindingPointCollision),
                        Disable(ast::DisabledValidation::kEntryPointParameter),
                    });
    Func("f", {s}, ty.void_(), {}, {Stage(ast::PipelineStage::kFragment)});

    EXPECT_TRUE(r()->Resolve()) << r()->error();
}

using EntryPointReturnTypeAttributeTest = ResolverTest;
TEST_F(EntryPointReturnTypeAttributeTest, DuplicateAttribute) {
    Func("main", ast::VariableList{}, ty.f32(), ast::StatementList{Return(1.f)},
         ast::AttributeList{Stage(ast::PipelineStage::kFragment)},
         ast::AttributeList{
             Location(Source{{12, 34}}, 2),
             Location(Source{{56, 78}}, 3),
         });

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(),
              R"(56:78 error: duplicate location attribute
12:34 note: first attribute declared here)");
}

TEST_F(EntryPointReturnTypeAttributeTest, DuplicateInternalAttribute) {
    Func("f", {}, ty.i32(), {Return(1_i)}, {Stage(ast::PipelineStage::kFragment)},
         ast::AttributeList{
             Disable(ast::DisabledValidation::kBindingPointCollision),
             Disable(ast::DisabledValidation::kEntryPointParameter),
         });

    EXPECT_TRUE(r()->Resolve()) << r()->error();
}
}  // namespace EntryPointInputAndOutputTests

namespace StructAndStructMemberTests {
using StructAttributeTest = TestWithParams;
using SpirvBlockAttribute = transform::AddSpirvBlockAttribute::SpirvBlockAttribute;
TEST_P(StructAttributeTest, IsValid) {
    auto& params = GetParam();

    auto* str = create<ast::Struct>(Sym("mystruct"), ast::StructMemberList{Member("a", ty.f32())},
                                    createAttributes(Source{{12, 34}}, *this, params.kind));
    AST().AddGlobalDeclaration(str);

    if (params.should_pass) {
        EXPECT_TRUE(r()->Resolve()) << r()->error();
    } else {
        EXPECT_FALSE(r()->Resolve());
        EXPECT_EQ(r()->error(), "12:34 error: attribute is not valid for struct declarations");
    }
}
INSTANTIATE_TEST_SUITE_P(ResolverAttributeValidationTest,
                         StructAttributeTest,
                         testing::Values(TestParams{AttributeKind::kAlign, false},
                                         TestParams{AttributeKind::kBinding, false},
                                         TestParams{AttributeKind::kBuiltin, false},
                                         TestParams{AttributeKind::kGroup, false},
                                         TestParams{AttributeKind::kId, false},
                                         TestParams{AttributeKind::kInterpolate, false},
                                         TestParams{AttributeKind::kInvariant, false},
                                         TestParams{AttributeKind::kLocation, false},
                                         TestParams{AttributeKind::kOffset, false},
                                         TestParams{AttributeKind::kSize, false},
                                         TestParams{AttributeKind::kStage, false},
                                         TestParams{AttributeKind::kStride, false},
                                         TestParams{AttributeKind::kWorkgroup, false},
                                         TestParams{AttributeKind::kBindingAndGroup, false}));

using StructMemberAttributeTest = TestWithParams;
TEST_P(StructMemberAttributeTest, IsValid) {
    auto& params = GetParam();
    ast::StructMemberList members;
    if (params.kind == AttributeKind::kBuiltin) {
        members.push_back(
            {Member("a", ty.vec4<f32>(), createAttributes(Source{{12, 34}}, *this, params.kind))});
    } else {
        members.push_back(
            {Member("a", ty.f32(), createAttributes(Source{{12, 34}}, *this, params.kind))});
    }
    Structure("mystruct", members);
    WrapInFunction();
    if (params.should_pass) {
        EXPECT_TRUE(r()->Resolve()) << r()->error();
    } else {
        EXPECT_FALSE(r()->Resolve());
        EXPECT_EQ(r()->error(), "12:34 error: attribute is not valid for structure members");
    }
}
INSTANTIATE_TEST_SUITE_P(ResolverAttributeValidationTest,
                         StructMemberAttributeTest,
                         testing::Values(TestParams{AttributeKind::kAlign, true},
                                         TestParams{AttributeKind::kBinding, false},
                                         TestParams{AttributeKind::kBuiltin, true},
                                         TestParams{AttributeKind::kGroup, false},
                                         TestParams{AttributeKind::kId, false},
                                         // kInterpolate tested separately (requires @location)
                                         // kInvariant tested separately (requires position builtin)
                                         TestParams{AttributeKind::kLocation, true},
                                         TestParams{AttributeKind::kOffset, true},
                                         TestParams{AttributeKind::kSize, true},
                                         TestParams{AttributeKind::kStage, false},
                                         TestParams{AttributeKind::kStride, false},
                                         TestParams{AttributeKind::kWorkgroup, false},
                                         TestParams{AttributeKind::kBindingAndGroup, false}));
TEST_F(StructMemberAttributeTest, DuplicateAttribute) {
    Structure("mystruct",
              {
                  Member("a", ty.i32(),
                         {
                             create<ast::StructMemberAlignAttribute>(Source{{12, 34}}, 4u),
                             create<ast::StructMemberAlignAttribute>(Source{{56, 78}}, 8u),
                         }),
              });
    WrapInFunction();
    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(),
              R"(56:78 error: duplicate align attribute
12:34 note: first attribute declared here)");
}
TEST_F(StructMemberAttributeTest, InvariantAttributeWithPosition) {
    Structure("mystruct", {
                              Member("a", ty.vec4<f32>(),
                                     {
                                         Invariant(),
                                         Builtin(ast::Builtin::kPosition),
                                     }),
                          });
    WrapInFunction();
    EXPECT_TRUE(r()->Resolve()) << r()->error();
}
TEST_F(StructMemberAttributeTest, InvariantAttributeWithoutPosition) {
    Structure("mystruct", {
                              Member("a", ty.vec4<f32>(),
                                     {
                                         Invariant(Source{{12, 34}}),
                                     }),
                          });
    WrapInFunction();
    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(),
              "12:34 error: invariant attribute must only be applied to a "
              "position builtin");
}

}  // namespace StructAndStructMemberTests

using ArrayAttributeTest = TestWithParams;
TEST_P(ArrayAttributeTest, IsValid) {
    auto& params = GetParam();

    auto* arr = ty.array(ty.f32(), nullptr, createAttributes(Source{{12, 34}}, *this, params.kind));
    Structure("mystruct", {
                              Member("a", arr),
                          });

    WrapInFunction();

    if (params.should_pass) {
        EXPECT_TRUE(r()->Resolve()) << r()->error();
    } else {
        EXPECT_FALSE(r()->Resolve());
        EXPECT_EQ(r()->error(), "12:34 error: attribute is not valid for array types");
    }
}
INSTANTIATE_TEST_SUITE_P(ResolverAttributeValidationTest,
                         ArrayAttributeTest,
                         testing::Values(TestParams{AttributeKind::kAlign, false},
                                         TestParams{AttributeKind::kBinding, false},
                                         TestParams{AttributeKind::kBuiltin, false},
                                         TestParams{AttributeKind::kGroup, false},
                                         TestParams{AttributeKind::kId, false},
                                         TestParams{AttributeKind::kInterpolate, false},
                                         TestParams{AttributeKind::kInvariant, false},
                                         TestParams{AttributeKind::kLocation, false},
                                         TestParams{AttributeKind::kOffset, false},
                                         TestParams{AttributeKind::kSize, false},
                                         TestParams{AttributeKind::kStage, false},
                                         TestParams{AttributeKind::kStride, true},
                                         TestParams{AttributeKind::kWorkgroup, false},
                                         TestParams{AttributeKind::kBindingAndGroup, false}));

using VariableAttributeTest = TestWithParams;
TEST_P(VariableAttributeTest, IsValid) {
    auto& params = GetParam();

    if (IsBindingAttribute(params.kind)) {
        Global("a", ty.sampler(ast::SamplerKind::kSampler), ast::StorageClass::kNone, nullptr,
               createAttributes(Source{{12, 34}}, *this, params.kind));
    } else {
        Global("a", ty.f32(), ast::StorageClass::kPrivate, nullptr,
               createAttributes(Source{{12, 34}}, *this, params.kind));
    }

    WrapInFunction();

    if (params.should_pass) {
        EXPECT_TRUE(r()->Resolve()) << r()->error();
    } else {
        EXPECT_FALSE(r()->Resolve());
        if (!IsBindingAttribute(params.kind)) {
            EXPECT_EQ(r()->error(), "12:34 error: attribute is not valid for variables");
        }
    }
}
INSTANTIATE_TEST_SUITE_P(ResolverAttributeValidationTest,
                         VariableAttributeTest,
                         testing::Values(TestParams{AttributeKind::kAlign, false},
                                         TestParams{AttributeKind::kBinding, false},
                                         TestParams{AttributeKind::kBuiltin, false},
                                         TestParams{AttributeKind::kGroup, false},
                                         TestParams{AttributeKind::kId, false},
                                         TestParams{AttributeKind::kInterpolate, false},
                                         TestParams{AttributeKind::kInvariant, false},
                                         TestParams{AttributeKind::kLocation, false},
                                         TestParams{AttributeKind::kOffset, false},
                                         TestParams{AttributeKind::kSize, false},
                                         TestParams{AttributeKind::kStage, false},
                                         TestParams{AttributeKind::kStride, false},
                                         TestParams{AttributeKind::kWorkgroup, false},
                                         TestParams{AttributeKind::kBindingAndGroup, true}));

TEST_F(VariableAttributeTest, DuplicateAttribute) {
    Global("a", ty.sampler(ast::SamplerKind::kSampler),
           ast::AttributeList{
               create<ast::BindingAttribute>(Source{{12, 34}}, 2),
               create<ast::GroupAttribute>(2),
               create<ast::BindingAttribute>(Source{{56, 78}}, 3),
           });

    WrapInFunction();

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(),
              R"(56:78 error: duplicate binding attribute
12:34 note: first attribute declared here)");
}

TEST_F(VariableAttributeTest, LocalVariable) {
    auto* v = Var("a", ty.f32(),
                  ast::AttributeList{
                      create<ast::BindingAttribute>(Source{{12, 34}}, 2),
                  });

    WrapInFunction(v);

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), "12:34 error: attributes are not valid on local variables");
}

using ConstantAttributeTest = TestWithParams;
TEST_P(ConstantAttributeTest, IsValid) {
    auto& params = GetParam();

    GlobalConst("a", ty.f32(), Expr(1.23f), createAttributes(Source{{12, 34}}, *this, params.kind));

    WrapInFunction();

    if (params.should_pass) {
        EXPECT_TRUE(r()->Resolve()) << r()->error();
    } else {
        EXPECT_FALSE(r()->Resolve());
        EXPECT_EQ(r()->error(), "12:34 error: attribute is not valid for constants");
    }
}
INSTANTIATE_TEST_SUITE_P(ResolverAttributeValidationTest,
                         ConstantAttributeTest,
                         testing::Values(TestParams{AttributeKind::kAlign, false},
                                         TestParams{AttributeKind::kBinding, false},
                                         TestParams{AttributeKind::kBuiltin, false},
                                         TestParams{AttributeKind::kGroup, false},
                                         TestParams{AttributeKind::kId, true},
                                         TestParams{AttributeKind::kInterpolate, false},
                                         TestParams{AttributeKind::kInvariant, false},
                                         TestParams{AttributeKind::kLocation, false},
                                         TestParams{AttributeKind::kOffset, false},
                                         TestParams{AttributeKind::kSize, false},
                                         TestParams{AttributeKind::kStage, false},
                                         TestParams{AttributeKind::kStride, false},
                                         TestParams{AttributeKind::kWorkgroup, false},
                                         TestParams{AttributeKind::kBindingAndGroup, false}));

TEST_F(ConstantAttributeTest, DuplicateAttribute) {
    GlobalConst("a", ty.f32(), Expr(1.23f),
                ast::AttributeList{
                    create<ast::IdAttribute>(Source{{12, 34}}, 0),
                    create<ast::IdAttribute>(Source{{56, 78}}, 1),
                });

    WrapInFunction();

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(),
              R"(56:78 error: duplicate id attribute
12:34 note: first attribute declared here)");
}

}  // namespace
}  // namespace AttributeTests

namespace ArrayStrideTests {
namespace {

struct Params {
    builder::ast_type_func_ptr create_el_type;
    uint32_t stride;
    bool should_pass;
};

template <typename T>
constexpr Params ParamsFor(uint32_t stride, bool should_pass) {
    return Params{DataType<T>::AST, stride, should_pass};
}

struct TestWithParams : ResolverTestWithParam<Params> {};

using ArrayStrideTest = TestWithParams;
TEST_P(ArrayStrideTest, All) {
    auto& params = GetParam();
    auto* el_ty = params.create_el_type(*this);

    std::stringstream ss;
    ss << "el_ty: " << FriendlyName(el_ty) << ", stride: " << params.stride
       << ", should_pass: " << params.should_pass;
    SCOPED_TRACE(ss.str());

    auto* arr = ty.array(Source{{12, 34}}, el_ty, 4_u, params.stride);

    Global("myarray", arr, ast::StorageClass::kPrivate);

    if (params.should_pass) {
        EXPECT_TRUE(r()->Resolve()) << r()->error();
    } else {
        EXPECT_FALSE(r()->Resolve());
        EXPECT_EQ(r()->error(),
                  "12:34 error: arrays decorated with the stride attribute must "
                  "have a stride that is at least the size of the element type, "
                  "and be a multiple of the element type's alignment value.");
    }
}

struct SizeAndAlignment {
    uint32_t size;
    uint32_t align;
};
constexpr SizeAndAlignment default_u32 = {4, 4};
constexpr SizeAndAlignment default_i32 = {4, 4};
constexpr SizeAndAlignment default_f32 = {4, 4};
constexpr SizeAndAlignment default_vec2 = {8, 8};
constexpr SizeAndAlignment default_vec3 = {12, 16};
constexpr SizeAndAlignment default_vec4 = {16, 16};
constexpr SizeAndAlignment default_mat2x2 = {16, 8};
constexpr SizeAndAlignment default_mat3x3 = {48, 16};
constexpr SizeAndAlignment default_mat4x4 = {64, 16};

INSTANTIATE_TEST_SUITE_P(ResolverAttributeValidationTest,
                         ArrayStrideTest,
                         testing::Values(
                             // Succeed because stride >= element size (while being multiple of
                             // element alignment)
                             ParamsFor<u32>(default_u32.size, true),
                             ParamsFor<i32>(default_i32.size, true),
                             ParamsFor<f32>(default_f32.size, true),
                             ParamsFor<vec2<f32>>(default_vec2.size, true),
                             // vec3's default size is not a multiple of its alignment
                             // ParamsFor<vec3<f32>, default_vec3.size, true},
                             ParamsFor<vec4<f32>>(default_vec4.size, true),
                             ParamsFor<mat2x2<f32>>(default_mat2x2.size, true),
                             ParamsFor<mat3x3<f32>>(default_mat3x3.size, true),
                             ParamsFor<mat4x4<f32>>(default_mat4x4.size, true),

                             // Fail because stride is < element size
                             ParamsFor<u32>(default_u32.size - 1, false),
                             ParamsFor<i32>(default_i32.size - 1, false),
                             ParamsFor<f32>(default_f32.size - 1, false),
                             ParamsFor<vec2<f32>>(default_vec2.size - 1, false),
                             ParamsFor<vec3<f32>>(default_vec3.size - 1, false),
                             ParamsFor<vec4<f32>>(default_vec4.size - 1, false),
                             ParamsFor<mat2x2<f32>>(default_mat2x2.size - 1, false),
                             ParamsFor<mat3x3<f32>>(default_mat3x3.size - 1, false),
                             ParamsFor<mat4x4<f32>>(default_mat4x4.size - 1, false),

                             // Succeed because stride equals multiple of element alignment
                             ParamsFor<u32>(default_u32.align * 7, true),
                             ParamsFor<i32>(default_i32.align * 7, true),
                             ParamsFor<f32>(default_f32.align * 7, true),
                             ParamsFor<vec2<f32>>(default_vec2.align * 7, true),
                             ParamsFor<vec3<f32>>(default_vec3.align * 7, true),
                             ParamsFor<vec4<f32>>(default_vec4.align * 7, true),
                             ParamsFor<mat2x2<f32>>(default_mat2x2.align * 7, true),
                             ParamsFor<mat3x3<f32>>(default_mat3x3.align * 7, true),
                             ParamsFor<mat4x4<f32>>(default_mat4x4.align * 7, true),

                             // Fail because stride is not multiple of element alignment
                             ParamsFor<u32>((default_u32.align - 1) * 7, false),
                             ParamsFor<i32>((default_i32.align - 1) * 7, false),
                             ParamsFor<f32>((default_f32.align - 1) * 7, false),
                             ParamsFor<vec2<f32>>((default_vec2.align - 1) * 7, false),
                             ParamsFor<vec3<f32>>((default_vec3.align - 1) * 7, false),
                             ParamsFor<vec4<f32>>((default_vec4.align - 1) * 7, false),
                             ParamsFor<mat2x2<f32>>((default_mat2x2.align - 1) * 7, false),
                             ParamsFor<mat3x3<f32>>((default_mat3x3.align - 1) * 7, false),
                             ParamsFor<mat4x4<f32>>((default_mat4x4.align - 1) * 7, false)));

TEST_F(ArrayStrideTest, DuplicateAttribute) {
    auto* arr = ty.array(Source{{12, 34}}, ty.i32(), 4_u,
                         {
                             create<ast::StrideAttribute>(Source{{12, 34}}, 4_i),
                             create<ast::StrideAttribute>(Source{{56, 78}}, 4_i),
                         });

    Global("myarray", arr, ast::StorageClass::kPrivate);

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(),
              R"(56:78 error: duplicate stride attribute
12:34 note: first attribute declared here)");
}

}  // namespace
}  // namespace ArrayStrideTests

namespace ResourceTests {
namespace {

using ResourceAttributeTest = ResolverTest;
TEST_F(ResourceAttributeTest, UniformBufferMissingBinding) {
    auto* s = Structure("S", {Member("x", ty.i32())});
    Global(Source{{12, 34}}, "G", ty.Of(s), ast::StorageClass::kUniform);

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(),
              R"(12:34 error: resource variables require @group and @binding attributes)");
}

TEST_F(ResourceAttributeTest, StorageBufferMissingBinding) {
    auto* s = Structure("S", {Member("x", ty.i32())});
    Global(Source{{12, 34}}, "G", ty.Of(s), ast::StorageClass::kStorage, ast::Access::kRead);

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(),
              R"(12:34 error: resource variables require @group and @binding attributes)");
}

TEST_F(ResourceAttributeTest, TextureMissingBinding) {
    Global(Source{{12, 34}}, "G", ty.depth_texture(ast::TextureDimension::k2d),
           ast::StorageClass::kNone);

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(),
              R"(12:34 error: resource variables require @group and @binding attributes)");
}

TEST_F(ResourceAttributeTest, SamplerMissingBinding) {
    Global(Source{{12, 34}}, "G", ty.sampler(ast::SamplerKind::kSampler), ast::StorageClass::kNone);

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(),
              R"(12:34 error: resource variables require @group and @binding attributes)");
}

TEST_F(ResourceAttributeTest, BindingPairMissingBinding) {
    Global(Source{{12, 34}}, "G", ty.sampler(ast::SamplerKind::kSampler), ast::StorageClass::kNone,
           ast::AttributeList{
               create<ast::GroupAttribute>(1),
           });

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(),
              R"(12:34 error: resource variables require @group and @binding attributes)");
}

TEST_F(ResourceAttributeTest, BindingPairMissingGroup) {
    Global(Source{{12, 34}}, "G", ty.sampler(ast::SamplerKind::kSampler), ast::StorageClass::kNone,
           ast::AttributeList{
               create<ast::BindingAttribute>(1),
           });

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(),
              R"(12:34 error: resource variables require @group and @binding attributes)");
}

TEST_F(ResourceAttributeTest, BindingPointUsedTwiceByEntryPoint) {
    Global(Source{{12, 34}}, "A", ty.sampled_texture(ast::TextureDimension::k2d, ty.f32()),
           ast::StorageClass::kNone,
           ast::AttributeList{
               create<ast::BindingAttribute>(1),
               create<ast::GroupAttribute>(2),
           });
    Global(Source{{56, 78}}, "B", ty.sampled_texture(ast::TextureDimension::k2d, ty.f32()),
           ast::StorageClass::kNone,
           ast::AttributeList{
               create<ast::BindingAttribute>(1),
               create<ast::GroupAttribute>(2),
           });

    Func("F", {}, ty.void_(),
         {
             Decl(Var("a", ty.vec4<f32>(), ast::StorageClass::kNone,
                      Call("textureLoad", "A", vec2<i32>(1_i, 2_i), 0_i))),
             Decl(Var("b", ty.vec4<f32>(), ast::StorageClass::kNone,
                      Call("textureLoad", "B", vec2<i32>(1_i, 2_i), 0_i))),
         },
         {Stage(ast::PipelineStage::kFragment)});

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(
        r()->error(),
        R"(56:78 error: entry point 'F' references multiple variables that use the same resource binding @group(2), @binding(1)
12:34 note: first resource binding usage declared here)");
}

TEST_F(ResourceAttributeTest, BindingPointUsedTwiceByDifferentEntryPoints) {
    Global(Source{{12, 34}}, "A", ty.sampled_texture(ast::TextureDimension::k2d, ty.f32()),
           ast::StorageClass::kNone,
           ast::AttributeList{
               create<ast::BindingAttribute>(1),
               create<ast::GroupAttribute>(2),
           });
    Global(Source{{56, 78}}, "B", ty.sampled_texture(ast::TextureDimension::k2d, ty.f32()),
           ast::StorageClass::kNone,
           ast::AttributeList{
               create<ast::BindingAttribute>(1),
               create<ast::GroupAttribute>(2),
           });

    Func("F_A", {}, ty.void_(),
         {
             Decl(Var("a", ty.vec4<f32>(), ast::StorageClass::kNone,
                      Call("textureLoad", "A", vec2<i32>(1_i, 2_i), 0_i))),
         },
         {Stage(ast::PipelineStage::kFragment)});
    Func("F_B", {}, ty.void_(),
         {
             Decl(Var("b", ty.vec4<f32>(), ast::StorageClass::kNone,
                      Call("textureLoad", "B", vec2<i32>(1_i, 2_i), 0_i))),
         },
         {Stage(ast::PipelineStage::kFragment)});

    EXPECT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResourceAttributeTest, BindingPointOnNonResource) {
    Global(Source{{12, 34}}, "G", ty.f32(), ast::StorageClass::kPrivate,
           ast::AttributeList{
               create<ast::BindingAttribute>(1),
               create<ast::GroupAttribute>(2),
           });

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(),
              R"(12:34 error: non-resource variables must not have @group or @binding attributes)");
}

}  // namespace
}  // namespace ResourceTests

namespace InvariantAttributeTests {
namespace {
using InvariantAttributeTests = ResolverTest;
TEST_F(InvariantAttributeTests, InvariantWithPosition) {
    auto* param =
        Param("p", ty.vec4<f32>(),
              {Invariant(Source{{12, 34}}), Builtin(Source{{56, 78}}, ast::Builtin::kPosition)});
    Func("main", ast::VariableList{param}, ty.vec4<f32>(),
         ast::StatementList{Return(Construct(ty.vec4<f32>()))},
         ast::AttributeList{Stage(ast::PipelineStage::kFragment)},
         ast::AttributeList{
             Location(0),
         });
    EXPECT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(InvariantAttributeTests, InvariantWithoutPosition) {
    auto* param = Param("p", ty.vec4<f32>(), {Invariant(Source{{12, 34}}), Location(0)});
    Func("main", ast::VariableList{param}, ty.vec4<f32>(),
         ast::StatementList{Return(Construct(ty.vec4<f32>()))},
         ast::AttributeList{Stage(ast::PipelineStage::kFragment)},
         ast::AttributeList{
             Location(0),
         });
    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(),
              "12:34 error: invariant attribute must only be applied to a "
              "position builtin");
}
}  // namespace
}  // namespace InvariantAttributeTests

namespace WorkgroupAttributeTests {
namespace {

using WorkgroupAttribute = ResolverTest;
TEST_F(WorkgroupAttribute, ComputeShaderPass) {
    Func("main", {}, ty.void_(), {},
         {Stage(ast::PipelineStage::kCompute),
          create<ast::WorkgroupAttribute>(Source{{12, 34}}, Expr(1_i))});

    EXPECT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(WorkgroupAttribute, Missing) {
    Func(Source{{12, 34}}, "main", {}, ty.void_(), {}, {Stage(ast::PipelineStage::kCompute)});

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(),
              "12:34 error: a compute shader must include 'workgroup_size' in its "
              "attributes");
}

TEST_F(WorkgroupAttribute, NotAnEntryPoint) {
    Func("main", {}, ty.void_(), {},
         {create<ast::WorkgroupAttribute>(Source{{12, 34}}, Expr(1_i))});

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(),
              "12:34 error: the workgroup_size attribute is only valid for "
              "compute stages");
}

TEST_F(WorkgroupAttribute, NotAComputeShader) {
    Func("main", {}, ty.void_(), {},
         {Stage(ast::PipelineStage::kFragment),
          create<ast::WorkgroupAttribute>(Source{{12, 34}}, Expr(1_i))});

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(),
              "12:34 error: the workgroup_size attribute is only valid for "
              "compute stages");
}

TEST_F(WorkgroupAttribute, DuplicateAttribute) {
    Func(Source{{12, 34}}, "main", {}, ty.void_(), {},
         {
             Stage(ast::PipelineStage::kCompute),
             WorkgroupSize(Source{{12, 34}}, 1_i, nullptr, nullptr),
             WorkgroupSize(Source{{56, 78}}, 2_i, nullptr, nullptr),
         });

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(),
              R"(56:78 error: duplicate workgroup_size attribute
12:34 note: first attribute declared here)");
}

}  // namespace
}  // namespace WorkgroupAttributeTests

namespace InterpolateTests {
namespace {

using InterpolateTest = ResolverTest;

struct Params {
    ast::InterpolationType type;
    ast::InterpolationSampling sampling;
    bool should_pass;
};

struct TestWithParams : ResolverTestWithParam<Params> {};

using InterpolateParameterTest = TestWithParams;
TEST_P(InterpolateParameterTest, All) {
    auto& params = GetParam();

    Func("main",
         ast::VariableList{
             Param("a", ty.f32(),
                   {Location(0), Interpolate(Source{{12, 34}}, params.type, params.sampling)})},
         ty.void_(), {}, ast::AttributeList{Stage(ast::PipelineStage::kFragment)});

    if (params.should_pass) {
        EXPECT_TRUE(r()->Resolve()) << r()->error();
    } else {
        EXPECT_FALSE(r()->Resolve());
        EXPECT_EQ(r()->error(),
                  "12:34 error: flat interpolation attribute must not have a "
                  "sampling parameter");
    }
}

TEST_P(InterpolateParameterTest, IntegerScalar) {
    auto& params = GetParam();

    Func("main",
         ast::VariableList{
             Param("a", ty.i32(),
                   {Location(0), Interpolate(Source{{12, 34}}, params.type, params.sampling)})},
         ty.void_(), {}, ast::AttributeList{Stage(ast::PipelineStage::kFragment)});

    if (params.type != ast::InterpolationType::kFlat) {
        EXPECT_FALSE(r()->Resolve());
        EXPECT_EQ(r()->error(),
                  "12:34 error: interpolation type must be 'flat' for integral "
                  "user-defined IO types");
    } else if (params.should_pass) {
        EXPECT_TRUE(r()->Resolve()) << r()->error();
    } else {
        EXPECT_FALSE(r()->Resolve());
        EXPECT_EQ(r()->error(),
                  "12:34 error: flat interpolation attribute must not have a "
                  "sampling parameter");
    }
}

TEST_P(InterpolateParameterTest, IntegerVector) {
    auto& params = GetParam();

    Func("main",
         ast::VariableList{
             Param("a", ty.vec4<u32>(),
                   {Location(0), Interpolate(Source{{12, 34}}, params.type, params.sampling)})},
         ty.void_(), {}, ast::AttributeList{Stage(ast::PipelineStage::kFragment)});

    if (params.type != ast::InterpolationType::kFlat) {
        EXPECT_FALSE(r()->Resolve());
        EXPECT_EQ(r()->error(),
                  "12:34 error: interpolation type must be 'flat' for integral "
                  "user-defined IO types");
    } else if (params.should_pass) {
        EXPECT_TRUE(r()->Resolve()) << r()->error();
    } else {
        EXPECT_FALSE(r()->Resolve());
        EXPECT_EQ(r()->error(),
                  "12:34 error: flat interpolation attribute must not have a "
                  "sampling parameter");
    }
}

INSTANTIATE_TEST_SUITE_P(
    ResolverAttributeValidationTest,
    InterpolateParameterTest,
    testing::Values(
        Params{ast::InterpolationType::kPerspective, ast::InterpolationSampling::kNone, true},
        Params{ast::InterpolationType::kPerspective, ast::InterpolationSampling::kCenter, true},
        Params{ast::InterpolationType::kPerspective, ast::InterpolationSampling::kCentroid, true},
        Params{ast::InterpolationType::kPerspective, ast::InterpolationSampling::kSample, true},
        Params{ast::InterpolationType::kLinear, ast::InterpolationSampling::kNone, true},
        Params{ast::InterpolationType::kLinear, ast::InterpolationSampling::kCenter, true},
        Params{ast::InterpolationType::kLinear, ast::InterpolationSampling::kCentroid, true},
        Params{ast::InterpolationType::kLinear, ast::InterpolationSampling::kSample, true},
        // flat interpolation must not have a sampling type
        Params{ast::InterpolationType::kFlat, ast::InterpolationSampling::kNone, true},
        Params{ast::InterpolationType::kFlat, ast::InterpolationSampling::kCenter, false},
        Params{ast::InterpolationType::kFlat, ast::InterpolationSampling::kCentroid, false},
        Params{ast::InterpolationType::kFlat, ast::InterpolationSampling::kSample, false}));

TEST_F(InterpolateTest, FragmentInput_Integer_MissingFlatInterpolation) {
    Func("main", ast::VariableList{Param(Source{{12, 34}}, "a", ty.i32(), {Location(0)})},
         ty.void_(), {}, ast::AttributeList{Stage(ast::PipelineStage::kFragment)});

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(
        r()->error(),
        R"(12:34 error: integral user-defined fragment inputs must have a flat interpolation attribute)");
}

TEST_F(InterpolateTest, VertexOutput_Integer_MissingFlatInterpolation) {
    auto* s = Structure("S", {
                                 Member("pos", ty.vec4<f32>(), {Builtin(ast::Builtin::kPosition)}),
                                 Member(Source{{12, 34}}, "u", ty.u32(), {Location(0)}),
                             });
    Func("main", {}, ty.Of(s), {Return(Construct(ty.Of(s)))},
         ast::AttributeList{Stage(ast::PipelineStage::kVertex)});

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(
        r()->error(),
        R"(12:34 error: integral user-defined vertex outputs must have a flat interpolation attribute
note: while analysing entry point 'main')");
}

TEST_F(InterpolateTest, MissingLocationAttribute_Parameter) {
    Func("main",
         ast::VariableList{Param("a", ty.vec4<f32>(),
                                 {Builtin(ast::Builtin::kPosition),
                                  Interpolate(Source{{12, 34}}, ast::InterpolationType::kFlat,
                                              ast::InterpolationSampling::kNone)})},
         ty.void_(), {}, ast::AttributeList{Stage(ast::PipelineStage::kFragment)});

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(),
              R"(12:34 error: interpolate attribute must only be used with @location)");
}

TEST_F(InterpolateTest, MissingLocationAttribute_ReturnType) {
    Func("main", {}, ty.vec4<f32>(), {Return(Construct(ty.vec4<f32>()))},
         ast::AttributeList{Stage(ast::PipelineStage::kVertex)},
         {Builtin(ast::Builtin::kPosition),
          Interpolate(Source{{12, 34}}, ast::InterpolationType::kFlat,
                      ast::InterpolationSampling::kNone)});

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(),
              R"(12:34 error: interpolate attribute must only be used with @location)");
}

TEST_F(InterpolateTest, MissingLocationAttribute_Struct) {
    Structure("S", {Member("a", ty.f32(),
                           {Interpolate(Source{{12, 34}}, ast::InterpolationType::kFlat,
                                        ast::InterpolationSampling::kNone)})});

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(),
              R"(12:34 error: interpolate attribute must only be used with @location)");
}

}  // namespace
}  // namespace InterpolateTests

}  // namespace tint::resolver
