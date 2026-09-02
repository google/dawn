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

#include "src/tint/lang/core/ir/multi_in_block.h"
#include "src/tint/lang/core/ir/validator/validator.h"
#include "src/tint/lang/core/type/function.h"
#include "src/tint/lang/core/type/i32.h"
#include "src/tint/lang/core/type/u32.h"

namespace tint::core::ir::validator {

void Validator::CheckEntryPoint(const Function* func) {
    if (!func->IsEntryPoint()) {
        return;
    }

    ValidateShaderIOAnnotations(func, func->ReturnType(), std::nullopt, func->ReturnAttributes(),
                                ShaderIOKind::kResultValue);

    WalkTypeAndMembers(func, func->ReturnType(), func->ReturnAttributes(),
                       [this](const Function* f, const core::type::Type* t, const IOAttributes&) {
                           CheckNotBool(f, t, "entry point returns can not be 'bool'");
                       });

    for (auto var : referenced_module_vars_.TransitiveReferences(func)) {
        const auto* mv = var->Result()->Type()->As<core::type::MemoryView>();
        const auto* ty = var->Result()->Type()->UnwrapPtrOrRef();
        const auto attr = var->Attributes();
        if (!mv || !ty) {
            continue;
        }

        switch (mv->AddressSpace()) {
            case AddressSpace::kIn:
            case AddressSpace::kOut:
                break;
            default:
                continue;
        }

        if (func->IsFragment() && mv->AddressSpace() == AddressSpace::kIn) {
            WalkTypeAndMembers(var, ty, attr, [this](const auto* v, const auto* t, const auto& a) {
                CheckFrontFacingIfBool(v->Result(), a, t,
                                       "input address space values referenced by fragment shaders "
                                       "can only be 'bool' if decorated with "
                                       "@builtin(front_facing)");
            });
        } else {
            WalkTypeAndMembers(var, ty, attr, [this](const auto* v, const auto* t, const auto&) {
                CheckNotBool(v->Result(), t,
                             "IO address space values referenced by shader entry points can "
                             "only be 'bool' if in the input space, used only by fragment "
                             "shaders and decorated with @builtin(front_facing)");
            });
        }
    }
}

void Validator::CheckFunction(const Function* func) {
    // Scope holds the parameters and block
    scope_stack_.Push();
    TINT_DEFER(scope_stack_.Pop());

    // The recursion checks require this to be true as it will be asserted by the
    // referenced_functions helper.
    func->ForEachUseUnsorted([&](const Usage& use) {
        if (use.instruction->As<UserCall>() || use.instruction->As<Return>()) {
            return;
        }
        AddError(use.instruction, use.operand_index) << "function may not be used as a operand";
    });

    if (!func->Type() || !func->Type()->Is<core::type::Function>()) {
        AddError(func) << "functions must have type '<function>'";
        return;
    }

    // Note: This is not a validator error because Function::SetBlock() asserts that the block is
    // not null, and the disassembler will crash if this is not null. This should only be hit due
    // to some sort of corruption, not a bad shader/programmer error.
    TINT_ASSERT(func->Block()) << "root block for function is undefined";

    if (func->Block()->Is<ir::MultiInBlock>()) {
        AddError(func) << "root block for function cannot be a multi-in block";
        return;
    }

    Hashset<const FunctionParam*, 4> param_set{};
    for (auto* param : func->Params()) {
        if (!CheckFunctionParam(func, param, param_set)) {
            return;
        }

        scope_stack_.Add(param);
    }

    CheckType(func->ReturnType(), [&]() -> diag::Diagnostic& { return AddError(func); });

    ValidateIOAttributes(func);
    CheckWorkgroupSize(func);
    CheckSubgroupSize(func);

    CheckEntryPoint(func);

    QueueBlock(func->Block());
    ProcessTasks();
}

bool Validator::CheckFunctionParam(const Function* func,
                                   const FunctionParam* param,
                                   Hashset<const FunctionParam*, 4>& param_set) {
    if (!param->Alive()) {
        AddError(param) << "destroyed parameter found in function parameter list";
        return false;
    }

    if (!param_set.Add(param)) {
        AddError(param) << "function parameter is not unique";
        return false;
    }

    if (!param->Type()) {
        AddError(param) << "function parameter has nullptr type";
        return false;
    }

    if (!param->Function()) {
        AddError(param) << "function parameter has nullptr parent function";
        return false;
    }

    if (param->Function() != func) {
        AddError(param) << "function parameter has incorrect parent function";
        AddNote(param->Function()) << "parent function declared here";
        return false;
    }

    CheckType(param->Type(), [&]() -> diag::Diagnostic& { return AddError(param); });

    if (func->IsFragment()) {
        WalkTypeAndMembers(param, param->Type(), param->Attributes(),
                           [this](const auto* p, const auto* t, const auto& a) {
                               CheckFrontFacingIfBool(
                                   p, a, t,
                                   "fragment entry point params can only be a bool if "
                                   "decorated with @builtin(front_facing)");
                           });
    } else if (func->IsEntryPoint()) {
        WalkTypeAndMembers(
            param, param->Type(), param->Attributes(),
            [this](const auto* p, const auto* t, const auto&) {
                CheckNotBool(p, t, "entry point params can only be a bool for fragment shaders");
            });
    }

    if (func->IsEntryPoint()) {
        ValidateShaderIOAnnotations(param, param->Type(), param->BindingPoint(),
                                    param->Attributes(), ShaderIOKind::kInputParam);
    } else {
        if (param->BindingPoint().has_value()) {
            AddError(param) << "input param to non-entry point function has a binding point set";
            return false;
        }

        if (param->Builtin().has_value()) {
            AddError(param) << "builtins can only be decorated on entry point params";
            return false;
        }
    }
    return true;
}

void Validator::CheckWorkgroupSize(const Function* func) {
    if (!func->IsCompute()) {
        if (func->WorkgroupSize().has_value()) {
            AddError(func) << "@workgroup_size only valid on compute entry point";
        }
        return;
    }

    if (!func->WorkgroupSize().has_value()) {
        AddError(func) << "compute entry point requires @workgroup_size";
        return;
    }

    auto workgroup_sizes = func->WorkgroupSize().value();
    // The number parameters cannot be checked here, since it is stored internally as a 3 element
    // array, so will always have 3 elements at this point.
    TINT_ASSERT(workgroup_sizes.size() == 3);

    uint64_t total_size = 1;

    std::optional<const core::type::Type*> sizes_ty;
    for (auto* size : workgroup_sizes) {
        if (!size || !size->Type()) {
            AddError(func) << "a @workgroup_size param is undefined or missing a type";
            return;
        }

        auto* ty = size->Type();
        if (!ty->IsAnyOf<core::type::I32, core::type::U32>()) {
            AddError(func) << "@workgroup_size params must be an 'i32' or 'u32', received "
                           << NameOf(ty);
            return;
        }

        if (!sizes_ty.has_value()) {
            sizes_ty = ty;
        }

        if (sizes_ty != ty) {
            AddError(func) << "@workgroup_size params must be all 'i32's or all 'u32's";
            return;
        }

        if (auto* c = size->As<ir::Constant>()) {
            if (c->Value()->ValueAs<int64_t>() <= 0) {
                AddError(func) << "@workgroup_size params must be greater than 0";
                return;
            }
            total_size *= c->Value()->ValueAs<uint64_t>();

            constexpr uint64_t kMaxGridSize = 0xffffffff;
            if (total_size > kMaxGridSize) {
                AddError(func) << "workgroup grid size cannot exceed 0x" << std::hex
                               << kMaxGridSize;
            }
            continue;
        }

        if (!ir_.properties.Contains(Property::kAllowOverrides)) {
            AddError(func) << "@workgroup_size param is not a constant value, and IR property "
                              "'AllowOverrides' is not enabled";
            return;
        }

        if (auto* r = size->As<ir::InstructionResult>()) {
            if (!r->Instruction()) {
                AddError(func) << "instruction for @workgroup_size param is not defined";
                return;
            }

            if (r->Instruction()->Block() != ir_.root_block) {
                AddError(func) << "@workgroup_size param defined by non-module scope value";
                return;
            }

            // Since above, it is already checked if the value is in the root block, it is assumed
            // to be pipeline creatable here, i.e. const/override or derived from consts and
            // overrides.
            // If that is not true, that indicates an issue in CheckRootBlock().
            continue;
        }

        AddError(func) << "@workgroup_size must be an InstructionResult or a Constant";
    }
}

void Validator::CheckSubgroupSize(const Function* func) {
    // @subgroup_size is optional
    if (!func->SubgroupSize().has_value()) {
        return;
    }

    auto subgroup_size = func->SubgroupSize().value();
    if (subgroup_size == nullptr) {
        AddError(func) << "a @subgroup_size param must have a value";
        return;
    }

    if (!subgroup_size->Type()) {
        AddError(func) << "a @subgroup_size param is missing a type";
        return;
    }

    if (subgroup_size->Is<Constant>()) {
        return;
    }

    if (!ir_.properties.Contains(Property::kAllowOverrides)) {
        AddError(func) << "@subgroup_size param is not a constant value, and IR property "
                          "'AllowOverrides' is not enabled";
        return;
    }

    if (auto* r = subgroup_size->As<ir::InstructionResult>()) {
        if (!r->Instruction()) {
            AddError(func) << "instruction for @subgroup_size param is not defined";
            return;
        }

        if (r->Instruction()->Block() != ir_.root_block) {
            AddError(func) << "@subgroup_size param defined by non-module scope value";
            return;
        }

        if (r->Instruction()->Is<Override>()) {
            return;
        }
    }

    AddError(func) << "@subgroup_size must be an InstructionResult or a Constant";
}

}  // namespace tint::core::ir::validator
