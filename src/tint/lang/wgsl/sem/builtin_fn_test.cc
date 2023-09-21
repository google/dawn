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

#include "src/tint/lang/wgsl/sem/builtin_fn.h"

#include "gtest/gtest.h"

namespace tint::sem {
namespace {

struct BuiltinData {
    const char* name;
    core::BuiltinFn builtin;
};

inline std::ostream& operator<<(std::ostream& out, BuiltinData data) {
    out << data.name;
    return out;
}

using BuiltinFunctionTest = testing::TestWithParam<BuiltinData>;

TEST_P(BuiltinFunctionTest, Parse) {
    auto param = GetParam();
    EXPECT_EQ(core::ParseBuiltinFn(param.name), param.builtin);
}

INSTANTIATE_TEST_SUITE_P(
    BuiltinFunctionTest,
    BuiltinFunctionTest,
    testing::Values(BuiltinData{"abs", core::BuiltinFn::kAbs},
                    BuiltinData{"acos", core::BuiltinFn::kAcos},
                    BuiltinData{"all", core::BuiltinFn::kAll},
                    BuiltinData{"any", core::BuiltinFn::kAny},
                    BuiltinData{"arrayLength", core::BuiltinFn::kArrayLength},
                    BuiltinData{"asin", core::BuiltinFn::kAsin},
                    BuiltinData{"atan", core::BuiltinFn::kAtan},
                    BuiltinData{"atan2", core::BuiltinFn::kAtan2},
                    BuiltinData{"ceil", core::BuiltinFn::kCeil},
                    BuiltinData{"clamp", core::BuiltinFn::kClamp},
                    BuiltinData{"cos", core::BuiltinFn::kCos},
                    BuiltinData{"cosh", core::BuiltinFn::kCosh},
                    BuiltinData{"countOneBits", core::BuiltinFn::kCountOneBits},
                    BuiltinData{"cross", core::BuiltinFn::kCross},
                    BuiltinData{"determinant", core::BuiltinFn::kDeterminant},
                    BuiltinData{"distance", core::BuiltinFn::kDistance},
                    BuiltinData{"dot", core::BuiltinFn::kDot},
                    BuiltinData{"dot4I8Packed", core::BuiltinFn::kDot4I8Packed},
                    BuiltinData{"dot4U8Packed", core::BuiltinFn::kDot4U8Packed},
                    BuiltinData{"dpdx", core::BuiltinFn::kDpdx},
                    BuiltinData{"dpdxCoarse", core::BuiltinFn::kDpdxCoarse},
                    BuiltinData{"dpdxFine", core::BuiltinFn::kDpdxFine},
                    BuiltinData{"dpdy", core::BuiltinFn::kDpdy},
                    BuiltinData{"dpdyCoarse", core::BuiltinFn::kDpdyCoarse},
                    BuiltinData{"dpdyFine", core::BuiltinFn::kDpdyFine},
                    BuiltinData{"exp", core::BuiltinFn::kExp},
                    BuiltinData{"exp2", core::BuiltinFn::kExp2},
                    BuiltinData{"faceForward", core::BuiltinFn::kFaceForward},
                    BuiltinData{"floor", core::BuiltinFn::kFloor},
                    BuiltinData{"fma", core::BuiltinFn::kFma},
                    BuiltinData{"fract", core::BuiltinFn::kFract},
                    BuiltinData{"frexp", core::BuiltinFn::kFrexp},
                    BuiltinData{"fwidth", core::BuiltinFn::kFwidth},
                    BuiltinData{"fwidthCoarse", core::BuiltinFn::kFwidthCoarse},
                    BuiltinData{"fwidthFine", core::BuiltinFn::kFwidthFine},
                    BuiltinData{"inverseSqrt", core::BuiltinFn::kInverseSqrt},
                    BuiltinData{"ldexp", core::BuiltinFn::kLdexp},
                    BuiltinData{"length", core::BuiltinFn::kLength},
                    BuiltinData{"log", core::BuiltinFn::kLog},
                    BuiltinData{"log2", core::BuiltinFn::kLog2},
                    BuiltinData{"max", core::BuiltinFn::kMax},
                    BuiltinData{"min", core::BuiltinFn::kMin},
                    BuiltinData{"mix", core::BuiltinFn::kMix},
                    BuiltinData{"modf", core::BuiltinFn::kModf},
                    BuiltinData{"normalize", core::BuiltinFn::kNormalize},
                    BuiltinData{"pow", core::BuiltinFn::kPow},
                    BuiltinData{"reflect", core::BuiltinFn::kReflect},
                    BuiltinData{"reverseBits", core::BuiltinFn::kReverseBits},
                    BuiltinData{"round", core::BuiltinFn::kRound},
                    BuiltinData{"select", core::BuiltinFn::kSelect},
                    BuiltinData{"sign", core::BuiltinFn::kSign},
                    BuiltinData{"sin", core::BuiltinFn::kSin},
                    BuiltinData{"sinh", core::BuiltinFn::kSinh},
                    BuiltinData{"smoothstep", core::BuiltinFn::kSmoothstep},
                    BuiltinData{"sqrt", core::BuiltinFn::kSqrt},
                    BuiltinData{"step", core::BuiltinFn::kStep},
                    BuiltinData{"storageBarrier", core::BuiltinFn::kStorageBarrier},
                    BuiltinData{"tan", core::BuiltinFn::kTan},
                    BuiltinData{"tanh", core::BuiltinFn::kTanh},
                    BuiltinData{"textureDimensions", core::BuiltinFn::kTextureDimensions},
                    BuiltinData{"textureLoad", core::BuiltinFn::kTextureLoad},
                    BuiltinData{"textureNumLayers", core::BuiltinFn::kTextureNumLayers},
                    BuiltinData{"textureNumLevels", core::BuiltinFn::kTextureNumLevels},
                    BuiltinData{"textureNumSamples", core::BuiltinFn::kTextureNumSamples},
                    BuiltinData{"textureSample", core::BuiltinFn::kTextureSample},
                    BuiltinData{"textureSampleBias", core::BuiltinFn::kTextureSampleBias},
                    BuiltinData{"textureSampleCompare", core::BuiltinFn::kTextureSampleCompare},
                    BuiltinData{"textureSampleCompareLevel",
                                core::BuiltinFn::kTextureSampleCompareLevel},
                    BuiltinData{"textureSampleGrad", core::BuiltinFn::kTextureSampleGrad},
                    BuiltinData{"textureSampleLevel", core::BuiltinFn::kTextureSampleLevel},
                    BuiltinData{"trunc", core::BuiltinFn::kTrunc},
                    BuiltinData{"unpack2x16float", core::BuiltinFn::kUnpack2X16Float},
                    BuiltinData{"unpack2x16snorm", core::BuiltinFn::kUnpack2X16Snorm},
                    BuiltinData{"unpack2x16unorm", core::BuiltinFn::kUnpack2X16Unorm},
                    BuiltinData{"unpack4x8snorm", core::BuiltinFn::kUnpack4X8Snorm},
                    BuiltinData{"unpack4x8unorm", core::BuiltinFn::kUnpack4X8Unorm},
                    BuiltinData{"workgroupBarrier", core::BuiltinFn::kWorkgroupBarrier},
                    BuiltinData{"workgroupUniformLoad", core::BuiltinFn::kWorkgroupUniformLoad}));

TEST_F(BuiltinFunctionTest, ParseNoMatch) {
    EXPECT_EQ(core::ParseBuiltinFn("not_builtin"), core::BuiltinFn::kNone);
}

}  // namespace
}  // namespace tint::sem
