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

#include "src/tint/transform/add_empty_entry_point.h"
#include "src/tint/transform/add_spirv_block_attribute.h"
#include "src/tint/transform/builtin_polyfill.h"
#include "src/tint/transform/canonicalize_entry_point_io.h"
#include "src/tint/transform/expand_compound_assignment.h"
#include "src/tint/transform/fold_constants.h"
#include "src/tint/transform/for_loop_to_loop.h"
#include "src/tint/transform/manager.h"
#include "src/tint/transform/promote_side_effects_to_decl.h"
#include "src/tint/transform/remove_unreachable_statements.h"
#include "src/tint/transform/simplify_pointers.h"
#include "src/tint/transform/unshadow.h"
#include "src/tint/transform/unwind_discard_functions.h"
#include "src/tint/transform/var_for_dynamic_index.h"
#include "src/tint/transform/vectorize_scalar_matrix_constructors.h"
#include "src/tint/transform/zero_init_workgroup_memory.h"
#include "src/tint/writer/generate_external_texture_bindings.h"

namespace tint::writer::spirv {

SanitizedResult Sanitize(const Program* in, const Options& options) {
    transform::Manager manager;
    transform::DataMap data;

    {  // Builtin polyfills
        transform::BuiltinPolyfill::Builtins polyfills;
        polyfills.count_leading_zeros = true;
        polyfills.count_trailing_zeros = true;
        polyfills.extract_bits = transform::BuiltinPolyfill::Level::kClampParameters;
        polyfills.first_leading_bit = true;
        polyfills.first_trailing_bit = true;
        polyfills.insert_bits = transform::BuiltinPolyfill::Level::kClampParameters;
        data.Add<transform::BuiltinPolyfill::Config>(polyfills);
        manager.Add<transform::BuiltinPolyfill>();
    }

    if (options.generate_external_texture_bindings) {
        auto new_bindings_map = GenerateExternalTextureBindings(in);
        data.Add<transform::MultiplanarExternalTexture::NewBindingPoints>(new_bindings_map);
    }
    manager.Add<transform::MultiplanarExternalTexture>();

    manager.Add<transform::Unshadow>();
    bool disable_workgroup_init_in_sanitizer =
        options.disable_workgroup_init || options.use_zero_initialize_workgroup_memory_extension;
    if (!disable_workgroup_init_in_sanitizer) {
        manager.Add<transform::ZeroInitWorkgroupMemory>();
    }
    manager.Add<transform::RemoveUnreachableStatements>();
    manager.Add<transform::ExpandCompoundAssignment>();
    manager.Add<transform::PromoteSideEffectsToDecl>();
    manager.Add<transform::UnwindDiscardFunctions>();
    manager.Add<transform::SimplifyPointers>();  // Required for arrayLength()
    manager.Add<transform::FoldConstants>();
    manager.Add<transform::VectorizeScalarMatrixConstructors>();
    manager.Add<transform::ForLoopToLoop>();  // Must come after
                                              // ZeroInitWorkgroupMemory
    manager.Add<transform::CanonicalizeEntryPointIO>();
    manager.Add<transform::AddEmptyEntryPoint>();
    manager.Add<transform::AddSpirvBlockAttribute>();
    manager.Add<transform::VarForDynamicIndex>();

    data.Add<transform::CanonicalizeEntryPointIO::Config>(
        transform::CanonicalizeEntryPointIO::Config(
            transform::CanonicalizeEntryPointIO::ShaderStyle::kSpirv, 0xFFFFFFFF,
            options.emit_vertex_point_size));

    SanitizedResult result;
    result.program = std::move(manager.Run(in, data).program);
    return result;
}

GeneratorImpl::GeneratorImpl(const Program* program, bool zero_initialize_workgroup_memory)
    : builder_(program, zero_initialize_workgroup_memory) {}

bool GeneratorImpl::Generate() {
    if (builder_.Build()) {
        writer_.WriteHeader(builder_.id_bound());
        writer_.WriteBuilder(&builder_);
        return true;
    }
    return false;
}

const std::vector<uint32_t>& GeneratorImpl::result() const {
    return writer_.result();
}

std::vector<uint32_t>& GeneratorImpl::result() {
    return writer_.result();
}

std::string GeneratorImpl::error() const {
    return builder_.error();
}

}  // namespace tint::writer::spirv
