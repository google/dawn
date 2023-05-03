// Copyright 2023 The Tint Authors.
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

#include "src/tint/writer/spirv/module.h"

namespace tint::writer::spirv {
namespace {

/// Helper to return the size in words of an instruction list when serialized.
/// @param instructions the instruction list
/// @returns the number of words needed to serialize the list
uint32_t SizeOf(const InstructionList& instructions) {
    uint32_t size = 0;
    for (const auto& inst : instructions) {
        size += inst.word_length();
    }
    return size;
}

}  // namespace

Module::Module() {}

Module::~Module() = default;

uint32_t Module::TotalSize() const {
    // The 5 covers the magic, version, generator, id bound and reserved.
    uint32_t size = 5;

    size += SizeOf(capabilities_);
    size += SizeOf(extensions_);
    size += SizeOf(ext_imports_);
    size += SizeOf(memory_model_);
    size += SizeOf(entry_points_);
    size += SizeOf(execution_modes_);
    size += SizeOf(debug_);
    size += SizeOf(annotations_);
    size += SizeOf(types_);
    for (const auto& func : functions_) {
        size += func.word_length();
    }

    return size;
}

void Module::Iterate(std::function<void(const Instruction&)> cb) const {
    for (const auto& inst : capabilities_) {
        cb(inst);
    }
    for (const auto& inst : extensions_) {
        cb(inst);
    }
    for (const auto& inst : ext_imports_) {
        cb(inst);
    }
    for (const auto& inst : memory_model_) {
        cb(inst);
    }
    for (const auto& inst : entry_points_) {
        cb(inst);
    }
    for (const auto& inst : execution_modes_) {
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
    for (const auto& func : functions_) {
        func.iterate(cb);
    }
}

void Module::PushCapability(uint32_t cap) {
    if (capability_set_.count(cap) == 0) {
        capability_set_.insert(cap);
        capabilities_.push_back(Instruction{spv::Op::OpCapability, {Operand(cap)}});
    }
}

void Module::PushExtension(const char* extension) {
    extensions_.push_back(Instruction{spv::Op::OpExtension, {Operand(extension)}});
}

}  // namespace tint::writer::spirv
