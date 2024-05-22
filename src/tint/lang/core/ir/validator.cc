// Copyright 2023 The Dawn & Tint Authors
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

#include "src/tint/lang/core/ir/validator.h"

#include <algorithm>
#include <cstdint>
#include <memory>
#include <string>
#include <string_view>
#include <utility>

#include "src/tint/lang/core/intrinsic/table.h"
#include "src/tint/lang/core/ir/access.h"
#include "src/tint/lang/core/ir/binary.h"
#include "src/tint/lang/core/ir/bitcast.h"
#include "src/tint/lang/core/ir/block_param.h"
#include "src/tint/lang/core/ir/break_if.h"
#include "src/tint/lang/core/ir/constant.h"
#include "src/tint/lang/core/ir/construct.h"
#include "src/tint/lang/core/ir/continue.h"
#include "src/tint/lang/core/ir/control_instruction.h"
#include "src/tint/lang/core/ir/convert.h"
#include "src/tint/lang/core/ir/core_builtin_call.h"
#include "src/tint/lang/core/ir/disassembly.h"
#include "src/tint/lang/core/ir/discard.h"
#include "src/tint/lang/core/ir/exit_if.h"
#include "src/tint/lang/core/ir/exit_loop.h"
#include "src/tint/lang/core/ir/exit_switch.h"
#include "src/tint/lang/core/ir/function.h"
#include "src/tint/lang/core/ir/function_param.h"
#include "src/tint/lang/core/ir/if.h"
#include "src/tint/lang/core/ir/instruction.h"
#include "src/tint/lang/core/ir/instruction_result.h"
#include "src/tint/lang/core/ir/let.h"
#include "src/tint/lang/core/ir/load.h"
#include "src/tint/lang/core/ir/load_vector_element.h"
#include "src/tint/lang/core/ir/loop.h"
#include "src/tint/lang/core/ir/multi_in_block.h"
#include "src/tint/lang/core/ir/next_iteration.h"
#include "src/tint/lang/core/ir/return.h"
#include "src/tint/lang/core/ir/store.h"
#include "src/tint/lang/core/ir/store_vector_element.h"
#include "src/tint/lang/core/ir/switch.h"
#include "src/tint/lang/core/ir/swizzle.h"
#include "src/tint/lang/core/ir/terminate_invocation.h"
#include "src/tint/lang/core/ir/unary.h"
#include "src/tint/lang/core/ir/unreachable.h"
#include "src/tint/lang/core/ir/user_call.h"
#include "src/tint/lang/core/ir/var.h"
#include "src/tint/lang/core/type/bool.h"
#include "src/tint/lang/core/type/memory_view.h"
#include "src/tint/lang/core/type/pointer.h"
#include "src/tint/lang/core/type/reference.h"
#include "src/tint/lang/core/type/type.h"
#include "src/tint/lang/core/type/vector.h"
#include "src/tint/lang/core/type/void.h"
#include "src/tint/utils/containers/hashset.h"
#include "src/tint/utils/containers/predicates.h"
#include "src/tint/utils/containers/reverse.h"
#include "src/tint/utils/containers/transform.h"
#include "src/tint/utils/diagnostic/diagnostic.h"
#include "src/tint/utils/ice/ice.h"
#include "src/tint/utils/macros/defer.h"
#include "src/tint/utils/result/result.h"
#include "src/tint/utils/rtti/castable.h"
#include "src/tint/utils/rtti/switch.h"
#include "src/tint/utils/text/styled_text.h"
#include "src/tint/utils/text/text_style.h"

/// If set to 1 then the Tint will dump the IR when validating.
#define TINT_DUMP_IR_WHEN_VALIDATING 0
#if TINT_DUMP_IR_WHEN_VALIDATING
#include <iostream>
#include "src/tint/utils/text/styled_text_printer.h"
#endif

using namespace tint::core::fluent_types;  // NOLINT

namespace tint::core::ir {

namespace {

/// @returns the parent block of @p block
const Block* ParentBlockOf(const Block* block) {
    if (auto* parent = block->Parent()) {
        return parent->Block();
    }
    return nullptr;
}

/// @returns true if @p block directly or transitively holds the instruction @p inst
bool TransitivelyHolds(const Block* block, const Instruction* inst) {
    for (auto* b = inst->Block(); b; b = ParentBlockOf(b)) {
        if (b == block) {
            return true;
        }
    }
    return false;
}

/// @returns true if the type @p type is of, or indirectly references a type of type `T`.
template <typename T>
bool HoldsType(const type::Type* type) {
    if (!type) {
        return false;
    }
    Vector<const type::Type*, 8> stack{type};
    Hashset<const type::Type*, 8> seen{type};
    while (!stack.IsEmpty()) {
        auto* ty = stack.Pop();
        if (ty->Is<T>()) {
            return true;
        }

        if (auto* view = ty->As<type::MemoryView>(); view && seen.Add(view)) {
            stack.Push(view);
            continue;
        }

        auto type_count = ty->Elements();
        if (type_count.type && seen.Add(type_count.type)) {
            stack.Push(type_count.type);
            continue;
        }

        for (uint32_t i = 0; i < type_count.count; i++) {
            if (auto* subtype = ty->Element(i); subtype && seen.Add(subtype)) {
                stack.Push(subtype);
            }
        }
    }
    return false;
}

/// The core IR validator.
class Validator {
  public:
    /// Create a core validator
    /// @param mod the module to be validated
    /// @param capabilities the optional capabilities that are allowed
    explicit Validator(const Module& mod, Capabilities capabilities);

    /// Destructor
    ~Validator();

    /// Runs the validator over the module provided during construction
    /// @returns success or failure
    Result<SuccessType> Run();

  private:
    /// @returns the IR disassembly, performing a disassemble if this is the first call.
    ir::Disassembly& Disassembly();

    /// Adds an error for the @p inst and highlights the instruction in the disassembly
    /// @param inst the instruction
    /// @returns the diagnostic
    diag::Diagnostic& AddError(const Instruction* inst);

    /// Adds an error for the @p inst operand at @p idx and highlights the operand in the
    /// disassembly
    /// @param inst the instruction
    /// @param idx the operand index
    /// @returns the diagnostic
    diag::Diagnostic& AddError(const Instruction* inst, size_t idx);

    /// Adds an error for the @p inst result at @p idx and highlgihts the result in the disassembly
    /// @param inst the instruction
    /// @param idx the result index
    /// @returns the diagnostic
    diag::Diagnostic& AddResultError(const Instruction* inst, size_t idx);

    /// Adds an error for the @p block and highlights the block header in the disassembly
    /// @param blk the block
    /// @returns the diagnostic
    diag::Diagnostic& AddError(const Block* blk);

    /// Adds an error for the @p param and highlights the parameter in the disassembly
    /// @param param the parameter
    /// @returns the diagnostic
    diag::Diagnostic& AddError(const BlockParam* param);

    /// Adds an error for the @p func and highlights the function in the disassembly
    /// @param func the function
    /// @returns the diagnostic
    diag::Diagnostic& AddError(const Function* func);

    /// Adds an error for the @p param and highlights the parameter in the disassembly
    /// @param param the parameter
    /// @returns the diagnostic
    diag::Diagnostic& AddError(const FunctionParam* param);

    /// Adds an error the @p block and highlights the block header in the disassembly
    /// @param src the source lines to highlight
    /// @returns the diagnostic
    diag::Diagnostic& AddError(Source src);

    /// Adds a note to @p inst and highlights the instruction in the disassembly
    /// @param inst the instruction
    diag::Diagnostic& AddNote(const Instruction* inst);

    /// Adds a note to @p func and highlights the function in the disassembly
    /// @param func the function
    diag::Diagnostic& AddNote(const Function* func);

    /// Adds a note to @p inst for operand @p idx and highlights the operand in the disassembly
    /// @param inst the instruction
    /// @param idx the operand index
    diag::Diagnostic& AddOperandNote(const Instruction* inst, size_t idx);

    /// Adds a note to @p inst for result @p idx and highlights the result in the disassembly
    /// @param inst the instruction
    /// @param idx the result index
    diag::Diagnostic& AddResultNote(const Instruction* inst, size_t idx);

    /// Adds a note to @p blk and highlights the block in the disassembly
    /// @param blk the block
    diag::Diagnostic& AddNote(const Block* blk);

    /// Adds a note to the diagnostics
    /// @param src the source lines to highlight
    diag::Diagnostic& AddNote(Source src = {});

    /// Adds a note to the diagnostics highlighting where the value instruction or block is
    /// declared, if it has a source location.
    /// @param decl the value instruction or block
    void AddDeclarationNote(const CastableBase* decl);

    /// Adds a note to the diagnostics highlighting where the block is declared, if it has a source
    /// location.
    /// @param block the block
    void AddDeclarationNote(const Block* block);

    /// Adds a note to the diagnostics highlighting where the block parameter is declared, if it
    /// has a source location.
    /// @param param the block parameter
    void AddDeclarationNote(const BlockParam* param);

    /// Adds a note to the diagnostics highlighting where the function is declared, if it has a
    /// source location.
    /// @param fn the function
    void AddDeclarationNote(const Function* fn);

    /// Adds a note to the diagnostics highlighting where the function parameter is declared, if it
    /// has a source location.
    /// @param param the function parameter
    void AddDeclarationNote(const FunctionParam* param);

    /// Adds a note to the diagnostics highlighting where the instruction is declared, if it has a
    /// source location.
    /// @param inst the inst
    void AddDeclarationNote(const Instruction* inst);

    /// Adds a note to the diagnostics highlighting where instruction result was declared, if it has
    /// a source location.
    /// @param res the res
    void AddDeclarationNote(const InstructionResult* res);

    /// @param decl the value, instruction or block to get the name for
    /// @returns the styled name for the given value, instruction or block
    StyledText NameOf(const CastableBase* decl);

    /// @param v the value to get the name for
    /// @returns the styled name for the given value
    StyledText NameOf(const Value* v);

    /// @param inst the instruction to get the name for
    /// @returns the styled  name for the given instruction
    StyledText NameOf(const Instruction* inst);

    /// @param block the block to get the name for
    /// @returns the styled  name for the given block
    StyledText NameOf(const Block* block);

    /// Checks the given operand is not null
    /// @param inst the instruction
    /// @param operand the operand
    /// @param idx the operand index
    void CheckOperandNotNull(const ir::Instruction* inst, const ir::Value* operand, size_t idx);

    /// Checks all operands in the given range (inclusive) for @p inst are not null
    /// @param inst the instruction
    /// @param start_operand the first operand to check
    /// @param end_operand the last operand to check
    void CheckOperandsNotNull(const ir::Instruction* inst,
                              size_t start_operand,
                              size_t end_operand);

    /// Validates the root block
    /// @param blk the block
    void CheckRootBlock(const Block* blk);

    /// Validates the given function
    /// @param func the function validate
    void CheckFunction(const Function* func);

    /// Validates the given instruction
    /// @param inst the instruction to validate
    void CheckInstruction(const Instruction* inst);

    /// Validates the given var
    /// @param var the var to validate
    void CheckVar(const Var* var);

    /// Validates the given let
    /// @param let the let to validate
    void CheckLet(const Let* let);

    /// Validates the given call
    /// @param call the call to validate
    void CheckCall(const Call* call);

    /// Validates the given builtin call
    /// @param call the call to validate
    void CheckBuiltinCall(const BuiltinCall* call);

    /// Validates the given user call
    /// @param call the call to validate
    void CheckUserCall(const UserCall* call);

    /// Validates the given access
    /// @param a the access to validate
    void CheckAccess(const Access* a);

    /// Validates the given binary
    /// @param b the binary to validate
    void CheckBinary(const Binary* b);

    /// Validates the given unary
    /// @param u the unary to validate
    void CheckUnary(const Unary* u);

    /// Validates the given if
    /// @param if_ the if to validate
    void CheckIf(const If* if_);

    /// Validates the given loop
    /// @param l the loop to validate
    void CheckLoop(const Loop* l);

    /// Validates the loop continuing block
    /// @param l the loop to validate
    void CheckLoopContinuing(const Loop* l);

    /// Validates the given switch
    /// @param s the switch to validate
    void CheckSwitch(const Switch* s);

    /// Validates the given terminator
    /// @param b the terminator to validate
    void CheckTerminator(const Terminator* b);

    /// Validates the break if instruction
    /// @param b the break if to validate
    void CheckBreakIf(const BreakIf* b);

    /// Validates the continue instruction
    /// @param c the continue to validate
    void CheckContinue(const Continue* c);

    /// Validates the given exit
    /// @param e the exit to validate
    void CheckExit(const Exit* e);

    /// Validates the next iteration instruction
    /// @param n the next iteration to validate
    void CheckNextIteration(const NextIteration* n);

    /// Validates the given exit if
    /// @param e the exit if to validate
    void CheckExitIf(const ExitIf* e);

    /// Validates the given return
    /// @param r the return to validate
    void CheckReturn(const Return* r);

    /// Validates the @p exit targets a valid @p control instruction where the instruction may jump
    /// over if control instructions.
    /// @param exit the exit to validate
    /// @param control the control instruction targeted
    void CheckControlsAllowingIf(const Exit* exit, const Instruction* control);

    /// Validates the given exit switch
    /// @param s the exit switch to validate
    void CheckExitSwitch(const ExitSwitch* s);

    /// Validates the given exit loop
    /// @param l the exit loop to validate
    void CheckExitLoop(const ExitLoop* l);

    /// Validates the given load
    /// @param l the load to validate
    void CheckLoad(const Load* l);

    /// Validates the given store
    /// @param s the store to validate
    void CheckStore(const Store* s);

    /// Validates the given load vector element
    /// @param l the load vector element to validate
    void CheckLoadVectorElement(const LoadVectorElement* l);

    /// Validates the given store vector element
    /// @param s the store vector element to validate
    void CheckStoreVectorElement(const StoreVectorElement* s);

    /// Validates that the number and types of the source instruction operands match the target's
    /// values.
    /// @param source_inst the source instruction
    /// @param source_operand_offset the index of the first operand of the source instruction
    /// @param source_operand_count the number of operands of the source instruction
    /// @param target the receiver of the operand values
    /// @param target_values the receiver of the operand values
    void CheckOperandsMatchTarget(const Instruction* source_inst,
                                  size_t source_operand_offset,
                                  size_t source_operand_count,
                                  const CastableBase* target,
                                  VectorRef<const Value*> target_values);

    /// @param inst the instruction
    /// @param idx the operand index
    /// @returns the vector pointer type for the given instruction operand
    const core::type::Type* GetVectorPtrElementType(const Instruction* inst, size_t idx);

    /// Executes all the pending tasks
    void ProcessTasks();

    /// Queues the block to be validated with ProcessTasks()
    /// @param blk the block to validate
    void QueueBlock(const Block* blk);

    /// Queues the list of instructions starting with @p inst to be validated
    /// @param inst the first instruction
    void QueueInstructions(const Instruction* inst);

    /// Begins validation of the block @p blk, and its instructions.
    /// BeginBlock() pushes a new scope for values.
    /// Must be paired with a call to EndBlock().
    void BeginBlock(const Block* blk);

    /// Ends validation of the block opened with BeginBlock() and closes the block's scope for
    /// values.
    void EndBlock();

    /// ScopeStack holds a stack of values that are currently in scope
    struct ScopeStack {
        void Push() { stack_.Push({}); }
        void Pop() { stack_.Pop(); }
        void Add(const Value* value) { stack_.Back().Add(value); }
        bool Contains(const Value* value) {
            return stack_.Any([&](auto& v) { return v.Contains(value); });
        }
        bool IsEmpty() const { return stack_.IsEmpty(); }

      private:
        Vector<Hashset<const Value*, 8>, 4> stack_;
    };

    const Module& mod_;
    Capabilities capabilities_;
    std::optional<ir::Disassembly> disassembly_;  // Use Disassembly()
    diag::List diagnostics_;
    Hashset<const Function*, 4> all_functions_;
    Hashset<const Instruction*, 4> visited_instructions_;
    Hashmap<const Loop*, const Continue*, 4> first_continues_;
    Vector<const ControlInstruction*, 8> control_stack_;
    Vector<const Block*, 8> block_stack_;
    ScopeStack scope_stack_;
    Vector<std::function<void()>, 16> tasks_;
    SymbolTable symbols_ = SymbolTable::Wrap(mod_.symbols);
    type::Manager type_mgr_ = type::Manager::Wrap(mod_.Types());
};

Validator::Validator(const Module& mod, Capabilities capabilities)
    : mod_(mod), capabilities_(capabilities) {}

Validator::~Validator() = default;

Disassembly& Validator::Disassembly() {
    if (!disassembly_) {
        disassembly_.emplace(Disassemble(mod_));
    }
    return *disassembly_;
}

Result<SuccessType> Validator::Run() {
    scope_stack_.Push();
    TINT_DEFER({
        scope_stack_.Pop();
        TINT_ASSERT(scope_stack_.IsEmpty());
        TINT_ASSERT(tasks_.IsEmpty());
        TINT_ASSERT(control_stack_.IsEmpty());
        TINT_ASSERT(block_stack_.IsEmpty());
    });
    CheckRootBlock(mod_.root_block);

    for (auto& func : mod_.functions) {
        if (!all_functions_.Add(func.Get())) {
            AddError(func) << "function " << NameOf(func.Get())
                           << " added to module multiple times";
        }
        scope_stack_.Add(func);
    }

    for (auto& func : mod_.functions) {
        CheckFunction(func);
    }

    if (!diagnostics_.ContainsErrors()) {
        // Check for orphaned instructions.
        for (auto* inst : mod_.Instructions()) {
            if (!visited_instructions_.Contains(inst)) {
                AddError(inst) << "orphaned instruction: " << inst->FriendlyName();
            }
        }
    }

    if (diagnostics_.ContainsErrors()) {
        diagnostics_.AddNote(Source{}) << "# Disassembly\n" << Disassembly().Text();
        return Failure{std::move(diagnostics_)};
    }
    return Success;
}

diag::Diagnostic& Validator::AddError(const Instruction* inst) {
    diagnostics_.ReserveAdditional(2);  // Ensure diagnostics don't resize alive after AddNote()
    auto src = Disassembly().InstructionSource(inst);
    auto& diag = AddError(src) << inst->FriendlyName() << ": ";

    if (!block_stack_.IsEmpty()) {
        AddNote(block_stack_.Back()) << "in block";
    }
    return diag;
}

diag::Diagnostic& Validator::AddError(const Instruction* inst, size_t idx) {
    diagnostics_.ReserveAdditional(2);  // Ensure diagnostics don't resize alive after AddNote()
    auto src =
        Disassembly().OperandSource(Disassembly::IndexedValue{inst, static_cast<uint32_t>(idx)});
    auto& diag = AddError(src) << inst->FriendlyName() << ": ";

    if (!block_stack_.IsEmpty()) {
        AddNote(block_stack_.Back()) << "in block";
    }
    return diag;
}

diag::Diagnostic& Validator::AddResultError(const Instruction* inst, size_t idx) {
    diagnostics_.ReserveAdditional(2);  // Ensure diagnostics don't resize alive after AddNote()
    auto src =
        Disassembly().ResultSource(Disassembly::IndexedValue{inst, static_cast<uint32_t>(idx)});
    auto& diag = AddError(src) << inst->FriendlyName() << ": ";

    if (!block_stack_.IsEmpty()) {
        AddNote(block_stack_.Back()) << "in block";
    }
    return diag;
}

diag::Diagnostic& Validator::AddError(const Block* blk) {
    auto src = Disassembly().BlockSource(blk);
    return AddError(src);
}

diag::Diagnostic& Validator::AddError(const BlockParam* param) {
    auto src = Disassembly().BlockParamSource(param);
    return AddError(src);
}

diag::Diagnostic& Validator::AddError(const Function* func) {
    auto src = Disassembly().FunctionSource(func);
    return AddError(src);
}

diag::Diagnostic& Validator::AddError(const FunctionParam* param) {
    auto src = Disassembly().FunctionParamSource(param);
    return AddError(src);
}

diag::Diagnostic& Validator::AddNote(const Instruction* inst) {
    auto src = Disassembly().InstructionSource(inst);
    return AddNote(src);
}

diag::Diagnostic& Validator::AddNote(const Function* func) {
    auto src = Disassembly().FunctionSource(func);
    return AddNote(src);
}

diag::Diagnostic& Validator::AddOperandNote(const Instruction* inst, size_t idx) {
    auto src =
        Disassembly().OperandSource(Disassembly::IndexedValue{inst, static_cast<uint32_t>(idx)});
    return AddNote(src);
}

diag::Diagnostic& Validator::AddResultNote(const Instruction* inst, size_t idx) {
    auto src =
        Disassembly().ResultSource(Disassembly::IndexedValue{inst, static_cast<uint32_t>(idx)});
    return AddNote(src);
}

diag::Diagnostic& Validator::AddNote(const Block* blk) {
    auto src = Disassembly().BlockSource(blk);
    return AddNote(src);
}

diag::Diagnostic& Validator::AddError(Source src) {
    auto& diag = diagnostics_.AddError(src);
    diag.owned_file = Disassembly().File();
    return diag;
}

diag::Diagnostic& Validator::AddNote(Source src) {
    auto& diag = diagnostics_.AddNote(src);
    diag.owned_file = Disassembly().File();
    return diag;
}

void Validator::AddDeclarationNote(const CastableBase* decl) {
    tint::Switch(
        decl,  //
        [&](const Block* block) { AddDeclarationNote(block); },
        [&](const BlockParam* param) { AddDeclarationNote(param); },
        [&](const Function* fn) { AddDeclarationNote(fn); },
        [&](const FunctionParam* param) { AddDeclarationNote(param); },
        [&](const Instruction* inst) { AddDeclarationNote(inst); },
        [&](const InstructionResult* res) { AddDeclarationNote(res); });
}

void Validator::AddDeclarationNote(const Block* block) {
    auto src = Disassembly().BlockSource(block);
    if (src.file) {
        AddNote(src) << NameOf(block) << " declared here";
    }
}

void Validator::AddDeclarationNote(const BlockParam* param) {
    auto src = Disassembly().BlockParamSource(param);
    if (src.file) {
        AddNote(src) << NameOf(param) << " declared here";
    }
}

void Validator::AddDeclarationNote(const Function* fn) {
    AddNote(fn) << NameOf(fn) << " declared here";
}

void Validator::AddDeclarationNote(const FunctionParam* param) {
    auto src = Disassembly().FunctionParamSource(param);
    if (src.file) {
        AddNote(src) << NameOf(param) << " declared here";
    }
}

void Validator::AddDeclarationNote(const Instruction* inst) {
    auto src = Disassembly().InstructionSource(inst);
    if (src.file) {
        AddNote(src) << NameOf(inst) << " declared here";
    }
}

void Validator::AddDeclarationNote(const InstructionResult* res) {
    if (auto* inst = res->Instruction()) {
        auto results = inst->Results();
        for (size_t i = 0; i < results.Length(); i++) {
            if (results[i] == res) {
                AddResultNote(res->Instruction(), i) << NameOf(res) << " declared here";
                return;
            }
        }
    }
}

StyledText Validator::NameOf(const CastableBase* decl) {
    return tint::Switch(
        decl,  //
        [&](const Value* value) { return NameOf(value); },
        [&](const Instruction* inst) { return NameOf(inst); },
        [&](const Block* block) { return NameOf(block); },  //
        TINT_ICE_ON_NO_MATCH);
}

StyledText Validator::NameOf(const Value* value) {
    return Disassembly().NameOf(value);
}

StyledText Validator::NameOf(const Instruction* inst) {
    return StyledText{} << style::Instruction(inst->FriendlyName());
}

StyledText Validator::NameOf(const Block* block) {
    return StyledText{} << style::Instruction(block->Parent()->FriendlyName()) << " block "
                        << Disassembly().NameOf(block);
}

void Validator::CheckOperandNotNull(const Instruction* inst, const ir::Value* operand, size_t idx) {
    if (operand == nullptr) {
        AddError(inst, idx) << "operand is undefined";
    }
}

void Validator::CheckOperandsNotNull(const Instruction* inst,
                                     size_t start_operand,
                                     size_t end_operand) {
    auto operands = inst->Operands();
    for (size_t i = start_operand; i <= end_operand; i++) {
        CheckOperandNotNull(inst, operands[i], i);
    }
}

void Validator::CheckRootBlock(const Block* blk) {
    block_stack_.Push(blk);
    TINT_DEFER(block_stack_.Pop());

    for (auto* inst : *blk) {
        if (inst->Block() != blk) {
            AddError(inst) << "instruction in root block does not have root block as parent";
            continue;
        }
        auto* var = inst->As<ir::Var>();
        if (!var) {
            AddError(inst) << "root block: invalid instruction: " << inst->TypeInfo().name;
            continue;
        }
        CheckInstruction(var);
    }
}

void Validator::CheckFunction(const Function* func) {
    // Scope holds the parameters and block
    scope_stack_.Push();
    TINT_DEFER(scope_stack_.Pop());

    for (auto* param : func->Params()) {
        if (!param->Alive()) {
            AddError(param) << "destroyed parameter found in function parameter list";
            return;
        }
        if (!param->Function()) {
            AddError(param) << "function parameter has nullptr parent function";
            return;
        } else if (param->Function() != func) {
            AddError(param) << "function parameter has incorrect parent function";
            AddNote(param->Function()) << "parent function declared here";
            return;
        }

        // References not allowed on function signatures even with Capability::kAllowRefTypes
        if (HoldsType<type::Reference>(param->Type())) {
            AddError(param) << "references are not permitted as parameter types";
        }

        scope_stack_.Add(param);
    }
    if (HoldsType<type::Reference>(func->ReturnType())) {
        AddError(func) << "references are not permitted as return types";
    }

    QueueBlock(func->Block());
    ProcessTasks();
}

void Validator::ProcessTasks() {
    while (!tasks_.IsEmpty()) {
        tasks_.Pop()();
    }
}

void Validator::QueueBlock(const Block* blk) {
    tasks_.Push([this] { EndBlock(); });
    tasks_.Push([this, blk] { BeginBlock(blk); });
}

void Validator::BeginBlock(const Block* blk) {
    scope_stack_.Push();
    block_stack_.Push(blk);

    if (auto* mb = blk->As<MultiInBlock>()) {
        for (auto* param : mb->Params()) {
            if (!param->Alive()) {
                AddError(param) << "destroyed parameter found in block parameter list";
                return;
            }
            if (!param->Block()) {
                AddError(param) << "block parameter has nullptr parent block";
                return;
            } else if (param->Block() != mb) {
                AddError(param) << "block parameter has incorrect parent block";
                AddNote(param->Block()) << "parent block declared here";
                return;
            }
            scope_stack_.Add(param);
        }
    }

    if (!blk->Terminator()) {
        AddError(blk) << "block does not end in a terminator instruction";
    }

    // Validate the instructions w.r.t. the parent block
    for (auto* inst : *blk) {
        if (inst->Block() != blk) {
            AddError(inst) << "block instruction does not have same block as parent";
            AddNote(blk) << "in block";
        }
    }

    // Enqueue validation of the instructions of the block
    if (!blk->IsEmpty()) {
        QueueInstructions(blk->Instructions());
    }
}

void Validator::EndBlock() {
    scope_stack_.Pop();
    block_stack_.Pop();
}

void Validator::QueueInstructions(const Instruction* inst) {
    tasks_.Push([this, inst] {
        CheckInstruction(inst);
        if (inst->next) {
            QueueInstructions(inst->next);
        }
    });
}

void Validator::CheckInstruction(const Instruction* inst) {
    visited_instructions_.Add(inst);
    if (!inst->Alive()) {
        AddError(inst) << "destroyed instruction found in instruction list";
        return;
    }
    auto results = inst->Results();
    for (size_t i = 0; i < results.Length(); ++i) {
        auto* res = results[i];
        if (!res) {
            AddResultError(inst, i) << "result is undefined";
            continue;
        }

        if (res->Instruction() == nullptr) {
            AddResultError(inst, i) << "instruction of result is undefined";
        } else if (res->Instruction() != inst) {
            AddResultError(inst, i) << "instruction of result is a different instruction";
        }

        if (!capabilities_.Contains(Capability::kAllowRefTypes)) {
            if (HoldsType<type::Reference>(res->Type())) {
                AddResultError(inst, i) << "reference type is not permitted";
            }
        }
    }

    auto ops = inst->Operands();
    for (size_t i = 0; i < ops.Length(); ++i) {
        auto* op = ops[i];
        if (!op) {
            continue;
        }

        // Note, a `nullptr` is a valid operand in some cases, like `var` so we can't just check
        // for `nullptr` here.
        if (!op->Alive()) {
            AddError(inst, i) << "operand is not alive";
        } else if (!op->HasUsage(inst, i)) {
            AddError(inst, i) << "operand missing usage";
        } else if (auto fn = op->As<Function>(); fn && !all_functions_.Contains(fn)) {
            AddError(inst, i) << NameOf(op) << " is not part of the module";
        } else if (!op->Is<Constant>() && !scope_stack_.Contains(op)) {
            AddError(inst, i) << NameOf(op) << " is not in scope";
            AddDeclarationNote(op);
        }

        if (!capabilities_.Contains(Capability::kAllowRefTypes)) {
            if (HoldsType<type::Reference>(op->Type())) {
                AddError(inst, i) << "reference type is not permitted";
            }
        }
    }

    tint::Switch(
        inst,                                                              //
        [&](const Access* a) { CheckAccess(a); },                          //
        [&](const Binary* b) { CheckBinary(b); },                          //
        [&](const Call* c) { CheckCall(c); },                              //
        [&](const If* if_) { CheckIf(if_); },                              //
        [&](const Let* let) { CheckLet(let); },                            //
        [&](const Load* load) { CheckLoad(load); },                        //
        [&](const LoadVectorElement* l) { CheckLoadVectorElement(l); },    //
        [&](const Loop* l) { CheckLoop(l); },                              //
        [&](const Store* s) { CheckStore(s); },                            //
        [&](const StoreVectorElement* s) { CheckStoreVectorElement(s); },  //
        [&](const Switch* s) { CheckSwitch(s); },                          //
        [&](const Swizzle*) {},                                            //
        [&](const Terminator* b) { CheckTerminator(b); },                  //
        [&](const Unary* u) { CheckUnary(u); },                            //
        [&](const Var* var) { CheckVar(var); },                            //
        [&](const Default) { AddError(inst) << "missing validation"; });

    for (auto* result : results) {
        scope_stack_.Add(result);
    }
}

void Validator::CheckVar(const Var* var) {
    if (var->Result(0) && var->Initializer()) {
        if (var->Initializer()->Type() != var->Result(0)->Type()->UnwrapPtrOrRef()) {
            AddError(var) << "initializer type "
                          << style::Type(var->Initializer()->Type()->FriendlyName())
                          << " does not match store type "
                          << style::Type(var->Result(0)->Type()->UnwrapPtrOrRef()->FriendlyName());
        }
    }
}

void Validator::CheckLet(const Let* let) {
    CheckOperandNotNull(let, let->Value(), Let::kValueOperandOffset);

    if (let->Result(0) && let->Value()) {
        if (let->Result(0)->Type() != let->Value()->Type()) {
            AddError(let) << "result type " << style::Type(let->Result(0)->Type()->FriendlyName())
                          << " does not match value type "
                          << style::Type(let->Value()->Type()->FriendlyName());
        }
    }
}

void Validator::CheckCall(const Call* call) {
    tint::Switch(
        call,                                                //
        [&](const Bitcast*) {},                              //
        [&](const BuiltinCall* c) { CheckBuiltinCall(c); },  //
        [&](const Construct*) {},                            //
        [&](const Convert*) {},                              //
        [&](const Discard*) {},                              //
        [&](const UserCall* c) { CheckUserCall(c); },        //
        [&](Default) {
            // Validation of custom IR instructions
        });
}

void Validator::CheckBuiltinCall(const BuiltinCall* call) {
    auto args = Transform<8>(call->Args(), [&](const ir::Value* v) { return v->Type(); });
    intrinsic::Context context{
        call->TableData(),
        type_mgr_,
        symbols_,
    };

    auto result = core::intrinsic::LookupFn(context, call->FriendlyName().c_str(), call->FuncId(),
                                            Empty, args, core::EvaluationStage::kRuntime);
    if (result != Success) {
        AddError(call) << result.Failure();
        return;
    }

    if (result->return_type != call->Result(0)->Type()) {
        AddError(call) << "call result type does not match builtin return type";
    }
}

void Validator::CheckUserCall(const UserCall* call) {
    if (call->Target()->Stage() != Function::PipelineStage::kUndefined) {
        AddError(call, UserCall::kFunctionOperandOffset)
            << "call target must not have a pipeline stage";
    }

    auto args = call->Args();
    auto params = call->Target()->Params();
    if (args.Length() != params.Length()) {
        AddError(call, UserCall::kFunctionOperandOffset)
            << "function has " << params.Length() << " parameters, but call provides "
            << args.Length() << " arguments";
        return;
    }

    for (size_t i = 0; i < args.Length(); i++) {
        if (args[i]->Type() != params[i]->Type()) {
            AddError(call, UserCall::kArgsOperandOffset + i)
                << "function parameter " << i << " is of type "
                << style::Type(params[i]->Type()->FriendlyName()) << ", but argument is of type "
                << style::Type(args[i]->Type()->FriendlyName());
        }
    }
}

void Validator::CheckAccess(const Access* a) {
    auto* obj_view = a->Object()->Type()->As<core::type::MemoryView>();
    auto* ty = obj_view ? obj_view->StoreType() : a->Object()->Type();
    enum Kind { kPtr, kRef, kValue };
    auto kind_of = [&](const core::type::Type* type) {
        return tint::Switch(
            type,                                                //
            [&](const core::type::Pointer*) { return kPtr; },    //
            [&](const core::type::Reference*) { return kRef; },  //
            [&](Default) { return kValue; });
    };
    const Kind in_kind = kind_of(a->Object()->Type());
    auto desc_of = [&](Kind kind, const core::type::Type* type) {
        switch (kind) {
            case kPtr:
                return StyledText{}
                       << style::Type("ptr<", obj_view->AddressSpace(), ", ", type->FriendlyName(),
                                      ", ", obj_view->Access(), ">");
            case kRef:
                return StyledText{}
                       << style::Type("ref<", obj_view->AddressSpace(), ", ", type->FriendlyName(),
                                      ", ", obj_view->Access(), ">");
            default:
                return StyledText{} << style::Type(type->FriendlyName());
        }
    };

    for (size_t i = 0; i < a->Indices().Length(); i++) {
        auto err = [&]() -> diag::Diagnostic& {
            return AddError(a, i + Access::kIndicesOperandOffset);
        };
        auto note = [&]() -> diag::Diagnostic& {
            return AddOperandNote(a, i + Access::kIndicesOperandOffset);
        };

        auto* index = a->Indices()[i];
        if (TINT_UNLIKELY(!index->Type()->is_integer_scalar())) {
            err() << "index must be integer, got " << index->Type()->FriendlyName();
            return;
        }

        if (!capabilities_.Contains(Capability::kAllowVectorElementPointer)) {
            if (in_kind != kValue && ty->Is<core::type::Vector>()) {
                err() << "cannot obtain address of vector element";
                return;
            }
        }

        if (auto* const_index = index->As<ir::Constant>()) {
            auto* value = const_index->Value();
            if (value->Type()->is_signed_integer_scalar()) {
                // index is a signed integer scalar. Check that the index isn't negative.
                // If the index is unsigned, we can skip this.
                auto idx = value->ValueAs<AInt>();
                if (TINT_UNLIKELY(idx < 0)) {
                    err() << "constant index must be positive, got " << idx;
                    return;
                }
            }

            auto idx = value->ValueAs<uint32_t>();
            auto* el = ty->Element(idx);
            if (TINT_UNLIKELY(!el)) {
                // Is index in bounds?
                if (auto el_count = ty->Elements().count; el_count != 0 && idx >= el_count) {
                    err() << "index out of bounds for type " << desc_of(in_kind, ty);
                    note() << "acceptable range: [0.." << (el_count - 1) << "]";
                    return;
                }
                err() << "type " << desc_of(in_kind, ty) << " cannot be indexed";
                return;
            }
            ty = el;
        } else {
            auto* el = ty->Elements().type;
            if (TINT_UNLIKELY(!el)) {
                err() << "type " << desc_of(in_kind, ty) << " cannot be dynamically indexed";
                return;
            }
            ty = el;
        }
    }

    auto* want = a->Result(0)->Type();
    auto* want_view = want->As<type::MemoryView>();
    bool ok = true;
    if (obj_view) {
        // Pointer source always means pointer result.
        ok = want_view && ty == want_view->StoreType();
        if (ok) {
            // Also check that the address space and access modes match.
            ok = obj_view->Is<type::Pointer>() == want_view->Is<type::Pointer>() &&
                 obj_view->AddressSpace() == want_view->AddressSpace() &&
                 obj_view->Access() == want_view->Access();
        }
    } else {
        // Otherwise, result types should exactly match.
        ok = ty == want;
    }
    if (TINT_UNLIKELY(!ok)) {
        AddError(a) << "result of access chain is type " << desc_of(in_kind, ty)
                    << " but instruction type is " << style::Type(want->FriendlyName());
    }
}

void Validator::CheckBinary(const Binary* b) {
    CheckOperandsNotNull(b, Binary::kLhsOperandOffset, Binary::kRhsOperandOffset);
    if (b->LHS() && b->RHS()) {
        intrinsic::Context context{b->TableData(), type_mgr_, symbols_};

        auto overload =
            core::intrinsic::LookupBinary(context, b->Op(), b->LHS()->Type(), b->RHS()->Type(),
                                          core::EvaluationStage::kRuntime, /* is_compound */ false);
        if (overload != Success) {
            AddError(b) << overload.Failure();
            return;
        }

        if (auto* result = b->Result(0)) {
            if (overload->return_type != result->Type()) {
                AddError(b) << "result value type " << style::Type(result->Type()->FriendlyName())
                            << " does not match "
                            << style::Instruction(Disassembly().NameOf(b->Op())) << " result type "
                            << style::Type(overload->return_type->FriendlyName());
            }
        }
    }
}

void Validator::CheckUnary(const Unary* u) {
    CheckOperandNotNull(u, u->Val(), Unary::kValueOperandOffset);
    if (u->Val()) {
        intrinsic::Context context{u->TableData(), type_mgr_, symbols_};

        auto overload = core::intrinsic::LookupUnary(context, u->Op(), u->Val()->Type(),
                                                     core::EvaluationStage::kRuntime);
        if (overload != Success) {
            AddError(u) << overload.Failure();
            return;
        }

        if (auto* result = u->Result(0)) {
            if (overload->return_type != result->Type()) {
                AddError(u) << "result value type " << style::Type(result->Type()->FriendlyName())
                            << " does not match "
                            << style::Instruction(Disassembly().NameOf(u->Op())) << " result type "
                            << style::Type(overload->return_type->FriendlyName());
            }
        }
    }
}

void Validator::CheckIf(const If* if_) {
    CheckOperandNotNull(if_, if_->Condition(), If::kConditionOperandOffset);

    if (if_->Condition() && !if_->Condition()->Type()->Is<core::type::Bool>()) {
        AddError(if_, If::kConditionOperandOffset)
            << "condition type must be " << style::Type("bool");
    }

    tasks_.Push([this] { control_stack_.Pop(); });

    if (!if_->False()->IsEmpty()) {
        QueueBlock(if_->False());
    }

    QueueBlock(if_->True());

    tasks_.Push([this, if_] { control_stack_.Push(if_); });
}

void Validator::CheckLoop(const Loop* l) {
    // Note: Tasks are queued in reverse order of their execution
    tasks_.Push([this, l] {
        first_continues_.Remove(l);  // No need for this any more. Free memory.
        control_stack_.Pop();
    });
    if (!l->Initializer()->IsEmpty()) {
        tasks_.Push([this] { EndBlock(); });
    }
    tasks_.Push([this] { EndBlock(); });
    if (!l->Continuing()->IsEmpty()) {
        tasks_.Push([this] { EndBlock(); });
    }

    // ⎡Initializer              ⎤
    // ⎢    ⎡Body               ⎤⎥
    // ⎣    ⎣    [Continuing ]  ⎦⎦

    if (!l->Continuing()->IsEmpty()) {
        tasks_.Push([this, l] {
            CheckLoopContinuing(l);
            BeginBlock(l->Continuing());
        });
    }

    tasks_.Push([this, l] { BeginBlock(l->Body()); });
    if (!l->Initializer()->IsEmpty()) {
        tasks_.Push([this, l] { BeginBlock(l->Initializer()); });
    }
    tasks_.Push([this, l] { control_stack_.Push(l); });
}

void Validator::CheckLoopContinuing(const Loop* loop) {
    if (!loop->HasContinuing()) {
        return;
    }

    // Ensure that values used in the loop continuing are not from the loop body, after a
    // continue instruction.
    if (auto* first_continue = first_continues_.GetOr(loop, nullptr)) {
        // Find the instruction in the body block that is or holds the first continue instruction.
        const Instruction* holds_continue = first_continue;
        while (holds_continue && holds_continue->Block() &&
               holds_continue->Block() != loop->Body()) {
            holds_continue = holds_continue->Block()->Parent();
        }

        // Check that all subsequent instruction values are not used in the continuing block.
        for (auto* inst = holds_continue; inst; inst = inst->next) {
            for (auto* result : inst->Results()) {
                result->ForEachUse([&](Usage use) {
                    if (TransitivelyHolds(loop->Continuing(), use.instruction)) {
                        AddError(use.instruction, use.operand_index)
                            << NameOf(result)
                            << " cannot be used in continuing block as it is declared after the "
                               "first "
                            << style::Instruction("continue") << " in the loop's body";
                        AddDeclarationNote(result);
                        AddNote(first_continue)
                            << "loop body's first " << style::Instruction("continue");
                    }
                });
            }
        }
    }
}

void Validator::CheckSwitch(const Switch* s) {
    tasks_.Push([this] { control_stack_.Pop(); });

    for (auto& cse : s->Cases()) {
        QueueBlock(cse.block);
    }

    tasks_.Push([this, s] { control_stack_.Push(s); });
}

void Validator::CheckTerminator(const Terminator* b) {
    // Note, transforms create `undef` terminator arguments (this is done in MergeReturn and
    // DemoteToHelper) so we can't add validation.

    tint::Switch(
        b,                                                           //
        [&](const ir::BreakIf* i) { CheckBreakIf(i); },              //
        [&](const ir::Continue* c) { CheckContinue(c); },            //
        [&](const ir::Exit* e) { CheckExit(e); },                    //
        [&](const ir::NextIteration* n) { CheckNextIteration(n); },  //
        [&](const ir::Return* ret) { CheckReturn(ret); },            //
        [&](const ir::TerminateInvocation*) {},                      //
        [&](const ir::Unreachable*) {},                              //
        [&](Default) { AddError(b) << "missing validation"; });

    if (b->next) {
        AddError(b) << "must be the last instruction in the block";
    }
}

void Validator::CheckBreakIf(const BreakIf* b) {
    auto* loop = b->Loop();
    if (loop == nullptr) {
        AddError(b) << "has no associated loop";
        return;
    }

    if (loop->Continuing() != b->Block()) {
        AddError(b) << "must only be called directly from loop continuing";
    }

    auto next_iter_values = b->NextIterValues();
    if (auto* body = loop->Body()) {
        CheckOperandsMatchTarget(b, b->ArgsOperandOffset(), next_iter_values.Length(), body,
                                 body->Params());
    }

    auto exit_values = b->ExitValues();
    CheckOperandsMatchTarget(b, b->ArgsOperandOffset() + next_iter_values.Length(),
                             exit_values.Length(), loop, loop->Results());
}

void Validator::CheckContinue(const Continue* c) {
    auto* loop = c->Loop();
    if (loop == nullptr) {
        AddError(c) << "has no associated loop";
        return;
    }
    if (!TransitivelyHolds(loop->Body(), c)) {
        if (control_stack_.Any(Eq<const ControlInstruction*>(loop))) {
            AddError(c) << "must only be called from loop body";
        } else {
            AddError(c) << "called outside of associated loop";
        }
    }

    if (auto* cont = loop->Continuing()) {
        CheckOperandsMatchTarget(c, Continue::kArgsOperandOffset, c->Args().Length(), cont,
                                 cont->Params());
    }

    first_continues_.Add(loop, c);
}

void Validator::CheckExit(const Exit* e) {
    if (e->ControlInstruction() == nullptr) {
        AddError(e) << "has no parent control instruction";
        return;
    }

    if (control_stack_.IsEmpty()) {
        AddError(e) << "found outside all control instructions";
        return;
    }

    auto args = e->Args();
    CheckOperandsMatchTarget(e, e->ArgsOperandOffset(), args.Length(), e->ControlInstruction(),
                             e->ControlInstruction()->Results());

    tint::Switch(
        e,                                                     //
        [&](const ir::ExitIf* i) { CheckExitIf(i); },          //
        [&](const ir::ExitLoop* l) { CheckExitLoop(l); },      //
        [&](const ir::ExitSwitch* s) { CheckExitSwitch(s); },  //
        [&](Default) { AddError(e) << "missing validation"; });
}

void Validator::CheckNextIteration(const NextIteration* n) {
    auto* loop = n->Loop();
    if (loop == nullptr) {
        AddError(n) << "has no associated loop";
        return;
    }
    if (!TransitivelyHolds(loop->Initializer(), n) && !TransitivelyHolds(loop->Continuing(), n)) {
        if (control_stack_.Any(Eq<const ControlInstruction*>(loop))) {
            AddError(n) << "must only be called from loop initializer or continuing";
        } else {
            AddError(n) << "called outside of associated loop";
        }
    }

    if (auto* body = loop->Body()) {
        CheckOperandsMatchTarget(n, NextIteration::kArgsOperandOffset, n->Args().Length(), body,
                                 body->Params());
    }
}

void Validator::CheckExitIf(const ExitIf* e) {
    if (control_stack_.Back() != e->If()) {
        AddError(e) << "if target jumps over other control instructions";
        AddNote(control_stack_.Back()) << "first control instruction jumped";
    }
}

void Validator::CheckReturn(const Return* ret) {
    auto* func = ret->Func();
    if (func == nullptr) {
        AddError(ret) << "undefined function";
        return;
    }
    if (func->ReturnType()->Is<core::type::Void>()) {
        if (ret->Value()) {
            AddError(ret) << "unexpected return value";
        }
    } else {
        if (!ret->Value()) {
            AddError(ret) << "expected return value";
        } else if (ret->Value()->Type() != func->ReturnType()) {
            AddError(ret) << "return value type "
                          << style::Type(ret->Value()->Type()->FriendlyName())
                          << " does not match function return type "
                          << style::Type(func->ReturnType()->FriendlyName());
        }
    }
}

void Validator::CheckControlsAllowingIf(const Exit* exit, const Instruction* control) {
    bool found = false;
    for (auto ctrl : tint::Reverse(control_stack_)) {
        if (ctrl == control) {
            found = true;
            break;
        }
        // A exit switch can step over if instructions, but no others.
        if (!ctrl->Is<ir::If>()) {
            AddError(exit) << control->FriendlyName()
                           << " target jumps over other control instructions";
            AddNote(ctrl) << "first control instruction jumped";
            return;
        }
    }
    if (!found) {
        AddError(exit) << control->FriendlyName() << " not found in parent control instructions";
    }
}

void Validator::CheckExitSwitch(const ExitSwitch* s) {
    CheckControlsAllowingIf(s, s->ControlInstruction());
}

void Validator::CheckExitLoop(const ExitLoop* l) {
    CheckControlsAllowingIf(l, l->ControlInstruction());

    const Instruction* inst = l;
    const Loop* control = l->Loop();
    while (inst) {
        // Found parent loop
        if (inst->Block()->Parent() == control) {
            if (inst->Block() == control->Continuing()) {
                AddError(l) << "loop exit jumps out of continuing block";
                if (control->Continuing() != l->Block()) {
                    AddNote(control->Continuing()) << "in continuing block";
                }
            } else if (inst->Block() == control->Initializer()) {
                AddError(l) << "loop exit not permitted in loop initializer";
                if (control->Initializer() != l->Block()) {
                    AddNote(control->Initializer()) << "in initializer block";
                }
            }
            break;
        }
        inst = inst->Block()->Parent();
    }
}

void Validator::CheckLoad(const Load* l) {
    CheckOperandNotNull(l, l->From(), Load::kFromOperandOffset);

    if (auto* from = l->From()) {
        auto* mv = from->Type()->As<core::type::MemoryView>();
        if (!mv) {
            AddError(l, Load::kFromOperandOffset) << "load source operand is not a memory view";
            return;
        }
        if (l->Result(0)->Type() != mv->StoreType()) {
            AddError(l, Load::kFromOperandOffset)
                << "result type " << style::Type(l->Result(0)->Type()->FriendlyName())
                << " does not match source store type "
                << style::Type(mv->StoreType()->FriendlyName());
        }
    }
}

void Validator::CheckStore(const Store* s) {
    CheckOperandsNotNull(s, Store::kToOperandOffset, Store::kFromOperandOffset);

    if (auto* from = s->From()) {
        if (auto* to = s->To()) {
            auto* mv = to->Type()->As<core::type::MemoryView>();
            if (!mv) {
                AddError(s, Store::kFromOperandOffset)
                    << "store target operand is not a memory view";
                return;
            }
            auto* value_type = from->Type();
            auto* store_type = mv->StoreType();
            if (value_type != store_type) {
                AddError(s, Store::kFromOperandOffset)
                    << "value type " << style::Type(value_type->FriendlyName())
                    << " does not match store type " << style::Type(store_type->FriendlyName());
            }
        }
    }
}

void Validator::CheckLoadVectorElement(const LoadVectorElement* l) {
    CheckOperandsNotNull(l,  //
                         LoadVectorElement::kFromOperandOffset,
                         LoadVectorElement::kIndexOperandOffset);

    if (auto* res = l->Result(0)) {
        if (auto* el_ty = GetVectorPtrElementType(l, LoadVectorElement::kFromOperandOffset)) {
            if (res->Type() != el_ty) {
                AddResultError(l, 0) << "result type " << style::Type(res->Type()->FriendlyName())
                                     << " does not match vector pointer element type "
                                     << style::Type(el_ty->FriendlyName());
            }
        }
    }
}

void Validator::CheckStoreVectorElement(const StoreVectorElement* s) {
    CheckOperandsNotNull(s,  //
                         StoreVectorElement::kToOperandOffset,
                         StoreVectorElement::kValueOperandOffset);

    if (auto* value = s->Value()) {
        if (auto* el_ty = GetVectorPtrElementType(s, StoreVectorElement::kToOperandOffset)) {
            if (value->Type() != el_ty) {
                AddError(s, StoreVectorElement::kValueOperandOffset)
                    << "value type " << style::Type(value->Type()->FriendlyName())
                    << " does not match vector pointer element type "
                    << style::Type(el_ty->FriendlyName());
            }
        }
    }
}

void Validator::CheckOperandsMatchTarget(const Instruction* source_inst,
                                         size_t source_operand_offset,
                                         size_t source_operand_count,
                                         const CastableBase* target,
                                         VectorRef<const Value*> target_values) {
    if (source_operand_count != target_values.Length()) {
        auto values = [&](size_t n) { return n == 1 ? " value" : " values"; };
        AddError(source_inst) << "provides " << source_operand_count << values(source_operand_count)
                              << " but " << NameOf(target) << " expects " << target_values.Length()
                              << values(target_values.Length());
        AddDeclarationNote(target);
    }
    size_t count = std::min(source_operand_count, target_values.Length());
    for (size_t i = 0; i < count; i++) {
        auto* source_value = source_inst->Operand(source_operand_offset + i);
        auto* target_value = target_values[i];
        if (!source_value || !target_value) {
            continue;  // Caller should be checking operands are not null
        }
        auto* source_type = source_value->Type();
        auto* target_type = target_value->Type();
        if (source_type != target_type) {
            AddError(source_inst, source_operand_offset + i)
                << "operand with type " << style::Type(source_type->FriendlyName())
                << " does not match " << NameOf(target) << " target type "
                << style::Type(target_type->FriendlyName());
            AddDeclarationNote(target_value);
        }
    }
}

const core::type::Type* Validator::GetVectorPtrElementType(const Instruction* inst, size_t idx) {
    auto* operand = inst->Operands()[idx];
    if (TINT_UNLIKELY(!operand)) {
        return nullptr;
    }

    auto* type = operand->Type();
    if (TINT_UNLIKELY(!type)) {
        return nullptr;
    }

    auto* memory_view_ty = type->As<core::type::MemoryView>();
    if (TINT_LIKELY(memory_view_ty)) {
        auto* vec_ty = memory_view_ty->StoreType()->As<core::type::Vector>();
        if (TINT_LIKELY(vec_ty)) {
            return vec_ty->type();
        }
    }

    AddError(inst, idx) << "operand must be a pointer to vector, got "
                        << style::Type(type->FriendlyName());
    return nullptr;
}

}  // namespace

Result<SuccessType> Validate(const Module& mod, Capabilities capabilities) {
    Validator v(mod, capabilities);
    return v.Run();
}

Result<SuccessType> ValidateAndDumpIfNeeded([[maybe_unused]] const Module& ir,
                                            [[maybe_unused]] const char* msg,
                                            [[maybe_unused]] Capabilities capabilities) {
#if TINT_DUMP_IR_WHEN_VALIDATING
    auto printer = StyledTextPrinter::Create(stdout);
    std::cout << "=========================================================" << std::endl;
    std::cout << "== IR dump before " << msg << ":" << std::endl;
    std::cout << "=========================================================" << std::endl;
    printer->Print(Disassemble(ir).Text());
#endif

#ifndef NDEBUG
    auto result = Validate(ir, capabilities);
    if (result != Success) {
        return result.Failure();
    }
#endif

    return Success;
}

}  // namespace tint::core::ir
