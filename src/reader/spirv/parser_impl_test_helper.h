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

#ifndef SRC_READER_SPIRV_PARSER_IMPL_TEST_HELPER_H_
#define SRC_READER_SPIRV_PARSER_IMPL_TEST_HELPER_H_

#include <memory>
#include <sstream>
#include <string>
#include <vector>

#include "gtest/gtest.h"
#include "source/opt/ir_context.h"
#include "src/demangler.h"
#include "src/reader/spirv/parser_impl.h"

namespace tint {
namespace reader {
namespace spirv {

/// SPIR-V Parser test class
template <typename T>
class SpvParserTestBase : public T {
 public:
  SpvParserTestBase() = default;
  ~SpvParserTestBase() override = default;

  /// Retrieves the parser from the helper
  /// @param input the SPIR-V binary to parse
  /// @returns a parser for the given binary
  std::unique_ptr<ParserImpl> parser(const std::vector<uint32_t>& input) {
    return std::make_unique<ParserImpl>(input);
  }

  /// Gets the internal representation of the function with the given ID.
  /// Assumes ParserImpl::BuildInternalRepresentation has been run and
  /// succeeded.
  /// @param parser the parser
  /// @param id the SPIR-V ID of the function
  /// @returns the internal representation of the function
  spvtools::opt::Function* spirv_function(ParserImpl* parser, uint32_t id) {
    return parser->ir_context()->GetFunction(id);
  }
};

// Use this form when you don't need to template any further.
using SpvParserTest = SpvParserTestBase<::testing::Test>;

/// Returns the string dump of a statement list.
/// @param program the program
/// @param stmts the statement list
/// @returns the string dump of a statement list.
inline std::string ToString(const Program& program,
                            const ast::StatementList& stmts) {
  std::ostringstream outs;
  for (const auto* stmt : stmts) {
    stmt->to_str(outs, 0);
  }
  return Demangler().Demangle(program, outs.str());
}

}  // namespace spirv
}  // namespace reader
}  // namespace tint

#endif  // SRC_READER_SPIRV_PARSER_IMPL_TEST_HELPER_H_
