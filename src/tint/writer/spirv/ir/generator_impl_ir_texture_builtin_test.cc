// Copyright 2023 The Tint Authors.
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

#include "src/tint/writer/spirv/ir/test_helper_ir.h"

#include "src/tint/builtin/function.h"

using namespace tint::number_suffixes;  // NOLINT

namespace tint::writer::spirv {
namespace {

/// An additional argument to a texture builtin.
struct Arg {
    /// The argument name.
    const char* name;
    /// The vector width of the argument (1 means scalar).
    uint32_t width;
    /// The element type of the argument.
    TestElementType type;
};

/// A parameterized texture builtin function test case.
struct TextureBuiltinTestCase {
    /// The builtin function.
    enum builtin::Function function;
    /// The builtin function arguments.
    utils::Vector<Arg, 4> optional_args;
    /// The expected SPIR-V instruction string for the texture call.
    const char* texture_call;
};

std::string PrintCase(testing::TestParamInfo<TextureBuiltinTestCase> cc) {
    utils::StringStream ss;
    ss << cc.param.function;
    for (const auto& arg : cc.param.optional_args) {
        ss << "_" << arg.name;
    }
    return ss.str();
}

class TextureBuiltinTest : public SpvGeneratorImplTestWithParam<TextureBuiltinTestCase> {
  protected:
    void Run(const type::Texture* texture_ty,
             const type::Sampler* sampler_ty,
             const type::Type* coord_ty,
             const type::Type* return_ty) {
        auto params = GetParam();

        auto* t = b.FunctionParam("t", texture_ty);
        auto* s = b.FunctionParam("s", sampler_ty);
        auto* coord = b.FunctionParam("coords", coord_ty);
        auto* func = b.Function("foo", return_ty);
        func->SetParams({t, s, coord});

        b.With(func->Block(), [&] {
            utils::Vector<ir::Value*, 4> args = {t, s, coord};
            uint32_t arg_value = 1;
            for (const auto& arg : params.optional_args) {
                auto* value = MakeScalarValue(arg.type, arg_value++);
                if (arg.width > 1) {
                    value = b.Constant(mod.constant_values.Splat(ty.vec(value->Type(), arg.width),
                                                                 value->Value(), arg.width));
                }
                args.Push(value);
                mod.SetName(value, arg.name);
            }
            auto* result = b.Call(return_ty, params.function, std::move(args));
            b.Return(func, result);
            mod.SetName(result, "result");
        });

        ASSERT_TRUE(Generate()) << Error() << output_;
        EXPECT_INST(params.texture_call);
    }
};

using Texture1D = TextureBuiltinTest;
TEST_P(Texture1D, Emit) {
    Run(ty.Get<type::SampledTexture>(type::TextureDimension::k1d, ty.f32()),
        ty.sampler(),   // sampler type
        ty.f32(),       // coord type
        ty.vec4<f32>()  // return type
    );
    EXPECT_INST("%11 = OpSampledImage %12 %t %s");
}
INSTANTIATE_TEST_SUITE_P(SpvGeneratorImplTest,
                         Texture1D,
                         testing::Values(TextureBuiltinTestCase{
                             builtin::Function::kTextureSample,
                             {},
                             "OpImageSampleImplicitLod %v4float %11 %coords None",
                         }),
                         PrintCase);

using Texture2D = TextureBuiltinTest;
TEST_P(Texture2D, Emit) {
    Run(ty.Get<type::SampledTexture>(type::TextureDimension::k2d, ty.f32()),
        ty.sampler(),    // sampler type
        ty.vec2<f32>(),  // coord type
        ty.vec4<f32>()   // return type
    );
    EXPECT_INST("%12 = OpSampledImage %13 %t %s");
}
INSTANTIATE_TEST_SUITE_P(
    SpvGeneratorImplTest,
    Texture2D,
    testing::Values(
        TextureBuiltinTestCase{
            builtin::Function::kTextureSample,
            {},
            "OpImageSampleImplicitLod %v4float %12 %coords None",
        },
        TextureBuiltinTestCase{
            builtin::Function::kTextureSample,
            {{"offset", 2, kI32}},
            "OpImageSampleImplicitLod %v4float %12 %coords ConstOffset %offset",
        },
        TextureBuiltinTestCase{
            builtin::Function::kTextureSampleBias,
            {{"bias", 1, kF32}},
            "OpImageSampleImplicitLod %v4float %12 %coords Bias %bias",
        },
        TextureBuiltinTestCase{
            builtin::Function::kTextureSampleBias,
            {{"bias", 1, kF32}, {"offset", 2, kI32}},
            "OpImageSampleImplicitLod %v4float %12 %coords Bias|ConstOffset %bias %offset",
        },
        TextureBuiltinTestCase{
            builtin::Function::kTextureSampleGrad,
            {{"ddx", 2, kF32}, {"ddy", 2, kF32}},
            "OpImageSampleExplicitLod %v4float %12 %coords Grad %ddx %ddy",
        },
        TextureBuiltinTestCase{
            builtin::Function::kTextureSampleGrad,
            {{"ddx", 2, kF32}, {"ddy", 2, kF32}, {"offset", 2, kI32}},
            "OpImageSampleExplicitLod %v4float %12 %coords Grad|ConstOffset %ddx %ddy %offset",
        },
        TextureBuiltinTestCase{
            builtin::Function::kTextureSampleLevel,
            {{"lod", 1, kF32}},
            "OpImageSampleExplicitLod %v4float %12 %coords Lod %lod",
        },
        TextureBuiltinTestCase{
            builtin::Function::kTextureSampleLevel,
            {{"lod", 1, kF32}, {"offset", 2, kI32}},
            "OpImageSampleExplicitLod %v4float %12 %coords Lod|ConstOffset %lod %offset",
        }),
    PrintCase);

using Texture2DArray = TextureBuiltinTest;
TEST_P(Texture2DArray, Emit) {
    Run(ty.Get<type::SampledTexture>(type::TextureDimension::k2dArray, ty.f32()),
        ty.sampler(),    // sampler type
        ty.vec2<f32>(),  // coord type
        ty.vec4<f32>()   // return type
    );
    EXPECT_INST("%12 = OpSampledImage %13 %t %s");
    EXPECT_INST("%14 = OpConvertSToF %float %array_idx");
    EXPECT_INST("%18 = OpCompositeConstruct %v3float %coords %14");
}
INSTANTIATE_TEST_SUITE_P(
    SpvGeneratorImplTest,
    Texture2DArray,
    testing::Values(
        TextureBuiltinTestCase{
            builtin::Function::kTextureSample,
            {{"array_idx", 1, kI32}},
            "OpImageSampleImplicitLod %v4float %12 %18 None",
        },
        TextureBuiltinTestCase{
            builtin::Function::kTextureSample,
            {{"array_idx", 1, kI32}, {"offset", 2, kI32}},
            "OpImageSampleImplicitLod %v4float %12 %18 ConstOffset %offset",
        },
        TextureBuiltinTestCase{
            builtin::Function::kTextureSampleBias,
            {{"array_idx", 1, kI32}, {"bias", 1, kF32}},
            "OpImageSampleImplicitLod %v4float %12 %18 Bias %bias",
        },
        TextureBuiltinTestCase{
            builtin::Function::kTextureSampleBias,
            {{"array_idx", 1, kI32}, {"bias", 1, kF32}, {"offset", 2, kI32}},
            "OpImageSampleImplicitLod %v4float %12 %18 Bias|ConstOffset %bias %offset",
        },
        TextureBuiltinTestCase{
            builtin::Function::kTextureSampleGrad,
            {{"array_idx", 1, kI32}, {"ddx", 2, kF32}, {"ddy", 2, kF32}},
            "OpImageSampleExplicitLod %v4float %12 %18 Grad %ddx %ddy",
        },
        TextureBuiltinTestCase{
            builtin::Function::kTextureSampleGrad,
            {{"array_idx", 1, kI32}, {"ddx", 2, kF32}, {"ddy", 2, kF32}, {"offset", 2, kI32}},
            "OpImageSampleExplicitLod %v4float %12 %18 Grad|ConstOffset %ddx %ddy %offset",
        },
        TextureBuiltinTestCase{
            builtin::Function::kTextureSampleLevel,
            {{"array_idx", 1, kI32}, {"lod", 1, kF32}},
            "OpImageSampleExplicitLod %v4float %12 %18 Lod %lod",
        },
        TextureBuiltinTestCase{
            builtin::Function::kTextureSampleLevel,
            {{"array_idx", 1, kI32}, {"lod", 1, kF32}, {"offset", 2, kI32}},
            "OpImageSampleExplicitLod %v4float %12 %18 Lod|ConstOffset %lod %offset",
        }),
    PrintCase);

using Texture3D = TextureBuiltinTest;
TEST_P(Texture3D, Emit) {
    Run(ty.Get<type::SampledTexture>(type::TextureDimension::k3d, ty.f32()),
        ty.sampler(),    // sampler type
        ty.vec3<f32>(),  // coord type
        ty.vec4<f32>()   // return type
    );
    EXPECT_INST("%12 = OpSampledImage %13 %t %s");
}
INSTANTIATE_TEST_SUITE_P(
    SpvGeneratorImplTest,
    Texture3D,
    testing::Values(
        TextureBuiltinTestCase{
            builtin::Function::kTextureSample,
            {},
            "OpImageSampleImplicitLod %v4float %12 %coords None",
        },
        TextureBuiltinTestCase{
            builtin::Function::kTextureSample,
            {{"offset", 3, kI32}},
            "OpImageSampleImplicitLod %v4float %12 %coords ConstOffset %offset",
        },
        TextureBuiltinTestCase{
            builtin::Function::kTextureSampleBias,
            {{"bias", 1, kF32}},
            "OpImageSampleImplicitLod %v4float %12 %coords Bias %bias",
        },
        TextureBuiltinTestCase{
            builtin::Function::kTextureSampleBias,
            {{"bias", 1, kF32}, {"offset", 3, kI32}},
            "OpImageSampleImplicitLod %v4float %12 %coords Bias|ConstOffset %bias %offset",
        },
        TextureBuiltinTestCase{
            builtin::Function::kTextureSampleGrad,
            {{"ddx", 3, kF32}, {"ddy", 3, kF32}},
            "OpImageSampleExplicitLod %v4float %12 %coords Grad %ddx %ddy",
        },
        TextureBuiltinTestCase{
            builtin::Function::kTextureSampleGrad,
            {{"ddx", 3, kF32}, {"ddy", 3, kF32}, {"offset", 3, kI32}},
            "OpImageSampleExplicitLod %v4float %12 %coords Grad|ConstOffset %ddx %ddy %offset",
        },
        TextureBuiltinTestCase{
            builtin::Function::kTextureSampleLevel,
            {{"lod", 1, kF32}},
            "OpImageSampleExplicitLod %v4float %12 %coords Lod %lod",
        },
        TextureBuiltinTestCase{
            builtin::Function::kTextureSampleLevel,
            {{"lod", 1, kF32}, {"offset", 3, kI32}},
            "OpImageSampleExplicitLod %v4float %12 %coords Lod|ConstOffset %lod %offset",
        }),
    PrintCase);

using TextureCube = TextureBuiltinTest;
TEST_P(TextureCube, Emit) {
    Run(ty.Get<type::SampledTexture>(type::TextureDimension::kCube, ty.f32()),
        ty.sampler(),    // sampler type
        ty.vec3<f32>(),  // coord type
        ty.vec4<f32>()   // return type
    );
    EXPECT_INST("%12 = OpSampledImage %13 %t %s");
}
INSTANTIATE_TEST_SUITE_P(SpvGeneratorImplTest,
                         TextureCube,
                         testing::Values(
                             TextureBuiltinTestCase{
                                 builtin::Function::kTextureSample,
                                 {},
                                 "OpImageSampleImplicitLod %v4float %12 %coords None",
                             },
                             TextureBuiltinTestCase{
                                 builtin::Function::kTextureSampleBias,
                                 {{"bias", 1, kF32}},
                                 "OpImageSampleImplicitLod %v4float %12 %coords Bias %bias",
                             },
                             TextureBuiltinTestCase{
                                 builtin::Function::kTextureSampleGrad,
                                 {{"ddx", 3, kF32}, {"ddy", 3, kF32}},
                                 "OpImageSampleExplicitLod %v4float %12 %coords Grad %ddx %ddy",
                             },
                             TextureBuiltinTestCase{
                                 builtin::Function::kTextureSampleLevel,
                                 {{"lod", 1, kF32}},
                                 "OpImageSampleExplicitLod %v4float %12 %coords Lod %lod",
                             }),
                         PrintCase);

using TextureCubeArray = TextureBuiltinTest;
TEST_P(TextureCubeArray, Emit) {
    Run(ty.Get<type::SampledTexture>(type::TextureDimension::kCubeArray, ty.f32()),
        ty.sampler(),    // sampler type
        ty.vec3<f32>(),  // coord type
        ty.vec4<f32>()   // return type
    );
    EXPECT_INST("%12 = OpSampledImage %13 %t %s");
    EXPECT_INST("%14 = OpConvertSToF %float %array_idx");
    EXPECT_INST("%17 = OpCompositeConstruct %v4float %coords %14");
}
INSTANTIATE_TEST_SUITE_P(SpvGeneratorImplTest,
                         TextureCubeArray,
                         testing::Values(
                             TextureBuiltinTestCase{
                                 builtin::Function::kTextureSample,
                                 {{"array_idx", 1, kI32}},
                                 "OpImageSampleImplicitLod %v4float %12 %17 None",
                             },
                             TextureBuiltinTestCase{
                                 builtin::Function::kTextureSampleBias,
                                 {{"array_idx", 1, kI32}, {"bias", 1, kF32}},
                                 "OpImageSampleImplicitLod %v4float %12 %17 Bias %bias",
                             },
                             TextureBuiltinTestCase{
                                 builtin::Function::kTextureSampleGrad,
                                 {{"array_idx", 1, kI32}, {"ddx", 3, kF32}, {"ddy", 3, kF32}},
                                 "OpImageSampleExplicitLod %v4float %12 %17 Grad %ddx %ddy",
                             },
                             TextureBuiltinTestCase{
                                 builtin::Function::kTextureSampleLevel,
                                 {{"array_idx", 1, kI32}, {"lod", 1, kF32}},
                                 "OpImageSampleExplicitLod %v4float %12 %17 Lod %lod",
                             }),
                         PrintCase);

using TextureDepth2D = TextureBuiltinTest;
TEST_P(TextureDepth2D, Emit) {
    Run(ty.Get<type::DepthTexture>(type::TextureDimension::k2d),
        ty.sampler(),    // sampler type
        ty.vec2<f32>(),  // coord type
        ty.f32()         // return type
    );
    EXPECT_INST("%11 = OpSampledImage %12 %t %s");
    EXPECT_INST("%result = OpCompositeExtract %float");
}
INSTANTIATE_TEST_SUITE_P(
    SpvGeneratorImplTest,
    TextureDepth2D,
    testing::Values(
        TextureBuiltinTestCase{
            builtin::Function::kTextureSample,
            {},
            "OpImageSampleImplicitLod %v4float %11 %coords None",
        },
        TextureBuiltinTestCase{
            builtin::Function::kTextureSample,
            {{"offset", 2, kI32}},
            "OpImageSampleImplicitLod %v4float %11 %coords ConstOffset %offset",
        },
        TextureBuiltinTestCase{
            builtin::Function::kTextureSampleLevel,
            {{"lod", 1, kI32}},
            "OpImageSampleExplicitLod %v4float %11 %coords Lod %13",
        },
        TextureBuiltinTestCase{
            builtin::Function::kTextureSampleLevel,
            {{"lod", 1, kI32}, {"offset", 2, kI32}},
            "OpImageSampleExplicitLod %v4float %11 %coords Lod|ConstOffset %13 %offset",
        }),
    PrintCase);

using TextureDepth2D_DepthComparison = TextureBuiltinTest;
TEST_P(TextureDepth2D_DepthComparison, Emit) {
    Run(ty.Get<type::DepthTexture>(type::TextureDimension::k2d),
        ty.comparison_sampler(),  // sampler type
        ty.vec2<f32>(),           // coord type
        ty.f32()                  // return type
    );
    EXPECT_INST("%11 = OpSampledImage %12 %t %s");
}
INSTANTIATE_TEST_SUITE_P(
    SpvGeneratorImplTest,
    TextureDepth2D_DepthComparison,
    testing::Values(
        TextureBuiltinTestCase{
            builtin::Function::kTextureSampleCompare,
            {{"depth", 1, kF32}},
            "OpImageSampleDrefImplicitLod %float %11 %coords %depth",
        },
        TextureBuiltinTestCase{
            builtin::Function::kTextureSampleCompare,
            {{"depth", 1, kF32}, {"offset", 2, kI32}},
            "OpImageSampleDrefImplicitLod %float %11 %coords %depth ConstOffset %offset",
        },
        TextureBuiltinTestCase{
            builtin::Function::kTextureSampleCompareLevel,
            {{"depth_l0", 1, kF32}},
            "OpImageSampleDrefExplicitLod %float %11 %coords %depth_l0 Lod %float_0",
        },
        TextureBuiltinTestCase{
            builtin::Function::kTextureSampleCompareLevel,
            {{"depth_l0", 1, kF32}, {"offset", 2, kI32}},
            "OpImageSampleDrefExplicitLod %float %11 %coords %depth_l0 Lod|ConstOffset %float_0 "
            "%offset",
        }),
    PrintCase);

using TextureDepth2DArray = TextureBuiltinTest;
TEST_P(TextureDepth2DArray, Emit) {
    Run(ty.Get<type::DepthTexture>(type::TextureDimension::k2dArray),
        ty.sampler(),    // sampler type
        ty.vec2<f32>(),  // coord type
        ty.f32()         // return type
    );
    EXPECT_INST("%11 = OpSampledImage %12 %t %s");
    EXPECT_INST("%13 = OpConvertSToF %float %array_idx");
    EXPECT_INST("%17 = OpCompositeConstruct %v3float %coords %13");
    EXPECT_INST("%result = OpCompositeExtract %float");
}
INSTANTIATE_TEST_SUITE_P(
    SpvGeneratorImplTest,
    TextureDepth2DArray,
    testing::Values(
        TextureBuiltinTestCase{
            builtin::Function::kTextureSample,
            {{"array_idx", 1, kI32}},
            "OpImageSampleImplicitLod %v4float %11 %17 None",
        },
        TextureBuiltinTestCase{
            builtin::Function::kTextureSample,
            {{"array_idx", 1, kI32}, {"offset", 2, kI32}},
            "OpImageSampleImplicitLod %v4float %11 %17 ConstOffset %offset",
        },
        TextureBuiltinTestCase{
            builtin::Function::kTextureSampleLevel,
            {{"array_idx", 1, kI32}, {"lod", 1, kI32}},
            "OpImageSampleExplicitLod %v4float %11 %17 Lod %18",
        },
        TextureBuiltinTestCase{
            builtin::Function::kTextureSampleLevel,
            {{"array_idx", 1, kI32}, {"lod", 1, kI32}, {"offset", 2, kI32}},
            "OpImageSampleExplicitLod %v4float %11 %17 Lod|ConstOffset %18 %offset",
        }),
    PrintCase);

using TextureDepth2DArray_DepthComparison = TextureBuiltinTest;
TEST_P(TextureDepth2DArray_DepthComparison, Emit) {
    Run(ty.Get<type::DepthTexture>(type::TextureDimension::k2dArray),
        ty.comparison_sampler(),  // sampler type
        ty.vec2<f32>(),           // coord type
        ty.f32()                  // return type
    );
    EXPECT_INST("%11 = OpSampledImage %12 %t %s");
    EXPECT_INST("%13 = OpConvertSToF %float %array_idx");
    EXPECT_INST("%17 = OpCompositeConstruct %v3float %coords %13");
}
INSTANTIATE_TEST_SUITE_P(
    SpvGeneratorImplTest,
    TextureDepth2DArray_DepthComparison,
    testing::Values(
        TextureBuiltinTestCase{
            builtin::Function::kTextureSampleCompare,
            {{"array_idx", 1, kI32}, {"depth", 1, kF32}},
            "OpImageSampleDrefImplicitLod %float %11 %17 %depth",
        },
        TextureBuiltinTestCase{
            builtin::Function::kTextureSampleCompare,
            {{"array_idx", 1, kI32}, {"depth", 1, kF32}, {"offset", 2, kI32}},
            "OpImageSampleDrefImplicitLod %float %11 %17 %depth ConstOffset %offset",
        },
        TextureBuiltinTestCase{
            builtin::Function::kTextureSampleCompareLevel,
            {{"array_idx", 1, kI32}, {"depth_l0", 1, kF32}},
            "OpImageSampleDrefExplicitLod %float %11 %17 %depth_l0 Lod %float_0",
        },
        TextureBuiltinTestCase{
            builtin::Function::kTextureSampleCompareLevel,
            {{"array_idx", 1, kI32}, {"depth_l0", 1, kF32}, {"offset", 2, kI32}},
            "OpImageSampleDrefExplicitLod %float %11 %17 %depth_l0 Lod|ConstOffset %float_0 "
            "%offset",
        }),
    PrintCase);

using TextureDepthCube = TextureBuiltinTest;
TEST_P(TextureDepthCube, Emit) {
    Run(ty.Get<type::DepthTexture>(type::TextureDimension::kCube),
        ty.sampler(),    // sampler type
        ty.vec3<f32>(),  // coord type
        ty.f32()         // return type
    );
    EXPECT_INST("%11 = OpSampledImage %12 %t %s");
    EXPECT_INST("%result = OpCompositeExtract %float");
}
INSTANTIATE_TEST_SUITE_P(SpvGeneratorImplTest,
                         TextureDepthCube,
                         testing::Values(
                             TextureBuiltinTestCase{
                                 builtin::Function::kTextureSample,
                                 {},
                                 "OpImageSampleImplicitLod %v4float %11 %coords None",
                             },
                             TextureBuiltinTestCase{
                                 builtin::Function::kTextureSampleLevel,
                                 {{"lod", 1, kI32}},
                                 "OpImageSampleExplicitLod %v4float %11 %coords Lod %13",
                             }),
                         PrintCase);

using TextureDepthCube_DepthComparison = TextureBuiltinTest;
TEST_P(TextureDepthCube_DepthComparison, Emit) {
    Run(ty.Get<type::DepthTexture>(type::TextureDimension::kCube),
        ty.comparison_sampler(),  // sampler typea
        ty.vec3<f32>(),           // coord type
        ty.f32()                  // return type
    );
    EXPECT_INST("%11 = OpSampledImage %12 %t %s");
}
INSTANTIATE_TEST_SUITE_P(
    SpvGeneratorImplTest,
    TextureDepthCube_DepthComparison,
    testing::Values(
        TextureBuiltinTestCase{
            builtin::Function::kTextureSampleCompare,
            {{"depth", 1, kF32}},
            "OpImageSampleDrefImplicitLod %float %11 %coords %depth",
        },
        TextureBuiltinTestCase{
            builtin::Function::kTextureSampleCompareLevel,
            {{"depth_l0", 1, kF32}},
            "OpImageSampleDrefExplicitLod %float %11 %coords %depth_l0 Lod %float_0",
        }),
    PrintCase);

using TextureDepthCubeArray = TextureBuiltinTest;
TEST_P(TextureDepthCubeArray, Emit) {
    Run(ty.Get<type::DepthTexture>(type::TextureDimension::kCubeArray),
        ty.sampler(),    // sampler type
        ty.vec3<f32>(),  // coord type
        ty.f32()         // return type
    );
    EXPECT_INST("%11 = OpSampledImage %12 %t %s");
    EXPECT_INST("%13 = OpConvertSToF %float %array_idx");
    EXPECT_INST("%17 = OpCompositeConstruct %v4float %coords %13");
    EXPECT_INST("%result = OpCompositeExtract %float");
}
INSTANTIATE_TEST_SUITE_P(SpvGeneratorImplTest,
                         TextureDepthCubeArray,
                         testing::Values(
                             TextureBuiltinTestCase{
                                 builtin::Function::kTextureSample,
                                 {{"array_idx", 1, kI32}},
                                 "OpImageSampleImplicitLod %v4float %11 %17 None",
                             },
                             TextureBuiltinTestCase{
                                 builtin::Function::kTextureSampleLevel,
                                 {{"array_idx", 1, kI32}, {"lod", 1, kI32}},
                                 "OpImageSampleExplicitLod %v4float %11 %17 Lod %18",
                             }),
                         PrintCase);

using TextureDepthCubeArray_DepthComparison = TextureBuiltinTest;
TEST_P(TextureDepthCubeArray_DepthComparison, Emit) {
    Run(ty.Get<type::DepthTexture>(type::TextureDimension::kCubeArray),
        ty.comparison_sampler(),  // sampler type
        ty.vec3<f32>(),           // coord type
        ty.f32()                  // return type
    );
    EXPECT_INST("%11 = OpSampledImage %12 %t %s");
    EXPECT_INST("%13 = OpConvertSToF %float %array_idx");
    EXPECT_INST("%17 = OpCompositeConstruct %v4float %coords %13");
}
INSTANTIATE_TEST_SUITE_P(
    SpvGeneratorImplTest,
    TextureDepthCubeArray_DepthComparison,
    testing::Values(
        TextureBuiltinTestCase{
            builtin::Function::kTextureSampleCompare,
            {{"array_idx", 1, kI32}, {"depth", 1, kF32}},
            "OpImageSampleDrefImplicitLod %float %11 %17 %depth",
        },
        TextureBuiltinTestCase{
            builtin::Function::kTextureSampleCompareLevel,
            {{"array_idx", 1, kI32}, {"depth_l0", 1, kF32}},
            "OpImageSampleDrefExplicitLod %float %11 %17 %depth_l0 Lod %float_0",
        }),
    PrintCase);

}  // namespace
}  // namespace tint::writer::spirv
