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
    MslPrinterTestHelperBase() : generator_(&mod) {}

    /// The test module.
    ir::Module mod;
    /// The test builder.
    ir::Builder b{mod};
    /// The type manager.
    type::Manager& ty{mod.Types()};

    /// @returns the metal header string
    std::string MetalHeader() const { return kMetalHeader; }
    /// @return the metal array string
    std::string MetalArray() const { return kMetalArray; }

  protected:
    /// The MSL generator.
    Printer generator_;

    /// Validation errors
    std::string err_;

    /// @returns the error string from the validation
    std::string Error() const { return err_; }

    /// @returns true if the IR module is valid
    bool IRIsValid() {
        auto res = ir::Validate(mod);
        if (!res) {
            err_ = res.Failure().str();
            return false;
        }
        return true;
    }
};

using MslPrinterTest = MslPrinterTestHelperBase<testing::Test>;

template <typename T>
using MslPrinterTestWithParam = MslPrinterTestHelperBase<testing::TestWithParam<T>>;

}  // namespace tint::msl::writer

#endif  // SRC_TINT_LANG_MSL_WRITER_PRINTER_HELPER_TEST_H_
