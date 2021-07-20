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

#include "src/writer/hlsl/generator.h"

#include "src/transform/hlsl.h"
#include "src/writer/hlsl/generator_impl.h"

namespace tint {
namespace writer {
namespace hlsl {

Result::Result() = default;
Result::~Result() = default;
Result::Result(const Result&) = default;

Result Generate(const Program* program, const Options& options) {
  Result result;

  // Run the HLSL sanitizer.
  transform::Hlsl sanitizer;
  transform::DataMap transform_input;
  transform_input.Add<transform::Hlsl::Config>(options.disable_workgroup_init);
  auto output = sanitizer.Run(program, transform_input);
  if (!output.program.IsValid()) {
    result.success = false;
    result.error = output.program.Diagnostics().str();
    return result;
  }

  // Generate the HLSL code.
  auto impl = std::make_unique<GeneratorImpl>(&output.program);
  result.success = impl->Generate();
  result.error = impl->error();
  result.hlsl = impl->result();

  // Collect the list of entry points in the sanitized program.
  for (auto* func : output.program.AST().Functions()) {
    if (func->IsEntryPoint()) {
      auto name = output.program.Symbols().NameFor(func->symbol());
      result.entry_points.push_back({name, func->pipeline_stage()});
    }
  }

  return result;
}

}  // namespace hlsl
}  // namespace writer
}  // namespace tint
