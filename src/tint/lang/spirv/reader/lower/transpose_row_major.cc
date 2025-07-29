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

#include "src/tint/lang/spirv/reader/lower/transpose_row_major.h"

#include <utility>

#include "src/tint/lang/core/ir/builder.h"
#include "src/tint/lang/core/ir/module.h"
#include "src/tint/lang/core/ir/validator.h"
#include "src/tint/lang/core/type/matrix.h"
#include "src/tint/lang/spirv/type/explicit_layout_array.h"

namespace tint::spirv::reader::lower {
namespace {

using namespace tint::core::fluent_types;  // NOLINT

/// PIMPL state for the transform.
struct State {
    /// The IR module.
    core::ir::Module& ir;

    /// The IR builder.
    core::ir::Builder b{ir};

    /// The type manager.
    core::type::Manager& ty{ir.Types()};

    /// The symbol manager.
    SymbolTable& sym{ir.symbols};

    /// Process the module.
    void Process() {
        for (auto* type : ty) {
            auto* strct = type->As<core::type::Struct>();
            if (!strct) {
                continue;
            }

            auto* replacement = RewriteStruct(strct);
            if (replacement == strct) {
                continue;
            }
        }
    }

    const core::type::Struct* RewriteStruct(const core::type::Struct* src) {
        bool made_changes = false;

        Vector<const core::type::StructMember*, 8> new_members;
        new_members.Reserve(src->Members().Length());
        for (auto* member : src->Members()) {
            auto* new_member_type = RewriteType(member->Type());
            if (member->RowMajor() || new_member_type != member->Type()) {
                // Recreate the struct member without the row-major attribute, and using the new
                // type.
                new_members.Push(ty.Get<core::type::StructMember>(
                    member->Name(), new_member_type, member->Index(), member->Offset(),
                    member->Align(), member->Size(), member->Attributes()));
                made_changes = true;
            } else {
                new_members.Push(member);
            }
        }
        if (!made_changes) {
            return src;
        }

        // TODO(dsinclair): record which members were re-written

        // Create the new struct and record the mapping to the old struct.
        auto* new_struct = ty.Struct(sym.New(src->Name().Name()), std::move(new_members));
        // struct_to_original.Add(new_struct, src);
        return new_struct;
    }

    const core::type::Type* RewriteType([[maybe_unused]] const core::type::Type* type) {
        return nullptr;
    }
};

}  // namespace

Result<SuccessType> TransposeRowMajor(core::ir::Module& ir) {
    auto result = ValidateAndDumpIfNeeded(ir, "spirv.TransposeRowMajor",
                                          core::ir::Capabilities{
                                              core::ir::Capability::kAllowStructMatrixDecorations,
                                              core::ir::Capability::kAllowNonCoreTypes,
                                              core::ir::Capability::kAllowOverrides,
                                          });
    if (result != Success) {
        return result.Failure();
    }

    State{ir}.Process();

    return Success;
}

}  // namespace tint::spirv::reader::lower
