// Copyright 2026 The Dawn & Tint Authors
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

#include "src/tint/lang/core/ir/functional_validator.h"

#include "src/tint/utils/rtti/switch.h"
#include "src/tint/utils/text/styled_text.h"

namespace tint::core::ir::validator {

Functional::Functional(const Module& ir, diag::List& diagnostics, ErrorSource error_source)
    : ir_(ir), diag_(diagnostics), error_source_(error_source) {}

Functional::~Functional() = default;

void Functional::Validate() {
    CheckRootBlock(ir_.root_block);
}

Disassembler& Functional::Disassemble() {
    TINT_ASSERT(error_source_ == ErrorSource::kIr);

    if (!disassembler_) {
        disassembler_.emplace(ir::Disassembler(ir_));
    }
    return *disassembler_;
}

StyledText Functional::NameOf(const core::type::Type* ty) {
    auto name = ty ? ty->FriendlyName() : "undef";
    return StyledText{} << style::Type(name);
}

diag::Diagnostic& Functional::AddError(Source src) {
    auto& diag = diag_.AddError(src);
    if (error_source_ == ErrorSource::kIr) {
        diag.owned_file = Disassemble().File();
    }
    return diag;
}

diag::Diagnostic& Functional::AddError(const Instruction* inst) {
    if (error_source_ == ErrorSource::kWgsl) {
        return AddError(ir_.SourceOf(inst));
    }

    auto src = Disassemble().InstructionSource(inst);
    auto& diag = AddError(src) << inst->FriendlyName() << ": ";

    if (!block_stack_.IsEmpty()) {
        AddNote(block_stack_.Back()) << "in block";

        // Adding the note may trigger a resize and invalidate the error diagnostic reference,
        // so we need to get a new reference to the error diagnostic here.
        return *(diag_.end() - 2);
    }
    return diag;
}

diag::Diagnostic& Functional::AddNote(Source src) {
    auto& diag = diag_.AddNote(src);
    diag.owned_file = Disassemble().File();
    return diag;
}

diag::Diagnostic& Functional::AddNote(const Block* blk) {
    auto src = Disassemble().BlockSource(blk);
    return AddNote(src);
}

void Functional::CheckRootBlock(const Block* blk) {
    block_stack_.Push(blk);
    TINT_DEFER(block_stack_.Pop());

    for (auto* inst : *blk) {
        CheckInstruction(inst);
    }
}

void Functional::CheckInstruction(const Instruction* inst) {
    tint::Switch(inst,                                         //
                 [&](const Override* o) { CheckOverride(o); }  //
                 // TODO(516717234): Add TINT_ICE_ON_NO_MATCH when all instructions covered
    );
}

void Functional::CheckOverride(const Override* o) {
    if (o->OverrideId().has_value()) {
        if (!seen_override_ids_.Add(o->OverrideId().value())) {
            AddError(o) << "duplicate override id encountered: " << o->OverrideId().value().value;
            return;
        }
    }

    if (!o->Result()->Type()->IsScalar()) {
        AddError(o) << "override type " << NameOf(o->Result()->Type()) << " is not a scalar";
        return;
    }

    if (o->Initializer() && o->Initializer()->Type() != o->Result()->Type()) {
        AddError(o) << "override type " << NameOf(o->Result()->Type())
                    << " does not match initializer type " << NameOf(o->Initializer()->Type());
        return;
    }

    if (!o->OverrideId().has_value() && (o->Initializer() == nullptr)) {
        AddError(o) << "must have an id or an initializer";
        return;
    }
}

}  // namespace tint::core::ir::validator
