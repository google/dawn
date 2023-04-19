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

#include <utility>

#include "src/tint/type/type.h"
#include "src/tint/utils/hash.h"
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
        out.unique_nodes_.Wrap(inner.unique_nodes_);
        return out;
    }

    /// @param args the arguments used to construct the type, unique node or node.
    /// @return a pointer to an instance of `T` with the provided arguments.
    ///         If NODE derives from UniqueNode and an existing instance of `T` has been
    ///         constructed, then the same pointer is returned.
    template <typename NODE, typename... ARGS>
    NODE* Get(ARGS&&... args) {
        if constexpr (utils::traits::IsTypeOrDerived<NODE, Type>) {
            return types_.Get<NODE>(std::forward<ARGS>(args)...);
        } else if constexpr (utils::traits::IsTypeOrDerived<NODE, UniqueNode>) {
            return unique_nodes_.Get<NODE>(std::forward<ARGS>(args)...);
        } else {
            return nodes_.Create<NODE>(std::forward<ARGS>(args)...);
        }
    }

    /// @param args the arguments used to create the temporary used for the search.
    /// @return a pointer to an instance of `T` with the provided arguments, or nullptr if the item
    ///         was not found.
    template <typename TYPE,
              typename _ = std::enable_if<utils::traits::IsTypeOrDerived<TYPE, Type>>,
              typename... ARGS>
    TYPE* Find(ARGS&&... args) const {
        return types_.Find<TYPE>(std::forward<ARGS>(args)...);
    }

    /// @returns an iterator to the beginning of the types
    TypeIterator begin() const { return types_.begin(); }
    /// @returns an iterator to the end of the types
    TypeIterator end() const { return types_.end(); }

  private:
    /// Unique types owned by the manager
    utils::UniqueAllocator<Type> types_;
    /// Unique nodes (excluding types) owned by the manager
    utils::UniqueAllocator<UniqueNode> unique_nodes_;
    /// Non-unique nodes owned by the manager
    utils::BlockAllocator<Node> nodes_;
};

}  // namespace tint::type

#endif  // SRC_TINT_TYPE_MANAGER_H_
