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

#ifndef SRC_TINT_TYPE_UNIQUE_NODE_H_
#define SRC_TINT_TYPE_UNIQUE_NODE_H_

#include <functional>

#include "src/tint/type/node.h"

namespace tint::type {

/// UniqueNode is the base class for objects that are de-duplicated by the Manager.
/// Deduplication is achieved by comparing a temporary object to the set of existing objects, using
/// Hash() and Equals(). If an existing object is found, then the pointer to that object is
/// returned, otherwise a new object is constructed, added to the Manager's set and returned.
class UniqueNode : public utils::Castable<UniqueNode, Node> {
  public:
    /// Constructor
    /// @param hash the immutable hash for the node
    inline explicit UniqueNode(size_t hash) : unique_hash(hash) {}

    /// Destructor
    ~UniqueNode() override;

    /// @param other the other node to compare this node against
    /// @returns true if the this node is equal to @p other
    virtual bool Equals(const UniqueNode& other) const = 0;

    /// the immutable hash for the node
    const size_t unique_hash;
};

}  // namespace tint::type

namespace std {

/// std::hash specialization for tint::type::UniqueNode
template <>
struct hash<tint::type::UniqueNode> {
    /// @param node the unique node to obtain a hash from
    /// @returns the hash of the node
    size_t operator()(const tint::type::UniqueNode& node) const { return node.unique_hash; }
};

/// std::equal_to specialization for tint::type::UniqueNode
template <>
struct equal_to<tint::type::UniqueNode> {
    /// @param a the first unique node to compare
    /// @param b the second unique node to compare
    /// @returns true if the two nodes are equal
    bool operator()(const tint::type::UniqueNode& a, const tint::type::UniqueNode& b) const {
        return &a == &b || a.Equals(b);
    }
};

}  // namespace std

#endif  // SRC_TINT_TYPE_UNIQUE_NODE_H_
