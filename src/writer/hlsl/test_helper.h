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

#ifndef SRC_WRITER_HLSL_TEST_HELPER_H_
#define SRC_WRITER_HLSL_TEST_HELPER_H_

#include <memory>
#include <sstream>
#include <string>
#include <utility>

#include "gtest/gtest.h"
#include "src/ast/builder.h"
#include "src/type_determiner.h"
#include "src/writer/hlsl/generator_impl.h"
#include "src/writer/test_namer.h"

namespace tint {
namespace writer {
namespace hlsl {

/// Helper class for testing
template <typename BODY>
class TestHelperBase : public BODY, public ast::BuilderWithModule {
 public:
  TestHelperBase() : td(mod), namer(mod), gen(mod, &namer) {}
  ~TestHelperBase() = default;

  /// @returns the result string
  std::string result() const { return out.str(); }

  /// @returns the pre result string
  std::string pre_result() const { return pre.str(); }

  /// The type determiner
  TypeDeterminer td;
  /// The test namer
  TestNamer namer;
  /// The generator
  GeneratorImpl gen;

  /// The output stream
  std::ostringstream out;
  /// The pre-output stream
  std::ostringstream pre;
};
using TestHelper = TestHelperBase<testing::Test>;

template <typename T>
using TestParamHelper = TestHelperBase<testing::TestWithParam<T>>;

}  // namespace hlsl
}  // namespace writer
}  // namespace tint

#endif  // SRC_WRITER_HLSL_TEST_HELPER_H_
