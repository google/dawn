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

#include <utility>

namespace tint {
namespace writer {
namespace spirv {

Generator::Generator(ast::Module module)
    : writer::Writer(std::move(module)),
      namer_(std::make_unique<MangleNamer>(module_)),
      builder_(std::make_unique<Builder>(module_, namer_.get())),
      writer_(std::make_unique<BinaryWriter>()) {}

Generator::Generator(ast::Module* module)
    : writer::Writer(module),
      namer_(std::make_unique<MangleNamer>(module_)),
      builder_(std::make_unique<Builder>(module_, namer_.get())),
      writer_(std::make_unique<BinaryWriter>()) {}

Generator::~Generator() = default;

void Generator::Reset() {
  namer_->Reset();
  builder_ = std::make_unique<Builder>(module_, namer_.get());
  writer_ = std::make_unique<BinaryWriter>();
}

bool Generator::Generate() {
  if (!builder_->Build()) {
    set_error(builder_->error());
    return false;
  }

  writer_->WriteHeader(builder_->id_bound());
  writer_->WriteBuilder(builder_.get());
  return true;
}

bool Generator::GenerateEntryPoint(ast::PipelineStage, const std::string&) {
  return false;
}

}  // namespace spirv
}  // namespace writer
}  // namespace tint
