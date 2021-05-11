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

/// A test class that wraps ParseImpl
class ParserImplWrapperForTest {
 public:
  /// Constructor
  /// @param input the input data to parse
  explicit ParserImplWrapperForTest(const std::vector<uint32_t>& input);
  /// Dumps SPIR-V if the conversion succeeded, then destroys the wrapper.
  ~ParserImplWrapperForTest();

  /// Sets global state to force dumping of the assembly text of succesfully
  /// SPIR-V.
  static void DumpSuccessfullyConvertedSpirv() {
    dump_successfully_converted_spirv_ = true;
  }
  /// Marks the test has having deliberately invalid SPIR-V
  void DeliberatelyInvalidSpirv() { skip_dumping_spirv_ = true; }
  /// Marks the test's SPIR-V as not being suitable for dumping, for a stated
  /// reason.
  void SkipDumpingPending(std::string) { skip_dumping_spirv_ = true; }

  /// @returns a new function emitter for the given function ID.
  /// Assumes ParserImpl::BuildInternalRepresentation has been run and
  /// succeeded.
  /// @param function_id the SPIR-V identifier of the function
  FunctionEmitter function_emitter(uint32_t function_id) {
    auto* spirv_function = impl_.ir_context()->GetFunction(function_id);
    return FunctionEmitter(&impl_, *spirv_function);
  }

  /// Run the parser
  /// @returns true if the parse was successful, false otherwise.
  bool Parse() { return impl_.Parse(); }

  /// @returns the program. The program builder in the parser will be reset
  /// after this.
  Program program() { return impl_.program(); }

  /// @returns the namer object
  Namer& namer() { return impl_.namer(); }

  /// @returns a reference to the internal builder, without building the
  /// program. To be used only for testing.
  ProgramBuilder& builder() { return impl_.builder(); }

  /// @returns the accumulated error string
  const std::string error() { return impl_.error(); }

  /// @return true if failure has not yet occurred
  bool success() { return impl_.success(); }

  /// Logs failure, ands return a failure stream to accumulate diagnostic
  /// messages. By convention, a failure should only be logged along with
  /// a non-empty string diagnostic.
  /// @returns the failure stream
  FailStream& Fail() { return impl_.Fail(); }

  /// @returns a borrowed pointer to the internal representation of the module.
  /// This is null until BuildInternalModule has been called.
  spvtools::opt::IRContext* ir_context() { return impl_.ir_context(); }

  /// Builds the internal representation of the SPIR-V module.
  /// Assumes the module is somewhat well-formed.  Normally you
  /// would want to validate the SPIR-V module before attempting
  /// to build this internal representation. Also computes a topological
  /// ordering of the functions.
  /// This is a no-op if the parser has already failed.
  /// @returns true if the parser is still successful.
  bool BuildInternalModule() { return impl_.BuildInternalModule(); }

  /// Builds an internal representation of the SPIR-V binary,
  /// and parses the module, except functions, into a Tint AST module.
  /// Diagnostics are emitted to the error stream.
  /// @returns true if it was successful.
  bool BuildAndParseInternalModuleExceptFunctions() {
    return impl_.BuildAndParseInternalModuleExceptFunctions();
  }

  /// Builds an internal representation of the SPIR-V binary,
  /// and parses it into a Tint AST module.  Diagnostics are emitted
  /// to the error stream.
  /// @returns true if it was successful.
  bool BuildAndParseInternalModule() {
    return impl_.BuildAndParseInternalModule();
  }

  /// Registers user names for SPIR-V objects, from OpName, and OpMemberName.
  /// Also synthesizes struct field names.  Ensures uniqueness for names for
  /// SPIR-V IDs, and uniqueness of names of fields within any single struct.
  /// This is a no-op if the parser has already failed.
  /// @returns true if parser is still successful.
  bool RegisterUserAndStructMemberNames() {
    return impl_.RegisterUserAndStructMemberNames();
  }

  /// Register Tint AST types for SPIR-V types, including type aliases as
  /// needed.  This is a no-op if the parser has already failed.
  /// @returns true if parser is still successful.
  bool RegisterTypes() { return impl_.RegisterTypes(); }

  /// Register sampler and texture usage for memory object declarations.
  /// This must be called after we've registered line numbers for all
  /// instructions. This is a no-op if the parser has already failed.
  /// @returns true if parser is still successful.
  bool RegisterHandleUsage() { return impl_.RegisterHandleUsage(); }

  /// Emits module-scope variables.
  /// This is a no-op if the parser has already failed.
  /// @returns true if parser is still successful.
  bool EmitModuleScopeVariables() { return impl_.EmitModuleScopeVariables(); }

  /// @returns the set of SPIR-V IDs for imports of the "GLSL.std.450"
  /// extended instruction set.
  const std::unordered_set<uint32_t>& glsl_std_450_imports() const {
    return impl_.glsl_std_450_imports();
  }

  /// Converts a SPIR-V type to a Tint type, and saves it for fast lookup.
  /// If the type is only used for builtins, then register that specially,
  /// and return null.  If the type is a sampler, image, or sampled image, then
  /// return the Void type, because those opaque types are handled in a
  /// different way.
  /// On failure, logs an error and returns null.  This should only be called
  /// after the internal representation of the module has been built.
  /// @param id the SPIR-V ID of a type.
  /// @returns a Tint type, or nullptr
  const Type* ConvertType(uint32_t id) { return impl_.ConvertType(id); }

  /// Gets the list of decorations for a SPIR-V result ID.  Returns an empty
  /// vector if the ID is not a result ID, or if no decorations target that ID.
  /// The internal representation must have already been built.
  /// @param id SPIR-V ID
  /// @returns the list of decorations on the given ID
  DecorationList GetDecorationsFor(uint32_t id) const {
    return impl_.GetDecorationsFor(id);
  }

  /// Gets the list of decorations for the member of a struct.  Returns an empty
  /// list if the `id` is not the ID of a struct, or if the member index is out
  /// of range, or if the target member has no decorations.
  /// The internal representation must have already been built.
  /// @param id SPIR-V ID of a struct
  /// @param member_index the member within the struct
  /// @returns the list of decorations on the member
  DecorationList GetDecorationsForMember(uint32_t id,
                                         uint32_t member_index) const {
    return impl_.GetDecorationsForMember(id, member_index);
  }

  /// Converts a SPIR-V struct member decoration. If the decoration is
  /// recognized but deliberately dropped, then returns nullptr without a
  /// diagnostic. On failure, emits a diagnostic and returns nullptr.
  /// @param struct_type_id the ID of the struct type
  /// @param member_index the index of the member
  /// @param decoration an encoded SPIR-V Decoration
  /// @returns the corresponding ast::StructuMemberDecoration
  ast::Decoration* ConvertMemberDecoration(uint32_t struct_type_id,
                                           uint32_t member_index,
                                           const Decoration& decoration) {
    return impl_.ConvertMemberDecoration(struct_type_id, member_index,
                                         decoration);
  }

  /// For a SPIR-V ID that might define a sampler, image, or sampled image
  /// value, return the SPIR-V instruction that represents the memory object
  /// declaration for the object.  If we encounter an OpSampledImage along the
  /// way, follow the image operand when follow_image is true; otherwise follow
  /// the sampler operand. Returns nullptr if we can't trace back to a memory
  /// object declaration.  Emits an error and returns nullptr when the scan
  /// fails due to a malformed module. This method can be used any time after
  /// BuildInternalModule has been invoked.
  /// @param id the SPIR-V ID of the sampler, image, or sampled image
  /// @param follow_image indicates whether to follow the image operand of
  /// OpSampledImage
  /// @returns the memory object declaration for the handle, or nullptr
  const spvtools::opt::Instruction* GetMemoryObjectDeclarationForHandle(
      uint32_t id,
      bool follow_image) {
    return impl_.GetMemoryObjectDeclarationForHandle(id, follow_image);
  }

  /// @param entry_point the SPIR-V ID of an entry point.
  /// @returns the entry point info for the given ID
  const std::vector<EntryPointInfo>& GetEntryPointInfo(uint32_t entry_point) {
    return impl_.GetEntryPointInfo(entry_point);
  }

  /// Returns the handle usage for a memory object declaration.
  /// @param id SPIR-V ID of a sampler or image OpVariable or
  /// OpFunctionParameter
  /// @returns the handle usage, or an empty usage object.
  Usage GetHandleUsage(uint32_t id) const { return impl_.GetHandleUsage(id); }

  /// Returns the SPIR-V instruction with the given ID, or nullptr.
  /// @param id the SPIR-V result ID
  /// @returns the instruction, or nullptr on error
  const spvtools::opt::Instruction* GetInstructionForTest(uint32_t id) const {
    return impl_.GetInstructionForTest(id);
  }

  /// @returns info about the gl_Position builtin variable.
  const ParserImpl::BuiltInPositionInfo& GetBuiltInPositionInfo() {
    return impl_.GetBuiltInPositionInfo();
  }

  /// Returns the source record for the SPIR-V instruction with the given
  /// result ID.
  /// @param id the SPIR-V result id.
  /// @return the Source record, or a default one
  Source GetSourceForResultIdForTest(uint32_t id) const {
    return impl_.GetSourceForResultIdForTest(id);
  }

  /// Changes pipeline IO to be HLSL-style: as entry point parameters and
  /// return.
  /// TODO(crbug.com/tint/508): Once all this support has landed, switch
  /// over to that, and remove the old support.
  void SetHLSLStylePipelineIO() { impl_.SetHLSLStylePipelineIO(); }

  /// @returns true if HLSL-style IO should be used.
  bool UseHLSLStylePipelineIO() const { return impl_.UseHLSLStylePipelineIO(); }

 private:
  ParserImpl impl_;
  /// When true, indicates the input SPIR-V module should not be emitted.
  /// It's either deliberately invalid, or not supported for some pending
  /// reason.
  bool skip_dumping_spirv_ = false;
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
