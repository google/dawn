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

#include "src/tint/lang/spirv/reader/lower/hoist_constant_array.h"

#include "src/tint/lang/core/ir/builder.h"
#include "src/tint/lang/core/ir/module.h"
#include "src/tint/lang/core/ir/validator/validate.h"
#include "src/tint/utils/rtti/switch.h"

namespace tint::spirv::reader::lower {
namespace {

using namespace tint::core::fluent_types;  // NOLINT

/// PIMPL state for the transform.
struct State {
    /// The IR module.
    core::ir::Module& ir;

    /// Process the module.
    void Process() {
        // Collect candidate function-scope variables:
        //  - in the initial part of the functions's (entry) block.
        //    In SPIR-V, function-scope variable declarations must appear
        //    in the function's entry basic block.
        //  - has no initializer. (Glslang never generates an initializer.)
        //  - has exactly two uses: the store and the access.
        //  - store type is an array.
        Vector<core::ir::Var*, 4> vars;
        for (auto* function : ir.functions) {
            auto* block = function->Block();
            for (auto* inst = block->Front(); inst; inst = inst->next) {
                // Only scan the initial part of the block, before any control instructions.
                if (inst->Is<core::ir::ControlInstruction>()) {
                    break;
                }
                if (auto* var = inst->As<core::ir::Var>()) {
                    if (var->Initializer() != nullptr) {
                        continue;
                    }
                    auto* result = var->Result();
                    if (result->NumUsages() != 2) {
                        continue;
                    }
                    if (!result->Type()->UnwrapPtr()->Is<core::type::Array>()) {
                        continue;
                    }
                    vars.Push(var);
                }
            }
        }

        for (auto* var : vars) {
            // Check that the two uses of the variable are:
            //  - target of Store with %arrval, immediately followed by:
            //  - base of Access with index %i, yielding %p
            //  - %p is used exactly once, and that use is a Load from %p, yielding %elem
            core::ir::Store* store = nullptr;
            core::ir::Access* access = nullptr;
            for (const auto& usage : var->Result()->UsagesUnsorted()) {
                tint::Switch(
                    usage->instruction,
                    [&](core::ir::Store* s) {
                        if (usage->operand_index == core::ir::Store::kToOperandOffset) {
                            store = s;
                        }
                    },
                    [&](core::ir::Access* a) {
                        if (usage->operand_index == core::ir::Access::kObjectOperandOffset) {
                            access = a;
                        }
                    });
            }
            if (!store || !access) {
                continue;
            }
            if (store->next != access) {
                continue;
            }
            if (access->Result()->NumUsages() != 1) {
                continue;
            }
            auto* load =
                (*access->Result()->UsagesUnsorted().begin())->instruction->As<core::ir::Load>();
            if (!load) {
                continue;
            }

            // Replace %elem with access of %arrval, and remove the load, store, and var.
            auto* elem = load->DetachResult();
            load->Destroy();
            access->DetachResult()->Destroy();
            access->SetResult(elem);
            access->SetOperand(core::ir::Access::kObjectOperandOffset, store->From());
            store->Destroy();
            var->Destroy();
        }
    }
};

}  // namespace

Result<SuccessType> HoistConstantArray(core::ir::Module& ir) {
    core::ir::AssertValid(ir, "before spirv.HoistConstantArray");

    State{ir}.Process();

    return Success;
}

}  // namespace tint::spirv::reader::lower
