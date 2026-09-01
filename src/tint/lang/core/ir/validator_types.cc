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

#include "src/tint/lang/core/ir/validator.h"
#include "src/tint/lang/core/type/array.h"
#include "src/tint/lang/core/type/binding_array.h"
#include "src/tint/lang/core/type/buffer.h"
#include "src/tint/lang/core/type/f16.h"
#include "src/tint/lang/core/type/f32.h"
#include "src/tint/lang/core/type/i32.h"
#include "src/tint/lang/core/type/i8.h"
#include "src/tint/lang/core/type/matrix.h"
#include "src/tint/lang/core/type/memory_view.h"
#include "src/tint/lang/core/type/pointer.h"
#include "src/tint/lang/core/type/reference.h"
#include "src/tint/lang/core/type/sampled_texture.h"
#include "src/tint/lang/core/type/storage_texture.h"
#include "src/tint/lang/core/type/subgroup_matrix.h"
#include "src/tint/lang/core/type/type.h"
#include "src/tint/lang/core/type/u16.h"
#include "src/tint/lang/core/type/u32.h"
#include "src/tint/lang/core/type/u64.h"
#include "src/tint/lang/core/type/u8.h"
#include "src/tint/lang/core/type/vector.h"
#include "src/tint/lang/core/type/void.h"
#include "src/tint/utils/internal_limits.h"
#include "src/tint/utils/rtti/switch.h"

namespace tint::core::ir::validator {
namespace {

struct Pending {
    const core::type::Type* type = nullptr;
    const core::type::Type* parent = nullptr;

    bool operator==(const Pending& other) const {
        return type == other.type && parent == other.parent;
    }

    struct Hasher {
        HashCode operator()(const Pending& p) const {
            return HashCombine(Hash(p.type), Hash(p.parent));
        }
    };
};

}  // namespace

void Validator::CheckType(const core::type::Type* root, std::function<diag::Diagnostic&()> diag) {
    if (root == nullptr) {
        return;
    }

    if (!ir_.properties.Contains(Property::kAllowNonCoreTypes) && !root->IsCore()) {
        diag() << "non-core types not allowed in core IR";
        return;
    }

    if (!validated_types_.Add(root)) {
        return;
    }

    if (!CheckNestDepth(root, diag)) {
        return;
    }

    AddressSpace addrspace = AddressSpace::kUndefined;
    if (auto* mv = root->As<core::type::MemoryView>()) {
        addrspace = mv->AddressSpace();
    }

    Vector<Pending, 8> stack{Pending{root, nullptr}};
    Hashset<Pending, 8, Pending::Hasher> seen{};
    while (!stack.IsEmpty()) {
        auto [ty, parent] = stack.Pop();
        if (!ty) {
            continue;
        }
        if (ty->IsAbstract()) {
            diag() << "abstracts are not permitted";
            return;
        }

        bool chk = tint::Switch(
            ty,  //
            [&](const core::type::Struct* str) { return CheckStruct(str, diag); },
            [&](const core::type::Reference* ref) { return CheckRef(ref, diag, root); },
            [&](const core::type::Pointer* ptr) { return CheckPtr(ptr, diag); },
            [&](const core::type::I8*) { return Check8BitInteger(diag, parent); },
            [&](const core::type::U8*) { return Check8BitInteger(diag, parent); },
            [&](const core::type::U16*) { return Check16BitInteger(diag); },
            [&](const core::type::U64*) { return Check64BitInteger(diag); },
            [&](const core::type::F16*) { return Check16BitFloat(diag); },
            [&](const core::type::Array* arr) { return CheckArray(arr, diag); },
            [&](const core::type::Vector* v) { return CheckVector(v, diag); },
            [&](const core::type::Matrix* m) { return CheckMatrix(m, diag); },
            [&](const core::type::Atomic* a) { return CheckAtomic(a, diag); },
            [&](const core::type::SampledTexture* s) { return CheckSampledTexture(s, diag); },
            [&](const core::type::MultisampledTexture* ms) {
                return CheckMultisampledTexture(ms, diag);
            },
            [&](const core::type::StorageTexture* s) { return CheckStorageTexture(s, diag); },
            [&](const core::type::InputAttachment* i) { return CheckInputAttachment(i, diag); },
            [&](const core::type::SubgroupMatrix* m) {
                return CheckSubgroupMatrix(m, diag, addrspace);
            },
            [&](const core::type::BindingArray* t) {
                return CheckBindingArray(t, diag, addrspace);
            },
            [&](const core::type::Buffer* buf) { return CheckBuffer(buf, diag); },
            [&](const core::type::SwizzleView* sv) { return CheckSwizzleView(sv, diag); },
            [](Default) { return true; });
        if (!chk) {
            return;
        }

        if (auto* view = ty->As<core::type::MemoryView>()) {
            Pending next{view->StoreType(), ty};
            if (seen.Add(next)) {
                stack.Push(next);
            }
            continue;
        }

        // Visit the elements of a composite type.
        auto type_count = ty->Elements();
        if (type_count.type) {
            // Every element has the same type (e.g. array, vector, matrix, ...), so validate that
            // type once if it has not been seen before.
            Pending next{type_count.type, ty};
            if (seen.Add(next)) {
                stack.Push(next);
            }
            continue;
        }

        // Different elements have different types (e.g. a struct), so we need to validate each
        // of them if they have not been seen before.
        for (uint32_t i = 0; i < type_count.count; i++) {
            if (auto* subtype = ty->Element(i)) {
                Pending next{subtype, ty};
                if (seen.Add(next)) {
                    stack.Push(next);
                }
            }
        }
    }
}

bool Validator::CheckNestDepth(const core::type::Type* type,
                               std::function<diag::Diagnostic&()> diag) {
    if (type == nullptr) {
        return true;
    }

    struct Task {
        const core::type::Type* type;
        uint64_t depth;
    };

    Vector<Task, 16> tasks;
    tasks.Push({type, 0});

    while (!tasks.IsEmpty()) {
        auto [cur, depth] = tasks.Pop();

        // Unwrap memory views
        if (auto* view = cur->As<core::type::MemoryView>()) {
            cur = view->StoreType();
        }

        if (cur == nullptr) {
            continue;
        }

        auto max_depth = max_nest_depth_.Get(cur);
        if (max_depth && *max_depth.value >= depth) {
            // A deeper depth has already been tested for this type
            continue;
        }
        max_nest_depth_.Replace(cur, depth);

        if (depth > internal_limits::kMaxNestDepthOfCompositeType) {
            diag() << "type has a nesting depth that exceeds the maximum of "
                   << internal_limits::kMaxNestDepthOfCompositeType;
            return false;
        }

        auto elems = cur->Elements();
        if (elems.count == 0) {
            // Not a composite type, so no further work needed
            continue;
        }

        // cur is a composite type so need to check all of the contained elements
        uint64_t next_depth = depth + 1;
        if (elems.type) {
            // Homogeneous elements, i.e. is an array, vec, etc, so only need to enqueue one type
            tasks.Push({elems.type, next_depth});
        } else {
            // Heterogeneous elements, so need to enqueue each type
            for (uint32_t i = 0; i < elems.count; i++) {
                if (auto* elem_type = cur->Element(i)) {
                    tasks.Push({elem_type, next_depth});
                }
            }
        }
    }

    return true;
}

bool Validator::CheckStruct(const core::type::Struct* str,
                            std::function<diag::Diagnostic&()>& diag) {
    uint32_t cur_offset = 0;
    for (auto* member : str->Members()) {
        if (member->Type()->Is<core::type::Void>()) {
            diag() << "struct member " << member->Index() << " cannot have void type";
            return false;
        }
        if (member->Type()->Is<core::type::Buffer>()) {
            diag() << "struct member " << member->Index() << " cannot have buffer type";
            return false;
        }

        if (!CheckStructMemberAttributes(member, diag)) {
            return false;
        }

        if (!ir_.properties.Contains(Property::kAllowMslEntryPointInterface)) {
            if (member->Type()->Is<core::type::Pointer>()) {
                diag() << "struct member " << member->Index() << " cannot be a pointer type";
                return false;
            }

            if (member->Type()->Is<core::type::Texture>()) {
                diag() << "struct member " << member->Index() << " cannot be a texture type";
                return false;
            }

            if (member->Type()->Is<core::type::Sampler>()) {
                diag() << "struct member " << member->Index() << " cannot be a sampler type";
                return false;
            }
        }

        if (auto* arr = member->Type()->As<core::type::Array>();
            arr && arr->Count()->Is<core::type::RuntimeArrayCount>()) {
            if (member != str->Members().Back()) {
                diag() << "runtime-sized arrays can only be the last member of a "
                          "struct";
                return false;
            }
        }

        if (member->Align() == 0) {
            diag() << "struct member must not have an alignment of 0";
            return false;
        }
        if (!tint::IsPowerOfTwo(member->Align())) {
            diag() << "struct member alignment must be a power of 2";
            return false;
        }

        if (member->Type()->Align() == 0) {
            diag() << "struct member type must not have an alignment of 0";
            return false;
        }
        if (!tint::IsPowerOfTwo(member->Type()->Align())) {
            diag() << "struct member type alignment must be a power of 2";
            return false;
        }
        if (ir_.properties.Contains(Property::kAllowStructMatrixDecorations)) {
            if (member->RowMajor() || member->HasMatrixStride()) {
                const core::type::Type* base_ty = member->Type();
                while (auto* arr = base_ty->As<core::type::Array>()) {
                    base_ty = arr->ElemType();
                }
                if (!base_ty->Is<core::type::Matrix>()) {
                    if (member->RowMajor()) {
                        diag() << "RowMajor attribute can only be applied to a matrix or an array "
                                  "of matrices";
                    } else {
                        diag() << "MatrixStride attribute can only be applied to a matrix or an "
                                  "array of matrices";
                    }
                    return false;
                }
            }
        } else {
            if (member->RowMajor()) {
                diag() << "Row major annotation not allowed on structures";
                return false;
            }
            if (member->HasMatrixStride()) {
                diag() << "Matrix stride annotation not allowed on structures";
                return false;
            }
        }

        // TODO(448608979): Remove guard once updated to handle RowMajor correctly
        if (!member->RowMajor()) {
            if (member->Size() < member->Type()->Size()) {
                diag() << "struct member " << member->Index() << " with size=" << member->Size()
                       << " must be at least as large as the type with size "
                       << member->Type()->Size();
                return false;
            }

            if (member->Align() % member->Type()->Align() != 0) {
                diag() << "struct member alignment (" << member->Align()
                       << ") must be divisible by type alignment (" << member->Type()->Align()
                       << ")";
                return false;
            }
        }

        cur_offset += (member->Offset() - cur_offset) + member->MinimumRequiredSize();
    }
    if (str->Size() < cur_offset) {
        diag() << "struct size (" << str->Size() << ") is smaller than the end of the last member ("
               << cur_offset << ")";
        return false;
    }

    return true;
}

bool Validator::CheckStructMemberAttributes(const core::type::StructMember* member,
                                            std::function<diag::Diagnostic&()> make_diag) {
    const auto checkers = IOAttributeCheckersFor(member->Attributes(), /*skip_builtins*/ false);
    for (const auto* checker : checkers) {
        auto res = checker->check(member->Type(), member->Attributes(), ir_.properties,
                                  IOAttributeUsage::kUndefinedUsage);
        if (res != Success) {
            make_diag() << res.Failure();
            return false;
        }
        if (!checker->type_check(member->Type(), ir_.properties)) {
            make_diag() << ToString(checker->kind) << " " << checker->type_error;
            return false;
        }
    }

    if (member->Attributes().location.has_value()) {
        if (ir_.properties.Contains(Property::kAllowLocationForNumericComposites)) {
            if (!member->Type()->UnwrapPtrOrRef()->IsNumericScalarOrVector() &&
                !member->Type()->UnwrapPtrOrRef()->Is<core::type::Struct>()) {
                make_diag() << "struct member with a location attribute must be a numeric scalar, "
                               "a numeric vector or a struct, but has type "
                            << member->Type()->FriendlyName();
                return false;
            }
        } else {
            if (!member->Type()->UnwrapPtrOrRef()->IsNumericScalarOrVector()) {
                make_diag() << "struct member with a location attribute must be "
                               "a numeric scalar or vector, but has type "
                            << member->Type()->FriendlyName();
                return false;
            }
        }
    }
    return true;
}

bool Validator::CheckRef(const core::type::Reference* ref,
                         std::function<diag::Diagnostic&()>& diag,
                         const core::type::Type* root) {
    if (ref->StoreType()->Is<core::type::Void>()) {
        diag() << "references to void are not permitted";
        return false;
    }

    // Reference types are guarded by the AllowRefTypes property.
    if (!ir_.properties.Contains(Property::kAllowRefTypes)) {
        diag() << "reference types are not permitted here";
        return false;
    }
    // If they are allowed, reference types still cannot be nested.
    if (ref != root) {
        diag() << "nested reference types are not permitted";
        return false;
    }
    return true;
}

bool Validator::CheckPtr(const core::type::Pointer* ptr, std::function<diag::Diagnostic&()>& diag) {
    if (ptr->StoreType()->Is<core::type::Void>()) {
        diag() << "pointers to void are not permitted";
        return false;
    }

    if (ptr->AddressSpace() == AddressSpace::kUniform ||
        ptr->AddressSpace() == AddressSpace::kHandle ||
        ptr->AddressSpace() == core::AddressSpace::kImmediate) {
        if (ptr->Access() != core::Access::kRead) {
            diag() << ToString(ptr->AddressSpace()) << " pointers must be read access";
            return false;
        }
    }

    if (ptr->AddressSpace() == AddressSpace::kWorkgroup ||
        ptr->AddressSpace() == AddressSpace::kFunction ||
        ptr->AddressSpace() == AddressSpace::kPrivate) {
        if (ptr->Access() != core::Access::kReadWrite) {
            diag() << ToString(ptr->AddressSpace()) << " pointers must be read_write access";
            return false;
        }
    }

    if (ptr->AddressSpace() == AddressSpace::kHandle) {
        if (!ptr->StoreType()->IsHandle()) {
            diag() << "the 'handle' address space can only be used for handle types";
            return false;
        }
    } else if (ptr->StoreType()->IsHandle()) {
        diag() << "handle types can only be declared in the 'handle' address space";
        return false;
    }

    if (ptr->StoreType()->Is<core::type::Pointer>()) {
        diag() << "pointers to pointers are not allowed";
        return false;
    }

    if (ptr->StoreType()->Is<core::type::Buffer>()) {
        if (ptr->AddressSpace() != AddressSpace::kWorkgroup &&
            ptr->AddressSpace() != AddressSpace::kStorage &&
            ptr->AddressSpace() != AddressSpace::kUniform) {
            diag() << "buffer types are not allowed in the '" << ToString(ptr->AddressSpace())
                   << "' address space";
            return false;
        }
    }

    return true;
}

// 8-bit integer types are guarded by the Allow8BitIntegers property.
// They can be used as the component type of a subgroup matrix without the property.
bool Validator::Check8BitInteger(std::function<diag::Diagnostic&()>& diag,
                                 const core::type::Type* parent) {
    if (!Is<core::type::SubgroupMatrix>(parent) &&
        !ir_.properties.Contains(Property::kAllow8BitIntegers)) {
        diag() << "8-bit integer types are not permitted";
        return false;
    }
    return true;
}

// 16-bit integer types are guarded by the Allow16BitIntegers property.
bool Validator::Check16BitInteger(std::function<diag::Diagnostic&()>& diag) {
    if (!ir_.properties.Contains(Property::kAllow16BitIntegers)) {
        diag() << "16-bit integer types are not permitted";
        return false;
    }
    return true;
}

// 64-bit integer types are guarded by the Allow64BitIntegers property.
bool Validator::Check64BitInteger(std::function<diag::Diagnostic&()>& diag) {
    if (!ir_.properties.Contains(Property::kAllow64BitIntegers)) {
        diag() << "64-bit integer types are not permitted";
        return false;
    }
    return true;
}

// 16-bit float types are guarded by the Allow16BitFloats property.
bool Validator::Check16BitFloat(std::function<diag::Diagnostic&()>& diag) {
    if (!ir_.properties.Contains(Property::kAllow16BitFloats)) {
        diag() << "16-bit float types are not permitted";
        return false;
    }
    return true;
}

bool Validator::CheckArray(const core::type::Array* arr, std::function<diag::Diagnostic&()>& diag) {
    if (!arr->ElemType()->HasCreationFixedFootprint()) {
        diag() << "array elements, " << NameOf(arr) << ", must have creation-fixed footprint";
        return false;
    }
    if (auto* count = arr->Count()->As<core::type::ConstantArrayCount>()) {
        if (count->value == 0) {
            diag() << "array requires a constant array size > 0";
            return false;
        }
        return true;
    }

    if (auto* val_count = arr->Count()->As<core::ir::type::ValueArrayCount>()) {
        if (!val_count->value) {
            diag() << "ValueArrayCount value is undefined";
            return false;
        }
        if (!val_count->value->Alive()) {
            diag() << "ValueArrayCount value is not alive";
            return false;
        }
        if (!val_count->value->Type()->IsIntegerScalar()) {
            diag() << "ValueArrayCount must be an integer scalar type";
            return false;
        }
        auto* inst_res = val_count->value->As<core::ir::InstructionResult>();
        if (!inst_res) {
            diag() << "ValueArrayCount must be an instruction result";
            return false;
        }
        auto* inst = inst_res->Instruction();
        if (!inst || inst->Block() != ir_.root_block) {
            diag() << "ValueArrayCount must be a module-scoped override expression";
            return false;
        }
    }
    return true;
}

bool Validator::CheckVector(const core::type::Vector* vec,
                            std::function<diag::Diagnostic&()>& diag) {
    if (!vec->Type()->IsScalar()) {
        diag() << "vector elements, " << NameOf(vec) << ", must be scalars";
        return false;
    }
    return true;
}

bool Validator::CheckMatrix(const core::type::Matrix* mat,
                            std::function<diag::Diagnostic&()>& diag) {
    if (!mat->Type()->IsFloatScalar()) {
        diag() << "matrix elements, " << NameOf(mat) << ", must be float scalars";
        return false;
    }
    return true;
}

bool Validator::CheckAtomic(const core::type::Atomic* atom,
                            std::function<diag::Diagnostic&()>& diag) {
    // Prior to lowering we allow for atomic operations on vec2u to support the
    // AtomicVec2UMinMax feature.
    if (auto* vec = atom->Type()->As<core::type::Vector>()) {
        if (vec->Width() == 2 && vec->Type()->Is<core::type::U32>()) {
            return true;
        }
    }

    if (!atom->Type()->IsAnyOf<core::type::I32, core::type::U32, core::type::U64>()) {
        diag() << "atomic subtype must be i32, u32 or u64 type is " << NameOf(atom->Type());
        return false;
    }
    return true;
}

bool Validator::CheckSampledTexture(const core::type::SampledTexture* s,
                                    std::function<diag::Diagnostic&()>& diag) {
    if (!s->Type()->IsAnyOf<core::type::F32, core::type::I32, core::type::U32>()) {
        diag() << "invalid sampled texture sample type: " << NameOf(s->Type());
        return false;
    }
    return true;
}

bool Validator::CheckMultisampledTexture(const core::type::MultisampledTexture* ms,
                                         std::function<diag::Diagnostic&()>& diag) {
    if (!ms->Type()->IsAnyOf<core::type::F32, core::type::I32, core::type::U32>()) {
        diag() << "invalid multisampled texture sample type: " << NameOf(ms->Type());
        return false;
    }

    switch (ms->Dim()) {
        case core::type::TextureDimension::k2d:
            break;
        default:
            diag() << "invalid multisampled texture dimension: "
                   << style::Literal(ToString(ms->Dim()));
            return false;
    }
    return true;
}

bool Validator::CheckStorageTexture(const core::type::StorageTexture* storage,
                                    std::function<diag::Diagnostic&()>& diag) {
    switch (storage->Dim()) {
        case core::type::TextureDimension::kCube:
        case core::type::TextureDimension::kCubeArray:
            diag() << "dimension " << style::Literal(ToString(storage->Dim()))
                   << " for storage textures does not in WGSL yet";
            return false;
        case core::type::TextureDimension::kNone:
            diag() << "invalid texture dimension " << style::Literal(ToString(storage->Dim()));
            return false;
        default:
            break;
    }
    return true;
}

bool Validator::CheckInputAttachment(const core::type::InputAttachment* ia,
                                     std::function<diag::Diagnostic&()>& diag) {
    if (!ia->Type()->IsAnyOf<core::type::F32, core::type::I32, core::type::U32>()) {
        diag() << "invalid input attachment component type: " << NameOf(ia->Type());
        return false;
    }
    return true;
}

bool Validator::CheckSubgroupMatrix(const core::type::SubgroupMatrix* m,
                                    std::function<diag::Diagnostic&()>& diag,
                                    core::AddressSpace addrspace) {
    if (!m->Type()
             ->IsAnyOf<core::type::F16, core::type::F32, core::type::I8, core::type::I32,
                       core::type::U8, core::type::U32>()) {
        diag() << "invalid subgroup matrix component type: " << NameOf(m->Type());
        return false;
    }
    if (!(addrspace == AddressSpace::kUndefined || addrspace == AddressSpace::kFunction)) {
        diag() << "invalid address space for subgroup matrix : " << addrspace;
        return false;
    }
    return true;
}

bool Validator::CheckBindingArray(const core::type::BindingArray* ba,
                                  std::function<diag::Diagnostic&()>& diag,
                                  core::AddressSpace addrspace) {
    if (!ba->Count()->Is<core::type::ConstantArrayCount>()) {
        diag() << "binding_array count must be a constant expression";
        return false;
    }

    auto count = ba->Count()->As<core::type::ConstantArrayCount>()->value;
    if (count == 0) {
        diag() << "binding array requires a constant array size > 0";
        return false;
    }

    if (!(addrspace == AddressSpace::kUndefined || addrspace == AddressSpace::kHandle) &&
        !ir_.properties.Contains(Property::kAllowMslEntryPointInterface)) {
        diag() << "invalid address space for binding_array : " << addrspace;
        return false;
    }

    if (!ir_.properties.Contains(Property::kAllowNonCoreTypes)) {
        if (!ba->ElemType()->Is<core::type::SampledTexture>()) {
            diag() << "binding_array element type must be a sampled texture type";
            return false;
        }
    }
    return true;
}

bool Validator::CheckBuffer(const core::type::Buffer* buf,
                            std::function<diag::Diagnostic&()>& diag) {
    if (!ir_.properties.Contains(Property::kAllowBufferTypes)) {
        diag() << "buffer types are not allowed in this context";
        return false;
    }
    if (auto count = buf->ConstantCount()) {
        const bool allow_16_bits = ir_.properties.Contains(Property::kAllow16BitFloats) ||
                                   ir_.properties.Contains(Property::kAllow16BitIntegers);
        const uint32_t divisor = allow_16_bits ? 2 : 4;
        if (count.value() % divisor != 0) {
            diag() << "buffer size must be evenly divisible by " << divisor;
            return false;
        }
    }
    return true;
}

bool Validator::CheckSwizzleView(const core::type::SwizzleView* sv,
                                 std::function<diag::Diagnostic&()>& diag) {
    if (!ir_.properties.Contains(Property::kAllowSwizzleView)) {
        diag() << "swizzle view is not allowed in this module";
        return false;
    }
    if (sv->FromSize() < 2 || sv->FromSize() > 4) {
        diag() << "swizzle view object must be a vector of 2, 3 or 4 elements, got "
               << sv->FromSize();
        return false;
    }
    if (sv->ToSize() < 1 || sv->ToSize() > 4) {
        diag() << "swizzle view result must be 1, 2, 3 or 4 elements, got " << sv->ToSize();
        return false;
    }
    return true;
}

}  // namespace tint::core::ir::validator
