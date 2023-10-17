// Copyright 2022 The Dawn & Tint Authors
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

#include "src/tint/lang/spirv/writer/ast_printer/ast_printer.h"

#include <unordered_map>
#include <utility>
#include <vector>

#include "src/tint/lang/spirv/writer/ast_raise/clamp_frag_depth.h"
#include "src/tint/lang/spirv/writer/ast_raise/for_loop_to_loop.h"
#include "src/tint/lang/spirv/writer/ast_raise/merge_return.h"
#include "src/tint/lang/spirv/writer/ast_raise/var_for_dynamic_index.h"
#include "src/tint/lang/spirv/writer/ast_raise/vectorize_matrix_conversions.h"
#include "src/tint/lang/spirv/writer/ast_raise/while_to_loop.h"
#include "src/tint/lang/spirv/writer/common/option_builder.h"
#include "src/tint/lang/wgsl/ast/transform/add_block_attribute.h"
#include "src/tint/lang/wgsl/ast/transform/add_empty_entry_point.h"
#include "src/tint/lang/wgsl/ast/transform/binding_remapper.h"
#include "src/tint/lang/wgsl/ast/transform/builtin_polyfill.h"
#include "src/tint/lang/wgsl/ast/transform/canonicalize_entry_point_io.h"
#include "src/tint/lang/wgsl/ast/transform/demote_to_helper.h"
#include "src/tint/lang/wgsl/ast/transform/direct_variable_access.h"
#include "src/tint/lang/wgsl/ast/transform/disable_uniformity_analysis.h"
#include "src/tint/lang/wgsl/ast/transform/expand_compound_assignment.h"
#include "src/tint/lang/wgsl/ast/transform/manager.h"
#include "src/tint/lang/wgsl/ast/transform/multiplanar_external_texture.h"
#include "src/tint/lang/wgsl/ast/transform/preserve_padding.h"
#include "src/tint/lang/wgsl/ast/transform/promote_side_effects_to_decl.h"
#include "src/tint/lang/wgsl/ast/transform/remove_phonies.h"
#include "src/tint/lang/wgsl/ast/transform/remove_unreachable_statements.h"
#include "src/tint/lang/wgsl/ast/transform/robustness.h"
#include "src/tint/lang/wgsl/ast/transform/simplify_pointers.h"
#include "src/tint/lang/wgsl/ast/transform/std140.h"
#include "src/tint/lang/wgsl/ast/transform/unshadow.h"
#include "src/tint/lang/wgsl/ast/transform/vectorize_scalar_matrix_initializers.h"
#include "src/tint/lang/wgsl/ast/transform/zero_init_workgroup_memory.h"

namespace tint::spirv::writer {

SanitizedResult Sanitize(const Program& in, const Options& options) {
    ast::transform::Manager manager;
    ast::transform::DataMap data;

    if (options.clamp_frag_depth) {
        manager.Add<ClampFragDepth>();
    }

    manager.Add<ast::transform::DisableUniformityAnalysis>();

    // ExpandCompoundAssignment must come before BuiltinPolyfill
    manager.Add<ast::transform::ExpandCompoundAssignment>();

    // Must come before DirectVariableAccess
    manager.Add<ast::transform::PreservePadding>();

    // Must come before DirectVariableAccess
    manager.Add<ast::transform::Unshadow>();

    manager.Add<ast::transform::RemoveUnreachableStatements>();
    manager.Add<ast::transform::PromoteSideEffectsToDecl>();

    // Required for arrayLength()
    manager.Add<ast::transform::SimplifyPointers>();

    manager.Add<ast::transform::RemovePhonies>();
    manager.Add<ast::transform::VectorizeScalarMatrixInitializers>();
    manager.Add<VectorizeMatrixConversions>();
    manager.Add<WhileToLoop>();  // ZeroInitWorkgroupMemory
    manager.Add<MergeReturn>();

    if (!options.disable_robustness) {
        // Robustness must come after PromoteSideEffectsToDecl
        // Robustness must come before BuiltinPolyfill and CanonicalizeEntryPointIO
        manager.Add<ast::transform::Robustness>();

        ast::transform::Robustness::Config config = {};
        if (options.disable_image_robustness) {
            config.texture_action = ast::transform::Robustness::Action::kIgnore;
        }
        config.disable_runtime_sized_array_index_clamping =
            options.disable_runtime_sized_array_index_clamping;
        data.Add<ast::transform::Robustness::Config>(config);
    }

    ExternalTextureOptions external_texture_options{};
    RemapperData remapper_data{};
    PopulateRemapperAndMultiplanarOptions(options, remapper_data, external_texture_options);

    // BindingRemapper must come before MultiplanarExternalTexture. Note, this is flipped to the
    // other generators which run Multiplanar first and then binding remapper.
    manager.Add<ast::transform::BindingRemapper>();

    data.Add<ast::transform::BindingRemapper::Remappings>(
        remapper_data, std::unordered_map<BindingPoint, core::Access>{},
        /* allow_collisions */ false);

    // Note: it is more efficient for MultiplanarExternalTexture to come after Robustness
    data.Add<ast::transform::MultiplanarExternalTexture::NewBindingPoints>(
        external_texture_options.bindings_map);
    manager.Add<ast::transform::MultiplanarExternalTexture>();

    {  // Builtin polyfills
        // BuiltinPolyfill must come before DirectVariableAccess, due to the use of pointer
        // parameter for workgroupUniformLoad()
        ast::transform::BuiltinPolyfill::Builtins polyfills;
        polyfills.acosh = ast::transform::BuiltinPolyfill::Level::kRangeCheck;
        polyfills.atanh = ast::transform::BuiltinPolyfill::Level::kRangeCheck;
        polyfills.bgra8unorm = true;
        polyfills.bitshift_modulo = true;
        polyfills.clamp_int = true;
        polyfills.conv_f32_to_iu32 = true;
        polyfills.count_leading_zeros = true;
        polyfills.count_trailing_zeros = true;
        polyfills.extract_bits = ast::transform::BuiltinPolyfill::Level::kClampParameters;
        polyfills.first_leading_bit = true;
        polyfills.first_trailing_bit = true;
        polyfills.insert_bits = ast::transform::BuiltinPolyfill::Level::kClampParameters;
        polyfills.int_div_mod = true;
        polyfills.saturate = true;
        polyfills.texture_sample_base_clamp_to_edge_2d_f32 = true;
        polyfills.quantize_to_vec_f16 = true;  // crbug.com/tint/1741
        polyfills.workgroup_uniform_load = true;
        data.Add<ast::transform::BuiltinPolyfill::Config>(polyfills);
        manager.Add<ast::transform::BuiltinPolyfill>();  // Must come before DirectVariableAccess
    }

    bool disable_workgroup_init_in_sanitizer =
        options.disable_workgroup_init || options.use_zero_initialize_workgroup_memory_extension;
    if (!disable_workgroup_init_in_sanitizer) {
        // ZeroInitWorkgroupMemory must come before CanonicalizeEntryPointIO as
        // ZeroInitWorkgroupMemory may inject new builtin parameters.
        manager.Add<ast::transform::ZeroInitWorkgroupMemory>();
    }

    {
        ast::transform::DirectVariableAccess::Options opts;
        opts.transform_private = true;
        opts.transform_function = true;
        data.Add<ast::transform::DirectVariableAccess::Config>(opts);
        manager.Add<ast::transform::DirectVariableAccess>();
    }

    // CanonicalizeEntryPointIO must come after Robustness
    manager.Add<ast::transform::CanonicalizeEntryPointIO>();
    manager.Add<ast::transform::AddEmptyEntryPoint>();

    // AddBlockAttribute must come after MultiplanarExternalTexture
    manager.Add<ast::transform::AddBlockAttribute>();

    // DemoteToHelper must come after CanonicalizeEntryPointIO, PromoteSideEffectsToDecl, and
    // ExpandCompoundAssignment.
    // TODO(crbug.com/tint/1752): Use SPV_EXT_demote_to_helper_invocation if available.
    manager.Add<ast::transform::DemoteToHelper>();

    // Std140 must come after PromoteSideEffectsToDecl.
    // Std140 must come before VarForDynamicIndex and ForLoopToLoop.
    manager.Add<ast::transform::Std140>();

    // VarForDynamicIndex must come after Std140
    manager.Add<VarForDynamicIndex>();

    // ForLoopToLoop must come after Std140, ZeroInitWorkgroupMemory
    manager.Add<ForLoopToLoop>();

    data.Add<ast::transform::CanonicalizeEntryPointIO::Config>(
        ast::transform::CanonicalizeEntryPointIO::Config(
            ast::transform::CanonicalizeEntryPointIO::ShaderStyle::kSpirv, 0xFFFFFFFF,
            options.emit_vertex_point_size));

    SanitizedResult result;
    ast::transform::DataMap outputs;
    result.program = manager.Run(in, data, outputs);
    return result;
}

ASTPrinter::ASTPrinter(const Program& program,
                       bool zero_initialize_workgroup_memory,
                       bool experimental_require_subgroup_uniform_control_flow)
    : builder_(program,
               zero_initialize_workgroup_memory,
               experimental_require_subgroup_uniform_control_flow) {}

bool ASTPrinter::Generate() {
    if (builder_.Build()) {
        auto& module = builder_.Module();
        writer_.WriteHeader(module.IdBound());
        writer_.WriteModule(module);
        return true;
    }
    return false;
}

}  // namespace tint::spirv::writer
