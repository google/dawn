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
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "source/opt/constants.h"
#include "source/opt/decoration_manager.h"
#include "source/opt/ir_context.h"
#include "source/opt/module.h"
#include "source/opt/type_manager.h"
#include "source/opt/types.h"
#include "spirv-tools/libspirv.hpp"
#include "src/ast/expression.h"
#include "src/ast/import.h"
#include "src/ast/module.h"
#include "src/ast/struct_member_decoration.h"
#include "src/ast/type/type.h"
#include "src/reader/reader.h"
#include "src/reader/spirv/enum_converter.h"
#include "src/reader/spirv/fail_stream.h"
#include "src/reader/spirv/namer.h"

namespace tint {
namespace reader {
namespace spirv {

/// The binary representation of a SPIR-V decoration enum followed by its
/// operands, if any.
/// Example:   { SpvDecorationBlock }
/// Example:   { SpvDecorationArrayStride, 16 }
using Decoration = std::vector<uint32_t>;
using DecorationList = std::vector<Decoration>;

// An AST expression with its type.
struct TypedExpression {
  /// Dummy constructor
  TypedExpression() : type(nullptr), expr(nullptr) {}
  /// Constructor
  /// @param t the type
  /// @param e the expression
  TypedExpression(ast::type::Type* t, std::unique_ptr<ast::Expression> e)
      : type(t), expr(std::move(e)) {}
  /// Move constructor
  /// @param other the other typed expression
  TypedExpression(TypedExpression&& other)
      : type(other.type), expr(std::move(other.expr)) {}
  /// The type
  ast::type::Type* type;
  /// The expression
  std::unique_ptr<ast::Expression> expr;
};

/// Parser implementation for SPIR-V.
class ParserImpl : Reader {
 public:
  /// Creates a new parser
  /// @param ctx the non-null context object
  /// @param input the input data to parse
  ParserImpl(Context* ctx, const std::vector<uint32_t>& input);
  /// Destructor
  ~ParserImpl() override;

  /// Run the parser
  /// @returns true if the parse was successful, false otherwise.
  bool Parse() override;

  /// @returns the module. The module in the parser will be reset after this.
  ast::Module module() override;

  /// Returns a pointer to the module, without resetting it.
  /// @returns the module
  ast::Module& get_module() { return ast_module_; }

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
  /// Builds an internal representation of the SPIR-V binary,
  /// and parses the module, except functions, into a Tint AST module.
  /// Diagnostics are emitted to the error stream.
  /// @returns true if it was successful.
  bool BuildAndParseInternalModuleExceptFunctions() {
    return BuildInternalModule() && ParseInternalModuleExceptFunctions();
  }

  /// @returns the set of SPIR-V IDs for imports of the "GLSL.std.450"
  /// extended instruction set.
  const std::unordered_set<uint32_t>& glsl_std_450_imports() const {
    return glsl_std_450_imports_;
  }

  /// Converts a SPIR-V type to a Tint type, and saves it for fast lookup.
  /// On failure, logs an error and returns null.  This should only be called
  /// after the internal representation of the module has been built.
  /// @param type_id the SPIR-V ID of a type.
  /// @returns a Tint type, or nullptr
  ast::type::Type* ConvertType(uint32_t type_id);

  /// @returns the fail stream object
  FailStream& fail_stream() { return fail_stream_; }
  /// @returns the namer object
  Namer& namer() { return namer_; }
  /// @returns a borrowed pointer to the internal representation of the module.
  /// This is null until BuildInternalModule has been called.
  spvtools::opt::IRContext* ir_context() { return ir_context_.get(); }

  /// Gets the list of decorations for a SPIR-V result ID.  Returns an empty
  /// vector if the ID is not a result ID, or if no decorations target that ID.
  /// The internal representation must have already been built.
  /// @param id SPIR-V ID
  /// @returns the list of decorations on the given ID
  DecorationList GetDecorationsFor(uint32_t id) const;
  /// Gets the list of decorations for the member of a struct.  Returns an empty
  /// list if the |id| is not the ID of a struct, or if the member index is out
  /// of range, or if the target member has no decorations.
  /// The internal representation must have already been built.
  /// @param id SPIR-V ID of a struct
  /// @param member_index the member within the struct
  /// @returns the list of decorations on the member
  DecorationList GetDecorationsForMember(uint32_t id,
                                         uint32_t member_index) const;

  /// Converts a SPIR-V decoration.  On failure, emits a diagnostic and returns
  /// nullptr.
  /// @param decoration an encoded SPIR-V Decoration
  /// @returns the corresponding ast::StructuMemberDecoration
  std::unique_ptr<ast::StructMemberDecoration> ConvertMemberDecoration(
      const Decoration& decoration);

  /// Builds the internal representation of the SPIR-V module.
  /// Assumes the module is somewhat well-formed.  Normally you
  /// would want to validate the SPIR-V module before attempting
  /// to build this internal representation.
  /// This is a no-op if the parser has already failed.
  /// @returns true if the parser is still successful.
  bool BuildInternalModule();

  /// Walks the internal representation of the module to populate
  /// the AST form of the module.
  /// This is a no-op if the parser has already failed.
  /// @returns true if the parser is still successful.
  bool ParseInternalModule();

  /// Walks the internal representation of the module, except for function
  /// definitions, to populate the AST form of the module.
  /// This is a no-op if the parser has already failed.
  /// @returns true if the parser is still successful.
  bool ParseInternalModuleExceptFunctions();

  /// Destroys the internal representation of the SPIR-V module.
  void ResetInternalModule();

  /// Registers extended instruction imports.  Only "GLSL.std.450" is supported.
  /// This is a no-op if the parser has already failed.
  /// @returns true if parser is still successful.
  bool RegisterExtendedInstructionImports();

  /// Registers user names for SPIR-V objects, from OpName, and OpMemberName.
  /// Also synthesizes struct field names.  Ensures uniqueness for names for
  /// SPIR-V IDs, and uniqueness of names of fields within any single struct.
  /// This is a no-op if the parser has already failed.
  /// @returns true if parser is still successful.
  bool RegisterUserAndStructMemberNames();

  /// Emit entry point AST nodes.
  /// This is a no-op if the parser has already failed.
  /// @returns true if parser is still successful.
  bool EmitEntryPoints();

  /// Register Tint AST types for SPIR-V types.
  /// This is a no-op if the parser has already failed.
  /// @returns true if parser is still successful.
  bool RegisterTypes();

  /// Emit type alias declarations for types requiring user-specified names:
  /// - struct types
  /// - decorated arrays and runtime arrays
  /// TODO(dneto): I expect images and samplers to require names as well.
  /// This is a no-op if the parser has already failed.
  /// @returns true if parser is still successful.
  bool EmitAliasTypes();

  /// Emits module-scope variables.
  /// This is a no-op if the parser has already failed.
  /// @returns true if parser is still successful.
  bool EmitModuleScopeVariables();

  /// Emits functions, with callees preceding their callers.
  /// This is a no-op if the parser has already failed.
  /// @returns true if parser is still successful.
  bool EmitFunctions();

  /// Emits a single function, if it has a body.
  /// This is a no-op if the parser has already failed.
  /// @param f the function to emit
  /// @returns true if parser is still successful.
  bool EmitFunction(const spvtools::opt::Function& f);

  /// Creates an AST Variable node for a SPIR-V ID, including any attached
  /// decorations.
  /// @param id the SPIR-V result ID
  /// @param sc the storage class, which cannot be ast::StorageClass::kNone
  /// @param type the type
  /// @returns a new Variable node, or null in the error case
  std::unique_ptr<ast::Variable> MakeVariable(uint32_t id,
                                              ast::StorageClass sc,
                                              ast::type::Type* type);

  /// Creates an AST expression node for a SPIR-V constant.
  /// @param id the SPIR-V ID of the constant
  /// @returns a new Literal node
  TypedExpression MakeConstantExpression(uint32_t id);

  /// Converts a given expression to the signedness demanded for an operand
  /// of the given SPIR-V opcode, if required.  If the operation assumes
  /// signed integer operands, and |expr| is unsigned, then return an
  /// as-cast expression converting it to signed. Otherwise, return
  /// |expr| itself.  Similarly, convert as required from unsigned
  /// to signed. Assumes all SPIR-V types have been mapped to AST types.
  /// @param op the SPIR-V opcode
  /// @param expr an expression
  /// @returns expr, or a cast of expr
  TypedExpression RectifyOperandSignedness(SpvOp op, TypedExpression&& expr);

  /// Returns the "forced" result type for the given SPIR-V opcode.
  /// If the WGSL result type for an operation has a more strict rule than
  /// requried by SPIR-V, then we say the result type is "forced".  This occurs
  /// for signed integer division (OpSDiv), for example, where the result type
  /// in WGSL must match the operand types.
  /// @param op the SPIR-V opcode
  /// @param first_operand_type the AST type for the first operand.
  /// @returns the forced AST result type, or nullptr if no forcing is required.
  ast::type::Type* ForcedResultType(SpvOp op,
                                    ast::type::Type* first_operand_type);

 private:
  /// Converts a specific SPIR-V type to a Tint type. Integer case
  ast::type::Type* ConvertType(const spvtools::opt::analysis::Integer* int_ty);
  /// Converts a specific SPIR-V type to a Tint type. Float case
  ast::type::Type* ConvertType(const spvtools::opt::analysis::Float* float_ty);
  /// Converts a specific SPIR-V type to a Tint type. Vector case
  ast::type::Type* ConvertType(const spvtools::opt::analysis::Vector* vec_ty);
  /// Converts a specific SPIR-V type to a Tint type. Matrix case
  ast::type::Type* ConvertType(const spvtools::opt::analysis::Matrix* mat_ty);
  /// Converts a specific SPIR-V type to a Tint type. RuntimeArray case
  ast::type::Type* ConvertType(
      const spvtools::opt::analysis::RuntimeArray* rtarr_ty);
  /// Converts a specific SPIR-V type to a Tint type. Array case
  ast::type::Type* ConvertType(const spvtools::opt::analysis::Array* arr_ty);
  /// Converts a specific SPIR-V type to a Tint type. Struct case
  ast::type::Type* ConvertType(
      const spvtools::opt::analysis::Struct* struct_ty);
  /// Converts a specific SPIR-V type to a Tint type. Pointer case
  ast::type::Type* ConvertType(const spvtools::opt::analysis::Pointer* ptr_ty);

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

  // An object used to store and generate names for SPIR-V objects.
  Namer namer_;
  // An object used to convert SPIR-V enums to Tint enums
  EnumConverter enum_converter_;

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

  // Maps a SPIR-V type ID to a Tint type.
  std::unordered_map<uint32_t, ast::type::Type*> id_to_type_;

  // Maps an unsigned type corresponding to the given signed type.
  std::unordered_map<ast::type::Type*, ast::type::Type*> signed_type_for_;
  // Maps an signed type corresponding to the given unsigned type.
  std::unordered_map<ast::type::Type*, ast::type::Type*> unsigned_type_for_;
};

}  // namespace spirv
}  // namespace reader
}  // namespace tint

#endif  // SRC_READER_SPIRV_PARSER_IMPL_H_
