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
#include <string>
#include <unordered_set>
#include <utility>
#include <vector>

#include "gtest/gtest.h"
#include "source/opt/ir_context.h"
#include "src/demangler.h"
#include "src/reader/spirv/fail_stream.h"
#include "src/reader/spirv/function.h"
#include "src/reader/spirv/namer.h"
#include "src/reader/spirv/parser_impl.h"
#include "src/reader/spirv/spirv_tools_helpers_test.h"
#include "src/reader/spirv/usage.h"

namespace tint {
namespace reader {
namespace spirv {
namespace test {

// A test class that wraps ParseImpl
class ParserImplWrapperForTest {
 public:
  // Constructor
  explicit ParserImplWrapperForTest(const std::vector<uint32_t>& input);
  // Dumps SPIR-V if the conversion succeeded, then destroys the wrapper.
  ~ParserImplWrapperForTest();

  // Sets global state to force dumping of the assembly text of succesfully
  // SPIR-V.
  static void DumpSuccessfullyConvertedSpirv() {
    dump_successfully_converted_spirv_ = true;
  }
  void DeliberatelyInvalidSpirv() { deliberately_invalid_spirv_ = true; }

  // Returns a new function emitter for the given function ID.
  // Assumes ParserImpl::BuildInternalRepresentation has been run and
  // succeeded.
  FunctionEmitter function_emitter(uint32_t function_id) {
    auto* spirv_function = impl_.ir_context()->GetFunction(function_id);
    return FunctionEmitter(&impl_, *spirv_function);
  }

  // Forward methods used by tests to the real implementation.
  bool Parse() { return impl_.Parse(); }
  Program program() { return impl_.program(); }
  Namer& namer() { return impl_.namer(); }
  ProgramBuilder& builder() { return impl_.builder(); }
  const std::string error() { return impl_.error(); }
  bool success() { return impl_.success(); }
  FailStream& Fail() { return impl_.Fail(); }
  spvtools::opt::IRContext* ir_context() { return impl_.ir_context(); }

  bool BuildInternalModule() { return impl_.BuildInternalModule(); }
  bool BuildAndParseInternalModuleExceptFunctions() {
    return impl_.BuildAndParseInternalModuleExceptFunctions();
  }
  bool BuildAndParseInternalModule() {
    return impl_.BuildAndParseInternalModule();
  }
  bool RegisterUserAndStructMemberNames() {
    return impl_.RegisterUserAndStructMemberNames();
  }
  bool RegisterTypes() { return impl_.RegisterTypes(); }
  bool RegisterHandleUsage() { return impl_.RegisterHandleUsage(); }
  bool EmitModuleScopeVariables() { return impl_.EmitModuleScopeVariables(); }

  const std::unordered_set<uint32_t>& glsl_std_450_imports() const {
    return impl_.glsl_std_450_imports();
  }

  typ::Type ConvertType(uint32_t id) { return impl_.ConvertType(id); }
  DecorationList GetDecorationsFor(uint32_t id) const {
    return impl_.GetDecorationsFor(id);
  }
  DecorationList GetDecorationsForMember(uint32_t id,
                                         uint32_t member_index) const {
    return impl_.GetDecorationsForMember(id, member_index);
  }
  ast::Decoration* ConvertMemberDecoration(uint32_t struct_type_id,
                                           uint32_t member_index,
                                           const Decoration& decoration) {
    return impl_.ConvertMemberDecoration(struct_type_id, member_index,
                                         decoration);
  }
  const spvtools::opt::Instruction* GetMemoryObjectDeclarationForHandle(
      uint32_t id,
      bool follow_image) {
    return impl_.GetMemoryObjectDeclarationForHandle(id, follow_image);
  }
  const std::vector<EntryPointInfo>& GetEntryPointInfo(uint32_t entry_point) {
    return impl_.GetEntryPointInfo(entry_point);
  }
  Usage GetHandleUsage(uint32_t id) const { return impl_.GetHandleUsage(id); }
  const spvtools::opt::Instruction* GetInstructionForTest(uint32_t id) const {
    return impl_.GetInstructionForTest(id);
  }
  const ParserImpl::BuiltInPositionInfo& GetBuiltInPositionInfo() {
    return impl_.GetBuiltInPositionInfo();
  }
  Source GetSourceForResultIdForTest(uint32_t id) const {
    return impl_.GetSourceForResultIdForTest(id);
  }
  void SetHLSLStylePipelineIO() { impl_.SetHLSLStylePipelineIO(); }
  bool UseHLSLStylePipelineIO() const { return impl_.UseHLSLStylePipelineIO(); }

 private:
  ParserImpl impl_;
  // When true, indicates the input SPIR-V module is expected to fail
  // validation, but the SPIR-V reader parser is permissive and lets it through.
  bool deliberately_invalid_spirv_ = false;
  static bool dump_successfully_converted_spirv_;
};

// Sets global state to force dumping of the assembly text of succesfully
// SPIR-V.
inline void DumpSuccessfullyConvertedSpirv() {
  ParserImplWrapperForTest::DumpSuccessfullyConvertedSpirv();
}

}  // namespace test

/// SPIR-V Parser test class
template <typename T>
class SpvParserTestBase : public T {
 public:
  SpvParserTestBase() = default;
  ~SpvParserTestBase() override = default;

  /// Retrieves the parser from the helper
  /// @param input the SPIR-V binary to parse
  /// @returns a parser for the given binary
  std::unique_ptr<test::ParserImplWrapperForTest> parser(
      const std::vector<uint32_t>& input) {
    auto parser = std::make_unique<test::ParserImplWrapperForTest>(input);

    // Don't run the Resolver when building the program.
    // We're not interested in type information with these tests.
    parser->builder().SetResolveOnBuild(false);
    return parser;
  }
};

// Use this form when you don't need to template any further.
using SpvParserTest = SpvParserTestBase<::testing::Test>;

/// Returns the string dump of a statement list.
/// @param program the Program
/// @param stmts the statement list
/// @returns the string dump of a statement list.
inline std::string ToString(const Program& program,
                            const ast::StatementList& stmts) {
  std::ostringstream outs;
  for (const auto* stmt : stmts) {
    program.to_str(stmt, outs, 0);
  }
  return Demangler().Demangle(program.Symbols(), outs.str());
}

/// Returns the string dump of a statement list.
/// @param builder the ProgramBuilder
/// @param stmts the statement list
/// @returns the string dump of a statement list.
inline std::string ToString(ProgramBuilder& builder,
                            const ast::StatementList& stmts) {
  std::ostringstream outs;
  for (const auto* stmt : stmts) {
    builder.to_str(stmt, outs, 0);
  }
  return Demangler().Demangle(builder.Symbols(), outs.str());
}

}  // namespace spirv
}  // namespace reader
}  // namespace tint

#endif  // SRC_READER_SPIRV_PARSER_IMPL_TEST_HELPER_H_
