// Copyright 2021 The Tint Authors.
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

#include "src/transform/glsl.h"

#include <utility>

#include "src/program_builder.h"
#include "src/transform/add_empty_entry_point.h"
#include "src/transform/add_spirv_block_attribute.h"
#include "src/transform/binding_remapper.h"
#include "src/transform/canonicalize_entry_point_io.h"
#include "src/transform/combine_samplers.h"
#include "src/transform/decompose_memory_access.h"
#include "src/transform/external_texture_transform.h"
#include "src/transform/fold_trivial_single_use_lets.h"
#include "src/transform/loop_to_for_loop.h"
#include "src/transform/manager.h"
#include "src/transform/pad_array_elements.h"
#include "src/transform/promote_initializers_to_const_var.h"
#include "src/transform/remove_phonies.h"
#include "src/transform/renamer.h"
#include "src/transform/simplify_pointers.h"
#include "src/transform/single_entry_point.h"
#include "src/transform/unshadow.h"
#include "src/transform/zero_init_workgroup_memory.h"

TINT_INSTANTIATE_TYPEINFO(tint::transform::Glsl);
TINT_INSTANTIATE_TYPEINFO(tint::transform::Glsl::Config);

namespace tint {
namespace transform {

Glsl::Glsl() = default;
Glsl::~Glsl() = default;

Output Glsl::Run(const Program* in, const DataMap& inputs) const {
  Manager manager;
  DataMap data;

  auto* cfg = inputs.Get<Config>();

  if (cfg && !cfg->entry_point.empty()) {
    manager.Add<SingleEntryPoint>();
    data.Add<SingleEntryPoint::Config>(cfg->entry_point);
  }
  manager.Add<Renamer>();
  data.Add<Renamer::Config>(Renamer::Target::kGlslKeywords);
  manager.Add<Unshadow>();

  // Attempt to convert `loop`s into for-loops. This is to try and massage the
  // output into something that will not cause FXC to choke or misbehave.
  manager.Add<FoldTrivialSingleUseLets>();
  manager.Add<LoopToForLoop>();

  if (!cfg || !cfg->disable_workgroup_init) {
    // ZeroInitWorkgroupMemory must come before CanonicalizeEntryPointIO as
    // ZeroInitWorkgroupMemory may inject new builtin parameters.
    manager.Add<ZeroInitWorkgroupMemory>();
  }
  manager.Add<CanonicalizeEntryPointIO>();
  manager.Add<SimplifyPointers>();

  manager.Add<RemovePhonies>();
  manager.Add<CombineSamplers>();
  if (auto* binding_info = inputs.Get<CombineSamplers::BindingInfo>()) {
    data.Add<CombineSamplers::BindingInfo>(*binding_info);
  } else {
    data.Add<CombineSamplers::BindingInfo>(CombineSamplers::BindingMap(),
                                           sem::BindingPoint());
  }
  manager.Add<BindingRemapper>();
  if (auto* remappings = inputs.Get<BindingRemapper::Remappings>()) {
    data.Add<BindingRemapper::Remappings>(*remappings);
  } else {
    BindingRemapper::BindingPoints bp;
    BindingRemapper::AccessControls ac;
    data.Add<BindingRemapper::Remappings>(bp, ac, /* mayCollide */ true);
  }
  manager.Add<ExternalTextureTransform>();
  manager.Add<PromoteInitializersToConstVar>();

  manager.Add<PadArrayElements>();
  manager.Add<AddEmptyEntryPoint>();
  manager.Add<AddSpirvBlockAttribute>();

  data.Add<CanonicalizeEntryPointIO::Config>(
      CanonicalizeEntryPointIO::ShaderStyle::kGlsl);
  auto out = manager.Run(in, data);
  if (!out.program.IsValid()) {
    return out;
  }

  ProgramBuilder builder;
  CloneContext ctx(&builder, &out.program);
  ctx.Clone();
  return Output{Program(std::move(builder))};
}

Glsl::Config::Config(const std::string& ep, bool disable_wi)
    : entry_point(ep), disable_workgroup_init(disable_wi) {}
Glsl::Config::Config(const Config&) = default;
Glsl::Config::~Config() = default;

}  // namespace transform
}  // namespace tint
