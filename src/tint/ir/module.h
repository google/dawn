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

#ifndef SRC_TINT_IR_MODULE_H_
#define SRC_TINT_IR_MODULE_H_

#include <string>

#include "src/tint/constant/value.h"
#include "src/tint/ir/function.h"
#include "src/tint/ir/instruction.h"
#include "src/tint/ir/value.h"
#include "src/tint/program_id.h"
#include "src/tint/symbol_table.h"
#include "src/tint/type/manager.h"
#include "src/tint/utils/block_allocator.h"
#include "src/tint/utils/result.h"
#include "src/tint/utils/vector.h"

// Forward Declarations
namespace tint {
class Program;
}  // namespace tint

namespace tint::ir {

/// Main module class for the IR.
class Module {
  public:
    /// The result type for the FromProgram method.
    using Result = utils::Result<Module, std::string>;

    /// Builds an ir::Module from the given Program
    /// @param program the Program to use.
    /// @returns the `utiils::Result` of generating the IR. The result will contain the `ir::Module`
    /// on success, otherwise the `std::string` error.
    ///
    /// @note this assumes the program |IsValid|, and has had const-eval done so
    /// any abstract values have been calculated and converted into the relevant
    /// concrete types.
    static Result FromProgram(const Program* program);

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

    /// Converts the module back to a Program
    /// @returns the resulting program, or nullptr on error
    ///  (Note, this will probably turn into a utils::Result, just stubbing for now)
    const Program* ToProgram() const;

  private:
    /// Program Id required to create other components
    ProgramID prog_id_;

  public:
    /// The flow node allocator
    utils::BlockAllocator<FlowNode> flow_nodes;
    /// The constant allocator
    utils::BlockAllocator<constant::Value> constants;
    /// The value allocator
    utils::BlockAllocator<Value> values;
    /// The instruction allocator
    utils::BlockAllocator<Instruction> instructions;

    /// List of functions in the program
    utils::Vector<Function*, 8> functions;
    /// List of indexes into the functions list for the entry points
    utils::Vector<Function*, 8> entry_points;

    /// The type manager for the module
    type::Manager types;

    /// The symbol table for the module
    SymbolTable symbols{prog_id_};
};

}  // namespace tint::ir

#endif  // SRC_TINT_IR_MODULE_H_
