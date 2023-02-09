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

#ifndef SRC_TINT_READER_WGSL_PARSER_IMPL_TEST_HELPER_H_
#define SRC_TINT_READER_WGSL_PARSER_IMPL_TEST_HELPER_H_

#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "gtest/gtest.h"
#include "src/tint/reader/wgsl/parser_impl.h"

namespace tint::reader::wgsl {

/// WGSL Parser test class
class ParserImplTest : public testing::Test {
  public:
    /// Constructor
    ParserImplTest();
    ~ParserImplTest() override;

    /// Retrieves the parser from the helper
    /// @param str the string to parse
    /// @returns the parser implementation
    std::unique_ptr<ParserImpl> parser(const std::string& str) {
        auto file = std::make_unique<Source::File>("test.wgsl", str);
        auto impl = std::make_unique<ParserImpl>(file.get());
        impl->InitializeLex();
        files_.emplace_back(std::move(file));
        return impl;
    }

  private:
    std::vector<std::unique_ptr<Source::File>> files_;
};

/// WGSL Parser test class with param
template <typename T>
class ParserImplTestWithParam : public testing::TestWithParam<T>, public ProgramBuilder {
  public:
    /// Constructor
    ParserImplTestWithParam() = default;
    ~ParserImplTestWithParam() override = default;

    /// Retrieves the parser from the helper
    /// @param str the string to parse
    /// @returns the parser implementation
    std::unique_ptr<ParserImpl> parser(const std::string& str) {
        auto file = std::make_unique<Source::File>("test.wgsl", str);
        auto impl = std::make_unique<ParserImpl>(file.get());
        impl->InitializeLex();
        files_.emplace_back(std::move(file));
        return impl;
    }

  private:
    std::vector<std::unique_ptr<Source::File>> files_;
};

}  // namespace tint::reader::wgsl

#endif  // SRC_TINT_READER_WGSL_PARSER_IMPL_TEST_HELPER_H_
