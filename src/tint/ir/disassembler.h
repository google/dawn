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
#include "src/tint/ir/block.h"
#include "src/tint/ir/call.h"
#include "src/tint/ir/if.h"
#include "src/tint/ir/loop.h"
#include "src/tint/ir/module.h"
#include "src/tint/ir/switch.h"
#include "src/tint/ir/unary.h"
#include "src/tint/utils/hashmap.h"
#include "src/tint/utils/hashset.h"
#include "src/tint/utils/string_stream.h"

// Forward declarations.
namespace tint::type {
class Struct;
}

namespace tint::ir {

/// Helper class to disassemble the IR
class Disassembler {
  public:
    /// Constructor
    /// @param mod the module
    explicit Disassembler(Module& mod);
    ~Disassembler();

    /// Returns the module as a string
    /// @returns the string representation of the module
    std::string Disassemble();

    /// Writes the block instructions to the stream
    /// @param b the block containing the instructions
    void EmitBlockInstructions(Block* b);

    /// @returns the string representation
    std::string AsString() const { return out_.str(); }

    /// @param inst the instruction to retrieve
    /// @returns the source for the instruction
    Source InstructionSource(Instruction* inst) {
        return instruction_to_src_.Get(inst).value_or(Source{});
    }

    /// @param operand the operand to retrieve
    /// @returns the source for the operand
    Source OperandSource(Usage operand) { return operand_to_src_.Get(operand).value_or(Source{}); }

    /// @param blk teh block to retrieve
    /// @returns the source for the block
    Source BlockSource(Block* blk) { return block_to_src_.Get(blk).value_or(Source{}); }

    /// Stores the given @p src location for @p inst instruction
    /// @param inst the instruction to store
    /// @param src the source location
    void SetSource(Instruction* inst, Source src) { instruction_to_src_.Add(inst, src); }

    /// Stores the given @p src location for @p blk block
    /// @param blk the block to store
    /// @param src the source location
    void SetSource(Block* blk, Source src) { block_to_src_.Add(blk, src); }

    /// Stores the given @p src location for @p op operand
    /// @param op the operand to store
    /// @param src the source location
    void SetSource(Usage op, Source src) { operand_to_src_.Add(op, src); }

    /// @returns the source location for the current emission location
    Source::Location MakeCurrentLocation();

  private:
    class SourceMarker {
      public:
        explicit SourceMarker(Disassembler* d) : dis_(d), begin_(dis_->MakeCurrentLocation()) {}
        ~SourceMarker() = default;

        void Store(Instruction* inst) { dis_->SetSource(inst, MakeSource()); }

        void Store(Block* blk) { dis_->SetSource(blk, MakeSource()); }

        void Store(Usage operand) { dis_->SetSource(operand, MakeSource()); }

        Source MakeSource() const {
            return Source(Source::Range(begin_, dis_->MakeCurrentLocation()));
        }

      private:
        Disassembler* dis_ = nullptr;
        Source::Location begin_;
    };

    utils::StringStream& Indent();

    size_t IdOf(Block* blk);
    std::string_view IdOf(Value* node);

    void Walk(Block* blk);
    void WalkInternal(Block* blk);
    void EmitFunction(Function* func);
    void EmitParamAttributes(FunctionParam* p);
    void EmitReturnAttributes(Function* func);
    void EmitBindingPoint(BindingPoint p);
    void EmitLocation(Location loc);
    void EmitInstruction(Instruction* inst);
    void EmitValueWithType(Value* val);
    void EmitValue(Value* val);
    void EmitValueList(utils::Slice<ir::Value* const> values);
    void EmitArgs(Call* call);
    void EmitBinary(Binary* b);
    void EmitUnary(Unary* b);
    void EmitBranch(Branch* b);
    void EmitSwitch(Switch* s);
    void EmitLoop(Loop* l);
    void EmitIf(If* i);
    void EmitStructDecl(const type::Struct* str);
    void EmitLine();
    void EmitOperand(Instruction* inst, Value* val, size_t index);
    void EmitOperandList(Instruction* inst,
                         utils::Slice<Value* const> operands,
                         size_t start_index);
    void EmitInstructionName(std::string_view name, Instruction* inst);

    Module& mod_;
    utils::StringStream out_;
    utils::Hashset<Block*, 32> visited_;
    utils::Hashmap<Block*, size_t, 32> block_ids_;
    utils::Hashmap<Value*, std::string, 32> value_ids_;
    uint32_t indent_size_ = 0;
    bool in_function_ = false;

    uint32_t current_output_line_ = 1;
    uint32_t current_output_start_pos_ = 0;

    utils::Hashmap<Block*, Source, 8> block_to_src_;
    utils::Hashmap<Instruction*, Source, 8> instruction_to_src_;
    utils::Hashmap<Usage, Source, 8, Usage::Hasher> operand_to_src_;
};

}  // namespace tint::ir

#endif  // SRC_TINT_IR_DISASSEMBLER_H_
