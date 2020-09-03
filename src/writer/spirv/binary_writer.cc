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

#include "src/writer/spirv/binary_writer.h"

#include <cstring>

namespace tint {
namespace writer {
namespace spirv {
namespace {

const uint32_t kGeneratorId = 23u << 16;

}  // namespace

BinaryWriter::BinaryWriter() = default;

BinaryWriter::~BinaryWriter() = default;

void BinaryWriter::WriteBuilder(Builder* builder) {
  out_.reserve(builder->total_size());
  builder->iterate(
      [this](const Instruction& inst) { this->process_instruction(inst); });
}

void BinaryWriter::WriteInstruction(const Instruction& inst) {
  process_instruction(inst);
}

void BinaryWriter::WriteHeader(uint32_t bound) {
  out_.push_back(spv::MagicNumber);
  out_.push_back(0x00010300);  // Version 1.3
  out_.push_back(kGeneratorId);
  out_.push_back(bound);
  out_.push_back(0);
}

void BinaryWriter::process_instruction(const Instruction& inst) {
  out_.push_back(inst.word_length() << 16 |
                 static_cast<uint32_t>(inst.opcode()));
  for (const auto& op : inst.operands()) {
    process_op(op);
  }
}

void BinaryWriter::process_op(const Operand& op) {
  if (op.IsFloat()) {
    // Allocate space for the float
    out_.push_back(0);
    auto f = op.to_f();
    uint8_t* ptr = reinterpret_cast<uint8_t*>(out_.data() + (out_.size() - 1));
    memcpy(ptr, &f, 4);
  } else if (op.IsInt()) {
    out_.push_back(op.to_i());
  } else {
    auto idx = out_.size();
    const auto& str = op.to_s();
    out_.resize(out_.size() + op.length(), 0);
    memcpy(out_.data() + idx, str.c_str(), str.size() + 1);
  }
}

}  // namespace spirv
}  // namespace writer
}  // namespace tint
