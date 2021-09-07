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

#include "src/transform/spirv.h"

#include <string>
#include <utility>

#include "src/ast/stage_decoration.h"
#include "src/program_builder.h"
#include "src/sem/variable.h"
#include "src/transform/add_empty_entry_point.h"
#include "src/transform/canonicalize_entry_point_io.h"
#include "src/transform/external_texture_transform.h"
#include "src/transform/fold_constants.h"
#include "src/transform/for_loop_to_loop.h"
#include "src/transform/inline_pointer_lets.h"
#include "src/transform/manager.h"
#include "src/transform/simplify.h"
#include "src/transform/zero_init_workgroup_memory.h"

TINT_INSTANTIATE_TYPEINFO(tint::transform::Spirv);
TINT_INSTANTIATE_TYPEINFO(tint::transform::Spirv::Config);

namespace tint {
namespace transform {

Spirv::Spirv() = default;
Spirv::~Spirv() = default;

Output Spirv::Run(const Program* in, const DataMap& data) {
  auto* cfg = data.Get<Config>();

  Manager manager;
  DataMap internal_inputs;
  if (!cfg || !cfg->disable_workgroup_init) {
    manager.Add<ZeroInitWorkgroupMemory>();
  }
  manager.Add<InlinePointerLets>();  // Required for arrayLength()
  manager.Add<Simplify>();           // Required for arrayLength()
  manager.Add<FoldConstants>();
  manager.Add<ExternalTextureTransform>();
  manager.Add<ForLoopToLoop>();  // Must come after ZeroInitWorkgroupMemory
  manager.Add<CanonicalizeEntryPointIO>();
  manager.Add<AddEmptyEntryPoint>();

  internal_inputs.Add<CanonicalizeEntryPointIO::Config>(
      CanonicalizeEntryPointIO::Config(
          CanonicalizeEntryPointIO::ShaderStyle::kSpirv, 0xFFFFFFFF,
          (cfg && cfg->emit_vertex_point_size)));

  auto transformedInput = manager.Run(in, internal_inputs);

  if (transformedInput.program.Diagnostics().contains_errors()) {
    return transformedInput;
  }

  ProgramBuilder builder;
  CloneContext ctx(&builder, &transformedInput.program);
  // TODO(jrprice): Move the sanitizer into the backend.
  ctx.Clone();

  builder.SetTransformApplied(this);
  return Output{Program(std::move(builder))};
}

Spirv::Config::Config(bool emit_vps, bool disable_wi)
    : emit_vertex_point_size(emit_vps), disable_workgroup_init(disable_wi) {}

Spirv::Config::Config(const Config&) = default;
Spirv::Config::~Config() = default;
Spirv::Config& Spirv::Config::operator=(const Config&) = default;

}  // namespace transform
}  // namespace tint
