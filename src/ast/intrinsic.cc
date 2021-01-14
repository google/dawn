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

#include "src/ast/intrinsic.h"

namespace tint {
namespace ast {

std::ostream& operator<<(std::ostream& out, Intrinsic i) {
  /// The emitted name matches the spelling in the WGSL spec.
  /// including case.
  switch (i) {
    case Intrinsic::kNone:
      return out;
    case Intrinsic::kAbs:
      out << "abs";
      return out;
    case Intrinsic::kAcos:
      out << "acos";
      return out;
    case Intrinsic::kAll:
      out << "all";
      return out;
    case Intrinsic::kAny:
      out << "any";
      return out;
    case Intrinsic::kArrayLength:
      out << "arrayLength";
      return out;
    case Intrinsic::kAsin:
      out << "asin";
      return out;
    case Intrinsic::kAtan:
      out << "atan";
      return out;
    case Intrinsic::kAtan2:
      out << "atan2";
      return out;
    case Intrinsic::kCeil:
      out << "ceil";
      return out;
    case Intrinsic::kClamp:
      out << "clamp";
      return out;
    case Intrinsic::kCos:
      out << "cos";
      return out;
    case Intrinsic::kCosh:
      out << "cosh";
      return out;
    case Intrinsic::kCountOneBits:
      out << "countOneBits";
      return out;
    case Intrinsic::kCross:
      out << "cross";
      return out;
    case Intrinsic::kDeterminant:
      out << "determinant";
      return out;
    case Intrinsic::kDistance:
      out << "distance";
      return out;
    case Intrinsic::kDot:
      out << "dot";
      return out;
    case Intrinsic::kDpdx:
      out << "dpdx";
      return out;
    case Intrinsic::kDpdxCoarse:
      out << "dpdxCoarse";
      return out;
    case Intrinsic::kDpdxFine:
      out << "dpdxFine";
      return out;
    case Intrinsic::kDpdy:
      out << "dpdy";
      return out;
    case Intrinsic::kDpdyCoarse:
      out << "dpdyCoarse";
      return out;
    case Intrinsic::kDpdyFine:
      out << "dpdyFine";
      return out;
    case Intrinsic::kExp:
      out << "exp";
      return out;
    case Intrinsic::kExp2:
      out << "exp2";
      return out;
    case Intrinsic::kFaceForward:
      out << "faceForward";
      return out;
    case Intrinsic::kFloor:
      out << "floor";
      return out;
    case Intrinsic::kFma:
      out << "fma";
      return out;
    case Intrinsic::kFract:
      out << "fract";
      return out;
    case Intrinsic::kFrexp:
      out << "frexp";
      return out;
    case Intrinsic::kFwidth:
      out << "fwidth";
      return out;
    case Intrinsic::kFwidthCoarse:
      out << "fwidthCoarse";
      return out;
    case Intrinsic::kFwidthFine:
      out << "fwidthFine";
      return out;
    case Intrinsic::kInverseSqrt:
      out << "inverseSqrt";
      return out;
    case Intrinsic::kIsFinite:
      out << "isFinite";
      return out;
    case Intrinsic::kIsInf:
      out << "isInf";
      return out;
    case Intrinsic::kIsNan:
      out << "isNan";
      return out;
    case Intrinsic::kIsNormal:
      out << "isNormal";
      return out;
    case Intrinsic::kLdexp:
      out << "ldexp";
      return out;
    case Intrinsic::kLength:
      out << "length";
      return out;
    case Intrinsic::kLog:
      out << "log";
      return out;
    case Intrinsic::kLog2:
      out << "log2";
      return out;
    case Intrinsic::kMax:
      out << "max";
      return out;
    case Intrinsic::kMin:
      out << "min";
      return out;
    case Intrinsic::kMix:
      out << "mix";
      return out;
    case Intrinsic::kModf:
      out << "modf";
      return out;
    case Intrinsic::kNormalize:
      out << "normalize";
      return out;
    case Intrinsic::kPow:
      out << "pow";
      return out;
    case Intrinsic::kReflect:
      out << "reflect";
      return out;
    case Intrinsic::kReverseBits:
      out << "reverseBits";
      return out;
    case Intrinsic::kRound:
      out << "round";
      return out;
    case Intrinsic::kSelect:
      out << "select";
      return out;
    case Intrinsic::kSign:
      out << "sign";
      return out;
    case Intrinsic::kSin:
      out << "sin";
      return out;
    case Intrinsic::kSinh:
      out << "sinh";
      return out;
    case Intrinsic::kSmoothStep:
      out << "smoothStep";
      return out;
    case Intrinsic::kSqrt:
      out << "sqrt";
      return out;
    case Intrinsic::kStep:
      out << "step";
      return out;
    case Intrinsic::kTan:
      out << "tan";
      return out;
    case Intrinsic::kTanh:
      out << "tanh";
      return out;
    case Intrinsic::kTextureDimensions:
      out << "textureDimensions";
      return out;
    case Intrinsic::kTextureLoad:
      out << "textureLoad";
      return out;
    case Intrinsic::kTextureNumLayers:
      out << "textureNumLayers";
      return out;
    case Intrinsic::kTextureNumLevels:
      out << "textureNumLevels";
      return out;
    case Intrinsic::kTextureNumSamples:
      out << "textureNumSamples";
      return out;
    case Intrinsic::kTextureSample:
      out << "textureSample";
      return out;
    case Intrinsic::kTextureSampleBias:
      out << "textureSampleBias";
      return out;
    case Intrinsic::kTextureSampleCompare:
      out << "textureSampleCompare";
      return out;
    case Intrinsic::kTextureSampleGrad:
      out << "textureSampleGrad";
      return out;
    case Intrinsic::kTextureSampleLevel:
      out << "textureSampleLevel";
      return out;
    case Intrinsic::kTextureStore:
      out << "textureStore";
      return out;
    case Intrinsic::kTrunc:
      out << "trunc";
      return out;
  }
  out << "Unknown";
  return out;
}

namespace intrinsic {

Signature::~Signature() = default;
TextureSignature::~TextureSignature() = default;

TextureSignature::Parameters::Index::Index() = default;
TextureSignature::Parameters::Index::Index(const Index&) = default;

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
  return i == ast::Intrinsic::kTextureDimensions ||
         i == Intrinsic::kTextureNumLayers ||
         i == Intrinsic::kTextureNumLevels ||
         i == Intrinsic::kTextureNumSamples;
}

}  // namespace intrinsic
}  // namespace ast
}  // namespace tint
