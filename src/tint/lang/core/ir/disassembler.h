// Copyright 2022 The Dawn & Tint Authors
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

#ifndef SRC_TINT_LANG_CORE_IR_DISASSEMBLER_H_
#define SRC_TINT_LANG_CORE_IR_DISASSEMBLER_H_

#include <string>

#include "src/tint/lang/core/ir/binary.h"
#include "src/tint/lang/core/ir/block.h"
#include "src/tint/lang/core/ir/call.h"
#include "src/tint/lang/core/ir/if.h"
#include "src/tint/lang/core/ir/loop.h"
#include "src/tint/lang/core/ir/module.h"
#include "src/tint/lang/core/ir/switch.h"
#include "src/tint/lang/core/ir/unary.h"
#include "src/tint/utils/containers/hashmap.h"
#include "src/tint/utils/containers/hashset.h"
#include "src/tint/utils/text/string_stream.h"

// Forward declarations.
namespace tint::core::type {
class Struct;
}

namespace tint::core::ir {

/// @returns the disassembly for the module @p mod
/// @param mod the module to disassemble
std::string Disassemble(Module& mod);

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

    /// @param result the result to retrieve
    /// @returns the source for the result
    Source ResultSource(Usage result) { return result_to_src_.Get(result).value_or(Source{}); }

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

    /// Stores the given @p src location for @p result
    /// @param result the result to store
    /// @param src the source location
    void SetResultSource(Usage result, Source src) { result_to_src_.Add(result, src); }

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

        void StoreResult(Usage result) { dis_->SetResultSource(result, MakeSource()); }

        Source MakeSource() const {
            return Source(Source::Range(begin_, dis_->MakeCurrentLocation()));
        }

      private:
        Disassembler* dis_ = nullptr;
        Source::Location begin_;
    };

    StringStream& Indent();

    size_t IdOf(Block* blk);
    std::string IdOf(Value* node);
    std::string NameOf(If* inst);
    std::string NameOf(Loop* inst);
    std::string NameOf(Switch* inst);

    void EmitBlock(Block* blk, std::string_view comment = "");
    void EmitFunction(Function* func);
    void EmitParamAttributes(FunctionParam* p);
    void EmitReturnAttributes(Function* func);
    void EmitBindingPoint(BindingPoint p);
    void EmitLocation(Location loc);
    void EmitInstruction(Instruction* inst);
    void EmitValueWithType(Instruction* val);
    void EmitValueWithType(Value* val);
    void EmitValue(Value* val);
    void EmitValueList(tint::Slice<ir::Value* const> values);
    void EmitBinary(Binary* b);
    void EmitUnary(Unary* b);
    void EmitTerminator(Terminator* b);
    void EmitSwitch(Switch* s);
    void EmitLoop(Loop* l);
    void EmitIf(If* i);
    void EmitStructDecl(const core::type::Struct* str);
    void EmitLine();
    void EmitOperand(Instruction* inst, size_t index);
    void EmitOperandList(Instruction* inst, size_t start_index = 0);
    void EmitInstructionName(Instruction* inst);

    Module& mod_;
    StringStream out_;
    Hashmap<Block*, size_t, 32> block_ids_;
    Hashmap<Value*, std::string, 32> value_ids_;
    Hashset<std::string, 32> ids_;
    uint32_t indent_size_ = 0;
    bool in_function_ = false;

    uint32_t current_output_line_ = 1;
    uint32_t current_output_start_pos_ = 0;

    Hashmap<Block*, Source, 8> block_to_src_;
    Hashmap<Instruction*, Source, 8> instruction_to_src_;
    Hashmap<Usage, Source, 8, Usage::Hasher> operand_to_src_;
    Hashmap<Usage, Source, 8, Usage::Hasher> result_to_src_;
    Hashmap<If*, std::string, 8> if_names_;
    Hashmap<Loop*, std::string, 8> loop_names_;
    Hashmap<Switch*, std::string, 8> switch_names_;
};

}  // namespace tint::core::ir

#endif  // SRC_TINT_LANG_CORE_IR_DISASSEMBLER_H_
