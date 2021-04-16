// Copyright 2021 The Tint Authors.
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

#ifndef SRC_RESOLVER_RESOLVER_TEST_HELPER_H_
#define SRC_RESOLVER_RESOLVER_TEST_HELPER_H_

#include <memory>
#include <vector>

#include "gtest/gtest.h"
#include "src/program_builder.h"
#include "src/resolver/resolver.h"
#include "src/semantic/expression.h"
#include "src/semantic/statement.h"
#include "src/semantic/variable.h"

namespace tint {
namespace resolver {

/// Helper class for testing
class TestHelper : public ProgramBuilder {
 public:
  /// Constructor
  TestHelper();

  /// Destructor
  ~TestHelper() override;

  /// @return a pointer to the Resolver
  Resolver* r() const { return resolver_.get(); }

  /// Returns the statement that holds the given expression.
  /// @param expr the ast::Expression
  /// @return the ast::Statement of the ast::Expression, or nullptr if the
  /// expression is not owned by a statement.
  const ast::Statement* StmtOf(ast::Expression* expr) {
    auto* sem_stmt = Sem().Get(expr)->Stmt();
    return sem_stmt ? sem_stmt->Declaration() : nullptr;
  }

  /// Returns the BlockStatement that holds the given statement.
  /// @param stmt the ast::Statment
  /// @return the ast::BlockStatement that holds the ast::Statement, or nullptr
  /// if the statement is not owned by a BlockStatement.
  const ast::BlockStatement* BlockOf(ast::Statement* stmt) {
    auto* sem_stmt = Sem().Get(stmt);
    return sem_stmt ? sem_stmt->Block() : nullptr;
  }

  /// Returns the BlockStatement that holds the given expression.
  /// @param expr the ast::Expression
  /// @return the ast::Statement of the ast::Expression, or nullptr if the
  /// expression is not indirectly owned by a BlockStatement.
  const ast::BlockStatement* BlockOf(ast::Expression* expr) {
    auto* sem_stmt = Sem().Get(expr)->Stmt();
    return sem_stmt ? sem_stmt->Block() : nullptr;
  }

  /// Returns the semantic variable for the given identifier expression.
  /// @param expr the identifier expression
  /// @return the resolved semantic::Variable of the identifier, or nullptr if
  /// the expression did not resolve to a variable.
  const semantic::Variable* VarOf(ast::Expression* expr) {
    auto* sem_ident = Sem().Get(expr);
    auto* var_user =
        sem_ident ? sem_ident->As<semantic::VariableUser>() : nullptr;
    return var_user ? var_user->Variable() : nullptr;
  }

  /// Checks that all the users of the given variable are as expected
  /// @param var the variable to check
  /// @param expected_users the expected users of the variable
  /// @return true if all users are as expected
  bool CheckVarUsers(ast::Variable* var,
                     std::vector<ast::Expression*>&& expected_users) {
    auto& var_users = Sem().Get(var)->Users();
    if (var_users.size() != expected_users.size()) {
      return false;
    }
    for (size_t i = 0; i < var_users.size(); i++) {
      if (var_users[i]->Declaration() != expected_users[i]) {
        return false;
      }
    }
    return true;
  }

 private:
  std::unique_ptr<Resolver> resolver_;
};

class ResolverTest : public TestHelper, public testing::Test {};

template <typename T>
class ResolverTestWithParam : public TestHelper,
                              public testing::TestWithParam<T> {};

inline type::Type* ty_bool_(const ProgramBuilder::TypesBuilder& ty) {
  return ty.bool_();
}
inline type::Type* ty_i32(const ProgramBuilder::TypesBuilder& ty) {
  return ty.i32();
}
inline type::Type* ty_u32(const ProgramBuilder::TypesBuilder& ty) {
  return ty.u32();
}
inline type::Type* ty_f32(const ProgramBuilder::TypesBuilder& ty) {
  return ty.f32();
}

using create_type_func_ptr =
    type::Type* (*)(const ProgramBuilder::TypesBuilder& ty);

template <typename T>
type::Type* ty_vec2(const ProgramBuilder::TypesBuilder& ty) {
  return ty.vec2<T>();
}

template <create_type_func_ptr create_type>
type::Type* ty_vec2(const ProgramBuilder::TypesBuilder& ty) {
  auto* type = create_type(ty);
  return ty.vec2(type);
}

template <typename T>
type::Type* ty_vec3(const ProgramBuilder::TypesBuilder& ty) {
  return ty.vec3<T>();
}

template <create_type_func_ptr create_type>
type::Type* ty_vec3(const ProgramBuilder::TypesBuilder& ty) {
  auto* type = create_type(ty);
  return ty.vec3(type);
}

template <typename T>
type::Type* ty_vec4(const ProgramBuilder::TypesBuilder& ty) {
  return ty.vec4<T>();
}

template <create_type_func_ptr create_type>
type::Type* ty_vec4(const ProgramBuilder::TypesBuilder& ty) {
  auto* type = create_type(ty);
  return ty.vec4(type);
}

template <typename T>
type::Type* ty_mat2x2(const ProgramBuilder::TypesBuilder& ty) {
  return ty.mat2x2<T>();
}

template <create_type_func_ptr create_type>
type::Type* ty_mat2x2(const ProgramBuilder::TypesBuilder& ty) {
  auto* type = create_type(ty);
  return ty.mat2x2(type);
}

template <typename T>
type::Type* ty_mat3x3(const ProgramBuilder::TypesBuilder& ty) {
  return ty.mat3x3<T>();
}

template <create_type_func_ptr create_type>
type::Type* ty_mat3x3(const ProgramBuilder::TypesBuilder& ty) {
  auto* type = create_type(ty);
  return ty.mat3x3(type);
}

template <typename T>
type::Type* ty_mat4x4(const ProgramBuilder::TypesBuilder& ty) {
  return ty.mat4x4<T>();
}

template <create_type_func_ptr create_type>
type::Type* ty_mat4x4(const ProgramBuilder::TypesBuilder& ty) {
  auto* type = create_type(ty);
  return ty.mat4x4(type);
}

template <create_type_func_ptr create_type>
type::Type* ty_alias(const ProgramBuilder::TypesBuilder& ty) {
  auto* type = create_type(ty);
  return ty.alias("alias_" + type->type_name(), type);
}

}  // namespace resolver
}  // namespace tint

#endif  // SRC_RESOLVER_RESOLVER_TEST_HELPER_H_
