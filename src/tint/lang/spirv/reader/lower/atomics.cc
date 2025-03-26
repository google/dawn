// Copyright 2025 The Dawn & Tint Authors
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

#include "src/tint/lang/spirv/reader/lower/atomics.h"

#include "src/tint/lang/core/ir/builder.h"
#include "src/tint/lang/core/ir/module.h"
#include "src/tint/lang/core/ir/validator.h"
#include "src/tint/lang/spirv/ir/builtin_call.h"
#include "src/tint/utils/containers/hashset.h"
#include "src/tint/utils/containers/vector.h"

namespace tint::spirv::reader::lower {
namespace {

using namespace tint::core::fluent_types;     // NOLINT
using namespace tint::core::number_suffixes;  // NOLINT

/// PIMPL state for the transform.
struct State {
    /// The IR module.
    core::ir::Module& ir;

    /// The IR builder.
    core::ir::Builder b{ir};

    /// The type manager.
    core::type::Manager& ty{ir.Types()};

    /// The `ir::Value`s to be converted to atomics
    Vector<core::ir::Value*, 8> values_to_convert_{};

    /// The `ir::Value`s which have been converted
    Hashset<core::ir::Value*, 8> converted_{};

    /// Process the module.
    void Process() {
        Vector<spirv::ir::BuiltinCall*, 4> builtin_worklist;
        for (auto* inst : ir.Instructions()) {
            if (auto* builtin = inst->As<spirv::ir::BuiltinCall>()) {
                builtin_worklist.Push(builtin);
            }
        }

        for (auto* builtin : builtin_worklist) {
            switch (builtin->Func()) {
                case spirv::BuiltinFn::kAtomicLoad:
                    break;
                case spirv::BuiltinFn::kAtomicStore:
                    AtomicStore(builtin);
                    break;
                case spirv::BuiltinFn::kAtomicExchange:
                case spirv::BuiltinFn::kAtomicCompareExchange:
                case spirv::BuiltinFn::kAtomicIAdd:
                case spirv::BuiltinFn::kAtomicISub:
                case spirv::BuiltinFn::kAtomicSMax:
                case spirv::BuiltinFn::kAtomicSMin:
                case spirv::BuiltinFn::kAtomicUMax:
                case spirv::BuiltinFn::kAtomicUMin:
                case spirv::BuiltinFn::kAtomicAnd:
                case spirv::BuiltinFn::kAtomicOr:
                case spirv::BuiltinFn::kAtomicXor:
                case spirv::BuiltinFn::kAtomicIIncrement:
                case spirv::BuiltinFn::kAtomicIDecrement:
                    break;
                default:
                    TINT_UNREACHABLE() << "unknown spirv builtin: " << builtin->Func();
            }
        }

        while (!values_to_convert_.IsEmpty()) {
            auto* val = values_to_convert_.Pop();

            if (converted_.Add(val)) {
                ConvertAtomicValue(val);
            }
        }
    }

    void AtomicStore(spirv::ir::BuiltinCall* call) {
        auto args = call->Args();

        b.InsertBefore(call, [&] {
            auto* var = args[0];
            values_to_convert_.Push(var);

            auto* val = args[3];
            b.CallWithResult(call->DetachResult(), core::BuiltinFn::kAtomicStore, var, val);
        });
        call->Destroy();
    }

    void ConvertAtomicValue(core::ir::Value* val) {
        auto* res = val->As<core::ir::InstructionResult>();
        TINT_ASSERT(res);

        auto* atomic_ty = AtomicTypeFor(res->Type());
        res->SetType(atomic_ty);

        tint::Switch(                                                            //
            res->Instruction(),                                                  //
            [&](core::ir::Access* a) { values_to_convert_.Push(a->Object()); },  //
            [&](core::ir::Var*) {},                                              //
            TINT_ICE_ON_NO_MATCH);
    }

    const core::type::Type* AtomicTypeFor(const core::type::Type* orig_ty) {
        return tint::Switch(
            orig_ty,  //
            [&](const core::type::I32*) { return ty.atomic(orig_ty); },
            [&](const core::type::U32*) { return ty.atomic(orig_ty); },
            // [&](const core::type::Struct* str) { return ty(Fork(str).name); },
            [&](const core::type::Array* arr) {
                if (arr->Count()->Is<core::type::RuntimeArrayCount>()) {
                    return ty.runtime_array(AtomicTypeFor(arr->ElemType()));
                }
                auto count = arr->ConstantCount();
                TINT_ASSERT(count);

                return ty.array(AtomicTypeFor(arr->ElemType()), u32(count.value()));
            },
            [&](const core::type::Pointer* ptr) {
                return ty.ptr(ptr->AddressSpace(), AtomicTypeFor(ptr->StoreType()), ptr->Access());
            },
            TINT_ICE_ON_NO_MATCH);
    }
};

}  // namespace

Result<SuccessType> Atomics(core::ir::Module& ir) {
    auto result = ValidateAndDumpIfNeeded(ir, "spirv.Atomics",
                                          core::ir::Capabilities{
                                              core::ir::Capability::kAllowOverrides,
                                          });
    if (result != Success) {
        return result.Failure();
    }

    State{ir}.Process();

    return Success;
}

}  // namespace tint::spirv::reader::lower
