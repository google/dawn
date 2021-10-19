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
#include <vector>

#include "src/sem/call_target.h"
#include "src/sem/intrinsic_type.h"
#include "src/sem/pipeline_stage_set.h"
#include "src/utils/hash.h"

namespace tint {
namespace sem {

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

/// Determines if the given `i` is a atomic intrinsic
/// @param i the intrinsic
/// @returns true if the given `i` is a atomic intrinsic
bool IsAtomicIntrinsic(IntrinsicType i);

/// Intrinsic holds the semantic information for an intrinsic function.
class Intrinsic : public Castable<Intrinsic, CallTarget> {
 public:
  /// Constructor
  /// @param type the intrinsic type
  /// @param return_type the return type for the intrinsic call
  /// @param parameters the parameters for the intrinsic overload
  /// @param supported_stages the pipeline stages that this intrinsic can be
  /// used in
  /// @param is_deprecated true if the particular overload is considered
  /// deprecated
  Intrinsic(IntrinsicType type,
            const sem::Type* return_type,
            std::vector<Parameter*> parameters,
            PipelineStageSet supported_stages,
            bool is_deprecated);

  /// Destructor
  ~Intrinsic() override;

  /// @return the type of the intrinsic
  IntrinsicType Type() const { return type_; }

  /// @return the pipeline stages that this intrinsic can be used in
  PipelineStageSet SupportedStages() const { return supported_stages_; }

  /// @return true if the intrinsic overload is considered deprecated
  bool IsDeprecated() const { return is_deprecated_; }

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

  /// @returns true if intrinsic is a atomic intrinsic
  bool IsAtomic() const;

 private:
  const IntrinsicType type_;
  const PipelineStageSet supported_stages_;
  const bool is_deprecated_;
};

}  // namespace sem
}  // namespace tint

namespace std {

/// Custom std::hash specialization for tint::sem::Intrinsic
template <>
class hash<tint::sem::Intrinsic> {
 public:
  /// @param i the Intrinsic to create a hash for
  /// @return the hash value
  inline std::size_t operator()(const tint::sem::Intrinsic& i) const {
    return tint::utils::Hash(i.Type(), i.SupportedStages(), i.ReturnType(),
                             i.Parameters(), i.IsDeprecated());
  }
};

}  // namespace std

#endif  // SRC_SEM_INTRINSIC_H_
