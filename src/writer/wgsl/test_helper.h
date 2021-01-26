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

#ifndef SRC_WRITER_WGSL_TEST_HELPER_H_
#define SRC_WRITER_WGSL_TEST_HELPER_H_

#include <memory>
#include <utility>

#include "gtest/gtest.h"
#include "src/program_builder.h"
#include "src/type_determiner.h"
#include "src/writer/wgsl/generator_impl.h"

namespace tint {
namespace writer {
namespace wgsl {

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
    program_ = std::make_unique<Program>(std::move(*this));
    gen_ = std::make_unique<GeneratorImpl>(program_.get());
    return *gen_;
  }

  /// The type determiner
  TypeDeterminer td;

 private:
  std::unique_ptr<Program> program_;
  std::unique_ptr<GeneratorImpl> gen_;
};
using TestHelper = TestHelperBase<testing::Test>;

template <typename T>
using TestParamHelper = TestHelperBase<testing::TestWithParam<T>>;

}  // namespace wgsl
}  // namespace writer
}  // namespace tint

#endif  // SRC_WRITER_WGSL_TEST_HELPER_H_
