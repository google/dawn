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

#ifndef SRC_TINT_WRITER_SPIRV_GENERATOR_IMPL_IR_H_
#define SRC_TINT_WRITER_SPIRV_GENERATOR_IMPL_IR_H_

#include <vector>

#include "src/tint/diagnostic/diagnostic.h"
#include "src/tint/writer/spirv/binary_writer.h"
#include "src/tint/writer/spirv/module.h"

// Forward declarations
namespace tint::ir {
class Module;
}  // namespace tint::ir

namespace tint::writer::spirv {

/// Implementation class for SPIR-V generator
class GeneratorImplIr {
  public:
    /// Constructor
    /// @param module the Tint IR module to generate
    /// @param zero_init_workgroup_memory `true` to initialize all the variables in the Workgroup
    ///                                   storage class with OpConstantNull
    GeneratorImplIr(const ir::Module* module, bool zero_init_workgroup_memory);

    /// @returns true on successful generation; false otherwise
    bool Generate();

    /// @returns the module that this generator has produced
    spirv::Module& Module() { return module_; }

    /// @returns the generated SPIR-V binary data
    const std::vector<uint32_t>& Result() const { return writer_.result(); }

    /// @returns the list of diagnostics raised by the generator
    diag::List Diagnostics() const { return diagnostics_; }

  private:
    const ir::Module* ir_;
    spirv::Module module_;
    BinaryWriter writer_;
    diag::List diagnostics_;

    bool zero_init_workgroup_memory_ = false;
};

}  // namespace tint::writer::spirv

#endif  // SRC_TINT_WRITER_SPIRV_GENERATOR_IMPL_IR_H_
