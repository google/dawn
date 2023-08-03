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

#include "src/tint/lang/core/builtin/function.h"
#include "src/tint/lang/core/type/depth_multisampled_texture.h"
#include "src/tint/lang/spirv/writer/common/helper_test.h"

using namespace tint::number_suffixes;  // NOLINT

namespace tint::spirv::writer {
namespace {

enum TextureType {
    kSampledTexture,
    kMultisampledTexture,
    kDepthTexture,
    kDepthMultisampledTexture,
    kStorageTexture,
};

enum SamplerUsage {
    kNoSampler,
    kSampler,
    kComparisonSampler,
};

/// A typed argument or result for a texture builtin.
struct NameAndType {
    /// The name.
    const char* name;
    /// The vector width of the value (1 means scalar).
    uint32_t width;
    /// The element type of the value.
    TestElementType type;
};

/// A parameterized texture builtin function test case.
struct TextureBuiltinTestCase {
    /// The texture type.
    TextureType texture_type;
    /// The dimensionality of the texture.
    type::TextureDimension dim;
    /// The texel type of the texture.
    TestElementType texel_type;
    /// The builtin function arguments.
    Vector<NameAndType, 4> args;
    /// The result type.
    NameAndType result;
    /// The expected SPIR-V instruction strings.
    Vector<const char*, 2> instructions;
};

template <typename STREAM, typename = traits::EnableIfIsOStream<STREAM>>
auto& operator<<(STREAM& out, TextureType type) {
    switch (type) {
        case kSampledTexture:
            out << "SampleTexture";
            break;
        case kMultisampledTexture:
            out << "MultisampleTexture";
            break;
        case kDepthTexture:
            out << "DepthTexture";
            break;
        case kDepthMultisampledTexture:
            out << "DepthMultisampledTexture";
            break;
        case kStorageTexture:
            out << "StorageTexture";
            break;
    }
    return out;
}

std::string PrintCase(testing::TestParamInfo<TextureBuiltinTestCase> cc) {
    StringStream ss;
    ss << cc.param.texture_type << cc.param.dim << "_" << cc.param.texel_type;
    for (const auto& arg : cc.param.args) {
        ss << "_" << arg.name;
    }
    return ss.str();
}

class TextureBuiltinTest : public SpirvWriterTestWithParam<TextureBuiltinTestCase> {
  protected:
    const type::Texture* MakeTextureType(TextureType type,
                                         type::TextureDimension dim,
                                         TestElementType texel_type) {
        switch (type) {
            case kSampledTexture:
                return ty.Get<type::SampledTexture>(dim, MakeScalarType(texel_type));
            case kMultisampledTexture:
                return ty.Get<type::MultisampledTexture>(dim, MakeScalarType(texel_type));
            case kDepthTexture:
                return ty.Get<type::DepthTexture>(dim);
            case kDepthMultisampledTexture:
                return ty.Get<type::DepthMultisampledTexture>(dim);
            case kStorageTexture:
                builtin::TexelFormat format;
                switch (texel_type) {
                    case kF32:
                        format = builtin::TexelFormat::kR32Float;
                        break;
                    case kI32:
                        format = builtin::TexelFormat::kR32Sint;
                        break;
                    case kU32:
                        format = builtin::TexelFormat::kR32Uint;
                        break;
                    default:
                        return nullptr;
                }
                return ty.Get<type::StorageTexture>(dim, format, builtin::Access::kWrite,
                                                    type::StorageTexture::SubtypeFor(format, ty));
        }
        return nullptr;
    }

    void Run(enum builtin::Function function, SamplerUsage sampler) {
        auto params = GetParam();

        auto* result_ty = MakeScalarType(params.result.type);
        if (function == builtin::Function::kTextureStore) {
            result_ty = ty.void_();
        }
        if (params.result.width > 1) {
            result_ty = ty.vec(result_ty, params.result.width);
        }

        Vector<ir::FunctionParam*, 4> func_params;

        auto* t = b.FunctionParam(
            "t", MakeTextureType(params.texture_type, params.dim, params.texel_type));
        func_params.Push(t);
        ir::FunctionParam* s = nullptr;
        if (sampler == kSampler) {
            s = b.FunctionParam("s", ty.sampler());
            func_params.Push(s);
        } else if (sampler == kComparisonSampler) {
            s = b.FunctionParam("s", ty.comparison_sampler());
            func_params.Push(s);
        }

        auto* func = b.Function("foo", result_ty);
        func->SetParams(std::move(func_params));

        b.Append(func->Block(), [&] {
            uint32_t arg_value = 1;

            Vector<ir::Value*, 4> args;
            if (function == builtin::Function::kTextureGather &&
                params.texture_type != kDepthTexture) {
                // Special case for textureGather, which has a component argument first.
                auto* component = MakeScalarValue(kU32, arg_value++);
                args.Push(component);
                mod.SetName(component, "component");
            }
            args.Push(t);
            if (s) {
                args.Push(s);
            }

            for (const auto& arg : params.args) {
                auto* value = MakeScalarValue(arg.type, arg_value++);
                if (arg.width > 1) {
                    value = b.Splat(ty.vec(value->Type(), arg.width), value, arg.width);
                }
                args.Push(value);
                mod.SetName(value, arg.name);
            }
            auto* result = b.Call(result_ty, function, std::move(args));
            if (result_ty->Is<type::Void>()) {
                b.Return(func);
            } else {
                b.Return(func, result);
                mod.SetName(result, "result");
            }
        });

        ASSERT_TRUE(Generate()) << Error() << output_;
        for (auto& inst : params.instructions) {
            EXPECT_INST(inst);
        }
    }
};

////////////////////////////////////////////////////////////////
//// textureSample
////////////////////////////////////////////////////////////////
using TextureSample = TextureBuiltinTest;
TEST_P(TextureSample, Emit) {
    Run(builtin::Function::kTextureSample, kSampler);
}
INSTANTIATE_TEST_SUITE_P(
    SpirvWriterTest,
    TextureSample,
    testing::Values(
        TextureBuiltinTestCase{
            kSampledTexture,
            type::TextureDimension::k1d,
            /* texel type */ kF32,
            {{"coord", 1, kF32}},
            {"result", 4, kF32},
            {
                "%10 = OpSampledImage %11 %t %s",
                "OpImageSampleImplicitLod %v4float %10 %coord None",
            },
        },
        TextureBuiltinTestCase{
            kSampledTexture,
            type::TextureDimension::k2d,
            /* texel type */ kF32,
            {{"coords", 2, kF32}},
            {"result", 4, kF32},
            {
                "%10 = OpSampledImage %11 %t %s",
                "OpImageSampleImplicitLod %v4float %10 %coords None",
            },
        },
        TextureBuiltinTestCase{
            kSampledTexture,
            type::TextureDimension::k2d,
            /* texel type */ kF32,
            {{"coords", 2, kF32}, {"offset", 2, kI32}},
            {"result", 4, kF32},
            {
                "%10 = OpSampledImage %11 %t %s",
                "OpImageSampleImplicitLod %v4float %10 %coords ConstOffset %offset",
            },
        },
        TextureBuiltinTestCase{
            kSampledTexture,
            type::TextureDimension::k2dArray,
            /* texel type */ kF32,
            {{"coords", 2, kF32}, {"array_idx", 1, kI32}},
            {"result", 4, kF32},
            {
                "%10 = OpSampledImage %11 %t %s",
                "%12 = OpConvertSToF %float %array_idx",
                "%16 = OpCompositeConstruct %v3float %coords %12",
                "OpImageSampleImplicitLod %v4float %10 %16 None",
            },
        },
        TextureBuiltinTestCase{
            kSampledTexture,
            type::TextureDimension::k2dArray,
            /* texel type */ kF32,
            {{"coords", 2, kF32}, {"array_idx", 1, kI32}, {"offset", 2, kI32}},
            {"result", 4, kF32},
            {
                "%10 = OpSampledImage %11 %t %s",
                "%12 = OpConvertSToF %float %array_idx",
                "%16 = OpCompositeConstruct %v3float %coords %12",
                "OpImageSampleImplicitLod %v4float %10 %16 ConstOffset %offset",
            },
        },
        TextureBuiltinTestCase{
            kSampledTexture,
            type::TextureDimension::k3d,
            /* texel type */ kF32,
            {{"coords", 3, kF32}},
            {"result", 4, kF32},
            {
                "%10 = OpSampledImage %11 %t %s",
                "OpImageSampleImplicitLod %v4float %10 %coords None",
            },
        },
        TextureBuiltinTestCase{
            kSampledTexture,
            type::TextureDimension::k3d,
            /* texel type */ kF32,
            {{"coords", 3, kF32}, {"offset", 3, kI32}},
            {"result", 4, kF32},
            {
                "%10 = OpSampledImage %11 %t %s",
                "OpImageSampleImplicitLod %v4float %10 %coords ConstOffset %offset",
            },
        },
        TextureBuiltinTestCase{
            kSampledTexture,
            type::TextureDimension::kCube,
            /* texel type */ kF32,
            {{"coords", 3, kF32}},
            {"result", 4, kF32},
            {
                "%10 = OpSampledImage %11 %t %s",
                "OpImageSampleImplicitLod %v4float %10 %coords None",
            },
        },
        TextureBuiltinTestCase{
            kSampledTexture,
            type::TextureDimension::kCubeArray,
            /* texel type */ kF32,
            {{"coords", 3, kF32}, {"array_idx", 1, kI32}},
            {"result", 4, kF32},
            {
                "%10 = OpSampledImage %11 %t %s",
                "%12 = OpConvertSToF %float %array_idx",
                "%15 = OpCompositeConstruct %v4float %coords %12",
                "OpImageSampleImplicitLod %v4float %10 %15 None",
            },
        },
        TextureBuiltinTestCase{
            kDepthTexture,
            type::TextureDimension::k2d,
            /* texel type */ kF32,
            {{"coords", 2, kF32}},
            {"result", 1, kF32},
            {
                "%9 = OpSampledImage %10 %t %s",
                "OpImageSampleImplicitLod %v4float %9 %coords None",
                "%result = OpCompositeExtract %float",
            },
        },
        TextureBuiltinTestCase{
            kDepthTexture,
            type::TextureDimension::k2d,
            /* texel type */ kF32,
            {{"coords", 2, kF32}, {"offset", 2, kI32}},
            {"result", 1, kF32},
            {
                "%9 = OpSampledImage %10 %t %s",
                "OpImageSampleImplicitLod %v4float %9 %coords ConstOffset %offset",
                "%result = OpCompositeExtract %float",
            },
        },
        TextureBuiltinTestCase{
            kDepthTexture,
            type::TextureDimension::kCube,
            /* texel type */ kF32,
            {{"coords", 3, kF32}},
            {"result", 1, kF32},
            {
                "%9 = OpSampledImage %10 %t %s",
                "OpImageSampleImplicitLod %v4float %9 %coords None",
                "%result = OpCompositeExtract %float",
            },
        },
        TextureBuiltinTestCase{
            kDepthTexture,
            type::TextureDimension::k2dArray,
            /* texel type */ kF32,
            {{"coords", 2, kF32}, {"array_idx", 1, kI32}},
            {"result", 1, kF32},
            {
                "%9 = OpSampledImage %10 %t %s",
                "%11 = OpConvertSToF %float %array_idx",
                "%15 = OpCompositeConstruct %v3float %coords %11",
                "OpImageSampleImplicitLod %v4float %9 %15 None",
                "%result = OpCompositeExtract %float",
            },
        },
        TextureBuiltinTestCase{
            kDepthTexture,
            type::TextureDimension::k2dArray,
            /* texel type */ kF32,
            {{"coords", 2, kF32}, {"array_idx", 1, kI32}, {"offset", 2, kI32}},
            {"result", 1, kF32},
            {
                "%9 = OpSampledImage %10 %t %s",
                "%11 = OpConvertSToF %float %array_idx",
                "%15 = OpCompositeConstruct %v3float %coords %11",
                "OpImageSampleImplicitLod %v4float %9 %15 ConstOffset %offset",
                "%result = OpCompositeExtract %float",
            },
        },
        TextureBuiltinTestCase{
            kDepthTexture,
            type::TextureDimension::kCubeArray,
            /* texel type */ kF32,
            {{"coords", 3, kF32}, {"array_idx", 1, kI32}},
            {"result", 1, kF32},
            {
                "%9 = OpSampledImage %10 %t %s",
                "%11 = OpConvertSToF %float %array_idx",
                "%15 = OpCompositeConstruct %v4float %coords %11",
                "OpImageSampleImplicitLod %v4float %9 %15 None",
            },
        }),
    PrintCase);

////////////////////////////////////////////////////////////////
//// textureSampleBias
////////////////////////////////////////////////////////////////
using TextureSampleBias = TextureBuiltinTest;
TEST_P(TextureSampleBias, Emit) {
    Run(builtin::Function::kTextureSampleBias, kSampler);
}
INSTANTIATE_TEST_SUITE_P(
    SpirvWriterTest,
    TextureSampleBias,
    testing::Values(
        TextureBuiltinTestCase{
            kSampledTexture,
            type::TextureDimension::k2d,
            /* texel type */ kF32,
            {{"coords", 2, kF32}, {"bias", 1, kF32}},
            {"result", 4, kF32},
            {
                "%10 = OpSampledImage %11 %t %s",
                "OpImageSampleImplicitLod %v4float %10 %coords Bias %bias",
            },
        },
        TextureBuiltinTestCase{
            kSampledTexture,
            type::TextureDimension::k2d,
            /* texel type */ kF32,
            {{"coords", 2, kF32}, {"bias", 1, kF32}, {"offset", 2, kI32}},
            {"result", 4, kF32},
            {
                "%10 = OpSampledImage %11 %t %s",
                "OpImageSampleImplicitLod %v4float %10 %coords Bias|ConstOffset %bias %offset",
            },
        },
        TextureBuiltinTestCase{
            kSampledTexture,
            type::TextureDimension::k2dArray,
            /* texel type */ kF32,
            {{"coords", 2, kF32}, {"array_idx", 1, kI32}, {"bias", 1, kF32}},
            {"result", 4, kF32},
            {
                "%10 = OpSampledImage %11 %t %s",
                "%12 = OpConvertSToF %float %array_idx",
                "%16 = OpCompositeConstruct %v3float %coords %12",
                "OpImageSampleImplicitLod %v4float %10 %16 Bias %bias",
            },
        },
        TextureBuiltinTestCase{
            kSampledTexture,
            type::TextureDimension::k2dArray,
            /* texel type */ kF32,
            {{"coords", 2, kF32}, {"array_idx", 1, kI32}, {"bias", 1, kF32}, {"offset", 2, kI32}},
            {"result", 4, kF32},
            {
                "%10 = OpSampledImage %11 %t %s",
                "%12 = OpConvertSToF %float %array_idx",
                "%16 = OpCompositeConstruct %v3float %coords %12",
                "OpImageSampleImplicitLod %v4float %10 %16 Bias|ConstOffset %bias %offset",
            },
        },
        TextureBuiltinTestCase{
            kSampledTexture,
            type::TextureDimension::k3d,
            /* texel type */ kF32,
            {{"coords", 3, kF32}, {"bias", 1, kF32}},
            {"result", 4, kF32},
            {
                "%10 = OpSampledImage %11 %t %s",
                "OpImageSampleImplicitLod %v4float %10 %coords Bias %bias",
            },
        },
        TextureBuiltinTestCase{
            kSampledTexture,
            type::TextureDimension::k3d,
            /* texel type */ kF32,
            {{"coords", 3, kF32}, {"bias", 1, kF32}, {"offset", 3, kI32}},
            {"result", 4, kF32},
            {
                "%10 = OpSampledImage %11 %t %s",
                "OpImageSampleImplicitLod %v4float %10 %coords Bias|ConstOffset %bias %offset",
            },
        },
        TextureBuiltinTestCase{
            kSampledTexture,
            type::TextureDimension::kCube,
            /* texel type */ kF32,
            {{"coords", 3, kF32}, {"bias", 1, kF32}},
            {"result", 4, kF32},
            {
                "%10 = OpSampledImage %11 %t %s",
                "OpImageSampleImplicitLod %v4float %10 %coords Bias %bias",
            },
        },
        TextureBuiltinTestCase{
            kSampledTexture,
            type::TextureDimension::kCubeArray,
            /* texel type */ kF32,
            {{"coords", 3, kF32}, {"array_idx", 1, kI32}, {"bias", 1, kF32}},
            {"result", 4, kF32},
            {
                "%10 = OpSampledImage %11 %t %s",
                "%12 = OpConvertSToF %float %array_idx",
                "%15 = OpCompositeConstruct %v4float %coords %12",
                "OpImageSampleImplicitLod %v4float %10 %15 Bias %bias",
            },
        }),
    PrintCase);

////////////////////////////////////////////////////////////////
//// textureSampleGrad
////////////////////////////////////////////////////////////////
using TextureSampleGrad = TextureBuiltinTest;
TEST_P(TextureSampleGrad, Emit) {
    Run(builtin::Function::kTextureSampleGrad, kSampler);
}
INSTANTIATE_TEST_SUITE_P(
    SpirvWriterTest,
    TextureSampleGrad,
    testing::Values(
        TextureBuiltinTestCase{
            kSampledTexture,
            type::TextureDimension::k2d,
            /* texel type */ kF32,
            {{"coords", 2, kF32}, {"ddx", 2, kF32}, {"ddy", 2, kF32}},
            {"result", 4, kF32},
            {
                "%10 = OpSampledImage %11 %t %s",
                "OpImageSampleExplicitLod %v4float %10 %coords Grad %ddx %ddy",
            },
        },
        TextureBuiltinTestCase{
            kSampledTexture,
            type::TextureDimension::k2d,
            /* texel type */ kF32,
            {{"coords", 2, kF32}, {"ddx", 2, kF32}, {"ddy", 2, kF32}, {"offset", 2, kI32}},
            {"result", 4, kF32},
            {
                "%10 = OpSampledImage %11 %t %s",
                "OpImageSampleExplicitLod %v4float %10 %coords Grad|ConstOffset %ddx %ddy %offset",
            },
        },
        TextureBuiltinTestCase{
            kSampledTexture,
            type::TextureDimension::k2dArray,
            /* texel type */ kF32,
            {{"coords", 2, kF32}, {"array_idx", 1, kI32}, {"ddx", 2, kF32}, {"ddy", 2, kF32}},
            {"result", 4, kF32},
            {
                "%10 = OpSampledImage %11 %t %s",
                "%12 = OpConvertSToF %float %array_idx",
                "%16 = OpCompositeConstruct %v3float %coords %12",
                "OpImageSampleExplicitLod %v4float %10 %16 Grad %ddx %ddy",
            },
        },
        TextureBuiltinTestCase{
            kSampledTexture,
            type::TextureDimension::k2dArray,
            /* texel type */ kF32,
            {{"coords", 2, kF32},
             {"array_idx", 1, kI32},
             {"ddx", 2, kF32},
             {"ddy", 2, kF32},
             {"offset", 2, kI32}},
            {"result", 4, kF32},
            {
                "%10 = OpSampledImage %11 %t %s",
                "%12 = OpConvertSToF %float %array_idx",
                "%16 = OpCompositeConstruct %v3float %coords %12",
                "OpImageSampleExplicitLod %v4float %10 %16 Grad|ConstOffset %ddx %ddy %offset",
            },
        },
        TextureBuiltinTestCase{
            kSampledTexture,
            type::TextureDimension::k3d,
            /* texel type */ kF32,
            {{"coords", 3, kF32}, {"ddx", 3, kF32}, {"ddy", 3, kF32}},
            {"result", 4, kF32},
            {
                "%10 = OpSampledImage %11 %t %s",
                "OpImageSampleExplicitLod %v4float %10 %coords Grad %ddx %ddy",
            },
        },
        TextureBuiltinTestCase{
            kSampledTexture,
            type::TextureDimension::k3d,
            /* texel type */ kF32,
            {{"coords", 3, kF32}, {"ddx", 3, kF32}, {"ddy", 3, kF32}, {"offset", 3, kI32}},
            {"result", 4, kF32},
            {
                "%10 = OpSampledImage %11 %t %s",
                "OpImageSampleExplicitLod %v4float %10 %coords Grad|ConstOffset %ddx %ddy %offset",
            },
        },
        TextureBuiltinTestCase{
            kSampledTexture,
            type::TextureDimension::kCube,
            /* texel type */ kF32,
            {{"coords", 3, kF32}, {"ddx", 3, kF32}, {"ddy", 3, kF32}},
            {"result", 4, kF32},
            {
                "%10 = OpSampledImage %11 %t %s",
                "OpImageSampleExplicitLod %v4float %10 %coords Grad %ddx %ddy",
            },
        },
        TextureBuiltinTestCase{
            kSampledTexture,
            type::TextureDimension::kCubeArray,
            /* texel type */ kF32,
            {{"coords", 3, kF32}, {"array_idx", 1, kI32}, {"ddx", 3, kF32}, {"ddy", 3, kF32}},
            {"result", 4, kF32},
            {
                "%10 = OpSampledImage %11 %t %s",
                "%12 = OpConvertSToF %float %array_idx",
                "%15 = OpCompositeConstruct %v4float %coords %12",
                "OpImageSampleExplicitLod %v4float %10 %15 Grad %ddx %ddy",
            },
        }),
    PrintCase);

////////////////////////////////////////////////////////////////
//// textureSampleLevel
////////////////////////////////////////////////////////////////
using TextureSampleLevel = TextureBuiltinTest;
TEST_P(TextureSampleLevel, Emit) {
    Run(builtin::Function::kTextureSampleLevel, kSampler);
}
INSTANTIATE_TEST_SUITE_P(
    SpirvWriterTest,
    TextureSampleLevel,
    testing::Values(
        TextureBuiltinTestCase{
            kSampledTexture,
            type::TextureDimension::k2d,
            /* texel type */ kF32,
            {{"coords", 2, kF32}, {"lod", 1, kF32}},
            {"result", 4, kF32},
            {
                "%10 = OpSampledImage %11 %t %s",
                "OpImageSampleExplicitLod %v4float %10 %coords Lod %lod",
            },
        },
        TextureBuiltinTestCase{
            kSampledTexture,
            type::TextureDimension::k2d,
            /* texel type */ kF32,
            {{"coords", 2, kF32}, {"lod", 1, kF32}, {"offset", 2, kI32}},
            {"result", 4, kF32},
            {
                "%10 = OpSampledImage %11 %t %s",
                "OpImageSampleExplicitLod %v4float %10 %coords Lod|ConstOffset %lod %offset",
            },
        },
        TextureBuiltinTestCase{
            kSampledTexture,
            type::TextureDimension::k2dArray,
            /* texel type */ kF32,
            {{"coords", 2, kF32}, {"array_idx", 1, kI32}, {"lod", 1, kF32}},
            {"result", 4, kF32},
            {
                "%10 = OpSampledImage %11 %t %s",
                "%12 = OpConvertSToF %float %array_idx",
                "%16 = OpCompositeConstruct %v3float %coords %12",
                "OpImageSampleExplicitLod %v4float %10 %16 Lod %lod",
            },
        },
        TextureBuiltinTestCase{
            kSampledTexture,
            type::TextureDimension::k2dArray,
            /* texel type */ kF32,
            {{"coords", 2, kF32}, {"array_idx", 1, kI32}, {"lod", 1, kF32}, {"offset", 2, kI32}},
            {"result", 4, kF32},
            {
                "%10 = OpSampledImage %11 %t %s",
                "%12 = OpConvertSToF %float %array_idx",
                "%16 = OpCompositeConstruct %v3float %coords %12",
                "OpImageSampleExplicitLod %v4float %10 %16 Lod|ConstOffset %lod %offset",
            },
        },
        TextureBuiltinTestCase{
            kSampledTexture,
            type::TextureDimension::k3d,
            /* texel type */ kF32,
            {{"coords", 3, kF32}, {"lod", 1, kF32}},
            {"result", 4, kF32},
            {
                "%10 = OpSampledImage %11 %t %s",
                "OpImageSampleExplicitLod %v4float %10 %coords Lod %lod",
            },
        },
        TextureBuiltinTestCase{
            kSampledTexture,
            type::TextureDimension::k3d,
            /* texel type */ kF32,
            {{"coords", 3, kF32}, {"lod", 1, kF32}, {"offset", 3, kI32}},
            {"result", 4, kF32},
            {
                "%10 = OpSampledImage %11 %t %s",
                "OpImageSampleExplicitLod %v4float %10 %coords Lod|ConstOffset %lod %offset",
            },
        },
        TextureBuiltinTestCase{
            kSampledTexture,
            type::TextureDimension::kCube,
            /* texel type */ kF32,
            {{"coords", 3, kF32}, {"lod", 1, kF32}},
            {"result", 4, kF32},
            {
                "%10 = OpSampledImage %11 %t %s",
                "OpImageSampleExplicitLod %v4float %10 %coords Lod %lod",
            },
        },
        TextureBuiltinTestCase{
            kSampledTexture,
            type::TextureDimension::kCubeArray,
            /* texel type */ kF32,
            {{"coords", 3, kF32}, {"array_idx", 1, kI32}, {"lod", 1, kF32}},
            {"result", 4, kF32},
            {
                "%10 = OpSampledImage %11 %t %s",
                "%12 = OpConvertSToF %float %array_idx",
                "%15 = OpCompositeConstruct %v4float %coords %12",
                "OpImageSampleExplicitLod %v4float %10 %15 Lod %lod",
            },
        },
        TextureBuiltinTestCase{
            kDepthTexture,
            type::TextureDimension::k2d,
            /* texel type */ kF32,
            {{"coords", 2, kF32}, {"lod", 1, kI32}},
            {"result", 1, kF32},
            {
                "%9 = OpSampledImage %10 %t %s",
                "%11 = OpConvertSToF %float %lod",
                "OpImageSampleExplicitLod %v4float %9 %coords Lod %11",
                "%result = OpCompositeExtract %float",
            },
        },
        TextureBuiltinTestCase{
            kDepthTexture,
            type::TextureDimension::k2d,
            /* texel type */ kF32,
            {{"coords", 2, kF32}, {"lod", 1, kI32}, {"offset", 2, kI32}},
            {"result", 1, kF32},
            {
                "%9 = OpSampledImage %10 %t %s",
                "%11 = OpConvertSToF %float %lod",
                "OpImageSampleExplicitLod %v4float %9 %coords Lod|ConstOffset %11 %offset",
                "%result = OpCompositeExtract %float",
            },
        },
        TextureBuiltinTestCase{
            kDepthTexture,
            type::TextureDimension::k2dArray,
            /* texel type */ kF32,
            {{"coords", 2, kF32}, {"array_idx", 1, kI32}, {"lod", 1, kI32}},
            {"result", 1, kF32},
            {
                "%9 = OpSampledImage %10 %t %s",
                "%11 = OpConvertSToF %float %array_idx",
                "%15 = OpCompositeConstruct %v3float %coords %11",
                "%19 = OpConvertSToF %float %lod",
                "OpImageSampleExplicitLod %v4float %9 %15 Lod %19",
                "%result = OpCompositeExtract %float",
            },
        },
        TextureBuiltinTestCase{
            kDepthTexture,
            type::TextureDimension::k2dArray,
            /* texel type */ kF32,
            {{"coords", 2, kF32}, {"array_idx", 1, kI32}, {"lod", 1, kI32}, {"offset", 2, kI32}},
            {"result", 1, kF32},
            {
                "%9 = OpSampledImage %10 %t %s",
                "%11 = OpConvertSToF %float %array_idx",
                "%15 = OpCompositeConstruct %v3float %coords %11",
                "%19 = OpConvertSToF %float %lod",
                "OpImageSampleExplicitLod %v4float %9 %15 Lod|ConstOffset %19 %offset",
                "%result = OpCompositeExtract %float",
            },
        },
        TextureBuiltinTestCase{
            kDepthTexture,
            type::TextureDimension::kCube,
            /* texel type */ kF32,
            {{"coords", 3, kF32}, {"lod", 1, kI32}},
            {"result", 1, kF32},
            {
                "%9 = OpSampledImage %10 %t %s",
                "%11 = OpConvertSToF %float %lod",
                "OpImageSampleExplicitLod %v4float %9 %coords Lod %11",
                "%result = OpCompositeExtract %float",
            },
        },
        TextureBuiltinTestCase{
            kDepthTexture,
            type::TextureDimension::kCubeArray,
            /* texel type */ kF32,
            {{"coords", 3, kF32}, {"array_idx", 1, kI32}, {"lod", 1, kI32}},
            {"result", 1, kF32},
            {
                "%9 = OpSampledImage %10 %t %s",
                "%11 = OpConvertSToF %float %array_idx",
                "%15 = OpCompositeConstruct %v4float %coords %11",
                "%19 = OpConvertSToF %float %lod",
                "OpImageSampleExplicitLod %v4float %9 %15 Lod %19",
            },
        }),
    PrintCase);

////////////////////////////////////////////////////////////////
//// textureSampleCompare
////////////////////////////////////////////////////////////////
using TextureSampleCompare = TextureBuiltinTest;
TEST_P(TextureSampleCompare, Emit) {
    Run(builtin::Function::kTextureSampleCompare, kComparisonSampler);
}
INSTANTIATE_TEST_SUITE_P(
    SpirvWriterTest,
    TextureSampleCompare,
    testing::Values(
        TextureBuiltinTestCase{
            kDepthTexture,
            type::TextureDimension::k2d,
            /* texel type */ kF32,
            {{"coords", 2, kF32}, {"depth", 1, kF32}},
            {"result", 1, kF32},
            {
                "%9 = OpSampledImage %10 %t %s",
                "OpImageSampleDrefImplicitLod %float %9 %coords %depth",
            },
        },
        TextureBuiltinTestCase{
            kDepthTexture,
            type::TextureDimension::k2d,
            /* texel type */ kF32,
            {{"coords", 2, kF32}, {"depth", 1, kF32}, {"offset", 2, kI32}},
            {"result", 1, kF32},
            {
                "%9 = OpSampledImage %10 %t %s",
                "OpImageSampleDrefImplicitLod %float %9 %coords %depth ConstOffset %offset",
            },
        },
        TextureBuiltinTestCase{
            kDepthTexture,
            type::TextureDimension::k2dArray,
            /* texel type */ kF32,
            {{"coords", 2, kF32}, {"array_idx", 1, kI32}, {"depth", 1, kF32}},
            {"result", 1, kF32},
            {
                "%9 = OpSampledImage %10 %t %s",
                "%11 = OpConvertSToF %float %array_idx",
                "%15 = OpCompositeConstruct %v3float %coords %11",
                "OpImageSampleDrefImplicitLod %float %9 %15 %depth",
            },
        },
        TextureBuiltinTestCase{
            kDepthTexture,
            type::TextureDimension::k2dArray,
            /* texel type */ kF32,
            {{"coords", 2, kF32}, {"array_idx", 1, kI32}, {"depth", 1, kF32}, {"offset", 2, kI32}},
            {"result", 1, kF32},
            {
                "%9 = OpSampledImage %10 %t %s",
                "%11 = OpConvertSToF %float %array_idx",
                "%15 = OpCompositeConstruct %v3float %coords %11",
                "OpImageSampleDrefImplicitLod %float %9 %15 %depth ConstOffset %offset",
            },
        },
        TextureBuiltinTestCase{
            kDepthTexture,
            type::TextureDimension::kCube,
            /* texel type */ kF32,
            {{"coords", 3, kF32}, {"depth", 1, kF32}},
            {"result", 1, kF32},
            {
                "%9 = OpSampledImage %10 %t %s",
                "OpImageSampleDrefImplicitLod %float %9 %coords %depth",
            },
        },
        TextureBuiltinTestCase{
            kDepthTexture,
            type::TextureDimension::kCubeArray,
            /* texel type */ kF32,
            {{"coords", 3, kF32}, {"array_idx", 1, kI32}, {"depth", 1, kF32}},
            {"result", 1, kF32},
            {
                "%9 = OpSampledImage %10 %t %s",
                "%11 = OpConvertSToF %float %array_idx",
                "%15 = OpCompositeConstruct %v4float %coords %11",
                "OpImageSampleDrefImplicitLod %float %9 %15 %depth",
            },
        }),
    PrintCase);

////////////////////////////////////////////////////////////////
//// textureSampleCompareLevel
////////////////////////////////////////////////////////////////
using TextureSampleCompareLevel = TextureBuiltinTest;
TEST_P(TextureSampleCompareLevel, Emit) {
    Run(builtin::Function::kTextureSampleCompareLevel, kComparisonSampler);
}
INSTANTIATE_TEST_SUITE_P(
    SpirvWriterTest,
    TextureSampleCompareLevel,
    testing::Values(
        TextureBuiltinTestCase{
            kDepthTexture,
            type::TextureDimension::k2d,
            /* texel type */ kF32,
            {{"coords", 2, kF32}, {"depth_l0", 1, kF32}},
            {"result", 1, kF32},
            {
                "%9 = OpSampledImage %10 %t %s",
                "OpImageSampleDrefExplicitLod %float %9 %coords %depth_l0 Lod %float_0",
            },
        },
        TextureBuiltinTestCase{
            kDepthTexture,
            type::TextureDimension::k2d,
            /* texel type */ kF32,
            {{"coords", 2, kF32}, {"depth_l0", 1, kF32}, {"offset", 2, kI32}},
            {"result", 1, kF32},
            {
                "%9 = OpSampledImage %10 %t %s",
                "OpImageSampleDrefExplicitLod %float %9 %coords %depth_l0 Lod|ConstOffset %float_0 "
                "%offset",
            },
        },
        TextureBuiltinTestCase{
            kDepthTexture,
            type::TextureDimension::k2dArray,
            /* texel type */ kF32,
            {{"coords", 2, kF32}, {"array_idx", 1, kI32}, {"depth_l0", 1, kF32}},
            {"result", 1, kF32},
            {
                "%9 = OpSampledImage %10 %t %s",
                "%11 = OpConvertSToF %float %array_idx",
                "%15 = OpCompositeConstruct %v3float %coords %11",
                "OpImageSampleDrefExplicitLod %float %9 %15 %depth_l0 Lod %float_0",
            },
        },
        TextureBuiltinTestCase{
            kDepthTexture,
            type::TextureDimension::k2dArray,
            /* texel type */ kF32,
            {{"coords", 2, kF32},
             {"array_idx", 1, kI32},
             {"depth_l0", 1, kF32},
             {"offset", 2, kI32}},
            {"result", 1, kF32},
            {
                "%9 = OpSampledImage %10 %t %s",
                "%11 = OpConvertSToF %float %array_idx",
                "%15 = OpCompositeConstruct %v3float %coords %11",
                "OpImageSampleDrefExplicitLod %float %9 %15 %depth_l0 Lod|ConstOffset %float_0 "
                "%offset",
            },
        },
        TextureBuiltinTestCase{
            kDepthTexture,
            type::TextureDimension::kCube,
            /* texel type */ kF32,
            {{"coords", 3, kF32}, {"depth_l0", 1, kF32}},
            {"result", 1, kF32},
            {
                "%9 = OpSampledImage %10 %t %s",
                "OpImageSampleDrefExplicitLod %float %9 %coords %depth_l0 Lod %float_0",
            },
        },
        TextureBuiltinTestCase{
            kDepthTexture,
            type::TextureDimension::kCubeArray,
            /* texel type */ kF32,
            {{"coords", 3, kF32}, {"array_idx", 1, kI32}, {"depth_l0", 1, kF32}},
            {"result", 1, kF32},
            {
                "%9 = OpSampledImage %10 %t %s",
                "%11 = OpConvertSToF %float %array_idx",
                "%15 = OpCompositeConstruct %v4float %coords %11",
                "OpImageSampleDrefExplicitLod %float %9 %15 %depth_l0 Lod %float_0",
            },
        }),
    PrintCase);

////////////////////////////////////////////////////////////////
//// textureGather
////////////////////////////////////////////////////////////////
using TextureGather = TextureBuiltinTest;
TEST_P(TextureGather, Emit) {
    Run(builtin::Function::kTextureGather, kSampler);
}
INSTANTIATE_TEST_SUITE_P(
    SpirvWriterTest,
    TextureGather,
    testing::Values(
        TextureBuiltinTestCase{
            kSampledTexture,
            type::TextureDimension::k2d,
            /* texel type */ kF32,
            {{"coords", 2, kF32}},
            {"result", 4, kF32},
            {
                "%10 = OpSampledImage %11 %t %s",
                "%result = OpImageGather %v4float %10 %coords %component None",
            },
        },
        TextureBuiltinTestCase{
            kSampledTexture,
            type::TextureDimension::k2d,
            /* texel type */ kF32,
            {{"coords", 2, kF32}, {"offset", 2, kI32}},
            {"result", 4, kF32},
            {
                "%10 = OpSampledImage %11 %t %s",
                "%result = OpImageGather %v4float %10 %coords %component ConstOffset %offset",
            },
        },
        TextureBuiltinTestCase{
            kSampledTexture,
            type::TextureDimension::k2dArray,
            /* texel type */ kF32,
            {{"coords", 2, kF32}, {"array_idx", 1, kI32}},
            {"result", 4, kF32},
            {
                "%10 = OpSampledImage %11 %t %s",
                "%12 = OpConvertSToF %float %array_idx",
                "%16 = OpCompositeConstruct %v3float %coords %12",
                "%result = OpImageGather %v4float %10 %16 %component None",
            },
        },
        TextureBuiltinTestCase{
            kSampledTexture,
            type::TextureDimension::k2dArray,
            /* texel type */ kF32,
            {{"coords", 2, kF32}, {"array_idx", 1, kI32}, {"offset", 2, kI32}},
            {"result", 4, kF32},
            {
                "%10 = OpSampledImage %11 %t %s",
                "%12 = OpConvertSToF %float %array_idx",
                "%16 = OpCompositeConstruct %v3float %coords %12",
                "%result = OpImageGather %v4float %10 %16 %component ConstOffset %offset",
            },
        },
        TextureBuiltinTestCase{
            kSampledTexture,
            type::TextureDimension::kCube,
            /* texel type */ kF32,
            {{"coords", 3, kF32}},
            {"result", 4, kF32},
            {
                "%10 = OpSampledImage %11 %t %s",
                "%result = OpImageGather %v4float %10 %coords %component None",
            },
        },
        TextureBuiltinTestCase{
            kSampledTexture,
            type::TextureDimension::kCubeArray,
            /* texel type */ kF32,
            {{"coords", 3, kF32}, {"array_idx", 1, kI32}},
            {"result", 4, kF32},
            {
                "%10 = OpSampledImage %11 %t %s",
                "%12 = OpConvertSToF %float %array_idx",
                "%15 = OpCompositeConstruct %v4float %coords %12",
                "%result = OpImageGather %v4float %10 %15 %component None",
            },
        },
        TextureBuiltinTestCase{
            kDepthTexture,
            type::TextureDimension::k2d,
            /* texel type */ kF32,
            {{"coords", 2, kF32}},
            {"result", 4, kF32},
            {
                "%10 = OpSampledImage %11 %t %s",
                "%result = OpImageGather %v4float %10 %coords %uint_0 None",
            },
        },
        TextureBuiltinTestCase{
            kDepthTexture,
            type::TextureDimension::k2d,
            /* texel type */ kF32,
            {{"coords", 2, kF32}, {"offset", 2, kI32}},
            {"result", 4, kF32},
            {
                "%10 = OpSampledImage %11 %t %s",
                "%result = OpImageGather %v4float %10 %coords %uint_0 ConstOffset %offset",
            },
        },
        TextureBuiltinTestCase{
            kDepthTexture,
            type::TextureDimension::kCube,
            /* texel type */ kF32,
            {{"coords", 3, kF32}},
            {"result", 4, kF32},
            {
                "%10 = OpSampledImage %11 %t %s",
                "%result = OpImageGather %v4float %10 %coords %uint_0 None",
            },
        },
        TextureBuiltinTestCase{
            kDepthTexture,
            type::TextureDimension::k2dArray,
            /* texel type */ kF32,
            {{"coords", 2, kF32}, {"array_idx", 1, kI32}},
            {"result", 4, kF32},
            {
                "%10 = OpSampledImage %11 %t %s",
                "%12 = OpConvertSToF %float %array_idx",
                "%16 = OpCompositeConstruct %v3float %coords %12",
                "%result = OpImageGather %v4float %10 %16 %uint_0 None",
            },
        },
        TextureBuiltinTestCase{
            kDepthTexture,
            type::TextureDimension::k2dArray,
            /* texel type */ kF32,
            {{"coords", 2, kF32}, {"array_idx", 1, kI32}, {"offset", 2, kI32}},
            {"result", 4, kF32},
            {
                "%10 = OpSampledImage %11 %t %s",
                "%12 = OpConvertSToF %float %array_idx",
                "%16 = OpCompositeConstruct %v3float %coords %12",
                "%result = OpImageGather %v4float %10 %16 %uint_0 ConstOffset %offset",
            },
        },
        TextureBuiltinTestCase{
            kDepthTexture,
            type::TextureDimension::kCubeArray,
            /* texel type */ kF32,
            {{"coords", 3, kF32}, {"array_idx", 1, kI32}},
            {"result", 4, kF32},
            {
                "%10 = OpSampledImage %11 %t %s",
                "%12 = OpConvertSToF %float %array_idx",
                "%15 = OpCompositeConstruct %v4float %coords %12",
                "%result = OpImageGather %v4float %10 %15 %uint_0 None",
            },
        },

        // Test some textures with integer texel types.
        TextureBuiltinTestCase{
            kSampledTexture,
            type::TextureDimension::k2d,
            /* texel type */ kI32,
            {{"coords", 2, kF32}},
            {"result", 4, kI32},
            {
                "%10 = OpSampledImage %11 %t %s",
                "%result = OpImageGather %v4int %10 %coords %component None",
            },
        },
        TextureBuiltinTestCase{
            kSampledTexture,
            type::TextureDimension::k2d,
            /* texel type */ kU32,
            {{"coords", 2, kF32}},
            {"result", 4, kU32},
            {
                "%10 = OpSampledImage %11 %t %s",
                "%result = OpImageGather %v4uint %10 %coords %component None",
            },
        }),
    PrintCase);

////////////////////////////////////////////////////////////////
//// textureGatherCompare
////////////////////////////////////////////////////////////////
using TextureGatherCompare = TextureBuiltinTest;
TEST_P(TextureGatherCompare, Emit) {
    Run(builtin::Function::kTextureGatherCompare, kComparisonSampler);
}
INSTANTIATE_TEST_SUITE_P(
    SpirvWriterTest,
    TextureGatherCompare,
    testing::Values(
        TextureBuiltinTestCase{
            kDepthTexture,
            type::TextureDimension::k2d,
            /* texel type */ kF32,
            {{"coords", 2, kF32}, {"depth", 1, kF32}},
            {"result", 4, kF32},
            {
                "%10 = OpSampledImage %11 %t %s",
                "%result = OpImageDrefGather %v4float %10 %coords %depth None",
            },
        },
        TextureBuiltinTestCase{
            kDepthTexture,
            type::TextureDimension::k2d,
            /* texel type */ kF32,
            {{"coords", 2, kF32}, {"depth", 1, kF32}, {"offset", 2, kI32}},
            {"result", 4, kF32},
            {
                "%10 = OpSampledImage %11 %t %s",
                "%result = OpImageDrefGather %v4float %10 %coords %depth ConstOffset %offset",
            },
        },
        TextureBuiltinTestCase{
            kDepthTexture,
            type::TextureDimension::k2dArray,
            /* texel type */ kF32,
            {{"coords", 2, kF32}, {"array_idx", 1, kI32}, {"depth", 1, kF32}},
            {"result", 4, kF32},
            {
                "%10 = OpSampledImage %11 %t %s",
                "%12 = OpConvertSToF %float %array_idx",
                "%16 = OpCompositeConstruct %v3float %coords %12",
                "%result = OpImageDrefGather %v4float %10 %16 %depth None",
            },
        },
        TextureBuiltinTestCase{
            kDepthTexture,
            type::TextureDimension::k2dArray,
            /* texel type */ kF32,
            {{"coords", 2, kF32}, {"array_idx", 1, kI32}, {"depth", 1, kF32}, {"offset", 2, kI32}},
            {"result", 4, kF32},
            {
                "%10 = OpSampledImage %11 %t %s",
                "%12 = OpConvertSToF %float %array_idx",
                "%16 = OpCompositeConstruct %v3float %coords %12",
                "%result = OpImageDrefGather %v4float %10 %16 %depth ConstOffset %offset",
            },
        },
        TextureBuiltinTestCase{
            kDepthTexture,
            type::TextureDimension::kCube,
            /* texel type */ kF32,
            {{"coords", 3, kF32}, {"depth", 1, kF32}},
            {"result", 4, kF32},
            {
                "%10 = OpSampledImage %11 %t %s",
                "%result = OpImageDrefGather %v4float %10 %coords %depth None",
            },
        },
        TextureBuiltinTestCase{
            kDepthTexture,
            type::TextureDimension::kCubeArray,
            /* texel type */ kF32,
            {{"coords", 3, kF32}, {"array_idx", 1, kI32}, {"depth", 1, kF32}},
            {"result", 4, kF32},
            {
                "%10 = OpSampledImage %11 %t %s",
                "%12 = OpConvertSToF %float %array_idx",
                "%15 = OpCompositeConstruct %v4float %coords %12",
                "%result = OpImageDrefGather %v4float %10 %15 %depth None",
            },
        }),
    PrintCase);

////////////////////////////////////////////////////////////////
//// textureLoad
////////////////////////////////////////////////////////////////
using TextureLoad = TextureBuiltinTest;
TEST_P(TextureLoad, Emit) {
    Run(builtin::Function::kTextureLoad, kNoSampler);
}
INSTANTIATE_TEST_SUITE_P(SpirvWriterTest,
                         TextureLoad,
                         testing::Values(
                             TextureBuiltinTestCase{
                                 kSampledTexture,
                                 type::TextureDimension::k1d,
                                 /* texel type */ kF32,
                                 {{"coord", 1, kI32}, {"lod", 1, kI32}},
                                 {"result", 4, kF32},
                                 {
                                     "OpImageFetch %v4float %t %coord Lod %lod",
                                 },
                             },
                             TextureBuiltinTestCase{
                                 kSampledTexture,
                                 type::TextureDimension::k2d,
                                 /* texel type */ kF32,
                                 {{"coords", 2, kI32}, {"lod", 1, kI32}},
                                 {"result", 4, kF32},
                                 {
                                     "OpImageFetch %v4float %t %coords Lod %lod",
                                 },
                             },
                             TextureBuiltinTestCase{
                                 kSampledTexture,
                                 type::TextureDimension::k2dArray,
                                 /* texel type */ kF32,
                                 {{"coords", 2, kI32}, {"array_idx", 1, kI32}, {"lod", 1, kI32}},
                                 {"result", 4, kF32},
                                 {
                                     "%10 = OpCompositeConstruct %v3int %coords %array_idx",
                                     "OpImageFetch %v4float %t %10 Lod %lod",
                                 },
                             },
                             TextureBuiltinTestCase{
                                 kSampledTexture,
                                 type::TextureDimension::k3d,
                                 /* texel type */ kF32,
                                 {{"coords", 3, kI32}, {"lod", 1, kI32}},
                                 {"result", 4, kF32},
                                 {
                                     "OpImageFetch %v4float %t %coords Lod %lod",
                                 },
                             },
                             TextureBuiltinTestCase{
                                 kMultisampledTexture,
                                 type::TextureDimension::k2d,
                                 /* texel type */ kF32,
                                 {{"coords", 2, kI32}, {"sample_idx", 1, kI32}},
                                 {"result", 4, kF32},
                                 {
                                     "OpImageFetch %v4float %t %coords Sample %sample_idx",
                                 },
                             },
                             TextureBuiltinTestCase{
                                 kDepthTexture,
                                 type::TextureDimension::k2d,
                                 /* texel type */ kF32,
                                 {{"coords", 2, kI32}, {"lod", 1, kI32}},
                                 {"result", 1, kF32},
                                 {
                                     "OpImageFetch %v4float %t %coords Lod %lod",
                                     "%result = OpCompositeExtract %float",
                                 },
                             },
                             TextureBuiltinTestCase{
                                 kDepthTexture,
                                 type::TextureDimension::k2dArray,
                                 /* texel type */ kF32,
                                 {{"coords", 2, kI32}, {"array_idx", 1, kI32}, {"lod", 1, kI32}},
                                 {"result", 1, kF32},
                                 {
                                     "%9 = OpCompositeConstruct %v3int %coords %array_idx",
                                     "OpImageFetch %v4float %t %9 Lod %lod",
                                     "%result = OpCompositeExtract %float",
                                 },
                             },
                             TextureBuiltinTestCase{
                                 kDepthMultisampledTexture,
                                 type::TextureDimension::k2d,
                                 /* texel type */ kF32,
                                 {{"coords", 3, kI32}, {"sample_idx", 1, kI32}},
                                 {"result", 1, kF32},
                                 {
                                     "OpImageFetch %v4float %t %coords Sample %sample_idx",
                                     "%result = OpCompositeExtract %float",
                                 },
                             },

                             // Test some textures with integer texel types.
                             TextureBuiltinTestCase{
                                 kSampledTexture,
                                 type::TextureDimension::k2d,
                                 /* texel type */ kI32,
                                 {{"coords", 2, kI32}, {"lod", 1, kI32}},
                                 {"result", 4, kI32},
                                 {
                                     "OpImageFetch %v4int %t %coords Lod %lod",
                                 },
                             },
                             TextureBuiltinTestCase{
                                 kSampledTexture,
                                 type::TextureDimension::k2d,
                                 /* texel type */ kU32,
                                 {{"coords", 2, kI32}, {"lod", 1, kI32}},
                                 {"result", 4, kU32},
                                 {
                                     "OpImageFetch %v4uint %t %coords Lod %lod",
                                 },
                             }),
                         PrintCase);

////////////////////////////////////////////////////////////////
//// textureStore
////////////////////////////////////////////////////////////////
using TextureStore = TextureBuiltinTest;
TEST_P(TextureStore, Emit) {
    Run(builtin::Function::kTextureStore, kNoSampler);
}
INSTANTIATE_TEST_SUITE_P(SpirvWriterTest,
                         TextureStore,
                         testing::Values(
                             TextureBuiltinTestCase{
                                 kStorageTexture,
                                 type::TextureDimension::k1d,
                                 /* texel type */ kF32,
                                 {{"coord", 1, kI32}, {"texel", 4, kF32}},
                                 {},
                                 {
                                     "OpImageWrite %t %coord %texel None",
                                 },
                             },
                             TextureBuiltinTestCase{
                                 kStorageTexture,
                                 type::TextureDimension::k2d,
                                 /* texel type */ kF32,
                                 {{"coords", 2, kI32}, {"texel", 4, kF32}},
                                 {},
                                 {
                                     "OpImageWrite %t %coords %texel None",
                                 },
                             },
                             TextureBuiltinTestCase{
                                 kStorageTexture,
                                 type::TextureDimension::k2dArray,
                                 /* texel type */ kF32,
                                 {{"coords", 2, kI32}, {"array_idx", 1, kI32}, {"texel", 4, kF32}},
                                 {},
                                 {
                                     "%10 = OpCompositeConstruct %v3int %coords %array_idx",
                                     "OpImageWrite %t %10 %texel None",
                                 },
                             },
                             TextureBuiltinTestCase{
                                 kStorageTexture,
                                 type::TextureDimension::k3d,
                                 /* texel type */ kF32,
                                 {{"coords", 3, kI32}, {"texel", 4, kF32}},
                                 {},
                                 {
                                     "OpImageWrite %t %coords %texel None",
                                 },
                             },

                             // Test some textures with integer texel types.
                             TextureBuiltinTestCase{
                                 kStorageTexture,
                                 type::TextureDimension::k2d,
                                 /* texel type */ kI32,
                                 {{"coords", 2, kI32}, {"texel", 4, kI32}},
                                 {},
                                 {
                                     "OpImageWrite %t %coords %texel None",
                                 },
                             },
                             TextureBuiltinTestCase{
                                 kStorageTexture,
                                 type::TextureDimension::k2d,
                                 /* texel type */ kU32,
                                 {{"coords", 2, kI32}, {"texel", 4, kU32}},
                                 {},
                                 {
                                     "OpImageWrite %t %coords %texel None",
                                 },
                             }),
                         PrintCase);

////////////////////////////////////////////////////////////////
//// textureDimensions
////////////////////////////////////////////////////////////////
using TextureDimensions = TextureBuiltinTest;
TEST_P(TextureDimensions, Emit) {
    Run(builtin::Function::kTextureDimensions, kNoSampler);
}
INSTANTIATE_TEST_SUITE_P(SpirvWriterTest,
                         TextureDimensions,
                         testing::Values(
                             // 1D implicit Lod.
                             TextureBuiltinTestCase{
                                 kSampledTexture,
                                 type::TextureDimension::k1d,
                                 /* texel type */ kF32,
                                 {},
                                 {"result", 1, kU32},
                                 {"%result = OpImageQuerySizeLod %uint %t %uint_0"},
                             },
                             TextureBuiltinTestCase{
                                 kStorageTexture,
                                 type::TextureDimension::k1d,
                                 /* texel type */ kF32,
                                 {},
                                 {"result", 1, kU32},
                                 {"%result = OpImageQuerySize %uint %t"},
                             },

                             // 1D explicit Lod.
                             TextureBuiltinTestCase{
                                 kSampledTexture,
                                 type::TextureDimension::k1d,
                                 /* texel type */ kF32,
                                 {{"lod", 1, kU32}},
                                 {"result", 1, kU32},
                                 {"%result = OpImageQuerySizeLod %uint %t %lod"},
                             },

                             // 2D implicit Lod.
                             TextureBuiltinTestCase{
                                 kSampledTexture,
                                 type::TextureDimension::k2d,
                                 /* texel type */ kF32,
                                 {},
                                 {"result", 2, kU32},
                                 {"%result = OpImageQuerySizeLod %v2uint %t %uint_0"},
                             },
                             TextureBuiltinTestCase{
                                 kSampledTexture,
                                 type::TextureDimension::k2dArray,
                                 /* texel type */ kF32,
                                 {},
                                 {"result", 2, kU32},
                                 {
                                     "%9 = OpImageQuerySizeLod %v3uint %t %uint_0",
                                     "%result = OpVectorShuffle %v2uint %9 %9 0 1",
                                 },
                             },
                             TextureBuiltinTestCase{
                                 kSampledTexture,
                                 type::TextureDimension::kCube,
                                 /* texel type */ kF32,
                                 {},
                                 {"result", 2, kU32},
                                 {"%result = OpImageQuerySizeLod %v2uint %t %uint_0"},
                             },
                             TextureBuiltinTestCase{
                                 kSampledTexture,
                                 type::TextureDimension::kCubeArray,
                                 /* texel type */ kF32,
                                 {},
                                 {"result", 2, kU32},
                                 {
                                     "%9 = OpImageQuerySizeLod %v3uint %t %uint_0",
                                     "%result = OpVectorShuffle %v2uint %9 %9 0 1",
                                 },
                             },
                             TextureBuiltinTestCase{
                                 kMultisampledTexture,
                                 type::TextureDimension::k2d,
                                 /* texel type */ kF32,
                                 {},
                                 {"result", 2, kU32},
                                 {"%result = OpImageQuerySize %v2uint %t"},
                             },
                             TextureBuiltinTestCase{
                                 kDepthTexture,
                                 type::TextureDimension::k2d,
                                 /* texel type */ kF32,
                                 {},
                                 {"result", 2, kU32},
                                 {"%result = OpImageQuerySizeLod %v2uint %t %uint_0"},
                             },
                             TextureBuiltinTestCase{
                                 kDepthTexture,
                                 type::TextureDimension::k2dArray,
                                 /* texel type */ kF32,
                                 {},
                                 {"result", 2, kU32},
                                 {
                                     "%9 = OpImageQuerySizeLod %v3uint %t %uint_0",
                                     "%result = OpVectorShuffle %v2uint %9 %9 0 1",
                                 },
                             },
                             TextureBuiltinTestCase{
                                 kDepthTexture,
                                 type::TextureDimension::kCube,
                                 /* texel type */ kF32,
                                 {},
                                 {"result", 2, kU32},
                                 {"%result = OpImageQuerySizeLod %v2uint %t %uint_0"},
                             },
                             TextureBuiltinTestCase{
                                 kDepthTexture,
                                 type::TextureDimension::kCubeArray,
                                 /* texel type */ kF32,
                                 {},
                                 {"result", 2, kU32},
                                 {
                                     "%9 = OpImageQuerySizeLod %v3uint %t %uint_0",
                                     "%result = OpVectorShuffle %v2uint %9 %9 0 1",
                                 },
                             },
                             TextureBuiltinTestCase{
                                 kDepthMultisampledTexture,
                                 type::TextureDimension::k2d,
                                 /* texel type */ kF32,
                                 {},
                                 {"result", 2, kU32},
                                 {"%result = OpImageQuerySize %v2uint %t"},
                             },
                             TextureBuiltinTestCase{
                                 kStorageTexture,
                                 type::TextureDimension::k2d,
                                 /* texel type */ kF32,
                                 {},
                                 {"result", 2, kU32},
                                 {"%result = OpImageQuerySize %v2uint %t"},
                             },
                             TextureBuiltinTestCase{
                                 kStorageTexture,
                                 type::TextureDimension::k2dArray,
                                 /* texel type */ kF32,
                                 {},
                                 {"result", 2, kU32},
                                 {
                                     "%9 = OpImageQuerySize %v3uint %t",
                                     "%result = OpVectorShuffle %v2uint %9 %9 0 1",
                                 },
                             },

                             // 2D explicit Lod.
                             TextureBuiltinTestCase{
                                 kSampledTexture,
                                 type::TextureDimension::k2d,
                                 /* texel type */ kF32,
                                 {{"lod", 1, kU32}},
                                 {"result", 2, kU32},
                                 {"%result = OpImageQuerySizeLod %v2uint %t %lod"},
                             },
                             TextureBuiltinTestCase{
                                 kSampledTexture,
                                 type::TextureDimension::k2dArray,
                                 /* texel type */ kF32,
                                 {{"lod", 1, kU32}},
                                 {"result", 2, kU32},
                                 {
                                     "%9 = OpImageQuerySizeLod %v3uint %t %lod",
                                     "%result = OpVectorShuffle %v2uint %9 %9 0 1",
                                 },
                             },
                             TextureBuiltinTestCase{
                                 kSampledTexture,
                                 type::TextureDimension::kCube,
                                 /* texel type */ kF32,
                                 {{"lod", 1, kU32}},
                                 {"result", 2, kU32},
                                 {"%result = OpImageQuerySizeLod %v2uint %t %lod"},
                             },
                             TextureBuiltinTestCase{
                                 kSampledTexture,
                                 type::TextureDimension::kCubeArray,
                                 /* texel type */ kF32,
                                 {{"lod", 1, kU32}},
                                 {"result", 2, kU32},
                                 {
                                     "%9 = OpImageQuerySizeLod %v3uint %t %lod",
                                     "%result = OpVectorShuffle %v2uint %9 %9 0 1",
                                 },
                             },
                             TextureBuiltinTestCase{
                                 kDepthTexture,
                                 type::TextureDimension::k2d,
                                 /* texel type */ kF32,
                                 {{"lod", 1, kU32}},
                                 {"result", 2, kU32},
                                 {"%result = OpImageQuerySizeLod %v2uint %t %lod"},
                             },
                             TextureBuiltinTestCase{
                                 kDepthTexture,
                                 type::TextureDimension::k2dArray,
                                 /* texel type */ kF32,
                                 {{"lod", 1, kU32}},
                                 {"result", 2, kU32},
                                 {
                                     "%9 = OpImageQuerySizeLod %v3uint %t %lod",
                                     "%result = OpVectorShuffle %v2uint %9 %9 0 1",
                                 },
                             },
                             TextureBuiltinTestCase{
                                 kDepthTexture,
                                 type::TextureDimension::kCube,
                                 /* texel type */ kF32,
                                 {{"lod", 1, kU32}},
                                 {"result", 2, kU32},
                                 {"%result = OpImageQuerySizeLod %v2uint %t %lod"},
                             },
                             TextureBuiltinTestCase{
                                 kDepthTexture,
                                 type::TextureDimension::kCubeArray,
                                 /* texel type */ kF32,
                                 {{"lod", 1, kU32}},
                                 {"result", 2, kU32},
                                 {
                                     "%9 = OpImageQuerySizeLod %v3uint %t %lod",
                                     "%result = OpVectorShuffle %v2uint %9 %9 0 1",
                                 },
                             },

                             // 3D implicit lod.
                             TextureBuiltinTestCase{
                                 kSampledTexture,
                                 type::TextureDimension::k3d,
                                 /* texel type */ kF32,
                                 {},
                                 {"result", 3, kU32},
                                 {"%result = OpImageQuerySizeLod %v3uint %t %uint_0"},
                             },

                             // 3D explicit lod.
                             TextureBuiltinTestCase{
                                 kSampledTexture,
                                 type::TextureDimension::k3d,
                                 /* texel type */ kF32,
                                 {{"lod", 1, kU32}},
                                 {"result", 3, kU32},
                                 {"%result = OpImageQuerySizeLod %v3uint %t %lod"},
                             }),
                         PrintCase);

////////////////////////////////////////////////////////////////
//// textureNumLayers
////////////////////////////////////////////////////////////////
using TextureNumLayers = TextureBuiltinTest;
TEST_P(TextureNumLayers, Emit) {
    Run(builtin::Function::kTextureNumLayers, kNoSampler);
}
INSTANTIATE_TEST_SUITE_P(SpirvWriterTest,
                         TextureNumLayers,
                         testing::Values(
                             TextureBuiltinTestCase{
                                 kSampledTexture,
                                 type::TextureDimension::k2dArray,
                                 /* texel type */ kF32,
                                 {},
                                 {"result", 1, kU32},
                                 {
                                     "%8 = OpImageQuerySizeLod %v3uint %t %uint_0",
                                     "%result = OpCompositeExtract %uint %8 2",
                                 },
                             },
                             TextureBuiltinTestCase{
                                 kSampledTexture,
                                 type::TextureDimension::kCubeArray,
                                 /* texel type */ kF32,
                                 {},
                                 {"result", 1, kU32},
                                 {
                                     "%8 = OpImageQuerySizeLod %v3uint %t %uint_0",
                                     "%result = OpCompositeExtract %uint %8 2",
                                 },
                             },
                             TextureBuiltinTestCase{
                                 kDepthTexture,
                                 type::TextureDimension::k2dArray,
                                 /* texel type */ kF32,
                                 {},
                                 {"result", 1, kU32},
                                 {
                                     "%8 = OpImageQuerySizeLod %v3uint %t %uint_0",
                                     "%result = OpCompositeExtract %uint %8 2",
                                 },
                             },
                             TextureBuiltinTestCase{
                                 kDepthTexture,
                                 type::TextureDimension::kCubeArray,
                                 /* texel type */ kF32,
                                 {},
                                 {"result", 1, kU32},
                                 {
                                     "%8 = OpImageQuerySizeLod %v3uint %t %uint_0",
                                     "%result = OpCompositeExtract %uint %8 2",
                                 },
                             },
                             TextureBuiltinTestCase{
                                 kStorageTexture,
                                 type::TextureDimension::k2dArray,
                                 /* texel type */ kF32,
                                 {},
                                 {"result", 1, kU32},
                                 {
                                     "%8 = OpImageQuerySize %v3uint %t",
                                     "%result = OpCompositeExtract %uint %8 2",
                                 },
                             }),
                         PrintCase);

////////////////////////////////////////////////////////////////
//// textureNumLevels
////////////////////////////////////////////////////////////////
using TextureNumLevels = TextureBuiltinTest;
TEST_P(TextureNumLevels, Emit) {
    Run(builtin::Function::kTextureNumLevels, kNoSampler);
}
INSTANTIATE_TEST_SUITE_P(SpirvWriterTest,
                         TextureNumLevels,
                         testing::Values(
                             TextureBuiltinTestCase{
                                 kSampledTexture,
                                 type::TextureDimension::k1d,
                                 /* texel type */ kF32,
                                 {},
                                 {"result", 1, kI32},
                                 {"%result = OpImageQueryLevels %int %t"},
                             },
                             TextureBuiltinTestCase{
                                 kSampledTexture,
                                 type::TextureDimension::k2d,
                                 /* texel type */ kF32,
                                 {},
                                 {"result", 1, kI32},
                                 {"%result = OpImageQueryLevels %int %t"},
                             },
                             TextureBuiltinTestCase{
                                 kSampledTexture,
                                 type::TextureDimension::k2dArray,
                                 /* texel type */ kF32,
                                 {},
                                 {"result", 1, kI32},
                                 {"%result = OpImageQueryLevels %int %t"},
                             },
                             TextureBuiltinTestCase{
                                 kSampledTexture,
                                 type::TextureDimension::k3d,
                                 /* texel type */ kF32,
                                 {},
                                 {"result", 1, kI32},
                                 {"%result = OpImageQueryLevels %int %t"},
                             },
                             TextureBuiltinTestCase{
                                 kSampledTexture,
                                 type::TextureDimension::kCube,
                                 /* texel type */ kF32,
                                 {},
                                 {"result", 1, kI32},
                                 {"%result = OpImageQueryLevels %int %t"},
                             },
                             TextureBuiltinTestCase{
                                 kSampledTexture,
                                 type::TextureDimension::kCubeArray,
                                 /* texel type */ kF32,
                                 {},
                                 {"result", 1, kI32},
                                 {"%result = OpImageQueryLevels %int %t"},
                             },
                             TextureBuiltinTestCase{
                                 kDepthTexture,
                                 type::TextureDimension::k2d,
                                 /* texel type */ kF32,
                                 {},
                                 {"result", 1, kI32},
                                 {"%result = OpImageQueryLevels %int %t"},
                             },
                             TextureBuiltinTestCase{
                                 kDepthTexture,
                                 type::TextureDimension::k2dArray,
                                 /* texel type */ kF32,
                                 {},
                                 {"result", 1, kI32},
                                 {"%result = OpImageQueryLevels %int %t"},
                             },
                             TextureBuiltinTestCase{
                                 kDepthTexture,
                                 type::TextureDimension::kCube,
                                 /* texel type */ kF32,
                                 {},
                                 {"result", 1, kI32},
                                 {"%result = OpImageQueryLevels %int %t"},
                             },
                             TextureBuiltinTestCase{
                                 kDepthTexture,
                                 type::TextureDimension::kCubeArray,
                                 /* texel type */ kF32,
                                 {},
                                 {"result", 1, kI32},
                                 {"%result = OpImageQueryLevels %int %t"},
                             }),
                         PrintCase);

////////////////////////////////////////////////////////////////
//// textureNumSamples
////////////////////////////////////////////////////////////////
using TextureNumSamples = TextureBuiltinTest;
TEST_P(TextureNumSamples, Emit) {
    Run(builtin::Function::kTextureNumSamples, kNoSampler);
}
INSTANTIATE_TEST_SUITE_P(SpirvWriterTest,
                         TextureNumSamples,
                         testing::Values(
                             TextureBuiltinTestCase{
                                 kMultisampledTexture,
                                 type::TextureDimension::k2d,
                                 /* texel type */ kF32,
                                 {},
                                 {"result", 1, kU32},
                                 {"%result = OpImageQuerySamples %uint %t"},
                             },
                             TextureBuiltinTestCase{
                                 kDepthMultisampledTexture,
                                 type::TextureDimension::k2d,
                                 /* texel type */ kF32,
                                 {},
                                 {"result", 1, kU32},
                                 {"%result = OpImageQuerySamples %uint %t"},
                             }),
                         PrintCase);

}  // namespace
}  // namespace tint::spirv::writer
