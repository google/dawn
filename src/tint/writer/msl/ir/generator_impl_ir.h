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

#ifndef SRC_TINT_WRITER_MSL_IR_GENERATOR_IMPL_IR_H_
#define SRC_TINT_WRITER_MSL_IR_GENERATOR_IMPL_IR_H_

#include "src/tint/diagnostic/diagnostic.h"
#include "src/tint/ir/module.h"
#include "src/tint/utils/string_stream.h"
#include "src/tint/writer/ir_text_generator.h"

namespace tint::writer::msl {

/// Implementation class for the MSL generator
class GeneratorImplIr : public IRTextGenerator {
  public:
    /// Constructor
    /// @param module the Tint IR module to generate
    explicit GeneratorImplIr(ir::Module* module);
    ~GeneratorImplIr() override;

    /// @returns true on successful generation; false otherwise
    bool Generate();

    /// Emit the function
    /// @param func the function to emit
    void EmitFunction(ir::Function* func);

    /// Emit a type
    /// @param out the stream to emit too
    /// @param ty the type to emit
    void EmitType(utils::StringStream& out, const type::Type* ty);
};

}  // namespace tint::writer::msl

#endif  // SRC_TINT_WRITER_MSL_IR_GENERATOR_IMPL_IR_H_
