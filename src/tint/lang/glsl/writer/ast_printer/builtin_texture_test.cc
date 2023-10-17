// Copyright 2021 The Dawn & Tint Authors
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// 1. Redistributions of source code must retain the above copyright notice, this
//    list of conditions and the following disclaimer.
//
// 2. Redistributions in binary form must reproduce the above copyright notice,
//    this list of conditions and the following disclaimer in the documentation
//    and/or other materials provided with the distribution.
//
// 3. Neither the name of the copyright holder nor the names of its
//    contributors may be used to endorse or promote products derived from
//    this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
// FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
// DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
// CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
// OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#include "gmock/gmock.h"
#include "src/tint/lang/glsl/writer/ast_printer/helper_test.h"
#include "src/tint/lang/wgsl/ast/builtin_texture_helper_test.h"
#include "src/tint/lang/wgsl/ast/call_statement.h"
#include "src/tint/lang/wgsl/ast/stage_attribute.h"

namespace tint::glsl::writer {
namespace {

using ::testing::HasSubstr;

struct ExpectedResult {
    ExpectedResult(const char* o) : out(o) {}  // NOLINT

    std::string pre;
    std::string out;
};

ExpectedResult expected_texture_overload(ast::test::ValidTextureOverload overload) {
    using ValidTextureOverload = ast::test::ValidTextureOverload;
    switch (overload) {
        case ValidTextureOverload::kDimensions1d:
        case ValidTextureOverload::kDimensions2d:
        case ValidTextureOverload::kDimensionsDepth2d:
        case ValidTextureOverload::kDimensionsDepthMultisampled2d:
        case ValidTextureOverload::kDimensionsMultisampled2d:
        case ValidTextureOverload::kDimensions2dArray:
        case ValidTextureOverload::kDimensionsDepth2dArray:
        case ValidTextureOverload::kDimensions3d:
        case ValidTextureOverload::kDimensionsCube:
        case ValidTextureOverload::kDimensionsDepthCube:
        case ValidTextureOverload::kDimensionsCubeArray:
        case ValidTextureOverload::kDimensionsDepthCubeArray:
        case ValidTextureOverload::kDimensions2dLevel:
        case ValidTextureOverload::kDimensionsDepth2dLevel:
        case ValidTextureOverload::kDimensions2dArrayLevel:
        case ValidTextureOverload::kDimensionsDepth2dArrayLevel:
        case ValidTextureOverload::kDimensions3dLevel:
        case ValidTextureOverload::kDimensionsCubeLevel:
        case ValidTextureOverload::kDimensionsDepthCubeLevel:
        case ValidTextureOverload::kDimensionsCubeArrayLevel:
        case ValidTextureOverload::kDimensionsDepthCubeArrayLevel:
            return {"textureSize"};
        case ValidTextureOverload::kDimensionsStorageWO1d:
        case ValidTextureOverload::kDimensionsStorageWO2d:
        case ValidTextureOverload::kDimensionsStorageWO2dArray:
        case ValidTextureOverload::kDimensionsStorageWO3d:
            return {"imageSize"};
        case ValidTextureOverload::kGather2dF32:
            return R"(textureGather(Texture_Sampler, vec2(1.0f, 2.0f), 0))";
        case ValidTextureOverload::kGather2dOffsetF32:
            return R"(textureGatherOffset(Texture_Sampler, vec2(1.0f, 2.0f), ivec2(3, 4), int(0u)))";
        case ValidTextureOverload::kGather2dArrayF32:
            return R"(textureGather(Texture_Sampler, vec3(1.0f, 2.0f, float(3)), 0))";
        case ValidTextureOverload::kGather2dArrayOffsetF32:
            return R"(textureGatherOffset(Texture_Sampler, vec3(1.0f, 2.0f, float(3u)), ivec2(4, 5), int(0u)))";
        case ValidTextureOverload::kGatherCubeF32:
            return R"(textureGather(Texture_Sampler, vec3(1.0f, 2.0f, 3.0f), 0))";
        case ValidTextureOverload::kGatherCubeArrayF32:
            return R"(textureGather(Texture_Sampler, vec4(1.0f, 2.0f, 3.0f, float(4u)), int(0u))";
        case ValidTextureOverload::kGatherDepth2dF32:
            return R"(textureGather(Texture_Sampler, vec2(1.0f, 2.0f), 0.0))";
        case ValidTextureOverload::kGatherDepth2dOffsetF32:
            return R"(textureGatherOffset(Texture_Sampler, vec2(1.0f, 2.0f), 0.0, ivec2(3, 4))";
        case ValidTextureOverload::kGatherDepth2dArrayF32:
            return R"(textureGather(Texture_Sampler, vec3(1.0f, 2.0f, float(3u)), 0.0))";
        case ValidTextureOverload::kGatherDepth2dArrayOffsetF32:
            return R"(textureGatherOffset(Texture_Sampler, vec3(1.0f, 2.0f, float(3)), 0.0, ivec2(4, 5)))";
        case ValidTextureOverload::kGatherDepthCubeF32:
            return R"(textureGather(Texture_Sampler, vec3(1.0f, 2.0f, 3.0f), 0.0))";
        case ValidTextureOverload::kGatherDepthCubeArrayF32:
            return R"(textureGather(Texture_Sampler, vec4(1.0f, 2.0f, 3.0f, float(4u)), 0.0))";
        case ValidTextureOverload::kGatherCompareDepth2dF32:
            return R"(textureGather(Texture_Sampler, vec2(1.0f, 2.0f), 3.0f))";
        case ValidTextureOverload::kGatherCompareDepth2dOffsetF32:
            return R"(textureGatherOffset(Texture_Sampler, vec2(1.0f, 2.0f), 3.0f, ivec2(4, 5)))";
        case ValidTextureOverload::kGatherCompareDepth2dArrayF32:
            return R"(textureGather(Texture_Sampler, vec3(1.0f, 2.0f, float(3)), 4.0f))";
        case ValidTextureOverload::kGatherCompareDepth2dArrayOffsetF32:
            return R"(textureGatherOffset(Texture_Sampler, vec3(1.0f, 2.0f, float(3)), 4.0f, ivec2(5, 6)))";
        case ValidTextureOverload::kGatherCompareDepthCubeF32:
            return R"(textureGather(Texture_Sampler, vec3(1.0f, 2.0f, 3.0f), 4.0f))";
        case ValidTextureOverload::kGatherCompareDepthCubeArrayF32:
            return R"(textureGather(Texture_Sampler, vec4(1.0f, 2.0f, 3.0f, float(4u)), 5.0f))";
        case ValidTextureOverload::kNumLayers2dArray:
        case ValidTextureOverload::kNumLayersDepth2dArray:
        case ValidTextureOverload::kNumLayersCubeArray:
        case ValidTextureOverload::kNumLayersDepthCubeArray:
            return {"textureSize"};
        case ValidTextureOverload::kNumLayersStorageWO2dArray:
            return {"imageSize"};
        case ValidTextureOverload::kNumLevels2d:
        case ValidTextureOverload::kNumLevelsCube:
        case ValidTextureOverload::kNumLevelsDepth2d:
        case ValidTextureOverload::kNumLevelsDepthCube:
        case ValidTextureOverload::kNumLevels2dArray:
        case ValidTextureOverload::kNumLevels3d:
        case ValidTextureOverload::kNumLevelsCubeArray:
        case ValidTextureOverload::kNumLevelsDepth2dArray:
        case ValidTextureOverload::kNumLevelsDepthCubeArray:
            return {"textureQueryLevels"};
        case ValidTextureOverload::kNumSamplesDepthMultisampled2d:
        case ValidTextureOverload::kNumSamplesMultisampled2d:
            return {"textureSamples"};
        case ValidTextureOverload::kSample1dF32:
            return R"(texture(Texture_Sampler, vec2(1.0f, 0.5f));)";
        case ValidTextureOverload::kSample2dF32:
            return R"(texture(Texture_Sampler, vec2(1.0f, 2.0f));)";
        case ValidTextureOverload::kSample2dOffsetF32:
            return R"(textureOffset(Texture_Sampler, vec2(1.0f, 2.0f), ivec2(3, 4));)";
        case ValidTextureOverload::kSample2dArrayF32:
            return R"(texture(Texture_Sampler, vec3(1.0f, 2.0f, float(3)));)";
        case ValidTextureOverload::kSample2dArrayOffsetF32:
            return R"(textureOffset(Texture_Sampler, vec3(1.0f, 2.0f, float(3u)), ivec2(4, 5));)";
        case ValidTextureOverload::kSample3dF32:
            return R"(texture(Texture_Sampler, vec3(1.0f, 2.0f, 3.0f));)";
        case ValidTextureOverload::kSample3dOffsetF32:
            return R"(textureOffset(Texture_Sampler, vec3(1.0f, 2.0f, 3.0f), ivec3(4, 5, 6));)";
        case ValidTextureOverload::kSampleCubeF32:
            return R"(texture(Texture_Sampler, vec3(1.0f, 2.0f, 3.0f));)";
        case ValidTextureOverload::kSampleCubeArrayF32:
            return R"(texture(Texture_Sampler, vec4(1.0f, 2.0f, 3.0f, float(4)));)";
        case ValidTextureOverload::kSampleDepth2dF32:
            return R"(texture(Texture_Sampler, vec3(1.0f, 2.0f, 0.0f));)";
        case ValidTextureOverload::kSampleDepth2dOffsetF32:
            return R"(textureOffset(Texture_Sampler, vec3(1.0f, 2.0f, 0.0f), ivec2(3, 4));)";
        case ValidTextureOverload::kSampleDepth2dArrayF32:
            return R"(texture(Texture_Sampler, vec4(1.0f, 2.0f, float(3), 0.0f));)";
        case ValidTextureOverload::kSampleDepth2dArrayOffsetF32:
            return R"(textureOffset(Texture_Sampler, vec4(1.0f, 2.0f, float(3), 0.0f), ivec2(4, 5));)";
        case ValidTextureOverload::kSampleDepthCubeF32:
            return R"(texture(Texture_Sampler, vec4(1.0f, 2.0f, 3.0f, 0.0f));)";
        case ValidTextureOverload::kSampleDepthCubeArrayF32:
            return R"(texture(Texture_Sampler, vec4(1.0f, 2.0f, 3.0f, float(4u)), 0.0f);)";
        case ValidTextureOverload::kSampleBias2dF32:
            return R"(texture(Texture_Sampler, vec2(1.0f, 2.0f), 3.0f);)";
        case ValidTextureOverload::kSampleBias2dOffsetF32:
            return R"(textureOffset(Texture_Sampler, vec2(1.0f, 2.0f), ivec2(4, 5), 3.0f);)";
        case ValidTextureOverload::kSampleBias2dArrayF32:
            return R"(texture(Texture_Sampler, vec3(1.0f, 2.0f, float(4u)), 3.0f);)";
        case ValidTextureOverload::kSampleBias2dArrayOffsetF32:
            return R"(textureOffset(Texture_Sampler, vec3(1.0f, 2.0f, float(3)), ivec2(5, 6), 4.0f);)";
        case ValidTextureOverload::kSampleBias3dF32:
            return R"(texture(Texture_Sampler, vec3(1.0f, 2.0f, 3.0f), 4.0f);)";
        case ValidTextureOverload::kSampleBias3dOffsetF32:
            return R"(textureOffset(Texture_Sampler, vec3(1.0f, 2.0f, 3.0f), ivec3(5, 6, 7), 4.0f);)";
        case ValidTextureOverload::kSampleBiasCubeF32:
            return R"(texture(Texture_Sampler, vec3(1.0f, 2.0f, 3.0f), 4.0f);)";
        case ValidTextureOverload::kSampleBiasCubeArrayF32:
            return R"(texture(Texture_Sampler, vec4(1.0f, 2.0f, 3.0f, float(3)), 4.0f);)";
        case ValidTextureOverload::kSampleLevel2dF32:
            return R"(textureLod(Texture_Sampler, vec2(1.0f, 2.0f), 3.0f);)";
        case ValidTextureOverload::kSampleLevel2dOffsetF32:
            return R"(textureLodOffset(Texture_Sampler, vec2(1.0f, 2.0f), 3.0f, ivec2(4, 5));)";
        case ValidTextureOverload::kSampleLevel2dArrayF32:
            return R"(textureLod(Texture_Sampler, vec3(1.0f, 2.0f, float(3)), 4.0f);)";
        case ValidTextureOverload::kSampleLevel2dArrayOffsetF32:
            return R"(textureLodOffset(Texture_Sampler, vec3(1.0f, 2.0f, float(3)), 4.0f, ivec2(5, 6));)";
        case ValidTextureOverload::kSampleLevel3dF32:
            return R"(textureLod(Texture_Sampler, vec3(1.0f, 2.0f, 3.0f), 4.0f);)";
        case ValidTextureOverload::kSampleLevel3dOffsetF32:
            return R"(textureLodOffset(Texture_Sampler, vec3(1.0f, 2.0f, 3.0f), 4.0f, ivec3(5, 6, 7));)";
        case ValidTextureOverload::kSampleLevelCubeF32:
            return R"(textureLod(Texture_Sampler, vec3(1.0f, 2.0f, 3.0f), 4.0f);)";
        case ValidTextureOverload::kSampleLevelCubeArrayF32:
            return R"(textureLod(Texture_Sampler, vec4(1.0f, 2.0f, 3.0f, float(4)), 5.0f);)";
        case ValidTextureOverload::kSampleLevelDepth2dF32:
            return R"(textureLod(Texture_Sampler, vec3(1.0f, 2.0f, 0.0f), float(3u));)";
        case ValidTextureOverload::kSampleLevelDepth2dOffsetF32:
            return R"(textureLodOffset(Texture_Sampler, vec3(1.0f, 2.0f, 0.0f), float(3), ivec2(4, 5));)";
        case ValidTextureOverload::kSampleLevelDepth2dArrayF32:
            return R"(textureLod(Texture_Sampler, vec4(1.0f, 2.0f, float(3u), 0.0f), float(4u));)";
        case ValidTextureOverload::kSampleLevelDepth2dArrayOffsetF32:
            return R"(textureLodOffset(Texture_Sampler, vec4(1.0f, 2.0f, float(3u), 0.0f), float(4u), ivec2(5, 6));)";
        case ValidTextureOverload::kSampleLevelDepthCubeF32:
            return R"(textureLod(Texture_Sampler, vec4(1.0f, 2.0f, 3.0f, 0.0f), float(4)))";
        case ValidTextureOverload::kSampleLevelDepthCubeArrayF32:
            return R"(textureLod(Texture_Sampler, vec4(1.0f, 2.0f, 3.0f, float(4)), float(5));)";
        case ValidTextureOverload::kSampleGrad2dF32:
            return R"(textureGrad(Texture_Sampler, vec2(1.0f, 2.0f), vec2(3.0f, 4.0f), vec2(5.0f, 6.0f));)";
        case ValidTextureOverload::kSampleGrad2dOffsetF32:
            return R"(textureGradOffset(Texture_Sampler, vec2(1.0f, 2.0f), vec2(3.0f, 4.0f), vec2(5.0f, 6.0f), ivec2(7));)";
        case ValidTextureOverload::kSampleGrad2dArrayF32:
            return R"(textureGrad(Texture_Sampler, vec3(1.0f, 2.0f, float(3)), vec2(4.0f, 5.0f), vec2(6.0f, 7.0f));)";
        case ValidTextureOverload::kSampleGrad2dArrayOffsetF32:
            return R"(textureGradOffset(Texture_Sampler, vec3(1.0f, 2.0f, float(3u)), vec2(4.0f, 5.0f), vec2(6.0f, 7.0f), ivec2(6, 7));)";
        case ValidTextureOverload::kSampleGrad3dF32:
            return R"(textureGrad(Texture_Sampler, vec3(1.0f, 2.0f, 3.0f), vec3(4.0f, 5.0f, 6.0f), vec3(7.0f, 8.0f, 9.0f));)";
        case ValidTextureOverload::kSampleGrad3dOffsetF32:
            return R"(textureGradOffset(Texture_Sampler, vec3(1.0f, 2.0f, 3.0f), vec3(4.0f, 5.0f, 6.0f), vec3(7.0f, 8.0f, 9.0f), ivec3(0, 1, 2));)";
        case ValidTextureOverload::kSampleGradCubeF32:
            return R"(textureGrad(Texture_Sampler, vec3(1.0f, 2.0f, 3.0f), vec3(4.0f, 5.0f, 6.0f), vec3(7.0f, 8.0f, 9.0f));)";
        case ValidTextureOverload::kSampleGradCubeArrayF32:
            return R"(textureGrad(Texture_Sampler, vec4(1.0f, 2.0f, 3.0f, float(4u)), vec3(5.0f, 6.0f, 7.0f), vec3(8.0f, 9.0f, 10.0f));)";
        case ValidTextureOverload::kSampleCompareDepth2dF32:
            return R"(texture(Texture_Sampler, vec3(1.0f, 2.0f, 3.0f));)";
        case ValidTextureOverload::kSampleCompareDepth2dOffsetF32:
            return R"(textureOffset(Texture_Sampler, vec3(1.0f, 2.0f, 3.0f), ivec2(4, 5));)";
        case ValidTextureOverload::kSampleCompareDepth2dArrayF32:
            return R"(texture(Texture_Sampler, vec4(1.0f, 2.0f, float(4), 3.0f));)";
        case ValidTextureOverload::kSampleCompareDepth2dArrayOffsetF32:
            return R"(textureOffset(Texture_Sampler, vec4(1.0f, 2.0f, float(4u), 3.0f), ivec2(5, 6));)";
        case ValidTextureOverload::kSampleCompareDepthCubeF32:
            return R"(texture(Texture_Sampler, vec4(1.0f, 2.0f, 3.0f, 4.0f));)";
        case ValidTextureOverload::kSampleCompareDepthCubeArrayF32:
            return R"(texture(Texture_Sampler, vec4(1.0f, 2.0f, 3.0f, float(4)), 5.0f);)";
        case ValidTextureOverload::kSampleCompareLevelDepth2dF32:
            return R"(texture(Texture_Sampler, vec3(1.0f, 2.0f, 3.0f));)";
        case ValidTextureOverload::kSampleCompareLevelDepth2dOffsetF32:
            return R"(textureOffset(Texture_Sampler, vec3(1.0f, 2.0f, 3.0f), ivec2(4, 5));)";
        case ValidTextureOverload::kSampleCompareLevelDepth2dArrayF32:
            return R"(texture(Texture_Sampler, vec4(1.0f, 2.0f, float(3), 4.0f));)";
        case ValidTextureOverload::kSampleCompareLevelDepth2dArrayOffsetF32:
            return R"(textureOffset(Texture_Sampler, vec4(1.0f, 2.0f, float(3), 4.0f), ivec2(5, 6));)";
        case ValidTextureOverload::kSampleCompareLevelDepthCubeF32:
            return R"(texture(Texture_Sampler, vec4(1.0f, 2.0f, 3.0f, 4.0f));)";
        case ValidTextureOverload::kSampleCompareLevelDepthCubeArrayF32:
            return R"(texture(Texture_Sampler, vec4(1.0f, 2.0f, 3.0f, float(4)), 5.0f);)";
        case ValidTextureOverload::kLoad1dLevelF32:
            return R"(texelFetch(Texture_1, ivec2(uvec2(1u, 0u)), int(3u));)";
        case ValidTextureOverload::kLoad1dLevelU32:
        case ValidTextureOverload::kLoad1dLevelI32:
            return R"(texelFetch(Texture_1, ivec2(1, 0), 3);)";
        case ValidTextureOverload::kLoad2dLevelU32:
            return R"(texelFetch(Texture_1, ivec2(1, 2), 3);)";
        case ValidTextureOverload::kLoad2dLevelF32:
        case ValidTextureOverload::kLoad2dLevelI32:
            return R"(texelFetch(Texture_1, ivec2(uvec2(1u, 2u)), int(3u));)";
        case ValidTextureOverload::kLoad2dArrayLevelF32:
        case ValidTextureOverload::kLoad2dArrayLevelU32:
        case ValidTextureOverload::kLoad3dLevelF32:
        case ValidTextureOverload::kLoad3dLevelU32:
            return R"(texelFetch(Texture_1, ivec3(1, 2, 3), 4);)";
        case ValidTextureOverload::kLoad2dArrayLevelI32:
        case ValidTextureOverload::kLoad3dLevelI32:
            return R"(texelFetch(Texture_1, ivec3(uvec3(1u, 2u, 3u)), int(4u));)";
        case ValidTextureOverload::kLoadMultisampled2dF32:
        case ValidTextureOverload::kLoadMultisampled2dU32:
            return R"(texelFetch(Texture_1, ivec2(1, 2), 3);)";
        case ValidTextureOverload::kLoadMultisampled2dI32:
            return R"(texelFetch(Texture_1, ivec2(uvec2(1u, 2u)), int(3u));)";
        case ValidTextureOverload::kLoadDepth2dLevelF32:
            return R"(texelFetch(Texture_1, ivec2(1, 2), 3).x;)";
        case ValidTextureOverload::kLoadDepth2dArrayLevelF32:
            return R"(texelFetch(Texture_1, ivec3(uvec3(1u, 2u, 3u)), int(4u)).x;)";
        case ValidTextureOverload::kLoadDepthMultisampled2dF32:
            return R"(texelFetch(Texture_1, ivec2(uvec2(1u, 2u)), int(3u)).x;)";
        case ValidTextureOverload::kStoreWO1dRgba32float:
            return R"(imageStore(Texture, ivec2(1, 0), vec4(2.0f, 3.0f, 4.0f, 5.0f));)";
        case ValidTextureOverload::kStoreWO2dRgba32float:
            return R"(imageStore(Texture, ivec2(1, 2), vec4(3.0f, 4.0f, 5.0f, 6.0f));)";
        case ValidTextureOverload::kStoreWO2dArrayRgba32float:
            return R"(imageStore(Texture, ivec3(uvec3(1u, 2u, 3u)), vec4(4.0f, 5.0f, 6.0f, 7.0f));)";
        case ValidTextureOverload::kStoreWO3dRgba32float:
            return R"(imageStore(Texture, ivec3(uvec3(1u, 2u, 3u)), vec4(4.0f, 5.0f, 6.0f, 7.0f));)";
    }
    return "<unmatched texture overload>";
}  // NOLINT - Ignore the length of this function

class GlslGeneratorBuiltinTextureTest : public TestParamHelper<ast::test::TextureOverloadCase> {};

TEST_P(GlslGeneratorBuiltinTextureTest, Call) {
    auto param = GetParam();

    param.BuildTextureVariable(this);
    param.BuildSamplerVariable(this);

    auto* call = Call(param.function, param.args(this));
    auto* stmt = param.returns_value ? static_cast<const ast::Statement*>(Decl(Var("v", call)))
                                     : static_cast<const ast::Statement*>(CallStmt(call));

    Func("main", tint::Empty, ty.void_(), Vector{stmt},
         Vector{Stage(ast::PipelineStage::kFragment)});

    ASTPrinter& gen = SanitizeAndBuild();

    gen.Generate();
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());

    auto expected = expected_texture_overload(param.overload);

    EXPECT_THAT(gen.Result(), HasSubstr(expected.pre));
    EXPECT_THAT(gen.Result(), HasSubstr(expected.out));
}

INSTANTIATE_TEST_SUITE_P(GlslGeneratorBuiltinTextureTest,
                         GlslGeneratorBuiltinTextureTest,
                         testing::ValuesIn(ast::test::TextureOverloadCase::ValidCases()));

}  // namespace
}  // namespace tint::glsl::writer
