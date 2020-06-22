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

#ifndef SRC_WRITER_SPIRV_SPV_DUMP_H_
#define SRC_WRITER_SPIRV_SPV_DUMP_H_

#include <string>
#include <vector>

#include "src/writer/spirv/builder.h"
#include "src/writer/spirv/instruction.h"

namespace tint {
namespace writer {
namespace spirv {

/// Dumps the given builder to a SPIR-V disassembly string
/// @param builder the builder to convert
/// @returns the builder as a SPIR-V disassembly string
std::string DumpBuilder(const Builder& builder);

/// Dumps the given instruction to a SPIR-V disassembly string
/// @param inst the instruction to dump
/// @returns the instruction as a SPIR-V disassembly string
std::string DumpInstruction(const Instruction& inst);

/// Dumps the given instructions to a SPIR-V disassembly string
/// @param insts the instructions to dump
/// @returns the instruction as a SPIR-V disassembly string
std::string DumpInstructions(const InstructionList& insts);

}  // namespace spirv
}  // namespace writer
}  // namespace tint

#endif  // SRC_WRITER_SPIRV_SPV_DUMP_H_
