// Copyright 2022 The Tint Authors.
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

#ifndef SRC_TINT_TYPE_BOOL_H_
#define SRC_TINT_TYPE_BOOL_H_

#include <string>

#include "src/tint/type/type.h"

// X11 likes to #define Bool leading to confusing error messages.
// If its defined, undefine it.
#ifdef Bool
#undef Bool
#endif

namespace tint::type {

/// A boolean type
class Bool final : public utils::Castable<Bool, Type> {
  public:
    /// Constructor
    Bool();

    /// Destructor
    ~Bool() override;

    /// @param other the other node to compare against
    /// @returns true if the this type is equal to @p other
    bool Equals(const UniqueNode& other) const override;

    /// @returns the name for this type that closely resembles how it would be
    /// declared in WGSL.
    std::string FriendlyName() const override;

    /// @returns the size in bytes of the type.
    /// @note: booleans are not host-sharable, but still may exist in workgroup
    /// storage.
    uint32_t Size() const override;

    /// @returns the alignment in bytes of the type.
    /// @note: booleans are not host-sharable, but still may exist in workgroup
    /// storage.
    uint32_t Align() const override;

    /// @param ctx the clone context
    /// @returns a clone of this type
    Bool* Clone(CloneContext& ctx) const override;
};

}  // namespace tint::type

#endif  // SRC_TINT_TYPE_BOOL_H_
