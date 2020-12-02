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

#ifndef SRC_AST_CLONE_CONTEXT_H_
#define SRC_AST_CLONE_CONTEXT_H_

#include <unordered_map>
#include <vector>

#include "src/source.h"

namespace tint {
namespace ast {

class Module;

/// CloneContext holds the state used while cloning AST nodes and types.
class CloneContext {
 public:
  /// Constructor
  /// @param m the target module to clone into
  explicit CloneContext(Module* m);
  /// Destructor
  ~CloneContext();

  /// Clones the `Node` or `type::Type` `a` into the module #mod if `a` is not
  /// null. If `a` is null, then Clone() returns null. If `a` has been cloned
  /// already by this CloneContext then the same cloned pointer is returned.
  /// @note Semantic information such as resolved expression type and intrinsic
  /// information is not cloned.
  /// @param a the `Node` or `type::Type` to clone
  /// @return the cloned node
  template <typename T>
  T* Clone(T* a) {
    if (a == nullptr) {
      return nullptr;
    }

    auto it = cloned_.find(a);
    if (it != cloned_.end()) {
      return static_cast<T*>(it->second);
    }
    auto* c = a->Clone(this);
    cloned_.emplace(a, c);
    return static_cast<T*>(c);
  }

  /// Clones the `Source` `s` into `mod`
  /// TODO(bclayton) - Currently this 'clone' is a shallow copy. If/when
  /// `Source.File`s are owned by the `Module` this should make a copy of the
  /// file.
  /// @param s the `Source` to clone
  /// @return the cloned source
  Source Clone(const Source& s) { return s; }

  /// Clones each of the elements of the vector `v` into the module #mod.
  /// @param v the vector to clone
  /// @return the cloned vector
  template <typename T>
  std::vector<T> Clone(const std::vector<T>& v) {
    std::vector<T> out;
    out.reserve(v.size());
    for (auto& el : v) {
      out.emplace_back(Clone(el));
    }
    return out;
  }

  /// The target module to clone into.
  Module* const mod;

 private:
  std::unordered_map<void*, void*> cloned_;
};

}  // namespace ast
}  // namespace tint

#endif  // SRC_AST_CLONE_CONTEXT_H_
