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

#ifndef SRC_SEM_INTRINSIC_H_
#define SRC_SEM_INTRINSIC_H_

#include <string>

#include "src/sem/call_target.h"

namespace tint {
namespace sem {

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
  kPack2x16Float,
  kPack2x16Snorm,
  kPack2x16Unorm,
  kPack4x8Snorm,
  kPack4x8Unorm,
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
  kStorageBarrier,
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
  kTrunc,
  kUnpack2x16Float,
  kUnpack2x16Snorm,
  kUnpack2x16Unorm,
  kUnpack4x8Snorm,
  kUnpack4x8Unorm,
  kWorkgroupBarrier,
};

/// Matches the IntrisicType by name
/// @param name the intrinsic name to parse
/// @returns the parsed IntrinsicType, or IntrinsicType::kNone if `name` did not
/// match any intrinsic.
IntrinsicType ParseIntrinsicType(const std::string& name);

/// @returns the name of the intrinsic function type. The spelling, including
/// case, matches the name in the WGSL spec.
const char* str(IntrinsicType i);

/// Determines if the given `i` is a coarse derivative
/// @param i the intrinsic type
/// @returns true if the given derivative is coarse.
bool IsCoarseDerivativeIntrinsic(IntrinsicType i);

/// Determines if the given `i` is a fine derivative
/// @param i the intrinsic type
/// @returns true if the given derivative is fine.
bool IsFineDerivativeIntrinsic(IntrinsicType i);

/// Determine if the given `i` is a derivative intrinsic
/// @param i the intrinsic type
/// @returns true if the given `i` is a derivative intrinsic
bool IsDerivativeIntrinsic(IntrinsicType i);

/// Determines if the given `i` is a float classification intrinsic
/// @param i the intrinsic type
/// @returns true if the given `i` is a float intrinsic
bool IsFloatClassificationIntrinsic(IntrinsicType i);

/// Determines if the given `i` is a texture operation intrinsic
/// @param i the intrinsic type
/// @returns true if the given `i` is a texture operation intrinsic
bool IsTextureIntrinsic(IntrinsicType i);

/// Determines if the given `i` is a image query intrinsic
/// @param i the intrinsic type
/// @returns true if the given `i` is a image query intrinsic
bool IsImageQueryIntrinsic(IntrinsicType i);

/// Determines if the given `i` is a data packing intrinsic
/// @param i the intrinsic
/// @returns true if the given `i` is a data packing intrinsic
bool IsDataPackingIntrinsic(IntrinsicType i);

/// Determines if the given `i` is a data unpacking intrinsic
/// @param i the intrinsic
/// @returns true if the given `i` is a data unpacking intrinsic
bool IsDataUnpackingIntrinsic(IntrinsicType i);

/// Determines if the given `i` is a barrier intrinsic
/// @param i the intrinsic
/// @returns true if the given `i` is a barrier intrinsic
bool IsBarrierIntrinsic(IntrinsicType i);

/// Intrinsic holds the semantic information for an intrinsic function.
class Intrinsic : public Castable<Intrinsic, CallTarget> {
 public:
  /// Constructor
  /// @param type the intrinsic type
  /// @param return_type the return type for the intrinsic call
  /// @param parameters the parameters for the intrinsic overload
  Intrinsic(IntrinsicType type,
            sem::Type* return_type,
            const ParameterList& parameters);

  /// Destructor
  ~Intrinsic() override;

  /// @return the type of the intrinsic
  IntrinsicType Type() const { return type_; }

  /// @returns the name of the intrinsic function type. The spelling, including
  /// case, matches the name in the WGSL spec.
  const char* str() const;

  /// @returns true if intrinsic is a coarse derivative intrinsic
  bool IsCoarseDerivative() const;

  /// @returns true if intrinsic is a fine a derivative intrinsic
  bool IsFineDerivative() const;

  /// @returns true if intrinsic is a derivative intrinsic
  bool IsDerivative() const;

  /// @returns true if intrinsic is a float intrinsic
  bool IsFloatClassification() const;

  /// @returns true if intrinsic is a texture operation intrinsic
  bool IsTexture() const;

  /// @returns true if intrinsic is a image query intrinsic
  bool IsImageQuery() const;

  /// @returns true if intrinsic is a data packing intrinsic
  bool IsDataPacking() const;

  /// @returns true if intrinsic is a data unpacking intrinsic
  bool IsDataUnpacking() const;

  /// @returns true if intrinsic is a barrier intrinsic
  bool IsBarrier() const;

 private:
  IntrinsicType const type_;
};

/// Emits the name of the intrinsic function type. The spelling, including case,
/// matches the name in the WGSL spec.
std::ostream& operator<<(std::ostream& out, IntrinsicType i);

}  // namespace sem
}  // namespace tint

#endif  // SRC_SEM_INTRINSIC_H_
