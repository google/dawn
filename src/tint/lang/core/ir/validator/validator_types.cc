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

#include "src/tint/lang/core/ir/validator/validator.h"
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

void Validator::CheckBuffersAndMatrices(const Var* var) {
    if (!IsWGSLValidation()) {
        return;
    }

    uint32_t var_size = 0;
    if (var->Result()->Type()->UnwrapPtr()->HasFixedFootprint()) {
        var_size = var->Result()->Type()->UnwrapPtr()->Size();
    }

    Vector<UseInfo, 4> uses;
    for (auto& u : var->Result()->UsagesSorted()) {
        uses.Push({u, var_size, 0, 0});
    }
    while (!uses.IsEmpty()) {
        auto info = uses.Pop();
        diag::Diagnostic error;
        bool errored = tint::Switch(
            info.use.instruction,
            [&](const Let* let) {
                for (auto& u : let->Result()->UsagesSorted()) {
                    uses.Push({u, info.storage_size, info.offset, info.pointer_size});
                }
                return false;
            },
            [&](const UserCall* user) {
                // If the buffer size is decreased at a function boundary, use that size
                // instead.
                auto* target = user->Target();
                auto* param = target->Params()[info.use.operand_index - user->ArgsOperandOffset()];
                auto* param_buffer_ty = param->Type()->UnwrapPtr()->As<core::type::Buffer>();
                uint32_t next_size = param_buffer_ty && param_buffer_ty->Size() > 0
                                         ? param_buffer_ty->Size()
                                         : info.storage_size;
                for (auto& u : param->UsagesSorted()) {
                    uses.Push({u, next_size, info.offset, info.pointer_size});
                }
                return false;
            },
            [&](const CoreBuiltinCall* call) {
                if (call->Func() == BuiltinFn::kBufferView ||
                    call->Func() == BuiltinFn::kBufferArrayView) {
                    if (!CheckBufferView(call, var, info.storage_size)) {
                        return true;
                    }

                    uint32_t offset = 0;
                    if (auto* const_offset = call->Args()[1]->As<Constant>()) {
                        offset = const_offset->Value()->ValueAs<uint32_t>();
                    }
                    uint32_t pointer_size = 0;
                    if (call->Func() == BuiltinFn::kBufferArrayView) {
                        if (auto* const_size = call->Args()[2]->As<Constant>()) {
                            pointer_size = const_size->Value()->ValueAs<uint32_t>();
                        }
                    } else if (call->Result()->Type()->UnwrapPtr()->HasFixedFootprint()) {
                        // Use the bufferView result size if it has a fixed size.
                        pointer_size = call->Result()->Type()->UnwrapPtr()->Size();
                    }

                    // Keep tracing to catch subgroupMatrixLoad/Store transitive uses.
                    for (auto& u : call->Result()->UsagesSorted()) {
                        uses.Push({u, info.storage_size, offset, pointer_size});
                    }
                } else if (call->Func() == BuiltinFn::kSubgroupMatrixLoad ||
                           call->Func() == BuiltinFn::kSubgroupMatrixStore) {
                    if (!CheckSubgroupMatrixMemory(call, var, info)) {
                        return true;
                    }
                }

                return false;
            },
            [&](const Access* access) {
                auto* obj_ty = access->Object()->Type()->UnwrapPtr();

                uint32_t offset = 0;
                for (auto* idx : access->Indices()) {
                    uint32_t idx_value = 0;
                    if (auto* const_idx = idx->As<Constant>()) {
                        idx_value = const_idx->Value()->ValueAs<uint32_t>();
                    }
                    // Matrix and vector can't be hit on the way to a subgroupMatrix and access
                    // won't be hit at all on the way to buffer[Array]View so we only handle
                    // structure and array here.
                    tint::Switch(
                        obj_ty,  //
                        [&](const core::type::Array* ary) {
                            obj_ty = ary->ElemType();
                            offset += idx_value * ary->ImplicitStride();
                        },
                        [&](const core::type::Struct* s) {
                            auto* mem = s->Members()[idx_value];
                            obj_ty = mem->Type();
                            offset += mem->Offset();
                        },
                        [&](Default) {});
                }

                uint32_t pointer_size = info.pointer_size;
                if (access->Result()->Type()->UnwrapPtr()->HasFixedFootprint()) {
                    // If the result has a fixed size, update pointer size.
                    pointer_size = access->Result()->Type()->UnwrapPtr()->Size();
                }

                for (auto& u : access->Result()->UsagesSorted()) {
                    // Accumulate the offset.
                    uses.Push({u, info.storage_size, info.offset + offset, pointer_size});
                }

                return false;
            },
            [&](Default) { return false; });
        if (errored) {
            return;
        }
    }
}

bool Validator::CheckBufferView(const CoreBuiltinCall* call, const Var* var, uint32_t buffer_size) {
    // Calculate the minimum type size.
    auto* store_ty = call->Result()->Type()->UnwrapPtr();
    uint64_t ty_required_size = 0;
    uint64_t ty_offset = 0;
    uint64_t ty_stride = 0;
    if (store_ty->HasFixedFootprint()) {
        ty_required_size = store_ty->Size();
    } else if (auto* str = store_ty->As<core::type::Struct>()) {
        auto* last = str->Members().Back();
        auto* arr_ty = last->Type()->As<core::type::Array>();
        ty_offset = last->Offset();
        ty_stride = arr_ty->ImplicitStride();
        ty_required_size = ty_offset + ty_stride;
    } else {
        ty_stride = store_ty->As<core::type::Array>()->ImplicitStride();
        ty_required_size = ty_stride;
    }

    // Error conditions:
    // For both bufferView and bufferArrayView:
    // * ty_required_size + offset < buffer_size
    // * offset % store_ty->Align() != 0
    // For bufferArrayView
    // * size + offset < buffer_size
    // * size < ty_required_size
    // * (size - offset) % stride != 0
    //
    // Also error if any addition overflows a uint32_t.

    uint64_t offset_val = 0;
    if (auto* const_offset = call->Args()[1]->As<Constant>()) {
        if (const_offset->Type()->IsSignedIntegerScalar()) {
            if (const_offset->Value()->ValueAs<int32_t>() < 0) {
                AddError(call) << call->FriendlyName() << " offset must be greater than 0";
                return false;
            }
        }
        offset_val = const_offset->Value()->ValueAs<uint64_t>();
    }

    if (offset_val + ty_required_size > std::numeric_limits<uint32_t>::max()) {
        AddError(call) << call->FriendlyName() << " requires a size beyond 32 bits";
        return false;
    }

    if (buffer_size > 0 && buffer_size < offset_val + ty_required_size) {
        AddError(var) << "invalid buffer size (" << buffer_size << " bytes) when used with "
                      << call->FriendlyName() << " (" << offset_val + ty_required_size
                      << " bytes required)";
        return false;
    }

    if (offset_val % store_ty->Align() != 0) {
        AddError(call) << call->FriendlyName() << " offset (" << offset_val
                       << " bytes) must be a multiple of result alignment (" << store_ty->Align()
                       << " bytes)";
        return false;
    }

    if (call->Func() == BuiltinFn::kBufferView) {
        return true;
    }

    uint64_t size_val = 0;
    if (auto* const_size = call->Args()[2]->As<Constant>()) {
        if (const_size->Type()->IsSignedIntegerScalar()) {
            if (const_size->Value()->ValueAs<int32_t>() < 0) {
                AddError(call) << call->FriendlyName() << " size must be greater than 0";
                return false;
            }
        }
        size_val = const_size->Value()->ValueAs<uint64_t>();
        if (size_val == 0) {
            AddError(call) << call->FriendlyName() << " cannot be 0 sized";
            return false;
        }
    }

    if (offset_val + size_val > std::numeric_limits<uint32_t>::max()) {
        AddError(call) << call->FriendlyName() << " requires a size beyond 32 bits";
        return false;
    }

    if (buffer_size > 0 && buffer_size < size_val + offset_val) {
        AddError(var) << "invalid buffer size (" << buffer_size << " bytes) when used with "
                      << call->FriendlyName() << " (" << size_val + offset_val
                      << " bytes required)";
        return false;
    }

    if (size_val > 0 && size_val < ty_required_size) {
        AddError(call) << call->FriendlyName() << " has invalid size (" << size_val
                       << " bytes, requires " << ty_required_size << " bytes)";
        return false;
    }

    if (size_val > 0 && ((size_val - ty_offset) % ty_stride != 0)) {
        AddError(call) << call->FriendlyName() << " size (" << size_val
                       << " bytes) minus type offset (" << ty_offset
                       << " bytes) must be a multiple of the type stride (" << ty_stride
                       << " bytes)";
        return false;
    }

    return true;
}

bool Validator::CheckSubgroupMatrixMemory(const CoreBuiltinCall* call,
                                          const Var* var,
                                          const UseInfo& info) {
    const bool is_load = call->Func() == BuiltinFn::kSubgroupMatrixLoad;
    bool col_major = false;
    auto* offset_arg = call->Args()[1];
    const Value* stride_arg = nullptr;
    if (is_load) {
        col_major = std::get<Majorness>(call->ExplicitTemplateParams()[1]) == Majorness::kColMajor;
        stride_arg = call->Args()[2];
    } else {
        col_major = std::get<Majorness>(call->ExplicitTemplateParams()[0]) == Majorness::kColMajor;
        stride_arg = call->Args()[3];
    }
    auto* ty = is_load ? call->Result()->Type() : call->Args()[2]->Type();
    auto* mat_ty = ty->As<core::type::SubgroupMatrix>();
    auto* ele_ty = mat_ty->Type();

    auto* array_ty = call->Args()[0]->Type()->UnwrapPtr()->As<core::type::Array>();
    const uint32_t array_stride = array_ty->ImplicitStride();

    // Error conditions:
    // * stride is less than minimal required stride
    // * pointed to memory is smaller than matrix requires
    // * variable memory is smaller than total required
    //
    // Also if any calculation overflows 32 bits.

    const uint32_t major_size = col_major ? mat_ty->Columns() : mat_ty->Rows();
    const uint32_t minor_size = col_major ? mat_ty->Rows() : mat_ty->Columns();

    uint64_t offset = 0;
    if (auto* const_offset = offset_arg->As<Constant>()) {
        // Offset is array elements of shader scalar type.
        offset = const_offset->Value()->ValueAs<uint64_t>() * array_stride;

        if (offset > std::numeric_limits<uint32_t>::max()) {
            AddError(call) << call->FriendlyName() << " has an offset exceeding 32 bits";
            return false;
        }
    }
    uint32_t min_stride = minor_size * ele_ty->Size();
    uint64_t stride = 0;
    if (auto* const_stride = stride_arg->As<Constant>()) {
        // Stride is in array elements of shader scalar type.
        stride = const_stride->Value()->ValueAs<uint64_t>() * array_stride;

        if (stride > std::numeric_limits<uint32_t>::max()) {
            AddError(call) << call->FriendlyName() << " has a stride exceeding 32 bits";
            return false;
        }
        if (stride < min_stride) {
            AddError(call) << call->FriendlyName() << " stride (" << stride
                           << " bytes) must be greater or equal to " << min_stride << " bytes";
            return false;
        }
    } else {
        stride = min_stride;
    }

    // Note: Offset and stride are in bytes.
    uint64_t mat_required_size = offset + static_cast<uint64_t>(stride) * (major_size - 1) +
                                 static_cast<uint64_t>(minor_size) * ele_ty->Size();
    // Round up to array element size.
    mat_required_size = RoundUp(static_cast<uint64_t>(array_stride), mat_required_size);
    if (mat_required_size > std::numeric_limits<uint32_t>::max()) {
        AddError(call) << call->FriendlyName() << " has a memory requirement exceeding 32 bits";
        return false;
    }

    if (info.pointer_size > 0 && info.pointer_size < mat_required_size) {
        AddError(call) << call->FriendlyName() << " requires more memory (" << mat_required_size
                       << " bytes) than pointed to (" << info.pointer_size << " bytes)";
        return false;
    }

    uint64_t mem_required_size = mat_required_size + info.offset;
    if (mem_required_size > std::numeric_limits<uint32_t>::max()) {
        AddError(call) << " has a total memory requirement exceeding 32 bits";
        return false;
    }

    if (info.storage_size > 0 && info.storage_size < mem_required_size) {
        AddError(var) << "invalid storage size (" << info.storage_size << " bytes) when used with "
                      << call->FriendlyName() << " (" << mem_required_size << " bytes required)";
        AddNote(call) << call->FriendlyName() << " here";
        return false;
    }

    return true;
}

void Validator::CheckAlignment(const Instruction* inst) {
    auto align = inst->Alignment();
    if (align.has_value()) {
        if (inst->GetSideEffects().Size() == 0) {
            AddError(inst) << "alignment can only be set on memory instructions";
        }

        auto align_val = align.value();
        if (!tint::IsPowerOfTwo(align_val)) {
            AddError(inst) << "alignment (" << align_val << ") must be a power of 2";
        }

        if (align_val > 256) {
            AddError(inst) << "alignment (" << align_val << ") must be less than or equal to 256";
        }

        // TODO(b/544359162): Which other instructions should be checked?
        uint32_t natural_align = tint::Switch(
            inst,  //
            [&](const Load* ld) { return ld->Result()->Type()->Align(); },
            [&](const Store* st) { return st->From()->Type()->Align(); },
            [&](const LoadVectorElement* lve) { return lve->Result()->Type()->Align(); },
            [&](const StoreVectorElement* sve) { return sve->Value()->Type()->Align(); },
            [&](const CoreBuiltinCall* call) {
                switch (call->Func()) {
                    case BuiltinFn::kSubgroupMatrixLoad:
                    case BuiltinFn::kSubgroupMatrixStore:
                        return call->Args()[0]->Type()->UnwrapPtr()->Align();
                    default:
                        return 0u;
                }
            },
            [&](Default) { return 0u; });

        if (align_val <= natural_align) {
            AddError(inst) << "alignment (" << align_val
                           << ") must be greater than natural alignment (" << natural_align << ")";
        }
    }
}

}  // namespace tint::core::ir::validator
