// Copyright 2023 The Tint Authors.
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

#ifndef SRC_TINT_LANG_CORE_IR_VALIDATOR_H_
#define SRC_TINT_LANG_CORE_IR_VALIDATOR_H_

#include <string>

#include "src/tint/lang/core/ir/disassembler.h"
#include "src/tint/lang/core/ir/module.h"
#include "src/tint/utils/diagnostic/diagnostic.h"
#include "src/tint/utils/result/result.h"

// Forward declarations
namespace tint::ir {
class Access;
class ExitIf;
class ExitLoop;
class ExitSwitch;
class Let;
class LoadVectorElement;
class Return;
class StoreVectorElement;
class Var;
}  // namespace tint::ir

namespace tint::ir {

/// Validates that a given IR module is correctly formed
/// @param mod the module to validate
/// @returns true on success, an error result otherwise
Result<SuccessType, diag::List> Validate(Module& mod);

/// Validates the module @p ir and dumps its contents if required by the build configuration.
/// @param ir the module to transform
/// @param msg the msg to accompany the output
/// @returns an error string if the module is not valid
Result<SuccessType, std::string> ValidateAndDumpIfNeeded(Module& ir, const char* msg);

/// The core IR validator.
class Validator {
  public:
    /// Create a core validator
    /// @param mod the module to be validated
    explicit Validator(Module& mod);

    /// Destructor
    ~Validator();

    /// Runs the validator over the module provided during construction
    /// @returns the results of validation, either a success result object or the diagnostics of
    /// validation failures.
    Result<SuccessType, diag::List> IsValid();

  protected:
    /// @param inst the instruction
    /// @param err the error message
    /// @returns a string with the instruction name name and error message formatted
    std::string InstError(Instruction* inst, std::string err);

    /// Adds an error for the @p inst and highlights the instruction in the disassembly
    /// @param inst the instruction
    /// @param err the error string
    void AddError(Instruction* inst, std::string err);

    /// Adds an error for the @p inst operand at @p idx and highlights the operand in the
    /// disassembly
    /// @param inst the instaruction
    /// @param idx the operand index
    /// @param err the error string
    void AddError(Instruction* inst, size_t idx, std::string err);

    /// Adds an error for the @p inst result at @p idx and highlgihts the result in the disassembly
    /// @param inst the instruction
    /// @param idx the result index
    /// @param err the error string
    void AddResultError(Instruction* inst, size_t idx, std::string err);

    /// Adds an error the @p block and highlights the block header in the disassembly
    /// @param blk the block
    /// @param err the error string
    void AddError(Block* blk, std::string err);

    /// Adds a note to @p inst and highlights the instruction in the disassembly
    /// @param inst the instruction
    /// @param err the message to emit
    void AddNote(Instruction* inst, std::string err);

    /// Adds a note to @p inst for operand @p idx and highlights the operand in the
    /// disassembly
    /// @param inst the instruction
    /// @param idx the operand index
    /// @param err the message string
    void AddNote(Instruction* inst, size_t idx, std::string err);

    /// Adds a note to @p blk and highlights the block in the disassembly
    /// @param blk the block
    /// @param err the message to emit
    void AddNote(Block* blk, std::string err);

    /// Adds an error to the diagnostics
    /// @param err the message to emit
    /// @param src the source lines to highlight
    void AddError(std::string err, Source src = {});

    /// Adds a note to the diagnostics
    /// @param note the note to emit
    /// @param src the source lines to highlight
    void AddNote(std::string note, Source src = {});

    /// @param v the value to get the name for
    /// @returns the name for the given value
    std::string Name(Value* v);

    /// Checks the given operand is not null
    /// @param inst the instruciton
    /// @param operand the operand
    /// @param idx the operand index
    void CheckOperandNotNull(ir::Instruction* inst, ir::Value* operand, size_t idx);

    /// Checks all operands in the given range (inclusive) for @p inst are not null
    /// @param inst the instruction
    /// @param start_operand the first operand to check
    /// @param end_operand the last operand to check
    void CheckOperandsNotNull(ir::Instruction* inst, size_t start_operand, size_t end_operand);

    /// Validates the root block
    /// @param blk the block
    void CheckRootBlock(Block* blk);

    /// Validates the given function
    /// @param func the function validate
    void CheckFunction(Function* func);

    /// Validates the given block
    /// @param blk the block to validate
    void CheckBlock(Block* blk);

    /// Validates the given instruction
    /// @param inst the instruction to validate
    void CheckInstruction(Instruction* inst);

    /// Validates the given var
    /// @param var the var to validate
    void CheckVar(Var* var);

    /// Validates the given let
    /// @param let the let to validate
    void CheckLet(Let* let);

    /// Validates the given call
    /// @param call the call to validate
    void CheckCall(Call* call);

    /// Validates the given access
    /// @param a the access to validate
    void CheckAccess(ir::Access* a);

    /// Validates the given binary
    /// @param b the binary to validate
    void CheckBinary(ir::Binary* b);

    /// Validates the given unary
    /// @param u the unary to validate
    void CheckUnary(ir::Unary* u);

    /// Validates the given if
    /// @param if_ the if to validate
    void CheckIf(If* if_);

    /// Validates the given loop
    /// @param l the loop to validate
    void CheckLoop(Loop* l);

    /// Validates the given switch
    /// @param s the switch to validate
    void CheckSwitch(Switch* s);

    /// Validates the given terminator
    /// @param b the terminator to validate
    void CheckTerminator(ir::Terminator* b);

    /// Validates the given exit
    /// @param e the exit to validate
    void CheckExit(ir::Exit* e);

    /// Validates the given exit if
    /// @param e the exit if to validate
    void CheckExitIf(ExitIf* e);

    /// Validates the given return
    /// @param r the return to validate
    void CheckReturn(Return* r);

    /// Validates the @p exit targets a valid @p control instruction where the instruction may jump
    /// over if control instructions.
    /// @param exit the exit to validate
    /// @param control the control instruction targeted
    void CheckControlsAllowingIf(Exit* exit, Instruction* control);

    /// Validates the given exit switch
    /// @param s the exit switch to validate
    void CheckExitSwitch(ExitSwitch* s);

    /// Validates the given exit loop
    /// @param l the exit loop to validate
    void CheckExitLoop(ExitLoop* l);

    /// Validates the given load vector element
    /// @param l the load vector element to validate
    void CheckLoadVectorElement(LoadVectorElement* l);

    /// Validates the given store vector element
    /// @param s the store vector element to validate
    void CheckStoreVectorElement(StoreVectorElement* s);

    /// @param inst the instruction
    /// @param idx the operand index
    /// @returns the vector pointer type for the given instruction operand
    const type::Type* GetVectorPtrElementType(Instruction* inst, size_t idx);

  private:
    Module& mod_;
    diag::List diagnostics_;
    Disassembler dis_{mod_};

    Block* current_block_ = nullptr;
    Hashset<Function*, 4> seen_functions_;
    Vector<ControlInstruction*, 8> control_stack_;

    void DisassembleIfNeeded();
};

}  // namespace tint::ir

#endif  // SRC_TINT_LANG_CORE_IR_VALIDATOR_H_
