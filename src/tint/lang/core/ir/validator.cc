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
#include "src/tint/lang/core/ir/disassembler.h"
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
#include "src/tint/lang/core/ir/member_builtin_call.h"
#include "src/tint/lang/core/ir/multi_in_block.h"
#include "src/tint/lang/core/ir/next_iteration.h"
#include "src/tint/lang/core/ir/referenced_module_vars.h"
#include "src/tint/lang/core/ir/return.h"
#include "src/tint/lang/core/ir/store.h"
#include "src/tint/lang/core/ir/store_vector_element.h"
#include "src/tint/lang/core/ir/switch.h"
#include "src/tint/lang/core/ir/swizzle.h"
#include "src/tint/lang/core/ir/terminate_invocation.h"
#include "src/tint/lang/core/ir/unary.h"
#include "src/tint/lang/core/ir/unreachable.h"
#include "src/tint/lang/core/ir/unused.h"
#include "src/tint/lang/core/ir/user_call.h"
#include "src/tint/lang/core/ir/var.h"
#include "src/tint/lang/core/type/bool.h"
#include "src/tint/lang/core/type/f32.h"
#include "src/tint/lang/core/type/i8.h"
#include "src/tint/lang/core/type/memory_view.h"
#include "src/tint/lang/core/type/pointer.h"
#include "src/tint/lang/core/type/reference.h"
#include "src/tint/lang/core/type/type.h"
#include "src/tint/lang/core/type/u8.h"
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

/// @returns true if @p attr contains both a location and builtin decoration
bool HasLocationAndBuiltin(const tint::core::IOAttributes& attr) {
    return attr.builtin.has_value() && attr.location.has_value();
}

/// @returns true if @p attr contains one of either location or builtin decoration
bool HasEitherLocationOrBuiltin(const tint::core::IOAttributes& attr) {
    return (attr.builtin.has_value() && !attr.location.has_value()) ||
           (!attr.builtin.has_value() && attr.location.has_value());
}

/// @return true if @param attr does not have invariant decoration or if it also has position
/// decoration
bool InvariantOnlyIfAlsoPosition(const tint::core::IOAttributes& attr) {
    return !attr.invariant || attr.builtin == BuiltinValue::kPosition;
}

/// @returns true if @p ty meets the basic function parameter rules (i.e. one of constructible,
///          pointer, sampler or texture).
///
/// Note: Does not handle corner cases like if certain capabilities are
/// enabled.
bool IsValidFunctionParamType(const core::type::Type* ty) {
    return ty->IsConstructible() || ty->Is<type::Pointer>() || ty->Is<type::Texture>() ||
           ty->Is<type::Sampler>();
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
    /// Runs validation to confirm the structural soundness of the module.
    /// Also runs any validation that is not dependent on the entire module being
    /// sound and sets up data structures for later checks.
    void RunStructuralSoundnessChecks();

    /// Checks that there are no orphaned instructions
    /// Depends on CheckStructuralSoundness() having previously been run
    void CheckForOrphanedInstructions();

    /// Checks that there are no discards called by non-fragment entrypoints
    /// Depends on CheckStructuralSoundness() having previously been run
    void CheckForNonFragmentDiscards();

    /// @returns the IR disassembly, performing a disassemble if this is the first call.
    ir::Disassembler& Disassemble();

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

    /// @param decl the type, value, instruction or block to get the name for
    /// @returns the styled name for the given value, instruction or block
    StyledText NameOf(const CastableBase* decl);

    // @param ty the type to get the name for
    /// @returns the styled name for the given type
    StyledText NameOf(const type::Type* ty);

    /// @param v the value to get the name for
    /// @returns the styled name for the given value
    StyledText NameOf(const Value* v);

    /// @param inst the instruction to get the name for
    /// @returns the styled  name for the given instruction
    StyledText NameOf(const Instruction* inst);

    /// @param block the block to get the name for
    /// @returns the styled  name for the given block
    StyledText NameOf(const Block* block);

    /// Checks the given result is not null and its type is not null
    /// @param inst the instruction
    /// @param idx the result index
    /// @returns true if the result is not null
    bool CheckResult(const Instruction* inst, size_t idx);

    /// Checks the results (and their types) for @p inst are not null. If count is specified then
    /// number of results is checked to be exact.
    /// @param inst the instruction
    /// @param count the number of results to check
    /// @returns true if the results count is as expected and none are null
    bool CheckResults(const ir::Instruction* inst, std::optional<size_t> count);

    /// Checks the given operand is not null and its type is not null
    /// @param inst the instruction
    /// @param idx the operand index
    /// @returns true if the operand is not null
    bool CheckOperand(const Instruction* inst, size_t idx);

    /// Checks the number of operands provided to @p inst and that none of them are null. Also
    /// checks that the types for the operands are not null
    /// @param inst the instruction
    /// @param min_count the minimum number of operands to expect
    /// @param max_count the maximum number of operands to expect, if not set, than only the minimum
    /// number is checked.
    /// @returns true if the number of operands is in the expected range and none are null
    bool CheckOperands(const ir::Instruction* inst,
                       size_t min_count,
                       std::optional<size_t> max_count);

    /// Checks the operands (and their types) for @p inst are not null. If count is specified then
    /// number of operands is checked to be exact.
    /// @param inst the instruction
    /// @param count the number of operands to check
    /// @returns true if the operands count is as expected and none are null
    bool CheckOperands(const ir::Instruction* inst, std::optional<size_t> count);

    /// Checks the number of results for @p inst are exactly equal to @p num_results and the number
    /// of operands is correctly. Both results and operands are confirmed to be non-null.
    /// @param inst the instruction
    /// @param num_results expected number of results for the instruction
    /// @param min_operands the minimum number of operands to expect
    /// @param max_operands the maximum number of operands to expect, if not set, than only the
    /// minimum number is checked.
    /// @returns true if the result and operand counts are as expected and none are null
    bool CheckResultsAndOperandRange(const ir::Instruction* inst,
                                     size_t num_results,
                                     size_t min_operands,
                                     std::optional<size_t> max_operands);

    /// Checks the number of results and operands for @p inst are exactly equal to num_results
    /// and num_operands, respectively, and that none of them are null.
    /// @param inst the instruction
    /// @param num_results expected number of results for the instruction
    /// @param num_operands expected number of operands for the instruction
    /// @returns true if the result and operand counts are as expected and none are null
    bool CheckResultsAndOperands(const ir::Instruction* inst,
                                 size_t num_results,
                                 size_t num_operands);

    /// Checks that @p type does not use any types that are prohibited by the target capabilities.
    /// @param type the type
    /// @param diag a function that creates an error diagnostic for the source of the type
    /// @param ignore_caps a set of capabilities to ignore for this check
    void CheckType(const core::type::Type* type,
                   std::function<diag::Diagnostic&()> diag,
                   Capabilities ignore_caps = {});

    /// Validates the root block
    /// @param blk the block
    void CheckRootBlock(const Block* blk);

    /// Validates the given function
    /// @param func the function to validate
    void CheckFunction(const Function* func);

    /// Validates the specific function as a vertex entry point
    /// @param ep the function to validate
    void CheckVertexEntryPoint(const Function* ep);

    /// Validates that the type annotated with @builtin(position) is correct
    /// @param ep the entry point to associate errors with
    /// @param type the type to validate
    void CheckBuiltinPosition(const Function* ep, const core::type::Type* type);

    /// Validates that the type annotated with @builtin(clip_distances) is correct
    /// @param ep the entry point to associate errors with
    /// @param type the type to validate
    void CheckBuiltinClipDistances(const Function* ep, const core::type::Type* type);

    /// Validates the given instruction
    /// @param inst the instruction to validate
    void CheckInstruction(const Instruction* inst);

    /// Validates the given var
    /// @param var the var to validate
    void CheckVar(const Var* var);

    /// Validates the given let
    /// @param l the let to validate
    void CheckLet(const Let* l);

    /// Validates the given call
    /// @param call the call to validate
    void CheckCall(const Call* call);

    /// Validates the given bitcast
    /// @param bitcast the bitcast to validate
    void CheckBitcast(const Bitcast* bitcast);

    /// Validates the given builtin call
    /// @param call the call to validate
    void CheckBuiltinCall(const BuiltinCall* call);

    /// Validates the given member builtin call
    /// @param call the member call to validate
    void CheckMemberBuiltinCall(const MemberBuiltinCall* call);

    /// Validates the given construct
    /// @param construct the construct to validate
    void CheckConstruct(const Construct* construct);

    /// Validates the given convert
    /// @param convert the convert to validate
    void CheckConvert(const Convert* convert);

    /// Validates the given discard
    /// @note Does not validate that the discard is in a fragment shader, that
    /// needs to be handled later in the validation.
    /// @param discard the discard to validate
    void CheckDiscard(const Discard* discard);

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

    /// Validates the loop body block
    /// @param l the loop to validate
    void CheckLoopBody(const Loop* l);

    /// Validates the loop continuing block
    /// @param l the loop to validate
    void CheckLoopContinuing(const Loop* l);

    /// Validates the given switch
    /// @param s the switch to validate
    void CheckSwitch(const Switch* s);

    /// Validates the given swizzle
    /// @param s the swizzle to validate
    void CheckSwizzle(const Swizzle* s);

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

    /// Validates the given unreachable
    /// @param u the unreachable to validate
    void CheckUnreachable(const Unreachable* u);

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

    /// Get the function that contains an instruction.
    /// @param inst the instruction
    /// @returns the function
    const ir::Function* ContainingFunction(const ir::Instruction* inst) {
        return block_to_function_.GetOrAdd(inst->Block(), [&] {  //
            return ContainingFunction(inst->Block()->Parent());
        });
    }

    /// Get any endpoints that call a function.
    /// @param f the function
    /// @returns all end points that call the function
    Hashset<const ir::Function*, 4> ContainingEndPoints(const ir::Function* f) {
        Hashset<const ir::Function*, 4> result{};
        Hashset<const ir::Function*, 4> visited{f};

        auto call_sites = user_func_calls_.GetOr(f, Hashset<const ir::UserCall*, 4>()).Vector();
        while (!call_sites.IsEmpty()) {
            auto call_site = call_sites.Pop();
            auto calling_function = ContainingFunction(call_site);
            if (visited.Contains(calling_function)) {
                continue;
            }
            visited.Add(calling_function);

            if (calling_function->Stage() != Function::PipelineStage::kUndefined) {
                result.Add(calling_function);
            }

            for (auto new_call_sites :
                 user_func_calls_.GetOr(f, Hashset<const ir::UserCall*, 4>())) {
                call_sites.Push(new_call_sites);
            }
        }

        return result;
    }

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
    std::optional<ir::Disassembler> disassembler_;  // Use Disassemble()
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
    Hashmap<const ir::Block*, const ir::Function*, 64> block_to_function_{};
    Hashmap<const ir::Function*, Hashset<const ir::UserCall*, 4>, 4> user_func_calls_;
    Hashset<const ir::Discard*, 4> discards_;
    core::ir::ReferencedModuleVars<const Module> referenced_module_vars_;
};

Validator::Validator(const Module& mod, Capabilities capabilities)
    : mod_(mod), capabilities_(capabilities), referenced_module_vars_(mod) {}

Validator::~Validator() = default;

Disassembler& Validator::Disassemble() {
    if (!disassembler_) {
        disassembler_.emplace(ir::Disassembler(mod_));
    }
    return *disassembler_;
}

Result<SuccessType> Validator::Run() {
    RunStructuralSoundnessChecks();

    CheckForOrphanedInstructions();
    CheckForNonFragmentDiscards();

    if (diagnostics_.ContainsErrors()) {
        diagnostics_.AddNote(Source{}) << "# Disassembly\n" << Disassemble().Text();
        return Failure{std::move(diagnostics_)};
    }
    return Success;
}

void Validator::CheckForOrphanedInstructions() {
    if (diagnostics_.ContainsErrors()) {
        return;
    }

    // Check for orphaned instructions.
    for (auto* inst : mod_.Instructions()) {
        if (!visited_instructions_.Contains(inst)) {
            AddError(inst) << "orphaned instruction: " << inst->FriendlyName();
        }
    }
}

void Validator::CheckForNonFragmentDiscards() {
    if (diagnostics_.ContainsErrors()) {
        return;
    }

    // Check for discards in non-fragments
    for (const auto& d : discards_) {
        const auto* f = ContainingFunction(d);
        for (const Function* ep : ContainingEndPoints(f)) {
            if (ep->Stage() != Function::PipelineStage::kFragment) {
                AddError(d) << "cannot be called in non-fragment end point";
            }
        }
    }
}

void Validator::RunStructuralSoundnessChecks() {
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
        block_to_function_.Add(func->Block(), func);
        CheckFunction(func);
    }
}

diag::Diagnostic& Validator::AddError(const Instruction* inst) {
    diagnostics_.ReserveAdditional(2);  // Ensure diagnostics don't resize alive after AddNote()
    auto src = Disassemble().InstructionSource(inst);
    auto& diag = AddError(src) << inst->FriendlyName() << ": ";

    if (!block_stack_.IsEmpty()) {
        AddNote(block_stack_.Back()) << "in block";
    }
    return diag;
}

diag::Diagnostic& Validator::AddError(const Instruction* inst, size_t idx) {
    diagnostics_.ReserveAdditional(2);  // Ensure diagnostics don't resize alive after AddNote()
    auto src =
        Disassemble().OperandSource(Disassembler::IndexedValue{inst, static_cast<uint32_t>(idx)});
    auto& diag = AddError(src) << inst->FriendlyName() << ": ";

    if (!block_stack_.IsEmpty()) {
        AddNote(block_stack_.Back()) << "in block";
    }
    return diag;
}

diag::Diagnostic& Validator::AddResultError(const Instruction* inst, size_t idx) {
    diagnostics_.ReserveAdditional(2);  // Ensure diagnostics don't resize alive after AddNote()
    auto src =
        Disassemble().ResultSource(Disassembler::IndexedValue{inst, static_cast<uint32_t>(idx)});
    auto& diag = AddError(src) << inst->FriendlyName() << ": ";

    if (!block_stack_.IsEmpty()) {
        AddNote(block_stack_.Back()) << "in block";
    }
    return diag;
}

diag::Diagnostic& Validator::AddError(const Block* blk) {
    auto src = Disassemble().BlockSource(blk);
    return AddError(src);
}

diag::Diagnostic& Validator::AddError(const BlockParam* param) {
    auto src = Disassemble().BlockParamSource(param);
    return AddError(src);
}

diag::Diagnostic& Validator::AddError(const Function* func) {
    auto src = Disassemble().FunctionSource(func);
    return AddError(src);
}

diag::Diagnostic& Validator::AddError(const FunctionParam* param) {
    auto src = Disassemble().FunctionParamSource(param);
    return AddError(src);
}

diag::Diagnostic& Validator::AddNote(const Instruction* inst) {
    auto src = Disassemble().InstructionSource(inst);
    return AddNote(src);
}

diag::Diagnostic& Validator::AddNote(const Function* func) {
    auto src = Disassemble().FunctionSource(func);
    return AddNote(src);
}

diag::Diagnostic& Validator::AddOperandNote(const Instruction* inst, size_t idx) {
    auto src =
        Disassemble().OperandSource(Disassembler::IndexedValue{inst, static_cast<uint32_t>(idx)});
    return AddNote(src);
}

diag::Diagnostic& Validator::AddResultNote(const Instruction* inst, size_t idx) {
    auto src =
        Disassemble().ResultSource(Disassembler::IndexedValue{inst, static_cast<uint32_t>(idx)});
    return AddNote(src);
}

diag::Diagnostic& Validator::AddNote(const Block* blk) {
    auto src = Disassemble().BlockSource(blk);
    return AddNote(src);
}

diag::Diagnostic& Validator::AddError(Source src) {
    auto& diag = diagnostics_.AddError(src);
    diag.owned_file = Disassemble().File();
    return diag;
}

diag::Diagnostic& Validator::AddNote(Source src) {
    auto& diag = diagnostics_.AddNote(src);
    diag.owned_file = Disassemble().File();
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
    auto src = Disassemble().BlockSource(block);
    if (src.file) {
        AddNote(src) << NameOf(block) << " declared here";
    }
}

void Validator::AddDeclarationNote(const BlockParam* param) {
    auto src = Disassemble().BlockParamSource(param);
    if (src.file) {
        AddNote(src) << NameOf(param) << " declared here";
    }
}

void Validator::AddDeclarationNote(const Function* fn) {
    AddNote(fn) << NameOf(fn) << " declared here";
}

void Validator::AddDeclarationNote(const FunctionParam* param) {
    auto src = Disassemble().FunctionParamSource(param);
    if (src.file) {
        AddNote(src) << NameOf(param) << " declared here";
    }
}

void Validator::AddDeclarationNote(const Instruction* inst) {
    auto src = Disassemble().InstructionSource(inst);
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
        [&](const type::Type* ty) { return NameOf(ty); },
        [&](const Value* value) { return NameOf(value); },
        [&](const Instruction* inst) { return NameOf(inst); },
        [&](const Block* block) { return NameOf(block); },  //
        TINT_ICE_ON_NO_MATCH);
}

StyledText Validator::NameOf(const type::Type* ty) {
    auto name = ty ? ty->FriendlyName() : "undef";
    return StyledText{} << style::Type(name);
}

StyledText Validator::NameOf(const Value* value) {
    return Disassemble().NameOf(value);
}

StyledText Validator::NameOf(const Instruction* inst) {
    auto name = inst ? inst->FriendlyName() : "undef";
    return StyledText{} << style::Instruction(name);
}

StyledText Validator::NameOf(const Block* block) {
    auto parent_name = block->Parent() ? block->Parent()->FriendlyName() : "undef";
    return StyledText{} << style::Instruction(parent_name) << " block "
                        << Disassemble().NameOf(block);
}

bool Validator::CheckResult(const Instruction* inst, size_t idx) {
    auto* result = inst->Result(idx);
    if (DAWN_UNLIKELY(result == nullptr)) {
        AddResultError(inst, idx) << "result is undefined";
        return false;
    }

    if (DAWN_UNLIKELY(result->Type() == nullptr)) {
        AddResultError(inst, idx) << "result type is undefined";
        return false;
    }

    if (DAWN_UNLIKELY(result->Instruction() == nullptr)) {
        AddResultError(inst, idx) << "result instruction is undefined";
        return false;
    }

    return true;
}

bool Validator::CheckResults(const ir::Instruction* inst, std::optional<size_t> count = {}) {
    if (count.has_value()) {
        if (DAWN_UNLIKELY(inst->Results().Length() != count.value())) {
            AddError(inst) << "expected exactly " << count.value() << " results, got "
                           << inst->Results().Length();
            return false;
        }
    }

    bool passed = true;
    for (size_t i = 0; i < inst->Results().Length(); i++) {
        if (DAWN_UNLIKELY(!CheckResult(inst, i))) {
            passed = false;
        }
    }
    return passed;
}

bool Validator::CheckOperand(const Instruction* inst, size_t idx) {
    auto* operand = inst->Operand(idx);
    if (DAWN_UNLIKELY(operand == nullptr)) {
        AddError(inst, idx) << "operand is undefined";
        return false;
    }

    // ir::Unused is a internal value used by some transforms to track unused entries, and is
    // removed as part of generating an output shader.
    if (DAWN_UNLIKELY(operand->Is<ir::Unused>())) {
        return true;
    }

    // ir::Function does not have a meaningful type, so does not override the default Type()
    // behaviour.
    if (DAWN_UNLIKELY(!operand->Is<ir::Function>() && operand->Type() == nullptr)) {
        AddError(inst, idx) << "operand type is undefined";
        return false;
    }

    if (DAWN_UNLIKELY(!operand->Alive())) {
        AddError(inst, idx) << "operand is not alive";
        return false;
    }

    if (DAWN_UNLIKELY(!operand->HasUsage(inst, idx))) {
        AddError(inst, idx) << "operand missing usage";
        return false;
    }

    if (auto fn = operand->As<Function>(); fn && !all_functions_.Contains(fn)) {
        AddError(inst, idx) << NameOf(operand) << " is not part of the module";
        return false;
    }

    if (DAWN_UNLIKELY(!operand->Is<ir::Unused>() && !operand->Is<Constant>() &&
                      !scope_stack_.Contains(operand))) {
        AddError(inst, idx) << NameOf(operand) << " is not in scope";
        AddDeclarationNote(operand);
        return false;
    }

    return true;
}

bool Validator::CheckOperands(const ir::Instruction* inst,
                              size_t min_count,
                              std::optional<size_t> max_count) {
    if (DAWN_UNLIKELY(inst->Operands().Length() < min_count)) {
        if (max_count.has_value()) {
            AddError(inst) << "expected between " << min_count << " and " << max_count.value()
                           << " operands, got " << inst->Operands().Length();
        } else {
            AddError(inst) << "expected at least " << min_count << " operands, got "
                           << inst->Operands().Length();
        }
        return false;
    }

    if (DAWN_UNLIKELY(max_count.has_value() && inst->Operands().Length() > max_count.value())) {
        AddError(inst) << "expected between " << min_count << " and " << max_count.value()
                       << " operands, got " << inst->Operands().Length();
        return false;
    }

    bool passed = true;
    for (size_t i = 0; i < inst->Operands().Length(); i++) {
        if (DAWN_UNLIKELY(!CheckOperand(inst, i))) {
            passed = false;
        }
    }
    return passed;
}

bool Validator::CheckOperands(const ir::Instruction* inst, std::optional<size_t> count = {}) {
    if (count.has_value()) {
        if (DAWN_UNLIKELY(inst->Operands().Length() != count.value())) {
            AddError(inst) << "expected exactly " << count.value() << " operands, got "
                           << inst->Operands().Length();
            return false;
        }
    }

    bool passed = true;
    for (size_t i = 0; i < inst->Operands().Length(); i++) {
        if (DAWN_UNLIKELY(!CheckOperand(inst, i))) {
            passed = false;
        }
    }
    return passed;
}

bool Validator::CheckResultsAndOperandRange(const ir::Instruction* inst,
                                            size_t num_results,
                                            size_t min_operands,
                                            std::optional<size_t> max_operands = {}) {
    // Intentionally avoiding short-circuiting here
    bool results_passed = CheckResults(inst, num_results);
    bool operands_passed = CheckOperands(inst, min_operands, max_operands);
    return results_passed && operands_passed;
}

bool Validator::CheckResultsAndOperands(const ir::Instruction* inst,
                                        size_t num_results,
                                        size_t num_operands) {
    // Intentionally avoiding short-circuiting here
    bool results_passed = CheckResults(inst, num_results);
    bool operands_passed = CheckOperands(inst, num_operands);
    return results_passed && operands_passed;
}

void Validator::CheckType(const core::type::Type* root,
                          std::function<diag::Diagnostic&()> diag,
                          Capabilities ignore_caps) {
    auto visit = [&](const type::Type* type) {
        return tint::Switch(
            type,
            [&](const type::Reference*) {
                // Reference types are guarded by the AllowRefTypes capability.
                if (!capabilities_.Contains(Capability::kAllowRefTypes) ||
                    ignore_caps.Contains(Capability::kAllowRefTypes)) {
                    diag() << "reference types are not permitted here";
                    return false;
                } else if (type != root) {
                    // If they are allowed, reference types still cannot be nested.
                    diag() << "nested reference types are not permitted";
                    return false;
                }
                return true;
            },
            [&](const type::Pointer*) {
                if (type != root) {
                    // Nesting pointer types inside structures is guarded by a capability.
                    if (!(root->Is<type::Struct>() &&
                          capabilities_.Contains(Capability::kAllowPointersInStructures))) {
                        diag() << "nested pointer types are not permitted";
                        return false;
                    }
                }
                return true;
            },
            [&](const type::I8*) {
                // i8 types are guarded by the Allow8BitIntegers capability.
                if (!capabilities_.Contains(Capability::kAllow8BitIntegers)) {
                    diag() << "8-bit integer types are not permitted";
                    return false;
                }
                return true;
            },
            [&](const type::U8*) {
                // u8 types are guarded by the Allow8BitIntegers capability.
                if (!capabilities_.Contains(Capability::kAllow8BitIntegers)) {
                    diag() << "8-bit integer types are not permitted";
                    return false;
                }
                return true;
            },
            [](Default) { return true; });
    };

    Vector<const type::Type*, 8> stack{root};
    Hashset<const type::Type*, 8> seen{};
    while (!stack.IsEmpty()) {
        auto* ty = stack.Pop();
        if (!ty) {
            continue;
        }
        if (!visit(ty)) {
            return;
        }

        if (auto* view = ty->As<type::MemoryView>(); view && seen.Add(view)) {
            stack.Push(view->StoreType());
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
}

void Validator::CheckRootBlock(const Block* blk) {
    block_stack_.Push(blk);
    TINT_DEFER(block_stack_.Pop());

    for (auto* inst : *blk) {
        if (inst->Block() != blk) {
            AddError(inst) << "instruction in root block does not have root block as parent";
            continue;
        }

        tint::Switch(
            inst,  //
            [&](const core::ir::Var* var) { CheckInstruction(var); },
            [&](const core::ir::Let* let) {
                if (capabilities_.Contains(Capability::kAllowModuleScopeLets)) {
                    CheckInstruction(let);
                } else {
                    AddError(inst) << "root block: invalid instruction: " << inst->TypeInfo().name;
                }
            },
            [&](const core::ir::Construct* c) {
                if (capabilities_.Contains(Capability::kAllowModuleScopeLets)) {
                    CheckInstruction(c);
                } else {
                    AddError(inst) << "root block: invalid instruction: " << inst->TypeInfo().name;
                }
            },
            [&](Default) {
                AddError(inst) << "root block: invalid instruction: " << inst->TypeInfo().name;
            });
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

        if (!param->Type()) {
            AddError(param) << "function parameter has nullptr type";
            return;
        }

        // References not allowed on function signatures even with Capability::kAllowRefTypes.
        CheckType(
            param->Type(), [&]() -> diag::Diagnostic& { return AddError(param); },
            Capabilities{Capability::kAllowRefTypes});

        if (!InvariantOnlyIfAlsoPosition(param->Attributes())) {
            AddError(func)
                << "invariant can only decorate a param iff it is also decorated with position";
        }

        if (!IsValidFunctionParamType(param->Type())) {
            auto struct_ty = param->Type()->As<core::type::Struct>();
            if (!capabilities_.Contains(Capability::kAllowPointersInStructures) || !struct_ty ||
                struct_ty->Members().Any([](const core::type::StructMember* m) {
                    return !IsValidFunctionParamType(m->Type());
                })) {
                AddError(param) << "function parameter type must be constructible, a pointer, a "
                                   "texture, or a sampler";
            }
        }

        if (auto* s = param->Type()->As<core::type::Struct>()) {
            for (auto* mem : s->Members()) {
                if (!InvariantOnlyIfAlsoPosition(mem->Attributes())) {
                    AddError(func) << "invariant can only decorate a param member iff it is also "
                                      "decorated with position";
                }

                if (HasLocationAndBuiltin(mem->Attributes())) {
                    AddError(param)
                        << "a builtin and location cannot be both declared for a struct member";
                }
            }
        } else {
            if (HasLocationAndBuiltin(param->Attributes())) {
                AddError(param) << "a builtin and location cannot be both declared for a param";
            }
        }

        scope_stack_.Add(param);
    }

    if (func->Stage() == Function::PipelineStage::kCompute) {
        if (DAWN_UNLIKELY(!func->WorkgroupSize().has_value())) {
            AddError(func) << "compute entry point requires workgroup size attribute";
        }

        if (DAWN_UNLIKELY(func->ReturnType() && !func->ReturnType()->Is<core::type::Void>())) {
            AddError(func) << "compute entry point must not have a return type";
        }
    } else if (auto* s = func->ReturnType()->As<core::type::Struct>()) {
        for (auto* mem : s->Members()) {
            if (HasLocationAndBuiltin(mem->Attributes())) {
                AddError(func)
                    << "a builtin and location cannot be both declared for a struct member";
            } else if (func->Stage() != Function::PipelineStage::kUndefined &&
                       !HasEitherLocationOrBuiltin(mem->Attributes())) {
                AddError(func) << "members of struct used for returns of entry points must "
                                  "have a builtin or location decoration";
            }
        }
    } else {
        if (HasLocationAndBuiltin(func->ReturnAttributes())) {
            AddError(func)
                << "a builtin and location cannot be both declared for a function return";
        } else if (!func->ReturnType()->Is<core::type::Void>() &&
                   func->Stage() != Function::PipelineStage::kUndefined &&
                   !HasEitherLocationOrBuiltin(func->ReturnAttributes())) {
            AddError(func) << "a non-void return for an entry point must have a builtin or "
                              "location decoration";
        }
    }

    // References not allowed on function signatures even with Capability::kAllowRefTypes.
    CheckType(
        func->ReturnType(), [&]() -> diag::Diagnostic& { return AddError(func); },
        Capabilities{Capability::kAllowRefTypes});

    if (func->Stage() != Function::PipelineStage::kUndefined) {
        if (DAWN_UNLIKELY(mod_.NameOf(func).Name().empty())) {
            AddError(func) << "entry points must have names";
        }
    }

    // void needs to be filtered out, since it isn't constructible, but used in the IR when no
    // return is specified.
    if (DAWN_UNLIKELY(!func->ReturnType()->Is<core::type::Void>() &&
                      !func->ReturnType()->IsConstructible())) {
        AddError(func) << "function return type must be constructible";
    }

    const auto* ret_struct = func->ReturnType()->As<core::type::Struct>();
    if (ret_struct) {
        for (auto* mem : ret_struct->Members()) {
            if (!InvariantOnlyIfAlsoPosition(mem->Attributes())) {
                AddError(func) << "invariant can only decorate a member iff it is also decorated "
                                  "with position";
            }
        }
    } else {
        if (!InvariantOnlyIfAlsoPosition(func->ReturnAttributes())) {
            AddError(func)
                << "invariant can only decorate a return iff it is also decorated with position";
        }
    }

    if (func->Stage() != Function::PipelineStage::kFragment) {
        if (DAWN_UNLIKELY(func->ReturnBuiltin().has_value() &&
                          func->ReturnBuiltin().value() == BuiltinValue::kFragDepth)) {
            AddError(func) << "frag_depth can only be declared for fragment entry points";
        }
    }

    if (func->Stage() == Function::PipelineStage::kVertex) {
        CheckVertexEntryPoint(func);
    }

    QueueBlock(func->Block());
    ProcessTasks();
}

void Validator::CheckVertexEntryPoint(const Function* ep) {
    const auto* ret_struct = ep->ReturnType()->As<core::type::Struct>();
    bool contains_position = false;
    if (ret_struct) {
        for (auto* mem : ret_struct->Members()) {
            if (!InvariantOnlyIfAlsoPosition(mem->Attributes())) {
                AddError(ep) << "invariant can only decorate output members iff they are also "
                                "position builtins";
            }

            if (!mem->Attributes().builtin.has_value()) {
                continue;
            }
            switch (mem->Attributes().builtin.value()) {
                case BuiltinValue::kPosition:
                    contains_position = true;
                    CheckBuiltinPosition(ep, mem->Type());
                    break;
                case BuiltinValue::kClipDistances:
                    CheckBuiltinClipDistances(ep, mem->Type());
                    break;
                default:
                    break;
            }
        }
    } else {
        if (!InvariantOnlyIfAlsoPosition(ep->ReturnAttributes())) {
            AddError(ep)
                << "invariant can only decorate outputs iff they are also position builtins";
        }
        if (ep->ReturnBuiltin() && ep->ReturnBuiltin() == BuiltinValue::kPosition) {
            contains_position = true;
            CheckBuiltinPosition(ep, ep->ReturnType());
        }
    }

    for (auto var : referenced_module_vars_.TransitiveReferences(ep)) {
        const auto* res_type = var->Result(0)->Type()->UnwrapPtrOrRef();
        const auto* res_struct = res_type->As<core::type::Struct>();
        if (res_struct) {
            for (auto* mem : res_struct->Members()) {
                if (!InvariantOnlyIfAlsoPosition(mem->Attributes())) {
                    AddError(ep) << "invariant can only decorate members iff they are also "
                                    "position builtins";
                }

                if (!mem->Attributes().builtin.has_value()) {
                    continue;
                }
                switch (mem->Attributes().builtin.value()) {
                    case BuiltinValue::kPosition:
                        contains_position = true;
                        CheckBuiltinPosition(ep, mem->Type()->UnwrapPtrOrRef());
                        break;
                    case BuiltinValue::kClipDistances:
                        CheckBuiltinClipDistances(ep, mem->Type()->UnwrapPtrOrRef());
                        break;
                    default:
                        break;
                }
            }
        } else {
            if (!InvariantOnlyIfAlsoPosition(var->Attributes())) {
                AddError(ep)
                    << "invariant can only decorate vars iff they are also position builtins";
            }

            if (!var->Attributes().builtin.has_value()) {
                continue;
            }
            switch (var->Attributes().builtin.value()) {
                case BuiltinValue::kPosition: {
                    contains_position = true;
                    CheckBuiltinPosition(ep, res_type);
                } break;
                case BuiltinValue::kClipDistances:
                    CheckBuiltinClipDistances(ep, var->Result(0)->Type()->UnwrapPtrOrRef());
                    break;
                default:
                    break;
            }
        }
    }

    if (DAWN_UNLIKELY(!contains_position)) {
        AddError(ep) << "position must be declared for vertex entry point output";
    }
}

void Validator::CheckBuiltinPosition(const Function* ep, const core::type::Type* type) {
    auto elems = type->Elements();
    if (!type->IsFloatVector() || !elems.type->Is<core::type::F32>() || elems.count != 4) {
        AddError(ep) << "position must be a vec4<f32>";
    }
}

void Validator::CheckBuiltinClipDistances(const Function* ep, const core::type::Type* type) {
    const auto elems = type->Elements();
    if (!elems.type || !elems.type->Is<core::type::F32>() || elems.count > 8) {
        AddError(ep) << "clip_distances must be an array<f32, N>, where N <= 8";
    }
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

            // References not allowed on block parameters even with Capability::kAllowRefTypes.
            CheckType(
                param->Type(), [&]() -> diag::Diagnostic& { return AddError(param); },
                Capabilities{Capability::kAllowRefTypes});

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
            continue;
        }
        CheckType(res->Type(), [&]() -> diag::Diagnostic& { return AddResultError(inst, i); });
    }

    auto ops = inst->Operands();
    for (size_t i = 0; i < ops.Length(); ++i) {
        auto* op = ops[i];
        if (!op) {
            continue;
        }

        CheckType(op->Type(), [&]() -> diag::Diagnostic& { return AddError(inst, i); });
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
        [&](const Swizzle* s) { CheckSwizzle(s); },                        //
        [&](const Terminator* b) { CheckTerminator(b); },                  //
        [&](const Unary* u) { CheckUnary(u); },                            //
        [&](const Var* var) { CheckVar(var); },                            //
        [&](const Default) { AddError(inst) << "missing validation"; });

    for (auto* result : results) {
        scope_stack_.Add(result);
    }
}

void Validator::CheckVar(const Var* var) {
    // Intentionally not checking operands, since Var may have a null operand
    if (!CheckResults(var, Var::kNumResults)) {
        return;
    }

    // Check that initializer and result type match
    if (var->Initializer()) {
        if (!CheckOperand(var, ir::Var::kInitializerOperandOffset)) {
            return;
        }

        if (var->Initializer()->Type() != var->Result(0)->Type()->UnwrapPtrOrRef()) {
            AddError(var) << "initializer type "
                          << style::Type(var->Initializer()->Type()->FriendlyName())
                          << " does not match store type "
                          << style::Type(var->Result(0)->Type()->UnwrapPtrOrRef()->FriendlyName());
            return;
        }
    }

    auto* result_type = var->Result(0)->Type();
    if (result_type == nullptr) {
        AddError(var) << "result type is undefined";
        return;
    }

    auto* mv = result_type->As<type::MemoryView>();
    if (!mv) {
        AddError(var) << "result type must be a pointer or a reference";
        return;
    }

    // Check that only resource variables have @group and @binding set
    switch (mv->AddressSpace()) {
        case AddressSpace::kHandle:
            if (!capabilities_.Contains(Capability::kAllowHandleVarsWithoutBindings)) {
                if (!var->BindingPoint().has_value()) {
                    AddError(var) << "resource variable missing binding points";
                }
            }
            break;
        case AddressSpace::kStorage:
        case AddressSpace::kUniform:
            if (!var->BindingPoint().has_value()) {
                AddError(var) << "resource variable missing binding points";
            }
            break;
        default:
            break;
    }

    // Check that non-handle variables don't have @input_attachment_index set
    if (var->InputAttachmentIndex().has_value() && mv->AddressSpace() != AddressSpace::kHandle) {
        AddError(var) << "'@input_attachment_index' is not valid for non-handle var";
        return;
    }

    if (HasLocationAndBuiltin(var->Attributes())) {
        AddError(var) << "a builtin and location cannot be both declared for a var";
        return;
    }

    if (auto* s = var->Result(0)->Type()->UnwrapPtrOrRef()->As<core::type::Struct>()) {
        for (auto* mem : s->Members()) {
            if (HasLocationAndBuiltin(mem->Attributes())) {
                AddError(var)
                    << "a builtin and location cannot be both declared for a struct member";
                return;
            }
        }
    }
}

void Validator::CheckLet(const Let* l) {
    if (!CheckResultsAndOperands(l, Let::kNumResults, Let::kNumOperands)) {
        return;
    }

    if (l->Result(0) && l->Value()) {
        if (l->Result(0)->Type() != l->Value()->Type()) {
            AddError(l) << "result type " << style::Type(l->Result(0)->Type()->FriendlyName())
                        << " does not match value type "
                        << style::Type(l->Value()->Type()->FriendlyName());
        }
    }
}

void Validator::CheckCall(const Call* call) {
    tint::Switch(
        call,                                                                   //
        [&](const Bitcast* b) { CheckBitcast(b); },                             //
        [&](const BuiltinCall* c) { CheckBuiltinCall(c); },                     //
        [&](const MemberBuiltinCall* c) { CheckMemberBuiltinCall(c); },         //
        [&](const Construct* c) { CheckConstruct(c); },                         //
        [&](const Convert* c) { CheckConvert(c); },                             //
        [&](const Discard* d) {                                                 //
            discards_.Add(d);                                                   //
            CheckDiscard(d);                                                    //
        },                                                                      //
        [&](const UserCall* c) {                                                //
            if (c->Target()) {                                                  //
                auto calls =                                                    //
                    user_func_calls_.GetOr(c->Target(),                         //
                                           Hashset<const ir::UserCall*, 4>{});  //
                calls.Add(c);                                                   //
                user_func_calls_.Replace(c->Target(), calls);                   //
            }                                                                   //
            CheckUserCall(c);                                                   //
        },                                                                      //
        [&](Default) {
            // Validation of custom IR instructions
        });
}

void Validator::CheckBitcast(const Bitcast* bitcast) {
    CheckResultsAndOperands(bitcast, Bitcast::kNumResults, Bitcast::kNumOperands);
}

void Validator::CheckBuiltinCall(const BuiltinCall* call) {
    auto args =
        Transform<8>(call->Args(), [&](const ir::Value* v) { return v ? v->Type() : nullptr; });
    if (args.Any([&](const type::Type* ty) { return ty == nullptr; })) {
        AddError(call) << "argument to builtin has undefined type";
        return;
    }

    intrinsic::Context context{
        call->TableData(),
        type_mgr_,
        symbols_,
    };

    auto builtin = core::intrinsic::LookupFn(context, call->FriendlyName().c_str(), call->FuncId(),
                                             Empty, args, core::EvaluationStage::kRuntime);
    if (builtin != Success) {
        AddError(call) << builtin.Failure();
        return;
    }

    TINT_ASSERT(builtin->return_type);

    if (call->Result(0) == nullptr) {
        AddError(call) << "call to builtin does not have a return type";
        return;
    }

    if (builtin->return_type != call->Result(0)->Type()) {
        AddError(call) << "call result type does not match builtin return type";
        return;
    }
}

void Validator::CheckMemberBuiltinCall(const MemberBuiltinCall* call) {
    auto args = Vector<const type::Type*, 8>({call->Object()->Type()});
    for (auto* arg : call->Args()) {
        args.Push(arg->Type());
    }
    intrinsic::Context context{
        call->TableData(),
        type_mgr_,
        symbols_,
    };

    auto result =
        core::intrinsic::LookupMemberFn(context, call->FriendlyName().c_str(), call->FuncId(),
                                        Empty, std::move(args), core::EvaluationStage::kRuntime);
    if (result != Success) {
        AddError(call) << result.Failure();
        return;
    }

    if (result->return_type != call->Result(0)->Type()) {
        AddError(call) << "member call result type does not match builtin return type";
    }
}

void Validator::CheckConstruct(const Construct* construct) {
    if (!CheckResultsAndOperandRange(construct, Construct::kNumResults, Construct::kMinOperands)) {
        return;
    }

    auto args = construct->Args();
    if (args.IsEmpty()) {
        // Zero-value constructors are valid for all constructible types.
        return;
    }

    if (auto* str = As<type::Struct>(construct->Result(0)->Type())) {
        auto members = str->Members();
        if (args.Length() != str->Members().Length()) {
            AddError(construct) << "structure has " << members.Length()
                                << " members, but construct provides " << args.Length()
                                << " arguments";
            return;
        }
        for (size_t i = 0; i < args.Length(); i++) {
            if (args[i]->Is<ir::Unused>()) {
                continue;
            }
            if (args[i]->Type() != members[i]->Type()) {
                AddError(construct, Construct::kArgsOperandOffset + i)
                    << "structure member " << i << " is of type "
                    << style::Type(members[i]->Type()->FriendlyName())
                    << ", but argument is of type " << style::Type(args[i]->Type()->FriendlyName());
            }
        }
    }
}

void Validator::CheckConvert(const Convert* convert) {
    CheckResultsAndOperands(convert, Convert::kNumResults, Convert::kNumOperands);
}

void Validator::CheckDiscard(const tint::core::ir::Discard* discard) {
    CheckResultsAndOperands(discard, Discard::kNumResults, Discard::kNumOperands);
}

void Validator::CheckUserCall(const UserCall* call) {
    if (!CheckResultsAndOperandRange(call, UserCall::kNumResults, UserCall::kMinOperands)) {
        return;
    }

    if (!call->Target()) {
        AddError(call, UserCall::kFunctionOperandOffset) << "target not defined or not a function";
        return;
    }

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
    if (!CheckResultsAndOperandRange(a, Access::kNumResults, Access::kMinNumOperands)) {
        return;
    }

    auto* obj_view = a->Object()->Type()->As<core::type::MemoryView>();
    auto* ty = obj_view ? obj_view->StoreType() : a->Object()->Type();

    enum Kind : uint8_t { kPtr, kRef, kValue };

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
        if (DAWN_UNLIKELY(!index->Type()->IsIntegerScalar())) {
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
            if (value->Type()->IsSignedIntegerScalar()) {
                // index is a signed integer scalar. Check that the index isn't negative.
                // If the index is unsigned, we can skip this.
                auto idx = value->ValueAs<AInt>();
                if (DAWN_UNLIKELY(idx < 0)) {
                    err() << "constant index must be positive, got " << idx;
                    return;
                }
            }

            auto idx = value->ValueAs<uint32_t>();
            auto* el = ty->Element(idx);
            if (DAWN_UNLIKELY(!el)) {
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
            if (DAWN_UNLIKELY(!el)) {
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
    if (DAWN_UNLIKELY(!ok)) {
        AddError(a) << "result of access chain is type " << desc_of(in_kind, ty)
                    << " but instruction type is " << style::Type(want->FriendlyName());
    }
}

void Validator::CheckBinary(const Binary* b) {
    if (!CheckResultsAndOperandRange(b, Binary::kNumResults, Binary::kNumOperands)) {
        return;
    }

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
                            << style::Instruction(Disassemble().NameOf(b->Op())) << " result type "
                            << style::Type(overload->return_type->FriendlyName());
            }
        }
    }
}

void Validator::CheckUnary(const Unary* u) {
    if (!CheckResultsAndOperandRange(u, Unary::kNumResults, Unary::kNumOperands)) {
        return;
    }

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
                            << style::Instruction(Disassemble().NameOf(u->Op())) << " result type "
                            << style::Type(overload->return_type->FriendlyName());
            }
        }
    }
}

void Validator::CheckIf(const If* if_) {
    CheckResults(if_);
    CheckOperand(if_, If::kConditionOperandOffset);

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

    tasks_.Push([this, l] {
        CheckLoopBody(l);
        BeginBlock(l->Body());
    });
    if (!l->Initializer()->IsEmpty()) {
        tasks_.Push([this, l] { BeginBlock(l->Initializer()); });
    }
    tasks_.Push([this, l] { control_stack_.Push(l); });
}

void Validator::CheckLoopBody(const Loop* loop) {
    // If the body block has parameters, there must be an initializer block.
    if (!loop->Body()->Params().IsEmpty()) {
        if (!loop->HasInitializer()) {
            AddError(loop) << "loop with body block parameters must have an initializer";
        }
    }
}

void Validator::CheckLoopContinuing(const Loop* loop) {
    if (!loop->HasContinuing()) {
        return;
    }

    // Ensure that values used in the loop continuing are not from the loop body, after a
    // continue instruction.
    if (auto* first_continue = first_continues_.GetOr(loop, nullptr)) {
        // Find the instruction in the body block that is or holds the first continue
        // instruction.
        const Instruction* holds_continue = first_continue;
        while (holds_continue && holds_continue->Block() &&
               holds_continue->Block() != loop->Body()) {
            holds_continue = holds_continue->Block()->Parent();
        }

        // Check that all subsequent instruction values are not used in the continuing block.
        for (auto* inst = holds_continue; inst; inst = inst->next) {
            for (auto* result : inst->Results()) {
                result->ForEachUseUnsorted([&](Usage use) {
                    if (TransitivelyHolds(loop->Continuing(), use.instruction)) {
                        AddError(use.instruction, use.operand_index)
                            << NameOf(result)
                            << " cannot be used in continuing block as it is declared after "
                               "the "
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
    CheckOperand(s, Switch::kConditionOperandOffset);

    if (s->Condition() && !s->Condition()->Type()->IsIntegerScalar()) {
        AddError(s, Switch::kConditionOperandOffset) << "condition type must be an integer scalar";
    }

    tasks_.Push([this] { control_stack_.Pop(); });

    bool found_default = false;
    for (auto& cse : s->Cases()) {
        QueueBlock(cse.block);

        for (const auto& sel : cse.selectors) {
            if (sel.IsDefault()) {
                found_default = true;
            }
        }
    }

    if (!found_default) {
        AddError(s) << "missing default case for switch";
    }

    tasks_.Push([this, s] { control_stack_.Push(s); });
}

void Validator::CheckSwizzle(const Swizzle* s) {
    if (!CheckResultsAndOperands(s, Swizzle::kNumResults, Swizzle::kNumOperands)) {
        return;
    }

    auto indices = s->Indices();
    if (indices.Length() < Swizzle::kMinNumIndices) {
        AddError(s) << "expected at least " << Swizzle::kMinNumIndices << " indices";
    }

    if (indices.Length() > Swizzle::kMaxNumIndices) {
        AddError(s) << "expected at most " << Swizzle::kMaxNumIndices << " indices";
    }

    for (auto& idx : indices) {
        if (idx > Swizzle::kMaxIndexValue) {
            AddError(s) << "invalid index value";
        }
    }
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
        [&](const ir::Unreachable* u) { CheckUnreachable(u); },      //
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
    if (!CheckResultsAndOperandRange(ret, Return::kNumResults, Return::kMinOperands,
                                     Return::kMaxOperands)) {
        return;
    }

    auto* func = ret->Func();
    if (func == nullptr) {
        // Func() returning nullptr after CheckResultsAndOperandRange is due to the first
        // operand being not a function
        AddError(ret) << "expected function for first operand";
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
            AddError(ret) << "return value type " << NameOf(ret->Value()->Type())
                          << " does not match function return type " << NameOf(func->ReturnType());
        }
    }
}

void Validator::CheckUnreachable(const Unreachable* u) {
    CheckResultsAndOperands(u, Unreachable::kNumResults, Unreachable::kNumOperands);
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
    if (!CheckResultsAndOperands(l, Load::kNumResults, Load::kNumOperands)) {
        return;
    }

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
    if (!CheckResultsAndOperands(s, Store::kNumResults, Store::kNumOperands)) {
        return;
    }

    if (auto* from = s->From()) {
        if (auto* to = s->To()) {
            auto* mv = As<core::type::MemoryView>(to->Type());
            if (!mv) {
                AddError(s, Store::kToOperandOffset) << "store target operand is not a memory view";
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
    if (!CheckResultsAndOperands(l, LoadVectorElement::kNumResults,
                                 LoadVectorElement::kNumOperands)) {
        return;
    }

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
    if (!CheckResultsAndOperands(s, StoreVectorElement::kNumResults,
                                 StoreVectorElement::kNumOperands)) {
        return;
    }

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
    if (DAWN_UNLIKELY(!operand)) {
        return nullptr;
    }

    auto* type = operand->Type();
    if (DAWN_UNLIKELY(!type)) {
        return nullptr;
    }

    auto* memory_view_ty = type->As<core::type::MemoryView>();
    if (DAWN_LIKELY(memory_view_ty)) {
        auto* vec_ty = memory_view_ty->StoreType()->As<core::type::Vector>();
        if (DAWN_LIKELY(vec_ty)) {
            return vec_ty->Type();
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
    std::cout << "=========================================================\n";
    std::cout << "== IR dump before " << msg << ":\n";
    std::cout << "=========================================================\n";
    printer->Print(Disassembler(ir).Text());
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
