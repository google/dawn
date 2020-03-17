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

#ifndef SRC_READER_SPIRV_PARSER_IMPL_H_
#define SRC_READER_SPIRV_PARSER_IMPL_H_

#include <cstdint>
#include <memory>
#include <sstream>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "source/opt/constants.h"
#include "source/opt/decoration_manager.h"
#include "source/opt/ir_context.h"
#include "source/opt/module.h"
#include "source/opt/type_manager.h"
#include "spirv-tools/libspirv.hpp"
#include "src/ast/import.h"
#include "src/ast/module.h"
#include "src/reader/reader.h"
#include "src/reader/spirv/fail_stream.h"

namespace tint {
namespace reader {
namespace spirv {

/// Parser implementation for SPIR-V.
class ParserImpl : Reader {
 public:
  /// Creates a new parser
  /// @param input the input data to parse
  explicit ParserImpl(const std::vector<uint32_t>& input);
  /// Destructor
  ~ParserImpl() override;

  /// Run the parser
  /// @returns true if the parse was successful, false otherwise.
  bool Parse() override;

  /// @returns the module. The module in the parser will be reset after this.
  ast::Module module() override;

  /// Logs failure, ands return a failure stream to accumulate diagnostic
  /// messages. By convention, a failure should only be logged along with
  /// a non-empty string diagnostic.
  /// @returns the failure stream
  FailStream& Fail() {
    success_ = false;
    return fail_stream_;
  }

  /// @returns the accumulated error string
  const std::string error() { return errors_.str(); }

  /// Builds an internal representation of the SPIR-V binary,
  /// and parses it into a Tint AST module.  Diagnostics are emitted
  /// to the error stream.
  /// @returns true if it was successful.
  bool BuildAndParseInternalModule() {
    return BuildInternalModule() && ParseInternalModule();
  }

  /// @returns the set of SPIR-V IDs for imports of the "GLSL.std.450"
  /// extended instruction set.
  const std::unordered_set<uint32_t>& glsl_std_450_imports() const {
    return glsl_std_450_imports_;
  }

 private:
  /// Builds the internal representation of the SPIR-V module.
  /// Assumes the module is somewhat well-formed.  Normally you
  /// would want to validate the SPIR-V module before attempting
  /// to build this internal representation.
  /// @returns true if successful.
  bool BuildInternalModule();

  /// Walks the internal representation of the module to populate
  /// the AST form of the module.
  /// @returns true on success
  bool ParseInternalModule();

  /// Destroys the internal representation of the SPIR-V module.
  void ResetInternalModule();

  /// Parses OpExtInstImport instructions.
  bool ParseExtendedInstructionImports();

  // The SPIR-V binary we're parsing
  std::vector<uint32_t> spv_binary_;

  // The resulting module in Tint AST form.
  ast::Module ast_module_;

  // Is the parse successful?
  bool success_ = true;
  // Collector for diagnostic messages.
  std::stringstream errors_;
  FailStream fail_stream_;
  spvtools::MessageConsumer message_consumer_;

  // The internal representation of the SPIR-V module and its context.
  spvtools::Context tools_context_;
  spvtools::SpirvTools tools_;
  // All the state is owned by ir_context_.
  std::unique_ptr<spvtools::opt::IRContext> ir_context_;
  // The following are borrowed pointers to the internal state of ir_context_.
  spvtools::opt::Module* module_ = nullptr;
  spvtools::opt::analysis::DefUseManager* def_use_mgr_ = nullptr;
  spvtools::opt::analysis::ConstantManager* constant_mgr_ = nullptr;
  spvtools::opt::analysis::TypeManager* type_mgr_ = nullptr;
  spvtools::opt::analysis::DecorationManager* deco_mgr_ = nullptr;

  /// Maps a SPIR-V ID for an external instruction import to an AST import
  std::unordered_map<uint32_t, ast::Import*> import_map_;
  // The set of IDs that are imports of the GLSL.std.450 extended instruction
  // sets.
  std::unordered_set<uint32_t> glsl_std_450_imports_;
};

}  // namespace spirv
}  // namespace reader
}  // namespace tint

#endif  // SRC_READER_SPIRV_PARSER_IMPL_H_
