// Copyright 2026 The Dawn & Tint Authors
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

#ifndef SRC_TINT_LANG_SPIRV_READER_LOWER_HOIST_CONSTANT_ARRAY_H_
#define SRC_TINT_LANG_SPIRV_READER_LOWER_HOIST_CONSTANT_ARRAY_H_

#include "src/tint/utils/result.h"

// Forward declarations.
namespace tint::core::ir {
class Module;
}

namespace tint::spirv::reader::lower {

/// HoistConstantArray hoists function-scope array variables
/// to constants when possible.
///
/// This undoes a pessimization forced by the fact that SPIR-V
/// does not support dynamic indexing of array constants.
///
/// Consider this GLSL fragment:
///     const uint lookup[100] = uint[100](...);
///
///     void main() {
///       uint elem = lookup[i]; // i is dynamic
///     }
///
/// Glslang compiles this as follows:
///   - The lookup variable is converted to a function-scope
///     variable without an initializer.
///   - To compute elem:
///       OpStore %lookup %initializer
///       %elem_ptr = OpAccessChain ptr-to-fn-uint %lookup %i
///       %elem = OpLoad %uint %elem_ptr
///   - There are no other uses of %lookup or %elem_ptr.
///   - Each array access of the lookup variable in the GLSL
///     source generates a new %lookup function-scope variable
///     in the function, and associated initialization.
///
/// If naively mapped to MSL, the resulting AIR code uses a
/// sequence of scalar stores to initialize each %lookup variable.
/// This yields terrible performance.
///
/// This pass rewrites these array variables as array constants
/// with have indexed accesses. Then the resulting MSL to AIR conversion
/// will map these array values to unique module-scope constants.
///
/// @param module the module to transform
/// @returns success or failure
Result<SuccessType> HoistConstantArray(core::ir::Module& module);

}  // namespace tint::spirv::reader::lower

#endif  // SRC_TINT_LANG_SPIRV_READER_LOWER_HOIST_CONSTANT_ARRAY_H_
