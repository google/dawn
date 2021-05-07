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
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "src/intrinsic_table.h"
#include "src/program_builder.h"
#include "src/scope_stack.h"
#include "src/sem/struct.h"
#include "src/utils/unique_vector.h"

namespace tint {

// Forward declarations
namespace ast {
class ArrayAccessorExpression;
class BinaryExpression;
class BitcastExpression;
class CallExpression;
class CaseStatement;
class ConstructorExpression;
class Function;
class IdentifierExpression;
class MemberAccessorExpression;
class ReturnStatement;
class SwitchStatement;
class UnaryOpExpression;
class Variable;
}  // namespace ast
namespace sem {
class Array;
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
  /// @returns true if the given type is storable
  bool IsStorable(const sem::Type* type);

  /// @param type the given type
  /// @returns true if the given type is host-shareable
  bool IsHostShareable(const sem::Type* type);

  /// @param lhs the assignment store type (non-pointer)
  /// @param rhs the assignment source type (non-pointer or pointer with
  /// auto-deref)
  /// @returns true an expression of type `rhs` can be assigned to a variable,
  /// structure member or array element of type `lhs`
  static bool IsValidAssignment(const sem::Type* lhs, const sem::Type* rhs);

  /// @param type the input type
  /// @returns the canonical type for `type`; that is, a type with all aliases
  /// removed. For example, `Canonical(alias<alias<vec3<alias<f32>>>>)` is
  /// `vec3<f32>`.
  sem::Type* Canonical(sem::Type* type);

 private:
  /// Structure holding semantic information about a variable.
  /// Used to build the sem::Variable nodes at the end of resolving.
  struct VariableInfo {
    VariableInfo(const ast::Variable* decl,
                 sem::Type* type,
                 const std::string& type_name);
    ~VariableInfo();

    ast::Variable const* const declaration;
    sem::Type* type;
    std::string const type_name;
    ast::StorageClass storage_class;
    std::vector<ast::IdentifierExpression*> users;
  };

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
    sem::Type* return_type = nullptr;
    std::string return_type_name;

    // List of transitive calls this function makes
    UniqueVector<FunctionInfo*> transitive_calls;
  };

  /// Structure holding semantic information about an expression.
  /// Used to build the sem::Expression nodes at the end of resolving.
  struct ExpressionInfo {
    sem::Type* type;
    std::string const type_name;  // Declared type name
    sem::Statement* statement;
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

  /// Resolves the program, without creating final the semantic nodes.
  /// @returns true on success, false on error
  bool ResolveInternal();

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
  bool BlockStatement(const ast::BlockStatement*);
  bool Call(ast::CallExpression*);
  bool CaseStatement(ast::CaseStatement*);
  bool Constructor(ast::ConstructorExpression*);
  bool Expression(ast::Expression*);
  bool Expressions(const ast::ExpressionList&);
  bool Function(ast::Function*);
  bool GlobalVariable(ast::Variable* var);
  bool Identifier(ast::IdentifierExpression*);
  bool IfStatement(ast::IfStatement*);
  bool IntrinsicCall(ast::CallExpression*, sem::IntrinsicType);
  bool MemberAccessor(ast::MemberAccessorExpression*);
  bool Parameter(ast::Variable* param);
  bool Return(ast::ReturnStatement* ret);
  bool Statement(ast::Statement*);
  bool Statements(const ast::StatementList&);
  bool Switch(ast::SwitchStatement* s);
  bool UnaryOp(ast::UnaryOpExpression*);
  bool VariableDeclStatement(const ast::VariableDeclStatement*);

  // AST and Type validation methods
  // Each return true on success, false on failure.
  bool ValidateArray(const sem::ArrayType* arr, const Source& source);
  bool ValidateArrayStrideDecoration(const ast::StrideDecoration* deco,
                                     uint32_t el_size,
                                     uint32_t el_align,
                                     const Source& source);
  bool ValidateAssignment(const ast::AssignmentStatement* a);
  bool ValidateBinary(ast::BinaryExpression* expr);
  bool ValidateEntryPoint(const ast::Function* func, const FunctionInfo* info);
  bool ValidateFunction(const ast::Function* func, const FunctionInfo* info);
  bool ValidateGlobalVariable(const VariableInfo* var);
  bool ValidateMatrixConstructor(const ast::TypeConstructorExpression* ctor,
                                 const sem::Matrix* matrix_type);
  bool ValidateParameter(const ast::Variable* param);
  bool ValidateReturn(const ast::ReturnStatement* ret);
  bool ValidateStructure(const sem::Struct* str);
  bool ValidateSwitch(const ast::SwitchStatement* s);
  bool ValidateVariable(const ast::Variable* param);
  bool ValidateVectorConstructor(const ast::TypeConstructorExpression* ctor,
                                 const sem::Vector* vec_type);

  /// @returns the sem::Type for the ast::Type `ty`, building it if it
  /// hasn't been constructed already. If an error is raised, nullptr is
  /// returned.
  /// @param ty the ast::Type
  sem::Type* Type(const ast::Type* ty);

  /// @returns the semantic information for the array `arr`, building it if it
  /// hasn't been constructed already. If an error is raised, nullptr is
  /// returned.
  /// @param arr the Array to get semantic information for
  /// @param source the Source of the ast node with this array as its type
  const sem::Array* Array(const sem::ArrayType* arr, const Source& source);

  /// @returns the sem::Struct for the AST structure `str`. If an error is
  /// raised, nullptr is returned.
  sem::Struct* Structure(const ast::Struct* str);

  /// @returns the VariableInfo for the variable `var`, building it if it hasn't
  /// been constructed already. If an error is raised, nullptr is returned.
  /// @param var the variable to create or return the `VariableInfo` for
  /// @param type optional type of `var` to use instead of `var->type()`.
  /// @param type_name optional type name of `var` to use instead of
  /// `var->type()->FriendlyName()`.
  VariableInfo* Variable(ast::Variable* var,
                         sem::Type* type = nullptr,
                         std::string type_name = "");

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

  /// @param align the output default alignment in bytes for the type `ty`
  /// @param size the output default size in bytes for the type `ty`
  /// @param source the Source of the variable declaration of type `ty`
  /// @returns true on success, false on error
  bool DefaultAlignAndSize(const sem::Type* ty,
                           uint32_t& align,
                           uint32_t& size,
                           const Source& source);

  /// @returns the resolved type of the ast::Expression `expr`
  /// @param expr the expression
  sem::Type* TypeOf(const ast::Expression* expr);

  /// @returns the declared type name of the ast::Expression `expr`
  /// @param expr the type name
  std::string TypeNameOf(const ast::Expression* expr);

  /// @returns the semantic type of the AST literal `lit`
  /// @param lit the literal
  sem::Type* TypeOf(const ast::Literal* lit);

  /// Creates a sem::Expression node with the resolved type `type`, and
  /// assigns this semantic node to the expression `expr`.
  /// @param expr the expression
  /// @param type the resolved type
  void SetType(ast::Expression* expr, typ::Type type);

  /// Creates a sem::Expression node with the resolved type `type`, the declared
  /// type name `type_name` and assigns this semantic node to the expression
  /// `expr`.
  /// @param expr the expression
  /// @param type the resolved type
  /// @param type_name the declared type name
  void SetType(ast::Expression* expr,
               typ::Type type,
               const std::string& type_name);

  /// Constructs a new BlockInfo with the given type and with #current_block_ as
  /// its parent, assigns this to #current_block_, and then calls `callback`.
  /// The original #current_block_ is restored on exit.
  template <typename F>
  bool BlockScope(const ast::BlockStatement* block,
                  BlockInfo::Type type,
                  F&& callback);

  /// Returns a human-readable string representation of the vector type name
  /// with the given parameters.
  /// @param size the vector dimension
  /// @param element_type scalar vector sub-element type
  /// @return pretty string representation
  std::string VectorPretty(uint32_t size, sem::Type* element_type);

  /// Mark records that the given AST node has been visited, and asserts that
  /// the given node has not already been seen. Diamonds in the AST are illegal.
  /// @param node the AST node.
  void Mark(const ast::Node* node);

  ProgramBuilder* const builder_;
  diag::List& diagnostics_;
  std::unique_ptr<IntrinsicTable> const intrinsic_table_;
  BlockInfo* current_block_ = nullptr;
  ScopeStack<VariableInfo*> variable_stack_;
  std::unordered_map<Symbol, FunctionInfo*> symbol_to_function_;
  std::unordered_map<const ast::Function*, FunctionInfo*> function_to_info_;
  std::unordered_map<const ast::Variable*, VariableInfo*> variable_to_info_;
  std::unordered_map<ast::CallExpression*, FunctionCallInfo> function_calls_;
  std::unordered_map<const ast::Expression*, ExpressionInfo> expr_info_;
  std::unordered_map<sem::Type*, sem::Type*> type_to_canonical_;
  std::unordered_map<Symbol, sem::Type*> named_types_;
  std::unordered_set<const ast::Node*> marked_;
  FunctionInfo* current_function_ = nullptr;
  sem::Statement* current_statement_ = nullptr;
  BlockAllocator<VariableInfo> variable_infos_;
  BlockAllocator<FunctionInfo> function_infos_;
};

}  // namespace resolver
}  // namespace tint

#endif  // SRC_RESOLVER_RESOLVER_H_
