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

#ifndef SRC_TINT_TYPE_MANAGER_H_
#define SRC_TINT_TYPE_MANAGER_H_

#include <functional>
#include <string>
#include <unordered_map>
#include <utility>

#include "src/tint/type/array_count.h"
#include "src/tint/type/node.h"
#include "src/tint/type/struct.h"
#include "src/tint/type/type.h"
#include "src/tint/utils/unique_allocator.h"

namespace tint::type {

/// The type manager holds all the pointers to the known types.
class Manager final {
  public:
    /// Iterator is the type returned by begin() and end()
    using TypeIterator = utils::BlockAllocator<Type>::ConstIterator;

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

    /// Wrap returns a new Manager created with the types of `inner`.
    /// The Manager returned by Wrap is intended to temporarily extend the types
    /// of an existing immutable Manager.
    /// As the copied types are owned by `inner`, `inner` must not be destructed
    /// or assigned while using the returned Manager.
    /// TODO(bclayton) - Evaluate whether there are safer alternatives to this
    /// function. See crbug.com/tint/460.
    /// @param inner the immutable Manager to extend
    /// @return the Manager that wraps `inner`
    static Manager Wrap(const Manager& inner) {
        Manager out;
        out.types_.Wrap(inner.types_);
        out.nodes_.Wrap(inner.nodes_);
        return out;
    }

    /// @param args the arguments used to construct the object.
    /// @return a pointer to an instance of `T` with the provided arguments.
    ///         If an existing instance of `T` has been constructed, then the same
    ///         pointer is returned.
    template <typename TYPE,
              typename _ = std::enable_if<traits::IsTypeOrDerived<TYPE, Type>>,
              typename... ARGS>
    TYPE* Get(ARGS&&... args) {
        return types_.Get<TYPE>(std::forward<ARGS>(args)...);
    }

    /// @param args the arguments used to create the temporary used for the search.
    /// @return a pointer to an instance of `T` with the provided arguments, or nullptr if the item
    ///         was not found.
    template <typename TYPE,
              typename _ = std::enable_if<traits::IsTypeOrDerived<TYPE, Type>>,
              typename... ARGS>
    TYPE* Find(ARGS&&... args) const {
        return types_.Find<TYPE>(std::forward<ARGS>(args)...);
    }

    /// @param args the arguments used to construct the object.
    /// @return a pointer to an instance of `T` with the provided arguments.
    ///         If an existing instance of `T` has been constructed, then the same
    ///         pointer is returned.
    template <typename TYPE,
              typename _ = std::enable_if<traits::IsTypeOrDerived<TYPE, ArrayCount> ||
                                          traits::IsTypeOrDerived<TYPE, StructMember>>,
              typename... ARGS>
    TYPE* GetNode(ARGS&&... args) {
        return nodes_.Get<TYPE>(std::forward<ARGS>(args)...);
    }

    /// @returns an iterator to the beginning of the types
    TypeIterator begin() const { return types_.begin(); }
    /// @returns an iterator to the end of the types
    TypeIterator end() const { return types_.end(); }

  private:
    utils::UniqueAllocator<Type> types_;
    utils::UniqueAllocator<Node> nodes_;
};

}  // namespace tint::type

namespace std {

/// std::hash specialization for tint::type::Node
template <>
struct hash<tint::type::Node> {
    /// @param type the type to obtain a hash from
    /// @returns the hash of the type
    size_t operator()(const tint::type::Node& type) const {
        if (const auto* ac = type.As<tint::type::ArrayCount>()) {
            return ac->Hash();
        } else if (type.Is<tint::type::StructMember>()) {
            return tint::TypeInfo::Of<tint::type::StructMember>().full_hashcode;
        }
        TINT_ASSERT(Type, false && "Unreachable");
        return 0;
    }
};

/// std::equal_to specialization for tint::type::Node
template <>
struct equal_to<tint::type::Node> {
    /// @param a the first type to compare
    /// @param b the second type to compare
    /// @returns true if the two types are equal
    bool operator()(const tint::type::Node& a, const tint::type::Node& b) const {
        if (const auto* ac = a.As<tint::type::ArrayCount>()) {
            if (const auto* bc = b.As<tint::type::ArrayCount>()) {
                return ac->Equals(*bc);
            }
            return false;
        } else if (a.Is<tint::type::StructMember>()) {
            return &a == &b;
        }
        TINT_ASSERT(Type, false && "Unreachable");
        return false;
    }
};

}  // namespace std

#endif  // SRC_TINT_TYPE_MANAGER_H_
