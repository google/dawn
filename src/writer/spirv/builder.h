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

#ifndef SRC_WRITER_SPIRV_BUILDER_H_
#define SRC_WRITER_SPIRV_BUILDER_H_

#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "spirv/unified1/spirv.h"
#include "src/ast/assignment_statement.h"
#include "src/ast/bitcast_expression.h"
#include "src/ast/break_statement.h"
#include "src/ast/continue_statement.h"
#include "src/ast/discard_statement.h"
#include "src/ast/if_statement.h"
#include "src/ast/loop_statement.h"
#include "src/ast/return_statement.h"
#include "src/ast/switch_statement.h"
#include "src/ast/unary_op_expression.h"
#include "src/ast/variable_decl_statement.h"
#include "src/program_builder.h"
#include "src/scope_stack.h"
#include "src/sem/storage_texture_type.h"
#include "src/writer/spirv/function.h"
#include "src/writer/spirv/scalar_constant.h"

namespace tint {

// Forward declarations
namespace sem {
class Call;
class Reference;
}  // namespace sem

namespace writer {
namespace spirv {

/// Builder class to create SPIR-V instructions from a module.
class Builder {
 public:
  /// Contains information for generating accessor chains
  struct AccessorInfo {
    AccessorInfo();
    ~AccessorInfo();

    /// The ID of the current chain source. The chain source may change as we
    /// evaluate the access chain. The chain source always points to the ID
    /// which we will use to evaluate the current set of accessors. This maybe
    /// the original variable, or maybe an intermediary if we had to evaulate
    /// the access chain early (in the case of a swizzle of an access chain).
    uint32_t source_id;
    /// The type of the current chain source. This type matches the deduced
    /// result_type of the current source defined above.
    const sem::Type* source_type;
    /// A list of access chain indices to emit. Note, we _only_ have access
    /// chain indices if the source is reference.
    std::vector<uint32_t> access_chain_indices;
  };

  /// Constructor
  /// @param program the program
  explicit Builder(const Program* program);
  ~Builder();

  /// Generates the SPIR-V instructions for the given program
  /// @returns true if the SPIR-V was successfully built
  bool Build();

  /// @returns the error string or blank if no error was reported.
  const std::string& error() const { return error_; }
  /// @returns true if the builder encountered an error
  bool has_error() const { return !error_.empty(); }

  /// @returns the number of uint32_t's needed to make up the results
  uint32_t total_size() const;

  /// @returns the id bound for this program
  uint32_t id_bound() const { return next_id_; }

  /// @returns the next id to be used
  uint32_t next_id() {
    auto id = next_id_;
    next_id_ += 1;
    return id;
  }

  /// Iterates over all the instructions in the correct order and calls the
  /// given callback
  /// @param cb the callback to execute
  void iterate(std::function<void(const Instruction&)> cb) const;

  /// Adds an instruction to the list of capabilities, if the capability
  /// hasn't already been added.
  /// @param cap the capability to set
  void push_capability(uint32_t cap);
  /// @returns the capabilities
  const InstructionList& capabilities() const { return capabilities_; }
  /// Adds an instruction to the extensions
  /// @param op the op to set
  /// @param operands the operands for the instruction
  void push_extension(spv::Op op, const OperandList& operands) {
    extensions_.push_back(Instruction{op, operands});
  }
  /// @returns the extensions
  const InstructionList& extensions() const { return extensions_; }
  /// Adds an instruction to the ext import
  /// @param op the op to set
  /// @param operands the operands for the instruction
  void push_ext_import(spv::Op op, const OperandList& operands) {
    ext_imports_.push_back(Instruction{op, operands});
  }
  /// @returns the ext imports
  const InstructionList& ext_imports() const { return ext_imports_; }
  /// Adds an instruction to the memory model
  /// @param op the op to set
  /// @param operands the operands for the instruction
  void push_memory_model(spv::Op op, const OperandList& operands) {
    memory_model_.push_back(Instruction{op, operands});
  }
  /// @returns the memory model
  const InstructionList& memory_model() const { return memory_model_; }
  /// Adds an instruction to the entry points
  /// @param op the op to set
  /// @param operands the operands for the instruction
  void push_entry_point(spv::Op op, const OperandList& operands) {
    entry_points_.push_back(Instruction{op, operands});
  }
  /// @returns the entry points
  const InstructionList& entry_points() const { return entry_points_; }
  /// Adds an instruction to the execution modes
  /// @param op the op to set
  /// @param operands the operands for the instruction
  void push_execution_mode(spv::Op op, const OperandList& operands) {
    execution_modes_.push_back(Instruction{op, operands});
  }
  /// @returns the execution modes
  const InstructionList& execution_modes() const { return execution_modes_; }
  /// Adds an instruction to the debug
  /// @param op the op to set
  /// @param operands the operands for the instruction
  void push_debug(spv::Op op, const OperandList& operands) {
    debug_.push_back(Instruction{op, operands});
  }
  /// @returns the debug instructions
  const InstructionList& debug() const { return debug_; }
  /// Adds an instruction to the types
  /// @param op the op to set
  /// @param operands the operands for the instruction
  void push_type(spv::Op op, const OperandList& operands) {
    types_.push_back(Instruction{op, operands});
  }
  /// @returns the type instructions
  const InstructionList& types() const { return types_; }
  /// Adds an instruction to the annotations
  /// @param op the op to set
  /// @param operands the operands for the instruction
  void push_annot(spv::Op op, const OperandList& operands) {
    annotations_.push_back(Instruction{op, operands});
  }
  /// @returns the annotations
  const InstructionList& annots() const { return annotations_; }

  /// Adds a function to the builder
  /// @param func the function to add
  void push_function(const Function& func) {
    functions_.push_back(func);
    current_label_id_ = func.label_id();
  }
  /// @returns the functions
  const std::vector<Function>& functions() const { return functions_; }
  /// Pushes an instruction to the current function. If we're outside
  /// a function then issue an internal error and return false.
  /// @param op the operation
  /// @param operands the operands
  /// @returns true if we succeeded
  bool push_function_inst(spv::Op op, const OperandList& operands);
  /// Pushes a variable to the current function
  /// @param operands the variable operands
  void push_function_var(const OperandList& operands) {
    if (functions_.empty()) {
      TINT_ICE(builder_.Diagnostics())
          << "push_function_var() called without a function";
    }
    functions_.back().push_var(operands);
  }

  /// Converts a storage class to a SPIR-V storage class.
  /// @param klass the storage class to convert
  /// @returns the SPIR-V storage class or SpvStorageClassMax on error.
  SpvStorageClass ConvertStorageClass(ast::StorageClass klass) const;
  /// Converts a builtin to a SPIR-V builtin and pushes a capability if needed.
  /// @param builtin the builtin to convert
  /// @param storage the storage class that this builtin is being used with
  /// @returns the SPIR-V builtin or SpvBuiltInMax on error.
  SpvBuiltIn ConvertBuiltin(ast::Builtin builtin, ast::StorageClass storage);

  /// Generates a label for the given id. Emits an error and returns false if
  /// we're currently outside a function.
  /// @param id the id to use for the label
  /// @returns true on success.
  bool GenerateLabel(uint32_t id);
  /// Generates an assignment statement
  /// @param assign the statement to generate
  /// @returns true if the statement was successfully generated
  bool GenerateAssignStatement(ast::AssignmentStatement* assign);
  /// Generates a block statement, wrapped in a push/pop scope
  /// @param stmt the statement to generate
  /// @returns true if the statement was successfully generated
  bool GenerateBlockStatement(const ast::BlockStatement* stmt);
  /// Generates a block statement
  /// @param stmt the statement to generate
  /// @returns true if the statement was successfully generated
  bool GenerateBlockStatementWithoutScoping(const ast::BlockStatement* stmt);
  /// Generates a break statement
  /// @param stmt the statement to generate
  /// @returns true if the statement was successfully generated
  bool GenerateBreakStatement(ast::BreakStatement* stmt);
  /// Generates a continue statement
  /// @param stmt the statement to generate
  /// @returns true if the statement was successfully generated
  bool GenerateContinueStatement(ast::ContinueStatement* stmt);
  /// Generates a discard statement
  /// @param stmt the statement to generate
  /// @returns true if the statement was successfully generated
  bool GenerateDiscardStatement(ast::DiscardStatement* stmt);
  /// Generates an entry point instruction
  /// @param func the function
  /// @param id the id of the function
  /// @returns true if the instruction was generated, false otherwise
  bool GenerateEntryPoint(ast::Function* func, uint32_t id);
  /// Generates execution modes for an entry point
  /// @param func the function
  /// @param id the id of the function
  /// @returns false on failure
  bool GenerateExecutionModes(ast::Function* func, uint32_t id);
  /// Generates an expression
  /// @param expr the expression to generate
  /// @returns the resulting ID of the expression or 0 on error
  uint32_t GenerateExpression(ast::Expression* expr);
  /// Generates the instructions for a function
  /// @param func the function to generate
  /// @returns true if the instructions were generated
  bool GenerateFunction(ast::Function* func);
  /// Generates a function type if not already created
  /// @param func the function to generate for
  /// @returns the ID to use for the function type. Returns 0 on failure.
  uint32_t GenerateFunctionTypeIfNeeded(const sem::Function* func);
  /// Generates access control annotations if needed
  /// @param type the type to generate for
  /// @param struct_id the struct id
  /// @param member_idx the member index
  void GenerateMemberAccessControlIfNeeded(const sem::Type* type,
                                           uint32_t struct_id,
                                           uint32_t member_idx);
  /// Generates a function variable
  /// @param var the variable
  /// @returns true if the variable was generated
  bool GenerateFunctionVariable(ast::Variable* var);
  /// Generates a global variable
  /// @param var the variable to generate
  /// @returns true if the variable is emited.
  bool GenerateGlobalVariable(ast::Variable* var);
  /// Generates an array accessor expression.
  ///
  /// For more information on accessors see the "Pointer evaluation" section of
  /// the WGSL specification.
  ///
  /// @param expr the expresssion to generate
  /// @returns the id of the expression or 0 on failure
  uint32_t GenerateAccessorExpression(ast::Expression* expr);
  /// Generates an array accessor
  /// @param expr the accessor to generate
  /// @param info the current accessor information
  /// @returns true if the accessor was generated successfully
  bool GenerateArrayAccessor(ast::ArrayAccessorExpression* expr,
                             AccessorInfo* info);
  /// Generates a member accessor
  /// @param expr the accessor to generate
  /// @param info the current accessor information
  /// @returns true if the accessor was generated successfully
  bool GenerateMemberAccessor(ast::MemberAccessorExpression* expr,
                              AccessorInfo* info);
  /// Generates an identifier expression
  /// @param expr the expresssion to generate
  /// @returns the id of the expression or 0 on failure
  uint32_t GenerateIdentifierExpression(ast::IdentifierExpression* expr);
  /// Generates a unary op expression
  /// @param expr the expression to generate
  /// @returns the id of the expression or 0 on failure
  uint32_t GenerateUnaryOpExpression(ast::UnaryOpExpression* expr);
  /// Generates an if statement
  /// @param stmt the statement to generate
  /// @returns true on success
  bool GenerateIfStatement(ast::IfStatement* stmt);
  /// Generates an import instruction
  void GenerateGLSLstd450Import();
  /// Generates a constructor expression
  /// @param var the variable generated for, nullptr if no variable associated.
  /// @param expr the expression to generate
  /// @param is_global_init set true if this is a global variable constructor
  /// @returns the ID of the expression or 0 on failure.
  uint32_t GenerateConstructorExpression(ast::Variable* var,
                                         ast::ConstructorExpression* expr,
                                         bool is_global_init);
  /// Generates a type constructor expression
  /// @param init the expression to generate
  /// @param is_global_init set true if this is a global variable constructor
  /// @returns the ID of the expression or 0 on failure.
  uint32_t GenerateTypeConstructorExpression(
      ast::TypeConstructorExpression* init,
      bool is_global_init);
  /// Generates a literal constant if needed
  /// @param var the variable generated for, nullptr if no variable associated.
  /// @param lit the literal to generate
  /// @returns the ID on success or 0 on failure
  uint32_t GenerateLiteralIfNeeded(ast::Variable* var, ast::Literal* lit);
  /// Generates a binary expression
  /// @param expr the expression to generate
  /// @returns the expression ID on success or 0 otherwise
  uint32_t GenerateBinaryExpression(ast::BinaryExpression* expr);
  /// Generates a bitcast expression
  /// @param expr the expression to generate
  /// @returns the expression ID on success or 0 otherwise
  uint32_t GenerateBitcastExpression(ast::BitcastExpression* expr);
  /// Generates a short circuting binary expression
  /// @param expr the expression to generate
  /// @returns teh expression ID on success or 0 otherwise
  uint32_t GenerateShortCircuitBinaryExpression(ast::BinaryExpression* expr);
  /// Generates a call expression
  /// @param expr the expression to generate
  /// @returns the expression ID on success or 0 otherwise
  uint32_t GenerateCallExpression(ast::CallExpression* expr);
  /// Generates an intrinsic call
  /// @param call the call expression
  /// @param intrinsic the semantic information for the intrinsic
  /// @returns the expression ID on success or 0 otherwise
  uint32_t GenerateIntrinsic(ast::CallExpression* call,
                             const sem::Intrinsic* intrinsic);
  /// Generates a texture intrinsic call. Emits an error and returns false if
  /// we're currently outside a function.
  /// @param call the call expression
  /// @param intrinsic the semantic information for the texture intrinsic
  /// @param result_type result type operand of the texture instruction
  /// @param result_id result identifier operand of the texture instruction
  /// parameters
  /// @returns true on success
  bool GenerateTextureIntrinsic(ast::CallExpression* call,
                                const sem::Intrinsic* intrinsic,
                                spirv::Operand result_type,
                                spirv::Operand result_id);
  /// Generates a control barrier statement.
  /// @param intrinsic the semantic information for the barrier intrinsic
  /// parameters
  /// @returns true on success
  bool GenerateControlBarrierIntrinsic(const sem::Intrinsic* intrinsic);
  /// Generates a sampled image
  /// @param texture_type the texture type
  /// @param texture_operand the texture operand
  /// @param sampler_operand the sampler operand
  /// @returns the expression ID
  uint32_t GenerateSampledImage(const sem::Type* texture_type,
                                Operand texture_operand,
                                Operand sampler_operand);
  /// Generates a cast or object copy for the expression result,
  /// or return the ID generated the expression if it is already
  /// of the right type.
  /// @param to_type the type we're casting too
  /// @param from_expr the expression to cast
  /// @returns the expression ID on success or 0 otherwise
  uint32_t GenerateCastOrCopyOrPassthrough(const sem::Type* to_type,
                                           ast::Expression* from_expr);
  /// Generates a loop statement
  /// @param stmt the statement to generate
  /// @returns true on successful generation
  bool GenerateLoopStatement(ast::LoopStatement* stmt);
  /// Generates a return statement
  /// @param stmt the statement to generate
  /// @returns true on success, false otherwise
  bool GenerateReturnStatement(ast::ReturnStatement* stmt);
  /// Generates a switch statement
  /// @param stmt the statement to generate
  /// @returns ture on success, false otherwise
  bool GenerateSwitchStatement(ast::SwitchStatement* stmt);
  /// Generates a conditional section merge block
  /// @param cond the condition
  /// @param true_body the statements making up the true block
  /// @param cur_else_idx the index of the current else statement to process
  /// @param else_stmts the list of all else statements
  /// @returns true on success, false on failure
  bool GenerateConditionalBlock(ast::Expression* cond,
                                const ast::BlockStatement* true_body,
                                size_t cur_else_idx,
                                const ast::ElseStatementList& else_stmts);
  /// Generates a statement
  /// @param stmt the statement to generate
  /// @returns true if the statement was generated
  bool GenerateStatement(ast::Statement* stmt);
  /// Geneates an OpLoad
  /// @param type the type to load
  /// @param id the variable id to load
  /// @returns the ID of the loaded value or `id` if type is not a reference
  uint32_t GenerateLoadIfNeeded(const sem::Type* type, uint32_t id);
  /// Generates an OpStore. Emits an error and returns false if we're
  /// currently outside a function.
  /// @param to the ID to store too
  /// @param from the ID to store from
  /// @returns true on success
  bool GenerateStore(uint32_t to, uint32_t from);
  /// Generates a type if not already created
  /// @param type the type to create
  /// @returns the ID to use for the given type. Returns 0 on unknown type.
  uint32_t GenerateTypeIfNeeded(const sem::Type* type);
  /// Generates a texture type declaration
  /// @param texture the texture to generate
  /// @param result the result operand
  /// @returns true if the texture was successfully generated
  bool GenerateTextureType(const sem::Texture* texture, const Operand& result);
  /// Generates an array type declaration
  /// @param ary the array to generate
  /// @param result the result operand
  /// @returns true if the array was successfully generated
  bool GenerateArrayType(const sem::Array* ary, const Operand& result);
  /// Generates a matrix type declaration
  /// @param mat the matrix to generate
  /// @param result the result operand
  /// @returns true if the matrix was successfully generated
  bool GenerateMatrixType(const sem::Matrix* mat, const Operand& result);
  /// Generates a pointer type declaration
  /// @param ptr the pointer type to generate
  /// @param result the result operand
  /// @returns true if the pointer was successfully generated
  bool GeneratePointerType(const sem::Pointer* ptr, const Operand& result);
  /// Generates a reference type declaration
  /// @param ref the reference type to generate
  /// @param result the result operand
  /// @returns true if the reference was successfully generated
  bool GenerateReferenceType(const sem::Reference* ref, const Operand& result);
  /// Generates a vector type declaration
  /// @param struct_type the vector to generate
  /// @param result the result operand
  /// @returns true if the vector was successfully generated
  bool GenerateStructType(const sem::Struct* struct_type,
                          const Operand& result);
  /// Generates a struct member
  /// @param struct_id the id of the parent structure
  /// @param idx the index of the member
  /// @param member the member to generate
  /// @returns the id of the struct member or 0 on error.
  uint32_t GenerateStructMember(uint32_t struct_id,
                                uint32_t idx,
                                ast::StructMember* member);
  /// Generates a variable declaration statement
  /// @param stmt the statement to generate
  /// @returns true on successfull generation
  bool GenerateVariableDeclStatement(ast::VariableDeclStatement* stmt);
  /// Generates a vector type declaration
  /// @param vec the vector to generate
  /// @param result the result operand
  /// @returns true if the vector was successfully generated
  bool GenerateVectorType(const sem::Vector* vec, const Operand& result);

  /// Converts AST image format to SPIR-V and pushes an appropriate capability.
  /// @param format AST image format type
  /// @returns SPIR-V image format type
  SpvImageFormat convert_image_format_to_spv(const ast::ImageFormat format);

  /// Determines if the given type constructor is created from constant values
  /// @param expr the expression to check
  /// @param is_global_init if this is a global initializer
  /// @returns true if the constructor is constant
  bool is_constructor_const(ast::Expression* expr, bool is_global_init);

 private:
  /// @returns an Operand with a new result ID in it. Increments the next_id_
  /// automatically.
  Operand result_op();

  /// @returns the resolved type of the ast::Expression `expr`
  /// @param expr the expression
  const sem::Type* TypeOf(ast::Expression* expr) const {
    return builder_.TypeOf(expr);
  }

  /// Generates a constant if needed
  /// @param constant the constant to generate.
  /// @returns the ID on success or 0 on failure
  uint32_t GenerateConstantIfNeeded(const ScalarConstant& constant);

  /// Generates a constant-null of the given type, if needed
  /// @param type the type of the constant null to generate.
  /// @returns the ID on success or 0 on failure
  uint32_t GenerateConstantNullIfNeeded(const sem::Type* type);

  ProgramBuilder builder_;
  std::string error_;
  uint32_t next_id_ = 1;
  uint32_t current_label_id_ = 0;
  InstructionList capabilities_;
  InstructionList extensions_;
  InstructionList ext_imports_;
  InstructionList memory_model_;
  InstructionList entry_points_;
  InstructionList execution_modes_;
  InstructionList debug_;
  InstructionList types_;
  InstructionList annotations_;
  std::vector<Function> functions_;

  std::unordered_map<std::string, uint32_t> import_name_to_id_;
  std::unordered_map<Symbol, uint32_t> func_symbol_to_id_;
  std::unordered_map<std::string, uint32_t> type_name_to_id_;
  std::unordered_map<ScalarConstant, uint32_t> const_to_id_;
  std::unordered_map<std::string, uint32_t> type_constructor_to_id_;
  std::unordered_map<std::string, uint32_t> const_null_to_id_;
  std::unordered_map<std::string, uint32_t>
      texture_type_name_to_sampled_image_type_id_;
  ScopeStack<uint32_t> scope_stack_;
  std::unordered_map<uint32_t, ast::Variable*> spirv_id_to_variable_;
  std::vector<uint32_t> merge_stack_;
  std::vector<uint32_t> continue_stack_;
  std::unordered_set<uint32_t> capability_set_;
  bool has_overridable_workgroup_size_ = false;
};

}  // namespace spirv
}  // namespace writer
}  // namespace tint

#endif  // SRC_WRITER_SPIRV_BUILDER_H_
