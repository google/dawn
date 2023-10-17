// Copyright 2021 The Dawn & Tint Authors
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// 1. Redistributions of source code must retain the above copyright notice, this
//    list of conditions and the following disclaimer.
//
// 2. Redistributions in binary form must reproduce the above copyright notice,
//    this list of conditions and the following disclaimer in the documentation
//    and/or other materials provided with the distribution.
//
// 3. Neither the name of the copyright holder nor the names of its
//    contributors may be used to endorse or promote products derived from
//    this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
// FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
// DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
// CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
// OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

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
