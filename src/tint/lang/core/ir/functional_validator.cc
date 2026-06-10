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

#include <utility>

#include "src/tint/lang/core/intrinsic/dialect.h"
#include "src/tint/lang/core/intrinsic/table.h"
#include "src/tint/lang/core/ir/multi_in_block.h"
#include "src/tint/lang/core/ir/phony.h"
#include "src/tint/lang/core/ir/type/array_count.h"
#include "src/tint/lang/core/ir/unused.h"
#include "src/tint/lang/core/type/array.h"
#include "src/tint/lang/core/type/bool.h"
#include "src/tint/lang/core/type/f16.h"
#include "src/tint/lang/core/type/i32.h"
#include "src/tint/lang/core/type/i8.h"
#include "src/tint/lang/core/type/matrix.h"
#include "src/tint/lang/core/type/memory_view.h"
#include "src/tint/lang/core/type/pointer.h"
#include "src/tint/lang/core/type/reference.h"
#include "src/tint/lang/core/type/u32.h"
#include "src/tint/lang/core/type/u8.h"
#include "src/tint/lang/core/type/vector.h"
#include "src/tint/lang/core/type/void.h"
#include "src/tint/utils/containers/transform.h"
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

Source Functional::SourceOf(const Instruction* inst, size_t idx) {
    if (error_source_ == ErrorSource::kWgsl) {
        return ir_.SourceOf(inst->Operands()[idx]);
    }
    return Disassemble().OperandSource(
        Disassembler::IndexedValue{inst, static_cast<uint32_t>(idx)});
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

diag::Diagnostic& Functional::AddError(const Instruction* inst, size_t idx) {
    auto& diag = AddError(SourceOf(inst, idx));
    if (error_source_ == ErrorSource::kWgsl) {
        return diag;
    }

    diag << inst->FriendlyName() << ": ";

    if (!block_stack_.IsEmpty()) {
        AddNote(block_stack_.Back()) << "in block";

        // Adding the note may trigger a resize and invalidate the error diagnostic reference, so we
        // need to get a new reference to the error diagnostic here.
        return *(diag_.end() - 2);
    }
    return diag;
}

diag::Diagnostic& Functional::AddNote(Source src) {
    auto& diag = diag_.AddNote(src);
    if (error_source_ == ErrorSource::kIr) {
        diag.owned_file = Disassemble().File();
    }
    return diag;
}

diag::Diagnostic& Functional::AddNote(const Block* blk) {
    TINT_ASSERT(error_source_ == ErrorSource::kIr);
    auto src = Disassemble().BlockSource(blk);
    return AddNote(src);
}

diag::Diagnostic& Functional::AddNote(const Instruction* inst) {
    return AddNote(SourceOf(inst));
}

diag::Diagnostic& Functional::AddNote(const Instruction* inst, size_t idx) {
    return AddNote(SourceOf(inst, idx));
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
        inst,                                                              //
        [&](const Access* a) { CheckAccess(a); },                          //
        [&](const Binary* b) { CheckBinary(b); },                          //
        [&](const Call* c) { CheckCall(c); },                              //
        [&](const If* if_) { CheckIf(if_); },                              //
        [&](const Let* l) { CheckLet(l); },                                //
        [&](const Load* load) { CheckLoad(load); },                        //
        [&](const LoadVectorElement* l) { CheckLoadVectorElement(l); },    //
        [&](const Loop* l) { CheckLoop(l); },                              //
        [&](const Override* o) { CheckOverride(o); },                      //
        [&](const Phony*) {},                                              //
        [&](const Store* s) { CheckStore(s); },                            //
        [&](const StoreVectorElement* s) { CheckStoreVectorElement(s); },  //
        [&](const Switch* s) { CheckSwitch(s); },                          //
        [&](const Swizzle* s) { CheckSwizzle(s); },                        //
        [&](const Terminator* b) { CheckTerminator(b); },                  //
        [&](const Unary* u) { CheckUnary(u); },                            //
        [&](const Var* var) { CheckVar(var); },                            //
        TINT_ICE_ON_NO_MATCH);

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

void Functional::CheckLet(const Let* l) {
    auto* value_ty = l->Value()->Type();
    auto* result_ty = l->Result()->Type();
    if (value_ty != result_ty) {
        AddError(l) << "result type " << NameOf(l->Result()->Type())
                    << " does not match value type " << NameOf(l->Value()->Type());
    }

    if (ir_.properties.Contains(Property::kAllowAnyLetType)) {
        if (value_ty->Is<core::type::Void>()) {
            AddError(l) << "value type cannot be void";
        }
        return;
    }

    if (!value_ty->IsConstructible() && !value_ty->Is<core::type::Pointer>()) {
        AddError(l) << "value type, " << NameOf(value_ty)
                    << ", must be a concrete constructible type or a pointer type";
    }

    if (auto* ptr = result_ty->As<core::type::Pointer>()) {
        if (ptr->AddressSpace() == AddressSpace::kHandle &&
            !ir_.properties.Contains(Property::kAllowPointerToHandle)) {
            AddError(l) << "handle pointer cannot be captured in a let";
        }
    } else if (!result_ty->IsConstructible()) {
        AddError(l) << "result type, " << NameOf(result_ty)
                    << ", must be a concrete constructible type or a pointer type";
    }
}

void Functional::CheckConstruct(const Construct* construct) {
    auto* result_type = construct->Result()->Type();
    if (!result_type->IsConstructible()) {
        // We only allow `construct` to create non-constructible types when they are structures that
        // contain pointers and handle types, with the corresponding capability enabled.
        if (!(result_type->Is<core::type::Struct>() &&
              ir_.properties.Contains(Property::kAllowMslEntryPointInterface))) {
            AddError(construct) << "type is not constructible";
            return;
        }
    }

    auto args = construct->Args();

    // Zero-value constructors are valid for all constructible types.
    if (args.empty()) {
        return;
    }

    // Check that type type of each argument matches the expected element type of the composite.
    auto check_args_match_elements = [&] {
        for (size_t i = 0; i < args.size(); i++) {
            if (args[i]->Is<ir::Unused>()) {
                continue;
            }
            auto* expected_type = result_type->Element(static_cast<uint32_t>(i));
            if (args[i]->Type() != expected_type) {
                AddError(construct, Construct::kArgsOperandOffset + i)
                    << "type " << NameOf(args[i]->Type()) << " of argument " << i
                    << " does not match expected type " << NameOf(expected_type);
            }
        }
    };

    if (result_type->Is<core::type::Scalar>()) {
        // The only valid non-zero scalar constructor is the identity operation.
        if (args.size() > 1) {
            AddError(construct) << "scalar construct must not have more than one argument";
        }
        if (args[0]->Type() != result_type) {
            AddError(construct, 0u) << "scalar construct argument type " << NameOf(args[0]->Type())
                                    << " does not match result type " << NameOf(result_type);
        }
        return;
    }

    if (auto* sg_mat = result_type->As<core::type::SubgroupMatrix>()) {
        if (args.size() > 1) {
            AddError(construct) << "subgroup matrix construct must not have more than 1 argument";
            return;
        }

        // 8-bit integer matrices use 32-bit shader scalar types in WGSL.
        // Some backends may support 8-bit integers, in which case they would pass an 8-bit
        // type for the constructor value instead.
        const core::type::Type* scalar_ty = sg_mat->Type();
        if (scalar_ty->Is<core::type::I8>()) {
            scalar_ty = type_mgr_.i32();
        } else if (scalar_ty->Is<core::type::U8>()) {
            scalar_ty = type_mgr_.u32();
        }
        if (args[0]->Type() != scalar_ty && args[0]->Type() != sg_mat->Type()) {
            AddError(construct) << "subgroup matrix construct argument type "
                                << NameOf(args[0]->Type())
                                << " does not match matrix shader scalar type "
                                << NameOf(scalar_ty);
        }
        return;
    }

    if (auto* arr = result_type->As<core::type::Array>()) {
        if (args.size() != arr->ConstantCount()) {
            AddError(construct) << "array has " << arr->ConstantCount().value()
                                << " elements, but construct provides " << args.size()
                                << " arguments";
            return;
        }
        check_args_match_elements();
        return;
    }

    if (auto* str = As<core::type::Struct>(result_type)) {
        auto members = str->Members();
        if (args.size() != str->Members().Length()) {
            AddError(construct) << "structure has " << members.Length()
                                << " members, but construct provides " << args.size()
                                << " arguments";
            return;
        }
        check_args_match_elements();
    }

    auto table = intrinsic::Table<intrinsic::Dialect>(type_mgr_, symbols_);
    auto arg_types = Transform<4>(args, [&](auto* v) { return v->Type(); });
    if (auto* vec = result_type->As<core::type::Vector>()) {
        auto ctor_conv = intrinsic::VectorCtorConv(vec->Width());
        auto match = table.Lookup(ctor_conv, Vector{vec->Type()}, std::move(arg_types),
                                  core::EvaluationStage::kConstant);
        if (match != Success || vec->Type() != arg_types[0]->DeepestElement()) {
            AddError(construct) << "no matching overload for " << vec->FriendlyName()
                                << " constructor";
        }
        return;
    }

    if (auto* mat = result_type->As<core::type::Matrix>()) {
        auto ctor_conv = intrinsic::MatrixCtorConv(mat->Columns(), mat->Rows());
        auto match = table.Lookup(ctor_conv, Vector{mat->Type()}, std::move(arg_types),
                                  core::EvaluationStage::kConstant);
        if (match != Success) {
            AddError(construct) << "no matching overload for " << mat->FriendlyName()
                                << " constructor";
        }
        return;
    }
}

void Functional::CheckCall(const Call* call) {
    tint::Switch(
        call,                                            //
        [&](const Construct* c) { CheckConstruct(c); },  //
        [&](Default) {
            // Validation of custom IR instructions
        });
}

void Functional::CheckAccess(const Access* a) {
    auto* obj_view = a->Object()->Type()->As<core::type::MemoryView>();
    auto* ty = obj_view ? obj_view->StoreType() : a->Object()->Type();

    enum Kind : uint8_t {
        kPtr,
        kRef,
        kValue,
    };
    const Kind in_kind = tint::Switch(
        a->Object()->Type(),                                 //
        [&](const core::type::Pointer*) { return kPtr; },    //
        [&](const core::type::Reference*) { return kRef; },  //
        [&](Default) { return kValue; });

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
                return NameOf(type);
        }
    };

    for (size_t i = 0; i < a->Indices().size(); i++) {
        auto err = [&]() -> diag::Diagnostic& {
            return AddError(a, i + Access::kIndicesOperandOffset);
        };

        auto* index = a->Indices()[i];
        if (DAWN_UNLIKELY(
                (!index->Type() || !index->Type()->IsAnyOf<core::type::I32, core::type::U32>()))) {
            err() << "index type " << NameOf(index->Type()) << " must be i32 or u32";
            return;
        }

        if (!ir_.properties.Contains(Property::kAllowVectorElementPointer)) {
            if (in_kind != kValue && ty->Is<core::type::Vector>()) {
                err() << "cannot obtain address of vector element";
                return;
            }
        }

        if (auto* const_index = index->As<ir::Constant>()) {
            auto* value = const_index->Value();
            if (!value->Type() || value->Type()->IsSignedIntegerScalar()) {
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
                    AddNote(a, i + Access::kIndicesOperandOffset)
                        << "acceptable range: [0.." << (el_count - 1) << "]";
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

    auto* want = a->Result()->Type();
    auto* want_view = want->As<core::type::MemoryView>();
    bool ok = true;
    if (obj_view) {
        // Pointer source always means pointer result.
        ok = (want_view != nullptr) && ty == want_view->StoreType();
        if (ok) {
            // Also check that the address space and access modes match.
            ok = obj_view->Is<core::type::Pointer>() == want_view->Is<core::type::Pointer>() &&
                 obj_view->AddressSpace() == want_view->AddressSpace() &&
                 obj_view->Access() == want_view->Access();
        }
    } else {
        // Otherwise, result types should exactly match.
        ok = ty == want;
    }
    if (DAWN_UNLIKELY(!ok)) {
        AddError(a) << "result of access chain is type " << desc_of(in_kind, ty)
                    << " but instruction type is " << NameOf(want);
    }
}

void Functional::CheckBinary(const Binary* b) {
    intrinsic::Context context{b->TableData(), type_mgr_, symbols_};

    auto overload =
        core::intrinsic::LookupBinary(context, b->Op(), b->LHS()->Type(), b->RHS()->Type(),
                                      core::EvaluationStage::kRuntime, /* is_compound */ false);
    if (overload != Success) {
        AddError(b) << overload.Failure();
        return;
    }

    auto* result = b->Result(0);
    TINT_ASSERT(result);

    if (overload->return_type != result->Type()) {
        AddError(b) << "result value type " << NameOf(result->Type()) << " does not match "
                    << style::Instruction(b->Op()) << " result type "
                    << NameOf(overload->return_type);
    }
}

void Functional::CheckIf(const If* if_) {
    if (if_->Condition() && !if_->Condition()->Type()->Is<core::type::Bool>()) {
        AddError(if_, If::kConditionOperandOffset) << "condition type must be 'bool'";
    }

    CheckBlock(if_->True());
    CheckBlock(if_->False());
}

bool Functional::CanLoad(const core::type::Type* ty) {
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

void Functional::CheckLoad(const Load* l) {
    const Value* from = l->From();
    TINT_ASSERT(from);

    auto* mv = from->Type()->As<core::type::MemoryView>();
    if (!mv) {
        AddError(l, Load::kFromOperandOffset)
            << "load source operand " << NameOf(from->Type()) << " is not a memory view";
        return;
    }

    if (mv->Access() != core::Access::kRead && mv->Access() != core::Access::kReadWrite) {
        AddError(l, Load::kFromOperandOffset)
            << "load source operand has a non-readable access type, "
            << style::Literal(ToString(mv->Access()));
        return;
    }

    if (l->Result()->Type() != mv->StoreType()) {
        AddError(l, Load::kFromOperandOffset)
            << "result type " << NameOf(l->Result()->Type()) << " does not match source store type "
            << NameOf(mv->StoreType());
    }

    if (!CanLoad(mv->StoreType())) {
        AddError(l, Load::kFromOperandOffset)
            << "type " << NameOf(mv->StoreType()) << " cannot be loaded";
        return;
    }
}

void Functional::CheckStore(const Store* s) {
    const core::ir::Value* from = s->From();
    const core::ir::Value* to = s->To();
    TINT_ASSERT(from != nullptr);
    TINT_ASSERT(to != nullptr);

    auto* mv = As<core::type::MemoryView>(to->Type());
    if (!mv) {
        AddError(s, Store::kToOperandOffset)
            << "store target operand " << NameOf(to->Type()) << " is not a memory view";
        return;
    }

    if (mv->Access() != core::Access::kWrite && mv->Access() != core::Access::kReadWrite) {
        AddError(s, Store::kToOperandOffset)
            << "store target operand has a non-writeable access type, "
            << style::Literal(ToString(mv->Access()));
        return;
    }

    const core::type::Type* value_type = from->Type();
    const core::type::Type* store_type = mv->StoreType();
    if (value_type != store_type) {
        AddError(s, Store::kFromOperandOffset)
            << "value type " << NameOf(value_type) << " does not match store type "
            << NameOf(store_type);
        return;
    }

    if (!store_type->IsConstructible()) {
        AddError(s) << "store type " << NameOf(store_type) << " is not constructible";
        return;
    }
}

const core::type::Type* Functional::GetVectorPtrElementType(const Instruction* inst, size_t idx) {
    auto* operand = inst->Operands()[idx];
    TINT_ASSERT(operand) << "missing element operand";

    auto* type = operand->Type();
    TINT_ASSERT(type) << "missing operand type";

    auto* memory_view_ty = type->As<core::type::MemoryView>();
    if (DAWN_LIKELY(memory_view_ty)) {
        auto* vec_ty = memory_view_ty->StoreType()->As<core::type::Vector>();
        if (DAWN_LIKELY(vec_ty)) {
            return vec_ty->Type();
        }
    }

    AddError(inst, idx) << "operand " << NameOf(type) << " must be a pointer to a vector";
    return nullptr;
}

void Functional::CheckLoadVectorElement(const LoadVectorElement* l) {
    if (auto* res = l->Result(0)) {
        const core::type::Type* el_ty =
            GetVectorPtrElementType(l, LoadVectorElement::kFromOperandOffset);
        if (!el_ty) {
            return;
        }
        if (res->Type() != el_ty) {
            AddError(l) << "result type " << NameOf(res->Type())
                        << " does not match vector pointer element type " << NameOf(el_ty);
            return;
        }
    }

    if (!l->Index()->Type()->IsIntegerScalar()) {
        AddError(l, LoadVectorElement::kIndexOperandOffset)
            << "load vector element index must be an integer scalar";
    }
    if (auto* c = l->Index()->As<core::ir::Constant>()) {
        uint32_t val = c->Value()->ValueAs<uint32_t>();

        const core::type::Vector* vec_ty =
            l->From()->Type()->UnwrapPtrOrRef()->As<core::type::Vector>();
        TINT_ASSERT(vec_ty);

        if (val >= vec_ty->Width()) {
            AddError(l, LoadVectorElement::kIndexOperandOffset)
                << "load vector element index must be in range [0, " << (vec_ty->Width() - 1)
                << "]";
        }
    }
}

void Functional::CheckStoreVectorElement(const StoreVectorElement* s) {
    if (auto* value = s->Value()) {
        const core::type::Type* el_ty =
            GetVectorPtrElementType(s, StoreVectorElement::kToOperandOffset);
        if (!el_ty) {
            return;
        }
        if (value->Type() != el_ty) {
            AddError(s, StoreVectorElement::kValueOperandOffset)
                << "value type " << NameOf(value->Type())
                << " does not match vector pointer element type " << NameOf(el_ty);
            return;
        }

        // The `GetVectorPtrElementType` has already validated that the pointer exists.
        const core::type::MemoryView* mv = s->To()->Type()->As<core::type::MemoryView>();
        if (mv->Access() != core::Access::kWrite && mv->Access() != core::Access::kReadWrite) {
            AddError(s, StoreVectorElement::kToOperandOffset)
                << "store_vector_element target operand has a non-writeable access type, "
                << style::Literal(ToString(mv->Access()));
            return;
        }
    }

    if (!s->Index()->Type()->IsIntegerScalar()) {
        AddError(s, StoreVectorElement::kIndexOperandOffset)
            << "store vector element index must be an integer scalar";
    }

    const core::ir::Constant* c = s->Index()->As<core::ir::Constant>();
    if (c == nullptr) {
        return;
    }

    uint32_t val = c->Value()->ValueAs<uint32_t>();
    const core::type::Vector* vec_ty = s->To()->Type()->UnwrapPtrOrRef()->As<core::type::Vector>();
    TINT_ASSERT(vec_ty);

    if (val >= vec_ty->Width()) {
        AddError(s, StoreVectorElement::kIndexOperandOffset)
            << "store vector element index must be in range [0, " << (vec_ty->Width() - 1) << "]";
    }
}

void Functional::CheckLoop(const Loop* l) {
    CheckBlock(l->Initializer());
    CheckLoopBody(l);
    CheckLoopContinuing(l);

    first_continues_.Remove(l);
}

void Functional::CheckLoopBody(const Loop* loop) {
    // If the body block has parameters, there must be an initializer block.
    if (!loop->Body()->Params().IsEmpty()) {
        if (!loop->HasInitializer()) {
            AddError(loop) << "loop with body block parameters must have an initializer";
        }
    }
    CheckBlock(loop->Body());
}

void Functional::CheckLoopContinuing(const Loop* loop) {
    // Ensure that values used in the loop continuing are not from the loop body, after a continue
    // instruction.
    auto* first_continue = first_continues_.GetOr(loop, nullptr);
    if (first_continue == nullptr) {
        return;
    }

    // Find the instruction in the body block that is or holds the first continue instruction.
    const Instruction* holds_continue = first_continue;
    while (holds_continue && holds_continue->Block() && holds_continue->Block() != loop->Body()) {
        holds_continue = holds_continue->Block()->Parent();
    }

    auto check_usage = [&](Usage use) {
        if (TransitivelyHolds(loop->Continuing(), use.instruction)) {
            AddError(use.instruction, use.operand_index)
                << NameOf(use.instruction->Operands()[use.operand_index])
                << " cannot be used in continuing block as it is declared after the first "
                << style::Instruction("continue") << " in the loop's body";
            AddNote(first_continue) << "loop body's first " << style::Instruction("continue");
        }
    };

    // Check that all subsequent instruction values are not used in the continuing block.
    for (auto* inst = holds_continue; inst; inst = inst->next) {
        for (auto* result : inst->Results()) {
            result->ForEachUseUnsorted(check_usage);
        }
    }

    CheckBlock(loop->Continuing());
}

void Functional::CheckTerminator(const Terminator* b) {
    tint::Switch(b,                                                //
                 [&](const ir::Continue* c) { CheckContinue(c); }  //
                 // TODO(516717234): Add TINT_ICE_ON_NO_MATCH
    );
}

void Functional::CheckContinue(const Continue* c) {
    auto* loop = c->Loop();
    TINT_ASSERT(loop);

    first_continues_.Add(loop, c);
}

void Functional::CheckSwitch(const Switch* s) {
    if (s->Condition() && !s->Condition()->Type()->IsIntegerScalar()) {
        auto* cond_ty = s->Condition() ? s->Condition()->Type() : nullptr;
        AddError(s, Switch::kConditionOperandOffset)
            << "condition type " << NameOf(cond_ty) << " must be an integer scalar";
    }

    bool found_default = false;
    for (auto& case_ : s->Cases()) {
        if (case_.selectors.IsEmpty()) {
            AddError(s) << "case does not have any selectors";
        }
        CheckBlock(case_.block);

        for (const auto& sel : case_.selectors) {
            if (sel.IsDefault()) {
                if (found_default) {
                    AddError(s) << "multiple default selectors in switch";
                }
                found_default = true;
            } else if (!sel.val->Type()->IsIntegerScalar()) {
                AddError(s) << "case selector type " << NameOf(sel.val->Type())
                            << " must be an integer scalar";
            } else if (s->Condition() && sel.val->Type() != s->Condition()->Type()) {
                AddError(s) << "case selector type " << NameOf(sel.val->Type())
                            << " must match the switch condition type "
                            << NameOf(s->Condition()->Type());
            }
        }
    }

    if (!found_default) {
        AddError(s) << "missing default case for switch";
    }
}

void Functional::CheckSwizzle(const Swizzle* s) {
    auto* src_vec = s->Object()->Type()->As<core::type::Vector>();
    if (!src_vec) {
        AddError(s) << "object of swizzle, " << NameOf(s->Object()) << ", is not a vector, "
                    << NameOf(s->Object()->Type());
        return;
    }

    auto indices = s->Indices();
    if (indices.Length() < Swizzle::kMinNumIndices) {
        AddError(s) << "expected at least " << Swizzle::kMinNumIndices << " indices";
        return;
    }

    if (indices.Length() > Swizzle::kMaxNumIndices) {
        AddError(s) << "expected at most " << Swizzle::kMaxNumIndices << " indices";
        return;
    }

    auto elem_count = src_vec->Elements().count;
    for (auto& idx : indices) {
        if (idx > Swizzle::kMaxIndexValue || idx >= elem_count) {
            AddError(s) << "invalid index value";
            return;
        }
    }

    auto* elem_ty = src_vec->Elements().type;
    auto* expected_ty = type_mgr_.MatchWidth(elem_ty, indices.Length());
    auto* result_ty = s->Result()->Type();
    if (result_ty != expected_ty) {
        AddError(s) << "result type " << NameOf(result_ty) << " does not match expected type, "
                    << NameOf(expected_ty);
        return;
    }
}

void Functional::CheckUnary(const Unary* u) {
    TINT_ASSERT(u->Val());

    intrinsic::Context context{u->TableData(), type_mgr_, symbols_};
    auto overload = core::intrinsic::LookupUnary(context, u->Op(), u->Val()->Type(),
                                                 core::EvaluationStage::kRuntime);
    if (overload != Success) {
        AddError(u) << overload.Failure();
        return;
    }

    const core::ir::Value* result = u->Result(0);
    if (result == nullptr) {
        return;
    }

    if (overload->return_type != result->Type()) {
        AddError(u) << "result value type " << NameOf(result->Type()) << " does not match "
                    << style::Instruction(Disassemble().NameOf(u->Op())) << " result type "
                    << NameOf(overload->return_type);
    }
}

}  // namespace tint::core::ir::validator
