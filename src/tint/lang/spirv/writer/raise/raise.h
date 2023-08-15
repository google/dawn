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

#ifndef SRC_TINT_LANG_SPIRV_WRITER_RAISE_RAISE_H_
#define SRC_TINT_LANG_SPIRV_WRITER_RAISE_RAISE_H_

#include <string>

#include "src/tint/lang/spirv/writer/common/options.h"
#include "src/tint/utils/result/result.h"

// Forward declarations
namespace tint::core::ir {
class Module;
}

namespace tint::spirv::writer::raise {

/// Raise a core IR module to the SPIR-V dialect of the IR.
/// @param module the core IR module to raise to SPIR-V dialect
/// @param options the SPIR-V writer options
/// @returns success or an error string
Result<SuccessType, std::string> Raise(core::ir::Module* module, const Options& options);

}  // namespace tint::spirv::writer::raise

#endif  // SRC_TINT_LANG_SPIRV_WRITER_RAISE_RAISE_H_
