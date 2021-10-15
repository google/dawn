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

#include "src/writer/glsl/generator.h"

#include "src/transform/glsl.h"
#include "src/writer/glsl/generator_impl.h"

namespace tint {
namespace writer {
namespace glsl {

Result::Result() = default;
Result::~Result() = default;
Result::Result(const Result&) = default;

Result Generate(const Program* program,
                const Options&,
                const std::string& entry_point) {
  Result result;

  // Run the GLSL sanitizer.
  transform::DataMap data;
  data.Add<transform::Glsl::Config>(entry_point);
  transform::Glsl sanitizer;
  auto output = sanitizer.Run(program, data);
  if (!output.program.IsValid()) {
    result.success = false;
    result.error = output.program.Diagnostics().str();
    return result;
  }

  // Generate the GLSL code.
  auto impl = std::make_unique<GeneratorImpl>(&output.program);
  result.success = impl->Generate();
  result.error = impl->error();
  result.glsl = impl->result();

  // Collect the list of entry points in the sanitized program.
  for (auto* func : output.program.AST().Functions()) {
    if (func->IsEntryPoint()) {
      auto name = output.program.Symbols().NameFor(func->symbol);
      result.entry_points.push_back({name, func->PipelineStage()});
    }
  }

  return result;
}

}  // namespace glsl
}  // namespace writer
}  // namespace tint
