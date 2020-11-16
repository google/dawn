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

#ifndef SRC_CONTEXT_H_
#define SRC_CONTEXT_H_

#include <assert.h>

#include <memory>
#include <type_traits>
#include <utility>
#include <vector>

#include "src/namer.h"
#include "src/type_manager.h"

namespace tint {

namespace ast {
class Node;
}

/// Context object for Tint. Holds various global resources used through
/// the system.
class Context {
 public:
  /// Constructor
  Context();
  /// Constructor
  /// @param namer the namer to set into the context
  explicit Context(std::unique_ptr<Namer> namer);
  /// Destructor
  ~Context();
  /// Resets the state of this context.
  void Reset();

  /// @returns the Type Manager
  TypeManager& type_mgr() { return type_mgr_; }

  /// @returns the namer object
  Namer* namer() const { return namer_.get(); }

  /// Creates a new `ast::Node` owned by the Context. When the Context is
  /// destructed, the `ast::Node` will also be destructed.
  /// @param args the arguments to pass to the type constructor
  /// @returns the node pointer
  template <typename T, typename... ARGS>
  T* create(ARGS&&... args) {
    static_assert(std::is_base_of<ast::Node, T>::value,
                  "T does not derive from ast::Node");
    auto uptr = std::make_unique<T>(std::forward<ARGS>(args)...);
    auto ptr = uptr.get();
    ast_nodes_.emplace_back(std::move(uptr));
    return ptr;
  }

 private:
  TypeManager type_mgr_;
  std::unique_ptr<Namer> namer_;
  std::vector<std::unique_ptr<ast::Node>> ast_nodes_;
};

}  // namespace tint

#endif  // SRC_CONTEXT_H_
