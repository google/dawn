// Copyright 2020 The Tint Authors.
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

#ifndef SRC_TINT_SEM_TYPE_MANAGER_H_
#define SRC_TINT_SEM_TYPE_MANAGER_H_

#include <string>
#include <unordered_map>
#include <utility>

#include "src/tint/sem/type.h"
#include "src/tint/utils/unique_allocator.h"

namespace tint::sem {

/// The type manager holds all the pointers to the known types.
class Manager final : public utils::UniqueAllocator<Type> {
  public:
    /// Iterator is the type returned by begin() and end()
    using Iterator = utils::BlockAllocator<Type>::ConstIterator;

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
        out.items = inner.items;
        return out;
    }

    /// @returns an iterator to the beginning of the types
    Iterator begin() const { return allocator.Objects().begin(); }
    /// @returns an iterator to the end of the types
    Iterator end() const { return allocator.Objects().end(); }
};

}  // namespace tint::sem

#endif  // SRC_TINT_SEM_TYPE_MANAGER_H_
