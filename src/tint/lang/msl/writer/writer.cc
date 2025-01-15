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

#include "src/tint/lang/msl/writer/writer.h"

#include <memory>
#include <utility>

#include "src/tint/lang/core/ir/module.h"
#include "src/tint/lang/core/ir/var.h"
#include "src/tint/lang/core/type/input_attachment.h"
#include "src/tint/lang/msl/writer/ast_printer/ast_printer.h"
#include "src/tint/lang/msl/writer/common/option_helpers.h"
#include "src/tint/lang/msl/writer/printer/printer.h"
#include "src/tint/lang/msl/writer/raise/raise.h"

namespace tint::msl::writer {

Result<SuccessType> CanGenerate(const core::ir::Module& ir, const Options&) {
    // Check for unsupported module-scope variable address spaces and types.
    for (auto* inst : *ir.root_block) {
        auto* var = inst->As<core::ir::Var>();
        auto* ptr = var->Result(0)->Type()->As<core::type::Pointer>();
        if (ptr->AddressSpace() == core::AddressSpace::kPushConstant) {
            return Failure("push constants are not supported by the MSL backend");
        }
        if (ptr->AddressSpace() == core::AddressSpace::kPixelLocal) {
            return Failure("pixel_local address space is not supported by the MSL backend");
        }
        if (ptr->StoreType()->Is<core::type::InputAttachment>()) {
            return Failure("input attachments are not supported by the MSL backend");
        }
    }
    return Success;
}

Result<Output> Generate(core::ir::Module& ir, const Options& options) {
    {
        auto res = ValidateBindingOptions(options);
        if (res != Success) {
            return res.Failure();
        }
    }

    Output output;

    // Raise from core-dialect to MSL-dialect.
    auto raise_result = Raise(ir, options);
    if (raise_result != Success) {
        return raise_result.Failure();
    }

    auto result = Print(ir, options);
    if (result != Success) {
        return result.Failure();
    }

    result->needs_storage_buffer_sizes = raise_result->needs_storage_buffer_sizes;
    return result;
}

Result<Output> Generate(const Program& program, const Options& options) {
    if (!program.IsValid()) {
        return Failure{program.Diagnostics()};
    }

    {
        auto res = ValidateBindingOptions(options);
        if (res != Success) {
            return res.Failure();
        }
    }

    Output output;

    // Sanitize the program.
    auto sanitized_result = Sanitize(program, options);
    if (!sanitized_result.program.IsValid()) {
        return Failure{sanitized_result.program.Diagnostics()};
    }
    output.needs_storage_buffer_sizes = sanitized_result.needs_storage_buffer_sizes;

    // Generate the MSL code.
    auto impl = std::make_unique<ASTPrinter>(sanitized_result.program, options);
    if (!impl->Generate()) {
        return Failure{impl->Diagnostics()};
    }
    output.msl = impl->Result();
    output.has_invariant_attribute = impl->HasInvariant();
    output.workgroup_info.allocations = impl->DynamicWorkgroupAllocations();

    return output;
}

}  // namespace tint::msl::writer
