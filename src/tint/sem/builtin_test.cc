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
    BuiltinType builtin;
};

inline std::ostream& operator<<(std::ostream& out, BuiltinData data) {
    out << data.name;
    return out;
}

using BuiltinTypeTest = testing::TestWithParam<BuiltinData>;

TEST_P(BuiltinTypeTest, Parse) {
    auto param = GetParam();
    EXPECT_EQ(ParseBuiltinType(param.name), param.builtin);
}

INSTANTIATE_TEST_SUITE_P(
    BuiltinTypeTest,
    BuiltinTypeTest,
    testing::Values(BuiltinData{"abs", BuiltinType::kAbs},
                    BuiltinData{"acos", BuiltinType::kAcos},
                    BuiltinData{"all", BuiltinType::kAll},
                    BuiltinData{"any", BuiltinType::kAny},
                    BuiltinData{"arrayLength", BuiltinType::kArrayLength},
                    BuiltinData{"asin", BuiltinType::kAsin},
                    BuiltinData{"atan", BuiltinType::kAtan},
                    BuiltinData{"atan2", BuiltinType::kAtan2},
                    BuiltinData{"ceil", BuiltinType::kCeil},
                    BuiltinData{"clamp", BuiltinType::kClamp},
                    BuiltinData{"cos", BuiltinType::kCos},
                    BuiltinData{"cosh", BuiltinType::kCosh},
                    BuiltinData{"countOneBits", BuiltinType::kCountOneBits},
                    BuiltinData{"cross", BuiltinType::kCross},
                    BuiltinData{"determinant", BuiltinType::kDeterminant},
                    BuiltinData{"distance", BuiltinType::kDistance},
                    BuiltinData{"dot", BuiltinType::kDot},
                    BuiltinData{"dot4I8Packed", BuiltinType::kDot4I8Packed},
                    BuiltinData{"dot4U8Packed", BuiltinType::kDot4U8Packed},
                    BuiltinData{"dpdx", BuiltinType::kDpdx},
                    BuiltinData{"dpdxCoarse", BuiltinType::kDpdxCoarse},
                    BuiltinData{"dpdxFine", BuiltinType::kDpdxFine},
                    BuiltinData{"dpdy", BuiltinType::kDpdy},
                    BuiltinData{"dpdyCoarse", BuiltinType::kDpdyCoarse},
                    BuiltinData{"dpdyFine", BuiltinType::kDpdyFine},
                    BuiltinData{"exp", BuiltinType::kExp},
                    BuiltinData{"exp2", BuiltinType::kExp2},
                    BuiltinData{"faceForward", BuiltinType::kFaceForward},
                    BuiltinData{"floor", BuiltinType::kFloor},
                    BuiltinData{"fma", BuiltinType::kFma},
                    BuiltinData{"fract", BuiltinType::kFract},
                    BuiltinData{"frexp", BuiltinType::kFrexp},
                    BuiltinData{"fwidth", BuiltinType::kFwidth},
                    BuiltinData{"fwidthCoarse", BuiltinType::kFwidthCoarse},
                    BuiltinData{"fwidthFine", BuiltinType::kFwidthFine},
                    BuiltinData{"inverseSqrt", BuiltinType::kInverseSqrt},
                    BuiltinData{"ldexp", BuiltinType::kLdexp},
                    BuiltinData{"length", BuiltinType::kLength},
                    BuiltinData{"log", BuiltinType::kLog},
                    BuiltinData{"log2", BuiltinType::kLog2},
                    BuiltinData{"max", BuiltinType::kMax},
                    BuiltinData{"min", BuiltinType::kMin},
                    BuiltinData{"mix", BuiltinType::kMix},
                    BuiltinData{"modf", BuiltinType::kModf},
                    BuiltinData{"normalize", BuiltinType::kNormalize},
                    BuiltinData{"pow", BuiltinType::kPow},
                    BuiltinData{"reflect", BuiltinType::kReflect},
                    BuiltinData{"reverseBits", BuiltinType::kReverseBits},
                    BuiltinData{"round", BuiltinType::kRound},
                    BuiltinData{"select", BuiltinType::kSelect},
                    BuiltinData{"sign", BuiltinType::kSign},
                    BuiltinData{"sin", BuiltinType::kSin},
                    BuiltinData{"sinh", BuiltinType::kSinh},
                    BuiltinData{"smoothstep", BuiltinType::kSmoothstep},
                    BuiltinData{"sqrt", BuiltinType::kSqrt},
                    BuiltinData{"step", BuiltinType::kStep},
                    BuiltinData{"storageBarrier", BuiltinType::kStorageBarrier},
                    BuiltinData{"tan", BuiltinType::kTan},
                    BuiltinData{"tanh", BuiltinType::kTanh},
                    BuiltinData{"textureDimensions", BuiltinType::kTextureDimensions},
                    BuiltinData{"textureLoad", BuiltinType::kTextureLoad},
                    BuiltinData{"textureNumLayers", BuiltinType::kTextureNumLayers},
                    BuiltinData{"textureNumLevels", BuiltinType::kTextureNumLevels},
                    BuiltinData{"textureNumSamples", BuiltinType::kTextureNumSamples},
                    BuiltinData{"textureSample", BuiltinType::kTextureSample},
                    BuiltinData{"textureSampleBias", BuiltinType::kTextureSampleBias},
                    BuiltinData{"textureSampleCompare", BuiltinType::kTextureSampleCompare},
                    BuiltinData{"textureSampleCompareLevel",
                                BuiltinType::kTextureSampleCompareLevel},
                    BuiltinData{"textureSampleGrad", BuiltinType::kTextureSampleGrad},
                    BuiltinData{"textureSampleLevel", BuiltinType::kTextureSampleLevel},
                    BuiltinData{"trunc", BuiltinType::kTrunc},
                    BuiltinData{"unpack2x16float", BuiltinType::kUnpack2X16Float},
                    BuiltinData{"unpack2x16snorm", BuiltinType::kUnpack2X16Snorm},
                    BuiltinData{"unpack2x16unorm", BuiltinType::kUnpack2X16Unorm},
                    BuiltinData{"unpack4x8snorm", BuiltinType::kUnpack4X8Snorm},
                    BuiltinData{"unpack4x8unorm", BuiltinType::kUnpack4X8Unorm},
                    BuiltinData{"workgroupBarrier", BuiltinType::kWorkgroupBarrier},
                    BuiltinData{"workgroupUniformLoad", BuiltinType::kWorkgroupUniformLoad}));

TEST_F(BuiltinTypeTest, ParseNoMatch) {
    EXPECT_EQ(ParseBuiltinType("not_builtin"), BuiltinType::kNone);
}

}  // namespace
}  // namespace tint::sem
