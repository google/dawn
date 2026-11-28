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

#include "src/tint/lang/spirv/writer/raise/polyfill_nested_aggregate_construct.h"

#include "src/tint/lang/core/fluent_types.h"
#include "src/tint/lang/core/ir/builder.h"
#include "src/tint/lang/core/ir/construct.h"
#include "src/tint/lang/core/ir/module.h"
#include "src/tint/lang/core/ir/unused.h"
#include "src/tint/lang/core/ir/validator/validate.h"
#include "src/tint/lang/core/type/array.h"
#include "src/tint/lang/core/type/struct.h"
#include "src/tint/utils/containers/vector.h"

using namespace tint::core::fluent_types;     // NOLINT
using namespace tint::core::number_suffixes;  // NOLINT

namespace tint::spirv::writer::raise {

namespace {

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
        Vector<core::ir::Construct*, 8> worklist;
        for (auto* inst : ir.Instructions()) {
            if (auto* construct = inst->As<core::ir::Construct>()) {
                if (ShouldReplace(construct)) {
                    worklist.Push(construct);
                }
            }
        }

        for (auto* construct : worklist) {
            Replace(construct);
        }
    }

    /// @returns true if @p construct builds a struct or array containing another struct or array.
    bool ShouldReplace(core::ir::Construct* construct) {
        if (construct->Block() == ir.root_block) {
            return false;
        }

        auto args = construct->Args();
        if (args.empty()) {
            // Zero-value constructs are emitted as OpConstantNull.
            return false;
        }

        auto* result_ty = construct->Result()->Type();
        if (!result_ty->IsConstructible()) {
            return false;
        }

        if (!result_ty->IsAnyOf<core::type::Struct, core::type::Array>()) {
            return false;
        }

        for (auto* arg : args) {
            if (!arg || arg->Is<core::ir::Unused>()) {
                continue;
            }
            if (arg->Type()->IsAnyOf<core::type::Struct, core::type::Array>()) {
                return true;
            }
        }

        return false;
    }

    /// Replace @p construct with a function-scope var, element-wise stores, and a load.
    void Replace(core::ir::Construct* construct) {
        auto* result_ty = construct->Result()->Type();
        auto args = construct->Args();

        core::ir::Var* var = nullptr;
        b.InsertBefore(construct, [&] {
            var = b.Var(ty.ptr<function>(result_ty));
            for (uint32_t i = 0; i < args.size(); ++i) {
                auto* arg = args[i];
                if (!arg || arg->Is<core::ir::Unused>()) {
                    continue;
                }
                auto* ptr = b.Access(ty.ptr<function>(arg->Type()), var, u32(i));
                b.Store(ptr, arg);
            }
        });

        auto* load = b.LoadWithResult(construct->DetachResult(), var);
        construct->ReplaceWith(load);
        construct->Destroy();
    }
};

}  // namespace

Result<SuccessType> PolyfillNestedAggregateConstruct(core::ir::Module& ir) {
    AssertValid(ir, "before spirv.PolyfillNestedAggregateConstruct");

    State{ir}.Process();

    return Success;
}

}  // namespace tint::spirv::writer::raise
