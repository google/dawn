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
  kTextureSampleGrad,
  kTextureSampleLevel,
  kTrunc
};

/// Emits the name of the intrinsic function. The spelling,
/// including case, matches the name in the WGSL spec.
std::ostream& operator<<(std::ostream& out, Intrinsic i);

namespace intrinsic {

/// Signature is the base struct for all intrinsic signature types.
/// Signatures are used to identify the particular overload for intrinsics that
/// have different signatures with the same function name.
struct Signature {
  virtual ~Signature();
};

/// TextureSignature describes the signature of a texture intrinsic function.
struct TextureSignature : public Signature {
  /// Parameters describes the parameters for the texture function.
  struct Parameters {
    /// kNotUsed is the constant that indicates the given parameter is not part
    /// of the texture function signature.
    static constexpr const size_t kNotUsed = ~static_cast<size_t>(0u);
    /// Index holds each of the possible parameter indices. If a parameter index
    /// is equal to `kNotUsed` then this parameter is not used by the function.
    struct Index {
      /// Constructor
      Index();
      /// Copy constructor
      Index(const Index&);
      /// `array_index` parameter index.
      size_t array_index = kNotUsed;
      /// `bias` parameter index.
      size_t bias = kNotUsed;
      /// `coords` parameter index.
      size_t coords = kNotUsed;
      /// `depth_ref` parameter index.
      size_t depth_ref = kNotUsed;
      /// `ddx` parameter index.
      size_t ddx = kNotUsed;
      /// `ddy` parameter index.
      size_t ddy = kNotUsed;
      /// `level` parameter index.
      size_t level = kNotUsed;
      /// `offset` parameter index.
      size_t offset = kNotUsed;
      /// `sampler` parameter index.
      size_t sampler = kNotUsed;
      /// `texture` parameter index.
      size_t texture = kNotUsed;
    };
    /// The indices of all possible parameters.
    Index idx;
    /// Total number of parameters.
    size_t count = 0;
  };

  /// Construct an immutable `TextureSignature`.
  /// @param p the texture intrinsic parameter signature.
  explicit TextureSignature(const Parameters& p) : params(p) {}

  ~TextureSignature() override;

  /// The texture intrinsic parameter signature.
  const Parameters params;
};

/// Determines if the given `i` is a coarse derivative
/// @param i the intrinsic
/// @returns true if the given derivative is coarse.
bool IsCoarseDerivative(Intrinsic i);

/// Determines if the given `i` is a fine derivative
/// @param i the intrinsic
/// @returns true if the given derivative is fine.
bool IsFineDerivative(Intrinsic i);

/// Determine if the given `i` is a derivative intrinsic
/// @param i the intrinsic
/// @returns true if the given `i` is a derivative intrinsic
bool IsDerivative(Intrinsic i);

/// Determines if the given `i` is a float classification intrinsic
/// @param i the intrinsic
/// @returns true if the given `i` is a float intrinsic
bool IsFloatClassificationIntrinsic(Intrinsic i);

/// Determines if the given `i` is a texture operation intrinsic
/// @param i the intrinsic
/// @returns true if the given `i` is a texture operation intrinsic
bool IsTextureIntrinsic(Intrinsic i);

}  // namespace intrinsic
}  // namespace ast
}  // namespace tint

#endif  // SRC_AST_INTRINSIC_H_
