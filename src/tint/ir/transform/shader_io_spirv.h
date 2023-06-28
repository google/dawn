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

#ifndef SRC_TINT_IR_TRANSFORM_SHADER_IO_SPIRV_H_
#define SRC_TINT_IR_TRANSFORM_SHADER_IO_SPIRV_H_

#include "src/tint/ir/transform/shader_io.h"

#include <memory>

namespace tint::ir::transform {

/// ShaderIOSpirv is the subclass of the ShaderIO transform used for the SPIR-V backend.
class ShaderIOSpirv final : public utils::Castable<ShaderIOSpirv, ShaderIO> {
  public:
    /// Constructor
    ShaderIOSpirv();
    /// Destructor
    ~ShaderIOSpirv() override;

    /// @copydoc ShaderIO::MakeBackendState
    std::unique_ptr<ShaderIO::BackendState> MakeBackendState(Module* mod,
                                                             Function* func) const override;
};

}  // namespace tint::ir::transform

#endif  // SRC_TINT_IR_TRANSFORM_SHADER_IO_SPIRV_H_
