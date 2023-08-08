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

#ifndef SRC_TINT_LANG_WGSL_SEM_VARIABLE_H_
#define SRC_TINT_LANG_WGSL_SEM_VARIABLE_H_

#include <optional>
#include <utility>
#include <vector>

#include "tint/override_id.h"

#include "src/tint/lang/core/access.h"
#include "src/tint/lang/core/address_space.h"
#include "src/tint/lang/core/type/type.h"
#include "src/tint/lang/wgsl/ast/parameter.h"
#include "src/tint/lang/wgsl/sem/parameter_usage.h"
#include "src/tint/lang/wgsl/sem/value_expression.h"
#include "src/tint/utils/containers/unique_vector.h"
#include "tint/binding_point.h"

// Forward declarations
namespace tint::ast {
class IdentifierExpression;
class Parameter;
class Variable;
}  // namespace tint::ast
namespace tint::sem {
class CallTarget;
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
    /// @param stage the evaluation stage for an expression of this variable type
    /// @param address_space the variable address space
    /// @param access the variable access control type
    /// @param constant_value the constant value for the variable. May be null
    Variable(const ast::Variable* declaration,
             const type::Type* type,
             core::EvaluationStage stage,
             core::AddressSpace address_space,
             core::Access access,
             const constant::Value* constant_value);

    /// Destructor
    ~Variable() override;

    /// @returns the AST declaration node
    const ast::Variable* Declaration() const { return declaration_; }

    /// @returns the canonical type for the variable
    const type::Type* Type() const { return type_; }

    /// @returns the evaluation stage for an expression of this variable type
    core::EvaluationStage Stage() const { return stage_; }

    /// @returns the address space for the variable
    core::AddressSpace AddressSpace() const { return address_space_; }

    /// @returns the access control for the variable
    core::Access Access() const { return access_; }

    /// @return the constant value of this expression
    const constant::Value* ConstantValue() const { return constant_value_; }

    /// @returns the variable initializer expression, or nullptr if the variable
    /// does not have one.
    const ValueExpression* Initializer() const { return initializer_; }

    /// Sets the variable initializer expression.
    /// @param initializer the initializer expression to assign to this variable.
    void SetInitializer(const ValueExpression* initializer) { initializer_ = initializer; }

    /// @returns the expressions that use the variable
    VectorRef<const VariableUser*> Users() const { return users_; }

    /// @param user the user to add
    void AddUser(const VariableUser* user) { users_.Push(user); }

  private:
    const ast::Variable* const declaration_;
    const type::Type* const type_;
    const core::EvaluationStage stage_;
    const core::AddressSpace address_space_;
    const core::Access access_;
    const constant::Value* constant_value_;
    const ValueExpression* initializer_ = nullptr;
    tint::Vector<const VariableUser*, 8> users_;
};

/// LocalVariable is a function-scope variable
class LocalVariable final : public Castable<LocalVariable, Variable> {
  public:
    /// Constructor
    /// @param declaration the AST declaration node
    /// @param type the variable type
    /// @param stage the evaluation stage for an expression of this variable type
    /// @param address_space the variable address space
    /// @param access the variable access control type
    /// @param statement the statement that declared this local variable
    /// @param constant_value the constant value for the variable. May be null
    LocalVariable(const ast::Variable* declaration,
                  const type::Type* type,
                  core::EvaluationStage stage,
                  core::AddressSpace address_space,
                  core::Access access,
                  const sem::Statement* statement,
                  const constant::Value* constant_value);

    /// Destructor
    ~LocalVariable() override;

    /// @returns the statement that declares this local variable
    const sem::Statement* Statement() const { return statement_; }

    /// @returns the Type, Function or Variable that this local variable shadows
    const CastableBase* Shadows() const { return shadows_; }

    /// Sets the Type, Function or Variable that this local variable shadows
    /// @param shadows the Type, Function or Variable that this variable shadows
    void SetShadows(const CastableBase* shadows) { shadows_ = shadows; }

  private:
    const sem::Statement* const statement_;
    const CastableBase* shadows_ = nullptr;
};

/// GlobalVariable is a module-scope variable
class GlobalVariable final : public Castable<GlobalVariable, Variable> {
  public:
    /// Constructor
    /// @param declaration the AST declaration node
    /// @param type the variable type
    /// @param stage the evaluation stage for an expression of this variable type
    /// @param address_space the variable address space
    /// @param access the variable access control type
    /// @param constant_value the constant value for the variable. May be null
    /// @param binding_point the optional resource binding point of the variable
    /// @param location the location value if provided
    /// @param index the index value if provided
    ///
    /// Note, a GlobalVariable generally doesn't have a `location` in WGSL, as it isn't allowed by
    /// the spec. The location maybe attached by transforms such as CanonicalizeEntryPointIO.
    GlobalVariable(const ast::Variable* declaration,
                   const type::Type* type,
                   core::EvaluationStage stage,
                   core::AddressSpace address_space,
                   core::Access access,
                   const constant::Value* constant_value,
                   std::optional<tint::BindingPoint> binding_point = std::nullopt,
                   std::optional<uint32_t> location = std::nullopt,
                   std::optional<uint32_t> index = std::nullopt);

    /// Destructor
    ~GlobalVariable() override;

    /// @returns the resource binding point for the variable
    std::optional<tint::BindingPoint> BindingPoint() const { return binding_point_; }

    /// @param id the constant identifier to assign to this variable
    void SetOverrideId(OverrideId id) { override_id_ = id; }

    /// @returns the pipeline constant ID associated with the variable
    tint::OverrideId OverrideId() const { return override_id_; }

    /// @returns the location value for the parameter, if set
    std::optional<uint32_t> Location() const { return location_; }

    /// @returns the index value for the parameter, if set
    std::optional<uint32_t> Index() const { return index_; }

  private:
    const std::optional<tint::BindingPoint> binding_point_;

    tint::OverrideId override_id_;
    std::optional<uint32_t> location_;
    std::optional<uint32_t> index_;
};

/// Parameter is a function parameter
class Parameter final : public Castable<Parameter, Variable> {
  public:
    /// Constructor for function parameters
    /// @param declaration the AST declaration node
    /// @param index the index of the parmeter in the function
    /// @param type the variable type
    /// @param address_space the variable address space
    /// @param access the variable access control type
    /// @param usage the semantic usage for the parameter
    /// @param binding_point the optional resource binding point of the parameter
    /// @param location the location value, if set
    Parameter(const ast::Parameter* declaration,
              uint32_t index,
              const type::Type* type,
              core::AddressSpace address_space,
              core::Access access,
              const ParameterUsage usage = ParameterUsage::kNone,
              std::optional<tint::BindingPoint> binding_point = {},
              std::optional<uint32_t> location = std::nullopt);

    /// Destructor
    ~Parameter() override;

    /// @returns the AST declaration node
    const ast::Parameter* Declaration() const {
        return static_cast<const ast::Parameter*>(Variable::Declaration());
    }

    /// @return the index of the parameter in the function
    uint32_t Index() const { return index_; }

    /// @returns the semantic usage for the parameter
    ParameterUsage Usage() const { return usage_; }

    /// @returns the CallTarget owner of this parameter
    CallTarget const* Owner() const { return owner_; }

    /// @param owner the CallTarget owner of this parameter
    void SetOwner(CallTarget const* owner) { owner_ = owner; }

    /// @returns the Type, Function or Variable that this local variable shadows
    const CastableBase* Shadows() const { return shadows_; }

    /// Sets the Type, Function or Variable that this local variable shadows
    /// @param shadows the Type, Function or Variable that this variable shadows
    void SetShadows(const CastableBase* shadows) { shadows_ = shadows; }

    /// @returns the resource binding point for the parameter
    std::optional<tint::BindingPoint> BindingPoint() const { return binding_point_; }

    /// @returns the location value for the parameter, if set
    std::optional<uint32_t> Location() const { return location_; }

  private:
    const uint32_t index_;
    const ParameterUsage usage_;
    CallTarget const* owner_ = nullptr;
    const CastableBase* shadows_ = nullptr;
    const std::optional<tint::BindingPoint> binding_point_;
    const std::optional<uint32_t> location_;
};

/// VariableUser holds the semantic information for an identifier expression
/// node that resolves to a variable.
class VariableUser final : public Castable<VariableUser, ValueExpression> {
  public:
    /// Constructor
    /// @param declaration the AST identifier node
    /// @param stage the evaluation stage for an expression of this variable type
    /// @param statement the statement that owns this expression
    /// @param constant the constant value of the expression. May be null
    /// @param variable the semantic variable
    VariableUser(const ast::IdentifierExpression* declaration,
                 core::EvaluationStage stage,
                 Statement* statement,
                 const constant::Value* constant,
                 sem::Variable* variable);
    ~VariableUser() override;

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
        return Hash(i.first, i.second);
    }
};

}  // namespace std

#endif  // SRC_TINT_LANG_WGSL_SEM_VARIABLE_H_
