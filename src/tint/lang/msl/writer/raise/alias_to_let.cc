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

#include "src/tint/lang/msl/writer/raise/alias_to_let.h"

#include "src/tint/lang/core/ir/builder.h"
#include "src/tint/lang/core/ir/validator/validate.h"
#include "src/tint/lang/msl/ir/builtin_call.h"

using namespace tint::core::fluent_types;     // NOLINT
using namespace tint::core::number_suffixes;  // NOLINT

namespace tint::msl::writer::raise {

namespace {

struct State {
    core::ir::Module& ir;

    core::ir::Builder b{ir};

    void Process() {
        Vector<msl::ir::BuiltinCall*, 16> worklist;
        for (auto* inst : ir.Instructions()) {
            if (auto* builtin = inst->As<msl::ir::BuiltinCall>()) {
                if (builtin->Func() == msl::BuiltinFn::kAliasPointerOffset) {
                    worklist.Push(builtin);
                }
            }
        }

        for (auto* builtin : worklist) {
            b.InsertAfter(builtin, [&] {
                auto* new_res = b.InstructionResult(builtin->Result()->Type());
                auto* old_res = builtin->DetachResult();
                builtin->SetResult(new_res);
                b.LetWithResult(old_res, new_res);
            });
        }
    }
};

}  // namespace

Result<SuccessType> AliasToLet(core::ir::Module& ir) {
    AssertValid(ir, "before msl.AliasToLet");

    State{ir}.Process();

    return Success;
}

}  // namespace tint::msl::writer::raise
