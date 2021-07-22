// Copyright 2021 The Tint Authors.
//
// Licensed under the Apache License, Version 2.0(the "License");
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

#ifndef SRC_SEM_VARIABLE_H_
#define SRC_SEM_VARIABLE_H_

#include <vector>

#include "src/ast/access.h"
#include "src/ast/storage_class.h"
#include "src/sem/binding_point.h"
#include "src/sem/expression.h"
#include "src/sem/parameter_usage.h"

namespace tint {

// Forward declarations
namespace ast {
class IdentifierExpression;
class Variable;
}  // namespace ast

namespace sem {

// Forward declarations
class CallTarget;
class Type;
class VariableUser;

/// Variable is the base class for local variables, global variables and
/// parameters.
class Variable : public Castable<Variable, Node> {
 public:
  /// Constructor
  /// @param declaration the AST declaration node
  /// @param type the variable type
  /// @param storage_class the variable storage class
  /// @param access the variable access control type
  Variable(const ast::Variable* declaration,
           const sem::Type* type,
           ast::StorageClass storage_class,
           ast::Access access);

  /// Destructor
  ~Variable() override;

  /// @returns the AST declaration node
  const ast::Variable* Declaration() const { return declaration_; }

  /// @returns the canonical type for the variable
  sem::Type* Type() const { return const_cast<sem::Type*>(type_); }

  /// @returns the storage class for the variable
  ast::StorageClass StorageClass() const { return storage_class_; }

  /// @returns the access control for the variable
  ast::Access Access() const { return access_; }

  /// @returns the expressions that use the variable
  const std::vector<const VariableUser*>& Users() const { return users_; }

  /// @param user the user to add
  void AddUser(const VariableUser* user) { users_.emplace_back(user); }

 private:
  const ast::Variable* const declaration_;
  const sem::Type* const type_;
  ast::StorageClass const storage_class_;
  ast::Access const access_;
  std::vector<const VariableUser*> users_;
};

/// LocalVariable is a function-scope variable
class LocalVariable : public Castable<LocalVariable, Variable> {
 public:
  /// Constructor
  /// @param declaration the AST declaration node
  /// @param type the variable type
  /// @param storage_class the variable storage class
  /// @param access the variable access control type
  LocalVariable(const ast::Variable* declaration,
                const sem::Type* type,
                ast::StorageClass storage_class,
                ast::Access access);

  /// Destructor
  ~LocalVariable() override;
};

/// GlobalVariable is a module-scope variable
class GlobalVariable : public Castable<GlobalVariable, Variable> {
 public:
  /// Constructor for non-overridable constants
  /// @param declaration the AST declaration node
  /// @param type the variable type
  /// @param storage_class the variable storage class
  /// @param access the variable access control type
  /// @param binding_point the optional resource binding point of the variable
  GlobalVariable(const ast::Variable* declaration,
                 const sem::Type* type,
                 ast::StorageClass storage_class,
                 ast::Access access,
                 sem::BindingPoint binding_point = {});

  /// Constructor for overridable pipeline constants
  /// @param declaration the AST declaration node
  /// @param type the variable type
  /// @param constant_id the pipeline constant ID
  GlobalVariable(const ast::Variable* declaration,
                 const sem::Type* type,
                 uint16_t constant_id);

  /// Destructor
  ~GlobalVariable() override;

  /// @returns the resource binding point for the variable
  sem::BindingPoint BindingPoint() const { return binding_point_; }

  /// @returns the pipeline constant ID associated with the variable
  uint16_t ConstantId() const { return constant_id_; }

  /// @returns true if this variable is an overridable pipeline constant
  bool IsPipelineConstant() const { return is_pipeline_constant_; }

 private:
  sem::BindingPoint binding_point_;
  bool const is_pipeline_constant_;
  uint16_t const constant_id_ = 0;
};

/// Parameter is a function parameter
class Parameter : public Castable<Parameter, Variable> {
 public:
  /// Constructor for function parameters
  /// @param declaration the AST declaration node
  /// @param index the index of the parmeter in the function
  /// @param type the variable type
  /// @param storage_class the variable storage class
  /// @param access the variable access control type
  /// @param usage the semantic usage for the parameter
  Parameter(const ast::Variable* declaration,
            uint32_t index,
            const sem::Type* type,
            ast::StorageClass storage_class,
            ast::Access access,
            const ParameterUsage usage = ParameterUsage::kNone);

  /// Destructor
  ~Parameter() override;

  /// @return the index of the parmeter in the function
  uint32_t Index() const { return index_; }

  /// @returns the semantic usage for the parameter
  ParameterUsage Usage() const { return usage_; }

  /// @returns the CallTarget owner of this parameter
  CallTarget const* Owner() const { return owner_; }

  /// @param owner the CallTarget owner of this parameter
  void SetOwner(CallTarget const* owner) { owner_ = owner; }

 private:
  uint32_t const index_;
  ParameterUsage const usage_;
  CallTarget const* owner_;
};

/// ParameterList is a list of Parameter
using ParameterList = std::vector<const Parameter*>;

/// VariableUser holds the semantic information for an identifier expression
/// node that resolves to a variable.
class VariableUser : public Castable<VariableUser, Expression> {
 public:
  /// Constructor
  /// @param declaration the AST identifier node
  /// @param type the resolved type of the expression
  /// @param statement the statement that owns this expression
  /// @param variable the semantic variable
  /// @param constant_value the constant value for the variable. May be invalid
  VariableUser(ast::IdentifierExpression* declaration,
               const sem::Type* type,
               Statement* statement,
               sem::Variable* variable,
               Constant constant_value);

  /// @returns the variable that this expression refers to
  const sem::Variable* Variable() const { return variable_; }

 private:
  sem::Variable const* const variable_;
};

}  // namespace sem
}  // namespace tint

#endif  // SRC_SEM_VARIABLE_H_
