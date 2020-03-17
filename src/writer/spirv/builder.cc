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

#include "src/writer/spirv/builder.h"

#include "spirv/unified1/spirv.h"

namespace tint {
namespace writer {
namespace spirv {
namespace {

uint32_t size_of(const std::vector<Instruction>& instructions) {
  uint32_t size = 0;
  for (const auto& inst : instructions)
    size += inst.word_length();

  return size;
}

}  // namespace

Builder::Builder() = default;

Builder::~Builder() = default;

bool Builder::Build(const ast::Module&) {
  push_preamble(spv::Op::OpCapability, {Operand::Int(SpvCapabilityShader)});
  push_preamble(spv::Op::OpExtInstImport,
                {result_op(), Operand::String("GLSL.std.450")});
  push_preamble(spv::Op::OpMemoryModel,
                {Operand::Int(SpvAddressingModelLogical),
                 Operand::Int(SpvMemoryModelVulkanKHR)});
  return true;
}

Operand Builder::result_op() {
  return Operand::Int(next_id());
}

uint32_t Builder::total_size() const {
  // The 5 covers the magic, version, generator, id bound and reserved.
  uint32_t size = 5;

  size += size_of(preamble_);
  size += size_of(debug_);
  size += size_of(annotations_);
  size += size_of(types_);
  size += size_of(instructions_);

  return size;
}

void Builder::iterate(std::function<void(const Instruction&)> cb) const {
  for (const auto& inst : preamble_) {
    cb(inst);
  }
  for (const auto& inst : debug_) {
    cb(inst);
  }
  for (const auto& inst : annotations_) {
    cb(inst);
  }
  for (const auto& inst : types_) {
    cb(inst);
  }
  for (const auto& inst : instructions_) {
    cb(inst);
  }
}

}  // namespace spirv
}  // namespace writer
}  // namespace tint
