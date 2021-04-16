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
#include "src/semantic/struct.h"
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
namespace semantic {
class Array;
class Statement;
}  // namespace semantic
namespace type {
class Struct;
}  // namespace type

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
  static bool IsStorable(type::Type* type);

  /// @param type the given type
  /// @returns true if the given type is host-shareable
  static bool IsHostShareable(type::Type* type);

  /// @param lhs the assignment store type (non-pointer)
  /// @param rhs the assignment source type (non-pointer or pointer with
  /// auto-deref)
  /// @returns true an expression of type `rhs` can be assigned to a variable,
  /// structure member or array element of type `lhs`
  static bool IsValidAssignment(type::Type* lhs, type::Type* rhs);

  /// @param type the input type
  /// @returns the canonical type for `type`; that is, a type with all aliases
  /// removed. For example, `Canonical(alias<alias<vec3<alias<f32>>>>)` is
  /// `vec3<f32>`.
  type::Type* Canonical(type::Type* type);

 private:
  /// Structure holding semantic information about a variable.
  /// Used to build the semantic::Variable nodes at the end of resolving.
  struct VariableInfo {
    VariableInfo(ast::Variable* decl, type::Type* type);
    ~VariableInfo();

    ast::Variable* const declaration;
    type::Type* type;
    ast::StorageClass storage_class;
    std::vector<ast::IdentifierExpression*> users;
  };

  /// Structure holding semantic information about a function.
  /// Used to build the semantic::Function nodes at the end of resolving.
  struct FunctionInfo {
    explicit FunctionInfo(ast::Function* decl);
    ~FunctionInfo();

    ast::Function* const declaration;
    std::vector<VariableInfo*> parameters;
    UniqueVector<VariableInfo*> referenced_module_vars;
    UniqueVector<VariableInfo*> local_referenced_module_vars;
    std::vector<const ast::ReturnStatement*> return_statements;

    // List of transitive calls this function makes
    UniqueVector<FunctionInfo*> transitive_calls;
  };

  /// Structure holding semantic information about an expression.
  /// Used to build the semantic::Expression nodes at the end of resolving.
  struct ExpressionInfo {
    type::Type* type;
    semantic::Statement* statement;
  };

  /// Structure holding semantic information about a call expression to an
  /// ast::Function.
  /// Used to build the semantic::Call nodes at the end of resolving.
  struct FunctionCallInfo {
    FunctionInfo* function;
    semantic::Statement* statement;
  };

  /// Structure holding semantic information about a struct.
  /// Used to build the semantic::Struct nodes at the end of resolving.
  struct StructInfo {
    StructInfo();
    ~StructInfo();

    std::vector<const semantic::StructMember*> members;
    uint32_t align = 0;
    uint32_t size = 0;
    uint32_t size_no_padding = 0;
    std::unordered_set<ast::StorageClass> storage_class_usage;
    std::unordered_set<semantic::PipelineStageUsage> pipeline_stage_uses;
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

  /// Creates the nodes and adds them to the semantic::Info mappings of the
  /// ProgramBuilder.
  void CreateSemanticNodes() const;

  /// Retrieves information for the requested import.
  /// @param src the source of the import
  /// @param path the import path
  /// @param name the method name to get information on
  /// @param params the parameters to the method call
  /// @param id out parameter for the external call ID. Must not be a nullptr.
  /// @returns the return type of `name` in `path` or nullptr on error.
  type::Type* GetImportData(const Source& src,
                            const std::string& path,
                            const std::string& name,
                            const ast::ExpressionList& params,
                            uint32_t* id);

  void set_referenced_from_function_if_needed(VariableInfo* var, bool local);

  // AST and Type traversal methods
  // Each return true on success, false on failure.
  bool ArrayAccessor(ast::ArrayAccessorExpression*);
  bool Binary(ast::BinaryExpression*);
  bool Bitcast(ast::BitcastExpression*);
  bool BlockStatement(const ast::BlockStatement*);
  bool Call(ast::CallExpression*);
  bool CaseStatement(ast::CaseStatement*);
  bool Constructor(ast::ConstructorExpression*);
  bool VectorConstructor(const type::Vector* vec_type,
                         const ast::ExpressionList& values);
  bool MatrixConstructor(const type::Matrix* matrix_type,
                         const ast::ExpressionList& values);
  bool Expression(ast::Expression*);
  bool Expressions(const ast::ExpressionList&);
  bool Function(ast::Function*);
  bool Identifier(ast::IdentifierExpression*);
  bool IfStatement(ast::IfStatement*);
  bool IntrinsicCall(ast::CallExpression*, semantic::IntrinsicType);
  bool MemberAccessor(ast::MemberAccessorExpression*);
  bool Statement(ast::Statement*);
  bool Statements(const ast::StatementList&);
  bool UnaryOp(ast::UnaryOpExpression*);
  bool VariableDeclStatement(const ast::VariableDeclStatement*);
  bool Return(ast::ReturnStatement* ret);
  bool Switch(ast::SwitchStatement* s);
  bool Assignment(ast::AssignmentStatement* a);
  bool GlobalVariable(ast::Variable* var);

  // AST and Type validation methods
  // Each return true on success, false on failure.
  bool ValidateBinary(ast::BinaryExpression* expr);
  bool ValidateVariable(const ast::Variable* param);
  bool ValidateParameter(const ast::Variable* param);
  bool ValidateFunction(const ast::Function* func);
  bool ValidateEntryPoint(const ast::Function* func);
  bool ValidateStructure(const type::Struct* st);
  bool ValidateReturn(const ast::ReturnStatement* ret);
  bool ValidateSwitch(const ast::SwitchStatement* s);
  bool ValidateAssignment(const ast::AssignmentStatement* a);

  /// @returns the semantic information for the array `arr`, building it if it
  /// hasn't been constructed already. If an error is raised, nullptr is
  /// returned.
  /// @param arr the Array to get semantic information for
  /// @param source the Source of the ast node with this array as its type
  const semantic::Array* Array(type::Array* arr, const Source& source);

  /// @returns the StructInfo for the structure `str`, building it if it hasn't
  /// been constructed already. If an error is raised, nullptr is returned.
  StructInfo* Structure(type::Struct* str);

  /// @returns the VariableInfo for the variable `var`, building it if it hasn't
  /// been constructed already. If an error is raised, nullptr is returned.
  /// @param var the variable to create or return the `VariableInfo` for
  /// @param type optional type of `var` to use instead of
  /// `var->declared_type()`. For type inference.
  VariableInfo* Variable(ast::Variable* var, type::Type* type = nullptr);

  /// Records the storage class usage for the given type, and any transient
  /// dependencies of the type. Validates that the type can be used for the
  /// given storage class, erroring if it cannot.
  /// @param sc the storage class to apply to the type and transitent types
  /// @param ty the type to apply the storage class on
  /// @param usage the Source of the root variable declaration that uses the
  /// given type and storage class. Used for generating sensible error messages.
  /// @returns true on success, false on error
  bool ApplyStorageClassUsageToType(ast::StorageClass sc,
                                    type::Type* ty,
                                    const Source& usage);

  /// @param align the output default alignment in bytes for the type `ty`
  /// @param size the output default size in bytes for the type `ty`
  /// @param source the Source of the variable declaration of type `ty`
  /// @returns true on success, false on error
  bool DefaultAlignAndSize(type::Type* ty,
                           uint32_t& align,
                           uint32_t& size,
                           const Source& source);

  /// @returns the resolved type of the ast::Expression `expr`
  /// @param expr the expression
  type::Type* TypeOf(ast::Expression* expr);

  /// Creates a semantic::Expression node with the resolved type `type`, and
  /// assigns this semantic node to the expression `expr`.
  /// @param expr the expression
  /// @param type the resolved type
  void SetType(ast::Expression* expr, type::Type* type);

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
  std::string VectorPretty(uint32_t size, type::Type* element_type);

  ProgramBuilder* const builder_;
  std::unique_ptr<IntrinsicTable> const intrinsic_table_;
  diag::List diagnostics_;
  BlockInfo* current_block_ = nullptr;
  ScopeStack<VariableInfo*> variable_stack_;
  std::unordered_map<Symbol, FunctionInfo*> symbol_to_function_;
  std::unordered_map<const ast::Function*, FunctionInfo*> function_to_info_;
  std::unordered_map<const ast::Variable*, VariableInfo*> variable_to_info_;
  std::unordered_map<ast::CallExpression*, FunctionCallInfo> function_calls_;
  std::unordered_map<ast::Expression*, ExpressionInfo> expr_info_;
  std::unordered_map<type::Struct*, StructInfo*> struct_info_;
  std::unordered_map<type::Type*, type::Type*> type_to_canonical_;
  FunctionInfo* current_function_ = nullptr;
  semantic::Statement* current_statement_ = nullptr;
  BlockAllocator<VariableInfo> variable_infos_;
  BlockAllocator<FunctionInfo> function_infos_;
  BlockAllocator<StructInfo> struct_infos_;
};

}  // namespace resolver
}  // namespace tint

#endif  // SRC_RESOLVER_RESOLVER_H_
