// Copyright 2023 The Tint Authors.
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

#ifndef SRC_TINT_LANG_MSL_WRITER_PRINTER_HELPER_TEST_H_
#define SRC_TINT_LANG_MSL_WRITER_PRINTER_HELPER_TEST_H_

#include <iostream>
#include <string>

#include "gtest/gtest.h"
#include "src/tint/lang/core/ir/builder.h"
#include "src/tint/lang/core/ir/validator.h"
#include "src/tint/lang/msl/writer/printer/printer.h"
#include "src/tint/lang/msl/writer/raise/raise.h"

namespace tint::msl::writer {

constexpr auto kMetalHeader = R"(#include <metal_stdlib>
using namespace metal;
)";

constexpr auto kMetalArray = R"(template<typename T, size_t N>
struct tint_array {
  const constant T& operator[](size_t i) const constant { return elements[i]; }
  device T& operator[](size_t i) device { return elements[i]; }
  const device T& operator[](size_t i) const device { return elements[i]; }
  thread T& operator[](size_t i) thread { return elements[i]; }
  const thread T& operator[](size_t i) const thread { return elements[i]; }
  threadgroup T& operator[](size_t i) threadgroup { return elements[i]; }
  const threadgroup T& operator[](size_t i) const threadgroup { return elements[i]; }
  T elements[N];
};

)";

/// Base helper class for testing the MSL generator implementation.
template <typename BASE>
class MslPrinterTestHelperBase : public BASE {
  public:
    MslPrinterTestHelperBase() : writer_(&mod) {}

    /// The test module.
    core::ir::Module mod;
    /// The test builder.
    core::ir::Builder b{mod};
    /// The type manager.
    core::type::Manager& ty{mod.Types()};

  protected:
    /// The MSL writer.
    Printer writer_;

    /// Validation errors
    std::string err_;

    /// Generated MSL
    std::string output_;

    /// Run the writer on the IR module and validate the result.
    /// @returns true if generation and validation succeeded
    bool Generate() {
        auto raised = raise::Raise(&mod);
        if (!raised) {
            err_ = raised.Failure();
            return false;
        }

        auto result = writer_.Generate();
        if (!result) {
            err_ = result.Failure();
            return false;
        }
        output_ = writer_.Result();

        return true;
    }

    /// @returns the metal header string
    std::string MetalHeader() const { return kMetalHeader; }
    /// @return the metal array string
    std::string MetalArray() const { return kMetalArray; }
};

using MslPrinterTest = MslPrinterTestHelperBase<testing::Test>;

template <typename T>
using MslPrinterTestWithParam = MslPrinterTestHelperBase<testing::TestWithParam<T>>;

}  // namespace tint::msl::writer

#endif  // SRC_TINT_LANG_MSL_WRITER_PRINTER_HELPER_TEST_H_
