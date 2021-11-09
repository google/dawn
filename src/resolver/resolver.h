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

#ifndef SRC_RESOLVER_RESOLVER_H_
#define SRC_RESOLVER_RESOLVER_H_

#include <memory>
#include <set>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

#include "src/intrinsic_table.h"
#include "src/program_builder.h"
#include "src/scope_stack.h"
#include "src/sem/binding_point.h"
#include "src/sem/block_statement.h"
#include "src/sem/constant.h"
#include "src/sem/function.h"
#include "src/sem/struct.h"
#include "src/utils/unique_vector.h"

namespace tint {

// Forward declarations
namespace ast {
class IndexAccessorExpression;
class BinaryExpression;
class BitcastExpression;
class CallExpression;
class CallStatement;
class CaseStatement;
class ConstructorExpression;
class ForLoopStatement;
class Function;
class IdentifierExpression;
class LoopStatement;
class MemberAccessorExpression;
class ReturnStatement;
class SwitchStatement;
class UnaryOpExpression;
class Variable;
}  // namespace ast
namespace sem {
class Array;
class Atomic;
class Intrinsic;
class Statement;
}  // namespace sem

namespace resolver {

/// Resolves types for all items in the given tint program
class Resolver {
 public:
  /// Constructor
  /// @param builder the program builder
  explicit Resolver(ProgramBuilder* builder);

  /// Destructor
  ~Resolver();

  /// @returns error messages from the resolver
  std::string error() const { return diagnostics_.str(); }

  /// @returns true if the resolver was successful
  bool Resolve();

  /// @param type the given type
  /// @returns true if the given type is a plain type
  bool IsPlain(const sem::Type* type) const;

  /// @param type the given type
  /// @returns true if the given type is storable
  bool IsStorable(const sem::Type* type) const;

  /// @param type the given type
  /// @returns true if the given type is host-shareable
  bool IsHostShareable(const sem::Type* type) const;

 private:
  /// Describes the context in which a variable is declared
  enum class VariableKind { kParameter, kLocal, kGlobal };

  std::set<std::pair<const sem::Struct*, ast::StorageClass>>
      valid_struct_storage_layouts_;

  /// Structure holding semantic information about a block (i.e. scope), such as
  /// parent block and variables declared in the block.
  /// Used to validate variable scoping rules.
  struct BlockInfo {
    enum class Type { kGeneric, kLoop, kLoopContinuing, kSwitchCase };

    BlockInfo(const ast::BlockStatement* block, Type type, BlockInfo* parent);
    ~BlockInfo();

    template <typename Pred>
    BlockInfo* FindFirstParent(Pred&& pred) {
      BlockInfo* curr = this;
      while (curr && !pred(curr)) {
        curr = curr->parent;
      }
      return curr;
    }

    BlockInfo* FindFirstParent(BlockInfo::Type ty) {
      return FindFirstParent(
          [ty](auto* block_info) { return block_info->type == ty; });
    }

    ast::BlockStatement const* const block;
    const Type type;
    BlockInfo* const parent;
    std::vector<const ast::Variable*> decls;

    // first_continue is set to the index of the first variable in decls
    // declared after the first continue statement in a loop block, if any.
    constexpr static size_t kNoContinue = size_t(~0);
    size_t first_continue = kNoContinue;
  };

  // Structure holding information for a TypeDecl
  struct TypeDeclInfo {
    ast::TypeDecl const* const ast;
    sem::Type* const sem;
  };

  /// Resolves the program, without creating final the semantic nodes.
  /// @returns true on success, false on error
  bool ResolveInternal();

  bool ValidatePipelineStages();

  /// Creates the nodes and adds them to the sem::Info mappings of the
  /// ProgramBuilder.
  void CreateSemanticNodes() const;

  /// Retrieves information for the requested import.
  /// @param src the source of the import
  /// @param path the import path
  /// @param name the method name to get information on
  /// @param params the parameters to the method call
  /// @param id out parameter for the external call ID. Must not be a nullptr.
  /// @returns the return type of `name` in `path` or nullptr on error.
  sem::Type* GetImportData(const Source& src,
                           const std::string& path,
                           const std::string& name,
                           const ast::ExpressionList& params,
                           uint32_t* id);

  //////////////////////////////////////////////////////////////////////////////
  // AST and Type traversal methods
  //////////////////////////////////////////////////////////////////////////////

  // Expression resolving methods
  // Returns the semantic node pointer on success, nullptr on failure.
  sem::Expression* IndexAccessor(const ast::IndexAccessorExpression*);
  sem::Expression* Binary(const ast::BinaryExpression*);
  sem::Expression* Bitcast(const ast::BitcastExpression*);
  sem::Expression* Call(const ast::CallExpression*);
  sem::Expression* Expression(const ast::Expression*);
  sem::Function* Function(const ast::Function*);
  sem::Call* FunctionCall(const ast::CallExpression*);
  sem::Expression* Identifier(const ast::IdentifierExpression*);
  sem::Call* IntrinsicCall(const ast::CallExpression*, sem::IntrinsicType);
  sem::Expression* Literal(const ast::Literal*);
  sem::Expression* MemberAccessor(const ast::MemberAccessorExpression*);
  sem::Expression* TypeConstructor(const ast::TypeConstructorExpression*);
  sem::Expression* UnaryOp(const ast::UnaryOpExpression*);

  // Statement resolving methods
  // Each return true on success, false on failure.
  bool Assignment(const ast::AssignmentStatement* a);
  bool BlockStatement(const ast::BlockStatement*);
  bool CaseStatement(const ast::CaseStatement*);
  bool ElseStatement(const ast::ElseStatement*);
  bool ForLoopStatement(const ast::ForLoopStatement*);
  bool Parameter(const ast::Variable* param);
  bool GlobalVariable(const ast::Variable* var);
  bool IfStatement(const ast::IfStatement*);
  bool LoopStatement(const ast::LoopStatement*);
  bool Return(const ast::ReturnStatement* ret);
  bool Statement(const ast::Statement*);
  bool Statements(const ast::StatementList&);
  bool SwitchStatement(const ast::SwitchStatement* s);
  bool VariableDeclStatement(const ast::VariableDeclStatement*);

  // AST and Type validation methods
  // Each return true on success, false on failure.
  bool ValidateArray(const sem::Array* arr, const Source& source);
  bool ValidateArrayStrideDecoration(const ast::StrideDecoration* deco,
                                     uint32_t el_size,
                                     uint32_t el_align,
                                     const Source& source);
  bool ValidateAtomic(const ast::Atomic* a, const sem::Atomic* s);
  bool ValidateAtomicVariable(const sem::Variable* var);
  bool ValidateAssignment(const ast::AssignmentStatement* a);
  bool ValidateBuiltinDecoration(const ast::BuiltinDecoration* deco,
                                 const sem::Type* storage_type,
                                 const bool is_input);
  bool ValidateCall(const sem::Call* call);
  bool ValidateEntryPoint(const sem::Function* func);
  bool ValidateFunction(const sem::Function* func);
  bool ValidateFunctionCall(const sem::Call* call);
  bool ValidateGlobalVariable(const sem::Variable* var);
  bool ValidateInterpolateDecoration(const ast::InterpolateDecoration* deco,
                                     const sem::Type* storage_type);
  bool ValidateLocationDecoration(const ast::LocationDecoration* location,
                                  const sem::Type* type,
                                  std::unordered_set<uint32_t>& locations,
                                  const Source& source,
                                  const bool is_input = false);
  bool ValidateMatrix(const sem::Matrix* ty, const Source& source);
  bool ValidateFunctionParameter(const ast::Function* func,
                                 const sem::Variable* var);
  bool ValidateNoDuplicateDefinition(Symbol sym,
                                     const Source& source,
                                     bool check_global_scope_only = false);
  bool ValidateParameter(const ast::Function* func, const sem::Variable* var);
  bool ValidateReturn(const ast::ReturnStatement* ret);
  bool ValidateStatements(const ast::StatementList& stmts);
  bool ValidateStorageTexture(const ast::StorageTexture* t);
  bool ValidateStructure(const sem::Struct* str);
  bool ValidateStructureConstructor(const ast::TypeConstructorExpression* ctor,
                                    const sem::Struct* struct_type);
  bool ValidateSwitch(const ast::SwitchStatement* s);
  bool ValidateVariable(const sem::Variable* var);
  bool ValidateVariableConstructor(const ast::Variable* var,
                                   ast::StorageClass storage_class,
                                   const sem::Type* storage_type,
                                   const sem::Type* rhs_type);
  bool ValidateVector(const sem::Vector* ty, const Source& source);
  bool ValidateVectorConstructor(const ast::TypeConstructorExpression* ctor,
                                 const sem::Vector* vec_type);
  bool ValidateMatrixConstructor(const ast::TypeConstructorExpression* ctor,
                                 const sem::Matrix* matrix_type);
  bool ValidateScalarConstructor(const ast::TypeConstructorExpression* ctor,
                                 const sem::Type* type);
  bool ValidateArrayConstructor(const ast::TypeConstructorExpression* ctor,
                                const sem::Array* arr_type);
  bool ValidateTypeDecl(const ast::TypeDecl* named_type) const;
  bool ValidateTextureIntrinsicFunction(const sem::Call* call);
  bool ValidateNoDuplicateDecorations(const ast::DecorationList& decorations);
  // sem::Struct is assumed to have at least one member
  bool ValidateStorageClassLayout(const sem::Struct* type,
                                  ast::StorageClass sc);
  bool ValidateStorageClassLayout(const sem::Variable* var);

  /// Resolves the WorkgroupSize for the given function
  bool WorkgroupSizeFor(const ast::Function*, sem::WorkgroupSize& ws);

  /// @returns the sem::Type for the ast::Type `ty`, building it if it
  /// hasn't been constructed already. If an error is raised, nullptr is
  /// returned.
  /// @param ty the ast::Type
  sem::Type* Type(const ast::Type* ty);

  /// @param named_type the named type to resolve
  /// @returns the resolved semantic type
  sem::Type* TypeDecl(const ast::TypeDecl* named_type);

  /// Builds and returns the semantic information for the array `arr`.
  /// This method does not mark the ast::Array node, nor attach the generated
  /// semantic information to the AST node.
  /// @returns the semantic Array information, or nullptr if an error is raised.
  /// @param arr the Array to get semantic information for
  sem::Array* Array(const ast::Array* arr);

  /// Builds and returns the semantic information for the structure `str`.
  /// This method does not mark the ast::Struct node, nor attach the generated
  /// semantic information to the AST node.
  /// @returns the semantic Struct information, or nullptr if an error is
  /// raised. raised, nullptr is returned.
  sem::Struct* Structure(const ast::Struct* str);

  /// @returns the semantic info for the variable `var`. If an error is raised,
  /// nullptr is returned.
  /// @note this method does not resolve the decorations as these are
  /// context-dependent (global, local, parameter)
  /// @param var the variable to create or return the `VariableInfo` for
  /// @param kind what kind of variable we are declaring
  /// @param index the index of the parameter, if this variable is a parameter
  sem::Variable* Variable(const ast::Variable* var,
                          VariableKind kind,
                          uint32_t index = 0);

  /// Records the storage class usage for the given type, and any transient
  /// dependencies of the type. Validates that the type can be used for the
  /// given storage class, erroring if it cannot.
  /// @param sc the storage class to apply to the type and transitent types
  /// @param ty the type to apply the storage class on
  /// @param usage the Source of the root variable declaration that uses the
  /// given type and storage class. Used for generating sensible error messages.
  /// @returns true on success, false on error
  bool ApplyStorageClassUsageToType(ast::StorageClass sc,
                                    sem::Type* ty,
                                    const Source& usage);

  /// @param storage_class the storage class
  /// @returns the default access control for the given storage class
  ast::Access DefaultAccessForStorageClass(ast::StorageClass storage_class);

  /// Allocate constant IDs for pipeline-overridable constants.
  void AllocateOverridableConstantIds();

  /// @returns the resolved type of the ast::Expression `expr`
  /// @param expr the expression
  sem::Type* TypeOf(const ast::Expression* expr);

  /// @returns the type name of the given semantic type, unwrapping references.
  std::string TypeNameOf(const sem::Type* ty);

  /// @returns the type name of the given semantic type, without unwrapping
  /// references.
  std::string RawTypeNameOf(const sem::Type* ty);

  /// @returns the semantic type of the AST literal `lit`
  /// @param lit the literal
  sem::Type* TypeOf(const ast::Literal* lit);

  /// Assigns `stmt` to #current_statement_, #current_compound_statement_, and
  /// possibly #current_block_, pushes the variable scope, then calls
  /// `callback`. Before returning #current_statement_,
  /// #current_compound_statement_, and #current_block_ are restored to their
  /// original values, and the variable scope is popped.
  /// @returns the value returned by callback
  template <typename F>
  bool Scope(sem::CompoundStatement* stmt, F&& callback);

  /// Returns a human-readable string representation of the vector type name
  /// with the given parameters.
  /// @param size the vector dimension
  /// @param element_type scalar vector sub-element type
  /// @return pretty string representation
  std::string VectorPretty(uint32_t size, const sem::Type* element_type);

  /// Mark records that the given AST node has been visited, and asserts that
  /// the given node has not already been seen. Diamonds in the AST are illegal.
  /// @param node the AST node.
  void Mark(const ast::Node* node);

  /// Adds the given error message to the diagnostics
  void AddError(const std::string& msg, const Source& source) const;

  /// Adds the given warning message to the diagnostics
  void AddWarning(const std::string& msg, const Source& source) const;

  /// Adds the given note message to the diagnostics
  void AddNote(const std::string& msg, const Source& source) const;

  template <typename CALLBACK>
  void TraverseCallChain(const sem::Function* from,
                         const sem::Function* to,
                         CALLBACK&& callback) const;

  //////////////////////////////////////////////////////////////////////////////
  /// Constant value evaluation methods
  //////////////////////////////////////////////////////////////////////////////
  /// Cast `Value` to `target_type`
  /// @return the casted value
  sem::Constant ConstantCast(const sem::Constant& value,
                             const sem::Type* target_elem_type);

  sem::Constant EvaluateConstantValue(const ast::Expression* expr,
                                      const sem::Type* type);
  sem::Constant EvaluateConstantValue(const ast::Literal* literal,
                                      const sem::Type* type);
  sem::Constant EvaluateConstantValue(
      const ast::TypeConstructorExpression* type_ctor,
      const sem::Type* type);

  /// Sem is a helper for obtaining the semantic node for the given AST node.
  template <typename SEM = sem::Info::InferFromAST,
            typename AST_OR_TYPE = CastableBase>
  const sem::Info::GetResultType<SEM, AST_OR_TYPE>* Sem(const AST_OR_TYPE* ast);

  ProgramBuilder* const builder_;
  diag::List& diagnostics_;
  std::unique_ptr<IntrinsicTable> const intrinsic_table_;
  ScopeStack<sem::Variable*> variable_stack_;
  std::unordered_map<Symbol, sem::Function*> symbol_to_function_;
  std::vector<sem::Function*> entry_points_;
  std::unordered_map<const sem::Type*, const Source&> atomic_composite_info_;
  std::unordered_map<Symbol, TypeDeclInfo> named_type_info_;

  std::unordered_set<const ast::Node*> marked_;
  std::unordered_map<uint32_t, const sem::Variable*> constant_ids_;

  sem::Function* current_function_ = nullptr;
  sem::Statement* current_statement_ = nullptr;
  sem::CompoundStatement* current_compound_statement_ = nullptr;
  sem::BlockStatement* current_block_ = nullptr;
};

}  // namespace resolver
}  // namespace tint

#endif  // SRC_RESOLVER_RESOLVER_H_
