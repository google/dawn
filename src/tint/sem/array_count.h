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

#ifndef SRC_TINT_SEM_ARRAY_COUNT_H_
#define SRC_TINT_SEM_ARRAY_COUNT_H_

#include <functional>
#include <string>

#include "src/tint/sem/expression.h"
#include "src/tint/sem/node.h"
#include "src/tint/sem/variable.h"

namespace tint::sem {

/// An array count
class ArrayCount : public Castable<ArrayCount, Node> {
  public:
    ~ArrayCount() override;

    /// @returns a hash of the array count.
    virtual size_t Hash() const = 0;

    /// @param t other array count
    /// @returns true if this array count is equal to the given array count
    virtual bool Equals(const ArrayCount& t) const = 0;

  protected:
    ArrayCount();
};

/// The variant of an ArrayCount when the array is a const-expression.
/// Example:
/// ```
/// const N = 123;
/// type arr = array<i32, N>
/// ```
class ConstantArrayCount final : public Castable<ConstantArrayCount, ArrayCount> {
  public:
    /// Constructor
    /// @param val the constant-expression value
    explicit ConstantArrayCount(uint32_t val);
    ~ConstantArrayCount() override;

    /// @returns a hash of the array count.
    size_t Hash() const override;

    /// @param t other array count
    /// @returns true if this array count is equal to the given array count
    bool Equals(const ArrayCount& t) const override;

    /// The array count constant-expression value.
    uint32_t value;
};

/// The variant of an ArrayCount when the array is is runtime-sized.
/// Example:
/// ```
/// type arr = array<i32>
/// ```
class RuntimeArrayCount final : public Castable<RuntimeArrayCount, ArrayCount> {
  public:
    /// Constructor
    RuntimeArrayCount();
    ~RuntimeArrayCount() override;

    /// @returns a hash of the array count.
    size_t Hash() const override;

    /// @param t other array count
    /// @returns true if this array count is equal to the given array count
    bool Equals(const ArrayCount& t) const override;
};

/// The variant of an ArrayCount when the count is a named override variable.
/// Example:
/// ```
/// override N : i32;
/// type arr = array<i32, N>
/// ```
class NamedOverrideArrayCount final : public Castable<NamedOverrideArrayCount, ArrayCount> {
  public:
    /// Constructor
    /// @param var the `override` variable
    explicit NamedOverrideArrayCount(const GlobalVariable* var);
    ~NamedOverrideArrayCount() override;

    /// @returns a hash of the array count.
    size_t Hash() const override;

    /// @param t other array count
    /// @returns true if this array count is equal to the given array count
    bool Equals(const ArrayCount& t) const override;

    /// The `override` variable.
    const GlobalVariable* variable;
};

/// The variant of an ArrayCount when the count is an unnamed override variable.
/// Example:
/// ```
/// override N : i32;
/// type arr = array<i32, N*2>
/// ```
class UnnamedOverrideArrayCount final : public Castable<UnnamedOverrideArrayCount, ArrayCount> {
  public:
    /// Constructor
    /// @param e the override expression
    explicit UnnamedOverrideArrayCount(const Expression* e);
    ~UnnamedOverrideArrayCount() override;

    /// @returns a hash of the array count.
    size_t Hash() const override;

    /// @param t other array count
    /// @returns true if this array count is equal to the given array count
    bool Equals(const ArrayCount& t) const override;

    /// The unnamed override expression.
    /// Note: Each AST expression gets a unique semantic expression node, so two equivalent AST
    /// expressions will not result in the same `expr` pointer. This property is important to ensure
    /// that two array declarations with equivalent AST expressions do not compare equal.
    /// For example, consider:
    /// ```
    /// override size : u32;
    /// var<workgroup> a : array<f32, size * 2>;
    /// var<workgroup> b : array<f32, size * 2>;
    /// ```
    // The array count for `a` and `b` have equivalent AST expressions, but the types for `a` and
    // `b` must not compare equal.
    const Expression* expr;
};

}  // namespace tint::sem

namespace std {

/// std::hash specialization for tint::sem::ArrayCount
template <>
struct hash<tint::sem::ArrayCount> {
    /// @param a the array count to obtain a hash from
    /// @returns the hash of the array count
    size_t operator()(const tint::sem::ArrayCount& a) const { return a.Hash(); }
};

/// std::equal_to specialization for tint::sem::ArrayCount
template <>
struct equal_to<tint::sem::ArrayCount> {
    /// @param a the first array count to compare
    /// @param b the second array count to compare
    /// @returns true if the two array counts are equal
    bool operator()(const tint::sem::ArrayCount& a, const tint::sem::ArrayCount& b) const {
        return a.Equals(b);
    }
};

}  // namespace std

#endif  // SRC_TINT_SEM_ARRAY_COUNT_H_
