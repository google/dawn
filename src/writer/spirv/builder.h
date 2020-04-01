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

#include <functional>
#include <string>
#include <unordered_map>
#include <vector>

#include "spirv/unified1/spirv.h"
#include "src/ast/builtin.h"
#include "src/ast/literal.h"
#include "src/ast/module.h"
#include "src/ast/struct_member.h"
#include "src/writer/spirv/function.h"
#include "src/writer/spirv/instruction.h"

namespace tint {
namespace writer {
namespace spirv {

/// Builder class to create SPIR-V instructions from a module.
class Builder {
 public:
  /// Constructor
  Builder();
  ~Builder();

  /// Generates the SPIR-V instructions for the given module
  /// @param module the module to generate from
  /// @returns true if the SPIR-V was successfully built
  bool Build(const ast::Module& module);

  /// @returns the error string or blank if no error was reported.
  const std::string& error() const { return error_; }
  /// @returns true if the builder encountered an error
  bool has_error() const { return !error_.empty(); }

  /// @returns the number of uint32_t's needed to make up the results
  uint32_t total_size() const;

  /// @returns the id bound for this module
  uint32_t id_bound() const { return next_id_; }

  /// @returns the next id to be used
  uint32_t next_id() {
    auto id = next_id_;
    next_id_ += 1;
    return id;
  }

  /// Sets the id for a given function name
  /// @param name the name to set
  /// @param id the id to set
  void set_func_name_to_id(const std::string& name, uint32_t id) {
    func_name_to_id_[name] = id;
  }

  /// Retrives the id for the given function name
  /// @param name the function name to search for
  /// @returns the id for the given name or 0 on failure
  uint32_t id_for_func_name(const std::string& name) {
    if (func_name_to_id_.count(name) == 0) {
      return 0;
    }
    return func_name_to_id_[name];
  }

  /// Iterates over all the instructions in the correct order and calls the
  /// given callback
  /// @param cb the callback to execute
  void iterate(std::function<void(const Instruction&)> cb) const;

  /// Adds an instruction to the preamble
  /// @param op the op to set
  /// @param operands the operands for the instruction
  void push_preamble(spv::Op op, const std::vector<Operand>& operands) {
    preamble_.push_back(Instruction{op, operands});
  }
  /// @returns the preamble
  const std::vector<Instruction>& preamble() const { return preamble_; }
  /// Adds an instruction to the debug
  /// @param op the op to set
  /// @param operands the operands for the instruction
  void push_debug(spv::Op op, const std::vector<Operand>& operands) {
    debug_.push_back(Instruction{op, operands});
  }
  /// @returns the debug instructions
  const std::vector<Instruction>& debug() const { return debug_; }
  /// Adds an instruction to the types
  /// @param op the op to set
  /// @param operands the operands for the instruction
  void push_type(spv::Op op, const std::vector<Operand>& operands) {
    types_.push_back(Instruction{op, operands});
  }
  /// @returns the type instructions
  const std::vector<Instruction>& types() const { return types_; }
  /// Adds an instruction to the annotations
  /// @param op the op to set
  /// @param operands the operands for the instruction
  void push_annot(spv::Op op, const std::vector<Operand>& operands) {
    annotations_.push_back(Instruction{op, operands});
  }
  /// @returns the annotations
  const std::vector<Instruction>& annots() const { return annotations_; }

  /// Adds a function to the builder
  /// @param func the function to add
  void push_function(const Function& func) { functions_.push_back(func); }
  /// @returns the functions
  const std::vector<Function>& functions() const { return functions_; }
  /// Pushes an instruction to the current function
  /// @param op the operation
  /// @param operands the operands
  void push_function_inst(spv::Op op, const std::vector<Operand>& operands) {
    functions_.back().push_inst(op, operands);
  }
  /// Pushes a variable to the current function
  /// @param operands the variable operands
  void push_function_var(const std::vector<Operand>& operands) {
    functions_.back().push_var(operands);
  }

  /// Converts a storage class to a SPIR-V storage class.
  /// @param klass the storage class to convert
  /// @returns the SPIR-V storage class or SpvStorageClassMax on error.
  SpvStorageClass ConvertStorageClass(ast::StorageClass klass) const;
  /// Converts a builtin to a SPIR-V builtin
  /// @param builtin the builtin to convert
  /// @returns the SPIR-V builtin or SpvBuiltInMax on error.
  SpvBuiltIn ConvertBuiltin(ast::Builtin builtin) const;

  /// Generates an entry point instruction
  /// @param ep the entry point
  /// @returns true if the instruction was generated, false otherwise
  bool GenerateEntryPoint(ast::EntryPoint* ep);
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
  uint32_t GenerateFunctionTypeIfNeeded(ast::Function* func);
  /// Generates a function variable
  /// @param var the variable
  /// @returns true if the variable was generated
  bool GenerateFunctionVariable(ast::Variable* var);
  /// Generates a global variable
  /// @param var the variable to generate
  /// @returns true if the variable is emited.
  bool GenerateGlobalVariable(ast::Variable* var);
  /// Generates an import instruction
  /// @param imp the import
  void GenerateImport(ast::Import* imp);
  /// Generates an constructor expression
  /// @param expr the expression to generate
  /// @param is_global_init set true if this is a global variable constructor
  /// @returns the ID of the expression or 0 on failure.
  uint32_t GenerateConstructorExpression(ast::ConstructorExpression* expr,
                                         bool is_global_init);
  /// Generates a literal constant if needed
  /// @param lit the literal to generate
  /// @returns the ID on success or 0 on failure
  uint32_t GenerateLiteralIfNeeded(ast::Literal* lit);
  /// Generates a return statement
  /// @param stmt the statement to generate
  /// @returns true on success, false otherwise
  bool GenerateReturnStatement(ast::ReturnStatement* stmt);
  /// Generates a statement
  /// @param stmt the statement to generate
  /// @returns true if the statement was generated
  bool GenerateStatement(ast::Statement* stmt);
  /// Generates a type if not already created
  /// @param type the type to create
  /// @returns the ID to use for the given type. Returns 0 on unknown type.
  uint32_t GenerateTypeIfNeeded(ast::type::Type* type);
  /// Generates an array type declaration
  /// @param ary the array to generate
  /// @param result the result operand
  /// @returns true if the array was successfully generated
  bool GenerateArrayType(ast::type::ArrayType* ary, const Operand& result);
  /// Generates a matrix type declaration
  /// @param mat the matrix to generate
  /// @param result the result operand
  /// @returns true if the matrix was successfully generated
  bool GenerateMatrixType(ast::type::MatrixType* mat, const Operand& result);
  /// Generates a pointer type declaration
  /// @param ptr the pointer type to generate
  /// @param result the result operand
  /// @returns true if the pointer was successfully generated
  bool GeneratePointerType(ast::type::PointerType* ptr, const Operand& result);
  /// Generates a vector type declaration
  /// @param struct_type the vector to generate
  /// @param result the result operand
  /// @returns true if the vector was successfully generated
  bool GenerateStructType(ast::type::StructType* struct_type,
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
  bool GenerateVectorType(ast::type::VectorType* vec, const Operand& result);

 private:
  /// @returns an Operand with a new result ID in it. Increments the next_id_
  /// automatically.
  Operand result_op();

  std::string error_;
  uint32_t next_id_ = 1;
  std::vector<Instruction> preamble_;
  std::vector<Instruction> debug_;
  std::vector<Instruction> types_;
  std::vector<Instruction> annotations_;
  std::vector<Function> functions_;

  std::unordered_map<std::string, uint32_t> import_name_to_id_;
  std::unordered_map<std::string, uint32_t> func_name_to_id_;
  std::unordered_map<std::string, uint32_t> type_name_to_id_;
  std::unordered_map<std::string, uint32_t> const_to_id_;
};

}  // namespace spirv
}  // namespace writer
}  // namespace tint

#endif  // SRC_WRITER_SPIRV_BUILDER_H_
