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

#ifndef SRC_TINT_SEM_SAMPLER_TEXTURE_PAIR_H_
#define SRC_TINT_SEM_SAMPLER_TEXTURE_PAIR_H_

#include <cstdint>
#include <functional>

#include "src/tint/sem/binding_point.h"
#include "src/tint/utils/string_stream.h"

namespace tint::sem {

/// Mapping of a sampler to a texture it samples.
struct SamplerTexturePair {
    /// group & binding values for a sampler.
    BindingPoint sampler_binding_point;
    /// group & binding values for a texture samepled by the sampler.
    BindingPoint texture_binding_point;

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
    inline bool operator!=(const SamplerTexturePair& rhs) const { return !(*this == rhs); }
};

/// Prints the SamplerTexturePair @p stp to @p o
/// @param o the stream to write to
/// @param stp the SamplerTexturePair
/// @return the stream so calls can be chained
inline utils::StringStream& operator<<(utils::StringStream& o, const SamplerTexturePair& stp) {
    return o << "[sampler: " << stp.sampler_binding_point
             << ", texture: " << stp.sampler_binding_point << "]";
}

}  // namespace tint::sem

namespace std {

/// Custom std::hash specialization for tint::sem::SamplerTexturePair so
/// SamplerTexturePairs be used as keys for std::unordered_map and
/// std::unordered_set.
template <>
class hash<tint::sem::SamplerTexturePair> {
  public:
    /// @param stp the texture pair to create a hash for
    /// @return the hash value
    inline std::size_t operator()(const tint::sem::SamplerTexturePair& stp) const {
        return tint::utils::Hash(stp.sampler_binding_point, stp.texture_binding_point);
    }
};

}  // namespace std

#endif  // SRC_TINT_SEM_SAMPLER_TEXTURE_PAIR_H_
