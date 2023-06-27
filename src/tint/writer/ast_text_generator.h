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

#ifndef SRC_TINT_WRITER_AST_TEXT_GENERATOR_H_
#define SRC_TINT_WRITER_AST_TEXT_GENERATOR_H_

#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

#include "src/tint/program_builder.h"
#include "src/tint/writer/text_generator.h"

namespace tint::writer {

/// Helper methods for generators which are creating text output
class ASTTextGenerator : public TextGenerator {
  public:
    /// Constructor
    /// @param program the program used by the generator
    explicit ASTTextGenerator(const Program* program);
    ~ASTTextGenerator() override;

    /// @return a new, unique identifier with the given prefix.
    /// @param prefix optional prefix to apply to the generated identifier. If
    /// empty "tint_symbol" will be used.
    std::string UniqueIdentifier(const std::string& prefix = "") override;

  protected:
    /// @returns the resolved type of the ast::Expression `expr`
    /// @param expr the expression
    const type::Type* TypeOf(const ast::Expression* expr) const { return builder_.TypeOf(expr); }

    /// @returns the resolved type of the ast::TypeDecl `type_decl`
    /// @param type_decl the type
    const type::Type* TypeOf(const ast::TypeDecl* type_decl) const {
        return builder_.TypeOf(type_decl);
    }

    /// The program
    Program const* const program_;
    /// A ProgramBuilder that thinly wraps program_
    ProgramBuilder builder_;

  private:
    /// Map of builtin structure to unique generated name
    std::unordered_map<const type::Struct*, std::string> builtin_struct_names_;
};

}  // namespace tint::writer

#endif  // SRC_TINT_WRITER_AST_TEXT_GENERATOR_H_
