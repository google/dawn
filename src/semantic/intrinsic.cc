// Copyright 2020 The Tint Authors.
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

#include "src/semantic/intrinsic.h"

namespace tint {
namespace semantic {

std::ostream& operator<<(std::ostream& out, Intrinsic i) {
  out << intrinsic::str(i);
  return out;
}

namespace intrinsic {

const char* str(Intrinsic i) {
  /// The emitted name matches the spelling in the WGSL spec.
  /// including case.
  switch (i) {
    case Intrinsic::kNone:
      return "<not-an-intrinsic>";
    case Intrinsic::kAbs:
      return "abs";
    case Intrinsic::kAcos:
      return "acos";
    case Intrinsic::kAll:
      return "all";
    case Intrinsic::kAny:
      return "any";
    case Intrinsic::kArrayLength:
      return "arrayLength";
    case Intrinsic::kAsin:
      return "asin";
    case Intrinsic::kAtan:
      return "atan";
    case Intrinsic::kAtan2:
      return "atan2";
    case Intrinsic::kCeil:
      return "ceil";
    case Intrinsic::kClamp:
      return "clamp";
    case Intrinsic::kCos:
      return "cos";
    case Intrinsic::kCosh:
      return "cosh";
    case Intrinsic::kCountOneBits:
      return "countOneBits";
    case Intrinsic::kCross:
      return "cross";
    case Intrinsic::kDeterminant:
      return "determinant";
    case Intrinsic::kDistance:
      return "distance";
    case Intrinsic::kDot:
      return "dot";
    case Intrinsic::kDpdx:
      return "dpdx";
    case Intrinsic::kDpdxCoarse:
      return "dpdxCoarse";
    case Intrinsic::kDpdxFine:
      return "dpdxFine";
    case Intrinsic::kDpdy:
      return "dpdy";
    case Intrinsic::kDpdyCoarse:
      return "dpdyCoarse";
    case Intrinsic::kDpdyFine:
      return "dpdyFine";
    case Intrinsic::kExp:
      return "exp";
    case Intrinsic::kExp2:
      return "exp2";
    case Intrinsic::kFaceForward:
      return "faceForward";
    case Intrinsic::kFloor:
      return "floor";
    case Intrinsic::kFma:
      return "fma";
    case Intrinsic::kFract:
      return "fract";
    case Intrinsic::kFrexp:
      return "frexp";
    case Intrinsic::kFwidth:
      return "fwidth";
    case Intrinsic::kFwidthCoarse:
      return "fwidthCoarse";
    case Intrinsic::kFwidthFine:
      return "fwidthFine";
    case Intrinsic::kInverseSqrt:
      return "inverseSqrt";
    case Intrinsic::kIsFinite:
      return "isFinite";
    case Intrinsic::kIsInf:
      return "isInf";
    case Intrinsic::kIsNan:
      return "isNan";
    case Intrinsic::kIsNormal:
      return "isNormal";
    case Intrinsic::kLdexp:
      return "ldexp";
    case Intrinsic::kLength:
      return "length";
    case Intrinsic::kLog:
      return "log";
    case Intrinsic::kLog2:
      return "log2";
    case Intrinsic::kMax:
      return "max";
    case Intrinsic::kMin:
      return "min";
    case Intrinsic::kMix:
      return "mix";
    case Intrinsic::kModf:
      return "modf";
    case Intrinsic::kNormalize:
      return "normalize";
    case Intrinsic::kPow:
      return "pow";
    case Intrinsic::kReflect:
      return "reflect";
    case Intrinsic::kReverseBits:
      return "reverseBits";
    case Intrinsic::kRound:
      return "round";
    case Intrinsic::kSelect:
      return "select";
    case Intrinsic::kSign:
      return "sign";
    case Intrinsic::kSin:
      return "sin";
    case Intrinsic::kSinh:
      return "sinh";
    case Intrinsic::kSmoothStep:
      return "smoothStep";
    case Intrinsic::kSqrt:
      return "sqrt";
    case Intrinsic::kStep:
      return "step";
    case Intrinsic::kTan:
      return "tan";
    case Intrinsic::kTanh:
      return "tanh";
    case Intrinsic::kTextureDimensions:
      return "textureDimensions";
    case Intrinsic::kTextureLoad:
      return "textureLoad";
    case Intrinsic::kTextureNumLayers:
      return "textureNumLayers";
    case Intrinsic::kTextureNumLevels:
      return "textureNumLevels";
    case Intrinsic::kTextureNumSamples:
      return "textureNumSamples";
    case Intrinsic::kTextureSample:
      return "textureSample";
    case Intrinsic::kTextureSampleBias:
      return "textureSampleBias";
    case Intrinsic::kTextureSampleCompare:
      return "textureSampleCompare";
    case Intrinsic::kTextureSampleGrad:
      return "textureSampleGrad";
    case Intrinsic::kTextureSampleLevel:
      return "textureSampleLevel";
    case Intrinsic::kTextureStore:
      return "textureStore";
    case Intrinsic::kTrunc:
      return "trunc";
  }
  return "<unknown>";
}

bool IsCoarseDerivative(Intrinsic i) {
  return i == Intrinsic::kDpdxCoarse || i == Intrinsic::kDpdyCoarse ||
         i == Intrinsic::kFwidthCoarse;
}

bool IsFineDerivative(Intrinsic i) {
  return i == Intrinsic::kDpdxFine || i == Intrinsic::kDpdyFine ||
         i == Intrinsic::kFwidthFine;
}

bool IsDerivative(Intrinsic i) {
  return i == Intrinsic::kDpdx || i == Intrinsic::kDpdy ||
         i == Intrinsic::kFwidth || IsCoarseDerivative(i) ||
         IsFineDerivative(i);
}

bool IsFloatClassificationIntrinsic(Intrinsic i) {
  return i == Intrinsic::kIsFinite || i == Intrinsic::kIsInf ||
         i == Intrinsic::kIsNan || i == Intrinsic::kIsNormal;
}

bool IsTextureIntrinsic(Intrinsic i) {
  return IsImageQueryIntrinsic(i) || i == Intrinsic::kTextureLoad ||
         i == Intrinsic::kTextureSample ||
         i == Intrinsic::kTextureSampleLevel ||
         i == Intrinsic::kTextureSampleBias ||
         i == Intrinsic::kTextureSampleCompare ||
         i == Intrinsic::kTextureSampleGrad || i == Intrinsic::kTextureStore;
}

bool IsImageQueryIntrinsic(Intrinsic i) {
  return i == semantic::Intrinsic::kTextureDimensions ||
         i == Intrinsic::kTextureNumLayers ||
         i == Intrinsic::kTextureNumLevels ||
         i == Intrinsic::kTextureNumSamples;
}

}  // namespace intrinsic
}  // namespace semantic
}  // namespace tint
