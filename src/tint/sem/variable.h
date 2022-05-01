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

#ifndef SRC_TINT_SEM_VARIABLE_H_
#define SRC_TINT_SEM_VARIABLE_H_

#include <utility>
#include <vector>

#include "src/tint/ast/access.h"
#include "src/tint/ast/storage_class.h"
#include "src/tint/sem/binding_point.h"
#include "src/tint/sem/expression.h"
#include "src/tint/sem/parameter_usage.h"

// Forward declarations
namespace tint::ast {
class IdentifierExpression;
class Variable;
}  // namespace tint::ast
namespace tint::sem {
class CallTarget;
class Type;
class VariableUser;
}  // namespace tint::sem

namespace tint::sem {

/// Variable is the base class for local variables, global variables and
/// parameters.
class Variable : public Castable<Variable, Node> {
  public:
    /// Constructor
    /// @param declaration the AST declaration node
    /// @param type the variable type
    /// @param storage_class the variable storage class
    /// @param access the variable access control type
    /// @param constant_value the constant value for the variable. May be invalid
    Variable(const ast::Variable* declaration,
             const sem::Type* type,
             ast::StorageClass storage_class,
             ast::Access access,
             Constant constant_value);

    /// Destructor
    ~Variable() override;

    /// @returns the AST declaration node
    const ast::Variable* Declaration() const { return declaration_; }

    /// @returns the canonical type for the variable
    const sem::Type* Type() const { return type_; }

    /// @returns the storage class for the variable
    ast::StorageClass StorageClass() const { return storage_class_; }

    /// @returns the access control for the variable
    ast::Access Access() const { return access_; }

    /// @return the constant value of this expression
    const Constant& ConstantValue() const { return constant_value_; }

    /// @returns the variable constructor expression, or nullptr if the variable
    /// does not have one.
    const Expression* Constructor() const { return constructor_; }

    /// Sets the variable constructor expression.
    /// @param constructor the constructor expression to assign to this variable.
    void SetConstructor(const Expression* constructor) { constructor_ = constructor; }

    /// @returns the expressions that use the variable
    const std::vector<const VariableUser*>& Users() const { return users_; }

    /// @param user the user to add
    void AddUser(const VariableUser* user) { users_.emplace_back(user); }

  private:
    const ast::Variable* const declaration_;
    const sem::Type* const type_;
    const ast::StorageClass storage_class_;
    const ast::Access access_;
    const Constant constant_value_;
    const Expression* constructor_ = nullptr;
    std::vector<const VariableUser*> users_;
};

/// LocalVariable is a function-scope variable
class LocalVariable final : public Castable<LocalVariable, Variable> {
  public:
    /// Constructor
    /// @param declaration the AST declaration node
    /// @param type the variable type
    /// @param storage_class the variable storage class
    /// @param access the variable access control type
    /// @param statement the statement that declared this local variable
    /// @param constant_value the constant value for the variable. May be invalid
    LocalVariable(const ast::Variable* declaration,
                  const sem::Type* type,
                  ast::StorageClass storage_class,
                  ast::Access access,
                  const sem::Statement* statement,
                  Constant constant_value);

    /// Destructor
    ~LocalVariable() override;

    /// @returns the statement that declares this local variable
    const sem::Statement* Statement() const { return statement_; }

    /// @returns the Type, Function or Variable that this local variable shadows
    const sem::Node* Shadows() const { return shadows_; }

    /// Sets the Type, Function or Variable that this local variable shadows
    /// @param shadows the Type, Function or Variable that this variable shadows
    void SetShadows(const sem::Node* shadows) { shadows_ = shadows; }

  private:
    const sem::Statement* const statement_;
    const sem::Node* shadows_ = nullptr;
};

/// GlobalVariable is a module-scope variable
class GlobalVariable final : public Castable<GlobalVariable, Variable> {
  public:
    /// Constructor
    /// @param declaration the AST declaration node
    /// @param type the variable type
    /// @param storage_class the variable storage class
    /// @param access the variable access control type
    /// @param constant_value the constant value for the variable. May be invalid
    /// @param binding_point the optional resource binding point of the variable
    GlobalVariable(const ast::Variable* declaration,
                   const sem::Type* type,
                   ast::StorageClass storage_class,
                   ast::Access access,
                   Constant constant_value,
                   sem::BindingPoint binding_point = {});

    /// Destructor
    ~GlobalVariable() override;

    /// @returns the resource binding point for the variable
    sem::BindingPoint BindingPoint() const { return binding_point_; }

    /// @param id the constant identifier to assign to this variable
    void SetConstantId(uint16_t id) {
        constant_id_ = id;
        is_overridable_ = true;
    }

    /// @returns the pipeline constant ID associated with the variable
    uint16_t ConstantId() const { return constant_id_; }

    /// @param is_overridable true if this is a pipeline overridable constant
    void SetIsOverridable(bool is_overridable = true) { is_overridable_ = is_overridable; }

    /// @returns true if this is pipeline overridable constant
    bool IsOverridable() const { return is_overridable_; }

  private:
    const sem::BindingPoint binding_point_;

    bool is_overridable_ = false;
    uint16_t constant_id_ = 0;
};

/// Parameter is a function parameter
class Parameter final : public Castable<Parameter, Variable> {
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

    /// @returns the Type, Function or Variable that this local variable shadows
    const sem::Node* Shadows() const { return shadows_; }

    /// Sets the Type, Function or Variable that this local variable shadows
    /// @param shadows the Type, Function or Variable that this variable shadows
    void SetShadows(const sem::Node* shadows) { shadows_ = shadows; }

  private:
    const uint32_t index_;
    const ParameterUsage usage_;
    CallTarget const* owner_ = nullptr;
    const sem::Node* shadows_ = nullptr;
};

/// ParameterList is a list of Parameter
using ParameterList = std::vector<const Parameter*>;

/// VariableUser holds the semantic information for an identifier expression
/// node that resolves to a variable.
class VariableUser final : public Castable<VariableUser, Expression> {
  public:
    /// Constructor
    /// @param declaration the AST identifier node
    /// @param statement the statement that owns this expression
    /// @param variable the semantic variable
    VariableUser(const ast::IdentifierExpression* declaration,
                 Statement* statement,
                 sem::Variable* variable);

    /// @returns the variable that this expression refers to
    const sem::Variable* Variable() const { return variable_; }

  private:
    const sem::Variable* const variable_;
};

/// A pair of sem::Variables. Can be hashed.
typedef std::pair<const Variable*, const Variable*> VariablePair;

}  // namespace tint::sem

namespace std {

/// Custom std::hash specialization for VariablePair
template <>
class hash<tint::sem::VariablePair> {
  public:
    /// @param i the variable pair to create a hash for
    /// @return the hash value
    inline std::size_t operator()(const tint::sem::VariablePair& i) const {
        return tint::utils::Hash(i.first, i.second);
    }
};

}  // namespace std

#endif  // SRC_TINT_SEM_VARIABLE_H_
