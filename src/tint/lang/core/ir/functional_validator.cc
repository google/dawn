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

#include "src/tint/lang/core/ir/multi_in_block.h"
#include "src/tint/lang/core/ir/type/array_count.h"
#include "src/tint/lang/core/type/array.h"
#include "src/tint/lang/core/type/f16.h"
#include "src/tint/lang/core/type/memory_view.h"
#include "src/tint/utils/rtti/switch.h"
#include "src/tint/utils/text/styled_text.h"

namespace tint::core::ir::validator {
namespace {

template <typename CTX, typename IMPL>
void WalkTypeAndMembers(CTX& ctx,
                        const core::type::Type* type,
                        const IOAttributes& attr,
                        IMPL&& impl);

/// Helper that walks the members of a struct, called from WalkTypeAndMembers and its helpers
/// @param ctx a context object to pass to the impl function
/// @param str the struct to walk the members of
/// @param impl an impl function to be run, see WalkTypeAndMembers for details
template <typename CTX, typename IMPL>
void WalkStructMembers(CTX& ctx, const core::type::Struct* str, IMPL&& impl) {
    for (auto* member : str->Members()) {
        WalkTypeAndMembers(ctx, member->Type(), member->Attributes(), impl);
    }
}

/// Helper that walks an array's element type, called from WalkTypeAndMembers and its helpers
/// @param ctx a context object to pass to the impl function
/// @param arr the array to walk the element type of
/// @param impl an impl function to be run, see WalkTypeAndMembers for details
template <typename CTX, typename IMPL>
void WalkArrayElements(CTX& ctx, const core::type::Array* arr, IMPL&& impl) {
    WalkTypeAndMembers(ctx, arr->ElemType(), IOAttributes{}, impl);
}

/// Helper for walking a type that maybe a struct, calling an impl function for the type and each of
/// its members.
/// @param ctx a context object to pass to the implementation function
/// @param type the type to walk
/// @param attr the attributes for @p type
/// @param impl a function with the signature `void(const core::type::Type*, const IOAttributes&,
///             CTX&)` that is called for each type.
template <typename CTX, typename IMPL>
void WalkTypeAndMembers(CTX& ctx,
                        const core::type::Type* type,
                        const IOAttributes& attr,
                        IMPL&& impl) {
    impl(ctx, type, attr);
    tint::Switch(
        type, [&](const core::type::Struct* s) { WalkStructMembers(ctx, s, impl); },
        [&](const core::type::Array* a) { WalkArrayElements(ctx, a, impl); });
}

/// @returns true if the type or any contained types are of type T
/// @param ty root of the types to walk
template <typename T>
bool ContainsType(const core::type::Type* ty) {
    bool found = false;
    WalkTypeAndMembers(found, ty, IOAttributes{},
                       [&](bool& ctx, const core::type::Type* t, const IOAttributes&) {
                           if (t != nullptr && t->DeepestElement()->Is<T>()) {
                               ctx = true;
                           }
                       });
    return found;
}

}  // namespace

Functional::Functional(const Module& ir, diag::List& diagnostics, ErrorSource error_source)
    : ir_(ir), diag_(diagnostics), error_source_(error_source) {}

Functional::~Functional() = default;

void Functional::Validate() {
    CheckRootBlock(ir_.root_block);

    for (const Function* func : ir_.functions) {
        CheckFunction(func);
    }
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

StyledText Functional::NameOf(const Value* value) {
    if (error_source_ == ErrorSource::kWgsl) {
        return StyledText{} << ir_.NameOf(value).to_str();
    }
    return Disassemble().NameOf(value);
}

Source Functional::SourceOf(const Instruction* inst) {
    if (error_source_ == ErrorSource::kWgsl) {
        return ir_.SourceOf(inst);
    }
    return Disassemble().InstructionSource(inst);
}

diag::Diagnostic& Functional::AddError(Source src) {
    auto& diag = diag_.AddError(src);
    if (error_source_ == ErrorSource::kIr) {
        diag.owned_file = Disassemble().File();
    }
    return diag;
}

diag::Diagnostic& Functional::AddError(const Instruction* inst) {
    auto& diag = AddError(SourceOf(inst));
    if (error_source_ == ErrorSource::kWgsl) {
        return diag;
    }

    diag << inst->FriendlyName() << ": ";
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
    scope_stack_.Push();
    TINT_DEFER({
        scope_stack_.Pop();
        block_stack_.Pop();
        TINT_ASSERT(block_stack_.IsEmpty());
        TINT_ASSERT(scope_stack_.IsEmpty());
    });

    for (auto* inst : *blk) {
        CheckInstruction(inst);
    }
}

void Functional::CheckFunction(const Function* func) {
    scope_stack_.Push();
    TINT_DEFER(scope_stack_.Pop());

    for (auto* param : func->Params()) {
        scope_stack_.Add(param);
    }

    CheckBlock(func->Block());
}

void Functional::CheckBlock(const Block* blk) {
    block_stack_.Push(blk);
    scope_stack_.Push();
    TINT_DEFER({
        scope_stack_.Pop();
        block_stack_.Pop();
    });

    if (auto* mb = blk->As<MultiInBlock>()) {
        for (auto* param : mb->Params()) {
            scope_stack_.Add(param);
        }
    }

    const Instruction* inst = blk->Instructions();
    while (inst != nullptr) {
        CheckInstruction(inst);
        inst = inst->next;
    }
}

void Functional::CheckInstruction(const Instruction* inst) {
    tint::Switch(
        inst,                                          //
        [&](const Override* o) { CheckOverride(o); },  //
        [&](const Var* var) { CheckVar(var); }
        // TODO(516717234): Add TINT_ICE_ON_NO_MATCH when all instructions covered
    );

    for (auto* result : inst->Results()) {
        scope_stack_.Add(result);
    }
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

void Functional::CheckVar(const Var* var) {
    auto* result_type = var->Result()->Type();
    auto* mv = result_type->As<core::type::MemoryView>();
    if (!mv) {
        AddError(var) << "result type " << NameOf(result_type)
                      << " must be a pointer or a reference";
        return;
    }

    // Check that initializer and result type match
    if (var->Initializer()) {
        if (mv->AddressSpace() != AddressSpace::kFunction &&
            mv->AddressSpace() != AddressSpace::kPrivate &&
            mv->AddressSpace() != AddressSpace::kOut) {
            AddError(var) << "only variables in the function, private, or __out address space may "
                             "be initialized";
            return;
        }

        if (var->Initializer()->Type() != result_type->UnwrapPtrOrRef()) {
            AddError(var) << "initializer type " << NameOf(var->Initializer()->Type())
                          << " does not match store type " << NameOf(result_type->UnwrapPtrOrRef());
            return;
        }
    }

    if (var->Block() == ir_.root_block && mv->AddressSpace() == AddressSpace::kFunction) {
        AddError(var) << "vars in the 'function' address space must be in a function scope";
        return;
    }
    if (var->Block() != ir_.root_block && mv->AddressSpace() != AddressSpace::kFunction) {
        if (!ir_.properties.Contains(Property::kAllowMslEntryPointInterface) ||
            mv->AddressSpace() != AddressSpace::kPrivate) {
            AddError(var) << "vars in a function scope must be in the 'function' address space";
            return;
        }
    }

    if (mv->AddressSpace() != AddressSpace::kStorage &&
        mv->AddressSpace() != AddressSpace::kHandle) {
        if (mv->AddressSpace() == AddressSpace::kWorkgroup ||
            !ir_.properties.Contains(Property::kAllowMslEntryPointInterface)) {
            if (!mv->StoreType()->HasFixedFootprint()) {
                AddError(var) << "vars not in the 'storage' or 'handle' address spaces "
                                 "must have a fixed footprint";
                return;
            }
        }
    }

    if (ContainsType<core::type::Atomic>(mv->StoreType())) {
        bool is_workgroup = mv->AddressSpace() == AddressSpace::kWorkgroup;
        bool is_read_write_storage = mv->AddressSpace() == AddressSpace::kStorage &&
                                     mv->Access() == core::Access::kReadWrite;
        if (!is_workgroup && !is_read_write_storage) {
            AddError(var)
                << "atomic types may only be used by 'workspace' or read write 'storage' variables";
            return;
        }
    }

    if (var->InputAttachmentIndex().has_value()) {
        if (mv->AddressSpace() != AddressSpace::kHandle) {
            AddError(var) << "'@input_attachment_index' is not valid for non-handle var";
            return;
        }
        if (!ir_.properties.Contains(Property::kAllowAnyInputAttachmentIndexType) &&
            !mv->UnwrapPtrOrRef()->Is<core::type::InputAttachment>()) {
            AddError(var)
                << "'@input_attachment_index' is only valid for 'input_attachment' type var";
            return;
        }
    }

    if (mv->AddressSpace() == AddressSpace::kWorkgroup) {
        if (auto* ary = result_type->UnwrapPtr()->As<core::type::Array>()) {
            if (auto* count = ary->Count()->As<core::ir::type::ValueArrayCount>()) {
                if (!scope_stack_.Contains(count->value)) {
                    AddError(var) << NameOf(count->value) << " is not in scope";
                }
            }
        }
    } else if (mv->AddressSpace() == AddressSpace::kStorage) {
        if (mv->StoreType() && !mv->StoreType()->IsHostShareable()) {
            AddError(var) << "vars in the 'storage' address space must be host-shareable";
            return;
        }
        if (mv->Access() != core::Access::kReadWrite && mv->Access() != core::Access::kRead) {
            AddError(var)
                << "vars in the 'storage' address space must have access 'read' or 'read-write'";
            return;
        }
    } else if (mv->AddressSpace() == AddressSpace::kUniform) {
        if (!(mv->StoreType()->IsConstructible() || mv->StoreType()->Is<core::type::Buffer>()) ||
            !mv->StoreType()->IsHostShareable()) {
            AddError(var) << "vars in the 'uniform' address space must be host-shareable and "
                             "constructible or a buffer";
            return;
        }
    } else if (mv->AddressSpace() == AddressSpace::kImmediate) {
        if (mv->StoreType() && !mv->StoreType()->IsHostShareable()) {
            AddError(var) << "vars in the 'immediate' address space must be host-shareable";
            return;
        }

        if (ContainsType<core::type::F16>(mv->StoreType())) {
            AddError(var) << "vars in the 'immediate' address space cannot contain f16 types";
            return;
        }
    } else if (mv->AddressSpace() == core::AddressSpace::kPixelLocal) {
        if (var->Block() == ir_.root_block) {
            if (!mv->StoreType()->Is<core::type::Struct>()) {
                AddError(var) << "pixel_local var must be of type struct";
                return;
            }
        }
    }
}

}  // namespace tint::core::ir::validator
