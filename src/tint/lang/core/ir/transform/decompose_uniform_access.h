// Copyright 2024 The Dawn & Tint Authors
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// 1. Redistributions of source code must retain the above copyright notice, this
//    list of conditions and the following disclaimer.
//
// 2. Redistributions in binary form must reproduce the above copyright notice,
//    this list of conditions and the following disclaimer in the documentation
//    and/or other materials provided with the distribution.
//
// 3. Neither the name of the copyright holder nor the names of its
//    contributors may be used to endorse or promote products derived from
//    this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
// FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
// DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
// CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
// OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#ifndef SRC_TINT_LANG_CORE_IR_TRANSFORM_DECOMPOSE_UNIFORM_ACCESS_H_
#define SRC_TINT_LANG_CORE_IR_TRANSFORM_DECOMPOSE_UNIFORM_ACCESS_H_

#include "src/tint/utils/result.h"

// Forward declarations.
namespace tint::core::ir {
class Module;
}  // namespace tint::core::ir

namespace tint::core::ir::transform {

struct DecomposeUniformAccessOptions {
    /// Specify whether all variables in the address space should be decomposed.
    bool storage = false;
    bool uniform = false;
    bool workgroup = false;

    // TODO(b/477295042): should there be a uniform standard layout option? When enabled uniform
    // could be treated like every other storage class.
};

/// DecomposeUniformAccess is a transform used to replace uniform, storage, or workgroup variables
/// with an a uniformly typed array instead.
/// TODO(b/459523229): take options in the base version, rename this transform/file and update all
/// uses.
///
/// @param module the module to transform
/// @returns success or failure
Result<SuccessType> DecomposeUniformAccess(core::ir::Module& module);
Result<SuccessType> DecomposeUniformAccessWithOptions(core::ir::Module& module,
                                                      const DecomposeUniformAccessOptions& options);

}  // namespace tint::core::ir::transform

#endif  // SRC_TINT_LANG_CORE_IR_TRANSFORM_DECOMPOSE_UNIFORM_ACCESS_H_
