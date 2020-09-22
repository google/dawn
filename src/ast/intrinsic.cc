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
  switch (i) {
    case Intrinsic::kAbs:
      out << "abs";
      break;
    case Intrinsic::kAcos:
      out << "acos";
      break;
    case Intrinsic::kAll:
      out << "all";
      break;
    case Intrinsic::kAny:
      out << "any";
      break;
    case Intrinsic::kAsin:
      out << "asin";
      break;
    case Intrinsic::kAtan:
      out << "atan";
      break;
    case Intrinsic::kAtan2:
      out << "atan2";
      break;
    case Intrinsic::kCeil:
      out << "ceil";
      break;
    case Intrinsic::kClamp:
      out << "clamp";
      break;
    case Intrinsic::kCos:
      out << "cos";
      break;
    case Intrinsic::kCosh:
      out << "cosh";
      break;
    case Intrinsic::kCountOneBits:
      out << "countOneBits";
      break;
    case Intrinsic::kCross:
      out << "cross";
      break;
    case Intrinsic::kDeterminant:
      out << "determinant";
      break;
    case Intrinsic::kDistance:
      out << "distance";
      break;
    case Intrinsic::kDot:
      out << "dot";
      break;
    case Intrinsic::kDpdx:
      out << "dpdx";
      break;
    case Intrinsic::kDpdxCoarse:
      out << "dpdxCoarse";
      break;
    case Intrinsic::kDpdxFine:
      out << "dpdxFine";
      break;
    case Intrinsic::kDpdy:
      out << "dpdy";
      break;
    case Intrinsic::kDpdyCoarse:
      out << "dpdyCoarse";
      break;
    case Intrinsic::kDpdyFine:
      out << "dpdyFine";
      break;
    case Intrinsic::kExp:
      out << "exp";
      break;
    case Intrinsic::kExp2:
      out << "exp2";
      break;
    case Intrinsic::kFaceForward:
      out << "faceForward";
      break;
    case Intrinsic::kFloor:
      out << "floor";
      break;
    case Intrinsic::kFma:
      out << "fma";
      break;
    case Intrinsic::kFract:
      out << "fract";
      break;
    case Intrinsic::kFrexp:
      out << "frexp";
      break;
    case Intrinsic::kFwidth:
      out << "fwidth";
      break;
    case Intrinsic::kFwidthCoarse:
      out << "fwidthCoarse";
      break;
    case Intrinsic::kFwidthFine:
      out << "fwidthFine";
      break;
    case Intrinsic::kInverseSqrt:
      out << "inverseSqrt";
      break;
    case Intrinsic::kIsFinite:
      out << "isFinite";
      break;
    case Intrinsic::kIsInf:
      out << "isInf";
      break;
    case Intrinsic::kIsNan:
      out << "isNan";
      break;
    case Intrinsic::kIsNormal:
      out << "isNormal";
      break;
    case Intrinsic::kLdexp:
      out << "ldexp";
      break;
    case Intrinsic::kLength:
      out << "length";
      break;
    case Intrinsic::kLog:
      out << "log";
      break;
    case Intrinsic::kLog2:
      out << "log2";
      break;
    case Intrinsic::kMax:
      out << "max";
      break;
    case Intrinsic::kMin:
      out << "min";
      break;
    case Intrinsic::kMix:
      out << "mix";
      break;
    case Intrinsic::kModf:
      out << "modf";
      break;
    case Intrinsic::kNormalize:
      out << "normalize";
      break;
    case Intrinsic::kOuterProduct:
      out << "outerProduct";
      break;
    case Intrinsic::kPow:
      out << "pow";
      break;
    case Intrinsic::kReflect:
      out << "reflect";
      break;
    case Intrinsic::kReverseBits:
      out << "reverseBits";
      break;
    case Intrinsic::kRound:
      out << "round";
      break;
    case Intrinsic::kSelect:
      out << "select";
      break;
    case Intrinsic::kSign:
      out << "sign";
      break;
    case Intrinsic::kSin:
      out << "sin";
      break;
    case Intrinsic::kSinh:
      out << "sinh";
      break;
    case Intrinsic::kSmoothStep:
      out << "smoothStep";
      break;
    case Intrinsic::kSqrt:
      out << "sqrt";
      break;
    case Intrinsic::kStep:
      out << "step";
      break;
    case Intrinsic::kTan:
      out << "tan";
      break;
    case Intrinsic::kTanh:
      out << "tanh";
      break;
    case Intrinsic::kTextureLoad:
      out << "textureLoad";
      break;
    case Intrinsic::kTextureSample:
      out << "textureSample";
      break;
    case Intrinsic::kTextureSampleBias:
      out << "textureSampleBias";
      break;
    case Intrinsic::kTextureSampleCompare:
      out << "textureSampleCompare";
      break;
    case Intrinsic::kTextureSampleLevel:
      out << "textureSampleLevel";
      break;
    case Intrinsic::kTrunc:
      out << "trunc";
      break;
    default:
      out << "Unknown";
      break;
  }
  return out;
}

namespace intrinsic {

bool IsCoarseDerivative(ast::Intrinsic i) {
  return i == Intrinsic::kDpdxCoarse ||
         i == Intrinsic::kDpdyCoarse | i == Intrinsic::kFwidthCoarse;
}

bool IsFineDerivative(ast::Intrinsic i) {
  return i == Intrinsic::kDpdxFine || i == Intrinsic::kDpdyFine ||
         i == Intrinsic::kFwidthFine;
}

bool IsDerivative(ast::Intrinsic i) {
  return i == Intrinsic::kDpdx || i == Intrinsic::kDpdy ||
         i == Intrinsic::kFwidth || IsCoarseDerivative(i) ||
         IsFineDerivative(i);
}

bool IsFloatClassificationIntrinsic(ast::Intrinsic i) {
  return i == Intrinsic::kIsFinite || i == Intrinsic::kIsInf ||
         i == Intrinsic::kIsNan || i == Intrinsic::kIsNormal;
}

bool IsTextureIntrinsic(ast::Intrinsic i) {
  return i == Intrinsic::kTextureLoad || i == Intrinsic::kTextureSample ||
         i == Intrinsic::kTextureSampleLevel ||
         i == Intrinsic::kTextureSampleBias ||
         i == Intrinsic::kTextureSampleCompare;
}

}  // namespace intrinsic
}  // namespace ast
}  // namespace tint
