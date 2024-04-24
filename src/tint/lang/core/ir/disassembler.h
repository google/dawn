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
#include "src/tint/lang/core/ir/block_param.h"
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
std::string Disassemble(const Module& mod);

/// Helper class to disassemble the IR
class Disassembler {
  public:
    /// A reference to an instruction's operand or result.
    struct IndexedValue {
        /// The instruction that is using the value;
        const Instruction* instruction = nullptr;
        /// The index of the operand that is the value being used.
        size_t index = 0u;

        /// @returns the hash code of the IndexedValue
        tint::HashCode HashCode() const { return Hash(instruction, index); }

        /// An equality helper for IndexedValue.
        /// @param other the IndexedValue to compare against
        /// @returns true if the two IndexedValues are equal
        bool operator==(const IndexedValue& other) const {
            return instruction == other.instruction && index == other.index;
        }
    };

    /// Constructor
    /// @param mod the module
    explicit Disassembler(const Module& mod);
    ~Disassembler();

    /// Returns the module as a string
    /// @returns the string representation of the module
    std::string Disassemble();

    /// @returns the string representation
    std::string AsString() const { return out_.str(); }

    /// @param inst the instruction to retrieve
    /// @returns the source for the instruction
    Source InstructionSource(const Instruction* inst) {
        return instruction_to_src_.GetOr(inst, Source{});
    }

    /// @param operand the operand to retrieve
    /// @returns the source for the operand
    Source OperandSource(IndexedValue operand) { return operand_to_src_.GetOr(operand, Source{}); }

    /// @param result the result to retrieve
    /// @returns the source for the result
    Source ResultSource(IndexedValue result) { return result_to_src_.GetOr(result, Source{}); }

    /// @param blk the block to retrieve
    /// @returns the source for the block
    Source BlockSource(const Block* blk) { return block_to_src_.GetOr(blk, Source{}); }

    /// @param param the block parameter to retrieve
    /// @returns the source for the parameter
    Source BlockParamSource(const BlockParam* param) {
        return block_param_to_src_.GetOr(param, Source{});
    }

    /// @param func the function to retrieve
    /// @returns the source for the function
    Source FunctionSource(const Function* func) { return function_to_src_.GetOr(func, Source{}); }

    /// @param param the function parameter to retrieve
    /// @returns the source for the parameter
    Source FunctionParamSource(const FunctionParam* param) {
        return function_param_to_src_.GetOr(param, Source{});
    }

    /// Stores the given @p src location for @p inst instruction
    /// @param inst the instruction to store
    /// @param src the source location
    void SetSource(const Instruction* inst, Source src) { instruction_to_src_.Add(inst, src); }

    /// Stores the given @p src location for @p blk block
    /// @param blk the block to store
    /// @param src the source location
    void SetSource(const Block* blk, Source src) { block_to_src_.Add(blk, src); }

    /// Stores the given @p src location for @p param block parameter
    /// @param param the block parameter to store
    /// @param src the source location
    void SetSource(const BlockParam* param, Source src) { block_param_to_src_.Add(param, src); }

    /// Stores the given @p src location for @p func function
    /// @param func the function to store
    /// @param src the source location
    void SetSource(const Function* func, Source src) { function_to_src_.Add(func, src); }

    /// Stores the given @p src location for @p param function parameter
    /// @param param the function parameter to store
    /// @param src the source location
    void SetSource(const FunctionParam* param, Source src) {
        function_param_to_src_.Add(param, src);
    }

    /// Stores the given @p src location for @p op operand
    /// @param op the operand to store
    /// @param src the source location
    void SetSource(IndexedValue op, Source src) { operand_to_src_.Add(op, src); }

    /// Stores the given @p src location for @p result
    /// @param result the result to store
    /// @param src the source location
    void SetResultSource(IndexedValue result, Source src) { result_to_src_.Add(result, src); }

    /// @returns the source location for the current emission location
    Source::Location MakeCurrentLocation();

  private:
    class SourceMarker {
      public:
        explicit SourceMarker(Disassembler* d) : dis_(d), begin_(dis_->MakeCurrentLocation()) {}
        ~SourceMarker() = default;

        void Store(const Instruction* inst) { dis_->SetSource(inst, MakeSource()); }

        void Store(const Block* blk) { dis_->SetSource(blk, MakeSource()); }

        void Store(const BlockParam* param) { dis_->SetSource(param, MakeSource()); }

        void Store(const Function* func) { dis_->SetSource(func, MakeSource()); }

        void Store(const FunctionParam* param) { dis_->SetSource(param, MakeSource()); }

        void Store(IndexedValue operand) { dis_->SetSource(operand, MakeSource()); }

        void StoreResult(IndexedValue result) { dis_->SetResultSource(result, MakeSource()); }

        Source MakeSource() const {
            return Source(Source::Range(begin_, dis_->MakeCurrentLocation()));
        }

      private:
        Disassembler* dis_ = nullptr;
        Source::Location begin_;
    };

    StringStream& Indent();

    size_t IdOf(const Block* blk);
    std::string IdOf(const Value* node);
    std::string NameOf(const If* inst);
    std::string NameOf(const Loop* inst);
    std::string NameOf(const Switch* inst);

    void EmitBlock(const Block* blk, std::string_view comment = "");
    void EmitFunction(const Function* func);
    void EmitParamAttributes(const FunctionParam* p);
    void EmitReturnAttributes(const Function* func);
    void EmitBindingPoint(BindingPoint p);
    void EmitLocation(Location loc);
    void EmitInstruction(const Instruction* inst);
    void EmitValueWithType(const Instruction* val);
    void EmitValueWithType(const Value* val);
    void EmitValue(const Value* val);
    void EmitBinary(const Binary* b);
    void EmitUnary(const Unary* b);
    void EmitTerminator(const Terminator* b);
    void EmitSwitch(const Switch* s);
    void EmitLoop(const Loop* l);
    void EmitIf(const If* i);
    void EmitStructDecl(const core::type::Struct* str);
    void EmitLine();
    void EmitOperand(const Instruction* inst, size_t index);
    void EmitOperandList(const Instruction* inst, size_t start_index = 0);
    void EmitInstructionName(const Instruction* inst);

    const Module& mod_;
    StringStream out_;
    Hashmap<const Block*, size_t, 32> block_ids_;
    Hashmap<const Value*, std::string, 32> value_ids_;
    Hashset<std::string, 32> ids_;
    uint32_t indent_size_ = 0;
    bool in_function_ = false;

    uint32_t current_output_line_ = 1;
    uint32_t current_output_start_pos_ = 0;

    Hashmap<const Block*, Source, 8> block_to_src_;
    Hashmap<const BlockParam*, Source, 8> block_param_to_src_;
    Hashmap<const Instruction*, Source, 8> instruction_to_src_;
    Hashmap<IndexedValue, Source, 8> operand_to_src_;
    Hashmap<IndexedValue, Source, 8> result_to_src_;
    Hashmap<const Function*, Source, 8> function_to_src_;
    Hashmap<const FunctionParam*, Source, 8> function_param_to_src_;
    Hashmap<const If*, std::string, 8> if_names_;
    Hashmap<const Loop*, std::string, 8> loop_names_;
    Hashmap<const Switch*, std::string, 8> switch_names_;
};

}  // namespace tint::core::ir

#endif  // SRC_TINT_LANG_CORE_IR_DISASSEMBLER_H_
