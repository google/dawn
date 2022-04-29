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

#ifndef SRC_TINT_RESOLVER_VALIDATOR_H_
#define SRC_TINT_RESOLVER_VALIDATOR_H_

#include <set>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

#include "src/tint/ast/pipeline_stage.h"
#include "src/tint/program_builder.h"
#include "src/tint/resolver/sem_helper.h"
#include "src/tint/source.h"

// Forward declarations
namespace tint::ast {
class IndexAccessorExpression;
class BinaryExpression;
class BitcastExpression;
class CallExpression;
class CallStatement;
class CaseStatement;
class ForLoopStatement;
class Function;
class IdentifierExpression;
class LoopStatement;
class MemberAccessorExpression;
class ReturnStatement;
class SwitchStatement;
class UnaryOpExpression;
class Variable;
}  // namespace tint::ast
namespace tint::sem {
class Array;
class Atomic;
class BlockStatement;
class Builtin;
class CaseStatement;
class ForLoopStatement;
class IfStatement;
class LoopStatement;
class Statement;
class SwitchStatement;
class TypeConstructor;
}  // namespace tint::sem

namespace tint::resolver {

/// Validation logic for various ast nodes. The validations in general should
/// be shallow and depend on the resolver to call on children. The validations
/// also assume that sem changes have already been made. The validation checks
/// should not alter the AST or SEM trees.
class Validator {
 public:
  /// The valid type storage layouts typedef
  using ValidTypeStorageLayouts =
      std::set<std::pair<const sem::Type*, ast::StorageClass>>;

  /// Constructor
  /// @param builder the program builder
  /// @param helper the SEM helper to validate with
  Validator(ProgramBuilder* builder, SemHelper& helper);
  ~Validator();

  /// Adds the given error message to the diagnostics
  /// @param msg the error message
  /// @param source the error source
  void AddError(const std::string& msg, const Source& source) const;

  /// Adds the given warning message to the diagnostics
  /// @param msg the warning message
  /// @param source the warning source
  void AddWarning(const std::string& msg, const Source& source) const;

  /// Adds the given note message to the diagnostics
  /// @param msg the note message
  /// @param source the note source
  void AddNote(const std::string& msg, const Source& source) const;

  /// @param type the given type
  /// @returns true if the given type is a plain type
  bool IsPlain(const sem::Type* type) const;

  /// @param type the given type
  /// @returns true if the given type is a fixed-footprint type
  bool IsFixedFootprint(const sem::Type* type) const;

  /// @param type the given type
  /// @returns true if the given type is storable
  bool IsStorable(const sem::Type* type) const;

  /// @param type the given type
  /// @returns true if the given type is host-shareable
  bool IsHostShareable(const sem::Type* type) const;

  /// Validates pipeline stages
  /// @param entry_points the entry points to the module
  /// @returns true on success, false otherwise.
  bool PipelineStages(const std::vector<sem::Function*>& entry_points) const;

  /// Validates aliases
  /// @param alias the alias to validate
  /// @returns true on success, false otherwise.
  bool Alias(const ast::Alias* alias) const;

  /// Validates the array
  /// @param arr the array to validate
  /// @param source the source of the array
  /// @returns true on success, false otherwise.
  bool Array(const sem::Array* arr, const Source& source) const;

  /// Validates an array stride attribute
  /// @param attr the stride attribute to validate
  /// @param el_size the element size
  /// @param el_align the element alignment
  /// @param source the source of the attribute
  /// @returns true on success, false otherwise
  bool ArrayStrideAttribute(const ast::StrideAttribute* attr,
                            uint32_t el_size,
                            uint32_t el_align,
                            const Source& source) const;

  /// Validates an atomic
  /// @param a the atomic ast node to validate
  /// @param s the atomic sem node
  /// @returns true on success, false otherwise.
  bool Atomic(const ast::Atomic* a, const sem::Atomic* s) const;

  /// Validates an atoic variable
  /// @param var the variable to validate
  /// @param atomic_composite_info store atomic information
  /// @returns true on success, false otherwise.
  bool AtomicVariable(const sem::Variable* var,
                      std::unordered_map<const sem::Type*, const Source&>
                          atomic_composite_info) const;

  /// Validates an assignment
  /// @param a the assignment statement
  /// @param rhs_ty the type of the right hand side
  /// @returns true on success, false otherwise.
  bool Assignment(const ast::Statement* a, const sem::Type* rhs_ty) const;

  /// Validates a bitcase
  /// @param cast the bitcast expression
  /// @param to the destination type
  /// @returns true on success, false otherwise
  bool Bitcast(const ast::BitcastExpression* cast, const sem::Type* to) const;

  /// Validates a break statement
  /// @param stmt the break statement to validate
  /// @param current_statement the current statement being resolved
  /// @returns true on success, false otherwise.
  bool BreakStatement(const sem::Statement* stmt,
                      sem::Statement* current_statement) const;

  /// Validates a builtin attribute
  /// @param attr the attribute to validate
  /// @param storage_type the attribute storage type
  /// @param stage the current pipeline stage
  /// @param is_input true if this is an input attribute
  /// @returns true on success, false otherwise.
  bool BuiltinAttribute(const ast::BuiltinAttribute* attr,
                        const sem::Type* storage_type,
                        ast::PipelineStage stage,
                        const bool is_input) const;

  /// Validates a continue statement
  /// @param stmt the continue statement to validate
  /// @param current_statement the current statement being resolved
  /// @returns true on success, false otherwise
  bool ContinueStatement(const sem::Statement* stmt,
                         sem::Statement* current_statement) const;

  /// Validates a discard statement
  /// @param stmt the statement to validate
  /// @param current_statement the current statement being resolved
  /// @returns true on success, false otherwise
  bool DiscardStatement(const sem::Statement* stmt,
                        sem::Statement* current_statement) const;

  /// Validates an entry point
  /// @param func the entry point function to validate
  /// @param stage the pipeline stage for the entry point
  /// @returns true on success, false otherwise
  bool EntryPoint(const sem::Function* func, ast::PipelineStage stage) const;

  /// Validates a for loop
  /// @param stmt the for loop statement to validate
  /// @returns true on success, false otherwise
  bool ForLoopStatement(const sem::ForLoopStatement* stmt) const;

  /// Validates a fallthrough statement
  /// @param stmt the fallthrough to validate
  /// @returns true on success, false otherwise
  bool FallthroughStatement(const sem::Statement* stmt) const;

  /// Validates a function
  /// @param func the function to validate
  /// @param stage the current pipeline stage
  /// @returns true on success, false otherwise.
  bool Function(const sem::Function* func, ast::PipelineStage stage) const;

  /// Validates a function call
  /// @param call the function call to validate
  /// @param current_statement the current statement being resolved
  /// @returns true on success, false otherwise
  bool FunctionCall(const sem::Call* call,
                    sem::Statement* current_statement) const;

  /// Validates a global variable
  /// @param var the global variable to validate
  /// @param constant_ids the set of constant ids in the module
  /// @param atomic_composite_info atomic composite info in the module
  /// @returns true on success, false otherwise
  bool GlobalVariable(
      const sem::Variable* var,
      std::unordered_map<uint32_t, const sem::Variable*> constant_ids,
      std::unordered_map<const sem::Type*, const Source&> atomic_composite_info)
      const;

  /// Validates an if statement
  /// @param stmt the statement to validate
  /// @returns true on success, false otherwise
  bool IfStatement(const sem::IfStatement* stmt) const;

  /// Validates an increment or decrement statement
  /// @param stmt the statement to validate
  /// @returns true on success, false otherwise
  bool IncrementDecrementStatement(
      const ast::IncrementDecrementStatement* stmt) const;

  /// Validates an interpolate attribute
  /// @param attr the interpolation attribute to validate
  /// @param storage_type the storage type of the attached variable
  /// @returns true on succes, false otherwise
  bool InterpolateAttribute(const ast::InterpolateAttribute* attr,
                            const sem::Type* storage_type) const;

  /// Validates a builtin call
  /// @param call the builtin call to validate
  /// @returns true on success, false otherwise.
  bool BuiltinCall(const sem::Call* call) const;

  /// Validates a location attribute
  /// @param location the location attribute to validate
  /// @param type the variable type
  /// @param locations the set of locations in the module
  /// @param stage the current pipeline stage
  /// @param source the source of the attribute
  /// @param is_input true if this is an input variable
  /// @returns true on success, false otherwise.
  bool LocationAttribute(const ast::LocationAttribute* location,
                         const sem::Type* type,
                         std::unordered_set<uint32_t>& locations,
                         ast::PipelineStage stage,
                         const Source& source,
                         const bool is_input = false) const;

  /// Validates a loop statement
  /// @param stmt the loop statement
  /// @returns true on success, false otherwise.
  bool LoopStatement(const sem::LoopStatement* stmt) const;

  /// Validates a matrix
  /// @param ty the matrix to validate
  /// @param source the source of the matrix
  /// @returns true on success, false otherwise
  bool Matrix(const sem::Matrix* ty, const Source& source) const;

  /// Validates a function parameter
  /// @param func the function the variable is for
  /// @param var the variable to validate
  /// @returns true on success, false otherwise
  bool FunctionParameter(const ast::Function* func,
                         const sem::Variable* var) const;

  /// Validates a return
  /// @param ret the return statement to validate
  /// @param func_type the return type of the curreunt function
  /// @param ret_type the return type
  /// @param current_statement the current statement being resolved
  /// @returns true on success, false otherwise
  bool Return(const ast::ReturnStatement* ret,
              const sem::Type* func_type,
              const sem::Type* ret_type,
              sem::Statement* current_statement) const;

  /// Validates a list of statements
  /// @param stmts the statements to validate
  /// @returns true on success, false otherwise
  bool Statements(const ast::StatementList& stmts) const;

  /// Validates a storage texture
  /// @param t the texture to validate
  /// @returns true on success, false otherwise
  bool StorageTexture(const ast::StorageTexture* t) const;

  /// Validates a structure
  /// @param str the structure to validate
  /// @param stage the current pipeline stage
  /// @returns true on success, false otherwise.
  bool Structure(const sem::Struct* str, ast::PipelineStage stage) const;

  /// Validates a structure constructor or cast
  /// @param ctor the call expression to validate
  /// @param struct_type the type of the structure
  /// @returns true on success, false otherwise
  bool StructureConstructorOrCast(const ast::CallExpression* ctor,
                                  const sem::Struct* struct_type) const;

  /// Validates a switch statement
  /// @param s the switch to validate
  /// @returns true on success, false otherwise
  bool SwitchStatement(const ast::SwitchStatement* s);

  /// Validates a variable
  /// @param var the variable to validate
  /// @returns true on success, false otherwise.
  bool Variable(const sem::Variable* var) const;

  /// Validates a variable constructor or cast
  /// @param var the variable to validate
  /// @param storage_class the storage class of the variable
  /// @param storage_type the type of the storage
  /// @param rhs_type the right hand side of the expression
  /// @returns true on succes, false otherwise
  bool VariableConstructorOrCast(const ast::Variable* var,
                                 ast::StorageClass storage_class,
                                 const sem::Type* storage_type,
                                 const sem::Type* rhs_type) const;

  /// Validates a vector
  /// @param ty the vector to validate
  /// @param source the source of the vector
  /// @returns true on success, false otherwise
  bool Vector(const sem::Vector* ty, const Source& source) const;

  /// Validates a vector constructor or cast
  /// @param ctor the call expression to validate
  /// @param vec_type the vector type
  /// @returns true on success, false otherwise
  bool VectorConstructorOrCast(const ast::CallExpression* ctor,
                               const sem::Vector* vec_type) const;

  /// Validates a matrix constructor or cast
  /// @param ctor the call expression to validate
  /// @param matrix_type the type of the matrix
  /// @returns true on success, false otherwise
  bool MatrixConstructorOrCast(const ast::CallExpression* ctor,
                               const sem::Matrix* matrix_type) const;

  /// Validates a scalar constructor or cast
  /// @param ctor the call expression to validate
  /// @param type the type of the scalar
  /// @returns true on success, false otherwise.
  bool ScalarConstructorOrCast(const ast::CallExpression* ctor,
                               const sem::Type* type) const;

  /// Validates an array constructor or cast
  /// @param ctor the call expresion to validate
  /// @param arr_type the type of the array
  /// @returns true on success, false otherwise
  bool ArrayConstructorOrCast(const ast::CallExpression* ctor,
                              const sem::Array* arr_type) const;

  /// Validates a texture builtin function
  /// @param call the builtin call to validate
  /// @returns true on success, false otherwise
  bool TextureBuiltinFunction(const sem::Call* call) const;

  /// Validates there are no duplicate attributes
  /// @param attributes the list of attributes to validate
  /// @returns true on success, false otherwise.
  bool NoDuplicateAttributes(const ast::AttributeList& attributes) const;

  /// Validates a storage class layout
  /// @param type the type to validate
  /// @param sc the storage class
  /// @param source the source of the type
  /// @param layouts previously validated storage layouts
  /// @returns true on success, false otherwise
  bool StorageClassLayout(const sem::Type* type,
                          ast::StorageClass sc,
                          Source source,
                          ValidTypeStorageLayouts& layouts) const;

  /// Validates a storage class layout
  /// @param var the variable to validate
  /// @param layouts previously validated storage layouts
  /// @returns true on success, false otherwise.
  bool StorageClassLayout(const sem::Variable* var,
                          ValidTypeStorageLayouts& layouts) const;

  /// @returns true if the attribute list contains a
  /// ast::DisableValidationAttribute with the validation mode equal to
  /// `validation`
  /// @param attributes the attribute list to check
  /// @param validation the validation mode to check
  bool IsValidationDisabled(const ast::AttributeList& attributes,
                            ast::DisabledValidation validation) const;

  /// @returns true if the attribute list does not contains a
  /// ast::DisableValidationAttribute with the validation mode equal to
  /// `validation`
  /// @param attributes the attribute list to check
  /// @param validation the validation mode to check
  bool IsValidationEnabled(const ast::AttributeList& attributes,
                           ast::DisabledValidation validation) const;

 private:
  /// Searches the current statement and up through parents of the current
  /// statement looking for a loop or for-loop continuing statement.
  /// @returns the closest continuing statement to the current statement that
  /// (transitively) owns the current statement.
  /// @param stop_at_loop if true then the function will return nullptr if a
  /// loop or for-loop was found before the continuing.
  /// @param current_statement the current statement being resolved
  const ast::Statement* ClosestContinuing(
      bool stop_at_loop,
      sem::Statement* current_statement) const;

  /// Returns a human-readable string representation of the vector type name
  /// with the given parameters.
  /// @param size the vector dimension
  /// @param element_type scalar vector sub-element type
  /// @return pretty string representation
  std::string VectorPretty(uint32_t size, const sem::Type* element_type) const;

  SymbolTable& symbols_;
  diag::List& diagnostics_;
  SemHelper& sem_;
};

}  // namespace tint::resolver

#endif  // SRC_TINT_RESOLVER_VALIDATOR_H_
