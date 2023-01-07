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

#ifndef SRC_TINT_IR_DISASSEMBLER_H_
#define SRC_TINT_IR_DISASSEMBLER_H_

#include <sstream>
#include <string>
#include <unordered_map>
#include <unordered_set>

#include "src/tint/ir/flow_node.h"
#include "src/tint/ir/module.h"

namespace tint::ir {

/// Helper class to disassemble the IR
class Disassembler {
  public:
    /// Constructor
    /// @param mod the module
    explicit Disassembler(const Module& mod);
    ~Disassembler();

    /// Returns the module as a string
    /// @returns the string representation of the module
    std::string Disassemble();

    /// Writes the block instructions to the stream
    /// @param b the block containing the instructions
    void EmitBlockInstructions(const Block* b);

    /// @returns the string representation
    std::string AsString() const { return out_.str(); }

  private:
    std::ostream& Indent();
    void Walk(const FlowNode* node);
    size_t GetIdForNode(const FlowNode* node);

    const Module& mod_;
    std::stringstream out_;
    std::unordered_set<const FlowNode*> visited_;
    std::unordered_set<const FlowNode*> stop_nodes_;
    std::unordered_map<const FlowNode*, size_t> flow_node_to_id_;
    size_t next_node_id_ = 0;
    uint32_t indent_size_ = 0;
};

}  // namespace tint::ir

#endif  // SRC_TINT_IR_DISASSEMBLER_H_
