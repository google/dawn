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

#ifndef SRC_TINT_CONSTANT_SPLAT_H_
#define SRC_TINT_CONSTANT_SPLAT_H_

#include "src/tint/constant/composite.h"
#include "src/tint/type/type.h"
#include "src/tint/utils/castable.h"
#include "src/tint/utils/vector.h"

namespace tint::constant {

/// Splat holds a single value, duplicated as all children.
///
/// Splat is used for zero-initializers, 'splat' initializers, or initializers where each element is
/// identical. Splat may be of a vector, matrix, array or structure type.
class Splat : public utils::Castable<Splat, Value> {
  public:
    /// Constructor
    /// @param t the splat type
    /// @param e the splat element
    /// @param n the number of items in the splat
    Splat(const type::Type* t, const Value* e, size_t n);
    ~Splat() override;

    /// @returns the type of the splat
    const type::Type* Type() const override { return type; }

    /// Retrieve item at index @p i
    /// @param i the index to retrieve
    /// @returns the element, or nullptr if out of bounds
    const Value* Index(size_t i) const override { return i < count ? el : nullptr; }

    /// @copydoc Value::NumElements()
    size_t NumElements() const override { return count; }

    /// @returns true if the element is zero
    bool AllZero() const override { return el->AllZero(); }
    /// @returns true if the element is zero
    bool AnyZero() const override { return el->AnyZero(); }

    /// @returns the hash for the splat
    size_t Hash() const override { return utils::Hash(type, el->Hash(), count); }

    /// Clones the constant into the provided context
    /// @param ctx the clone context
    /// @returns the cloned node
    Splat* Clone(CloneContext& ctx) const override;

    /// The type of the splat element
    type::Type const* const type;
    /// The element stored in the splat
    const Value* el;
    /// The number of items in the splat
    const size_t count;

  protected:
    /// @returns a monostate variant.
    std::variant<std::monostate, AInt, AFloat> InternalValue() const override { return {}; }
};

}  // namespace tint::constant

#endif  // SRC_TINT_CONSTANT_SPLAT_H_
