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

#ifndef INCLUDE_TINT_BINDING_POINT_H_
#define INCLUDE_TINT_BINDING_POINT_H_

#include <stdint.h>

#include <functional>

#include "src/tint/utils/math/hash.h"
#include "src/tint/utils/reflection/reflection.h"
#include "src/tint/utils/text/string_stream.h"

namespace tint {

/// BindingPoint holds a group and binding index.
struct BindingPoint {
    /// The `@group` part of the binding point
    uint32_t group = 0;
    /// The `@binding` part of the binding point
    uint32_t binding = 0;

    /// Reflect the fields of this class so that it can be used by tint::ForeachField()
    TINT_REFLECT(group, binding);

    /// Equality operator
    /// @param rhs the BindingPoint to compare against
    /// @returns true if this BindingPoint is equal to `rhs`
    inline bool operator==(const BindingPoint& rhs) const {
        return group == rhs.group && binding == rhs.binding;
    }

    /// Inequality operator
    /// @param rhs the BindingPoint to compare against
    /// @returns true if this BindingPoint is not equal to `rhs`
    inline bool operator!=(const BindingPoint& rhs) const { return !(*this == rhs); }

    /// Less-than operator
    /// @param rhs the BindingPoint to compare against
    /// @returns true if this BindingPoint comes before @p rhs
    inline bool operator<(const BindingPoint& rhs) const {
        if (group < rhs.group) {
            return true;
        }
        if (group > rhs.group) {
            return false;
        }
        return binding < rhs.binding;
    }
};

/// Prints the BindingPoint @p bp to @p o
/// @param o the stream to write to
/// @param bp the BindingPoint
/// @return the stream so calls can be chained
template <typename STREAM, typename = traits::EnableIfIsOStream<STREAM>>
auto& operator<<(STREAM& o, const BindingPoint& bp) {
    return o << "[group: " << bp.group << ", binding: " << bp.binding << "]";
}

}  // namespace tint

namespace std {

/// Custom std::hash specialization for tint::BindingPoint so BindingPoints can be used as keys for
/// std::unordered_map and std::unordered_set.
template <>
class hash<tint::BindingPoint> {
  public:
    /// @param binding_point the binding point to create a hash for
    /// @return the hash value
    inline std::size_t operator()(const tint::BindingPoint& binding_point) const {
        return tint::Hash(binding_point.group, binding_point.binding);
    }
};

}  // namespace std

#endif  // INCLUDE_TINT_BINDING_POINT_H_
