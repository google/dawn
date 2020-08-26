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

#include <sstream>

#include "gtest/gtest.h"
#include "src/ast/module.h"
#include "src/context.h"
#include "src/type_determiner.h"
#include "src/writer/hlsl/generator_impl.h"

namespace tint {
namespace writer {
namespace hlsl {

/// Helper class for testing
template <typename T>
class TestHelperBase : public T {
 public:
  TestHelperBase() : td_(&ctx_, &mod_), impl_(&mod_) {}
  ~TestHelperBase() = default;

  /// @returns the generator implementation
  GeneratorImpl& gen() { return impl_; }

  /// @returns the module
  ast::Module* mod() { return &mod_; }

  /// @returns the type determiner
  TypeDeterminer& td() { return td_; }

  /// @returns the output stream
  std::ostream& out() { return out_; }

  /// @returns the result string
  std::string result() const { return out_.str(); }

 private:
  Context ctx_;
  ast::Module mod_;
  TypeDeterminer td_;
  GeneratorImpl impl_;
  std::ostringstream out_;
};
using TestHelper = TestHelperBase<testing::Test>;

}  // namespace hlsl
}  // namespace writer
}  // namespace tint

#endif  // SRC_WRITER_HLSL_TEST_HELPER_H_
