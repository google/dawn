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

#ifndef SRC_TINT_LANG_WGSL_WRITER_RAISE_WORKGROUP_UNIFORM_LOAD_H_
#define SRC_TINT_LANG_WGSL_WRITER_RAISE_WORKGROUP_UNIFORM_LOAD_H_

#include "src/tint/utils/result.h"

// Forward declarations.
namespace tint::core::ir {
class Module;
}

namespace tint::wgsl::writer::raise {

/// WorkgroupUniformLoad is a transform that replaces loads of workgroup memory that follow a
/// workgroupBarrier with calls to workgroupUniformLoad, so that the uniformity analysis knows the
/// loaded value is uniform.
///
/// A load is replaced when it reads through a pointer that is the same for every invocation, and
/// only instructions that do not branch, access workgroup memory, or synchronize sit between it and
/// the barrier. If another workgroupBarrier follows the load in the same way, both barriers are
/// removed, as the workgroupUniformLoad call performs them. Otherwise the call adds a barrier after
/// the load, so the load is only replaced if its value is used to make a control flow decision.
/// @param module the module to transform
/// @returns success or failure
Result<SuccessType> WorkgroupUniformLoad(core::ir::Module& module);

}  // namespace tint::wgsl::writer::raise

#endif  // SRC_TINT_LANG_WGSL_WRITER_RAISE_WORKGROUP_UNIFORM_LOAD_H_
