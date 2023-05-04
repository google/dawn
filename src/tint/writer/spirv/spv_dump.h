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

#ifndef SRC_TINT_WRITER_SPIRV_SPV_DUMP_H_
#define SRC_TINT_WRITER_SPIRV_SPV_DUMP_H_

#include <string>
#include <vector>

#include "src/tint/writer/spirv/builder.h"

namespace tint::writer::spirv {

/// Disassembles SPIR-V binary data into its textual form.
/// @param data the SPIR-V binary data
/// @returns the disassembled SPIR-V string
std::string Disassemble(const std::vector<uint32_t>& data);

/// Dumps the given builder to a SPIR-V disassembly string
/// @param builder the builder to convert
/// @returns the builder as a SPIR-V disassembly string
std::string DumpBuilder(Builder& builder);

/// Dumps the given module to a SPIR-V disassembly string
/// @param module the module to convert
/// @returns the module as a SPIR-V disassembly string
std::string DumpModule(Module& module);

/// Dumps the given instruction to a SPIR-V disassembly string
/// @param inst the instruction to dump
/// @returns the instruction as a SPIR-V disassembly string
std::string DumpInstruction(const Instruction& inst);

/// Dumps the given instructions to a SPIR-V disassembly string
/// @param insts the instructions to dump
/// @returns the instruction as a SPIR-V disassembly string
std::string DumpInstructions(const InstructionList& insts);

}  // namespace tint::writer::spirv

#endif  // SRC_TINT_WRITER_SPIRV_SPV_DUMP_H_
