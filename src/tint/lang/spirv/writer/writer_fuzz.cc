// Copyright 2023 The Dawn & Tint Authors
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

#include "src/tint/api/helpers/generate_bindings.h"
#include "src/tint/cmd/fuzz/ir/fuzz.h"
#include "src/tint/lang/core/ir/disassembler.h"
#include "src/tint/lang/core/ir/transform/resource_binding_helper.h"
#include "src/tint/lang/spirv/validate/validate.h"
#include "src/tint/lang/spirv/writer/printer/printer.h"

namespace tint::spirv::writer {

// Fuzzed options used to init tint::spirv::writer::Options
struct FuzzedOptions {
    ///////////////////////////////////////////////////////////////////////////////////////////////
    // NOTE: These options should not be reordered or removed as it will change the operation of //
    // pre-existing fuzzer cases. Always append new options to the end of the list.              //
    ///////////////////////////////////////////////////////////////////////////////////////////////
    bool strip_all_names;
    bool disable_robustness;
    bool disable_workgroup_init;
    bool disable_polyfill_integer_div_mod;
    bool enable_integer_range_analysis;
    bool emit_vertex_point_size;
    bool polyfill_pixel_center;
    bool polyfill_case_switch;
    bool scalarize_max_min_clamp;
    bool dva_transform_handle;
    bool polyfill_pack_unpack_4x8_norm;
    bool subgroup_shuffle_clamped;
    bool polyfill_subgroup_broadcast_f16;
    bool pass_matrix_by_pointer;
    bool polyfill_unary_f32_negation;
    bool polyfill_f32_abs;
    bool use_demote_to_helper_invocation;
    bool use_storage_input_output_16;
    bool use_zero_initialize_workgroup_memory;
    bool use_vulkan_memory_model;
    bool disable_image_robustness;
    bool disable_runtime_sized_array_index_clamping;
    bool dot_4x8_packed;
    std::optional<Options::RangeOffsets> depth_range_offsets;
    SpvVersion spirv_version;
    SubstituteOverridesConfig substitute_overrides_config;
    bool texture_sample_compare_depth_cube_array;

    /// Reflect the fields of this class so that it can be used by tint::ForeachField()
    TINT_REFLECT(FuzzedOptions,
                 strip_all_names,
                 disable_robustness,
                 disable_workgroup_init,
                 disable_polyfill_integer_div_mod,
                 enable_integer_range_analysis,
                 emit_vertex_point_size,
                 polyfill_pixel_center,
                 polyfill_case_switch,
                 scalarize_max_min_clamp,
                 dva_transform_handle,
                 polyfill_pack_unpack_4x8_norm,
                 subgroup_shuffle_clamped,
                 polyfill_subgroup_broadcast_f16,
                 pass_matrix_by_pointer,
                 polyfill_unary_f32_negation,
                 polyfill_f32_abs,
                 use_demote_to_helper_invocation,
                 use_storage_input_output_16,
                 use_zero_initialize_workgroup_memory,
                 use_vulkan_memory_model,
                 disable_image_robustness,
                 disable_runtime_sized_array_index_clamping,
                 dot_4x8_packed,
                 depth_range_offsets,
                 spirv_version,
                 substitute_overrides_config,
                 texture_sample_compare_depth_cube_array);
    TINT_REFLECT_HASH_CODE(FuzzedOptions);
};

namespace {

Result<SuccessType> IRFuzzer(core::ir::Module& module,
                             const fuzz::ir::Context&,
                             FuzzedOptions fuzzed_options) {
    // TODO(375388101): We cannot run the backend for every entry point in the module unless we
    // clone the whole module each time, so for now we just generate the first entry point.

    // Strip the module down to a single entry point.
    core::ir::Function* entry_point = nullptr;
    for (auto& func : module.functions) {
        if (func->IsEntryPoint()) {
            entry_point = func;
            break;
        }
    }
    std::string ep_name;
    if (entry_point) {
        ep_name = module.NameOf(entry_point).NameView();
    }
    if (ep_name.empty()) {
        // No entry point, just return success
        return Success;
    }

    // We fuzz options that Dawn will vary depending on the platform and provided toggles.
    // Options that are entirely controlled by Dawn (e.g. binding points) are not fuzzed.
    Options options;
    options.entry_point_name = ep_name;
    options.bindings = GenerateBindings(module, ep_name, false, false);
    options.strip_all_names = fuzzed_options.strip_all_names;
    options.disable_robustness = fuzzed_options.disable_robustness;
    options.disable_workgroup_init = fuzzed_options.disable_workgroup_init;
    options.disable_polyfill_integer_div_mod = fuzzed_options.disable_polyfill_integer_div_mod;
    options.enable_integer_range_analysis = fuzzed_options.enable_integer_range_analysis;
    options.emit_vertex_point_size = fuzzed_options.emit_vertex_point_size;
    options.polyfill_pixel_center = fuzzed_options.polyfill_pixel_center;
    options.workarounds.polyfill_case_switch = fuzzed_options.polyfill_case_switch;
    options.workarounds.scalarize_max_min_clamp = fuzzed_options.scalarize_max_min_clamp;
    options.workarounds.dva_transform_handle = fuzzed_options.dva_transform_handle;
    options.workarounds.polyfill_pack_unpack_4x8_norm =
        fuzzed_options.polyfill_pack_unpack_4x8_norm;
    options.workarounds.subgroup_shuffle_clamped = fuzzed_options.subgroup_shuffle_clamped;
    options.workarounds.polyfill_subgroup_broadcast_f16 =
        fuzzed_options.polyfill_subgroup_broadcast_f16;
    options.workarounds.pass_matrix_by_pointer = fuzzed_options.pass_matrix_by_pointer;
    options.workarounds.polyfill_unary_f32_negation = fuzzed_options.polyfill_unary_f32_negation;
    options.workarounds.polyfill_f32_abs = fuzzed_options.polyfill_f32_abs;
    options.extensions.use_demote_to_helper_invocation =
        fuzzed_options.use_demote_to_helper_invocation;
    options.extensions.use_storage_input_output_16 = fuzzed_options.use_storage_input_output_16;
    options.extensions.use_zero_initialize_workgroup_memory =
        fuzzed_options.use_zero_initialize_workgroup_memory;
    options.extensions.use_vulkan_memory_model = fuzzed_options.use_vulkan_memory_model;
    options.extensions.disable_image_robustness = fuzzed_options.disable_image_robustness;
    options.extensions.disable_runtime_sized_array_index_clamping =
        fuzzed_options.disable_runtime_sized_array_index_clamping;
    options.extensions.dot_4x8_packed = fuzzed_options.dot_4x8_packed;
    options.depth_range_offsets = fuzzed_options.depth_range_offsets;
    options.spirv_version = fuzzed_options.spirv_version;
    options.substitute_overrides_config = fuzzed_options.substitute_overrides_config;
    options.workarounds.texture_sample_compare_depth_cube_array =
        fuzzed_options.texture_sample_compare_depth_cube_array;

    options.resource_binding = tint::core::ir::transform::GenerateResourceBindingConfig(module);

    auto check = CanGenerate(module, options);
    if (check != Success) {
        return Failure{check.Failure().reason};
    }

    auto output = Generate(module, options);
    if (output != Success) {
        return output.Failure();
    }

    spv_target_env target_env = SPV_ENV_VULKAN_1_1;
    switch (options.spirv_version) {
        case SpvVersion::kSpv13:
            target_env = SPV_ENV_VULKAN_1_1;
            break;
        case SpvVersion::kSpv14:
            target_env = SPV_ENV_VULKAN_1_1_SPIRV_1_4;
            break;
        case SpvVersion::kSpv15:
            target_env = SPV_ENV_VULKAN_1_2;
            break;
        default:
            TINT_ICE() << "unsupported SPIR-V version";
    }

    auto& spirv = output->spirv;
    auto res = validate::Validate(Slice(spirv.data(), spirv.size()), target_env);
    TINT_ASSERT(res == Success) << "output of SPIR-V writer failed to validate with SPIR-V Tools\n"
                                << res.Failure() << "\n\n"
                                << "IR:\n"
                                << core::ir::Disassembler(module).Plain();

    return Success;
}

}  // namespace
}  // namespace tint::spirv::writer

TINT_IR_MODULE_FUZZER(tint::spirv::writer::IRFuzzer,
                      tint::core::ir::Capabilities{},
                      tint::spirv::writer::kPrinterCapabilities);
