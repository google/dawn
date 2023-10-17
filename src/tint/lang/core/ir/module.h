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

#ifndef SRC_TINT_LANG_CORE_IR_MODULE_H_
#define SRC_TINT_LANG_CORE_IR_MODULE_H_

#include <memory>
#include <string>

#include "src/tint/lang/core/constant/manager.h"
#include "src/tint/lang/core/ir/block.h"
#include "src/tint/lang/core/ir/constant.h"
#include "src/tint/lang/core/ir/function.h"
#include "src/tint/lang/core/ir/instruction.h"
#include "src/tint/lang/core/ir/value.h"
#include "src/tint/lang/core/type/manager.h"
#include "src/tint/utils/containers/vector.h"
#include "src/tint/utils/diagnostic/source.h"
#include "src/tint/utils/id/generation_id.h"
#include "src/tint/utils/memory/block_allocator.h"
#include "src/tint/utils/result/result.h"
#include "src/tint/utils/symbol/symbol_table.h"

namespace tint::core::ir {

/// Main module class for the IR.
class Module {
    /// Program Id required to create other components
    GenerationID prog_id_;

    /// Map of value to name
    Hashmap<Value*, Symbol, 32> value_to_name_;

  public:
    /// Constructor
    Module();
    /// Move constructor
    /// @param o the module to move from
    Module(Module&& o);
    /// Destructor
    ~Module();

    /// Move assign
    /// @param o the module to assign from
    /// @returns a reference to this module
    Module& operator=(Module&& o);

    /// @param inst the instruction
    /// @return the name of the given instruction, or an invalid symbol if the instruction is not
    /// named. Requires that the instruction only has a single return value.
    Symbol NameOf(Instruction* inst);

    /// @param value the value
    /// @return the name of the given value, or an invalid symbol if the value is not named.
    Symbol NameOf(Value* value);

    /// @param inst the instruction to set the name of
    /// @param name the desired name of the value. May be suffixed on collision.
    /// @note requires the instruction be a single result instruction.
    void SetName(Instruction* inst, std::string_view name);

    /// @param value the value to name.
    /// @param name the desired name of the value. May be suffixed on collision.
    void SetName(Value* value, std::string_view name);

    /// @param value the value to name
    /// @param name the desired name of the value
    void SetName(Value* value, Symbol name);

    /// @return the type manager for the module
    core::type::Manager& Types() { return constant_values.types; }

    /// The block allocator
    BlockAllocator<Block> blocks;

    /// The constant value manager
    core::constant::Manager constant_values;

    /// The instruction allocator
    BlockAllocator<Instruction> instructions;

    /// The value allocator
    BlockAllocator<Value> values;

    /// List of functions in the program
    Vector<Function*, 8> functions;

    /// The block containing module level declarations, if any exist.
    Block* root_block = nullptr;

    /// The symbol table for the module
    SymbolTable symbols{prog_id_};

    /// The map of core::constant::Value to their ir::Constant.
    Hashmap<const core::constant::Value*, ir::Constant*, 16> constants;

    /// If the module generated a validation error, will store the file for the disassembly text.
    std::unique_ptr<Source::File> disassembly_file;
};

}  // namespace tint::core::ir

#endif  // SRC_TINT_LANG_CORE_IR_MODULE_H_
