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

#include "src/tint/sem/array_count.h"
#include "src/tint/sem/type.h"
#include "src/tint/utils/unique_allocator.h"

namespace tint::sem {

/// The type manager holds all the pointers to the known types.
class TypeManager final {
  public:
    /// Iterator is the type returned by begin() and end()
    using TypeIterator = utils::BlockAllocator<Type>::ConstIterator;

    /// Constructor
    TypeManager();

    /// Move constructor
    TypeManager(TypeManager&&);

    /// Move assignment operator
    /// @param rhs the Manager to move
    /// @return this Manager
    TypeManager& operator=(TypeManager&& rhs);

    /// Destructor
    ~TypeManager();

    /// Wrap returns a new Manager created with the types of `inner`.
    /// The Manager returned by Wrap is intended to temporarily extend the types
    /// of an existing immutable Manager.
    /// As the copied types are owned by `inner`, `inner` must not be destructed
    /// or assigned while using the returned Manager.
    /// TODO(bclayton) - Evaluate whether there are safer alternatives to this
    /// function. See crbug.com/tint/460.
    /// @param inner the immutable Manager to extend
    /// @return the Manager that wraps `inner`
    static TypeManager Wrap(const TypeManager& inner) {
        TypeManager out;
        out.types_.Wrap(inner.types_);
        out.array_counts_.Wrap(inner.array_counts_);
        return out;
    }

    /// @param args the arguments used to construct the object.
    /// @return a pointer to an instance of `T` with the provided arguments.
    ///         If an existing instance of `T` has been constructed, then the same
    ///         pointer is returned.
    template <typename TYPE,
              typename _ = std::enable_if<traits::IsTypeOrDerived<TYPE, sem::Type>>,
              typename... ARGS>
    TYPE* Get(ARGS&&... args) {
        return types_.Get<TYPE>(std::forward<ARGS>(args)...);
    }

    /// @param args the arguments used to create the temporary used for the search.
    /// @return a pointer to an instance of `T` with the provided arguments, or nullptr if the item
    ///         was not found.
    template <typename TYPE,
              typename _ = std::enable_if<traits::IsTypeOrDerived<TYPE, sem::Type>>,
              typename... ARGS>
    TYPE* Find(ARGS&&... args) const {
        return types_.Find<TYPE>(std::forward<ARGS>(args)...);
    }

    /// @param args the arguments used to construct the object.
    /// @return a pointer to an instance of `T` with the provided arguments.
    ///         If an existing instance of `T` has been constructed, then the same
    ///         pointer is returned.
    template <typename TYPE,
              typename _ = std::enable_if<traits::IsTypeOrDerived<TYPE, sem::ArrayCount>>,
              typename... ARGS>
    TYPE* GetArrayCount(ARGS&&... args) {
        return array_counts_.Get<TYPE>(std::forward<ARGS>(args)...);
    }

    /// @returns an iterator to the beginning of the types
    TypeIterator begin() const { return types_.begin(); }
    /// @returns an iterator to the end of the types
    TypeIterator end() const { return types_.end(); }

  private:
    utils::UniqueAllocator<Type> types_;
    utils::UniqueAllocator<ArrayCount> array_counts_;
};

}  // namespace tint::sem

#endif  // SRC_TINT_SEM_TYPE_MANAGER_H_
