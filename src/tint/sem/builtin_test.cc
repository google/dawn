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

#include "src/tint/sem/builtin.h"

#include "gtest/gtest.h"

namespace tint::sem {
namespace {

struct BuiltinData {
    const char* name;
    builtin::Function builtin;
};

inline std::ostream& operator<<(std::ostream& out, BuiltinData data) {
    out << data.name;
    return out;
}

using BuiltinFunctionTest = testing::TestWithParam<BuiltinData>;

TEST_P(BuiltinFunctionTest, Parse) {
    auto param = GetParam();
    EXPECT_EQ(builtin::ParseFunction(param.name), param.builtin);
}

INSTANTIATE_TEST_SUITE_P(
    BuiltinFunctionTest,
    BuiltinFunctionTest,
    testing::Values(BuiltinData{"abs", builtin::Function::kAbs},
                    BuiltinData{"acos", builtin::Function::kAcos},
                    BuiltinData{"all", builtin::Function::kAll},
                    BuiltinData{"any", builtin::Function::kAny},
                    BuiltinData{"arrayLength", builtin::Function::kArrayLength},
                    BuiltinData{"asin", builtin::Function::kAsin},
                    BuiltinData{"atan", builtin::Function::kAtan},
                    BuiltinData{"atan2", builtin::Function::kAtan2},
                    BuiltinData{"ceil", builtin::Function::kCeil},
                    BuiltinData{"clamp", builtin::Function::kClamp},
                    BuiltinData{"cos", builtin::Function::kCos},
                    BuiltinData{"cosh", builtin::Function::kCosh},
                    BuiltinData{"countOneBits", builtin::Function::kCountOneBits},
                    BuiltinData{"cross", builtin::Function::kCross},
                    BuiltinData{"determinant", builtin::Function::kDeterminant},
                    BuiltinData{"distance", builtin::Function::kDistance},
                    BuiltinData{"dot", builtin::Function::kDot},
                    BuiltinData{"dot4I8Packed", builtin::Function::kDot4I8Packed},
                    BuiltinData{"dot4U8Packed", builtin::Function::kDot4U8Packed},
                    BuiltinData{"dpdx", builtin::Function::kDpdx},
                    BuiltinData{"dpdxCoarse", builtin::Function::kDpdxCoarse},
                    BuiltinData{"dpdxFine", builtin::Function::kDpdxFine},
                    BuiltinData{"dpdy", builtin::Function::kDpdy},
                    BuiltinData{"dpdyCoarse", builtin::Function::kDpdyCoarse},
                    BuiltinData{"dpdyFine", builtin::Function::kDpdyFine},
                    BuiltinData{"exp", builtin::Function::kExp},
                    BuiltinData{"exp2", builtin::Function::kExp2},
                    BuiltinData{"faceForward", builtin::Function::kFaceForward},
                    BuiltinData{"floor", builtin::Function::kFloor},
                    BuiltinData{"fma", builtin::Function::kFma},
                    BuiltinData{"fract", builtin::Function::kFract},
                    BuiltinData{"frexp", builtin::Function::kFrexp},
                    BuiltinData{"fwidth", builtin::Function::kFwidth},
                    BuiltinData{"fwidthCoarse", builtin::Function::kFwidthCoarse},
                    BuiltinData{"fwidthFine", builtin::Function::kFwidthFine},
                    BuiltinData{"inverseSqrt", builtin::Function::kInverseSqrt},
                    BuiltinData{"ldexp", builtin::Function::kLdexp},
                    BuiltinData{"length", builtin::Function::kLength},
                    BuiltinData{"log", builtin::Function::kLog},
                    BuiltinData{"log2", builtin::Function::kLog2},
                    BuiltinData{"max", builtin::Function::kMax},
                    BuiltinData{"min", builtin::Function::kMin},
                    BuiltinData{"mix", builtin::Function::kMix},
                    BuiltinData{"modf", builtin::Function::kModf},
                    BuiltinData{"normalize", builtin::Function::kNormalize},
                    BuiltinData{"pow", builtin::Function::kPow},
                    BuiltinData{"reflect", builtin::Function::kReflect},
                    BuiltinData{"reverseBits", builtin::Function::kReverseBits},
                    BuiltinData{"round", builtin::Function::kRound},
                    BuiltinData{"select", builtin::Function::kSelect},
                    BuiltinData{"sign", builtin::Function::kSign},
                    BuiltinData{"sin", builtin::Function::kSin},
                    BuiltinData{"sinh", builtin::Function::kSinh},
                    BuiltinData{"smoothstep", builtin::Function::kSmoothstep},
                    BuiltinData{"sqrt", builtin::Function::kSqrt},
                    BuiltinData{"step", builtin::Function::kStep},
                    BuiltinData{"storageBarrier", builtin::Function::kStorageBarrier},
                    BuiltinData{"tan", builtin::Function::kTan},
                    BuiltinData{"tanh", builtin::Function::kTanh},
                    BuiltinData{"textureDimensions", builtin::Function::kTextureDimensions},
                    BuiltinData{"textureLoad", builtin::Function::kTextureLoad},
                    BuiltinData{"textureNumLayers", builtin::Function::kTextureNumLayers},
                    BuiltinData{"textureNumLevels", builtin::Function::kTextureNumLevels},
                    BuiltinData{"textureNumSamples", builtin::Function::kTextureNumSamples},
                    BuiltinData{"textureSample", builtin::Function::kTextureSample},
                    BuiltinData{"textureSampleBias", builtin::Function::kTextureSampleBias},
                    BuiltinData{"textureSampleCompare", builtin::Function::kTextureSampleCompare},
                    BuiltinData{"textureSampleCompareLevel",
                                builtin::Function::kTextureSampleCompareLevel},
                    BuiltinData{"textureSampleGrad", builtin::Function::kTextureSampleGrad},
                    BuiltinData{"textureSampleLevel", builtin::Function::kTextureSampleLevel},
                    BuiltinData{"trunc", builtin::Function::kTrunc},
                    BuiltinData{"unpack2x16float", builtin::Function::kUnpack2X16Float},
                    BuiltinData{"unpack2x16snorm", builtin::Function::kUnpack2X16Snorm},
                    BuiltinData{"unpack2x16unorm", builtin::Function::kUnpack2X16Unorm},
                    BuiltinData{"unpack4x8snorm", builtin::Function::kUnpack4X8Snorm},
                    BuiltinData{"unpack4x8unorm", builtin::Function::kUnpack4X8Unorm},
                    BuiltinData{"workgroupBarrier", builtin::Function::kWorkgroupBarrier},
                    BuiltinData{"workgroupUniformLoad", builtin::Function::kWorkgroupUniformLoad}));

TEST_F(BuiltinFunctionTest, ParseNoMatch) {
    EXPECT_EQ(builtin::ParseFunction("not_builtin"), builtin::Function::kNone);
}

}  // namespace
}  // namespace tint::sem
