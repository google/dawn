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

#ifndef SRC_WRITER_SPIRV_TEST_HELPER_H_
#define SRC_WRITER_SPIRV_TEST_HELPER_H_

#include <memory>
#include <utility>

#include "gtest/gtest.h"
#include "src/ast/builder.h"
#include "src/ast/module.h"
#include "src/type_determiner.h"
#include "src/writer/spirv/builder.h"

namespace tint {
namespace writer {
namespace spirv {

/// Helper class for testing
template <typename BASE>
class TestHelperBase : public ast::BuilderWithModule, public BASE {
 public:
  TestHelperBase() : td(mod) {}
  ~TestHelperBase() override = default;

  /// Builds and returns a spirv::Builder from the module.
  /// @note The spirv::Builder is only built once. Multiple calls to Build()
  /// will return the same spirv::Builder without rebuilding.
  /// @return the built spirv::Builder
  spirv::Builder& Build() {
    if (spirv_builder) {
      return *spirv_builder;
    }
    spirv_builder = std::make_unique<spirv::Builder>(mod);
    return *spirv_builder;
  }

  /// The type determiner
  TypeDeterminer td;

 protected:
  /// Called whenever a new variable is built with `Var()`.
  /// @param var the variable that was built
  void OnVariableBuilt(ast::Variable* var) override {
    td.RegisterVariableForTesting(var);
  }

 private:
  std::unique_ptr<spirv::Builder> spirv_builder;
};
using TestHelper = TestHelperBase<testing::Test>;

template <typename T>
using TestParamHelper = TestHelperBase<testing::TestWithParam<T>>;

}  // namespace spirv
}  // namespace writer
}  // namespace tint

#endif  // SRC_WRITER_SPIRV_TEST_HELPER_H_
