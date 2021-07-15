// Copyright 2021 The Tint Authors.
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

#ifndef SRC_INSPECTOR_SAMPLER_TEXTURE_PAIR_H_
#define SRC_INSPECTOR_SAMPLER_TEXTURE_PAIR_H_

#include <cstdint>
#include <functional>

#include "src/sem/binding_point.h"

namespace tint {
namespace inspector {

/// Mapping of a sampler to a texture it samples.
struct SamplerTexturePair {
  /// group & binding values for a sampler.
  sem::BindingPoint sampler_binding_point;
  /// group & binding values for a texture samepled by the sampler.
  sem::BindingPoint texture_binding_point;

  /// Equality operator
  /// @param rhs the SamplerTexturePair to compare against
  /// @returns true if this SamplerTexturePair is equal to `rhs`
  inline bool operator==(const SamplerTexturePair& rhs) const {
    return sampler_binding_point == rhs.sampler_binding_point &&
           texture_binding_point == rhs.texture_binding_point;
  }

  /// Inequality operator
  /// @param rhs the SamplerTexturePair to compare against
  /// @returns true if this SamplerTexturePair is not equal to `rhs`
  inline bool operator!=(const SamplerTexturePair& rhs) const {
    return !(*this == rhs);
  }
};

}  // namespace inspector
}  // namespace tint

namespace std {

/// Custom std::hash specialization for tint::inspector::SamplerTexturePair so
/// SamplerTexturePairs be used as keys for std::unordered_map and
/// std::unordered_set.
template <>
class hash<tint::inspector::SamplerTexturePair> {
 public:
  /// @param stp the texture pair to create a hash for
  /// @return the hash value
  inline std::size_t operator()(
      const tint::inspector::SamplerTexturePair& stp) const {
    return tint::utils::Hash(stp.sampler_binding_point,
                             stp.texture_binding_point);
  }
};

}  // namespace std

#endif  // SRC_INSPECTOR_SAMPLER_TEXTURE_PAIR_H_
