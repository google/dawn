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

#ifndef SRC_VALIDATOR_VALIDATOR_TEST_HELPER_H_
#define SRC_VALIDATOR_VALIDATOR_TEST_HELPER_H_

#include <memory>
#include <utility>
#include <vector>

#include "gtest/gtest.h"
#include "src/program_builder.h"
#include "src/semantic/expression.h"
#include "src/validator/validator_impl.h"

namespace tint {

/// A helper for testing validation
class ValidatorTestHelper : public ProgramBuilder {
 public:
  /// Constructor
  ValidatorTestHelper();
  ~ValidatorTestHelper() override;

  /// Builds and returns a validator from the program.
  /// @note The validator is only built once. Multiple calls to Build() will
  /// return the same ValidatorImpl without rebuilding.
  /// @return the built validator
  ValidatorImpl& Build() {
    if (val_) {
      return *val_;
    }
    program_ = std::make_unique<Program>(std::move(*this));
    [&]() {
      ASSERT_TRUE(program_->IsValid())
          << diag::Formatter().format(program_->Diagnostics());
    }();
    val_ = std::make_unique<ValidatorImpl>(program_.get());
    for (auto* var : vars_for_testing_) {
      val_->RegisterVariableForTesting(var);
    }
    return *val_;
  }

  /// Inserts a variable into the current scope.
  /// @param var the variable to register.
  void RegisterVariable(ast::Variable* var) {
    AST().AddGlobalVariable(var);
    vars_for_testing_.emplace_back(var);
  }

  /// Helper for returning the resolved semantic type of the expression `expr`
  /// from the built program.
  /// @param expr the AST expression
  /// @return the resolved semantic type for the expression, or nullptr if the
  /// expression has no resolved type.
  type::Type* TypeOf(ast::Expression* expr) const {
    auto* sem = program_->Sem().Get(expr);
    return sem ? sem->Type() : nullptr;
  }

 private:
  std::unique_ptr<Program> program_;
  std::unique_ptr<ValidatorImpl> val_;
  std::vector<ast::Variable*> vars_for_testing_;
};

}  // namespace tint

#endif  // SRC_VALIDATOR_VALIDATOR_TEST_HELPER_H_
