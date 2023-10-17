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

#ifndef SRC_TINT_LANG_GLSL_WRITER_PRINTER_PRINTER_H_
#define SRC_TINT_LANG_GLSL_WRITER_PRINTER_PRINTER_H_

#include <string>

#include "src/tint/lang/core/ir/module.h"
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

    /// @returns success or failure
    tint::Result<SuccessType> Generate();

    /// @copydoc tint::TextGenerator::Result
    std::string Result() const override;

  private:
    core::ir::Module& ir_;

    /// The buffer holding preamble text
    TextBuffer preamble_buffer_;
};

}  // namespace tint::glsl::writer

#endif  // SRC_TINT_LANG_GLSL_WRITER_PRINTER_PRINTER_H_
