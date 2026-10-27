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

#include "src/tint/lang/glsl/writer/raise/hoist_constant_array_initializers.h"

#include <string>
#include <utility>

#include "src/tint/lang/core/ir/builder.h"
#include "src/tint/lang/core/ir/module.h"
#include "src/tint/lang/core/ir/validator/validate.h"
#include "src/tint/lang/core/type/array.h"
#include "src/tint/lang/core/type/struct.h"
#include "src/tint/utils/containers/hashmap.h"
#include "src/tint/utils/containers/vector.h"

using namespace tint::core::fluent_types;  // NOLINT

namespace tint::glsl::writer::raise {
namespace {

/// Returns true if @p type contains an array inside a structure.
bool HasArrayInsideStruct(const core::type::Type* type) {
    if (auto* str = type->As<core::type::Struct>()) {
        for (auto* member : str->Members()) {
            if (member->Type()->Is<core::type::Array>()) {
                return true;
            }
            if (HasArrayInsideStruct(member->Type())) {
                return true;
            }
        }
        return false;
    }
    if (auto* arr = type->As<core::type::Array>()) {
        return HasArrayInsideStruct(arr->ElemType());
    }
    return false;
}

/// State for the transform.
struct State {
    /// The IR module.
    core::ir::Module& ir;

    /// The IR builder.
    core::ir::Builder b{ir};

    /// The type manager.
    core::type::Manager& ty{ir.Types()};

    /// Process the module.
    void Process() {
        // Process variable initializers first to handle instructions in the root block and implicit
        // zero initializers.
        ProcessVariableInitializers();

        // Find all constants that match the pattern and replace their usages.
        Vector<core::ir::Constant*, 16> worklist;
        for (auto& entry : ir.constants) {
            if (HasArrayInsideStruct(entry.value->Type())) {
                worklist.Push(entry.value);
            }
        }
        for (auto* c : worklist) {
            for (auto use : c->UsagesSorted()) {
                auto* inst = use.instruction;
                b.InsertBefore(inst, [&] {
                    auto* replacement = Convert(c->Value());
                    inst->SetOperand(use.operand_index, replacement);
                });
            }
        }
    }

    /// Map from struct type to constructor helper function.
    Hashmap<const core::type::Struct*, core::ir::Function*, 4> constructor_helpers_{};

    /// Gets or creates a constructor helper function for @p str.
    core::ir::Function* GetConstructorHelper(const core::type::Struct* str) {
        return constructor_helpers_.GetOrAdd(str, [&] {
            std::string name = "tint_construct_" + str->IdentifierName();
            auto* fn = b.Function(name, str);

            b.Append(fn->Block(), [&] {
                auto* var = b.Var(ty.ptr<function>(str));
                var->SetInitializer(b.Undef(str));

                Vector<core::ir::FunctionParam*, 8> params;
                for (auto* mem : str->Members()) {
                    params.Push(b.FunctionParam(mem->Name().NameView(), mem->Type()));
                }
                for (auto* mem : str->Members()) {
                    auto idx = mem->Index();
                    auto* access = b.Access(ty.ptr<function>(mem->Type()), var, u32(idx));
                    b.Store(access, params[idx]);
                }
                fn->SetParams(std::move(params));

                b.Return(fn, b.Load(var));
            });

            return fn;
        });
    }

    /// Recursively converts @p val into `let` and `construct` instructions.
    core::ir::Value* Convert(const core::constant::Value* val) {
        if (auto* str = val->Type()->As<core::type::Struct>()) {
            // Rewrite the struct initializer as a call to a constructor helper function.
            // Calling the struct constructor directly hits a bug in some GLSL compiler frontends.
            Vector<core::ir::Value*, 8> args;
            for (size_t i = 0; i < str->Members().Length(); ++i) {
                auto* member = str->Members()[i];
                auto* member_val = val->Index(i);
                auto* member_type = member->Type();

                if (member_type->Is<core::type::Array>()) {
                    // Hoist the array initializer into a `let` instruction, and use that as the
                    // argument to the struct constructor.
                    core::ir::Value* arr_val = nullptr;
                    if (HasArrayInsideStruct(member_type)) {
                        arr_val = Convert(member_val);
                    } else {
                        arr_val = b.Constant(member_val);
                    }
                    auto* arr_let = b.Let(arr_val);
                    args.Push(arr_let->Result());
                } else if (member_type->Is<core::type::Struct>() &&
                           HasArrayInsideStruct(member_type)) {
                    args.Push(Convert(member_val));
                } else {
                    args.Push(b.Constant(member_val));
                }
            }
            return b.Call(GetConstructorHelper(str), std::move(args))->Result();
        }

        if (auto* arr = val->Type()->As<core::type::Array>()) {
            // Recurse to handle structs nested inside this array.
            Vector<core::ir::Value*, 8> elem_args;
            for (size_t j = 0; j < val->NumElements(); ++j) {
                elem_args.Push(Convert(val->Index(j)));
            }
            return b.Construct(arr, std::move(elem_args));
        }

        return b.Constant(val);
    }

    void ProcessVariableInitializers() {
        // Find the entry point function.
        core::ir::Function* ep = nullptr;
        for (auto* func : ir.functions) {
            if (func->IsEntryPoint()) {
                ep = func;
                break;
            }
        }
        TINT_IR_ASSERT(ir, ep);

        Vector<core::ir::Var*, 16> worklist;
        for (auto* inst : ir.Instructions()) {
            if (auto* var = inst->As<core::ir::Var>()) {
                worklist.Push(var);
            }
        }

        for (auto* var : worklist) {
            auto* ptr = var->Result()->Type()->As<core::type::Pointer>();
            if (ptr->AddressSpace() != core::AddressSpace::kFunction &&
                ptr->AddressSpace() != core::AddressSpace::kPrivate) {
                continue;
            }
            if (!HasArrayInsideStruct(ptr->StoreType())) {
                continue;
            }

            // Get the provided initializer or the implicit zero initializer.
            const core::constant::Value* init_val = nullptr;
            if (!var->Initializer()) {
                init_val = ir.constant_values.Zero(ptr->StoreType());
            } else if (auto* c = var->Initializer()->As<core::ir::Constant>()) {
                init_val = c->Value();
            } else {
                continue;
            }

            if (ptr->AddressSpace() == core::AddressSpace::kFunction) {
                b.InsertBefore(var, [&] {
                    auto* replacement = Convert(init_val);
                    var->SetInitializer(replacement);
                });
            } else if (ptr->AddressSpace() == core::AddressSpace::kPrivate) {
                // In GLSL ES, global initializers must be constant expressions. To prevent the
                // compiler from re-inlining global constants into the struct constructor, we leave
                // the global uninitialized at module scope and initialize it at the beginning of
                // each entry point instead.
                b.InsertBefore(ep->Block()->Front(), [&] {
                    auto* replacement = Convert(init_val);
                    b.Store(var, replacement);
                    var->SetInitializer(b.Undef(ptr->StoreType()));
                });
            }
        }
    }
};

}  // namespace

Result<SuccessType> HoistConstantArrayInitializers(core::ir::Module& ir) {
    core::ir::AssertValid(ir, "before glsl.HoistConstantArrayInitializers");

    State{ir}.Process();

    return Success;
}

}  // namespace tint::glsl::writer::raise
