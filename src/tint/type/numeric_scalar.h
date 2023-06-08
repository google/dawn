// Copyright 2023 The Tint Authors.
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

#ifndef SRC_TINT_TYPE_NUMERIC_SCALAR_H_
#define SRC_TINT_TYPE_NUMERIC_SCALAR_H_

#include "src/tint/type/scalar.h"

namespace tint::type {

/// Base class for all numeric-scalar types
/// @see https://www.w3.org/TR/WGSL/#scalar-types
class NumericScalar : public utils::Castable<NumericScalar, Scalar> {
  public:
    /// Destructor
    ~NumericScalar() override;

  protected:
    /// Constructor
    /// @param hash the immutable hash for the node
    /// @param flags the flags of this type
    NumericScalar(size_t hash, type::Flags flags);
};

}  // namespace tint::type

#endif  // SRC_TINT_TYPE_NUMERIC_SCALAR_H_
