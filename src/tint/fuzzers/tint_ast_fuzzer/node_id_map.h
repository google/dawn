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

#ifndef SRC_TINT_FUZZERS_TINT_AST_FUZZER_NODE_ID_MAP_H_
#define SRC_TINT_FUZZERS_TINT_AST_FUZZER_NODE_ID_MAP_H_

#include <unordered_map>

#include "src/tint/program.h"

namespace tint::fuzzers::ast_fuzzer {

/// Contains a one-to-one mapping between the nodes in the AST of the program
/// and their ids.
///
/// The motivation for having this mapping is:
/// - To be able to uniquely identify a node in the AST. This will be used
///   to record transformations in the protobuf messages.
/// - When the AST is being modified, only the mapping for the modified nodes
///   must be affected. That is, if some node is unchanged, it must have the
///   same id defined in this class.
///
/// This class achieves these goals partially. Concretely, the only way to
/// change the AST is by cloning it since all instances of `tint::ast::` classes
/// are immutable. This will invalidate all the pointers to the AST nodes which
/// are used in this class. To overcome this, a new instance of this class is
/// created with all the cloned nodes and the old instance is discarded.
class NodeIdMap {
  public:
    /// Type of the id used by this map.
    using IdType = uint32_t;

    /// Creates an empty map.
    NodeIdMap();

    /// @brief Initializes this instance with all the nodes in the `program`.
    /// @param program - must be valid.
    explicit NodeIdMap(const Program& program);

    /// @brief Returns a node for the given `id`.
    /// @param id - any value is accepted.
    /// @return a pointer to some node if `id` exists in this map.
    /// @return `nullptr` otherwise.
    const ast::Node* GetNode(IdType id) const;

    /// @brief Returns an id of the given `node`.
    /// @param node - can be a `nullptr`.
    /// @return not equal to 0 if `node` exists in this map.
    /// @return 0 otherwise.
    IdType GetId(const ast::Node* node) const;

    /// @brief Adds a mapping from `node` to `id` to this map.
    /// @param node - may not be a `nullptr` and can't be present in this map.
    /// @param id - may not be 0 and can't be present in this map.
    void Add(const ast::Node* node, IdType id);

    /// @brief Returns whether the id is fresh by checking if it exists in
    /// the id map and the id is not 0.
    /// @param id - an id that is used to check in the map.
    /// @return true the given id is fresh and valid (non-zero).
    /// @return false otherwise.
    bool IdIsFreshAndValid(IdType id) const;

    /// @brief Returns an id that is guaranteed to be unoccupied in this map.
    ///
    /// This will effectively increase the counter. This means that two
    /// consecutive calls to this method will return different ids.
    ///
    /// @return an unoccupied id.
    IdType TakeFreshId();

  private:
    IdType fresh_id_ = 1;

    std::unordered_map<const ast::Node*, IdType> node_to_id_;
    std::unordered_map<IdType, const ast::Node*> id_to_node_;
};

}  // namespace tint::fuzzers::ast_fuzzer

#endif  // SRC_TINT_FUZZERS_TINT_AST_FUZZER_NODE_ID_MAP_H_
