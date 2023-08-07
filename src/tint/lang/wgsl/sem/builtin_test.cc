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

#include "src/tint/lang/wgsl/sem/builtin.h"

#include "gtest/gtest.h"

namespace tint::sem {
namespace {

struct BuiltinData {
    const char* name;
    core::Function builtin;
};

inline std::ostream& operator<<(std::ostream& out, BuiltinData data) {
    out << data.name;
    return out;
}

using BuiltinFunctionTest = testing::TestWithParam<BuiltinData>;

TEST_P(BuiltinFunctionTest, Parse) {
    auto param = GetParam();
    EXPECT_EQ(core::ParseFunction(param.name), param.builtin);
}

INSTANTIATE_TEST_SUITE_P(
    BuiltinFunctionTest,
    BuiltinFunctionTest,
    testing::Values(BuiltinData{"abs", core::Function::kAbs},
                    BuiltinData{"acos", core::Function::kAcos},
                    BuiltinData{"all", core::Function::kAll},
                    BuiltinData{"any", core::Function::kAny},
                    BuiltinData{"arrayLength", core::Function::kArrayLength},
                    BuiltinData{"asin", core::Function::kAsin},
                    BuiltinData{"atan", core::Function::kAtan},
                    BuiltinData{"atan2", core::Function::kAtan2},
                    BuiltinData{"ceil", core::Function::kCeil},
                    BuiltinData{"clamp", core::Function::kClamp},
                    BuiltinData{"cos", core::Function::kCos},
                    BuiltinData{"cosh", core::Function::kCosh},
                    BuiltinData{"countOneBits", core::Function::kCountOneBits},
                    BuiltinData{"cross", core::Function::kCross},
                    BuiltinData{"determinant", core::Function::kDeterminant},
                    BuiltinData{"distance", core::Function::kDistance},
                    BuiltinData{"dot", core::Function::kDot},
                    BuiltinData{"dot4I8Packed", core::Function::kDot4I8Packed},
                    BuiltinData{"dot4U8Packed", core::Function::kDot4U8Packed},
                    BuiltinData{"dpdx", core::Function::kDpdx},
                    BuiltinData{"dpdxCoarse", core::Function::kDpdxCoarse},
                    BuiltinData{"dpdxFine", core::Function::kDpdxFine},
                    BuiltinData{"dpdy", core::Function::kDpdy},
                    BuiltinData{"dpdyCoarse", core::Function::kDpdyCoarse},
                    BuiltinData{"dpdyFine", core::Function::kDpdyFine},
                    BuiltinData{"exp", core::Function::kExp},
                    BuiltinData{"exp2", core::Function::kExp2},
                    BuiltinData{"faceForward", core::Function::kFaceForward},
                    BuiltinData{"floor", core::Function::kFloor},
                    BuiltinData{"fma", core::Function::kFma},
                    BuiltinData{"fract", core::Function::kFract},
                    BuiltinData{"frexp", core::Function::kFrexp},
                    BuiltinData{"fwidth", core::Function::kFwidth},
                    BuiltinData{"fwidthCoarse", core::Function::kFwidthCoarse},
                    BuiltinData{"fwidthFine", core::Function::kFwidthFine},
                    BuiltinData{"inverseSqrt", core::Function::kInverseSqrt},
                    BuiltinData{"ldexp", core::Function::kLdexp},
                    BuiltinData{"length", core::Function::kLength},
                    BuiltinData{"log", core::Function::kLog},
                    BuiltinData{"log2", core::Function::kLog2},
                    BuiltinData{"max", core::Function::kMax},
                    BuiltinData{"min", core::Function::kMin},
                    BuiltinData{"mix", core::Function::kMix},
                    BuiltinData{"modf", core::Function::kModf},
                    BuiltinData{"normalize", core::Function::kNormalize},
                    BuiltinData{"pow", core::Function::kPow},
                    BuiltinData{"reflect", core::Function::kReflect},
                    BuiltinData{"reverseBits", core::Function::kReverseBits},
                    BuiltinData{"round", core::Function::kRound},
                    BuiltinData{"select", core::Function::kSelect},
                    BuiltinData{"sign", core::Function::kSign},
                    BuiltinData{"sin", core::Function::kSin},
                    BuiltinData{"sinh", core::Function::kSinh},
                    BuiltinData{"smoothstep", core::Function::kSmoothstep},
                    BuiltinData{"sqrt", core::Function::kSqrt},
                    BuiltinData{"step", core::Function::kStep},
                    BuiltinData{"storageBarrier", core::Function::kStorageBarrier},
                    BuiltinData{"tan", core::Function::kTan},
                    BuiltinData{"tanh", core::Function::kTanh},
                    BuiltinData{"textureDimensions", core::Function::kTextureDimensions},
                    BuiltinData{"textureLoad", core::Function::kTextureLoad},
                    BuiltinData{"textureNumLayers", core::Function::kTextureNumLayers},
                    BuiltinData{"textureNumLevels", core::Function::kTextureNumLevels},
                    BuiltinData{"textureNumSamples", core::Function::kTextureNumSamples},
                    BuiltinData{"textureSample", core::Function::kTextureSample},
                    BuiltinData{"textureSampleBias", core::Function::kTextureSampleBias},
                    BuiltinData{"textureSampleCompare", core::Function::kTextureSampleCompare},
                    BuiltinData{"textureSampleCompareLevel",
                                core::Function::kTextureSampleCompareLevel},
                    BuiltinData{"textureSampleGrad", core::Function::kTextureSampleGrad},
                    BuiltinData{"textureSampleLevel", core::Function::kTextureSampleLevel},
                    BuiltinData{"trunc", core::Function::kTrunc},
                    BuiltinData{"unpack2x16float", core::Function::kUnpack2X16Float},
                    BuiltinData{"unpack2x16snorm", core::Function::kUnpack2X16Snorm},
                    BuiltinData{"unpack2x16unorm", core::Function::kUnpack2X16Unorm},
                    BuiltinData{"unpack4x8snorm", core::Function::kUnpack4X8Snorm},
                    BuiltinData{"unpack4x8unorm", core::Function::kUnpack4X8Unorm},
                    BuiltinData{"workgroupBarrier", core::Function::kWorkgroupBarrier},
                    BuiltinData{"workgroupUniformLoad", core::Function::kWorkgroupUniformLoad}));

TEST_F(BuiltinFunctionTest, ParseNoMatch) {
    EXPECT_EQ(core::ParseFunction("not_builtin"), core::Function::kNone);
}

}  // namespace
}  // namespace tint::sem
