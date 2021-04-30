// Copyright 2021 The Tint Authors.
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

#ifndef SRC_SEM_INFO_H_
#define SRC_SEM_INFO_H_

#include <type_traits>
#include <unordered_map>

#include "src/debug.h"
#include "src/sem/node.h"
#include "src/sem/type_mappings.h"

namespace tint {
namespace sem {

/// Info holds all the resolved semantic information for a Program.
class Info {
  /// Placeholder type used by Get() to provide a default value for EXPLICIT_SEM
  using InferFromAST = std::nullptr_t;

 public:
  /// Constructor
  Info();

  /// Move constructor
  Info(Info&&);

  /// Destructor
  ~Info();

  /// Move assignment operator
  /// @param rhs the Program to move
  /// @return this Program
  Info& operator=(Info&& rhs);

  /// Get looks up the semantic information for the AST or type node `node`.
  /// @param node the AST or type node
  /// @returns a pointer to the semantic node if found, otherwise nullptr
  template <typename SEM = InferFromAST,
            typename AST_OR_TYPE = CastableBase,
            typename RESULT =
                std::conditional_t<std::is_same<SEM, InferFromAST>::value,
                                   SemanticNodeTypeFor<AST_OR_TYPE>,
                                   SEM>>
  const RESULT* Get(const AST_OR_TYPE* node) const {
    auto it = map.find(node);
    if (it == map.end()) {
      return nullptr;
    }
    return As<RESULT>(it->second);
  }

  /// Add registers the semantic node `sem_node` for the AST or type node
  /// `node`.
  /// @param node the AST or type node
  /// @param sem_node the semantic node
  template <typename AST_OR_TYPE>
  void Add(const AST_OR_TYPE* node,
           const SemanticNodeTypeFor<AST_OR_TYPE>* sem_node) {
    // Check there's no semantic info already existing for the node
    TINT_ASSERT(Get(node) == nullptr);
    map.emplace(node, sem_node);
  }

  /// Wrap returns a new Info created with the contents of `inner`.
  /// The Info returned by Wrap is intended to temporarily extend the contents
  /// of an existing immutable Info.
  /// As the copied contents are owned by `inner`, `inner` must not be
  /// destructed or assigned while using the returned Info.
  /// @param inner the immutable Info to extend
  /// @return the Info that wraps `inner`
  static Info Wrap(const Info& inner) {
    Info out;
    out.map = inner.map;
    return out;
  }

 private:
  // TODO(crbug.com/tint/724): Once finished, this map should be:
  // std::unordered_map<const ast::Node*, const sem::Node*>
  std::unordered_map<const CastableBase*, const CastableBase*> map;
};

}  // namespace sem
}  // namespace tint

#endif  // SRC_SEM_INFO_H_
