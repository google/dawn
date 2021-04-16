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

#include "src/sem/intrinsic.h"

#include "gtest/gtest.h"

namespace tint {
namespace sem {
namespace {

struct IntrinsicData {
  const char* name;
  IntrinsicType intrinsic;
};

inline std::ostream& operator<<(std::ostream& out, IntrinsicData data) {
  out << data.name;
  return out;
}

using IntrinsicTypeTest = testing::TestWithParam<IntrinsicData>;

TEST_P(IntrinsicTypeTest, Parse) {
  auto param = GetParam();
  EXPECT_EQ(ParseIntrinsicType(param.name), param.intrinsic);
}

INSTANTIATE_TEST_SUITE_P(
    IntrinsicTypeTest,
    IntrinsicTypeTest,
    testing::Values(
        IntrinsicData{"abs", IntrinsicType::kAbs},
        IntrinsicData{"acos", IntrinsicType::kAcos},
        IntrinsicData{"all", IntrinsicType::kAll},
        IntrinsicData{"any", IntrinsicType::kAny},
        IntrinsicData{"arrayLength", IntrinsicType::kArrayLength},
        IntrinsicData{"asin", IntrinsicType::kAsin},
        IntrinsicData{"atan", IntrinsicType::kAtan},
        IntrinsicData{"atan2", IntrinsicType::kAtan2},
        IntrinsicData{"ceil", IntrinsicType::kCeil},
        IntrinsicData{"clamp", IntrinsicType::kClamp},
        IntrinsicData{"cos", IntrinsicType::kCos},
        IntrinsicData{"cosh", IntrinsicType::kCosh},
        IntrinsicData{"countOneBits", IntrinsicType::kCountOneBits},
        IntrinsicData{"cross", IntrinsicType::kCross},
        IntrinsicData{"determinant", IntrinsicType::kDeterminant},
        IntrinsicData{"distance", IntrinsicType::kDistance},
        IntrinsicData{"dot", IntrinsicType::kDot},
        IntrinsicData{"dpdx", IntrinsicType::kDpdx},
        IntrinsicData{"dpdxCoarse", IntrinsicType::kDpdxCoarse},
        IntrinsicData{"dpdxFine", IntrinsicType::kDpdxFine},
        IntrinsicData{"dpdy", IntrinsicType::kDpdy},
        IntrinsicData{"dpdyCoarse", IntrinsicType::kDpdyCoarse},
        IntrinsicData{"dpdyFine", IntrinsicType::kDpdyFine},
        IntrinsicData{"exp", IntrinsicType::kExp},
        IntrinsicData{"exp2", IntrinsicType::kExp2},
        IntrinsicData{"faceForward", IntrinsicType::kFaceForward},
        IntrinsicData{"floor", IntrinsicType::kFloor},
        IntrinsicData{"fma", IntrinsicType::kFma},
        IntrinsicData{"fract", IntrinsicType::kFract},
        IntrinsicData{"frexp", IntrinsicType::kFrexp},
        IntrinsicData{"fwidth", IntrinsicType::kFwidth},
        IntrinsicData{"fwidthCoarse", IntrinsicType::kFwidthCoarse},
        IntrinsicData{"fwidthFine", IntrinsicType::kFwidthFine},
        IntrinsicData{"inverseSqrt", IntrinsicType::kInverseSqrt},
        IntrinsicData{"isFinite", IntrinsicType::kIsFinite},
        IntrinsicData{"isInf", IntrinsicType::kIsInf},
        IntrinsicData{"isNan", IntrinsicType::kIsNan},
        IntrinsicData{"isNormal", IntrinsicType::kIsNormal},
        IntrinsicData{"ldexp", IntrinsicType::kLdexp},
        IntrinsicData{"length", IntrinsicType::kLength},
        IntrinsicData{"log", IntrinsicType::kLog},
        IntrinsicData{"log2", IntrinsicType::kLog2},
        IntrinsicData{"max", IntrinsicType::kMax},
        IntrinsicData{"min", IntrinsicType::kMin},
        IntrinsicData{"mix", IntrinsicType::kMix},
        IntrinsicData{"modf", IntrinsicType::kModf},
        IntrinsicData{"normalize", IntrinsicType::kNormalize},
        IntrinsicData{"pow", IntrinsicType::kPow},
        IntrinsicData{"reflect", IntrinsicType::kReflect},
        IntrinsicData{"reverseBits", IntrinsicType::kReverseBits},
        IntrinsicData{"round", IntrinsicType::kRound},
        IntrinsicData{"select", IntrinsicType::kSelect},
        IntrinsicData{"sign", IntrinsicType::kSign},
        IntrinsicData{"sin", IntrinsicType::kSin},
        IntrinsicData{"sinh", IntrinsicType::kSinh},
        IntrinsicData{"smoothStep", IntrinsicType::kSmoothStep},
        IntrinsicData{"sqrt", IntrinsicType::kSqrt},
        IntrinsicData{"step", IntrinsicType::kStep},
        IntrinsicData{"storageBarrier", IntrinsicType::kStorageBarrier},
        IntrinsicData{"tan", IntrinsicType::kTan},
        IntrinsicData{"tanh", IntrinsicType::kTanh},
        IntrinsicData{"textureDimensions", IntrinsicType::kTextureDimensions},
        IntrinsicData{"textureLoad", IntrinsicType::kTextureLoad},
        IntrinsicData{"textureNumLayers", IntrinsicType::kTextureNumLayers},
        IntrinsicData{"textureNumLevels", IntrinsicType::kTextureNumLevels},
        IntrinsicData{"textureNumSamples", IntrinsicType::kTextureNumSamples},
        IntrinsicData{"textureSample", IntrinsicType::kTextureSample},
        IntrinsicData{"textureSampleBias", IntrinsicType::kTextureSampleBias},
        IntrinsicData{"textureSampleCompare",
                      IntrinsicType::kTextureSampleCompare},
        IntrinsicData{"textureSampleGrad", IntrinsicType::kTextureSampleGrad},
        IntrinsicData{"textureSampleLevel", IntrinsicType::kTextureSampleLevel},
        IntrinsicData{"trunc", IntrinsicType::kTrunc},
        IntrinsicData{"unpack2x16float", IntrinsicType::kUnpack2x16Float},
        IntrinsicData{"unpack2x16snorm", IntrinsicType::kUnpack2x16Snorm},
        IntrinsicData{"unpack2x16unorm", IntrinsicType::kUnpack2x16Unorm},
        IntrinsicData{"unpack4x8snorm", IntrinsicType::kUnpack4x8Snorm},
        IntrinsicData{"unpack4x8unorm", IntrinsicType::kUnpack4x8Unorm},
        IntrinsicData{"workgroupBarrier", IntrinsicType::kWorkgroupBarrier}));

TEST_F(IntrinsicTypeTest, ParseNoMatch) {
  EXPECT_EQ(ParseIntrinsicType("not_intrinsic"), IntrinsicType::kNone);
}

}  // namespace
}  // namespace sem
}  // namespace tint
