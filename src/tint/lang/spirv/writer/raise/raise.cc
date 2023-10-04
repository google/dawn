// Copyright 2023 The Tint Authors.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "src/tint/lang/spirv/writer/raise/raise.h"

#include <utility>

#include "src/tint/lang/core/ir/transform/add_empty_entry_point.h"
#include "src/tint/lang/core/ir/transform/bgra8unorm_polyfill.h"
#include "src/tint/lang/core/ir/transform/binary_polyfill.h"
#include "src/tint/lang/core/ir/transform/binding_remapper.h"
#include "src/tint/lang/core/ir/transform/block_decorated_structs.h"
#include "src/tint/lang/core/ir/transform/builtin_polyfill.h"
#include "src/tint/lang/core/ir/transform/combine_access_instructions.h"
#include "src/tint/lang/core/ir/transform/conversion_polyfill.h"
#include "src/tint/lang/core/ir/transform/demote_to_helper.h"
#include "src/tint/lang/core/ir/transform/direct_variable_access.h"
#include "src/tint/lang/core/ir/transform/multiplanar_external_texture.h"
#include "src/tint/lang/core/ir/transform/preserve_padding.h"
#include "src/tint/lang/core/ir/transform/robustness.h"
#include "src/tint/lang/core/ir/transform/std140.h"
#include "src/tint/lang/core/ir/transform/zero_init_workgroup_memory.h"
#include "src/tint/lang/spirv/writer/common/option_builder.h"
#include "src/tint/lang/spirv/writer/raise/builtin_polyfill.h"
#include "src/tint/lang/spirv/writer/raise/expand_implicit_splats.h"
#include "src/tint/lang/spirv/writer/raise/handle_matrix_arithmetic.h"
#include "src/tint/lang/spirv/writer/raise/merge_return.h"
#include "src/tint/lang/spirv/writer/raise/pass_matrix_by_pointer.h"
#include "src/tint/lang/spirv/writer/raise/shader_io.h"
#include "src/tint/lang/spirv/writer/raise/var_for_dynamic_index.h"

namespace tint::spirv::writer::raise {

Result<SuccessType> Raise(core::ir::Module& module, const Options& options) {
#define RUN_TRANSFORM(name, ...)         \
    do {                                 \
        auto result = name(__VA_ARGS__); \
        if (!result) {                   \
            return result;               \
        }                                \
    } while (false)

    ExternalTextureOptions external_texture_options{};
    RemapperData remapper_data{};
    PopulateRemapperAndMultiplanarOptions(options, remapper_data, external_texture_options);

    RUN_TRANSFORM(core::ir::transform::BindingRemapper, module, remapper_data);

    core::ir::transform::BinaryPolyfillConfig binary_polyfills;
    binary_polyfills.bitshift_modulo = true;
    binary_polyfills.int_div_mod = true;
    RUN_TRANSFORM(core::ir::transform::BinaryPolyfill, module, binary_polyfills);

    core::ir::transform::BuiltinPolyfillConfig core_polyfills;
    core_polyfills.clamp_int = true;
    core_polyfills.count_leading_zeros = true;
    core_polyfills.count_trailing_zeros = true;
    core_polyfills.extract_bits = core::ir::transform::BuiltinPolyfillLevel::kClampOrRangeCheck;
    core_polyfills.first_leading_bit = true;
    core_polyfills.first_trailing_bit = true;
    core_polyfills.insert_bits = core::ir::transform::BuiltinPolyfillLevel::kClampOrRangeCheck;
    core_polyfills.saturate = true;
    core_polyfills.texture_sample_base_clamp_to_edge_2d_f32 = true;
    RUN_TRANSFORM(core::ir::transform::BuiltinPolyfill, module, core_polyfills);

    core::ir::transform::ConversionPolyfillConfig conversion_polyfills;
    conversion_polyfills.ftoi = true;
    RUN_TRANSFORM(core::ir::transform::ConversionPolyfill, module, conversion_polyfills);

    if (!options.disable_robustness) {
        core::ir::transform::RobustnessConfig config;
        if (options.disable_image_robustness) {
            config.clamp_texture = false;
        }
        config.disable_runtime_sized_array_index_clamping =
            options.disable_runtime_sized_array_index_clamping;
        RUN_TRANSFORM(core::ir::transform::Robustness, module, config);
    }

    RUN_TRANSFORM(core::ir::transform::MultiplanarExternalTexture, module,
                  external_texture_options);

    if (!options.disable_workgroup_init &&
        !options.use_zero_initialize_workgroup_memory_extension) {
        RUN_TRANSFORM(core::ir::transform::ZeroInitWorkgroupMemory, module);
    }

    // PreservePadding must come before DirectVariableAccess.
    RUN_TRANSFORM(core::ir::transform::PreservePadding, module);

    core::ir::transform::DirectVariableAccessOptions dva_options;
    dva_options.transform_function = true;
    dva_options.transform_private = true;
    RUN_TRANSFORM(core::ir::transform::DirectVariableAccess, module, dva_options);

    if (options.pass_matrix_by_pointer) {
        // PassMatrixByPointer must come after PreservePadding+DirectVariableAccess.
        RUN_TRANSFORM(PassMatrixByPointer, module);
    }

    RUN_TRANSFORM(core::ir::transform::AddEmptyEntryPoint, module);
    RUN_TRANSFORM(core::ir::transform::Bgra8UnormPolyfill, module);
    RUN_TRANSFORM(core::ir::transform::BlockDecoratedStructs, module);

    // CombineAccessInstructions must come after DirectVariableAccess and BlockDecoratedStructs.
    // We run this transform as some Qualcomm drivers struggle with partial access chains that
    // produce pointers to matrices.
    RUN_TRANSFORM(core::ir::transform::CombineAccessInstructions, module);

    RUN_TRANSFORM(BuiltinPolyfill, module);
    RUN_TRANSFORM(core::ir::transform::DemoteToHelper, module);
    RUN_TRANSFORM(ExpandImplicitSplats, module);
    RUN_TRANSFORM(HandleMatrixArithmetic, module);
    RUN_TRANSFORM(MergeReturn, module);
    RUN_TRANSFORM(ShaderIO, module,
                  ShaderIOConfig{options.clamp_frag_depth, options.emit_vertex_point_size});
    RUN_TRANSFORM(core::ir::transform::Std140, module);
    RUN_TRANSFORM(VarForDynamicIndex, module);

    return Success;
}

}  // namespace tint::spirv::writer::raise
