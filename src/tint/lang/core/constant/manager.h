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

#ifndef SRC_TINT_LANG_CORE_CONSTANT_MANAGER_H_
#define SRC_TINT_LANG_CORE_CONSTANT_MANAGER_H_

#include <utility>

#include "src/tint/lang/core/constant/value.h"
#include "src/tint/lang/core/number.h"
#include "src/tint/lang/core/type/manager.h"
#include "src/tint/utils/containers/unique_allocator.h"
#include "src/tint/utils/math/hash.h"

namespace tint::constant {
class Splat;

template <typename T>
class Scalar;
}  // namespace tint::constant

namespace tint::constant {

/// The constant manager holds a type manager and all the pointers to the known constant values.
class Manager final {
  public:
    /// Iterator is the type returned by begin() and end()
    using TypeIterator = BlockAllocator<Value>::ConstIterator;

    /// Constructor
    Manager();

    /// Move constructor
    Manager(Manager&&);

    /// Move assignment operator
    /// @param rhs the Manager to move
    /// @return this Manager
    Manager& operator=(Manager&& rhs);

    /// Destructor
    ~Manager();

    /// Wrap returns a new Manager created with the constants and types of `inner`.
    /// The Manager returned by Wrap is intended to temporarily extend the constants and types of an
    /// existing immutable Manager. As the copied constants and types are owned by `inner`, `inner`
    /// must not be destructed or assigned while using the returned Manager.
    /// TODO(bclayton) - Evaluate whether there are safer alternatives to this
    /// function. See crbug.com/tint/460.
    /// @param inner the immutable Manager to extend
    /// @return the Manager that wraps `inner`
    static Manager Wrap(const Manager& inner) {
        Manager out;
        out.values_.Wrap(inner.values_);
        out.types = type::Manager::Wrap(inner.types);
        return out;
    }

    /// @param args the arguments used to construct the type, unique node or node.
    /// @return a pointer to an instance of `T` with the provided arguments.
    ///         If NODE derives from UniqueNode and an existing instance of `T` has been
    ///         constructed, then the same pointer is returned.
    template <typename NODE, typename... ARGS>
    NODE* Get(ARGS&&... args) {
        return values_.Get<NODE>(std::forward<ARGS>(args)...);
    }

    /// @returns an iterator to the beginning of the types
    TypeIterator begin() const { return values_.begin(); }
    /// @returns an iterator to the end of the types
    TypeIterator end() const { return values_.end(); }

    /// Constructs a constant of a vector, matrix or array type.
    ///
    /// Examines the element values and will return either a constant::Composite or a
    /// constant::Splat, depending on the element types and values.
    ///
    /// @param type the composite type
    /// @param elements the composite elements
    /// @returns the value pointer
    const constant::Value* Composite(const type::Type* type,
                                     VectorRef<const constant::Value*> elements);

    /// Constructs a splat constant.
    /// @param type the splat type
    /// @param element the splat element
    /// @param n the number of elements
    /// @returns the value pointer
    const constant::Splat* Splat(const type::Type* type, const constant::Value* element, size_t n);

    /// @param value the constant value
    /// @return a Scalar holding the i32 value @p value
    const Scalar<i32>* Get(i32 value);

    /// @param value the constant value
    /// @return a Scalar holding the u32 value @p value
    const Scalar<u32>* Get(u32 value);

    /// @param value the constant value
    /// @return a Scalar holding the f32 value @p value
    const Scalar<f32>* Get(f32 value);

    /// @param value the constant value
    /// @return a Scalar holding the f16 value @p value
    const Scalar<f16>* Get(f16 value);

    /// @param value the constant value
    /// @return a Scalar holding the bool value @p value
    const Scalar<bool>* Get(bool value);

    /// @param value the constant value
    /// @return a Scalar holding the AFloat value @p value
    const Scalar<AFloat>* Get(AFloat value);

    /// @param value the constant value
    /// @return a Scalar holding the AInt value @p value
    const Scalar<AInt>* Get(AInt value);

    /// The type manager
    type::Manager types;

  private:
    /// A specialization of Hasher for constant::Value
    struct Hasher {
        /// @param value the value to hash
        /// @returns a hash of the value
        size_t operator()(const constant::Value& value) const { return value.Hash(); }
    };

    /// An equality helper for constant::Value
    struct Equal {
        /// @param a the LHS value
        /// @param b the RHS value
        /// @returns true if the two constants are equal
        bool operator()(const constant::Value& a, const constant::Value& b) const {
            return a.Equal(&b);
        }
    };

    /// Unique types owned by the manager
    UniqueAllocator<Value, Hasher, Equal> values_;
};

}  // namespace tint::constant

#endif  // SRC_TINT_LANG_CORE_CONSTANT_MANAGER_H_
