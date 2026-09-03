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

#include "src/tint/lang/core/ir/validator/validator.h"

#include <algorithm>

#include "src/tint/lang/core/ir/constant.h"
#include "src/tint/lang/core/ir/constexpr_if.h"
#include "src/tint/lang/core/ir/disassembler.h"
#include "src/tint/lang/core/ir/multi_in_block.h"
#include "src/tint/lang/core/ir/referenced_functions.h"
#include "src/tint/lang/core/ir/unused.h"
#include "src/tint/lang/core/ir/validator/functional_validator.h"
#include "src/tint/lang/core/type/pointer.h"
#include "src/tint/lang/core/type/reference.h"
#include "src/tint/lang/core/type/void.h"
#include "src/tint/utils/ice/ice.h"
#include "src/tint/utils/internal_limits.h"
#include "src/tint/utils/macros/defer.h"
#include "src/tint/utils/rtti/switch.h"
#include "src/tint/utils/text/text_style.h"

using namespace tint::core::fluent_types;  // NOLINT

#define TINT_CHECK_ERRORS()           \
    do {                              \
        if (diag_.ContainsErrors()) { \
            return;                   \
        }                             \
    } while (false)

namespace tint::core::ir::validator {

Validator::Validator(Module& mod, ErrorSource error_source)
    : ir_(mod), error_source_(error_source), referenced_module_vars_(ir_) {}

bool Validator::IsWGSLValidation() const {
    return error_source_ == ErrorSource::kWgsl;
}

bool Validator::IsIRValidation() const {
    return error_source_ == ErrorSource::kIr;
}

Disassembler& Validator::Disassemble() {
    if (!disassembler_) {
        disassembler_.emplace(ir::Disassembler(ir_));
    }
    return *disassembler_;
}

Source Validator::SourceOf(const Function* func) {
    if (IsWGSLValidation()) {
        return ir_.SourceOf(func);
    }
    return Disassemble().FunctionSource(func);
}

Source Validator::SourceOf(const FunctionParam* param) {
    if (IsWGSLValidation()) {
        return ir_.SourceOf(param);
    }
    return Disassemble().FunctionParamSource(param);
}

Source Validator::SourceOf(const Instruction* inst) {
    if (IsWGSLValidation()) {
        return ir_.SourceOf(inst);
    }
    return Disassemble().InstructionSource(inst);
}

Source Validator::SourceOf(const Instruction* inst, size_t idx) {
    if (IsWGSLValidation()) {
        return ir_.SourceOf(inst->Operands()[idx]);
    }
    return Disassemble().OperandSource(
        Disassembler::IndexedValue{inst, static_cast<uint32_t>(idx)});
}

diag::Diagnostic& Validator::AddError(const Instruction* inst) {
    auto& diag = AddError(SourceOf(inst)) << inst->FriendlyName() << ": ";

    if (IsIRValidation() && !block_stack_.IsEmpty()) {
        AddNote(block_stack_.Back()) << "in block";

        // Adding the note may trigger a resize and invalidate the error diagnostic reference, so we
        // need to get a new reference to the error diagnostic here.
        return *(diag_.end() - 2);
    }
    return diag;
}

diag::Diagnostic& Validator::AddError(const Instruction* inst, size_t idx) {
    auto& diag = AddError(SourceOf(inst, idx)) << inst->FriendlyName() << ": ";

    if (IsIRValidation() && !block_stack_.IsEmpty()) {
        AddNote(block_stack_.Back()) << "in block";

        // Adding the note may trigger a resize and invalidate the error diagnostic reference,
        // so we need to get a new reference to the error diagnostic here.
        return *(diag_.end() - 2);
    }
    return diag;
}

diag::Diagnostic& Validator::AddError(const InstructionResult* inst) {
    return AddError(inst->Instruction());
}

diag::Diagnostic& Validator::AddError(const Block* blk) {
    TINT_IR_ASSERT(ir_, IsIRValidation());
    auto src = Disassemble().BlockSource(blk);
    return AddError(src);
}

diag::Diagnostic& Validator::AddError(const BlockParam* param) {
    TINT_IR_ASSERT(ir_, IsIRValidation());
    auto src = Disassemble().BlockParamSource(param);
    return AddError(src);
}

diag::Diagnostic& Validator::AddError(const Function* func) {
    return AddError(SourceOf(func));
}

diag::Diagnostic& Validator::AddError(const FunctionParam* param) {
    return AddError(SourceOf(param));
}

diag::Diagnostic& Validator::AddError(const Value* param) {
    diag::Diagnostic* diag = nullptr;
    tint::Switch(
        param,  //
        [&](const InstructionResult* r) { diag = &AddError(r); },
        [&](const Function* f) { diag = &AddError(f); },
        [&](const FunctionParam* f) { diag = &AddError(f); },
        [&](const BlockParam* b) { diag = &AddError(b); },  //
        TINT_ICE_ON_NO_MATCH);
    TINT_IR_ASSERT(ir_, diag);
    return *diag;
}

diag::Diagnostic& Validator::AddError(Source src) {
    auto& diag = diag_.AddError(src);
    if (IsIRValidation()) {
        diag.owned_file = Disassemble().File();
    }
    return diag;
}

diag::Diagnostic& Validator::AddResultError(const Instruction* inst, size_t idx) {
    TINT_IR_ASSERT(ir_, IsIRValidation());
    auto src =
        Disassemble().ResultSource(Disassembler::IndexedValue{inst, static_cast<uint32_t>(idx)});
    auto& diag = AddError(src) << inst->FriendlyName() << ": ";

    if (IsIRValidation() && !block_stack_.IsEmpty()) {
        AddNote(block_stack_.Back()) << "in block";

        // Adding the note may trigger a resize and invalidate the error diagnostic reference, so we
        // need to get a new reference to the error diagnostic here.
        return *(diag_.end() - 2);
    }
    return diag;
}

diag::Diagnostic& Validator::AddNote(const Instruction* inst) {
    return AddNote(SourceOf(inst));
}

diag::Diagnostic& Validator::AddNote(const Function* func) {
    return AddNote(SourceOf(func));
}

diag::Diagnostic& Validator::AddOperandNote(const Instruction* inst, size_t idx) {
    return AddNote(SourceOf(inst, idx));
}

diag::Diagnostic& Validator::AddResultNote(const Instruction* inst, size_t idx) {
    TINT_IR_ASSERT(ir_, IsIRValidation());
    auto src =
        Disassemble().ResultSource(Disassembler::IndexedValue{inst, static_cast<uint32_t>(idx)});
    return AddNote(src);
}

diag::Diagnostic& Validator::AddNote(const Block* blk) {
    TINT_IR_ASSERT(ir_, IsIRValidation());
    auto src = Disassemble().BlockSource(blk);
    return AddNote(src);
}

diag::Diagnostic& Validator::AddNote(Source src) {
    auto& diag = diag_.AddNote(src);
    if (IsIRValidation()) {
        diag.owned_file = Disassemble().File();
    }
    return diag;
}

void Validator::AddDeclarationNote(const Block* block) {
    TINT_IR_ASSERT(ir_, IsIRValidation());
    auto src = Disassemble().BlockSource(block);
    if (src.file) {
        AddNote(src) << NameOf(block) << " declared here";
    }
}

void Validator::AddDeclarationNote(const BlockParam* param) {
    TINT_IR_ASSERT(ir_, IsIRValidation());
    auto src = Disassemble().BlockParamSource(param);
    if (src.file) {
        AddNote(src) << NameOf(param) << " declared here";
    }
}

void Validator::AddDeclarationNote(const Function* fn) {
    AddNote(fn) << NameOf(fn) << " declared here";
}

void Validator::AddDeclarationNote(const FunctionParam* param) {
    auto src = SourceOf(param);
    if (src.file) {
        AddNote(src) << NameOf(param) << " declared here";
    }
}

void Validator::AddDeclarationNote(const Instruction* inst) {
    auto src = SourceOf(inst);
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

void Validator::AddDeclarationNote(const Value* res) {
    tint::Switch(
        res,  //
        [&](const InstructionResult* r) { AddDeclarationNote(r); },
        [&](const Function* f) { AddDeclarationNote(f); },
        [&](const FunctionParam* f) { AddDeclarationNote(f); },
        [&](const BlockParam* b) { AddDeclarationNote(b); },  //
        TINT_ICE_ON_NO_MATCH);
}

StyledText Validator::NameOf(const core::type::Type* ty) {
    auto name = ty ? ty->FriendlyName() : "undef";
    return StyledText{} << style::Type(name);
}

StyledText Validator::NameOf(const Value* value) {
    if (IsWGSLValidation()) {
        return StyledText{} << ir_.NameOf(value).to_str();
    }
    return Disassemble().NameOf(value);
}

StyledText Validator::NameOf(const Instruction* inst) {
    auto name = inst ? inst->FriendlyName() : "undef";
    return StyledText{} << style::Instruction(name);
}

StyledText Validator::NameOf(const Block* block) {
    TINT_IR_ASSERT(ir_, IsIRValidation());
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

    if (DAWN_UNLIKELY(result->Instruction() != inst)) {
        AddResultError(inst, idx)
            << "result instruction does not match instruction (possible double usage)";
        return false;
    }

    if (!inst->Is<Call>() && result->Type()->Is<core::type::Void>()) {
        AddResultError(inst, idx) << "result type cannot be void";
        return false;
    }

    if (inst->Is<ControlInstruction>()) {
        if (result->Type()->Is<core::type::Pointer>()) {
            AddResultError(inst, idx) << "result type cannot be a pointer";
            return false;
        }
        if (!result->Type()->IsConstructible()) {
            AddResultError(inst, idx) << "result type must be constructable";
            return false;
        }
    }

    if (result->Type()->Is<core::type::Void>() && ir_.NameOf(result)) {
        AddResultError(inst, idx) << "void results must not have names";
        return false;
    }

    const core::type::Type* ty = result->Type();
    bool check_size = false;

    if (auto* mv = ty->As<core::type::MemoryView>()) {
        if (mv->AddressSpace() == core::AddressSpace::kFunction ||
            mv->AddressSpace() == core::AddressSpace::kPrivate) {
            check_size = true;
            ty = mv->StoreType();
        }
    } else {
        check_size = true;
    }

    if (check_size) {
        if (ty->Size() > tint::internal_limits::kMaxTemporaryStorageSize) {
            AddResultError(inst, idx)
                << "result type size (" << ty->Size() << ") exceeds maximum allowed ("
                << tint::internal_limits::kMaxTemporaryStorageSize << ")";
            return false;
        }
    }

    return true;
}

bool Validator::CheckResults(const ir::Instruction* inst, std::optional<size_t> count) {
    if (count.has_value()) {
        if (DAWN_UNLIKELY(inst->Results().Length() != count.value())) {
            AddError(inst) << "expected exactly " << count.value() << " results, got "
                           << inst->Results().Length();
            return false;
        }
    }

    bool passed = true;
    Hashset<const InstructionResult*, 4> seen_instruction_results;
    for (size_t i = 0; i < inst->Results().Length(); i++) {
        if (DAWN_UNLIKELY(!CheckResult(inst, i))) {
            passed = false;
        }

        if (!seen_instruction_results.Add(inst->Result(i))) {
            AddResultError(inst, i) << "result was seen previously as a result";
            passed = false;
        }
    }
    return passed;
}

bool Validator::CheckResultsAndOperandRange(const ir::Instruction* inst,
                                            size_t num_results,
                                            size_t min_operands,
                                            std::optional<size_t> max_operands) {
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

bool Validator::CheckOperand(const Instruction* inst, size_t idx) {
    auto* operand = inst->Operand(idx);

    if (DAWN_UNLIKELY(operand == nullptr)) {
        // var and override instructions are allowed to have a nullptr initializers.
        // terminator instructions use nullptr operands to signal 'undef'.
        if (inst->IsAnyOf<Terminator, Var, Override>()) {
            return true;
        }

        AddError(inst, idx) << "operand is undefined";
        return false;
    }

    // ir::Unused is a internal value used by some transforms to track unused entries, and is
    // removed as part of generating an output shader.
    if (DAWN_UNLIKELY(operand->Is<ir::Unused>())) {
        return true;
    }

    if (DAWN_UNLIKELY(operand->Type() == nullptr)) {
        AddError(inst, idx) << "operand type is undefined";
        return false;
    }

    if (DAWN_UNLIKELY(!operand->Alive())) {
        AddError(inst, idx) << "operand is not alive";
        return false;
    }

    if (operand->Type() && !operand->Type()->Is<core::type::MemoryView>()) {
        if (operand->Type()->Size() > tint::internal_limits::kMaxTemporaryStorageSize) {
            AddError(inst, idx) << "operand size (" << operand->Type()->Size()
                                << ") exceeds maximum allowed ("
                                << tint::internal_limits::kMaxTemporaryStorageSize << ")";
            return false;
        }
    }

    if (DAWN_UNLIKELY(operand->Is<Constant>() &&
                      operand->Type()->Is<core::type::SubgroupMatrix>())) {
        AddError(inst, idx) << "subgroup_matrix values cannot be constant";
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

bool Validator::CheckOperands(const ir::Instruction* inst, std::optional<size_t> count) {
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

void Validator::CheckOperandsMatchTarget(const Instruction* source_inst,
                                         size_t source_operand_offset,
                                         size_t source_operand_count,
                                         const MultiInBlock* target,
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
                << "operand with type " << NameOf(source_type) << " does not match "
                << NameOf(target) << " target type " << NameOf(target_type);
            AddDeclarationNote(target_value);
        }
    }
}

void Validator::CheckOperandsMatchTarget(const Instruction* source_inst,
                                         size_t source_operand_offset,
                                         size_t source_operand_count,
                                         const ControlInstruction* target,
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
                << "operand with type " << NameOf(source_type) << " does not match "
                << NameOf(target) << " target type " << NameOf(target_type);
            AddDeclarationNote(target_value);
        }
    }
}

void Validator::QueueTasks(std::function<void()> begin,
                           std::function<void()> mid,
                           std::function<void()> end) {
    tasks_.Push(end);
    tasks_.Push(mid);
    tasks_.Push(begin);
}

std::function<void()> Validator::QueueNestedTasks(std::function<void()> begin,
                                                  std::function<void()> mid,
                                                  std::function<void()> end) {
    return [this, begin, mid, end] { QueueTasks(begin, mid, end); };
}

void Validator::QueueBlock(const Block* blk) {
    QueueTasks([this, blk] { BeginBlock(blk); }, [] {}, [this] { EndBlock(); });
}

void Validator::QueueInstructions(const Instruction* inst) {
    TINT_CHECK_ERRORS();

    // Note, the ordering here is very specific. The `CheckInstruction` will push both more control
    // blocks but also result validation. So, you can change the ordering of the tasks if you change
    // the ordering of these calls.
    tasks_.Push([this, inst] {
        // Tasks are processed LIFO, so push the next instruction to the stack before checking the
        // current instruction, which may need to add more blocks to the stack itself.
        if (inst->next) {
            QueueInstructions(inst->next);
        }
        CheckInstruction(inst);
    });
}

std::function<void()> Validator::PushControlStack(const ControlInstruction* ctrl) {
    return [this, ctrl] { control_stack_.Push(ctrl); };
}

std::function<void()> Validator::PopControlStack() {
    return [this] { control_stack_.Pop(); };
}

std::function<void()> Validator::BeginBlockTask(const Block* blk) {
    return [this, blk] {
        if (!blk->IsEmpty()) {
            BeginBlock(blk);
        }
    };
}

std::function<void()> Validator::EndBlockTask(const Block* blk) {
    return [this, blk] {
        if (!blk->IsEmpty()) {
            EndBlock();
        }
    };
}

void Validator::ProcessTasks() {
    while (!tasks_.IsEmpty()) {
        tasks_.Pop()();
    }
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

            CheckType(param->Type(), [&]() -> diag::Diagnostic& { return AddError(param); });

            if (param->Type()->Is<core::type::Void>()) {
                AddError(param) << "block parameter type cannot be void";
            }
            if (param->Type()->Is<core::type::Reference>()) {
                AddError(param) << "block parameter type cannot be a reference";
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

const ir::Function* Validator::ContainingFunction(const ir::Instruction* inst) {
    if (inst->Block() == ir_.root_block) {
        return nullptr;
    }

    return block_to_function_.GetOrAdd(inst->Block(), [&] {  //
        return ContainingFunction(inst->Block()->Parent());
    });
}

Hashset<const ir::Function*, 4> Validator::ContainingEndPoints(const ir::Function* f) {
    if (!f) {
        return {};
    }

    Hashset<const ir::Function*, 4> result{};
    Hashset<const ir::Function*, 4> visited{f};

    auto call_sites = user_func_calls_.GetOr(f, Hashset<const ir::UserCall*, 4>()).Vector();
    while (!call_sites.IsEmpty()) {
        auto call_site = call_sites.Pop();
        auto calling_function = ContainingFunction(call_site);
        if (!calling_function) {
            continue;
        }

        if (visited.Contains(calling_function)) {
            continue;
        }
        visited.Add(calling_function);

        if (calling_function->IsEntryPoint()) {
            result.Add(calling_function);
        }

        for (auto new_call_sites : user_func_calls_.GetOr(f, Hashset<const ir::UserCall*, 4>())) {
            call_sites.Push(new_call_sites);
        }
    }

    return result;
}

Result<SuccessType> Validator::Run() {
    RunStructuralSoundnessChecks();

    CheckForRecursion();
    CheckForOrphanedInstructions();
    CheckStageRestrictedInstructions();

    // Only run the functional validation if we are structurally valid
    if (!diag_.ContainsErrors()) {
        validator::Functional f(ir_, diag_, error_source_);
        f.Validate();
    }

    if (diag_.ContainsErrors()) {
        if (IsIRValidation()) {
            const StyledText disassembly = ir::Disassembler(ir_).Text();
            diag_.AddNote(Source{}) << "# Disassembly\n" << disassembly;
        }
        return Failure{diag_.Str()};
    }
    return Success;
}

void Validator::RunStructuralSoundnessChecks() {
    {
        scope_stack_.Push();
        TINT_DEFER(scope_stack_.Pop());

        CheckRootBlock(ir_.root_block);

        for (auto& func : ir_.functions) {
            if (!all_functions_.Add(func)) {
                AddError(func) << "function " << NameOf(func) << " added to module multiple times";
            }
            scope_stack_.Add(func);
        }

        for (auto& func : ir_.functions) {
            block_to_function_.Add(func->Block(), func);
            CheckFunction(func);
        }
    }

    TINT_ASSERT(scope_stack_.IsEmpty());
    TINT_ASSERT(tasks_.IsEmpty());
    TINT_ASSERT(control_stack_.IsEmpty());
    TINT_ASSERT(block_stack_.IsEmpty());
}

void Validator::CheckForRecursion() {
    TINT_CHECK_ERRORS();

    ReferencedFunctions<const Module> referenced_functions(ir_);
    for (auto& func : ir_.functions) {
        auto& refs = referenced_functions.TransitiveReferences(func);
        if (refs.Contains(func)) {
            // TODO(434684891): Consider improving this error with more information.
            AddError(func) << "recursive function calls are not allowed";
            return;
        }
    }
}

void Validator::CheckForOrphanedInstructions() {
    TINT_CHECK_ERRORS();

    // Check for orphaned instructions.
    for (auto* inst : ir_.Instructions()) {
        if (!visited_instructions_.Contains(inst)) {
            AddError(inst) << "orphaned instruction: " << inst->FriendlyName();
        }
    }
}

void Validator::CheckStageRestrictedInstructions() {
    TINT_CHECK_ERRORS();

    // Check for instructions being used in stages that do not support them.
    for (const auto& i : stage_restricted_instructions_) {
        const auto& inst = i.key;
        const auto& stages = i.value;
        const auto* f = ContainingFunction(inst);
        if (f == nullptr) {
            continue;
        }

        if (f->IsEntryPoint() && !stages.Contains(f->Stage())) {
            AddError(inst) << "cannot be used in a " << f->Stage() << " shader";
        } else {
            for (const Function* ep : ContainingEndPoints(f)) {
                if (!stages.Contains(ep->Stage())) {
                    AddError(inst) << "cannot be used in a " << ep->Stage() << " shader";
                }
            }
        }
    }
}

void Validator::CheckRootBlock(const Block* blk) {
    block_stack_.Push(blk);
    TINT_DEFER(block_stack_.Pop());

    Hashset<const Value*, 8> pipeline_evaluatable{};

    auto add_evaluatable = [&](const Instruction* inst, const bool is_creatable) {
        if (auto* res = inst->Result(0); res != nullptr && is_creatable) {
            pipeline_evaluatable.Add(res);
        }
    };

    for (auto* inst : *blk) {
        if (inst->Block() != blk) {
            AddError(inst) << "instruction in root block does not have root block as parent";
            continue;
        }

        auto is_pipeline_creatable = true;
        for (auto* op : inst->Operands()) {
            if (!op) {
                continue;
            }
            if (op->Is<Constant>()) {
                continue;
            }
            if (pipeline_evaluatable.Contains(op)) {
                continue;
            }
            is_pipeline_creatable = false;
            break;
        }

        if (!is_pipeline_creatable) {
            AddError(inst) << "instruction is not evaluatable at pipeline creation time";
        }

        tint::Switch(
            inst,  //
            [&](const Override* o) {
                if (ir_.properties.Contains(Property::kAllowOverrides)) {
                    CheckInstruction(o);
                    add_evaluatable(o, is_pipeline_creatable);
                } else {
                    AddError(inst) << "root block: invalid instruction: " << inst->TypeInfo().name;
                }
            },
            [&](const Var* var) { CheckInstruction(var); },
            [&](const Let* let) {
                if (ir_.properties.Contains(Property::kAllowModuleScopeLets)) {
                    CheckInstruction(let);
                    add_evaluatable(let, is_pipeline_creatable);
                } else {
                    AddError(inst) << "root block: invalid instruction: " << inst->TypeInfo().name;
                }
            },
            [&](const Construct* c) {
                if (ir_.properties.Contains(Property::kAllowModuleScopeLets) ||
                    ir_.properties.Contains(Property::kAllowOverrides)) {
                    CheckInstruction(c);
                    CheckOnlyUsedInRootBlock(inst);
                    add_evaluatable(c, is_pipeline_creatable);
                } else {
                    AddError(inst) << "root block: invalid instruction: " << inst->TypeInfo().name;
                }
            },
            [&](Default) {
                // Note, this validation around kAllowOverrides is looser than it could be. There
                // are only certain expressions and builtins which can be used in an override, which
                // currently isn't checked.
                if (ir_.properties.Contains(Property::kAllowOverrides) &&
                    inst->IsAnyOf<Unary, Binary, BuiltinCall, Convert, Swizzle, Access,
                                  ConstExprIf>()) {
                    CheckInstruction(inst);
                    // If overrides are allowed we can have certain regular instructions in the root
                    // block, with the caveat that those instructions can _only_ be used in the root
                    // block.
                    CheckOnlyUsedInRootBlock(inst);
                    add_evaluatable(inst, is_pipeline_creatable);
                } else {
                    AddError(inst) << "root block: invalid instruction: " << inst->TypeInfo().name;
                }
            });

        // Process tasks queued by CheckInstruction (like AddResults) before moving to next
        // instruction.
        ProcessTasks();
    }
}

void Validator::CheckOnlyUsedInRootBlock(const Instruction* inst) {
    if (inst->Result(0)) {
        for (auto& usage : inst->Result(0)->UsagesSorted()) {
            if (usage.instruction->Block() != ir_.root_block) {
                AddError(inst) << "root block: instruction used outside of root block "
                               << inst->TypeInfo().name;
            }
        }
    }

    CheckInstruction(inst);
}

bool Validator::CanLoad(const core::type::Type* ty) {
    return tint::Switch(
        ty,  //
        [&](const core::type::Array* arr) {
            if (arr->Count()->Is<core::type::RuntimeArrayCount>()) {
                return false;
            }
            return CanLoad(arr->Elements().type);
        },
        [&](const core::type::Struct* str) {
            for (auto* member : str->Members()) {
                if (member->Type()->Is<core::type::Pointer>() &&
                    ir_.properties.Contains(Property::kAllowMslEntryPointInterface)) {
                    continue;
                }
                if (!CanLoad(member->Type())) {
                    return false;
                }
            }
            return true;
        },
        [&](Default) { return ty->IsConstructible() || ty->IsHandle(); });
}

}  // namespace tint::core::ir::validator
