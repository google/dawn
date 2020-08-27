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

#ifndef SRC_VALIDATOR_TEST_HELPER_H_
#define SRC_VALIDATOR_TEST_HELPER_H_

#include "src/ast/type/void_type.h"
#include "src/type_determiner.h"
#include "src/validator_impl.h"

namespace tint {

/// A helper for testing validation
class ValidatorTestHelper {
 public:
  /// Constructor
  ValidatorTestHelper();
  ~ValidatorTestHelper();

  /// A handle to validator
  /// @returns a pointer to the validator
  ValidatorImpl* v() const { return v_.get(); }
  /// A handle to type_determiner
  /// @returns a pointer to the type_determiner object
  TypeDeterminer* td() const { return td_.get(); }
  /// A handle to the created module
  /// @return a pointer to the test module
  ast::Module* mod() { return &mod_; }
  /// Creates a function and add an entry point to it
  void AddFakeEntryPoint();

 private:
  std::unique_ptr<ValidatorImpl> v_;
  Context ctx_;
  ast::Module mod_;
  std::unique_ptr<TypeDeterminer> td_;
  ast::type::VoidType void_type_;
};

}  // namespace tint

#endif  // SRC_VALIDATOR_TEST_HELPER_H_
