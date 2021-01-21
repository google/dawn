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

#include "src/ast/builder.h"
#include "src/type/void_type.h"
#include "src/type_determiner.h"
#include "src/validator/validator_impl.h"

namespace tint {

/// A helper for testing validation
class ValidatorTestHelper : public ast::BuilderWithModule {
 public:
  /// Constructor
  ValidatorTestHelper();
  ~ValidatorTestHelper() override;

  /// A handle to validator
  /// @returns a pointer to the validator
  ValidatorImpl* v() const { return v_.get(); }
  /// A handle to type_determiner
  /// @returns a pointer to the type_determiner object
  TypeDeterminer* td() const { return td_.get(); }

  /// Inserts a variable into the current scope.
  /// @param var the variable to register.
  void RegisterVariable(ast::Variable* var) {
    v_->RegisterVariableForTesting(var);
    td_->RegisterVariableForTesting(var);
  }

 private:
  std::unique_ptr<ValidatorImpl> v_;
  std::unique_ptr<TypeDeterminer> td_;
};

}  // namespace tint

#endif  // SRC_VALIDATOR_VALIDATOR_TEST_HELPER_H_
