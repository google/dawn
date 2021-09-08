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
class ArrayAccessorExpression;
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

  /// Structure holding semantic information about a variable.
  /// Used to build the sem::Variable nodes at the end of resolving.
  struct VariableInfo {
    VariableInfo(const ast::Variable* decl,
                 sem::Type* type,
                 const std::string& type_name,
                 ast::StorageClass storage_class,
                 ast::Access ac,
                 VariableKind k,
                 uint32_t idx);
    ~VariableInfo();

    ast::Variable const* const declaration;
    sem::Type* type;
    std::string const type_name;
    ast::StorageClass storage_class;
    ast::Access const access;
    std::vector<ast::IdentifierExpression*> users;
    sem::BindingPoint binding_point;
    VariableKind kind;
    uint32_t index = 0;  // Parameter index, if kind == kParameter
  };

  struct IntrinsicCallInfo {
    const ast::CallExpression* call;
    const sem::Intrinsic* intrinsic;
  };

  std::set<std::pair<const sem::Struct*, ast::StorageClass>>
      valid_struct_storage_layouts_;

  /// Structure holding semantic information about a function.
  /// Used to build the sem::Function nodes at the end of resolving.
  struct FunctionInfo {
    explicit FunctionInfo(ast::Function* decl);
    ~FunctionInfo();

    ast::Function* const declaration;
    std::vector<VariableInfo*> parameters;
    UniqueVector<VariableInfo*> referenced_module_vars;
    UniqueVector<VariableInfo*> local_referenced_module_vars;
    std::vector<const ast::ReturnStatement*> return_statements;
    std::vector<const ast::CallExpression*> callsites;
    sem::Type* return_type = nullptr;
    std::string return_type_name;
    std::array<sem::WorkgroupDimension, 3> workgroup_size;
    std::vector<IntrinsicCallInfo> intrinsic_calls;

    // List of transitive calls this function makes
    UniqueVector<FunctionInfo*> transitive_calls;

    // List of entry point functions that transitively call this function
    UniqueVector<FunctionInfo*> ancestor_entry_points;
  };

  /// Structure holding semantic information about an expression.
  /// Used to build the sem::Expression nodes at the end of resolving.
  struct ExpressionInfo {
    sem::Type const* type;
    std::string const type_name;  // Declared type name
    sem::Statement* statement;
    sem::Constant constant_value;
  };

  /// Structure holding semantic information about a call expression to an
  /// ast::Function.
  /// Used to build the sem::Call nodes at the end of resolving.
  struct FunctionCallInfo {
    FunctionInfo* function;
    sem::Statement* statement;
  };

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
    Type const type;
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

  void set_referenced_from_function_if_needed(VariableInfo* var, bool local);

  // AST and Type traversal methods
  // Each return true on success, false on failure.
  bool ArrayAccessor(ast::ArrayAccessorExpression*);
  bool Assignment(ast::AssignmentStatement* a);
  bool Binary(ast::BinaryExpression*);
  bool Bitcast(ast::BitcastExpression*);
  bool BlockStatement(ast::BlockStatement*);
  bool Call(ast::CallExpression*);
  bool CaseStatement(ast::CaseStatement*);
  bool Constructor(ast::ConstructorExpression*);
  bool ElseStatement(ast::ElseStatement*);
  bool Expression(ast::Expression*);
  bool ForLoopStatement(ast::ForLoopStatement*);
  bool Function(ast::Function*);
  bool FunctionCall(const ast::CallExpression* call);
  bool GlobalVariable(ast::Variable* var);
  bool Identifier(ast::IdentifierExpression*);
  bool IfStatement(ast::IfStatement*);
  bool IntrinsicCall(ast::CallExpression*, sem::IntrinsicType);
  bool LoopStatement(ast::LoopStatement*);
  bool MemberAccessor(ast::MemberAccessorExpression*);
  bool Parameter(ast::Variable* param);
  bool Return(ast::ReturnStatement* ret);
  bool Statement(ast::Statement*);
  bool Statements(const ast::StatementList&);
  bool SwitchStatement(ast::SwitchStatement* s);
  bool UnaryOp(ast::UnaryOpExpression*);
  bool VariableDeclStatement(const ast::VariableDeclStatement*);

  /// Performs a depth-first traversal of the expression nodes from `root`,
  /// collecting all the visited expressions in pre-ordering (root first).
  /// @param root the root expression node
  /// @param out the ordered list of visited expression nodes, starting with the
  ///        root node, and ending with leaf nodes
  /// @return true on success, false on error
  bool TraverseExpressions(ast::Expression* root,
                          std::vector<ast::Expression*>& out);

  // AST and Type validation methods
  // Each return true on success, false on failure.
  bool ValidateArray(const sem::Array* arr, const Source& source);
  bool ValidateArrayStrideDecoration(const ast::StrideDecoration* deco,
                                     uint32_t el_size,
                                     uint32_t el_align,
                                     const Source& source);
  bool ValidateAtomic(const ast::Atomic* a, const sem::Atomic* s);
  bool ValidateAtomicVariable(const VariableInfo* info);
  bool ValidateAssignment(const ast::AssignmentStatement* a);
  bool ValidateBuiltinDecoration(const ast::BuiltinDecoration* deco,
                                 const sem::Type* storage_type,
                                 const bool is_input,
                                 const bool is_struct_member);
  bool ValidateCall(ast::CallExpression* call);
  bool ValidateCallStatement(ast::CallStatement* stmt);
  bool ValidateEntryPoint(const ast::Function* func, const FunctionInfo* info);
  bool ValidateFunction(const ast::Function* func, const FunctionInfo* info);
  bool ValidateFunctionCall(const ast::CallExpression* call,
                            const FunctionInfo* target);
  bool ValidateGlobalVariable(const VariableInfo* var);
  bool ValidateInterpolateDecoration(const ast::InterpolateDecoration* deco,
                                     const sem::Type* storage_type);
  bool ValidateLocationDecoration(const ast::LocationDecoration* location,
                                  const sem::Type* type,
                                  std::unordered_set<uint32_t>& locations,
                                  const Source& source,
                                  const bool is_input = false);
  bool ValidateMatrix(const sem::Matrix* ty, const Source& source);
  bool ValidateFunctionParameter(const ast::Function* func,
                                 const VariableInfo* info);
  bool ValidateNoDuplicateDefinition(Symbol sym,
                                     const Source& source,
                                     bool check_global_scope_only = false);
  bool ValidateParameter(const ast::Function* func, const VariableInfo* info);
  bool ValidateReturn(const ast::ReturnStatement* ret);
  bool ValidateStatements(const ast::StatementList& stmts);
  bool ValidateStorageTexture(const ast::StorageTexture* t);
  bool ValidateStructure(const sem::Struct* str);
  bool ValidateStructureConstructor(const ast::TypeConstructorExpression* ctor,
                                    const sem::Struct* struct_type);
  bool ValidateSwitch(const ast::SwitchStatement* s);
  bool ValidateVariable(const VariableInfo* info);
  bool ValidateVariableConstructor(const ast::Variable* var,
                                   ast::StorageClass storage_class,
                                   const sem::Type* storage_type,
                                   const std::string& type_name,
                                   const sem::Type* rhs_type,
                                   const std::string& rhs_type_name);
  bool ValidateVector(const sem::Vector* ty, const Source& source);
  bool ValidateVectorConstructor(const ast::TypeConstructorExpression* ctor,
                                 const sem::Vector* vec_type,
                                 const std::string& type_name);
  bool ValidateMatrixConstructor(const ast::TypeConstructorExpression* ctor,
                                 const sem::Matrix* matrix_type,
                                 const std::string& type_name);
  bool ValidateScalarConstructor(const ast::TypeConstructorExpression* ctor,
                                 const sem::Type* type,
                                 const std::string& type_name);
  bool ValidateArrayConstructor(const ast::TypeConstructorExpression* ctor,
                                const sem::Array* arr_type);
  bool ValidateTypeDecl(const ast::TypeDecl* named_type) const;
  bool ValidateTextureIntrinsicFunction(const ast::CallExpression* ast_call,
                                        const sem::Call* sem_call);
  bool ValidateNoDuplicateDecorations(const ast::DecorationList& decorations);
  // sem::Struct is assumed to have at least one member
  bool ValidateStorageClassLayout(const sem::Struct* type,
                                  ast::StorageClass sc);
  bool ValidateStorageClassLayout(const VariableInfo* info);

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

  /// @returns the VariableInfo for the variable `var`, building it if it hasn't
  /// been constructed already. If an error is raised, nullptr is returned.
  /// @note this method does not resolve the decorations as these are
  /// context-dependent (global, local, parameter)
  /// @param var the variable to create or return the `VariableInfo` for
  /// @param kind what kind of variable we are declaring
  /// @param index the index of the parameter, if this variable is a parameter
  VariableInfo* Variable(ast::Variable* var,
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

  /// @returns the resolved type of the ast::Expression `expr`
  /// @param expr the expression
  sem::Type* TypeOf(const ast::Expression* expr);

  /// @returns the declared type name of the ast::Expression `expr`
  /// @param expr the type name
  std::string TypeNameOf(const ast::Expression* expr);

  /// @returns the semantic type of the AST literal `lit`
  /// @param lit the literal
  sem::Type* TypeOf(const ast::Literal* lit);

  /// Records the semantic information for the expression node with the resolved
  /// type `type` and optional declared type name `type_name`.
  /// @param expr the expression
  /// @param type the resolved type
  /// @param type_name the declared type name
  void SetExprInfo(const ast::Expression* expr,
                   const sem::Type* type,
                   std::string type_name = "");

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
  void TraverseCallChain(FunctionInfo* from,
                         FunctionInfo* to,
                         CALLBACK&& callback) const;

  //////////////////////////////////////////////////////////////////////////////
  /// Constant value evaluation methods
  //////////////////////////////////////////////////////////////////////////////
  /// @return the Constant value of the given Expression
  sem::Constant ConstantValueOf(const ast::Expression* expr);

  /// Cast `Value` to `target_type`
  /// @return the casted value
  sem::Constant ConstantCast(const sem::Constant& value,
                             const sem::Type* target_elem_type);

  sem::Constant EvaluateConstantValue(const ast::Expression* expr,
                                      const sem::Type* type);
  sem::Constant EvaluateConstantValue(
      const ast::ScalarConstructorExpression* scalar_ctor,
      const sem::Type* type);
  sem::Constant EvaluateConstantValue(
      const ast::TypeConstructorExpression* type_ctor,
      const sem::Type* type);

  ProgramBuilder* const builder_;
  diag::List& diagnostics_;
  std::unique_ptr<IntrinsicTable> const intrinsic_table_;
  ScopeStack<VariableInfo*> variable_stack_;
  std::unordered_map<Symbol, FunctionInfo*> symbol_to_function_;
  std::vector<FunctionInfo*> entry_points_;
  std::unordered_map<const sem::Type*, const Source&> atomic_composite_info_;
  std::unordered_map<const ast::Function*, FunctionInfo*> function_to_info_;
  std::unordered_map<const ast::Variable*, VariableInfo*> variable_to_info_;
  std::unordered_map<const ast::CallExpression*, FunctionCallInfo>
      function_calls_;
  std::unordered_map<const ast::Expression*, ExpressionInfo> expr_info_;
  std::unordered_map<Symbol, TypeDeclInfo> named_type_info_;

  std::unordered_set<const ast::Node*> marked_;
  std::unordered_map<uint32_t, const VariableInfo*> constant_ids_;

  FunctionInfo* current_function_ = nullptr;
  sem::Statement* current_statement_ = nullptr;
  sem::CompoundStatement* current_compound_statement_ = nullptr;
  sem::BlockStatement* current_block_ = nullptr;
  BlockAllocator<VariableInfo> variable_infos_;
  BlockAllocator<FunctionInfo> function_infos_;
};

}  // namespace resolver
}  // namespace tint

#endif  // SRC_RESOLVER_RESOLVER_H_
