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

#ifndef SRC_TINT_LANG_SPIRV_WRITER_COMMON_BINARY_WRITER_H_
#define SRC_TINT_LANG_SPIRV_WRITER_COMMON_BINARY_WRITER_H_

#include <vector>

#include "src/tint/lang/spirv/writer/common/module.h"

namespace tint::spirv::writer {

/// Writer to convert from module to SPIR-V binary.
class BinaryWriter {
  public:
    /// Constructor
    BinaryWriter();
    ~BinaryWriter();

    /// Writes the SPIR-V header.
    /// @param bound the bound to output
    /// @param version the generator version number
    void WriteHeader(uint32_t bound, uint32_t version = 0);

    /// Writes the given module data into a binary. Note, this does not emit the SPIR-V header. You
    /// **must** call WriteHeader() before WriteModule() if you want the SPIR-V to be emitted.
    /// @param module the module to assemble from
    void WriteModule(const Module* module);

    /// Writes the given instruction into the binary.
    /// @param inst the instruction to assemble
    void WriteInstruction(const Instruction& inst);

    /// @returns the assembled SPIR-V
    const std::vector<uint32_t>& Result() const { return out_; }

    /// @returns the assembled SPIR-V
    std::vector<uint32_t>& Result() { return out_; }

  private:
    void process_instruction(const Instruction& inst);
    void process_op(const Operand& op);

    std::vector<uint32_t> out_;
};

}  // namespace tint::spirv::writer

#endif  // SRC_TINT_LANG_SPIRV_WRITER_COMMON_BINARY_WRITER_H_
