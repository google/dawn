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

#include "src/writer/spirv/generator.h"

#include "src/transform/spirv.h"
#include "src/writer/spirv/binary_writer.h"

namespace tint {
namespace writer {
namespace spirv {

Result::Result() = default;
Result::~Result() = default;
Result::Result(const Result&) = default;

Result Generate(const Program* program, const Options& options) {
  Result result;

  // Run the SPIR-V sanitizer.
  transform::Spirv sanitizer;
  transform::DataMap transform_input;
  transform_input.Add<transform::Spirv::Config>(options.emit_vertex_point_size);
  auto output = sanitizer.Run(program, transform_input);
  if (!output.program.IsValid()) {
    result.success = false;
    result.error = output.program.Diagnostics().str();
    return result;
  }

  // Generate the SPIR-V code.
  auto builder = std::make_unique<Builder>(&output.program);
  auto writer = std::make_unique<BinaryWriter>();
  if (!builder->Build()) {
    result.success = false;
    result.error = builder->error();
    return result;
  }

  writer->WriteHeader(builder->id_bound());
  writer->WriteBuilder(builder.get());

  result.success = true;
  result.spirv = writer->result();

  return result;
}

Generator::Generator(const Program* program)
    : builder_(std::make_unique<Builder>(program)),
      writer_(std::make_unique<BinaryWriter>()) {}

Generator::~Generator() = default;

bool Generator::Generate() {
  if (!builder_->Build()) {
    set_error(builder_->error());
    return false;
  }

  writer_->WriteHeader(builder_->id_bound());
  writer_->WriteBuilder(builder_.get());
  return true;
}

const std::vector<uint32_t>& Generator::result() const {
  return writer_->result();
}

}  // namespace spirv
}  // namespace writer
}  // namespace tint
