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

#ifndef SRC_TINT_LANG_SPIRV_INTRINSIC_DIALECT_H_
#define SRC_TINT_LANG_SPIRV_INTRINSIC_DIALECT_H_

#include "src/tint/lang/core/intrinsic/table_data.h"
#include "src/tint/lang/spirv/builtin_fn.h"

namespace tint::spirv::intrinsic {

/// Dialect holds the intrinsic table data and types for the SPIR-V dialect
struct Dialect {
    /// The dialect's intrinsic table data
    static const core::intrinsic::TableData kData;

    /// The dialect's builtin function enumerator
    using BuiltinFn = spirv::BuiltinFn;

    /// @returns the name of the builtin function @p fn
    /// @param fn the builtin function
    static std::string_view ToString(BuiltinFn fn) { return str(fn); }
};

}  // namespace tint::spirv::intrinsic

#endif  // SRC_TINT_LANG_SPIRV_INTRINSIC_DIALECT_H_
