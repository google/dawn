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

#include "src/tint/lang/core/ir/transform/polyfill_bool_vector_dynamic_loads.h"

#include <cstdint>

#include "src/tint/lang/core/fluent_types.h"
#include "src/tint/lang/core/ir/builder.h"
#include "src/tint/lang/core/ir/module.h"
#include "src/tint/lang/core/ir/validator/validate.h"
#include "src/tint/lang/core/type/vector.h"

namespace tint::core::ir::transform {

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

    /// Process the module.
    void Process() {
        Vector<core::ir::Access*, 8> worklist;
        for (auto* inst : ir.Instructions()) {
            if (auto* access = inst->As<core::ir::Access>(); access && ShouldReplace(access)) {
                worklist.Push(access);
            }
        }

        for (auto* access : worklist) {
            b.InsertBefore(access, [&] { ReplaceWithSelectChain(access); });
            access->Destroy();
        }
    }

    /// @returns true if @p access reads an element of a boolean vector value with a non-constant
    /// index.
    bool ShouldReplace(core::ir::Access* access) {
        auto indices = access->Indices();
        return access->Object()->Type()->IsBoolVector() && indices.size() == 1 &&
               !indices[0]->Is<core::ir::Constant>();
    }

    /// Replaces `vec[index]` with a chain of selects over constant-index reads.
    /// Out-of-bounds indices return vec[0].
    /// For example, if `vec` is a vec3, `vec[index]` will become:
    ///   index == 2 ? vec[2] : (index == 1 ? vec[1] : vec[0])
    void ReplaceWithSelectChain(core::ir::Access* access) {
        auto* vec = access->Object();
        uint32_t width = vec->Type()->As<core::type::Vector>()->Width();
        auto* index = b.InsertConvertIfNeeded(ty.u32(), access->Indices()[0]);

        core::ir::Value* result = b.Access<bool>(vec, 0_u);
        for (uint32_t i = 1; i < width; ++i) {
            auto* elem = b.Access<bool>(vec, u32(i));
            auto* cond = b.Equal(index, u32(i));
            auto* dst = (i + 1 == width) ? access->DetachResult() : b.InstructionResult(ty.bool_());
            result = b.CallReplaceResult(dst, core::BuiltinFn::kSelect, result, elem, cond);
        }
    }
};

}  // namespace

Result<SuccessType> PolyfillBoolVectorDynamicLoads(core::ir::Module& ir) {
    AssertValid(ir, "before core.PolyfillBoolVectorDynamicLoads");

    State{ir}.Process();

    return Success;
}

}  // namespace tint::core::ir::transform
