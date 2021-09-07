// Copyright 2020 The Tint Authors.
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

#include "src/transform/msl.h"

#include <memory>
#include <unordered_map>
#include <utility>
#include <vector>

#include "src/ast/disable_validation_decoration.h"
#include "src/program_builder.h"
#include "src/sem/call.h"
#include "src/sem/function.h"
#include "src/sem/statement.h"
#include "src/sem/variable.h"
#include "src/transform/array_length_from_uniform.h"
#include "src/transform/canonicalize_entry_point_io.h"
#include "src/transform/external_texture_transform.h"
#include "src/transform/inline_pointer_lets.h"
#include "src/transform/manager.h"
#include "src/transform/module_scope_var_to_entry_point_param.h"
#include "src/transform/pad_array_elements.h"
#include "src/transform/promote_initializers_to_const_var.h"
#include "src/transform/simplify.h"
#include "src/transform/wrap_arrays_in_structs.h"
#include "src/transform/zero_init_workgroup_memory.h"

TINT_INSTANTIATE_TYPEINFO(tint::transform::Msl);
TINT_INSTANTIATE_TYPEINFO(tint::transform::Msl::Config);
TINT_INSTANTIATE_TYPEINFO(tint::transform::Msl::Result);

namespace tint {
namespace transform {

Msl::Msl() = default;
Msl::~Msl() = default;

Output Msl::Run(const Program* in, const DataMap& inputs) {
  Manager manager;
  DataMap internal_inputs;

  auto* cfg = inputs.Get<Config>();

  // Build the configs for the internal transforms.
  uint32_t buffer_size_ubo_index = kDefaultBufferSizeUniformIndex;
  uint32_t fixed_sample_mask = 0xFFFFFFFF;
  bool emit_point_size = false;
  if (cfg) {
    buffer_size_ubo_index = cfg->buffer_size_ubo_index;
    fixed_sample_mask = cfg->fixed_sample_mask;
    emit_point_size = cfg->emit_vertex_point_size;
  }
  auto array_length_from_uniform_cfg = ArrayLengthFromUniform::Config(
      sem::BindingPoint{0, buffer_size_ubo_index});
  auto entry_point_io_cfg = CanonicalizeEntryPointIO::Config(
      CanonicalizeEntryPointIO::ShaderStyle::kMsl, fixed_sample_mask,
      emit_point_size);

  // Use the SSBO binding numbers as the indices for the buffer size lookups.
  for (auto* var : in->AST().GlobalVariables()) {
    auto* global = in->Sem().Get<sem::GlobalVariable>(var);
    if (global && global->StorageClass() == ast::StorageClass::kStorage) {
      array_length_from_uniform_cfg.bindpoint_to_size_index.emplace(
          global->BindingPoint(), global->BindingPoint().binding);
    }
  }

  if (!cfg || !cfg->disable_workgroup_init) {
    // ZeroInitWorkgroupMemory must come before CanonicalizeEntryPointIO as
    // ZeroInitWorkgroupMemory may inject new builtin parameters.
    manager.Add<ZeroInitWorkgroupMemory>();
  }
  manager.Add<CanonicalizeEntryPointIO>();
  manager.Add<ExternalTextureTransform>();
  manager.Add<PromoteInitializersToConstVar>();
  manager.Add<WrapArraysInStructs>();
  manager.Add<PadArrayElements>();
  manager.Add<ModuleScopeVarToEntryPointParam>();
  manager.Add<InlinePointerLets>();
  manager.Add<Simplify>();
  // ArrayLengthFromUniform must come after InlinePointerLets and Simplify, as
  // it assumes that the form of the array length argument is &var.array.
  manager.Add<ArrayLengthFromUniform>();
  internal_inputs.Add<ArrayLengthFromUniform::Config>(
      std::move(array_length_from_uniform_cfg));
  internal_inputs.Add<CanonicalizeEntryPointIO::Config>(
      std::move(entry_point_io_cfg));
  auto out = manager.Run(in, internal_inputs);
  if (!out.program.IsValid()) {
    return out;
  }

  ProgramBuilder builder;
  CloneContext ctx(&builder, &out.program);
  // TODO(jrprice): Move the sanitizer into the backend.
  ctx.Clone();

  auto result = std::make_unique<Result>(
      out.data.Get<ArrayLengthFromUniform::Result>()->needs_buffer_sizes);

  builder.SetTransformApplied(this);
  return Output{Program(std::move(builder)), std::move(result)};
}

Msl::Config::Config(uint32_t buffer_size_ubo_idx,
                    uint32_t sample_mask,
                    bool emit_point_size,
                    bool disable_wi)
    : buffer_size_ubo_index(buffer_size_ubo_idx),
      fixed_sample_mask(sample_mask),
      emit_vertex_point_size(emit_point_size),
      disable_workgroup_init(disable_wi) {}
Msl::Config::Config(const Config&) = default;
Msl::Config::~Config() = default;

Msl::Result::Result(bool needs_buffer_sizes)
    : needs_storage_buffer_sizes(needs_buffer_sizes) {}
Msl::Result::Result(const Result&) = default;
Msl::Result::~Result() = default;

}  // namespace transform
}  // namespace tint
