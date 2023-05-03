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

#include "src/tint/writer/spirv/binary_writer.h"

#include <cstring>
#include <string>

namespace tint::writer::spirv {
namespace {

const uint32_t kGeneratorId = 23u << 16;

}  // namespace

BinaryWriter::BinaryWriter() = default;

BinaryWriter::~BinaryWriter() = default;

void BinaryWriter::WriteModule(const Module* module) {
    out_.reserve(module->TotalSize());
    module->Iterate([this](const Instruction& inst) { this->process_instruction(inst); });
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
    out_.push_back(inst.word_length() << 16 | static_cast<uint32_t>(inst.opcode()));
    for (const auto& op : inst.operands()) {
        process_op(op);
    }
}

void BinaryWriter::process_op(const Operand& op) {
    if (auto* i = std::get_if<uint32_t>(&op)) {
        out_.push_back(*i);
        return;
    }
    if (auto* f = std::get_if<float>(&op)) {
        // Allocate space for the float
        out_.push_back(0);
        uint8_t* ptr = reinterpret_cast<uint8_t*>(out_.data() + (out_.size() - 1));
        memcpy(ptr, f, 4);
        return;
    }
    if (auto* str = std::get_if<std::string>(&op)) {
        auto idx = out_.size();
        out_.resize(out_.size() + OperandLength(op), 0);
        memcpy(out_.data() + idx, str->c_str(), str->size() + 1);
        return;
    }
}

}  // namespace tint::writer::spirv
