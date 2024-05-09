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

#include "src/tint/lang/core/ir/transform/common/referenced_module_vars.h"

#include "src/tint/lang/core/ir/control_instruction.h"
#include "src/tint/lang/core/ir/module.h"
#include "src/tint/lang/core/ir/user_call.h"
#include "src/tint/lang/core/ir/var.h"
#include "src/tint/utils/rtti/switch.h"

namespace tint::core::ir {

const ReferencedModuleVars::VarSet& ReferencedModuleVars::TransitiveReferences(Function* func) {
    return transitive_references_.GetOrAdd(func, [&] {
        VarSet vars;
        GetTransitiveReferences(func->Block(), vars);
        return vars;
    });
}

/// Get the set of variables transitively referenced by @p block.
/// @param block the block
/// @param vars the set of transitively referenced variables to populate
void ReferencedModuleVars::GetTransitiveReferences(Block* block, VarSet& vars) {
    // Add directly referenced vars.
    if (auto itr = block_to_direct_vars_.Get(block)) {
        for (auto& var : *itr) {
            vars.Add(var);
        }
    }

    // Loop over instructions in the block to find indirectly referenced vars.
    for (auto* inst : *block) {
        tint::Switch(
            inst,
            [&](UserCall* call) {
                // Get variables referenced by a function called from this block.
                const auto& callee_vars = TransitiveReferences(call->Target());
                for (auto* var : callee_vars) {
                    vars.Add(var);
                }
            },
            [&](ControlInstruction* ctrl) {
                // Recurse into control instructions and gather their referenced vars.
                ctrl->ForeachBlock([&](Block* blk) { GetTransitiveReferences(blk, vars); });
            });
    }
}

}  // namespace tint::core::ir
