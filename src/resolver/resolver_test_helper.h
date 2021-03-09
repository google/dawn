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
  Resolver* r() const { return td_.get(); }

  /// Returns the statement that holds the given expression.
  /// @param expr the ast::Expression
  /// @return the ast::Statement of the ast::Expression, or nullptr if the
  /// expression is not owned by a statement.
  ast::Statement* StmtOf(ast::Expression* expr) {
    auto* sem_stmt = Sem().Get(expr)->Stmt();
    return sem_stmt ? sem_stmt->Declaration() : nullptr;
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
  std::unique_ptr<Resolver> td_;
};

class ResolverTest : public TestHelper, public testing::Test {};

template <typename T>
class ResolverTestWithParam : public TestHelper,
                              public testing::TestWithParam<T> {};

}  // namespace resolver
}  // namespace tint

#endif  // SRC_RESOLVER_RESOLVER_TEST_HELPER_H_
