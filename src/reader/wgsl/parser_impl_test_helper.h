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

#ifndef SRC_READER_WGSL_PARSER_IMPL_TEST_HELPER_H_
#define SRC_READER_WGSL_PARSER_IMPL_TEST_HELPER_H_

#include <memory>
#include <string>

#include "gtest/gtest.h"
#include "src/context.h"
#include "src/reader/wgsl/parser_impl.h"

namespace tint {
namespace reader {
namespace wgsl {

/// WGSL Parser test class
class ParserImplTest : public testing::Test {
 public:
  /// Constructor
  ParserImplTest();
  ~ParserImplTest() override;

  /// Sets up the test helper
  void SetUp() override;

  /// Tears down the test helper
  void TearDown() override;

  /// Retrieves the parser from the helper
  /// @param str the string to parse
  /// @returns the parser implementation
  ParserImpl* parser(const std::string& str) {
    impl_ = std::make_unique<ParserImpl>(&ctx_, str);
    return impl_.get();
  }

  /// @returns the type manager
  TypeManager* tm() { return &(ctx_.type_mgr()); }

 private:
  std::unique_ptr<ParserImpl> impl_;
  Context ctx_;
};

}  // namespace wgsl
}  // namespace reader
}  // namespace tint

#endif  // SRC_READER_WGSL_PARSER_IMPL_TEST_HELPER_H_
