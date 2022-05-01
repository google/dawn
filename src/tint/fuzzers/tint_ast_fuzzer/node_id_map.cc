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

#include "src/tint/fuzzers/tint_ast_fuzzer/node_id_map.h"

#include <cassert>

namespace tint::fuzzers::ast_fuzzer {

NodeIdMap::NodeIdMap() = default;

NodeIdMap::NodeIdMap(const Program& program) : NodeIdMap() {
    for (const auto* node : program.ASTNodes().Objects()) {
        Add(node, TakeFreshId());
    }
}

NodeIdMap::IdType NodeIdMap::GetId(const ast::Node* node) const {
    auto it = node_to_id_.find(node);
    return it == node_to_id_.end() ? 0 : it->second;
}

const ast::Node* NodeIdMap::GetNode(IdType id) const {
    auto it = id_to_node_.find(id);
    return it == id_to_node_.end() ? nullptr : it->second;
}

void NodeIdMap::Add(const ast::Node* node, IdType id) {
    assert(!node_to_id_.count(node) && "The node already exists in the map");
    assert(IdIsFreshAndValid(id) && "Id already exists in the map or Id is zero");
    assert(node && "`node` can't be a nullptr");

    node_to_id_[node] = id;
    id_to_node_[id] = node;

    if (id >= fresh_id_) {
        fresh_id_ = id + 1;
    }
}

bool NodeIdMap::IdIsFreshAndValid(IdType id) const {
    return id && !id_to_node_.count(id);
}

NodeIdMap::IdType NodeIdMap::TakeFreshId() {
    assert(fresh_id_ != 0 && "`NodeIdMap` id has overflowed");
    return fresh_id_++;
}

}  // namespace tint::fuzzers::ast_fuzzer
