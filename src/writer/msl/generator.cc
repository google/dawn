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

#include "src/writer/msl/generator.h"

#include "src/transform/msl.h"
#include "src/writer/msl/generator_impl.h"

namespace tint {
namespace writer {
namespace msl {

Result::Result() = default;
Result::~Result() = default;
Result::Result(const Result&) = default;

Result Generate(const Program* program, const Options& options) {
  Result result;

  // Run the MSL sanitizer.
  transform::Msl sanitizer;
  transform::DataMap transform_input;
  transform_input.Add<transform::Msl::Config>(
      options.buffer_size_ubo_index, options.fixed_sample_mask,
      options.emit_vertex_point_size, options.disable_workgroup_init);
  auto output = sanitizer.Run(program, transform_input);
  if (!output.program.IsValid()) {
    result.success = false;
    result.error = output.program.Diagnostics().str();
    return result;
  }
  auto* transform_output = output.data.Get<transform::Msl::Result>();
  result.needs_storage_buffer_sizes =
      transform_output->needs_storage_buffer_sizes;

  // Generate the MSL code.
  auto impl = std::make_unique<GeneratorImpl>(&output.program);
  result.success = impl->Generate();
  result.error = impl->error();
  result.msl = impl->result();
  result.has_invariant_attribute = impl->HasInvariant();

  return result;
}

}  // namespace msl
}  // namespace writer
}  // namespace tint
