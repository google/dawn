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
#include "src/ast/module.h"
#include "src/program_builder.h"
#include "src/type_determiner.h"
#include "src/writer/msl/generator_impl.h"

namespace tint {
namespace writer {
namespace msl {

/// Helper class for testing
template <typename BASE>
class TestHelperBase : public BASE, public ProgramBuilder {
 public:
  TestHelperBase() : td(this) {}
  ~TestHelperBase() = default;

  /// Builds and returns a GeneratorImpl from the program.
  /// @note The generator is only built once. Multiple calls to Build() will
  /// return the same GeneratorImpl without rebuilding.
  /// @return the built generator
  GeneratorImpl& Build() {
    if (gen_) {
      return *gen_;
    }
    program = std::make_unique<Program>(std::move(*this));
    gen_ = std::make_unique<GeneratorImpl>(program.get());
    return *gen_;
  }

  /// The type determiner
  TypeDeterminer td;
  /// The program built with a call to Build()
  std::unique_ptr<Program> program;

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
