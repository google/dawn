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

#ifndef SRC_SEM_TYPE_MANAGER_H_
#define SRC_SEM_TYPE_MANAGER_H_

#include <string>
#include <unordered_map>
#include <utility>

#include "src/block_allocator.h"
#include "src/sem/type.h"

namespace tint {
namespace sem {

/// The type manager holds all the pointers to the known types.
class Manager {
 public:
  /// Iterator is the type returned by begin() and end()
  using Iterator = BlockAllocator<sem::Type>::ConstIterator;

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

  /// Get the given type `T` from the type manager
  /// @param args the arguments to pass to the type constructor
  /// @return the pointer to the registered type
  template <typename T, typename... ARGS>
  T* Get(ARGS&&... args) {
    // Note: We do not use std::forward here, as we may need to use the
    // arguments again for the call to Create<T>() below.
    auto name = T(args...).type_name();
    auto it = by_name_.find(name);
    if (it != by_name_.end()) {
      return static_cast<T*>(it->second);
    }

    auto* type = types_.Create<T>(std::forward<ARGS>(args)...);
    by_name_.emplace(name, type);
    return type;
  }

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
    out.by_name_ = inner.by_name_;
    return out;
  }

  /// Returns the type map
  /// @returns the mapping from name string to type.
  const std::unordered_map<std::string, sem::Type*>& types() const {
    return by_name_;
  }

  /// @returns an iterator to the beginning of the types
  Iterator begin() const { return types_.Objects().begin(); }
  /// @returns an iterator to the end of the types
  Iterator end() const { return types_.Objects().end(); }

 private:
  std::unordered_map<std::string, sem::Type*> by_name_;
  BlockAllocator<sem::Type> types_;
};

}  // namespace sem
}  // namespace tint

#endif  // SRC_SEM_TYPE_MANAGER_H_
