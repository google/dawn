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

#ifndef SRC_SEMANTIC_INTRINSIC_H_
#define SRC_SEMANTIC_INTRINSIC_H_

#include <ostream>

namespace tint {
namespace semantic {

enum class IntrinsicType {
  kNone = -1,

  kAbs,
  kAcos,
  kAll,
  kAny,
  kArrayLength,
  kAsin,
  kAtan,
  kAtan2,
  kCeil,
  kClamp,
  kCos,
  kCosh,
  kCountOneBits,
  kCross,
  kDeterminant,
  kDistance,
  kDot,
  kDpdx,
  kDpdxCoarse,
  kDpdxFine,
  kDpdy,
  kDpdyCoarse,
  kDpdyFine,
  kExp,
  kExp2,
  kFaceForward,
  kFloor,
  kFma,
  kFract,
  kFrexp,
  kFwidth,
  kFwidthCoarse,
  kFwidthFine,
  kInverseSqrt,
  kIsFinite,
  kIsInf,
  kIsNan,
  kIsNormal,
  kLdexp,
  kLength,
  kLog,
  kLog2,
  kMax,
  kMin,
  kMix,
  kModf,
  kNormalize,
  kPack4x8Snorm,
  kPack4x8Unorm,
  kPack2x16Snorm,
  kPack2x16Unorm,
  kPack2x16Float,
  kPow,
  kReflect,
  kReverseBits,
  kRound,
  kSelect,
  kSign,
  kSin,
  kSinh,
  kSmoothStep,
  kSqrt,
  kStep,
  kTan,
  kTanh,
  kTextureDimensions,
  kTextureLoad,
  kTextureNumLayers,
  kTextureNumLevels,
  kTextureNumSamples,
  kTextureSample,
  kTextureSampleBias,
  kTextureSampleCompare,
  kTextureSampleGrad,
  kTextureSampleLevel,
  kTextureStore,
  kTrunc
};

/// Emits the name of the intrinsic function. The spelling,
/// including case, matches the name in the WGSL spec.
std::ostream& operator<<(std::ostream& out, IntrinsicType i);

namespace intrinsic {

/// Determines if the given `i` is a coarse derivative
/// @param i the intrinsic
/// @returns true if the given derivative is coarse.
bool IsCoarseDerivative(IntrinsicType i);

/// Determines if the given `i` is a fine derivative
/// @param i the intrinsic
/// @returns true if the given derivative is fine.
bool IsFineDerivative(IntrinsicType i);

/// Determine if the given `i` is a derivative intrinsic
/// @param i the intrinsic
/// @returns true if the given `i` is a derivative intrinsic
bool IsDerivative(IntrinsicType i);

/// Determines if the given `i` is a float classification intrinsic
/// @param i the intrinsic
/// @returns true if the given `i` is a float intrinsic
bool IsFloatClassificationIntrinsic(IntrinsicType i);

/// Determines if the given `i` is a texture operation intrinsic
/// @param i the intrinsic
/// @returns true if the given `i` is a texture operation intrinsic
bool IsTextureIntrinsic(IntrinsicType i);

/// Determines if the given `i` is a image query intrinsic
/// @param i the intrinsic
/// @returns true if the given `i` is a image query intrinsic
bool IsImageQueryIntrinsic(IntrinsicType i);

/// Determines if the given `i` is a data packing intrinsic
/// @param i the intrinsic
/// @returns true if the given `i` is a data packing intrinsic
bool IsDataPackingIntrinsic(IntrinsicType i);

/// @returns the name of the intrinsic function. The spelling, including case,
/// matches the name in the WGSL spec.
const char* str(IntrinsicType i);

}  // namespace intrinsic
}  // namespace semantic
}  // namespace tint

#endif  // SRC_SEMANTIC_INTRINSIC_H_
