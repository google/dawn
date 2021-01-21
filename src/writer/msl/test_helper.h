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

#ifndef SRC_WRITER_MSL_TEST_HELPER_H_
#define SRC_WRITER_MSL_TEST_HELPER_H_

#include <memory>
#include <utility>

#include "gtest/gtest.h"
#include "src/ast/builder.h"
#include "src/ast/module.h"
#include "src/type_determiner.h"
#include "src/writer/msl/generator_impl.h"

namespace tint {
namespace writer {
namespace msl {

/// Helper class for testing
template <typename BASE>
class TestHelperBase : public BASE, public ast::BuilderWithModule {
 public:
  TestHelperBase() : td(mod) {}
  ~TestHelperBase() = default;

  /// Builds and returns a GeneratorImpl from the module.
  /// @note The generator is only built once. Multiple calls to Build() will
  /// return the same GeneratorImpl without rebuilding.
  /// @return the built generator
  GeneratorImpl& Build() {
    if (gen_) {
      return *gen_;
    }
    gen_ = std::make_unique<GeneratorImpl>(mod);
    return *gen_;
  }

  /// The type determiner
  TypeDeterminer td;

 private:
  std::unique_ptr<GeneratorImpl> gen_;
};
using TestHelper = TestHelperBase<testing::Test>;

template <typename T>
using TestParamHelper = TestHelperBase<testing::TestWithParam<T>>;

}  // namespace msl
}  // namespace writer
}  // namespace tint

#endif  // SRC_WRITER_MSL_TEST_HELPER_H_
