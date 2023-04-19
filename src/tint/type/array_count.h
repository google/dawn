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

#ifndef SRC_TINT_TYPE_ARRAY_COUNT_H_
#define SRC_TINT_TYPE_ARRAY_COUNT_H_

#include <functional>
#include <string>

#include "src/tint/symbol_table.h"
#include "src/tint/type/clone_context.h"
#include "src/tint/type/unique_node.h"

namespace tint::type {

/// An array count
class ArrayCount : public utils::Castable<ArrayCount, UniqueNode> {
  public:
    ~ArrayCount() override;

    /// @returns the friendly name for this array count
    virtual std::string FriendlyName() const = 0;

    /// @param ctx the clone context
    /// @returns a clone of this type
    virtual ArrayCount* Clone(CloneContext& ctx) const = 0;

  protected:
    /// Constructor
    /// @param hash the unique hash of the node
    explicit ArrayCount(size_t hash);
};

/// The variant of an ArrayCount when the array is a const-expression.
/// Example:
/// ```
/// const N = 123;
/// type arr = array<i32, N>
/// ```
class ConstantArrayCount final : public utils::Castable<ConstantArrayCount, ArrayCount> {
  public:
    /// Constructor
    /// @param val the constant-expression value
    explicit ConstantArrayCount(uint32_t val);
    ~ConstantArrayCount() override;

    /// @param other the other object
    /// @returns true if this array count is equal to other
    bool Equals(const UniqueNode& other) const override;

    /// @returns the friendly name for this array count
    std::string FriendlyName() const override;

    /// @param ctx the clone context
    /// @returns a clone of this type
    ConstantArrayCount* Clone(CloneContext& ctx) const override;

    /// The array count constant-expression value.
    uint32_t value;
};

/// The variant of an ArrayCount when the array is is runtime-sized.
/// Example:
/// ```
/// type arr = array<i32>
/// ```
class RuntimeArrayCount final : public utils::Castable<RuntimeArrayCount, ArrayCount> {
  public:
    /// Constructor
    RuntimeArrayCount();
    ~RuntimeArrayCount() override;

    /// @param other the other object
    /// @returns true if this array count is equal to other
    bool Equals(const UniqueNode& other) const override;

    /// @returns the friendly name for this array count
    std::string FriendlyName() const override;

    /// @param ctx the clone context
    /// @returns a clone of this type
    RuntimeArrayCount* Clone(CloneContext& ctx) const override;
};

}  // namespace tint::type

#endif  // SRC_TINT_TYPE_ARRAY_COUNT_H_
