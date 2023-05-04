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

#ifndef SRC_TINT_WRITER_SPIRV_TEST_HELPER_IR_H_
#define SRC_TINT_WRITER_SPIRV_TEST_HELPER_IR_H_

#include <string>

#include "gtest/gtest.h"
#include "src/tint/ir/builder.h"
#include "src/tint/writer/spirv/generator_impl_ir.h"
#include "src/tint/writer/spirv/spv_dump.h"

namespace tint::writer::spirv {

/// Base helper class for testing the SPIR-V generator implementation.
template <typename BASE>
class SpvGeneratorTestHelperBase : public ir::Builder, public BASE {
  public:
    SpvGeneratorTestHelperBase() : generator_(&ir, false) {}

  protected:
    /// The SPIR-V generator.
    GeneratorImplIr generator_;

    /// @returns the disassembled types from the generated module.
    std::string DumpTypes() { return DumpInstructions(generator_.Module().Types()); }
};

using SpvGeneratorImplTest = SpvGeneratorTestHelperBase<testing::Test>;

template <typename T>
using SpvGeneratorImplTestWithParam = SpvGeneratorTestHelperBase<testing::TestWithParam<T>>;

}  // namespace tint::writer::spirv

#endif  // SRC_TINT_WRITER_SPIRV_TEST_HELPER_IR_H_
