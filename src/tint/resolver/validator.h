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
#include <utility>

#include "src/tint/ast/pipeline_stage.h"
#include "src/tint/program_builder.h"
#include "src/tint/resolver/sem_helper.h"
#include "src/tint/sem/evaluation_stage.h"
#include "src/tint/source.h"
#include "src/tint/utils/hashmap.h"
#include "src/tint/utils/vector.h"

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
class WhileStatement;
}  // namespace tint::ast
namespace tint::sem {
class Array;
class Atomic;
class BlockStatement;
class BreakIfStatement;
class Builtin;
class Call;
class CaseStatement;
class ForLoopStatement;
class IfStatement;
class LoopStatement;
class Materialize;
class Statement;
class SwitchStatement;
class TypeInitializer;
class WhileStatement;
}  // namespace tint::sem

namespace tint::resolver {

/// Validation logic for various ast nodes. The validations in general should
/// be shallow and depend on the resolver to call on children. The validations
/// also assume that sem changes have already been made. The validation checks
/// should not alter the AST or SEM trees.
class Validator {
  public:
    /// The valid type storage layouts typedef
    using ValidTypeStorageLayouts = std::set<std::pair<const sem::Type*, ast::AddressSpace>>;

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
    bool PipelineStages(const utils::VectorRef<sem::Function*> entry_points) const;

    /// Validates push_constant variables
    /// @param entry_points the entry points to the module
    /// @returns true on success, false otherwise.
    bool PushConstants(const utils::VectorRef<sem::Function*> entry_points) const;

    /// Validates aliases
    /// @param alias the alias to validate
    /// @returns true on success, false otherwise.
    bool Alias(const ast::Alias* alias) const;

    /// Validates the array
    /// @param arr the array to validate
    /// @param el_source the source of the array element, or the array if the array does not have a
    ///        locally-declared element AST node.
    /// @returns true on success, false otherwise.
    bool Array(const sem::Array* arr, const Source& el_source) const;

    /// Validates an array stride attribute
    /// @param attr the stride attribute to validate
    /// @param el_size the element size
    /// @param el_align the element alignment
    /// @returns true on success, false otherwise
    bool ArrayStrideAttribute(const ast::StrideAttribute* attr,
                              uint32_t el_size,
                              uint32_t el_align) const;

    /// Validates an atomic
    /// @param a the atomic ast node to validate
    /// @param s the atomic sem node
    /// @returns true on success, false otherwise.
    bool Atomic(const ast::Atomic* a, const sem::Atomic* s) const;

    /// Validates an atoic variable
    /// @param var the variable to validate
    /// @param atomic_composite_info store atomic information
    /// @returns true on success, false otherwise.
    bool AtomicVariable(
        const sem::Variable* var,
        const utils::Hashmap<const sem::Type*, const Source*, 8>& atomic_composite_info) const;

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
    bool BreakStatement(const sem::Statement* stmt, sem::Statement* current_statement) const;

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
    bool ContinueStatement(const sem::Statement* stmt, sem::Statement* current_statement) const;

    /// Validates a call
    /// @param call the call
    /// @param current_statement the current statement being resolved
    /// @returns true on success, false otherwise
    bool Call(const sem::Call* call, sem::Statement* current_statement) const;

    /// Validates an entry point
    /// @param func the entry point function to validate
    /// @param stage the pipeline stage for the entry point
    /// @returns true on success, false otherwise
    bool EntryPoint(const sem::Function* func, ast::PipelineStage stage) const;

    /// Validates that the expression must not be evaluated any later than @p latest_stage
    /// @param expr the expression to check
    /// @param latest_stage the latest evaluation stage that the expression can be evaluated
    /// @param constraint the 'thing' that is imposing the contraint. e.g. "var declaration"
    /// @returns true if @p expr is evaluated in or before @p latest_stage, false otherwise
    bool EvaluationStage(const sem::Expression* expr,
                         sem::EvaluationStage latest_stage,
                         std::string_view constraint) const;

    /// Validates a for loop
    /// @param stmt the for loop statement to validate
    /// @returns true on success, false otherwise
    bool ForLoopStatement(const sem::ForLoopStatement* stmt) const;

    /// Validates a while loop
    /// @param stmt the while statement to validate
    /// @returns true on success, false otherwise
    bool WhileStatement(const sem::WhileStatement* stmt) const;

    /// Validates a function
    /// @param func the function to validate
    /// @param stage the current pipeline stage
    /// @returns true on success, false otherwise.
    bool Function(const sem::Function* func, ast::PipelineStage stage) const;

    /// Validates a function call
    /// @param call the function call to validate
    /// @param current_statement the current statement being resolved
    /// @returns true on success, false otherwise
    bool FunctionCall(const sem::Call* call, sem::Statement* current_statement) const;

    /// Validates a global variable
    /// @param var the global variable to validate
    /// @param override_id the set of override ids in the module
    /// @param atomic_composite_info atomic composite info in the module
    /// @returns true on success, false otherwise
    bool GlobalVariable(
        const sem::GlobalVariable* var,
        const utils::Hashmap<OverrideId, const sem::Variable*, 8>& override_id,
        const utils::Hashmap<const sem::Type*, const Source*, 8>& atomic_composite_info) const;

    /// Validates a break-if statement
    /// @param stmt the statement to validate
    /// @param current_statement the current statement being resolved
    /// @returns true on success, false otherwise
    bool BreakIfStatement(const sem::BreakIfStatement* stmt,
                          sem::Statement* current_statement) const;

    /// Validates an if statement
    /// @param stmt the statement to validate
    /// @returns true on success, false otherwise
    bool IfStatement(const sem::IfStatement* stmt) const;

    /// Validates an increment or decrement statement
    /// @param stmt the statement to validate
    /// @returns true on success, false otherwise
    bool IncrementDecrementStatement(const ast::IncrementDecrementStatement* stmt) const;

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

    /// Validates a local variable
    /// @param v the variable to validate
    /// @returns true on success, false otherwise.
    bool LocalVariable(const sem::Variable* v) const;

    /// Validates a location attribute
    /// @param loc_attr the location attribute to validate
    /// @param location the location value
    /// @param type the variable type
    /// @param locations the set of locations in the module
    /// @param stage the current pipeline stage
    /// @param source the source of the attribute
    /// @param is_input true if this is an input variable
    /// @returns true on success, false otherwise.
    bool LocationAttribute(const ast::LocationAttribute* loc_attr,
                           uint32_t location,
                           const sem::Type* type,
                           utils::Hashset<uint32_t, 8>& locations,
                           ast::PipelineStage stage,
                           const Source& source,
                           const bool is_input = false) const;

    /// Validates a loop statement
    /// @param stmt the loop statement
    /// @returns true on success, false otherwise.
    bool LoopStatement(const sem::LoopStatement* stmt) const;

    /// Validates a materialize of an abstract numeric value from the type `from` to the type `to`.
    /// @param to the target type
    /// @param from the abstract numeric type
    /// @param source the source of the materialization
    /// @returns true on success, false otherwise
    bool Materialize(const sem::Type* to, const sem::Type* from, const Source& source) const;

    /// Validates a matrix
    /// @param ty the matrix to validate
    /// @param source the source of the matrix
    /// @returns true on success, false otherwise
    bool Matrix(const sem::Matrix* ty, const Source& source) const;

    /// Validates a function parameter
    /// @param func the function the variable is for
    /// @param var the variable to validate
    /// @returns true on success, false otherwise
    bool Parameter(const ast::Function* func, const sem::Variable* var) const;

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
    bool Statements(utils::VectorRef<const ast::Statement*> stmts) const;

    /// Validates a storage texture
    /// @param t the texture to validate
    /// @returns true on success, false otherwise
    bool StorageTexture(const ast::StorageTexture* t) const;

    /// Validates a sampled texture
    /// @param t the texture to validate
    /// @param source the source of the texture
    /// @returns true on success, false otherwise
    bool SampledTexture(const sem::SampledTexture* t, const Source& source) const;

    /// Validates a multisampled texture
    /// @param t the texture to validate
    /// @param source the source of the texture
    /// @returns true on success, false otherwise
    bool MultisampledTexture(const sem::MultisampledTexture* t, const Source& source) const;

    /// Validates a structure
    /// @param str the structure to validate
    /// @param stage the current pipeline stage
    /// @returns true on success, false otherwise.
    bool Structure(const sem::Struct* str, ast::PipelineStage stage) const;

    /// Validates a structure initializer
    /// @param ctor the call expression to validate
    /// @param struct_type the type of the structure
    /// @returns true on success, false otherwise
    bool StructureInitializer(const ast::CallExpression* ctor,
                              const sem::Struct* struct_type) const;

    /// Validates a switch statement
    /// @param s the switch to validate
    /// @returns true on success, false otherwise
    bool SwitchStatement(const ast::SwitchStatement* s);

    /// Validates a 'var' variable declaration
    /// @param v the variable to validate
    /// @returns true on success, false otherwise.
    bool Var(const sem::Variable* v) const;

    /// Validates a 'let' variable declaration
    /// @param v the variable to validate
    /// @returns true on success, false otherwise.
    bool Let(const sem::Variable* v) const;

    /// Validates a 'override' variable declaration
    /// @param v the variable to validate
    /// @param override_id the set of override ids in the module
    /// @returns true on success, false otherwise.
    bool Override(const sem::GlobalVariable* v,
                  const utils::Hashmap<OverrideId, const sem::Variable*, 8>& override_id) const;

    /// Validates a 'const' variable declaration
    /// @param v the variable to validate
    /// @returns true on success, false otherwise.
    bool Const(const sem::Variable* v) const;

    /// Validates a variable initializer
    /// @param v the variable to validate
    /// @param address_space the address space of the variable
    /// @param storage_type the type of the storage
    /// @param initializer the RHS initializer expression
    /// @returns true on succes, false otherwise
    bool VariableInitializer(const ast::Variable* v,
                             ast::AddressSpace address_space,
                             const sem::Type* storage_type,
                             const sem::Expression* initializer) const;

    /// Validates a vector
    /// @param ty the vector to validate
    /// @param source the source of the vector
    /// @returns true on success, false otherwise
    bool Vector(const sem::Vector* ty, const Source& source) const;

    /// Validates an array initializer
    /// @param ctor the call expresion to validate
    /// @param arr_type the type of the array
    /// @returns true on success, false otherwise
    bool ArrayInitializer(const ast::CallExpression* ctor, const sem::Array* arr_type) const;

    /// Validates a texture builtin function
    /// @param call the builtin call to validate
    /// @returns true on success, false otherwise
    bool TextureBuiltinFunction(const sem::Call* call) const;

    /// Validates an optional builtin function and its required extension.
    /// @param call the builtin call to validate
    /// @param enabled_extensions all the extensions declared in current module
    /// @returns true on success, false otherwise
    bool RequiredExtensionForBuiltinFunction(const sem::Call* call,
                                             const ast::Extensions& enabled_extensions) const;

    /// Validates there are no duplicate attributes
    /// @param attributes the list of attributes to validate
    /// @returns true on success, false otherwise.
    bool NoDuplicateAttributes(utils::VectorRef<const ast::Attribute*> attributes) const;

    /// Validates a address space layout
    /// @param type the type to validate
    /// @param sc the address space
    /// @param source the source of the type
    /// @param layouts previously validated storage layouts
    /// @returns true on success, false otherwise
    bool AddressSpaceLayout(const sem::Type* type,
                            ast::AddressSpace sc,
                            Source source,
                            ValidTypeStorageLayouts& layouts) const;

    /// Validates a address space layout
    /// @param var the variable to validate
    /// @param layouts previously validated storage layouts
    /// @param enabled_extensions all the extensions declared in current module
    /// @returns true on success, false otherwise.
    bool AddressSpaceLayout(const sem::Variable* var,
                            const ast::Extensions& enabled_extensions,
                            ValidTypeStorageLayouts& layouts) const;

    /// @returns true if the attribute list contains a
    /// ast::DisableValidationAttribute with the validation mode equal to
    /// `validation`
    /// @param attributes the attribute list to check
    /// @param validation the validation mode to check
    bool IsValidationDisabled(utils::VectorRef<const ast::Attribute*> attributes,
                              ast::DisabledValidation validation) const;

    /// @returns true if the attribute list does not contains a
    /// ast::DisableValidationAttribute with the validation mode equal to
    /// `validation`
    /// @param attributes the attribute list to check
    /// @param validation the validation mode to check
    bool IsValidationEnabled(utils::VectorRef<const ast::Attribute*> attributes,
                             ast::DisabledValidation validation) const;

  private:
    /// @param ty the type to check
    /// @returns true if @p ty is an array with an `override` expression element count, otherwise
    ///          false.
    bool IsArrayWithOverrideCount(const sem::Type* ty) const;

    /// Raises an error about an array type using an `override` expression element count, outside
    /// the single allowed use of a `var<workgroup>`.
    /// @param source the source for the error
    void RaiseArrayWithOverrideCountError(const Source& source) const;

    /// Searches the current statement and up through parents of the current
    /// statement looking for a loop or for-loop continuing statement.
    /// @returns the closest continuing statement to the current statement that
    /// (transitively) owns the current statement.
    /// @param stop_at_loop if true then the function will return nullptr if a
    /// loop or for-loop was found before the continuing.
    /// @param current_statement the current statement being resolved
    const ast::Statement* ClosestContinuing(bool stop_at_loop,
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
