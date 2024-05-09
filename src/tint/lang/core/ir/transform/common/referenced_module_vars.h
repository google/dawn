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

#ifndef SRC_TINT_LANG_CORE_IR_TRANSFORM_COMMON_REFERENCED_MODULE_VARS_H_
#define SRC_TINT_LANG_CORE_IR_TRANSFORM_COMMON_REFERENCED_MODULE_VARS_H_

#include <functional>

#include "src/tint/lang/core/ir/module.h"
#include "src/tint/lang/core/ir/var.h"
#include "src/tint/utils/containers/hashmap.h"
#include "src/tint/utils/containers/unique_vector.h"

// Forward declarations.
namespace tint::core::ir {
class Block;
class Function;
}  // namespace tint::core::ir

namespace tint::core::ir {

/// ReferencedModuleVars is a helper to determine the set of module-scope variables that are
/// transitively referenced by functions in a module.
/// References are determined lazily and cached for future requests.
///
/// Note: changes to the module can invalidate the cached data. This is intended to be created by
/// a transform that need this information, and discarded when that transform completes. Tracking
/// this information inside the IR module would add overhead any time an instruction is added or
/// removed from the module. Since only a few transforms need this information, we expect it to be
/// more efficient to generate it as and when needed instead.
class ReferencedModuleVars {
  public:
    /// The signature of a predicate used to filter variables.
    /// A predicate function should return `true` when the variable should be added to the set.
    using Predicate = std::function<bool(const Var*)>;

    /// A set of a variables referenced by a function (in declaration order).
    using VarSet = UniqueVector<Var*, 16>;

    /// Constructor.
    /// @param ir the module
    /// @param pred an optional predicate function for filtering variables
    /// Note: @p pred is not stored by the class, so can be a lambda that captures by reference.
    explicit ReferencedModuleVars(Module& ir, Predicate&& pred = {}) : ir_(ir) {
        // Loop over module-scope variables, recording the blocks that they are referenced from.
        for (auto inst : *ir_.root_block) {
            if (auto* var = inst->As<Var>()) {
                if (!pred || pred(var)) {
                    var->Result(0)->ForEachUse([&](const Usage& use) {
                        block_to_direct_vars_.GetOrAddZero(use.instruction->Block()).Add(var);
                    });
                }
            }
        }
    }

    /// Get the set of transitively referenced module-scope variables for a function, filtered by
    /// the predicate function if provided.
    /// @param func the function
    /// @returns the set of (possibly filtered) transitively reference module-scope variables
    const VarSet& TransitiveReferences(Function* func);

  private:
    /// The module.
    Module& ir_;

    /// A map from blocks to their directly referenced variables.
    Hashmap<Block*, VarSet, 64> block_to_direct_vars_{};

    /// A map from functions to their transitively referenced variables.
    Hashmap<Function*, VarSet, 8> transitive_references_;

    /// Get the set of transitively referenced module-scope variables for a block.
    /// @param block the block
    /// @param vars the set of transitively reference module-scope variables to populate
    void GetTransitiveReferences(Block* block, VarSet& vars);
};

}  // namespace tint::core::ir

#endif  // SRC_TINT_LANG_CORE_IR_TRANSFORM_COMMON_REFERENCED_MODULE_VARS_H_
