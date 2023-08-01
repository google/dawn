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

namespace tint::ir {

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
    type::Manager& Types() { return constant_values.types; }

    /// The block allocator
    BlockAllocator<Block> blocks;

    /// The constant value manager
    constant::Manager constant_values;

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

    /// The map of constant::Value to their ir::Constant.
    Hashmap<const constant::Value*, ir::Constant*, 16> constants;

    /// If the module generated a validation error, will store the file for the disassembly text.
    std::unique_ptr<Source::File> disassembly_file;
};

}  // namespace tint::ir

#endif  // SRC_TINT_LANG_CORE_IR_MODULE_H_
