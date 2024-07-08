// Copyright 2024 The Dawn & Tint Authors
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

#include "src/tint/lang/hlsl/writer/raise/decompose_memory_access.h"

#include <utility>

#include "src/tint/lang/core/ir/builder.h"
#include "src/tint/lang/core/ir/validator.h"
#include "src/tint/lang/hlsl/builtin_fn.h"
#include "src/tint/lang/hlsl/ir/member_builtin_call.h"
#include "src/tint/lang/hlsl/type/byte_address_buffer.h"
#include "src/tint/utils/result/result.h"

namespace tint::hlsl::writer::raise {
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

    using VarTypePair = std::pair<core::ir::Var*, const core::type::Type*>;
    /// Maps a struct to the load function
    Hashmap<VarTypePair, core::ir::Function*, 2> var_and_type_to_load_fn_{};

    /// Process the module.
    void Process() {
        Vector<core::ir::Var*, 4> var_worklist;
        for (auto* inst : *ir.root_block) {
            // Allow this to run before or after PromoteInitializers by handling non-var root_block
            // entries
            auto* var = inst->As<core::ir::Var>();
            if (!var) {
                continue;
            }

            // Var must be a pointer
            auto* var_ty = var->Result(0)->Type()->As<core::type::Pointer>();
            TINT_ASSERT(var_ty);

            // Only care about storage address space variables.
            if (var_ty->AddressSpace() != core::AddressSpace::kStorage) {
                continue;
            }

            var_worklist.Push(var);
        }

        for (auto* var : var_worklist) {
            auto* result = var->Result(0);

            // Find all the usages of the `var` which is loading or storing.
            Vector<core::ir::Instruction*, 4> usage_worklist;
            for (auto& usage : result->Usages()) {
                Switch(
                    usage->instruction,
                    [&](core::ir::LoadVectorElement* lve) { usage_worklist.Push(lve); },
                    [&](core::ir::StoreVectorElement* sve) { usage_worklist.Push(sve); },
                    [&](core::ir::Store* st) { usage_worklist.Push(st); },
                    [&](core::ir::Load* ld) { usage_worklist.Push(ld); },
                    [&](core::ir::Access* a) { usage_worklist.Push(a); },
                    [&](core::ir::Let* l) { usage_worklist.Push(l); },
                    [&](core::ir::CoreBuiltinCall* call) {
                        TINT_ASSERT(call->Func() == core::BuiltinFn::kArrayLength);
                        usage_worklist.Push(call);
                    },
                    //
                    TINT_ICE_ON_NO_MATCH);
            }

            auto* var_ty = result->Type()->As<core::type::Pointer>();
            while (!usage_worklist.IsEmpty()) {
                auto* inst = usage_worklist.Pop();
                // Load instructions can be destroyed by the replacing access function
                if (!inst->Alive()) {
                    continue;
                }

                Switch(
                    inst,
                    [&](core::ir::LoadVectorElement* l) { LoadVectorElement(l, var, var_ty); },
                    [&](core::ir::StoreVectorElement* s) { StoreVectorElement(s, var, var_ty); },
                    [&](core::ir::Store* s) { Store(s); },     //
                    [&](core::ir::Load* l) { Load(l, var); },  //
                    [&](core::ir::Access* a) {
                        OffsetData offset;
                        Access(a, var, a->Object()->Type(), &offset);
                    },
                    [&](core::ir::Let* let) {
                        // The `let` is, essentially, an alias for the `var` as it's assigned
                        // directly. Gather all the `let` usages into our worklist, and then replace
                        // the `let` with the `var` itself.
                        for (auto& usage : let->Result(0)->Usages()) {
                            usage_worklist.Push(usage->instruction);
                        }
                        let->Result(0)->ReplaceAllUsesWith(result);
                        let->Destroy();
                    },
                    [&](core::ir::CoreBuiltinCall* call) {
                        ArrayLength(var, call, var_ty->StoreType(), 0);
                    },  //
                    TINT_ICE_ON_NO_MATCH);
            }

            // Swap the result type of the `var` to the new HLSL result type
            result->SetType(ty.Get<hlsl::type::ByteAddressBuffer>(var_ty->Access()));
        }
    }

    void ArrayLength(core::ir::Var* var,
                     core::ir::CoreBuiltinCall* call,
                     const core::type::Type* type,
                     uint32_t offset) {
        auto* arr_ty = type->As<core::type::Array>();
        // If the `arrayLength` was called directly on the storage buffer then
        // it _must_ be a runtime array.
        TINT_ASSERT(arr_ty && arr_ty->Count()->As<core::type::RuntimeArrayCount>());

        b.InsertBefore(call, [&] {
            // The `GetDimensions` call uses out parameters for all return values, there is no
            // return value. This ends up being the result value we care about.
            //
            // This creates a var with an access which means that when we emit the HLSL we'll emit
            // the correct `var` name.
            core::ir::Instruction* inst = b.Var(ty.ptr(function, ty.u32()));
            b.MemberCall<hlsl::ir::MemberBuiltinCall>(ty.void_(), BuiltinFn::kGetDimensions, var,
                                                      inst->Result(0));

            inst = b.Load(inst);
            if (offset > 0) {
                inst = b.Subtract(ty.u32(), inst, u32(offset));
            }
            auto* div = b.Divide(ty.u32(), inst, u32(arr_ty->Stride()));
            call->Result(0)->ReplaceAllUsesWith(div->Result(0));
        });
        call->Destroy();
    }

    struct OffsetData {
        uint32_t byte_offset = 0;
        Vector<core::ir::Value*, 4> expr{};
    };

    // Note, must be called inside a builder insert block (Append, InsertBefore, etc)
    void UpdateOffsetData(core::ir::Value* v, uint32_t elm_size, OffsetData* offset) {
        tint::Switch(
            v,  //
            [&](core::ir::Constant* idx_value) {
                offset->byte_offset += idx_value->Value()->ValueAs<uint32_t>() * elm_size;
            },
            [&](core::ir::Value* val) {
                offset->expr.Push(
                    b.Multiply(ty.u32(), b.Convert(ty.u32(), val), u32(elm_size))->Result(0));
            },
            TINT_ICE_ON_NO_MATCH);
    }

    // Note, must be called inside a builder insert block (Append, InsertBefore, etc)
    core::ir::Value* OffsetToValue(OffsetData offset) {
        core::ir::Value* val = b.Value(u32(offset.byte_offset));
        for (core::ir::Value* expr : offset.expr) {
            val = b.Add(ty.u32(), val, expr)->Result(0);
        }
        return val;
    }

    // Creates the appropriate load instructions for the given result type.
    core::ir::Call* MakeLoad(core::ir::Instruction* inst,
                             core::ir::Var* var,
                             const core::type::Type* result_ty,
                             core::ir::Value* offset) {
        if (result_ty->is_numeric_scalar_or_vector()) {
            return MakeScalarOrVectorLoad(var, result_ty, offset);
        }

        return tint::Switch(
            result_ty,  //
            [&](const core::type::Struct* s) {
                auto* fn = GetLoadFunctionFor(inst, var, s);
                return b.Call(fn, offset);
            },
            [&](const core::type::Matrix* m) {
                auto* fn = GetLoadFunctionFor(inst, var, m);
                return b.Call(fn, offset);
            },  //
            [&](const core::type::Array* a) {
                auto* fn = GetLoadFunctionFor(inst, var, a);
                return b.Call(fn, offset);
            },  //
            TINT_ICE_ON_NO_MATCH);
    }

    // Creates a `v.Load{2,3,4} offset` call based on the provided type. The load returns a
    // `u32` or vector of `u32` and then a `bitcast` is done to get back to the desired type.
    //
    // This only works for `u32`, `i32`, `f16`, `f32` and the vector sizes of those types.
    //
    // The `f16` type is special in that `f16` uses a templated load in HLSL `Load<float16_t>`
    // and returns the correct type, so there is no bitcast.
    core::ir::Call* MakeScalarOrVectorLoad(core::ir::Var* var,
                                           const core::type::Type* result_ty,
                                           core::ir::Value* offset) {
        bool is_f16 = result_ty->DeepestElement()->Is<core::type::F16>();

        const core::type::Type* load_ty = ty.u32();
        // An `f16` load returns an `f16` instead of a `u32`
        if (is_f16) {
            load_ty = ty.f16();
        }

        auto fn = is_f16 ? BuiltinFn::kLoadF16 : BuiltinFn::kLoad;
        if (auto* v = result_ty->As<core::type::Vector>()) {
            load_ty = ty.vec(load_ty, v->Width());
            switch (v->Width()) {
                case 2:
                    fn = is_f16 ? BuiltinFn::kLoad2F16 : BuiltinFn::kLoad2;
                    break;
                case 3:
                    fn = is_f16 ? BuiltinFn::kLoad3F16 : BuiltinFn::kLoad3;
                    break;
                case 4:
                    fn = is_f16 ? BuiltinFn::kLoad4F16 : BuiltinFn::kLoad4;
                    break;
                default:
                    TINT_UNREACHABLE();
            }
        }

        auto* builtin = b.MemberCall<hlsl::ir::MemberBuiltinCall>(load_ty, fn, var, offset);
        core::ir::Call* res = nullptr;

        // Do not bitcast the `f16` conversions as they need to be a templated Load instruction
        if (is_f16) {
            res = builtin;
        } else {
            res = b.Bitcast(result_ty, builtin->Result(0));
        }
        return res;
    }

    // Creates a load function for the given `var` and `struct` combination. Essentially creates
    // a function similar to:
    //
    // fn custom_load_S(offset: u32) {
    //   let a = <load S member 0>(offset + member 0 offset);
    //   let b = <load S member 1>(offset + member 1 offset);
    //   ...
    //   let z = <load S member last>(offset + member last offset);
    //   return S(a, b, ..., z);
    // }
    core::ir::Function* GetLoadFunctionFor(core::ir::Instruction* inst,
                                           core::ir::Var* var,
                                           const core::type::Struct* s) {
        return var_and_type_to_load_fn_.GetOrAdd(VarTypePair{var, s}, [&] {
            auto* p = b.FunctionParam("offset", ty.u32());
            auto* fn = b.Function(s);
            fn->SetParams({p});

            b.Append(fn->Block(), [&] {
                Vector<core::ir::Value*, 4> values;
                for (const auto* mem : s->Members()) {
                    values.Push(MakeLoad(inst, var, mem->Type(),
                                         b.Add<u32>(p, u32(mem->Offset()))->Result(0))
                                    ->Result(0));
                }

                b.Return(fn, b.Construct(s, values));
            });

            return fn;
        });
    }

    // Creates a load function for the given `var` and `matrix` combination. Essentially creates
    // a function similar to:
    //
    // fn custom_load_M(offset: u32) {
    //   let a = <load M column 1>(offset + (1 * ColumnStride));
    //   let b = <load M column 2>(offset + (2 * ColumnStride));
    //   ...
    //   let z = <load M column last>(offset + (last * ColumnStride));
    //   return M(a, b, ... z);
    // }
    core::ir::Function* GetLoadFunctionFor(core::ir::Instruction* inst,
                                           core::ir::Var* var,
                                           const core::type::Matrix* mat) {
        return var_and_type_to_load_fn_.GetOrAdd(VarTypePair{var, mat}, [&] {
            auto* p = b.FunctionParam("offset", ty.u32());
            auto* fn = b.Function(mat);
            fn->SetParams({p});

            b.Append(fn->Block(), [&] {
                Vector<core::ir::Value*, 4> values;
                for (size_t i = 0; i < mat->columns(); ++i) {
                    auto* add = b.Add<u32>(p, u32(i * mat->ColumnStride()));
                    auto* load = MakeLoad(inst, var, mat->ColumnType(), add->Result(0));
                    values.Push(load->Result(0));
                }

                b.Return(fn, b.Construct(mat, values));
            });

            return fn;
        });
    }

    // Creates a load function for the given `var` and `array` combination. Essentially creates
    // a function similar to:
    //
    // fn custom_load_A(offset: u32) {
    //   A a = A();
    //   u32 i = 0;
    //   loop {
    //     if (i >= A length) {
    //       break;
    //     }
    //     a[i] = <load array type>(offset + (i * A->Stride()));
    //     i = i + 1;
    //   }
    //   return a;
    // }
    core::ir::Function* GetLoadFunctionFor(core::ir::Instruction* inst,
                                           core::ir::Var* var,
                                           const core::type::Array* arr) {
        return var_and_type_to_load_fn_.GetOrAdd(VarTypePair{var, arr}, [&] {
            auto* p = b.FunctionParam("offset", ty.u32());
            auto* fn = b.Function(arr);
            fn->SetParams({p});

            b.Append(fn->Block(), [&] {
                auto* result_arr = b.Var<function>("a", b.Zero(arr));

                auto* count = arr->Count()->As<core::type::ConstantArrayCount>();
                TINT_ASSERT(count);

                b.LoopRange(ty, 0_u, u32(count->value), 1_u, [&](core::ir::Value* idx) {
                    auto* access = b.Access(ty.ptr<function>(arr->ElemType()), result_arr, idx);
                    auto* stride = b.Multiply<u32>(idx, u32(arr->Stride()));
                    auto* byte_offset = b.Add<u32>(p, stride);
                    b.Store(access, MakeLoad(inst, var, arr->ElemType(), byte_offset->Result(0)));
                });

                b.Return(fn, b.Load(result_arr));
            });

            return fn;
        });
    }

    void InsertLoad(core::ir::Var* var, core::ir::Instruction* inst, OffsetData offset) {
        b.InsertBefore(inst, [&] {
            auto* call =
                MakeLoad(inst, var, inst->Result(0)->Type()->UnwrapPtr(), OffsetToValue(offset));
            inst->Result(0)->ReplaceAllUsesWith(call->Result(0));
        });
        inst->Destroy();
    }

    void Access(core::ir::Access* a,
                core::ir::Var* var,
                const core::type::Type* obj,
                OffsetData* offset) {
        TINT_ASSERT(offset);

        // Note, because we recurse through the `access` helper, the object passed in isn't
        // necessarily the originating `var` object, but maybe a partially resolved access chain
        // object.
        if (auto* view = obj->As<core::type::MemoryView>()) {
            obj = view->StoreType();
        }

        for (auto* idx_value : a->Indices()) {
            tint::Switch(
                obj,  //
                [&](const core::type::Vector* v) {
                    b.InsertBefore(a,
                                   [&] { UpdateOffsetData(idx_value, v->type()->Size(), offset); });
                    obj = v->type();
                },
                [&](const core::type::Matrix* m) {
                    b.InsertBefore(a,
                                   [&] { UpdateOffsetData(idx_value, m->ColumnStride(), offset); });
                    obj = m->ColumnType();
                },
                [&](const core::type::Array* ary) {
                    b.InsertBefore(a, [&] { UpdateOffsetData(idx_value, ary->Stride(), offset); });
                    obj = ary->ElemType();
                },
                [&](const core::type::Struct* s) {
                    auto* cnst = idx_value->As<core::ir::Constant>();

                    // A struct index must be a constant
                    TINT_ASSERT(cnst);

                    uint32_t idx = cnst->Value()->ValueAs<uint32_t>();
                    auto* mem = s->Members()[idx];
                    offset->byte_offset += mem->Offset();
                    obj = mem->Type();
                },
                TINT_ICE_ON_NO_MATCH);
        }

        // Copy the usages into a vector so we can remove items from the hashset.
        auto usages = a->Result(0)->Usages().Vector();
        while (!usages.IsEmpty()) {
            auto usage = usages.Pop();
            tint::Switch(
                usage.instruction,
                [&](core::ir::Let* let) {
                    // The `let` is essentially an alias to the `access`. So, add the `let`
                    // usages into the usage worklist, and replace the let with the access chain
                    // directly.
                    for (auto& u : let->Result(0)->Usages()) {
                        usages.Push(u);
                    }
                    let->Result(0)->ReplaceAllUsesWith(a->Result(0));
                    let->Destroy();
                },
                [&](core::ir::Access* sub_access) {
                    // Treat an access chain of the access chain as a continuation of the outer
                    // chain. Pass through the object we stopped at and the current byte_offset
                    // and then restart the access chain replacement for the new access chain.
                    Access(sub_access, var, obj, offset);
                },

                [&](core::ir::LoadVectorElement* lve) {
                    a->Result(0)->RemoveUsage(usage);

                    b.InsertBefore(lve, [&] {
                        UpdateOffsetData(lve->Index(), obj->DeepestElement()->Size(), offset);
                    });
                    InsertLoad(var, lve, *offset);
                },
                [&](core::ir::Load* ld) {
                    a->Result(0)->RemoveUsage(usage);
                    InsertLoad(var, ld, *offset);
                },

                [&](core::ir::StoreVectorElement*) {
                    // TODO(dsinclair): Handle store vector elements
                },  //
                [&](core::ir::Store*) {
                    // TODO(dsinclair): Handle store
                },  //
                [&](core::ir::CoreBuiltinCall* call) {
                    // Array length calls require the access
                    TINT_ASSERT(call->Func() == core::BuiltinFn::kArrayLength);

                    // If this access chain is being used in an `arrayLength` call then the access
                    // chain _must_ have resolved to the runtime array member of the structure. So,
                    // we _must_ have set `obj` to the array member which is a runtime array.
                    ArrayLength(var, call, obj, offset->byte_offset);
                },  //
                TINT_ICE_ON_NO_MATCH);
        }

        a->Destroy();
    }

    void Store(core::ir::Store*) {
        // TODO(dsinclair): Handle store
    }

    // This should _only_ be handling a `var` parameter as any `access` parameters would have
    // been replaced by the `access` being converted.
    void Load(core::ir::Load* ld, core::ir::Var* var) {
        auto* result = ld->From()->As<core::ir::InstructionResult>();
        TINT_ASSERT(result);

        auto* inst = result->Instruction()->As<core::ir::Var>();
        TINT_ASSERT(inst);

        const core::type::Type* result_ty = inst->Result(0)->Type()->UnwrapPtr();

        b.InsertBefore(ld, [&] {
            auto* call = MakeLoad(ld, var, result_ty, b.Value(0_u));
            ld->Result(0)->ReplaceAllUsesWith(call->Result(0));
        });
        ld->Destroy();
    }

    // Converts to:
    //
    // %1:u32 = v.Load 0u
    // %b:f32 = bitcast %1
    void LoadVectorElement(core::ir::LoadVectorElement* lve,
                           core::ir::Var* var,
                           const core::type::Pointer* var_ty) {
        b.InsertBefore(lve, [&] {
            OffsetData offset{};
            UpdateOffsetData(lve->Index(), var_ty->StoreType()->DeepestElement()->Size(), &offset);

            auto* result =
                MakeScalarOrVectorLoad(var, lve->Result(0)->Type(), OffsetToValue(offset));
            lve->Result(0)->ReplaceAllUsesWith(result->Result(0));
        });

        lve->Destroy();
    }

    // Converts to:
    //
    // %1 = <sve->Value()>
    // %2:u32 = bitcast %1
    // %3:void = v.Store 0u, %2
    void StoreVectorElement(core::ir::StoreVectorElement* sve,
                            core::ir::Var* var,
                            const core::type::Pointer* var_ty) {
        b.InsertBefore(sve, [&] {
            OffsetData offset{};
            UpdateOffsetData(sve->Index(), var_ty->StoreType()->DeepestElement()->Size(), &offset);

            auto* cast = b.Bitcast(ty.u32(), sve->Value());
            b.MemberCall<hlsl::ir::MemberBuiltinCall>(ty.void_(), BuiltinFn::kStore, var,
                                                      OffsetToValue(offset), cast);
        });
        sve->Destroy();
    }
};

}  // namespace

Result<SuccessType> DecomposeMemoryAccess(core::ir::Module& ir) {
    auto result = ValidateAndDumpIfNeeded(ir, "DecomposeMemoryAccess transform");
    if (result != Success) {
        return result.Failure();
    }

    State{ir}.Process();

    return Success;
}

}  // namespace tint::hlsl::writer::raise
