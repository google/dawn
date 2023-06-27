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

#ifndef SRC_TINT_WRITER_IR_TEXT_GENERATOR_H_
#define SRC_TINT_WRITER_IR_TEXT_GENERATOR_H_

#include <string>

#include "src/tint/ir/module.h"
#include "src/tint/writer/text_generator.h"

namespace tint::writer {

/// Helper methods for generators which are creating text output
class IRTextGenerator : public TextGenerator {
  public:
    /// Constructor
    /// @param mod the IR module used by the generator
    explicit IRTextGenerator(ir::Module* mod);
    ~IRTextGenerator() override;

    /// @return a new, unique identifier with the given prefix.
    /// @param prefix optional prefix to apply to the generated identifier. If
    /// empty "tint_symbol" will be used.
    std::string UniqueIdentifier(const std::string& prefix = "") override;

    /// @returns the generated shader string
    std::string Result() const override {
        utils::StringStream ss;
        ss << preamble_buffer_.String() << std::endl << main_buffer_.String();
        return ss.str();
    }

  protected:
    /// The IR module
    ir::Module* ir_ = nullptr;

    /// The buffer holding preamble text
    TextBuffer preamble_buffer_;
};

}  // namespace tint::writer

#endif  // SRC_TINT_WRITER_IR_TEXT_GENERATOR_H_
