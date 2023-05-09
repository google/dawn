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

#include <string>

#include "src/tint/ir/binary.h"
#include "src/tint/ir/call.h"
#include "src/tint/ir/flow_node.h"
#include "src/tint/ir/module.h"
#include "src/tint/ir/unary.h"
#include "src/tint/utils/hashmap.h"
#include "src/tint/utils/hashset.h"
#include "src/tint/utils/string_stream.h"

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
    utils::StringStream& Indent();

    size_t IdOf(const FlowNode* node);
    std::string_view IdOf(const Value* node);

    void Walk(const FlowNode* node);
    void EmitInstruction(const Instruction* inst);
    void EmitValue(const Value* val);
    void EmitArgs(const Call* call);
    void EmitBinary(const Binary* b);
    void EmitUnary(const Unary* b);

    const Module& mod_;
    utils::StringStream out_;
    utils::Hashset<const FlowNode*, 32> visited_;
    utils::Hashset<const FlowNode*, 32> stop_nodes_;
    utils::Hashmap<const FlowNode*, size_t, 32> flow_node_ids_;
    utils::Hashmap<const Value*, std::string, 32> value_ids_;
    uint32_t indent_size_ = 0;
    bool in_function_ = false;
};

}  // namespace tint::ir

#endif  // SRC_TINT_IR_DISASSEMBLER_H_
