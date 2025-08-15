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

#include "src/tint/lang/core/ir/module.h"
#include "src/tint/lang/core/ir/var.h"
#include "src/tint/lang/core/type/f16.h"
#include "src/tint/lang/core/type/f32.h"
#include "src/tint/lang/core/type/input_attachment.h"
#include "src/tint/lang/core/type/pointer.h"
#include "src/tint/lang/msl/writer/common/option_helpers.h"
#include "src/tint/lang/msl/writer/printer/printer.h"
#include "src/tint/lang/msl/writer/raise/raise.h"

namespace tint::msl::writer {

Result<SuccessType> CanGenerate(const core::ir::Module& ir, const Options& options) {
    // Check for unsupported types.
    for (auto* ty : ir.Types()) {
        if (auto* m = ty->As<core::type::SubgroupMatrix>()) {
            if (!m->Type()->IsAnyOf<core::type::F16, core::type::F32>()) {
                return Failure("non-float subgroup matrices are not supported by the MSL backend");
            }
            if (m->Columns() != 8 || m->Rows() != 8) {
                return Failure("the MSL backend only supports 8x8 subgroup matrices");
            }
        }
    }

    // Check for unsupported module-scope variable address spaces and types.
    // Also make sure there is at most one user-declared immediate data, and make a note of its
    // size.
    uint32_t user_immediate_data_size = 0;
    for (auto* inst : *ir.root_block) {
        auto* var = inst->As<core::ir::Var>();
        auto* ptr = var->Result()->Type()->As<core::type::Pointer>();
        if (ptr->AddressSpace() == core::AddressSpace::kPixelLocal) {
            return Failure("pixel_local address space is not supported by the MSL backend");
        }
        if (ptr->StoreType()->Is<core::type::InputAttachment>()) {
            return Failure("input attachments are not supported by the MSL backend");
        }
        if (ptr->AddressSpace() == core::AddressSpace::kImmediate) {
            if (user_immediate_data_size > 0) {
                // We've already seen a user-declared immediate data.
                return Failure("module contains multiple user-declared immediate data");
            }
            user_immediate_data_size = tint::RoundUp(4u, ptr->StoreType()->Size());
        }
    }

    // Metal suggested smaller than 4KB data uses SetBytes.
    static constexpr uint32_t kMaxOffset = 0x1000;
    Hashset<uint32_t, 4> immediate_data_word_offsets;
    auto check_immediate_data_offset = [&](uint32_t offset) {
        // Offset must be 4-byte aligned.
        if (offset & 0x3) {
            return false;
        }
        // Offset must not have already been used.
        if (!immediate_data_word_offsets.Add(offset >> 2)) {
            return false;
        }
        // Offset must be after the user-defined immediate data.
        if (offset < user_immediate_data_size) {
            return false;
        }
        return true;
    };

    auto valid_buffer_sizes_offset = [](const auto& buffer_sizes_offset) -> bool {
        if (!buffer_sizes_offset) {
            return false;
        }

        // Excessive values can cause OOM / timeouts when padding structures in the printer.
        if (buffer_sizes_offset.value() > kMaxOffset) {
            return false;
        }

        return true;
    };

    // Buffer sizes uses vec4 array which requires 16 bytes alignment.
    if (valid_buffer_sizes_offset(options.array_length_from_constants.buffer_sizes_offset)) {
        if (!check_immediate_data_offset(
                options.array_length_from_constants.buffer_sizes_offset.value()) ||
            (options.array_length_from_constants.buffer_sizes_offset.value() & 0xF)) {
            return Failure("invalid offsets for buffer sizes offset in immediate block");
        }
    }

    // Check the vertex pulling config, if provided.
    if (options.vertex_pulling_config) {
        // Find the vertex entry point.
        const core::ir::Function* ep = nullptr;
        for (auto& func : ir.functions) {
            if (func->IsVertex()) {
                if (ep) {
                    return Failure("vertex pulling config provided with multiple vertex shaders");
                }
                ep = func;
            }
        }
        if (!ep) {
            return Failure("vertex pulling config provided without a vertex shader");
        }

        // Gather all of the vertex attribute locations in the config.
        Hashset<uint32_t, 4> locations;
        for (auto& buffer : options.vertex_pulling_config->vertex_state) {
            if (buffer.array_stride & 3) {
                return Failure(
                    "vertex pulling config contains array stride that is not a multiple of 4");
            }
            for (auto& attr : buffer.attributes) {
                if (!locations.Add(attr.shader_location)) {
                    return Failure("vertex pulling config contains duplicate shader locations");
                }
            }
        }

        // Check the parameters to make sure all vertex attributes are present in the config.
        for (auto* param : ep->Params()) {
            if (auto* str = param->Type()->As<core::type::Struct>()) {
                for (auto* member : str->Members()) {
                    if (auto loc = member->Attributes().location) {
                        if (!locations.Contains(*loc)) {
                            return Failure("shader location " + std::to_string(*loc) +
                                           " missing from vertex pulling map");
                        }
                    }
                }
            } else if (auto loc = param->Location()) {
                if (!locations.Contains(*loc)) {
                    return Failure("shader location " + std::to_string(*loc) +
                                   " missing from vertex pulling map");
                }
            }
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

}  // namespace tint::msl::writer
