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

#include "src/sem/intrinsic.h"

TINT_INSTANTIATE_TYPEINFO(tint::sem::Intrinsic);

namespace tint {
namespace sem {

std::ostream& operator<<(std::ostream& out, IntrinsicType i) {
  out << str(i);
  return out;
}

const char* Intrinsic::str() const {
  return sem::str(type_);
}

/// Name matches the spelling in the WGSL spec including case.
#define INTRINSIC_LIST()                                                  \
  INTRINSIC(IntrinsicType::kNone, "<not-an-intrinsic>")                   \
  INTRINSIC(IntrinsicType::kAbs, "abs")                                   \
  INTRINSIC(IntrinsicType::kAcos, "acos")                                 \
  INTRINSIC(IntrinsicType::kAll, "all")                                   \
  INTRINSIC(IntrinsicType::kAny, "any")                                   \
  INTRINSIC(IntrinsicType::kArrayLength, "arrayLength")                   \
  INTRINSIC(IntrinsicType::kAsin, "asin")                                 \
  INTRINSIC(IntrinsicType::kAtan, "atan")                                 \
  INTRINSIC(IntrinsicType::kAtan2, "atan2")                               \
  INTRINSIC(IntrinsicType::kCeil, "ceil")                                 \
  INTRINSIC(IntrinsicType::kClamp, "clamp")                               \
  INTRINSIC(IntrinsicType::kCos, "cos")                                   \
  INTRINSIC(IntrinsicType::kCosh, "cosh")                                 \
  INTRINSIC(IntrinsicType::kCountOneBits, "countOneBits")                 \
  INTRINSIC(IntrinsicType::kCross, "cross")                               \
  INTRINSIC(IntrinsicType::kDeterminant, "determinant")                   \
  INTRINSIC(IntrinsicType::kDistance, "distance")                         \
  INTRINSIC(IntrinsicType::kDot, "dot")                                   \
  INTRINSIC(IntrinsicType::kDpdx, "dpdx")                                 \
  INTRINSIC(IntrinsicType::kDpdxCoarse, "dpdxCoarse")                     \
  INTRINSIC(IntrinsicType::kDpdxFine, "dpdxFine")                         \
  INTRINSIC(IntrinsicType::kDpdy, "dpdy")                                 \
  INTRINSIC(IntrinsicType::kDpdyCoarse, "dpdyCoarse")                     \
  INTRINSIC(IntrinsicType::kDpdyFine, "dpdyFine")                         \
  INTRINSIC(IntrinsicType::kExp, "exp")                                   \
  INTRINSIC(IntrinsicType::kExp2, "exp2")                                 \
  INTRINSIC(IntrinsicType::kFaceForward, "faceForward")                   \
  INTRINSIC(IntrinsicType::kFloor, "floor")                               \
  INTRINSIC(IntrinsicType::kFma, "fma")                                   \
  INTRINSIC(IntrinsicType::kFract, "fract")                               \
  INTRINSIC(IntrinsicType::kFrexp, "frexp")                               \
  INTRINSIC(IntrinsicType::kFwidth, "fwidth")                             \
  INTRINSIC(IntrinsicType::kFwidthCoarse, "fwidthCoarse")                 \
  INTRINSIC(IntrinsicType::kFwidthFine, "fwidthFine")                     \
  INTRINSIC(IntrinsicType::kInverseSqrt, "inverseSqrt")                   \
  INTRINSIC(IntrinsicType::kIsFinite, "isFinite")                         \
  INTRINSIC(IntrinsicType::kIsInf, "isInf")                               \
  INTRINSIC(IntrinsicType::kIsNan, "isNan")                               \
  INTRINSIC(IntrinsicType::kIsNormal, "isNormal")                         \
  INTRINSIC(IntrinsicType::kLdexp, "ldexp")                               \
  INTRINSIC(IntrinsicType::kLength, "length")                             \
  INTRINSIC(IntrinsicType::kLog, "log")                                   \
  INTRINSIC(IntrinsicType::kLog2, "log2")                                 \
  INTRINSIC(IntrinsicType::kMax, "max")                                   \
  INTRINSIC(IntrinsicType::kMin, "min")                                   \
  INTRINSIC(IntrinsicType::kMix, "mix")                                   \
  INTRINSIC(IntrinsicType::kModf, "modf")                                 \
  INTRINSIC(IntrinsicType::kNormalize, "normalize")                       \
  INTRINSIC(IntrinsicType::kPack4x8Snorm, "pack4x8snorm")                 \
  INTRINSIC(IntrinsicType::kPack4x8Unorm, "pack4x8unorm")                 \
  INTRINSIC(IntrinsicType::kPack2x16Snorm, "pack2x16snorm")               \
  INTRINSIC(IntrinsicType::kPack2x16Unorm, "pack2x16unorm")               \
  INTRINSIC(IntrinsicType::kPack2x16Float, "pack2x16float")               \
  INTRINSIC(IntrinsicType::kPow, "pow")                                   \
  INTRINSIC(IntrinsicType::kReflect, "reflect")                           \
  INTRINSIC(IntrinsicType::kReverseBits, "reverseBits")                   \
  INTRINSIC(IntrinsicType::kRound, "round")                               \
  INTRINSIC(IntrinsicType::kSelect, "select")                             \
  INTRINSIC(IntrinsicType::kSign, "sign")                                 \
  INTRINSIC(IntrinsicType::kSin, "sin")                                   \
  INTRINSIC(IntrinsicType::kSinh, "sinh")                                 \
  INTRINSIC(IntrinsicType::kSmoothStep, "smoothStep")                     \
  INTRINSIC(IntrinsicType::kSqrt, "sqrt")                                 \
  INTRINSIC(IntrinsicType::kStep, "step")                                 \
  INTRINSIC(IntrinsicType::kStorageBarrier, "storageBarrier")             \
  INTRINSIC(IntrinsicType::kTan, "tan")                                   \
  INTRINSIC(IntrinsicType::kTanh, "tanh")                                 \
  INTRINSIC(IntrinsicType::kTextureDimensions, "textureDimensions")       \
  INTRINSIC(IntrinsicType::kTextureLoad, "textureLoad")                   \
  INTRINSIC(IntrinsicType::kTextureNumLayers, "textureNumLayers")         \
  INTRINSIC(IntrinsicType::kTextureNumLevels, "textureNumLevels")         \
  INTRINSIC(IntrinsicType::kTextureNumSamples, "textureNumSamples")       \
  INTRINSIC(IntrinsicType::kTextureSample, "textureSample")               \
  INTRINSIC(IntrinsicType::kTextureSampleBias, "textureSampleBias")       \
  INTRINSIC(IntrinsicType::kTextureSampleCompare, "textureSampleCompare") \
  INTRINSIC(IntrinsicType::kTextureSampleGrad, "textureSampleGrad")       \
  INTRINSIC(IntrinsicType::kTextureSampleLevel, "textureSampleLevel")     \
  INTRINSIC(IntrinsicType::kTextureStore, "textureStore")                 \
  INTRINSIC(IntrinsicType::kTrunc, "trunc")                               \
  INTRINSIC(IntrinsicType::kUnpack2x16Float, "unpack2x16float")           \
  INTRINSIC(IntrinsicType::kUnpack2x16Snorm, "unpack2x16snorm")           \
  INTRINSIC(IntrinsicType::kUnpack2x16Unorm, "unpack2x16unorm")           \
  INTRINSIC(IntrinsicType::kUnpack4x8Snorm, "unpack4x8snorm")             \
  INTRINSIC(IntrinsicType::kUnpack4x8Unorm, "unpack4x8unorm")             \
  INTRINSIC(IntrinsicType::kWorkgroupBarrier, "workgroupBarrier")

IntrinsicType ParseIntrinsicType(const std::string& name) {
#define INTRINSIC(ENUM, NAME) \
  if (name == NAME) {         \
    return ENUM;              \
  }
  INTRINSIC_LIST()
#undef INTRINSIC
  return IntrinsicType::kNone;
}

const char* str(IntrinsicType i) {
#define INTRINSIC(ENUM, NAME) \
  case ENUM:                  \
    return NAME;
  switch (i) { INTRINSIC_LIST() }
#undef INTRINSIC
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

bool IsBarrierIntrinsic(IntrinsicType i) {
  return i == IntrinsicType::kWorkgroupBarrier ||
         i == IntrinsicType::kStorageBarrier;
}

Intrinsic::Intrinsic(IntrinsicType type,
                     sem::Type* return_type,
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

bool Intrinsic::IsBarrier() const {
  return IsBarrierIntrinsic(type_);
}

}  // namespace sem
}  // namespace tint
