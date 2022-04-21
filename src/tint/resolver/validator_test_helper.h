// Copyright 2022 The Tint Authors.
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

#ifndef SRC_TINT_RESOLVER_VALIDATOR_TEST_HELPER_H_
#define SRC_TINT_RESOLVER_VALIDATOR_TEST_HELPER_H_

#include <memory>

#include "gtest/gtest.h"
#include "src/tint/program_builder.h"
#include "src/tint/resolver/validator.h"

namespace tint::resolver {

/// Helper class for testing
class TestHelper : public ProgramBuilder {
 public:
  /// Constructor
  TestHelper();

  /// Destructor
  ~TestHelper() override;

  /// @return a pointer to the Validator
  Validator* v() const { return validator_.get(); }

 private:
  std::unique_ptr<Validator> validator_;
};

class ValidatorTest : public TestHelper, public testing::Test {};

}  // namespace tint::resolver

#endif  // SRC_TINT_RESOLVER_VALIDATOR_TEST_HELPER_H_
