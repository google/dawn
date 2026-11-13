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

#ifndef SRC_TINT_LANG_WGSL_RESOLVER_ALIAS_ANALYSIS_INFO_H_
#define SRC_TINT_LANG_WGSL_RESOLVER_ALIAS_ANALYSIS_INFO_H_

#include "src/tint/utils/containers/hashmap.h"
#include "src/tint/utils/containers/hashset.h"

// Forward declarations.
namespace tint::sem {
class Variable;
class ValueExpression;
}  // namespace tint::sem

namespace tint::resolver {

/// AliasAnalysisInfo captures the memory accesses performed by a given function for the purpose
/// of determining if any two arguments alias at any callsite.
struct AliasAnalysisInfo {
    /// The set of module-scope variables that are written to, and where that write occurs.
    Hashmap<const sem::Variable*, const sem::ValueExpression*, 4> module_scope_writes;
    /// The set of module-scope variables that are read from, and where that read occurs.
    Hashmap<const sem::Variable*, const sem::ValueExpression*, 4> module_scope_reads;
    /// The set of function parameters that are written to.
    Hashset<const sem::Variable*, 4> parameter_writes;
    /// The set of function parameters that are read from.
    Hashset<const sem::Variable*, 4> parameter_reads;
};

}  // namespace tint::resolver

#endif  // SRC_TINT_LANG_WGSL_RESOLVER_ALIAS_ANALYSIS_INFO_H_
