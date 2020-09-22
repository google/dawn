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

#ifndef SRC_AST_INTRINSIC_H_
#define SRC_AST_INTRINSIC_H_

#include <ostream>
#include <string>

namespace tint {
namespace ast {

enum class Intrinsic {
  kNone = -1,

  kAbs,
  kAcos,
  kAll,
  kAny,
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
  kOuterProduct,
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
  kTextureLoad,
  kTextureSample,
  kTextureSampleBias,
  kTextureSampleCompare,
  kTextureSampleLevel,
  kTrunc
};

std::ostream& operator<<(std::ostream& out, Intrinsic i);

namespace intrinsic {

/// Determines if the given |name| is a coarse derivative
/// @param i the intrinsic
/// @returns true if the given derivative is coarse.
bool IsCoarseDerivative(ast::Intrinsic i);

/// Determines if the given |name| is a fine derivative
/// @param i the intrinsic
/// @returns true if the given derivative is fine.
bool IsFineDerivative(ast::Intrinsic i);

/// Determine if the given |name| is a derivative intrinsic
/// @param i the intrinsic
/// @returns true if the given |name| is a derivative intrinsic
bool IsDerivative(ast::Intrinsic i);

/// Determines if the given |name| is a float classification intrinsic
/// @param i the intrinsic
/// @returns true if the given |name| is a float intrinsic
bool IsFloatClassificationIntrinsic(ast::Intrinsic i);

/// Determines if the given |name| is a texture operation intrinsic
/// @param i the intrinsic
/// @returns true if the given |name| is a texture operation intrinsic
bool IsTextureIntrinsic(ast::Intrinsic i);

}  // namespace intrinsic
}  // namespace ast
}  // namespace tint

#endif  // SRC_AST_INTRINSIC_H_
