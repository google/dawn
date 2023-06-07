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
    /// An operand used in an instruction
    struct Operand {
        /// The instruction
        const Instruction* instruction = nullptr;
        /// The operand index
        uint32_t operand_index = 0u;

        /// A specialization of utils::Hasher for Operand.
        struct Hasher {
            /// @param u the operand to hash
            /// @returns a hash of the operand
            inline std::size_t operator()(const Operand& u) const {
                return utils::Hash(u.instruction, u.operand_index);
            }
        };

        /// An equality helper for Operand.
        /// @param other the operand to compare against
        /// @returns true if the two operands are equal
        bool operator==(const Operand& other) const {
            return instruction == other.instruction && operand_index == other.operand_index;
        }
    };

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

    /// @param inst the instruction to retrieve
    /// @returns the source for the instruction
    Source InstructionSource(const Instruction* inst) {
        return instruction_to_src_.Get(inst).value_or(Source{});
    }

    /// @param operand the operand to retrieve
    /// @returns the source for the operand
    Source OperandSource(Operand operand) {
        return operand_to_src_.Get(operand).value_or(Source{});
    }

    /// @param blk teh block to retrieve
    /// @returns the source for the block
    Source BlockSource(const Block* blk) { return block_to_src_.Get(blk).value_or(Source{}); }

    /// Stores the given @p src location for @p inst instruction
    /// @param inst the instruction to store
    /// @param src the source location
    void SetSource(const Instruction* inst, Source src) { instruction_to_src_.Add(inst, src); }

    /// Stores the given @p src location for @p blk block
    /// @param blk the block to store
    /// @param src the source location
    void SetSource(const Block* blk, Source src) { block_to_src_.Add(blk, src); }

    /// Stores the given @p src location for @p op operand
    /// @param op the operand to store
    /// @param src the source location
    void SetSource(Operand op, Source src) { operand_to_src_.Add(op, src); }

    /// @returns the source location for the current emission location
    Source::Location MakeCurrentLocation();

  private:
    class SourceMarker {
      public:
        explicit SourceMarker(Disassembler* d) : dis_(d), begin_(dis_->MakeCurrentLocation()) {}
        ~SourceMarker() = default;

        void Store(const Instruction* inst) { dis_->SetSource(inst, MakeSource()); }

        void Store(const Block* blk) { dis_->SetSource(blk, MakeSource()); }

        void Store(Operand operand) { dis_->SetSource(operand, MakeSource()); }

        Source MakeSource() const {
            return Source(Source::Range(begin_, dis_->MakeCurrentLocation()));
        }

      private:
        Disassembler* dis_ = nullptr;
        Source::Location begin_;
    };

    utils::StringStream& Indent();

    size_t IdOf(const Block* blk);
    std::string_view IdOf(const Value* node);

    void Walk(const Block* blk);
    void WalkInternal(const Block* blk);
    void EmitFunction(const Function* func);
    void EmitParamAttributes(const FunctionParam* p);
    void EmitReturnAttributes(const Function* func);
    void EmitBindingPoint(BindingPoint p);
    void EmitLocation(Location loc);
    void EmitInstruction(const Instruction* inst);
    void EmitValueWithType(const Value* val);
    void EmitValue(const Value* val);
    void EmitValueList(utils::Slice<ir::Value const* const> values);
    void EmitArgs(const Call* call);
    void EmitBinary(const Binary* b);
    void EmitUnary(const Unary* b);
    void EmitBranch(const Branch* b);
    void EmitSwitch(const Switch* s);
    void EmitLoop(const Loop* l);
    void EmitIf(const If* i);
    void EmitStructDecl(const type::Struct* str);
    void EmitLine();
    void EmitOperand(const Value* val, const Instruction* inst, uint32_t index);
    void EmitInstructionName(std::string_view name, const Instruction* inst);

    const Module& mod_;
    utils::StringStream out_;
    utils::Hashset<const Block*, 32> visited_;
    utils::Hashmap<const Block*, size_t, 32> block_ids_;
    utils::Hashmap<const Value*, std::string, 32> value_ids_;
    uint32_t indent_size_ = 0;
    bool in_function_ = false;

    uint32_t current_output_line_ = 1;
    uint32_t current_output_start_pos_ = 0;

    utils::Hashmap<const Block*, Source, 8> block_to_src_;
    utils::Hashmap<const Instruction*, Source, 8> instruction_to_src_;
    utils::Hashmap<Operand, Source, 8, Operand::Hasher> operand_to_src_;
};

}  // namespace tint::ir

#endif  // SRC_TINT_IR_DISASSEMBLER_H_
