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
#include "src/tint/builtin/builtin_value.h"
#include "src/tint/resolver/resolver.h"
#include "src/tint/resolver/resolver_test_helper.h"
#include "src/tint/transform/add_block_attribute.h"
#include "src/tint/type/texture_dimension.h"
#include "src/tint/utils/string_stream.h"

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
    kDiagnostic,
    kGroup,
    kId,
    kInterpolate,
    kInvariant,
    kLocation,
    kMustUse,
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

static utils::Vector<const ast::Attribute*, 2> createAttributes(const Source& source,
                                                                ProgramBuilder& builder,
                                                                AttributeKind kind) {
    switch (kind) {
        case AttributeKind::kAlign:
            return {builder.MemberAlign(source, 4_i)};
        case AttributeKind::kBinding:
            return {builder.Binding(source, 1_a)};
        case AttributeKind::kBuiltin:
            return {builder.Builtin(source, builtin::BuiltinValue::kPosition)};
        case AttributeKind::kDiagnostic:
            return {builder.DiagnosticAttribute(source, builtin::DiagnosticSeverity::kInfo,
                                                "chromium", "unreachable_code")};
        case AttributeKind::kGroup:
            return {builder.Group(source, 1_a)};
        case AttributeKind::kId:
            return {builder.Id(source, 0_a)};
        case AttributeKind::kInterpolate:
            return {builder.Interpolate(source, builtin::InterpolationType::kLinear,
                                        builtin::InterpolationSampling::kCenter)};
        case AttributeKind::kInvariant:
            return {builder.Invariant(source)};
        case AttributeKind::kLocation:
            return {builder.Location(source, 1_a)};
        case AttributeKind::kOffset:
            return {builder.MemberOffset(source, 4_a)};
        case AttributeKind::kMustUse:
            return {builder.MustUse(source)};
        case AttributeKind::kSize:
            return {builder.MemberSize(source, 16_a)};
        case AttributeKind::kStage:
            return {builder.Stage(source, ast::PipelineStage::kCompute)};
        case AttributeKind::kStride:
            return {builder.create<ast::StrideAttribute>(source, 4u)};
        case AttributeKind::kWorkgroup:
            return {builder.create<ast::WorkgroupAttribute>(source, builder.Expr(1_i))};
        case AttributeKind::kBindingAndGroup:
            return {builder.Binding(source, 1_a), builder.Group(source, 1_a)};
    }
    return {};
}

static std::string name(AttributeKind kind) {
    switch (kind) {
        case AttributeKind::kAlign:
            return "@align";
        case AttributeKind::kBinding:
            return "@binding";
        case AttributeKind::kBuiltin:
            return "@builtin";
        case AttributeKind::kDiagnostic:
            return "@diagnostic";
        case AttributeKind::kGroup:
            return "@group";
        case AttributeKind::kId:
            return "@id";
        case AttributeKind::kInterpolate:
            return "@interpolate";
        case AttributeKind::kInvariant:
            return "@invariant";
        case AttributeKind::kLocation:
            return "@location";
        case AttributeKind::kOffset:
            return "@offset";
        case AttributeKind::kMustUse:
            return "@must_use";
        case AttributeKind::kSize:
            return "@size";
        case AttributeKind::kStage:
            return "@stage";
        case AttributeKind::kStride:
            return "@stride";
        case AttributeKind::kWorkgroup:
            return "@workgroup_size";
        case AttributeKind::kBindingAndGroup:
            return "@binding";
    }
    return "<unknown>";
}
namespace FunctionInputAndOutputTests {
using FunctionParameterAttributeTest = TestWithParams;
TEST_P(FunctionParameterAttributeTest, IsValid) {
    auto& params = GetParam();

    Func("main",
         utils::Vector{
             Param("a", ty.vec4<f32>(), createAttributes({}, *this, params.kind)),
         },
         ty.void_(), utils::Empty);

    if (params.should_pass) {
        EXPECT_TRUE(r()->Resolve()) << r()->error();
    } else if (params.kind == AttributeKind::kLocation || params.kind == AttributeKind::kBuiltin ||
               params.kind == AttributeKind::kInvariant ||
               params.kind == AttributeKind::kInterpolate) {
        EXPECT_FALSE(r()->Resolve());
        EXPECT_EQ(r()->error(), "error: " + name(params.kind) +
                                    " is not valid for non-entry point function parameters");
    } else {
        EXPECT_FALSE(r()->Resolve());
        EXPECT_EQ(r()->error(),
                  "error: " + name(params.kind) + " is not valid for function parameters");
    }
}
INSTANTIATE_TEST_SUITE_P(ResolverAttributeValidationTest,
                         FunctionParameterAttributeTest,
                         testing::Values(TestParams{AttributeKind::kAlign, false},
                                         TestParams{AttributeKind::kBinding, false},
                                         TestParams{AttributeKind::kBuiltin, false},
                                         TestParams{AttributeKind::kDiagnostic, false},
                                         TestParams{AttributeKind::kGroup, false},
                                         TestParams{AttributeKind::kId, false},
                                         TestParams{AttributeKind::kInterpolate, false},
                                         TestParams{AttributeKind::kInvariant, false},
                                         TestParams{AttributeKind::kLocation, false},
                                         TestParams{AttributeKind::kMustUse, false},
                                         TestParams{AttributeKind::kOffset, false},
                                         TestParams{AttributeKind::kSize, false},
                                         TestParams{AttributeKind::kStage, false},
                                         TestParams{AttributeKind::kStride, false},
                                         TestParams{AttributeKind::kWorkgroup, false},
                                         TestParams{AttributeKind::kBindingAndGroup, false}));

using FunctionReturnTypeAttributeTest = TestWithParams;
TEST_P(FunctionReturnTypeAttributeTest, IsValid) {
    auto& params = GetParam();

    Func("main", utils::Empty, ty.f32(),
         utils::Vector{
             Return(1_f),
         },
         utils::Empty, createAttributes({}, *this, params.kind));

    if (params.should_pass) {
        EXPECT_TRUE(r()->Resolve()) << r()->error();
    } else {
        EXPECT_FALSE(r()->Resolve());
        EXPECT_EQ(r()->error(), "error: " + name(params.kind) +
                                    " is not valid for non-entry point function "
                                    "return types");
    }
}
INSTANTIATE_TEST_SUITE_P(ResolverAttributeValidationTest,
                         FunctionReturnTypeAttributeTest,
                         testing::Values(TestParams{AttributeKind::kAlign, false},
                                         TestParams{AttributeKind::kBinding, false},
                                         TestParams{AttributeKind::kBuiltin, false},
                                         TestParams{AttributeKind::kDiagnostic, false},
                                         TestParams{AttributeKind::kGroup, false},
                                         TestParams{AttributeKind::kId, false},
                                         TestParams{AttributeKind::kInterpolate, false},
                                         TestParams{AttributeKind::kInvariant, false},
                                         TestParams{AttributeKind::kLocation, false},
                                         TestParams{AttributeKind::kMustUse, false},
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
    Func("main",
         utils::Vector{
             Param("a", ty.vec4<f32>(), createAttributes(Source{{12, 34}}, *this, params.kind)),
         },
         ty.void_(), utils::Empty,
         utils::Vector{
             Stage(ast::PipelineStage::kCompute),
             WorkgroupSize(1_i),
         });

    if (params.should_pass) {
        EXPECT_TRUE(r()->Resolve()) << r()->error();
    } else {
        EXPECT_FALSE(r()->Resolve());
        if (params.kind == AttributeKind::kBuiltin) {
            EXPECT_EQ(
                r()->error(),
                R"(12:34 error: @builtin(position) cannot be used in input of compute pipeline stage)");
        } else if (params.kind == AttributeKind::kInterpolate ||
                   params.kind == AttributeKind::kLocation ||
                   params.kind == AttributeKind::kInvariant) {
            EXPECT_EQ(r()->error(), "12:34 error: " + name(params.kind) +
                                        " is not valid for compute shader inputs");
        } else {
            EXPECT_EQ(r()->error(), "12:34 error: " + name(params.kind) +
                                        " is not valid for function parameters");
        }
    }
}
INSTANTIATE_TEST_SUITE_P(ResolverAttributeValidationTest,
                         ComputeShaderParameterAttributeTest,
                         testing::Values(TestParams{AttributeKind::kAlign, false},
                                         TestParams{AttributeKind::kBinding, false},
                                         TestParams{AttributeKind::kBuiltin, false},
                                         TestParams{AttributeKind::kDiagnostic, false},
                                         TestParams{AttributeKind::kGroup, false},
                                         TestParams{AttributeKind::kId, false},
                                         TestParams{AttributeKind::kInterpolate, false},
                                         TestParams{AttributeKind::kInvariant, false},
                                         TestParams{AttributeKind::kLocation, false},
                                         TestParams{AttributeKind::kMustUse, false},
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
        attrs.Push(Builtin(Source{{34, 56}}, builtin::BuiltinValue::kPosition));
    }
    auto* p = Param("a", ty.vec4<f32>(), attrs);
    Func("frag_main", utils::Vector{p}, ty.void_(), utils::Empty,
         utils::Vector{
             Stage(ast::PipelineStage::kFragment),
         });

    if (params.should_pass) {
        EXPECT_TRUE(r()->Resolve()) << r()->error();
    } else {
        EXPECT_FALSE(r()->Resolve());
        EXPECT_EQ(r()->error(),
                  "12:34 error: " + name(params.kind) + " is not valid for function parameters");
    }
}
INSTANTIATE_TEST_SUITE_P(ResolverAttributeValidationTest,
                         FragmentShaderParameterAttributeTest,
                         testing::Values(TestParams{AttributeKind::kAlign, false},
                                         TestParams{AttributeKind::kBinding, false},
                                         TestParams{AttributeKind::kBuiltin, true},
                                         TestParams{AttributeKind::kDiagnostic, false},
                                         TestParams{AttributeKind::kGroup, false},
                                         TestParams{AttributeKind::kId, false},
                                         // kInterpolate tested separately (requires @location)
                                         TestParams{AttributeKind::kInvariant, true},
                                         TestParams{AttributeKind::kLocation, true},
                                         TestParams{AttributeKind::kMustUse, false},
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
        attrs.Push(Location(Source{{34, 56}}, 2_a));
    }
    auto* p = Param("a", ty.vec4<f32>(), attrs);
    Func("vertex_main", utils::Vector{p}, ty.vec4<f32>(),
         utils::Vector{
             Return(Call(ty.vec4<f32>())),
         },
         utils::Vector{
             Stage(ast::PipelineStage::kVertex),
         },
         utils::Vector{
             Builtin(builtin::BuiltinValue::kPosition),
         });

    if (params.should_pass) {
        EXPECT_TRUE(r()->Resolve()) << r()->error();
    } else {
        EXPECT_FALSE(r()->Resolve());
        if (params.kind == AttributeKind::kBuiltin) {
            EXPECT_EQ(
                r()->error(),
                R"(12:34 error: @builtin(position) cannot be used in input of vertex pipeline stage)");
        } else if (params.kind == AttributeKind::kInvariant) {
            EXPECT_EQ(r()->error(),
                      "12:34 error: invariant attribute must only be applied to a "
                      "position builtin");
        } else {
            EXPECT_EQ(r()->error(), "12:34 error: " + name(params.kind) +
                                        " is not valid for function parameters");
        }
    }
}
INSTANTIATE_TEST_SUITE_P(ResolverAttributeValidationTest,
                         VertexShaderParameterAttributeTest,
                         testing::Values(TestParams{AttributeKind::kAlign, false},
                                         TestParams{AttributeKind::kBinding, false},
                                         TestParams{AttributeKind::kBuiltin, false},
                                         TestParams{AttributeKind::kDiagnostic, false},
                                         TestParams{AttributeKind::kGroup, false},
                                         TestParams{AttributeKind::kId, false},
                                         TestParams{AttributeKind::kInterpolate, true},
                                         TestParams{AttributeKind::kInvariant, false},
                                         TestParams{AttributeKind::kLocation, true},
                                         TestParams{AttributeKind::kMustUse, false},
                                         TestParams{AttributeKind::kOffset, false},
                                         TestParams{AttributeKind::kSize, false},
                                         TestParams{AttributeKind::kStage, false},
                                         TestParams{AttributeKind::kStride, false},
                                         TestParams{AttributeKind::kWorkgroup, false},
                                         TestParams{AttributeKind::kBindingAndGroup, false}));

using ComputeShaderReturnTypeAttributeTest = TestWithParams;
TEST_P(ComputeShaderReturnTypeAttributeTest, IsValid) {
    auto& params = GetParam();
    Func("main", utils::Empty, ty.vec4<f32>(),
         utils::Vector{
             Return(Call(ty.vec4<f32>(), 1_f)),
         },
         utils::Vector{
             Stage(ast::PipelineStage::kCompute),
             WorkgroupSize(1_i),
         },
         createAttributes(Source{{12, 34}}, *this, params.kind));

    if (params.should_pass) {
        EXPECT_TRUE(r()->Resolve()) << r()->error();
    } else {
        EXPECT_FALSE(r()->Resolve());
        if (params.kind == AttributeKind::kBuiltin) {
            EXPECT_EQ(
                r()->error(),
                R"(12:34 error: @builtin(position) cannot be used in output of compute pipeline stage)");
        } else if (params.kind == AttributeKind::kInterpolate ||
                   params.kind == AttributeKind::kLocation ||
                   params.kind == AttributeKind::kInvariant) {
            EXPECT_EQ(r()->error(), "12:34 error: " + name(params.kind) +
                                        " is not valid for compute shader output");
        } else {
            EXPECT_EQ(r()->error(), "12:34 error: " + name(params.kind) +
                                        " is not valid for entry point return "
                                        "types");
        }
    }
}
INSTANTIATE_TEST_SUITE_P(ResolverAttributeValidationTest,
                         ComputeShaderReturnTypeAttributeTest,
                         testing::Values(TestParams{AttributeKind::kAlign, false},
                                         TestParams{AttributeKind::kBinding, false},
                                         TestParams{AttributeKind::kBuiltin, false},
                                         TestParams{AttributeKind::kDiagnostic, false},
                                         TestParams{AttributeKind::kGroup, false},
                                         TestParams{AttributeKind::kId, false},
                                         TestParams{AttributeKind::kInterpolate, false},
                                         TestParams{AttributeKind::kInvariant, false},
                                         TestParams{AttributeKind::kLocation, false},
                                         TestParams{AttributeKind::kMustUse, false},
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
    attrs.Push(Location(Source{{34, 56}}, 2_a));
    Func("frag_main", utils::Empty, ty.vec4<f32>(), utils::Vector{Return(Call(ty.vec4<f32>()))},
         utils::Vector{
             Stage(ast::PipelineStage::kFragment),
         },
         attrs);

    if (params.should_pass) {
        EXPECT_TRUE(r()->Resolve()) << r()->error();
    } else {
        EXPECT_FALSE(r()->Resolve());
        if (params.kind == AttributeKind::kBuiltin) {
            EXPECT_EQ(
                r()->error(),
                R"(12:34 error: @builtin(position) cannot be used in output of fragment pipeline stage)");
        } else if (params.kind == AttributeKind::kInvariant) {
            EXPECT_EQ(
                r()->error(),
                R"(12:34 error: invariant attribute must only be applied to a position builtin)");
        } else if (params.kind == AttributeKind::kLocation) {
            EXPECT_EQ(r()->error(),
                      R"(34:56 error: duplicate location attribute
12:34 note: first attribute declared here)");
        } else {
            EXPECT_EQ(r()->error(), "12:34 error: " + name(params.kind) +
                                        " is not valid for entry point return types");
        }
    }
}
INSTANTIATE_TEST_SUITE_P(ResolverAttributeValidationTest,
                         FragmentShaderReturnTypeAttributeTest,
                         testing::Values(TestParams{AttributeKind::kAlign, false},
                                         TestParams{AttributeKind::kBinding, false},
                                         TestParams{AttributeKind::kBuiltin, false},
                                         TestParams{AttributeKind::kDiagnostic, false},
                                         TestParams{AttributeKind::kGroup, false},
                                         TestParams{AttributeKind::kId, false},
                                         TestParams{AttributeKind::kInterpolate, true},
                                         TestParams{AttributeKind::kInvariant, false},
                                         TestParams{AttributeKind::kLocation, false},
                                         TestParams{AttributeKind::kMustUse, false},
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
        attrs.Push(Builtin(Source{{34, 56}}, builtin::BuiltinValue::kPosition));
    }
    Func("vertex_main", utils::Empty, ty.vec4<f32>(),
         utils::Vector{
             Return(Call(ty.vec4<f32>())),
         },
         utils::Vector{
             Stage(ast::PipelineStage::kVertex),
         },
         attrs);

    if (params.should_pass) {
        EXPECT_TRUE(r()->Resolve()) << r()->error();
    } else {
        EXPECT_FALSE(r()->Resolve());
        if (params.kind == AttributeKind::kLocation) {
            EXPECT_EQ(r()->error(),
                      R"(34:56 error: multiple entry point IO attributes
12:34 note: previously consumed @location)");
        } else {
            EXPECT_EQ(r()->error(), "12:34 error: " + name(params.kind) +
                                        " is not valid for entry point return types");
        }
    }
}
INSTANTIATE_TEST_SUITE_P(ResolverAttributeValidationTest,
                         VertexShaderReturnTypeAttributeTest,
                         testing::Values(TestParams{AttributeKind::kAlign, false},
                                         TestParams{AttributeKind::kBinding, false},
                                         TestParams{AttributeKind::kBuiltin, true},
                                         TestParams{AttributeKind::kDiagnostic, false},
                                         TestParams{AttributeKind::kGroup, false},
                                         TestParams{AttributeKind::kId, false},
                                         // kInterpolate tested separately (requires @location)
                                         TestParams{AttributeKind::kInvariant, true},
                                         TestParams{AttributeKind::kLocation, false},
                                         TestParams{AttributeKind::kMustUse, false},
                                         TestParams{AttributeKind::kOffset, false},
                                         TestParams{AttributeKind::kSize, false},
                                         TestParams{AttributeKind::kStage, false},
                                         TestParams{AttributeKind::kStride, false},
                                         TestParams{AttributeKind::kWorkgroup, false},
                                         TestParams{AttributeKind::kBindingAndGroup, false}));

using EntryPointParameterAttributeTest = TestWithParams;
TEST_F(EntryPointParameterAttributeTest, DuplicateAttribute) {
    Func("main", utils::Empty, ty.f32(),
         utils::Vector{
             Return(1_f),
         },
         utils::Vector{
             Stage(ast::PipelineStage::kFragment),
         },
         utils::Vector{
             Location(Source{{12, 34}}, 2_a),
             Location(Source{{56, 78}}, 3_a),
         });

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(),
              R"(56:78 error: duplicate location attribute
12:34 note: first attribute declared here)");
}

TEST_F(EntryPointParameterAttributeTest, DuplicateInternalAttribute) {
    auto* s = Param("s", ty.sampler(type::SamplerKind::kSampler),
                    utils::Vector{
                        Binding(0_a),
                        Group(0_a),
                        Disable(ast::DisabledValidation::kBindingPointCollision),
                        Disable(ast::DisabledValidation::kEntryPointParameter),
                    });
    Func("f", utils::Vector{s}, ty.void_(), utils::Empty,
         utils::Vector{
             Stage(ast::PipelineStage::kFragment),
         });

    EXPECT_TRUE(r()->Resolve()) << r()->error();
}

using EntryPointReturnTypeAttributeTest = ResolverTest;
TEST_F(EntryPointReturnTypeAttributeTest, DuplicateAttribute) {
    Func("main", utils::Empty, ty.f32(),
         utils::Vector{
             Return(1_f),
         },
         utils::Vector{
             Stage(ast::PipelineStage::kFragment),
         },
         utils::Vector{
             Location(Source{{12, 34}}, 2_a),
             Location(Source{{56, 78}}, 3_a),
         });

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(),
              R"(56:78 error: duplicate location attribute
12:34 note: first attribute declared here)");
}

TEST_F(EntryPointReturnTypeAttributeTest, DuplicateInternalAttribute) {
    Func("f", utils::Empty, ty.i32(), utils::Vector{Return(1_i)},
         utils::Vector{
             Stage(ast::PipelineStage::kFragment),
         },
         utils::Vector{
             Disable(ast::DisabledValidation::kBindingPointCollision),
             Disable(ast::DisabledValidation::kEntryPointParameter),
         });

    EXPECT_TRUE(r()->Resolve()) << r()->error();
}
}  // namespace EntryPointInputAndOutputTests

namespace StructAndStructMemberTests {
using StructAttributeTest = TestWithParams;
using SpirvBlockAttribute = transform::AddBlockAttribute::BlockAttribute;
TEST_P(StructAttributeTest, IsValid) {
    auto& params = GetParam();

    Structure("mystruct", utils::Vector{Member("a", ty.f32())},
              createAttributes(Source{{12, 34}}, *this, params.kind));

    if (params.should_pass) {
        EXPECT_TRUE(r()->Resolve()) << r()->error();
    } else {
        EXPECT_FALSE(r()->Resolve());
        EXPECT_EQ(r()->error(),
                  "12:34 error: " + name(params.kind) + " is not valid for struct declarations");
    }
}
INSTANTIATE_TEST_SUITE_P(ResolverAttributeValidationTest,
                         StructAttributeTest,
                         testing::Values(TestParams{AttributeKind::kAlign, false},
                                         TestParams{AttributeKind::kBinding, false},
                                         TestParams{AttributeKind::kBuiltin, false},
                                         TestParams{AttributeKind::kDiagnostic, false},
                                         TestParams{AttributeKind::kGroup, false},
                                         TestParams{AttributeKind::kId, false},
                                         TestParams{AttributeKind::kInterpolate, false},
                                         TestParams{AttributeKind::kInvariant, false},
                                         TestParams{AttributeKind::kLocation, false},
                                         TestParams{AttributeKind::kMustUse, false},
                                         TestParams{AttributeKind::kOffset, false},
                                         TestParams{AttributeKind::kSize, false},
                                         TestParams{AttributeKind::kStage, false},
                                         TestParams{AttributeKind::kStride, false},
                                         TestParams{AttributeKind::kWorkgroup, false},
                                         TestParams{AttributeKind::kBindingAndGroup, false}));

using StructMemberAttributeTest = TestWithParams;
TEST_P(StructMemberAttributeTest, IsValid) {
    auto& params = GetParam();
    utils::Vector<const ast::StructMember*, 1> members;
    if (params.kind == AttributeKind::kBuiltin) {
        members.Push(
            Member("a", ty.vec4<f32>(), createAttributes(Source{{12, 34}}, *this, params.kind)));
    } else {
        members.Push(Member("a", ty.f32(), createAttributes(Source{{12, 34}}, *this, params.kind)));
    }
    Structure("mystruct", members);
    if (params.should_pass) {
        EXPECT_TRUE(r()->Resolve()) << r()->error();
    } else {
        EXPECT_FALSE(r()->Resolve());
        EXPECT_EQ(r()->error(),
                  "12:34 error: " + name(params.kind) + " is not valid for struct members");
    }
}
INSTANTIATE_TEST_SUITE_P(ResolverAttributeValidationTest,
                         StructMemberAttributeTest,
                         testing::Values(TestParams{AttributeKind::kAlign, true},
                                         TestParams{AttributeKind::kBinding, false},
                                         TestParams{AttributeKind::kBuiltin, true},
                                         TestParams{AttributeKind::kDiagnostic, false},
                                         TestParams{AttributeKind::kGroup, false},
                                         TestParams{AttributeKind::kId, false},
                                         // kInterpolate tested separately (requires @location)
                                         // kInvariant tested separately (requires position builtin)
                                         TestParams{AttributeKind::kLocation, true},
                                         TestParams{AttributeKind::kMustUse, false},
                                         TestParams{AttributeKind::kOffset, true},
                                         TestParams{AttributeKind::kSize, true},
                                         TestParams{AttributeKind::kStage, false},
                                         TestParams{AttributeKind::kStride, false},
                                         TestParams{AttributeKind::kWorkgroup, false},
                                         TestParams{AttributeKind::kBindingAndGroup, false}));
TEST_F(StructMemberAttributeTest, DuplicateAttribute) {
    Structure("mystruct", utils::Vector{
                              Member("a", ty.i32(),
                                     utils::Vector{
                                         MemberAlign(Source{{12, 34}}, 4_i),
                                         MemberAlign(Source{{56, 78}}, 8_i),
                                     }),
                          });
    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(),
              R"(56:78 error: duplicate align attribute
12:34 note: first attribute declared here)");
}
TEST_F(StructMemberAttributeTest, InvariantAttributeWithPosition) {
    Structure("mystruct", utils::Vector{
                              Member("a", ty.vec4<f32>(),
                                     utils::Vector{
                                         Invariant(),
                                         Builtin(builtin::BuiltinValue::kPosition),
                                     }),
                          });
    EXPECT_TRUE(r()->Resolve()) << r()->error();
}
TEST_F(StructMemberAttributeTest, InvariantAttributeWithoutPosition) {
    Structure("mystruct", utils::Vector{
                              Member("a", ty.vec4<f32>(),
                                     utils::Vector{
                                         Invariant(Source{{12, 34}}),
                                     }),
                          });
    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(),
              "12:34 error: invariant attribute must only be applied to a "
              "position builtin");
}

TEST_F(StructMemberAttributeTest, Align_Attribute_Const) {
    GlobalConst("val", ty.i32(), Expr(1_i));

    Structure("mystruct", utils::Vector{Member("a", ty.f32(), utils::Vector{MemberAlign("val")})});
    EXPECT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(StructMemberAttributeTest, Align_Attribute_ConstNegative) {
    GlobalConst("val", ty.i32(), Expr(-2_i));

    Structure("mystruct", utils::Vector{Member(
                              "a", ty.f32(), utils::Vector{MemberAlign(Source{{12, 34}}, "val")})});
    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(),
              R"(12:34 error: @align value must be a positive, power-of-two integer)");
}

TEST_F(StructMemberAttributeTest, Align_Attribute_ConstPowerOfTwo) {
    GlobalConst("val", ty.i32(), Expr(3_i));

    Structure("mystruct", utils::Vector{Member(
                              "a", ty.f32(), utils::Vector{MemberAlign(Source{{12, 34}}, "val")})});
    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(),
              R"(12:34 error: @align value must be a positive, power-of-two integer)");
}

TEST_F(StructMemberAttributeTest, Align_Attribute_ConstF32) {
    GlobalConst("val", ty.f32(), Expr(1.23_f));

    Structure("mystruct", utils::Vector{Member(
                              "a", ty.f32(), utils::Vector{MemberAlign(Source{{12, 34}}, "val")})});
    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), R"(12:34 error: @align must be an i32 or u32 value)");
}

TEST_F(StructMemberAttributeTest, Align_Attribute_ConstU32) {
    GlobalConst("val", ty.u32(), Expr(2_u));

    Structure("mystruct", utils::Vector{Member(
                              "a", ty.f32(), utils::Vector{MemberAlign(Source{{12, 34}}, "val")})});
    EXPECT_TRUE(r()->Resolve());
}

TEST_F(StructMemberAttributeTest, Align_Attribute_ConstAInt) {
    GlobalConst("val", Expr(2_a));

    Structure("mystruct", utils::Vector{Member(
                              "a", ty.f32(), utils::Vector{MemberAlign(Source{{12, 34}}, "val")})});
    EXPECT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(StructMemberAttributeTest, Align_Attribute_ConstAFloat) {
    GlobalConst("val", Expr(2.0_a));

    Structure("mystruct", utils::Vector{Member(
                              "a", ty.f32(), utils::Vector{MemberAlign(Source{{12, 34}}, "val")})});
    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), R"(12:34 error: @align must be an i32 or u32 value)");
}

TEST_F(StructMemberAttributeTest, Align_Attribute_Var) {
    GlobalVar(Source{{1, 2}}, "val", ty.f32(), builtin::AddressSpace::kPrivate,
              builtin::Access::kUndefined, Expr(1.23_f));

    Structure(Source{{6, 4}}, "mystruct",
              utils::Vector{Member(Source{{12, 5}}, "a", ty.f32(),
                                   utils::Vector{MemberAlign(Expr(Source{{12, 35}}, "val"))})});
    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), R"(12:35 error: var 'val' cannot be referenced at module-scope
1:2 note: var 'val' declared here)");
}

TEST_F(StructMemberAttributeTest, Align_Attribute_Override) {
    Override("val", ty.f32(), Expr(1.23_f));

    Structure("mystruct",
              utils::Vector{Member("a", ty.f32(),
                                   utils::Vector{MemberAlign(Expr(Source{{12, 34}}, "val"))})});
    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(
        r()->error(),
        R"(12:34 error: @align requires a const-expression, but expression is an override-expression)");
}

TEST_F(StructMemberAttributeTest, Size_Attribute_Const) {
    GlobalConst("val", ty.i32(), Expr(4_i));

    Structure("mystruct", utils::Vector{Member("a", ty.f32(), utils::Vector{MemberSize("val")})});
    EXPECT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(StructMemberAttributeTest, Size_Attribute_ConstNegative) {
    GlobalConst("val", ty.i32(), Expr(-2_i));

    Structure("mystruct", utils::Vector{Member(
                              "a", ty.f32(), utils::Vector{MemberSize(Source{{12, 34}}, "val")})});
    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), R"(12:34 error: @size must be a positive integer)");
}

TEST_F(StructMemberAttributeTest, Size_Attribute_ConstF32) {
    GlobalConst("val", ty.f32(), Expr(1.23_f));

    Structure("mystruct", utils::Vector{Member(
                              "a", ty.f32(), utils::Vector{MemberSize(Source{{12, 34}}, "val")})});
    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), R"(12:34 error: @size must be an i32 or u32 value)");
}

TEST_F(StructMemberAttributeTest, Size_Attribute_ConstU32) {
    GlobalConst("val", ty.u32(), Expr(4_u));

    Structure("mystruct", utils::Vector{Member(
                              "a", ty.f32(), utils::Vector{MemberSize(Source{{12, 34}}, "val")})});
    EXPECT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(StructMemberAttributeTest, Size_Attribute_ConstAInt) {
    GlobalConst("val", Expr(4_a));

    Structure("mystruct", utils::Vector{Member(
                              "a", ty.f32(), utils::Vector{MemberSize(Source{{12, 34}}, "val")})});
    EXPECT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(StructMemberAttributeTest, Size_Attribute_ConstAFloat) {
    GlobalConst("val", Expr(2.0_a));

    Structure("mystruct", utils::Vector{Member(
                              "a", ty.f32(), utils::Vector{MemberSize(Source{{12, 34}}, "val")})});
    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), R"(12:34 error: @size must be an i32 or u32 value)");
}

TEST_F(StructMemberAttributeTest, Size_Attribute_Var) {
    GlobalVar(Source{{1, 2}}, "val", ty.f32(), builtin::AddressSpace::kPrivate,
              builtin::Access::kUndefined, Expr(1.23_f));

    Structure(Source{{6, 4}}, "mystruct",
              utils::Vector{Member(Source{{12, 5}}, "a", ty.f32(),
                                   utils::Vector{MemberSize(Expr(Source{{12, 35}}, "val"))})});
    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), R"(12:35 error: var 'val' cannot be referenced at module-scope
1:2 note: var 'val' declared here)");
}

TEST_F(StructMemberAttributeTest, Size_Attribute_Override) {
    Override("val", ty.f32(), Expr(1.23_f));

    Structure("mystruct",
              utils::Vector{
                  Member("a", ty.f32(), utils::Vector{MemberSize(Expr(Source{{12, 34}}, "val"))}),
              });
    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(
        r()->error(),
        R"(12:34 error: @size requires a const-expression, but expression is an override-expression)");
}

TEST_F(StructMemberAttributeTest, Size_On_RuntimeSizedArray) {
    Structure("mystruct",
              utils::Vector{
                  Member("a", ty.array<i32>(), utils::Vector{MemberSize(Source{{12, 34}}, 8_a)}),
              });
    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(
        r()->error(),
        R"(12:34 error: @size can only be applied to members where the member's type size can be fully determined at shader creation time)");
}

}  // namespace StructAndStructMemberTests

using ArrayAttributeTest = TestWithParams;
TEST_P(ArrayAttributeTest, IsValid) {
    auto& params = GetParam();

    auto arr = ty.array(ty.f32(), createAttributes(Source{{12, 34}}, *this, params.kind));
    Structure("mystruct", utils::Vector{
                              Member("a", arr),
                          });

    if (params.should_pass) {
        EXPECT_TRUE(r()->Resolve()) << r()->error();
    } else {
        EXPECT_FALSE(r()->Resolve());
        EXPECT_EQ(r()->error(),
                  "12:34 error: " + name(params.kind) + " is not valid for array types");
    }
}
INSTANTIATE_TEST_SUITE_P(ResolverAttributeValidationTest,
                         ArrayAttributeTest,
                         testing::Values(TestParams{AttributeKind::kAlign, false},
                                         TestParams{AttributeKind::kBinding, false},
                                         TestParams{AttributeKind::kBuiltin, false},
                                         TestParams{AttributeKind::kDiagnostic, false},
                                         TestParams{AttributeKind::kGroup, false},
                                         TestParams{AttributeKind::kId, false},
                                         TestParams{AttributeKind::kInterpolate, false},
                                         TestParams{AttributeKind::kInvariant, false},
                                         TestParams{AttributeKind::kLocation, false},
                                         TestParams{AttributeKind::kMustUse, false},
                                         TestParams{AttributeKind::kOffset, false},
                                         TestParams{AttributeKind::kSize, false},
                                         TestParams{AttributeKind::kStage, false},
                                         TestParams{AttributeKind::kStride, true},
                                         TestParams{AttributeKind::kWorkgroup, false},
                                         TestParams{AttributeKind::kBindingAndGroup, false}));

using VariableAttributeTest = TestWithParams;
TEST_P(VariableAttributeTest, IsValid) {
    auto& params = GetParam();

    auto attrs = createAttributes(Source{{12, 34}}, *this, params.kind);
    if (IsBindingAttribute(params.kind)) {
        GlobalVar("a", ty.sampler(type::SamplerKind::kSampler), attrs);
    } else {
        GlobalVar("a", ty.f32(), builtin::AddressSpace::kPrivate, attrs);
    }

    if (params.should_pass) {
        EXPECT_TRUE(r()->Resolve()) << r()->error();
    } else {
        EXPECT_FALSE(r()->Resolve());
        if (!IsBindingAttribute(params.kind)) {
            EXPECT_EQ(r()->error(),
                      "12:34 error: " + name(params.kind) + " is not valid for module-scope 'var'");
        }
    }
}
INSTANTIATE_TEST_SUITE_P(ResolverAttributeValidationTest,
                         VariableAttributeTest,
                         testing::Values(TestParams{AttributeKind::kAlign, false},
                                         TestParams{AttributeKind::kBinding, false},
                                         TestParams{AttributeKind::kBuiltin, false},
                                         TestParams{AttributeKind::kDiagnostic, false},
                                         TestParams{AttributeKind::kGroup, false},
                                         TestParams{AttributeKind::kId, false},
                                         TestParams{AttributeKind::kInterpolate, false},
                                         TestParams{AttributeKind::kInvariant, false},
                                         TestParams{AttributeKind::kLocation, false},
                                         TestParams{AttributeKind::kMustUse, false},
                                         TestParams{AttributeKind::kOffset, false},
                                         TestParams{AttributeKind::kSize, false},
                                         TestParams{AttributeKind::kStage, false},
                                         TestParams{AttributeKind::kStride, false},
                                         TestParams{AttributeKind::kWorkgroup, false},
                                         TestParams{AttributeKind::kBindingAndGroup, true}));

TEST_F(VariableAttributeTest, DuplicateAttribute) {
    GlobalVar("a", ty.sampler(type::SamplerKind::kSampler), Binding(Source{{12, 34}}, 2_a),
              Group(2_a), Binding(Source{{56, 78}}, 3_a));

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(),
              R"(56:78 error: duplicate binding attribute
12:34 note: first attribute declared here)");
}

TEST_F(VariableAttributeTest, LocalVar) {
    auto* v = Var("a", ty.f32(), utils::Vector{Binding(Source{{12, 34}}, 2_a)});

    WrapInFunction(v);

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), "12:34 error: @binding is not valid for function-scope 'var'");
}

TEST_F(VariableAttributeTest, LocalLet) {
    auto* v = Let("a", utils::Vector{Binding(Source{{12, 34}}, 2_a)}, Expr(1_a));

    WrapInFunction(v);

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), "12:34 error: @binding is not valid for 'let' declaration");
}

using ConstantAttributeTest = TestWithParams;
TEST_P(ConstantAttributeTest, IsValid) {
    auto& params = GetParam();

    GlobalConst("a", ty.f32(), Expr(1.23_f),
                createAttributes(Source{{12, 34}}, *this, params.kind));

    if (params.should_pass) {
        EXPECT_TRUE(r()->Resolve()) << r()->error();
    } else {
        EXPECT_FALSE(r()->Resolve());
        EXPECT_EQ(r()->error(),
                  "12:34 error: " + name(params.kind) + " is not valid for 'const' declaration");
    }
}
INSTANTIATE_TEST_SUITE_P(ResolverAttributeValidationTest,
                         ConstantAttributeTest,
                         testing::Values(TestParams{AttributeKind::kAlign, false},
                                         TestParams{AttributeKind::kBinding, false},
                                         TestParams{AttributeKind::kBuiltin, false},
                                         TestParams{AttributeKind::kDiagnostic, false},
                                         TestParams{AttributeKind::kGroup, false},
                                         TestParams{AttributeKind::kId, false},
                                         TestParams{AttributeKind::kInterpolate, false},
                                         TestParams{AttributeKind::kInvariant, false},
                                         TestParams{AttributeKind::kLocation, false},
                                         TestParams{AttributeKind::kMustUse, false},
                                         TestParams{AttributeKind::kOffset, false},
                                         TestParams{AttributeKind::kSize, false},
                                         TestParams{AttributeKind::kStage, false},
                                         TestParams{AttributeKind::kStride, false},
                                         TestParams{AttributeKind::kWorkgroup, false},
                                         TestParams{AttributeKind::kBindingAndGroup, false}));

TEST_F(ConstantAttributeTest, InvalidAttribute) {
    GlobalConst("a", ty.f32(), Expr(1.23_f),
                utils::Vector{
                    Id(Source{{12, 34}}, 0_a),
                });

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), "12:34 error: @id is not valid for 'const' declaration");
}

using OverrideAttributeTest = TestWithParams;
TEST_P(OverrideAttributeTest, IsValid) {
    auto& params = GetParam();

    Override("a", ty.f32(), Expr(1.23_f), createAttributes(Source{{12, 34}}, *this, params.kind));

    if (params.should_pass) {
        EXPECT_TRUE(r()->Resolve()) << r()->error();
    } else {
        EXPECT_FALSE(r()->Resolve());
        EXPECT_EQ(r()->error(),
                  "12:34 error: " + name(params.kind) + " is not valid for 'override' declaration");
    }
}
INSTANTIATE_TEST_SUITE_P(ResolverAttributeValidationTest,
                         OverrideAttributeTest,
                         testing::Values(TestParams{AttributeKind::kAlign, false},
                                         TestParams{AttributeKind::kBinding, false},
                                         TestParams{AttributeKind::kBuiltin, false},
                                         TestParams{AttributeKind::kDiagnostic, false},
                                         TestParams{AttributeKind::kGroup, false},
                                         TestParams{AttributeKind::kId, true},
                                         TestParams{AttributeKind::kInterpolate, false},
                                         TestParams{AttributeKind::kInvariant, false},
                                         TestParams{AttributeKind::kLocation, false},
                                         TestParams{AttributeKind::kMustUse, false},
                                         TestParams{AttributeKind::kOffset, false},
                                         TestParams{AttributeKind::kSize, false},
                                         TestParams{AttributeKind::kStage, false},
                                         TestParams{AttributeKind::kStride, false},
                                         TestParams{AttributeKind::kWorkgroup, false},
                                         TestParams{AttributeKind::kBindingAndGroup, false}));

TEST_F(OverrideAttributeTest, DuplicateAttribute) {
    Override("a", ty.f32(), Expr(1.23_f),
             utils::Vector{
                 Id(Source{{12, 34}}, 0_a),
                 Id(Source{{56, 78}}, 1_a),
             });

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(),
              R"(56:78 error: duplicate id attribute
12:34 note: first attribute declared here)");
}

using SwitchStatementAttributeTest = TestWithParams;
TEST_P(SwitchStatementAttributeTest, IsValid) {
    auto& params = GetParam();

    WrapInFunction(Switch(Expr(0_a), utils::Vector{DefaultCase()},
                          createAttributes(Source{{12, 34}}, *this, params.kind)));

    if (params.should_pass) {
        EXPECT_TRUE(r()->Resolve()) << r()->error();
    } else {
        EXPECT_FALSE(r()->Resolve());
        EXPECT_EQ(r()->error(),
                  "12:34 error: " + name(params.kind) + " is not valid for switch statements");
    }
}
INSTANTIATE_TEST_SUITE_P(ResolverAttributeValidationTest,
                         SwitchStatementAttributeTest,
                         testing::Values(TestParams{AttributeKind::kAlign, false},
                                         TestParams{AttributeKind::kBinding, false},
                                         TestParams{AttributeKind::kBuiltin, false},
                                         TestParams{AttributeKind::kDiagnostic, true},
                                         TestParams{AttributeKind::kGroup, false},
                                         TestParams{AttributeKind::kId, false},
                                         TestParams{AttributeKind::kInterpolate, false},
                                         TestParams{AttributeKind::kInvariant, false},
                                         TestParams{AttributeKind::kLocation, false},
                                         TestParams{AttributeKind::kMustUse, false},
                                         TestParams{AttributeKind::kOffset, false},
                                         TestParams{AttributeKind::kSize, false},
                                         TestParams{AttributeKind::kStage, false},
                                         TestParams{AttributeKind::kStride, false},
                                         TestParams{AttributeKind::kWorkgroup, false},
                                         TestParams{AttributeKind::kBindingAndGroup, false}));

using SwitchBodyAttributeTest = TestWithParams;
TEST_P(SwitchBodyAttributeTest, IsValid) {
    auto& params = GetParam();

    WrapInFunction(Switch(Expr(0_a), utils::Vector{DefaultCase()}, utils::Empty,
                          createAttributes(Source{{12, 34}}, *this, params.kind)));

    if (params.should_pass) {
        EXPECT_TRUE(r()->Resolve()) << r()->error();
    } else {
        EXPECT_FALSE(r()->Resolve());
        EXPECT_EQ(r()->error(),
                  "12:34 error: " + name(params.kind) + " is not valid for switch body");
    }
}
INSTANTIATE_TEST_SUITE_P(ResolverAttributeValidationTest,
                         SwitchBodyAttributeTest,
                         testing::Values(TestParams{AttributeKind::kAlign, false},
                                         TestParams{AttributeKind::kBinding, false},
                                         TestParams{AttributeKind::kBuiltin, false},
                                         TestParams{AttributeKind::kDiagnostic, true},
                                         TestParams{AttributeKind::kGroup, false},
                                         TestParams{AttributeKind::kId, false},
                                         TestParams{AttributeKind::kInterpolate, false},
                                         TestParams{AttributeKind::kInvariant, false},
                                         TestParams{AttributeKind::kLocation, false},
                                         TestParams{AttributeKind::kMustUse, false},
                                         TestParams{AttributeKind::kOffset, false},
                                         TestParams{AttributeKind::kSize, false},
                                         TestParams{AttributeKind::kStage, false},
                                         TestParams{AttributeKind::kStride, false},
                                         TestParams{AttributeKind::kWorkgroup, false},
                                         TestParams{AttributeKind::kBindingAndGroup, false}));

using IfStatementAttributeTest = TestWithParams;
TEST_P(IfStatementAttributeTest, IsValid) {
    auto& params = GetParam();

    WrapInFunction(If(Expr(true), Block(), ElseStmt(),
                      createAttributes(Source{{12, 34}}, *this, params.kind)));

    if (params.should_pass) {
        EXPECT_TRUE(r()->Resolve()) << r()->error();
    } else {
        EXPECT_FALSE(r()->Resolve());
        EXPECT_EQ(r()->error(),
                  "12:34 error: " + name(params.kind) + " is not valid for if statements");
    }
}
INSTANTIATE_TEST_SUITE_P(ResolverAttributeValidationTest,
                         IfStatementAttributeTest,
                         testing::Values(TestParams{AttributeKind::kAlign, false},
                                         TestParams{AttributeKind::kBinding, false},
                                         TestParams{AttributeKind::kBuiltin, false},
                                         TestParams{AttributeKind::kDiagnostic, true},
                                         TestParams{AttributeKind::kGroup, false},
                                         TestParams{AttributeKind::kId, false},
                                         TestParams{AttributeKind::kInterpolate, false},
                                         TestParams{AttributeKind::kInvariant, false},
                                         TestParams{AttributeKind::kLocation, false},
                                         TestParams{AttributeKind::kMustUse, false},
                                         TestParams{AttributeKind::kOffset, false},
                                         TestParams{AttributeKind::kSize, false},
                                         TestParams{AttributeKind::kStage, false},
                                         TestParams{AttributeKind::kStride, false},
                                         TestParams{AttributeKind::kWorkgroup, false},
                                         TestParams{AttributeKind::kBindingAndGroup, false}));

using ForStatementAttributeTest = TestWithParams;
TEST_P(ForStatementAttributeTest, IsValid) {
    auto& params = GetParam();

    WrapInFunction(For(nullptr, Expr(false), nullptr, Block(),
                       createAttributes(Source{{12, 34}}, *this, params.kind)));

    if (params.should_pass) {
        EXPECT_TRUE(r()->Resolve()) << r()->error();
    } else {
        EXPECT_FALSE(r()->Resolve());
        EXPECT_EQ(r()->error(),
                  "12:34 error: " + name(params.kind) + " is not valid for for statements");
    }
}
INSTANTIATE_TEST_SUITE_P(ResolverAttributeValidationTest,
                         ForStatementAttributeTest,
                         testing::Values(TestParams{AttributeKind::kAlign, false},
                                         TestParams{AttributeKind::kBinding, false},
                                         TestParams{AttributeKind::kBuiltin, false},
                                         TestParams{AttributeKind::kDiagnostic, true},
                                         TestParams{AttributeKind::kGroup, false},
                                         TestParams{AttributeKind::kId, false},
                                         TestParams{AttributeKind::kInterpolate, false},
                                         TestParams{AttributeKind::kInvariant, false},
                                         TestParams{AttributeKind::kLocation, false},
                                         TestParams{AttributeKind::kMustUse, false},
                                         TestParams{AttributeKind::kOffset, false},
                                         TestParams{AttributeKind::kSize, false},
                                         TestParams{AttributeKind::kStage, false},
                                         TestParams{AttributeKind::kStride, false},
                                         TestParams{AttributeKind::kWorkgroup, false},
                                         TestParams{AttributeKind::kBindingAndGroup, false}));

using LoopStatementAttributeTest = TestWithParams;
TEST_P(LoopStatementAttributeTest, IsValid) {
    auto& params = GetParam();

    WrapInFunction(
        Loop(Block(Return()), Block(), createAttributes(Source{{12, 34}}, *this, params.kind)));

    if (params.should_pass) {
        EXPECT_TRUE(r()->Resolve()) << r()->error();
    } else {
        EXPECT_FALSE(r()->Resolve());
        EXPECT_EQ(r()->error(),
                  "12:34 error: " + name(params.kind) + " is not valid for loop statements");
    }
}
INSTANTIATE_TEST_SUITE_P(ResolverAttributeValidationTest,
                         LoopStatementAttributeTest,
                         testing::Values(TestParams{AttributeKind::kAlign, false},
                                         TestParams{AttributeKind::kBinding, false},
                                         TestParams{AttributeKind::kBuiltin, false},
                                         TestParams{AttributeKind::kDiagnostic, true},
                                         TestParams{AttributeKind::kGroup, false},
                                         TestParams{AttributeKind::kId, false},
                                         TestParams{AttributeKind::kInterpolate, false},
                                         TestParams{AttributeKind::kInvariant, false},
                                         TestParams{AttributeKind::kLocation, false},
                                         TestParams{AttributeKind::kMustUse, false},
                                         TestParams{AttributeKind::kOffset, false},
                                         TestParams{AttributeKind::kSize, false},
                                         TestParams{AttributeKind::kStage, false},
                                         TestParams{AttributeKind::kStride, false},
                                         TestParams{AttributeKind::kWorkgroup, false},
                                         TestParams{AttributeKind::kBindingAndGroup, false}));

using WhileStatementAttributeTest = TestWithParams;
TEST_P(WhileStatementAttributeTest, IsValid) {
    auto& params = GetParam();

    WrapInFunction(
        While(Expr(false), Block(), createAttributes(Source{{12, 34}}, *this, params.kind)));

    if (params.should_pass) {
        EXPECT_TRUE(r()->Resolve()) << r()->error();
    } else {
        EXPECT_FALSE(r()->Resolve());
        EXPECT_EQ(r()->error(),
                  "12:34 error: " + name(params.kind) + " is not valid for while statements");
    }
}
INSTANTIATE_TEST_SUITE_P(ResolverAttributeValidationTest,
                         WhileStatementAttributeTest,
                         testing::Values(TestParams{AttributeKind::kAlign, false},
                                         TestParams{AttributeKind::kBinding, false},
                                         TestParams{AttributeKind::kBuiltin, false},
                                         TestParams{AttributeKind::kDiagnostic, true},
                                         TestParams{AttributeKind::kGroup, false},
                                         TestParams{AttributeKind::kId, false},
                                         TestParams{AttributeKind::kInterpolate, false},
                                         TestParams{AttributeKind::kInvariant, false},
                                         TestParams{AttributeKind::kLocation, false},
                                         TestParams{AttributeKind::kMustUse, false},
                                         TestParams{AttributeKind::kOffset, false},
                                         TestParams{AttributeKind::kSize, false},
                                         TestParams{AttributeKind::kStage, false},
                                         TestParams{AttributeKind::kStride, false},
                                         TestParams{AttributeKind::kWorkgroup, false},
                                         TestParams{AttributeKind::kBindingAndGroup, false}));

namespace BlockStatementTests {
class BlockStatementTest : public TestWithParams {
  protected:
    void Check() {
        if (GetParam().should_pass) {
            EXPECT_TRUE(r()->Resolve()) << r()->error();
        } else {
            EXPECT_FALSE(r()->Resolve());
            EXPECT_EQ(r()->error(),
                      "error: " + name(GetParam().kind) + " is not valid for block statements");
        }
    }
};
TEST_P(BlockStatementTest, CompoundStatement) {
    Func("foo", utils::Empty, ty.void_(),
         utils::Vector{
             Block(utils::Vector{Return()}, createAttributes({}, *this, GetParam().kind)),
         });
    Check();
}
TEST_P(BlockStatementTest, FunctionBody) {
    Func("foo", utils::Empty, ty.void_(),
         Block(utils::Vector{Return()}, createAttributes({}, *this, GetParam().kind)));
    Check();
}
TEST_P(BlockStatementTest, IfStatementBody) {
    Func("foo", utils::Empty, ty.void_(),
         utils::Vector{
             If(Expr(true),
                Block(utils::Vector{Return()}, createAttributes({}, *this, GetParam().kind))),
         });
    Check();
}
TEST_P(BlockStatementTest, ElseStatementBody) {
    Func("foo", utils::Empty, ty.void_(),
         utils::Vector{
             If(Expr(true), Block(utils::Vector{Return()}),
                Else(Block(utils::Vector{Return()}, createAttributes({}, *this, GetParam().kind)))),
         });
    Check();
}
TEST_P(BlockStatementTest, ForStatementBody) {
    Func("foo", utils::Empty, ty.void_(),
         utils::Vector{
             For(nullptr, Expr(true), nullptr,
                 Block(utils::Vector{Break()}, createAttributes({}, *this, GetParam().kind))),
         });
    Check();
}
TEST_P(BlockStatementTest, LoopStatementBody) {
    Func("foo", utils::Empty, ty.void_(),
         utils::Vector{
             Loop(Block(utils::Vector{Break()}, createAttributes({}, *this, GetParam().kind))),
         });
    Check();
}
TEST_P(BlockStatementTest, WhileStatementBody) {
    Func("foo", utils::Empty, ty.void_(),
         utils::Vector{
             While(Expr(true),
                   Block(utils::Vector{Break()}, createAttributes({}, *this, GetParam().kind))),
         });
    Check();
}
TEST_P(BlockStatementTest, CaseStatementBody) {
    Func("foo", utils::Empty, ty.void_(),
         utils::Vector{
             Switch(1_a,
                    Case(CaseSelector(1_a), Block(utils::Vector{Break()},
                                                  createAttributes({}, *this, GetParam().kind))),
                    DefaultCase(Block({}))),
         });
    Check();
}
TEST_P(BlockStatementTest, DefaultStatementBody) {
    Func("foo", utils::Empty, ty.void_(),
         utils::Vector{
             Switch(1_a, Case(CaseSelector(1_a), Block()),
                    DefaultCase(Block(utils::Vector{Break()},
                                      createAttributes({}, *this, GetParam().kind)))),
         });
    Check();
}
INSTANTIATE_TEST_SUITE_P(ResolverAttributeValidationTest,
                         BlockStatementTest,
                         testing::Values(TestParams{AttributeKind::kAlign, false},
                                         TestParams{AttributeKind::kBinding, false},
                                         TestParams{AttributeKind::kBuiltin, false},
                                         TestParams{AttributeKind::kDiagnostic, true},
                                         TestParams{AttributeKind::kGroup, false},
                                         TestParams{AttributeKind::kId, false},
                                         TestParams{AttributeKind::kInterpolate, false},
                                         TestParams{AttributeKind::kInvariant, false},
                                         TestParams{AttributeKind::kLocation, false},
                                         TestParams{AttributeKind::kMustUse, false},
                                         TestParams{AttributeKind::kOffset, false},
                                         TestParams{AttributeKind::kSize, false},
                                         TestParams{AttributeKind::kStage, false},
                                         TestParams{AttributeKind::kStride, false},
                                         TestParams{AttributeKind::kWorkgroup, false},
                                         TestParams{AttributeKind::kBindingAndGroup, false}));

}  // namespace BlockStatementTests

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
    ast::Type el_ty = params.create_el_type(*this);

    utils::StringStream ss;
    ss << "el_ty: " << FriendlyName(el_ty) << ", stride: " << params.stride
       << ", should_pass: " << params.should_pass;
    SCOPED_TRACE(ss.str());

    auto arr = ty.array(el_ty, 4_u,
                        utils::Vector{
                            create<ast::StrideAttribute>(Source{{12, 34}}, params.stride),
                        });

    GlobalVar("myarray", arr, builtin::AddressSpace::kPrivate);

    if (params.should_pass) {
        EXPECT_TRUE(r()->Resolve()) << r()->error();
    } else {
        EXPECT_FALSE(r()->Resolve());
        EXPECT_EQ(r()->error(),
                  "12:34 error: arrays decorated with the stride attribute must "
                  "have a stride that is at least the size of the element type, "
                  "and be a multiple of the element type's alignment value");
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
    auto arr = ty.array(Source{{12, 34}}, ty.i32(), 4_u,
                        utils::Vector{
                            create<ast::StrideAttribute>(Source{{12, 34}}, 4u),
                            create<ast::StrideAttribute>(Source{{56, 78}}, 4u),
                        });

    GlobalVar("myarray", arr, builtin::AddressSpace::kPrivate);

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
    auto* s = Structure("S", utils::Vector{
                                 Member("x", ty.i32()),
                             });
    GlobalVar(Source{{12, 34}}, "G", ty.Of(s), builtin::AddressSpace::kUniform);

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(),
              R"(12:34 error: resource variables require @group and @binding attributes)");
}

TEST_F(ResourceAttributeTest, StorageBufferMissingBinding) {
    auto* s = Structure("S", utils::Vector{
                                 Member("x", ty.i32()),
                             });
    GlobalVar(Source{{12, 34}}, "G", ty.Of(s), builtin::AddressSpace::kStorage,
              builtin::Access::kRead);

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(),
              R"(12:34 error: resource variables require @group and @binding attributes)");
}

TEST_F(ResourceAttributeTest, TextureMissingBinding) {
    GlobalVar(Source{{12, 34}}, "G", ty.depth_texture(type::TextureDimension::k2d));

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(),
              R"(12:34 error: resource variables require @group and @binding attributes)");
}

TEST_F(ResourceAttributeTest, SamplerMissingBinding) {
    GlobalVar(Source{{12, 34}}, "G", ty.sampler(type::SamplerKind::kSampler));

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(),
              R"(12:34 error: resource variables require @group and @binding attributes)");
}

TEST_F(ResourceAttributeTest, BindingPairMissingBinding) {
    GlobalVar(Source{{12, 34}}, "G", ty.sampler(type::SamplerKind::kSampler), Group(1_a));

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(),
              R"(12:34 error: resource variables require @group and @binding attributes)");
}

TEST_F(ResourceAttributeTest, BindingPairMissingGroup) {
    GlobalVar(Source{{12, 34}}, "G", ty.sampler(type::SamplerKind::kSampler), Binding(1_a));

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(),
              R"(12:34 error: resource variables require @group and @binding attributes)");
}

TEST_F(ResourceAttributeTest, BindingPointUsedTwiceByEntryPoint) {
    GlobalVar(Source{{12, 34}}, "A", ty.sampled_texture(type::TextureDimension::k2d, ty.f32()),
              Binding(1_a), Group(2_a));
    GlobalVar(Source{{56, 78}}, "B", ty.sampled_texture(type::TextureDimension::k2d, ty.f32()),
              Binding(1_a), Group(2_a));

    Func("F", utils::Empty, ty.void_(),
         utils::Vector{
             Decl(Var("a", ty.vec4<f32>(), Call("textureLoad", "A", vec2<i32>(1_i, 2_i), 0_i))),
             Decl(Var("b", ty.vec4<f32>(), Call("textureLoad", "B", vec2<i32>(1_i, 2_i), 0_i))),
         },
         utils::Vector{
             Stage(ast::PipelineStage::kFragment),
         });

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(
        r()->error(),
        R"(56:78 error: entry point 'F' references multiple variables that use the same resource binding @group(2), @binding(1)
12:34 note: first resource binding usage declared here)");
}

TEST_F(ResourceAttributeTest, BindingPointUsedTwiceByDifferentEntryPoints) {
    GlobalVar(Source{{12, 34}}, "A", ty.sampled_texture(type::TextureDimension::k2d, ty.f32()),
              Binding(1_a), Group(2_a));
    GlobalVar(Source{{56, 78}}, "B", ty.sampled_texture(type::TextureDimension::k2d, ty.f32()),
              Binding(1_a), Group(2_a));

    Func("F_A", utils::Empty, ty.void_(),
         utils::Vector{
             Decl(Var("a", ty.vec4<f32>(), Call("textureLoad", "A", vec2<i32>(1_i, 2_i), 0_i))),
         },
         utils::Vector{
             Stage(ast::PipelineStage::kFragment),
         });
    Func("F_B", utils::Empty, ty.void_(),
         utils::Vector{
             Decl(Var("b", ty.vec4<f32>(), Call("textureLoad", "B", vec2<i32>(1_i, 2_i), 0_i))),
         },
         utils::Vector{
             Stage(ast::PipelineStage::kFragment),
         });

    EXPECT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResourceAttributeTest, BindingPointOnNonResource) {
    GlobalVar(Source{{12, 34}}, "G", ty.f32(), builtin::AddressSpace::kPrivate, Binding(1_a),
              Group(2_a));

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
    auto* param = Param("p", ty.vec4<f32>(),
                        utils::Vector{
                            Invariant(Source{{12, 34}}),
                            Builtin(Source{{56, 78}}, builtin::BuiltinValue::kPosition),
                        });
    Func("main", utils::Vector{param}, ty.vec4<f32>(),
         utils::Vector{
             Return(Call(ty.vec4<f32>())),
         },
         utils::Vector{
             Stage(ast::PipelineStage::kFragment),
         },
         utils::Vector{
             Location(0_a),
         });
    EXPECT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(InvariantAttributeTests, InvariantWithoutPosition) {
    auto* param = Param("p", ty.vec4<f32>(),
                        utils::Vector{
                            Invariant(Source{{12, 34}}),
                            Location(0_a),
                        });
    Func("main", utils::Vector{param}, ty.vec4<f32>(),
         utils::Vector{
             Return(Call(ty.vec4<f32>())),
         },
         utils::Vector{
             Stage(ast::PipelineStage::kFragment),
         },
         utils::Vector{
             Location(0_a),
         });
    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(),
              "12:34 error: invariant attribute must only be applied to a "
              "position builtin");
}
}  // namespace
}  // namespace InvariantAttributeTests

namespace MustUseAttributeTests {
namespace {

using MustUseAttributeTests = ResolverTest;
TEST_F(MustUseAttributeTests, MustUse) {
    Func("main", utils::Empty, ty.vec4<f32>(),
         utils::Vector{
             Return(Call(ty.vec4<f32>())),
         },
         utils::Vector{
             MustUse(Source{{12, 34}}),
         });
    EXPECT_TRUE(r()->Resolve()) << r()->error();
}

}  // namespace
}  // namespace MustUseAttributeTests

namespace WorkgroupAttributeTests {
namespace {

using WorkgroupAttribute = ResolverTest;
TEST_F(WorkgroupAttribute, ComputeShaderPass) {
    Func("main", utils::Empty, ty.void_(), utils::Empty,
         utils::Vector{
             Stage(ast::PipelineStage::kCompute),
             create<ast::WorkgroupAttribute>(Source{{12, 34}}, Expr(1_i)),
         });

    EXPECT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(WorkgroupAttribute, Missing) {
    Func(Source{{12, 34}}, "main", utils::Empty, ty.void_(), utils::Empty,
         utils::Vector{
             Stage(ast::PipelineStage::kCompute),
         });

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(),
              "12:34 error: a compute shader must include 'workgroup_size' in its "
              "attributes");
}

TEST_F(WorkgroupAttribute, NotAnEntryPoint) {
    Func("main", utils::Empty, ty.void_(), utils::Empty,
         utils::Vector{
             create<ast::WorkgroupAttribute>(Source{{12, 34}}, Expr(1_i)),
         });

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), "12:34 error: @workgroup_size is only valid for compute stages");
}

TEST_F(WorkgroupAttribute, NotAComputeShader) {
    Func("main", utils::Empty, ty.void_(), utils::Empty,
         utils::Vector{
             Stage(ast::PipelineStage::kFragment),
             create<ast::WorkgroupAttribute>(Source{{12, 34}}, Expr(1_i)),
         });

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), "12:34 error: @workgroup_size is only valid for compute stages");
}

TEST_F(WorkgroupAttribute, DuplicateAttribute) {
    Func(Source{{12, 34}}, "main", utils::Empty, ty.void_(), utils::Empty,
         utils::Vector{
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
    builtin::InterpolationType type;
    builtin::InterpolationSampling sampling;
    bool should_pass;
};

struct TestWithParams : ResolverTestWithParam<Params> {};

using InterpolateParameterTest = TestWithParams;
TEST_P(InterpolateParameterTest, All) {
    auto& params = GetParam();

    Func("main",
         utils::Vector{
             Param("a", ty.f32(),
                   utils::Vector{
                       Location(0_a),
                       Interpolate(Source{{12, 34}}, params.type, params.sampling),
                   }),
         },
         ty.void_(), utils::Empty,
         utils::Vector{
             Stage(ast::PipelineStage::kFragment),
         });

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
         utils::Vector{
             Param("a", ty.i32(),
                   utils::Vector{
                       Location(0_a),
                       Interpolate(Source{{12, 34}}, params.type, params.sampling),
                   }),
         },
         ty.void_(), utils::Empty,
         utils::Vector{
             Stage(ast::PipelineStage::kFragment),
         });

    if (params.type != builtin::InterpolationType::kFlat) {
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
         utils::Vector{
             Param("a", ty.vec4<u32>(),
                   utils::Vector{
                       Location(0_a),
                       Interpolate(Source{{12, 34}}, params.type, params.sampling),
                   }),
         },
         ty.void_(), utils::Empty,
         utils::Vector{
             Stage(ast::PipelineStage::kFragment),
         });

    if (params.type != builtin::InterpolationType::kFlat) {
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
        Params{builtin::InterpolationType::kPerspective, builtin::InterpolationSampling::kUndefined,
               true},
        Params{builtin::InterpolationType::kPerspective, builtin::InterpolationSampling::kCenter,
               true},
        Params{builtin::InterpolationType::kPerspective, builtin::InterpolationSampling::kCentroid,
               true},
        Params{builtin::InterpolationType::kPerspective, builtin::InterpolationSampling::kSample,
               true},
        Params{builtin::InterpolationType::kLinear, builtin::InterpolationSampling::kUndefined,
               true},
        Params{builtin::InterpolationType::kLinear, builtin::InterpolationSampling::kCenter, true},
        Params{builtin::InterpolationType::kLinear, builtin::InterpolationSampling::kCentroid,
               true},
        Params{builtin::InterpolationType::kLinear, builtin::InterpolationSampling::kSample, true},
        // flat interpolation must not have a sampling type
        Params{builtin::InterpolationType::kFlat, builtin::InterpolationSampling::kUndefined, true},
        Params{builtin::InterpolationType::kFlat, builtin::InterpolationSampling::kCenter, false},
        Params{builtin::InterpolationType::kFlat, builtin::InterpolationSampling::kCentroid, false},
        Params{builtin::InterpolationType::kFlat, builtin::InterpolationSampling::kSample, false}));

TEST_F(InterpolateTest, FragmentInput_Integer_MissingFlatInterpolation) {
    Func("main",
         utils::Vector{Param(Source{{12, 34}}, "a", ty.i32(), utils::Vector{Location(0_a)})},
         ty.void_(), utils::Empty,
         utils::Vector{
             Stage(ast::PipelineStage::kFragment),
         });

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(
        r()->error(),
        R"(12:34 error: integral user-defined fragment inputs must have a flat interpolation attribute)");
}

TEST_F(InterpolateTest, VertexOutput_Integer_MissingFlatInterpolation) {
    auto* s = Structure(
        "S",
        utils::Vector{
            Member("pos", ty.vec4<f32>(), utils::Vector{Builtin(builtin::BuiltinValue::kPosition)}),
            Member(Source{{12, 34}}, "u", ty.u32(), utils::Vector{Location(0_a)}),
        });
    Func("main", utils::Empty, ty.Of(s),
         utils::Vector{
             Return(Call(ty.Of(s))),
         },
         utils::Vector{
             Stage(ast::PipelineStage::kVertex),
         });

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(
        r()->error(),
        R"(12:34 error: integral user-defined vertex outputs must have a flat interpolation attribute
note: while analyzing entry point 'main')");
}

TEST_F(InterpolateTest, MissingLocationAttribute_Parameter) {
    Func("main",
         utils::Vector{
             Param("a", ty.vec4<f32>(),
                   utils::Vector{
                       Builtin(builtin::BuiltinValue::kPosition),
                       Interpolate(Source{{12, 34}}, builtin::InterpolationType::kFlat),
                   }),
         },
         ty.void_(), utils::Empty,
         utils::Vector{
             Stage(ast::PipelineStage::kFragment),
         });

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(),
              R"(12:34 error: interpolate attribute must only be used with @location)");
}

TEST_F(InterpolateTest, MissingLocationAttribute_ReturnType) {
    Func("main", utils::Empty, ty.vec4<f32>(),
         utils::Vector{
             Return(Call(ty.vec4<f32>())),
         },
         utils::Vector{
             Stage(ast::PipelineStage::kVertex),
         },
         utils::Vector{
             Builtin(builtin::BuiltinValue::kPosition),
             Interpolate(Source{{12, 34}}, builtin::InterpolationType::kFlat),
         });

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(),
              R"(12:34 error: interpolate attribute must only be used with @location)");
}

TEST_F(InterpolateTest, MissingLocationAttribute_Struct) {
    Structure(
        "S",
        utils::Vector{
            Member("a", ty.f32(),
                   utils::Vector{Interpolate(Source{{12, 34}}, builtin::InterpolationType::kFlat)}),
        });

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(),
              R"(12:34 error: interpolate attribute must only be used with @location)");
}

using GroupAndBindingTest = ResolverTest;

TEST_F(GroupAndBindingTest, Const_I32) {
    GlobalConst("b", Expr(4_i));
    GlobalConst("g", Expr(2_i));
    GlobalVar("val", ty.sampled_texture(type::TextureDimension::k2d, ty.f32()), Binding("b"),
              Group("g"));

    EXPECT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(GroupAndBindingTest, Const_U32) {
    GlobalConst("b", Expr(4_u));
    GlobalConst("g", Expr(2_u));
    GlobalVar("val", ty.sampled_texture(type::TextureDimension::k2d, ty.f32()), Binding("b"),
              Group("g"));

    EXPECT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(GroupAndBindingTest, Const_AInt) {
    GlobalConst("b", Expr(4_a));
    GlobalConst("g", Expr(2_a));
    GlobalVar("val", ty.sampled_texture(type::TextureDimension::k2d, ty.f32()), Binding("b"),
              Group("g"));

    EXPECT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(GroupAndBindingTest, Binding_NonConstant) {
    GlobalVar("val", ty.sampled_texture(type::TextureDimension::k2d, ty.f32()),
              Binding(Call<u32>(Call(Source{{12, 34}}, "dpdx", 1_a))), Group(1_i));

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(
        r()->error(),
        R"(12:34 error: @binding requires a const-expression, but expression is a runtime-expression)");
}

TEST_F(GroupAndBindingTest, Binding_Negative) {
    GlobalVar("val", ty.sampled_texture(type::TextureDimension::k2d, ty.f32()),
              Binding(Source{{12, 34}}, -2_i), Group(1_i));

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), R"(12:34 error: @binding value must be non-negative)");
}

TEST_F(GroupAndBindingTest, Binding_F32) {
    GlobalVar("val", ty.sampled_texture(type::TextureDimension::k2d, ty.f32()),
              Binding(Source{{12, 34}}, 2.0_f), Group(1_u));

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), R"(12:34 error: @binding must be an i32 or u32 value)");
}

TEST_F(GroupAndBindingTest, Binding_AFloat) {
    GlobalVar("val", ty.sampled_texture(type::TextureDimension::k2d, ty.f32()),
              Binding(Source{{12, 34}}, 2.0_a), Group(1_u));

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), R"(12:34 error: @binding must be an i32 or u32 value)");
}

TEST_F(GroupAndBindingTest, Group_NonConstant) {
    GlobalVar("val", ty.sampled_texture(type::TextureDimension::k2d, ty.f32()), Binding(2_u),
              Group(Call<u32>(Call(Source{{12, 34}}, "dpdx", 1_a))));

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(
        r()->error(),
        R"(12:34 error: @group requires a const-expression, but expression is a runtime-expression)");
}

TEST_F(GroupAndBindingTest, Group_Negative) {
    GlobalVar("val", ty.sampled_texture(type::TextureDimension::k2d, ty.f32()), Binding(2_u),
              Group(Source{{12, 34}}, -1_i));

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), R"(12:34 error: @group value must be non-negative)");
}

TEST_F(GroupAndBindingTest, Group_F32) {
    GlobalVar("val", ty.sampled_texture(type::TextureDimension::k2d, ty.f32()), Binding(2_u),
              Group(Source{{12, 34}}, 1.0_f));

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), R"(12:34 error: @group must be an i32 or u32 value)");
}

TEST_F(GroupAndBindingTest, Group_AFloat) {
    GlobalVar("val", ty.sampled_texture(type::TextureDimension::k2d, ty.f32()), Binding(2_u),
              Group(Source{{12, 34}}, 1.0_a));

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), R"(12:34 error: @group must be an i32 or u32 value)");
}

using IdTest = ResolverTest;

TEST_F(IdTest, Const_I32) {
    Override("val", ty.f32(), utils::Vector{Id(1_i)});
    EXPECT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(IdTest, Const_U32) {
    Override("val", ty.f32(), utils::Vector{Id(1_u)});
    EXPECT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(IdTest, Const_AInt) {
    Override("val", ty.f32(), utils::Vector{Id(1_a)});
    EXPECT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(IdTest, NonConstant) {
    Override("val", ty.f32(), utils::Vector{Id(Call<u32>(Call(Source{{12, 34}}, "dpdx", 1_a)))});
    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(
        r()->error(),
        R"(12:34 error: @id requires a const-expression, but expression is a runtime-expression)");
}

TEST_F(IdTest, Negative) {
    Override("val", ty.f32(), utils::Vector{Id(Source{{12, 34}}, -1_i)});
    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), R"(12:34 error: @id value must be non-negative)");
}

TEST_F(IdTest, F32) {
    Override("val", ty.f32(), utils::Vector{Id(Source{{12, 34}}, 1_f)});
    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), R"(12:34 error: @id must be an i32 or u32 value)");
}

TEST_F(IdTest, AFloat) {
    Override("val", ty.f32(), utils::Vector{Id(Source{{12, 34}}, 1.0_a)});
    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), R"(12:34 error: @id must be an i32 or u32 value)");
}

enum class LocationAttributeType {
    kEntryPointParameter,
    kEntryPointReturnType,
    kStructureMember,
};

struct LocationTest : ResolverTestWithParam<LocationAttributeType> {
    void Build(const ast::Expression* location_value) {
        switch (GetParam()) {
            case LocationAttributeType::kEntryPointParameter:
                Func("main",
                     utils::Vector{Param(Source{{12, 34}}, "a", ty.i32(),
                                         utils::Vector{
                                             Location(Source{{12, 34}}, location_value),
                                             Flat(),
                                         })},
                     ty.void_(), utils::Empty,
                     utils::Vector{
                         Stage(ast::PipelineStage::kFragment),
                     });
                return;
            case LocationAttributeType::kEntryPointReturnType:
                Func("main", utils::Empty, ty.f32(),
                     utils::Vector{
                         Return(1_a),
                     },
                     utils::Vector{
                         Stage(ast::PipelineStage::kFragment),
                     },
                     utils::Vector{
                         Location(Source{{12, 34}}, location_value),
                     });
                return;
            case LocationAttributeType::kStructureMember:
                Structure("S", utils::Vector{
                                   Member("m", ty.f32(),
                                          utils::Vector{
                                              Location(Source{{12, 34}}, location_value),
                                          }),
                               });
                return;
        }
    }
};

TEST_P(LocationTest, Const_I32) {
    Build(Expr(0_i));
    EXPECT_TRUE(r()->Resolve()) << r()->error();
}

TEST_P(LocationTest, Const_U32) {
    Build(Expr(0_u));
    EXPECT_TRUE(r()->Resolve()) << r()->error();
}

TEST_P(LocationTest, Const_AInt) {
    Build(Expr(0_a));
    EXPECT_TRUE(r()->Resolve()) << r()->error();
}

TEST_P(LocationTest, NonConstant) {
    Build(Call<u32>(Call(Source{{12, 34}}, "dpdx", 1_a)));
    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(
        r()->error(),
        R"(12:34 error: @location value requires a const-expression, but expression is a runtime-expression)");
}

TEST_P(LocationTest, Negative) {
    Build(Expr(-1_a));
    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), R"(12:34 error: @location value must be non-negative)");
}

TEST_P(LocationTest, F32) {
    Build(Expr(1_f));
    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), R"(12:34 error: @location must be an i32 or u32 value)");
}

TEST_P(LocationTest, AFloat) {
    Build(Expr(1.0_a));
    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), R"(12:34 error: @location must be an i32 or u32 value)");
}

INSTANTIATE_TEST_SUITE_P(LocationTest,
                         LocationTest,
                         testing::Values(LocationAttributeType::kEntryPointParameter,
                                         LocationAttributeType::kEntryPointReturnType,
                                         LocationAttributeType::kStructureMember));

}  // namespace
}  // namespace InterpolateTests

namespace MustUseTests {
namespace {

using MustUseAttributeTest = ResolverTest;
TEST_F(MustUseAttributeTest, UsedOnFnWithNoReturnValue) {
    Func("fn_must_use", utils::Empty, ty.void_(), utils::Empty,
         utils::Vector{MustUse(Source{{12, 34}})});

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(),
              R"(12:34 error: @must_use can only be applied to functions that return a value)");
}

}  // namespace
}  // namespace MustUseTests

namespace InternalAttributeDeps {
namespace {

class TestAttribute : public utils::Castable<TestAttribute, ast::InternalAttribute> {
  public:
    TestAttribute(ProgramID pid, ast::NodeID nid, const ast::IdentifierExpression* dep)
        : Base(pid, nid, utils::Vector{dep}) {}
    std::string InternalName() const override { return "test_attribute"; }
    const Cloneable* Clone(CloneContext*) const override { return nullptr; }
};

using InternalAttributeDepsTest = ResolverTest;
TEST_F(InternalAttributeDepsTest, Dependency) {
    auto* ident = Expr("v");
    auto* attr = ASTNodes().Create<TestAttribute>(ID(), AllocateNodeID(), ident);
    auto* f = Func("f", utils::Empty, ty.void_(), utils::Empty, utils::Vector{attr});
    auto* v = GlobalVar("v", ty.i32(), builtin::AddressSpace::kPrivate);

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    auto* user = As<sem::VariableUser>(Sem().Get(ident));
    ASSERT_NE(user, nullptr);

    auto* var = Sem().Get(v);
    EXPECT_EQ(user->Variable(), var);

    auto* fn = Sem().Get(f);
    EXPECT_THAT(fn->DirectlyReferencedGlobals(), testing::ElementsAre(var));
    EXPECT_THAT(fn->TransitivelyReferencedGlobals(), testing::ElementsAre(var));
}

}  // namespace
}  // namespace InternalAttributeDeps

}  // namespace tint::resolver

TINT_INSTANTIATE_TYPEINFO(tint::resolver::InternalAttributeDeps::TestAttribute);
