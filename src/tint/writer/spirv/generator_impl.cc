// Copyright 2022 The Tint Authors.
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

#include "src/tint/writer/spirv/generator_impl.h"

#include <utility>
#include <vector>

#include "src/tint/ast/transform/add_block_attribute.h"
#include "src/tint/ast/transform/add_empty_entry_point.h"
#include "src/tint/ast/transform/binding_remapper.h"
#include "src/tint/ast/transform/builtin_polyfill.h"
#include "src/tint/ast/transform/canonicalize_entry_point_io.h"
#include "src/tint/ast/transform/clamp_frag_depth.h"
#include "src/tint/ast/transform/demote_to_helper.h"
#include "src/tint/ast/transform/direct_variable_access.h"
#include "src/tint/ast/transform/disable_uniformity_analysis.h"
#include "src/tint/ast/transform/expand_compound_assignment.h"
#include "src/tint/ast/transform/for_loop_to_loop.h"
#include "src/tint/ast/transform/merge_return.h"
#include "src/tint/ast/transform/multiplanar_external_texture.h"
#include "src/tint/ast/transform/preserve_padding.h"
#include "src/tint/ast/transform/promote_side_effects_to_decl.h"
#include "src/tint/ast/transform/remove_phonies.h"
#include "src/tint/ast/transform/remove_unreachable_statements.h"
#include "src/tint/ast/transform/robustness.h"
#include "src/tint/ast/transform/simplify_pointers.h"
#include "src/tint/ast/transform/std140.h"
#include "src/tint/ast/transform/unshadow.h"
#include "src/tint/ast/transform/var_for_dynamic_index.h"
#include "src/tint/ast/transform/vectorize_matrix_conversions.h"
#include "src/tint/ast/transform/vectorize_scalar_matrix_initializers.h"
#include "src/tint/ast/transform/while_to_loop.h"
#include "src/tint/ast/transform/zero_init_workgroup_memory.h"
#include "src/tint/transform/manager.h"

namespace tint::writer::spirv {

SanitizedResult Sanitize(const Program* in, const Options& options) {
    transform::Manager manager;
    transform::DataMap data;

    if (options.clamp_frag_depth) {
        manager.Add<tint::ast::transform::ClampFragDepth>();
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
    manager.Add<ast::transform::VectorizeMatrixConversions>();
    manager.Add<ast::transform::WhileToLoop>();  // ZeroInitWorkgroupMemory
    manager.Add<ast::transform::MergeReturn>();

    if (!options.disable_robustness) {
        // Robustness must come after PromoteSideEffectsToDecl
        // Robustness must come before BuiltinPolyfill and CanonicalizeEntryPointIO
        manager.Add<ast::transform::Robustness>();
    }

    // BindingRemapper must come before MultiplanarExternalTexture. Note, this is flipped to the
    // other generators which run Multiplanar first and then binding remapper.
    manager.Add<ast::transform::BindingRemapper>();
    data.Add<ast::transform::BindingRemapper::Remappings>(
        options.binding_remapper_options.binding_points,
        options.binding_remapper_options.access_controls,
        options.binding_remapper_options.allow_collisions);

    // Note: it is more efficient for MultiplanarExternalTexture to come after Robustness
    data.Add<ast::transform::MultiplanarExternalTexture::NewBindingPoints>(
        options.external_texture_options.bindings_map);
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
    manager.Add<ast::transform::VarForDynamicIndex>();

    // ForLoopToLoop must come after Std140, ZeroInitWorkgroupMemory
    manager.Add<ast::transform::ForLoopToLoop>();

    data.Add<ast::transform::CanonicalizeEntryPointIO::Config>(
        ast::transform::CanonicalizeEntryPointIO::Config(
            ast::transform::CanonicalizeEntryPointIO::ShaderStyle::kSpirv, 0xFFFFFFFF,
            options.emit_vertex_point_size));

    SanitizedResult result;
    transform::DataMap outputs;
    result.program = manager.Run(in, data, outputs);
    return result;
}

GeneratorImpl::GeneratorImpl(const Program* program, bool zero_initialize_workgroup_memory)
    : builder_(program, zero_initialize_workgroup_memory) {}

bool GeneratorImpl::Generate() {
    if (builder_.Build()) {
        auto& module = builder_.Module();
        writer_.WriteHeader(module.IdBound());
        writer_.WriteModule(&module);
        return true;
    }
    return false;
}

}  // namespace tint::writer::spirv
