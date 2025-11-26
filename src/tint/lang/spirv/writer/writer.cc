// Copyright 2020 The Dawn & Tint Authors
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

#include "src/tint/lang/spirv/writer/writer.h"

#include <string>
#include <utility>
#include <vector>

#include "src/tint/lang/core/ir/analysis/subgroup_matrix.h"
#include "src/tint/lang/core/ir/core_builtin_call.h"
#include "src/tint/lang/core/ir/referenced_module_vars.h"
#include "src/tint/lang/core/ir/validator.h"
#include "src/tint/lang/core/ir/var.h"
#include "src/tint/lang/core/type/binding_array.h"
#include "src/tint/lang/core/type/pointer.h"
#include "src/tint/lang/spirv/writer/common/option_helpers.h"
#include "src/tint/lang/spirv/writer/printer/printer.h"
#include "src/tint/lang/spirv/writer/raise/raise.h"

// Included by 'ast_printer.h', included again here for './tools/run gen' track the dependency.
#include "spirv/unified1/spirv.h"

namespace tint::spirv::writer {

Result<SuccessType> CanGenerate(const core::ir::Module& ir, const Options& options) {
    // The enum is accessible in the API so ensure we have a valid value.
    switch (options.spirv_version) {
        case SpvVersion::kSpv13:
        case SpvVersion::kSpv14:
        case SpvVersion::kSpv15:
            break;
        default:
            return Failure("unsupported SPIR-V version");
    }

    // Check optionally supported types against their required options.
    for (auto* ty : ir.Types()) {
        if (ty->Is<core::type::SubgroupMatrix>()) {
            if (!options.use_vulkan_memory_model) {
                return Failure("using subgroup matrices requires the Vulkan Memory Model");
            }
        }
    }

    // If a remapped entry point name is provided, it must not be empty, and must not contain
    // embedded null characters.
    if (!options.remapped_entry_point_name.empty()) {
        if (options.remapped_entry_point_name.find('\0') != std::string::npos) {
            return Failure("remapped entry point name contains null character");
        }

        // Check for multiple entry points.
        // TODO(375388101): Remove this check when SingleEntryPoint is part of the backend.
        bool has_entry_point = false;
        for (auto& func : ir.functions) {
            if (func->IsEntryPoint()) {
                if (has_entry_point) {
                    return Failure("module must only contain a single entry point");
                }
                has_entry_point = true;
            }
        }
    }

    core::ir::Function* ep_func = nullptr;
    for (auto* f : ir.functions) {
        if (!f->IsEntryPoint()) {
            continue;
        }
        if (ir.NameOf(f).NameView() == options.entry_point_name) {
            ep_func = f;
            break;
        }
    }

    // No entrypoint, so no bindings needed
    if (!ep_func) {
        return Failure("entry point not found");
    }

    core::ir::ReferencedModuleVars<const core::ir::Module> referenced_module_vars{ir};
    auto& refs = referenced_module_vars.TransitiveReferences(ep_func);

    // Check for unsupported module-scope variable address spaces and ensure at most one user
    // immediate.
    for (auto* var : refs) {
        auto* ptr = var->Result()->Type()->As<core::type::Pointer>();
        if (ptr->AddressSpace() == core::AddressSpace::kPixelLocal) {
            return Failure("pixel_local address space is not supported by the SPIR-V backend");
        }
    }

    auto user_immediate_res = core::ir::ValidateSingleUserImmediate(ir, ep_func);
    if (user_immediate_res != Success) {
        return user_immediate_res.Failure();
    }

    uint32_t user_immediate_size = user_immediate_res.Get();

    if (options.depth_range_offsets) {
        std::vector<core::ir::ImmediateInfo> immediates = {
            {options.depth_range_offsets->max, 4u},
            {options.depth_range_offsets->min, 4u},
        };
        if (auto res =
                core::ir::ValidateInternalImmediateOffset(0x1000, user_immediate_size, immediates);
            res != Success) {
            return res.Failure();
        }
    }

    {
        auto res = ValidateBindingOptions(options);
        if (res != Success) {
            return res.Failure();
        }
    }

    return Success;
}

Result<Output> Generate(core::ir::Module& ir, const Options& options) {
    // Raise from core-dialect to SPIR-V-dialect.
    if (auto res = Raise(ir, options); res != Success) {
        return std::move(res.Failure());
    }

    auto res = Print(ir, options);
    if (res != Success) {
        return res;
    }

    res->subgroup_matrix_info = core::ir::analysis::GatherSubgroupMatrixInfo(ir);

    return res;
}

}  // namespace tint::spirv::writer
