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

// TODO(dsinclair): Reserve a generator ID for Tint.
// https://github.com/KhronosGroup/SPIRV-Headers/blob/master/include/spirv/spir-v.xml#L75
const uint32_t kGeneratorId = 0;

}  // namespace

BinaryWriter::BinaryWriter() = default;

BinaryWriter::~BinaryWriter() = default;

bool BinaryWriter::Write(const Builder& builder) {
  out_.resize(builder.total_size(), 0);

  out_[idx_++] = spv::MagicNumber;
  out_[idx_++] = 0x00010300;  // Version 1.3
  out_[idx_++] = kGeneratorId;
  out_[idx_++] = builder.id_bound();
  out_[idx_++] = 0;

  builder.iterate([this](const Instruction& inst) {
    out_[idx_++] =
        inst.word_length() << 16 | static_cast<uint32_t>(inst.opcode());

    for (const auto& op : inst.operands()) {
      process_op(op);
    }
  });
  return true;
}

void BinaryWriter::process_op(const Operand& op) {
  if (op.IsFloat()) {
    auto f = op.to_f();
    memcpy(out_.data() + idx_, &f, 4);
  } else if (op.IsInt()) {
    out_[idx_] = op.to_i();
  } else {
    const auto& str = op.to_s();
    // This depends on the vector being initialized to 0 values so the string
    // is correctly padded.
    memcpy(out_.data() + idx_, str.c_str(), str.size() + 1);
  }
  idx_ += op.length();
}

}  // namespace spirv
}  // namespace writer
}  // namespace tint
