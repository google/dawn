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

#ifndef SRC_TINT_TYPE_ABSTRACT_FLOAT_H_
#define SRC_TINT_TYPE_ABSTRACT_FLOAT_H_

#include <string>

#include "src/tint/type/abstract_numeric.h"

namespace tint::type {

/// An abstract-float type.
/// @see https://www.w3.org/TR/WGSL/#abstractFloat
class AbstractFloat final : public utils::Castable<AbstractFloat, AbstractNumeric> {
  public:
    /// Constructor
    AbstractFloat();

    /// Destructor
    ~AbstractFloat() override;

    /// @param other the other type to compare against
    /// @returns true if this type is equal to the given type
    bool Equals(const UniqueNode& other) const override;

    /// @returns the name for this type when printed in diagnostics.
    std::string FriendlyName() const override;

    /// @param ctx the clone context
    /// @returns a clone of this type
    AbstractFloat* Clone(CloneContext& ctx) const override;
};

}  // namespace tint::type

#endif  // SRC_TINT_TYPE_ABSTRACT_FLOAT_H_
