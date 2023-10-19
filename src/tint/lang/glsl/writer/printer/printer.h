// Copyright 2023 The Dawn & Tint Authors
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// 1. Redistributions of source code must retain the above copyright notice, this
//    list of conditions and the following disclaimer.
//
// 2. Redistributions in binary form must reproduce the above copyright notice,
//    this list of conditions and the following disclaimer in the documentation
//    and/or other materials provided with the distribution.
//
// 3. Neither the name of the copyright holder nor the names of its
//    contributors may be used to endorse or promote products derived from
//    this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
// FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
// DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
// CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
// OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#ifndef SRC_TINT_LANG_GLSL_WRITER_PRINTER_PRINTER_H_
#define SRC_TINT_LANG_GLSL_WRITER_PRINTER_PRINTER_H_

#include <string>

#include "src/tint/lang/core/ir/module.h"
#include "src/tint/lang/glsl/writer/common/version.h"
#include "src/tint/utils/generator/text_generator.h"

// Forward declarations
namespace tint::core::ir {
class Binary;
class ExitIf;
class If;
class Let;
class Load;
class Return;
class Unreachable;
class Var;
}  // namespace tint::core::ir

namespace tint::glsl::writer {

/// Implementation class for the MSL generator
class Printer : public tint::TextGenerator {
  public:
    /// Constructor
    /// @param module the Tint IR module to generate
    explicit Printer(core::ir::Module& module);
    ~Printer() override;

    /// @param version the GLSL version information
    /// @returns success or failure
    tint::Result<SuccessType> Generate(Version version);

    /// @copydoc tint::TextGenerator::Result
    std::string Result() const override;

  private:
    /// Emit the function
    /// @param func the function to emit
    void EmitFunction(core::ir::Function* func);

    /// Emit a block
    /// @param block the block to emit
    void EmitBlock(core::ir::Block* block);
    /// Emit the instructions in a block
    /// @param block the block with the instructions to emit
    void EmitBlockInstructions(core::ir::Block* block);

    /// Emit a return instruction
    /// @param r the return instruction
    void EmitReturn(core::ir::Return* r);
    /// Emit an unreachable instruction
    void EmitUnreachable();

    /// Emit a type
    /// @param out the stream to emit too
    /// @param ty the type to emit
    void EmitType(StringStream& out, const core::type::Type* ty);

    core::ir::Module& ir_;

    /// The buffer holding preamble text
    TextBuffer preamble_buffer_;

    /// The current function being emitted
    core::ir::Function* current_function_ = nullptr;
    /// The current block being emitted
    core::ir::Block* current_block_ = nullptr;
};

}  // namespace tint::glsl::writer

#endif  // SRC_TINT_LANG_GLSL_WRITER_PRINTER_PRINTER_H_
