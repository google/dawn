// Copyright 2024 The Dawn & Tint Authors
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

#include "src/tint/lang/hlsl/writer/printer/printer.h"

#include <utility>

#include "src/tint/lang/core/ir/module.h"
#include "src/tint/lang/core/ir/return.h"
#include "src/tint/lang/core/ir/validator.h"
#include "src/tint/lang/core/type/void.h"
#include "src/tint/utils/generator/text_generator.h"

namespace tint::hlsl::writer {
namespace {

/// PIMPL class for the HLSL generator
class Printer : public tint::TextGenerator {
  public:
    /// Constructor
    /// @param module the IR module to generate
    explicit Printer(core::ir::Module& module) : ir_(module) {}

    /// @returns the generated HLSL shader
    tint::Result<PrintResult> Generate() {
        auto valid = core::ir::ValidateAndDumpIfNeeded(ir_, "HLSL writer");
        if (valid != Success) {
            return std::move(valid.Failure());
        }

        // TOOD(dsinclair): EmitRootBlock

        // Emit functions.
        for (auto* func : ir_.DependencyOrderedFunctions()) {
            EmitFunction(func);
        }

        result_.hlsl = main_buffer_.String();
        return std::move(result_);
    }

  private:
    /// The result of printing the module.
    PrintResult result_;

    core::ir::Module& ir_;

    /// A hashmap of value to name
    Hashmap<const core::ir::Value*, std::string, 32> names_;

    /// Emit the function
    /// @param func the function to emit
    void EmitFunction(const core::ir::Function* func) {
        {
            auto out = Line();
            auto func_name = NameOf(func);

            // TODO(dsinclair): Pipeline stage and workgroup information

            EmitType(out, func->ReturnType());
            out << " " << func_name << "(";

            // TODO(dsinclair): Parameters

            out << ") {";
        }
        {
            const ScopedIndent si(current_buffer_);
            EmitBlock(func->Block());
        }

        Line() << "}";
    }

    /// Emit a block
    /// @param block the block to emit
    void EmitBlock(const core::ir::Block* block) {
        for (auto* inst : *block) {
            TINT_ASSERT(inst->Is<core::ir::Return>());

            // TODO(dsinclair): handle instructions
            Line() << "return;";
        }
    }

    /// Emit a type
    /// @param out the stream to emit too
    /// @param ty the type to emit
    void EmitType(StringStream& out, const core::type::Type* ty) {
        TINT_ASSERT(ty->Is<core::type::Void>());

        // TODO(dsinclair): Emit types
        out << "void";
    }

    /// @param value the value to get the name of
    /// @returns the name of the given value, creating a new unique name if the value is unnamed in
    /// the module.
    std::string NameOf(const core::ir::Value* value) {
        return names_.GetOrAdd(value, [&] {
            auto sym = ir_.NameOf(value);
            return sym.IsValid() ? sym.Name() : UniqueIdentifier("v");
        });
    }

    /// @return a new, unique identifier with the given prefix.
    /// @param prefix optional prefix to apply to the generated identifier. If empty
    /// "tint_symbol" will be used.
    std::string UniqueIdentifier(const std::string& prefix /* = "" */) {
        return ir_.symbols.New(prefix).Name();
    }
};

}  // namespace

Result<PrintResult> Print(core::ir::Module& module) {
    return Printer{module}.Generate();
}

PrintResult::PrintResult() = default;

PrintResult::~PrintResult() = default;

PrintResult::PrintResult(const PrintResult&) = default;

PrintResult& PrintResult::operator=(const PrintResult&) = default;

}  // namespace tint::hlsl::writer
