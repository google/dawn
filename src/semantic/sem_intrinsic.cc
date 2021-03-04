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

TINT_INSTANTIATE_TYPEINFO(tint::semantic::Intrinsic);

namespace tint {
namespace semantic {

std::ostream& operator<<(std::ostream& out, IntrinsicType i) {
  out << str(i);
  return out;
}

const char* Intrinsic::str() const {
  return semantic::str(type_);
}

IntrinsicType ParseIntrinsicType(const std::string& name) {
  if (name == "abs") {
    return IntrinsicType::kAbs;
  } else if (name == "acos") {
    return IntrinsicType::kAcos;
  } else if (name == "all") {
    return IntrinsicType::kAll;
  } else if (name == "any") {
    return IntrinsicType::kAny;
  } else if (name == "arrayLength") {
    return IntrinsicType::kArrayLength;
  } else if (name == "asin") {
    return IntrinsicType::kAsin;
  } else if (name == "atan") {
    return IntrinsicType::kAtan;
  } else if (name == "atan2") {
    return IntrinsicType::kAtan2;
  } else if (name == "ceil") {
    return IntrinsicType::kCeil;
  } else if (name == "clamp") {
    return IntrinsicType::kClamp;
  } else if (name == "cos") {
    return IntrinsicType::kCos;
  } else if (name == "cosh") {
    return IntrinsicType::kCosh;
  } else if (name == "countOneBits") {
    return IntrinsicType::kCountOneBits;
  } else if (name == "cross") {
    return IntrinsicType::kCross;
  } else if (name == "determinant") {
    return IntrinsicType::kDeterminant;
  } else if (name == "distance") {
    return IntrinsicType::kDistance;
  } else if (name == "dot") {
    return IntrinsicType::kDot;
  } else if (name == "dpdx") {
    return IntrinsicType::kDpdx;
  } else if (name == "dpdxCoarse") {
    return IntrinsicType::kDpdxCoarse;
  } else if (name == "dpdxFine") {
    return IntrinsicType::kDpdxFine;
  } else if (name == "dpdy") {
    return IntrinsicType::kDpdy;
  } else if (name == "dpdyCoarse") {
    return IntrinsicType::kDpdyCoarse;
  } else if (name == "dpdyFine") {
    return IntrinsicType::kDpdyFine;
  } else if (name == "exp") {
    return IntrinsicType::kExp;
  } else if (name == "exp2") {
    return IntrinsicType::kExp2;
  } else if (name == "faceForward") {
    return IntrinsicType::kFaceForward;
  } else if (name == "floor") {
    return IntrinsicType::kFloor;
  } else if (name == "fma") {
    return IntrinsicType::kFma;
  } else if (name == "fract") {
    return IntrinsicType::kFract;
  } else if (name == "frexp") {
    return IntrinsicType::kFrexp;
  } else if (name == "fwidth") {
    return IntrinsicType::kFwidth;
  } else if (name == "fwidthCoarse") {
    return IntrinsicType::kFwidthCoarse;
  } else if (name == "fwidthFine") {
    return IntrinsicType::kFwidthFine;
  } else if (name == "inverseSqrt") {
    return IntrinsicType::kInverseSqrt;
  } else if (name == "isFinite") {
    return IntrinsicType::kIsFinite;
  } else if (name == "isInf") {
    return IntrinsicType::kIsInf;
  } else if (name == "isNan") {
    return IntrinsicType::kIsNan;
  } else if (name == "isNormal") {
    return IntrinsicType::kIsNormal;
  } else if (name == "ldexp") {
    return IntrinsicType::kLdexp;
  } else if (name == "length") {
    return IntrinsicType::kLength;
  } else if (name == "log") {
    return IntrinsicType::kLog;
  } else if (name == "log2") {
    return IntrinsicType::kLog2;
  } else if (name == "max") {
    return IntrinsicType::kMax;
  } else if (name == "min") {
    return IntrinsicType::kMin;
  } else if (name == "mix") {
    return IntrinsicType::kMix;
  } else if (name == "modf") {
    return IntrinsicType::kModf;
  } else if (name == "normalize") {
    return IntrinsicType::kNormalize;
  } else if (name == "pack4x8snorm") {
    return IntrinsicType::kPack4x8Snorm;
  } else if (name == "pack4x8unorm") {
    return IntrinsicType::kPack4x8Unorm;
  } else if (name == "pack2x16snorm") {
    return IntrinsicType::kPack2x16Snorm;
  } else if (name == "pack2x16unorm") {
    return IntrinsicType::kPack2x16Unorm;
  } else if (name == "pack2x16float") {
    return IntrinsicType::kPack2x16Float;
  } else if (name == "pow") {
    return IntrinsicType::kPow;
  } else if (name == "reflect") {
    return IntrinsicType::kReflect;
  } else if (name == "reverseBits") {
    return IntrinsicType::kReverseBits;
  } else if (name == "round") {
    return IntrinsicType::kRound;
  } else if (name == "select") {
    return IntrinsicType::kSelect;
  } else if (name == "sign") {
    return IntrinsicType::kSign;
  } else if (name == "sin") {
    return IntrinsicType::kSin;
  } else if (name == "sinh") {
    return IntrinsicType::kSinh;
  } else if (name == "smoothStep") {
    return IntrinsicType::kSmoothStep;
  } else if (name == "sqrt") {
    return IntrinsicType::kSqrt;
  } else if (name == "step") {
    return IntrinsicType::kStep;
  } else if (name == "tan") {
    return IntrinsicType::kTan;
  } else if (name == "tanh") {
    return IntrinsicType::kTanh;
  } else if (name == "textureDimensions") {
    return IntrinsicType::kTextureDimensions;
  } else if (name == "textureNumLayers") {
    return IntrinsicType::kTextureNumLayers;
  } else if (name == "textureNumLevels") {
    return IntrinsicType::kTextureNumLevels;
  } else if (name == "textureNumSamples") {
    return IntrinsicType::kTextureNumSamples;
  } else if (name == "textureLoad") {
    return IntrinsicType::kTextureLoad;
  } else if (name == "textureStore") {
    return IntrinsicType::kTextureStore;
  } else if (name == "textureSample") {
    return IntrinsicType::kTextureSample;
  } else if (name == "textureSampleBias") {
    return IntrinsicType::kTextureSampleBias;
  } else if (name == "textureSampleCompare") {
    return IntrinsicType::kTextureSampleCompare;
  } else if (name == "textureSampleGrad") {
    return IntrinsicType::kTextureSampleGrad;
  } else if (name == "textureSampleLevel") {
    return IntrinsicType::kTextureSampleLevel;
  } else if (name == "trunc") {
    return IntrinsicType::kTrunc;
  } else if (name == "unpack4x8snorm") {
    return IntrinsicType::kUnpack4x8Snorm;
  } else if (name == "unpack4x8unorm") {
    return IntrinsicType::kUnpack4x8Unorm;
  } else if (name == "unpack2x16snorm") {
    return IntrinsicType::kUnpack2x16Snorm;
  } else if (name == "unpack2x16unorm") {
    return IntrinsicType::kUnpack2x16Unorm;
  } else if (name == "unpack2x16float") {
    return IntrinsicType::kUnpack2x16Float;
  }
  return IntrinsicType::kNone;
}

const char* str(IntrinsicType i) {
  /// The emitted name matches the spelling in the WGSL spec.
  /// including case.
  switch (i) {
    case IntrinsicType::kNone:
      return "<not-an-intrinsic>";
    case IntrinsicType::kAbs:
      return "abs";
    case IntrinsicType::kAcos:
      return "acos";
    case IntrinsicType::kAll:
      return "all";
    case IntrinsicType::kAny:
      return "any";
    case IntrinsicType::kArrayLength:
      return "arrayLength";
    case IntrinsicType::kAsin:
      return "asin";
    case IntrinsicType::kAtan:
      return "atan";
    case IntrinsicType::kAtan2:
      return "atan2";
    case IntrinsicType::kCeil:
      return "ceil";
    case IntrinsicType::kClamp:
      return "clamp";
    case IntrinsicType::kCos:
      return "cos";
    case IntrinsicType::kCosh:
      return "cosh";
    case IntrinsicType::kCountOneBits:
      return "countOneBits";
    case IntrinsicType::kCross:
      return "cross";
    case IntrinsicType::kDeterminant:
      return "determinant";
    case IntrinsicType::kDistance:
      return "distance";
    case IntrinsicType::kDot:
      return "dot";
    case IntrinsicType::kDpdx:
      return "dpdx";
    case IntrinsicType::kDpdxCoarse:
      return "dpdxCoarse";
    case IntrinsicType::kDpdxFine:
      return "dpdxFine";
    case IntrinsicType::kDpdy:
      return "dpdy";
    case IntrinsicType::kDpdyCoarse:
      return "dpdyCoarse";
    case IntrinsicType::kDpdyFine:
      return "dpdyFine";
    case IntrinsicType::kExp:
      return "exp";
    case IntrinsicType::kExp2:
      return "exp2";
    case IntrinsicType::kFaceForward:
      return "faceForward";
    case IntrinsicType::kFloor:
      return "floor";
    case IntrinsicType::kFma:
      return "fma";
    case IntrinsicType::kFract:
      return "fract";
    case IntrinsicType::kFrexp:
      return "frexp";
    case IntrinsicType::kFwidth:
      return "fwidth";
    case IntrinsicType::kFwidthCoarse:
      return "fwidthCoarse";
    case IntrinsicType::kFwidthFine:
      return "fwidthFine";
    case IntrinsicType::kInverseSqrt:
      return "inverseSqrt";
    case IntrinsicType::kIsFinite:
      return "isFinite";
    case IntrinsicType::kIsInf:
      return "isInf";
    case IntrinsicType::kIsNan:
      return "isNan";
    case IntrinsicType::kIsNormal:
      return "isNormal";
    case IntrinsicType::kLdexp:
      return "ldexp";
    case IntrinsicType::kLength:
      return "length";
    case IntrinsicType::kLog:
      return "log";
    case IntrinsicType::kLog2:
      return "log2";
    case IntrinsicType::kMax:
      return "max";
    case IntrinsicType::kMin:
      return "min";
    case IntrinsicType::kMix:
      return "mix";
    case IntrinsicType::kModf:
      return "modf";
    case IntrinsicType::kNormalize:
      return "normalize";
    case IntrinsicType::kPack4x8Snorm:
      return "pack4x8snorm";
    case IntrinsicType::kPack4x8Unorm:
      return "pack4x8unorm";
    case IntrinsicType::kPack2x16Snorm:
      return "pack2x16snorm";
    case IntrinsicType::kPack2x16Unorm:
      return "pack2x16unorm";
    case IntrinsicType::kPack2x16Float:
      return "pack2x16float";
    case IntrinsicType::kPow:
      return "pow";
    case IntrinsicType::kReflect:
      return "reflect";
    case IntrinsicType::kReverseBits:
      return "reverseBits";
    case IntrinsicType::kRound:
      return "round";
    case IntrinsicType::kSelect:
      return "select";
    case IntrinsicType::kSign:
      return "sign";
    case IntrinsicType::kSin:
      return "sin";
    case IntrinsicType::kSinh:
      return "sinh";
    case IntrinsicType::kSmoothStep:
      return "smoothStep";
    case IntrinsicType::kSqrt:
      return "sqrt";
    case IntrinsicType::kStep:
      return "step";
    case IntrinsicType::kTan:
      return "tan";
    case IntrinsicType::kTanh:
      return "tanh";
    case IntrinsicType::kTextureDimensions:
      return "textureDimensions";
    case IntrinsicType::kTextureLoad:
      return "textureLoad";
    case IntrinsicType::kTextureNumLayers:
      return "textureNumLayers";
    case IntrinsicType::kTextureNumLevels:
      return "textureNumLevels";
    case IntrinsicType::kTextureNumSamples:
      return "textureNumSamples";
    case IntrinsicType::kTextureSample:
      return "textureSample";
    case IntrinsicType::kTextureSampleBias:
      return "textureSampleBias";
    case IntrinsicType::kTextureSampleCompare:
      return "textureSampleCompare";
    case IntrinsicType::kTextureSampleGrad:
      return "textureSampleGrad";
    case IntrinsicType::kTextureSampleLevel:
      return "textureSampleLevel";
    case IntrinsicType::kTextureStore:
      return "textureStore";
    case IntrinsicType::kTrunc:
      return "trunc";
    case IntrinsicType::kUnpack4x8Snorm:
      return "unpack4x8snorm";
    case IntrinsicType::kUnpack4x8Unorm:
      return "unpack4x8unorm";
    case IntrinsicType::kUnpack2x16Snorm:
      return "unpack2x16snorm";
    case IntrinsicType::kUnpack2x16Unorm:
      return "unpack2x16unorm";
    case IntrinsicType::kUnpack2x16Float:
      return "unpack2x16float";
  }
  return "<unknown>";
}

bool IsCoarseDerivativeIntrinsic(IntrinsicType i) {
  return i == IntrinsicType::kDpdxCoarse || i == IntrinsicType::kDpdyCoarse ||
         i == IntrinsicType::kFwidthCoarse;
}

bool IsFineDerivativeIntrinsic(IntrinsicType i) {
  return i == IntrinsicType::kDpdxFine || i == IntrinsicType::kDpdyFine ||
         i == IntrinsicType::kFwidthFine;
}

bool IsDerivativeIntrinsic(IntrinsicType i) {
  return i == IntrinsicType::kDpdx || i == IntrinsicType::kDpdy ||
         i == IntrinsicType::kFwidth || IsCoarseDerivativeIntrinsic(i) ||
         IsFineDerivativeIntrinsic(i);
}

bool IsFloatClassificationIntrinsic(IntrinsicType i) {
  return i == IntrinsicType::kIsFinite || i == IntrinsicType::kIsInf ||
         i == IntrinsicType::kIsNan || i == IntrinsicType::kIsNormal;
}

bool IsTextureIntrinsic(IntrinsicType i) {
  return IsImageQueryIntrinsic(i) || i == IntrinsicType::kTextureLoad ||
         i == IntrinsicType::kTextureSample ||
         i == IntrinsicType::kTextureSampleLevel ||
         i == IntrinsicType::kTextureSampleBias ||
         i == IntrinsicType::kTextureSampleCompare ||
         i == IntrinsicType::kTextureSampleGrad ||
         i == IntrinsicType::kTextureStore;
}

bool IsImageQueryIntrinsic(IntrinsicType i) {
  return i == IntrinsicType::kTextureDimensions ||
         i == IntrinsicType::kTextureNumLayers ||
         i == IntrinsicType::kTextureNumLevels ||
         i == IntrinsicType::kTextureNumSamples;
}

bool IsDataPackingIntrinsic(IntrinsicType i) {
  return i == IntrinsicType::kPack4x8Snorm ||
         i == IntrinsicType::kPack4x8Unorm ||
         i == IntrinsicType::kPack2x16Snorm ||
         i == IntrinsicType::kPack2x16Unorm ||
         i == IntrinsicType::kPack2x16Float;
}

bool IsDataUnpackingIntrinsic(IntrinsicType i) {
  return i == IntrinsicType::kUnpack4x8Snorm ||
         i == IntrinsicType::kUnpack4x8Unorm ||
         i == IntrinsicType::kUnpack2x16Snorm ||
         i == IntrinsicType::kUnpack2x16Unorm ||
         i == IntrinsicType::kUnpack2x16Float;
}

Intrinsic::Intrinsic(IntrinsicType type,
                     type::Type* return_type,
                     const ParameterList& parameters)
    : Base(return_type, parameters), type_(type) {}

Intrinsic::~Intrinsic() = default;

bool Intrinsic::IsCoarseDerivative() const {
  return IsCoarseDerivativeIntrinsic(type_);
}

bool Intrinsic::IsFineDerivative() const {
  return IsFineDerivativeIntrinsic(type_);
}

bool Intrinsic::IsDerivative() const {
  return IsDerivativeIntrinsic(type_);
}

bool Intrinsic::IsFloatClassification() const {
  return IsFloatClassificationIntrinsic(type_);
}

bool Intrinsic::IsTexture() const {
  return IsTextureIntrinsic(type_);
}

bool Intrinsic::IsImageQuery() const {
  return IsImageQueryIntrinsic(type_);
}

bool Intrinsic::IsDataPacking() const {
  return IsDataPackingIntrinsic(type_);
}

bool Intrinsic::IsDataUnpacking() const {
  return IsDataUnpackingIntrinsic(type_);
}

}  // namespace semantic
}  // namespace tint
