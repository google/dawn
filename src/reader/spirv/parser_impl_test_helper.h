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

#ifndef SRC_READER_SPIRV_PARSER_IMPL_TEST_HELPER_H_
#define SRC_READER_SPIRV_PARSER_IMPL_TEST_HELPER_H_

#include <memory>
#include <string>

#include "gtest/gtest.h"
#include "src/context.h"
#include "src/reader/spirv/parser_impl.h"

namespace tint {
namespace reader {
namespace spirv {

class SpvParserTest : public testing::Test {
 public:
  SpvParserTest() = default;
  ~SpvParserTest() = default;

  void SetUp() { ctx_.type_mgr = &tm_; }

  void TearDown() {
    impl_ = nullptr;
    ctx_.type_mgr = nullptr;
  }

  ParserImpl* parser(const std::vector<uint32_t>& input) {
    impl_ = std::make_unique<ParserImpl>(ctx_, input);
    return impl_.get();
  }

 private:
  std::unique_ptr<ParserImpl> impl_;
  Context ctx_;
  TypeManager tm_;
};

}  // namespace spirv
}  // namespace reader
}  // namespace tint

#endif  // SRC_READER_WGSL_PARSER_IMPL_TEST_HELPER_H_
