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

#ifndef SRC_TINT_LANG_CORE_INTRINSIC_DIALECT_H_
#define SRC_TINT_LANG_CORE_INTRINSIC_DIALECT_H_

#include "src/tint/lang/core/builtin_fn.h"
#include "src/tint/lang/core/intrinsic/ctor_conv.h"
#include "src/tint/lang/core/intrinsic/table_data.h"

namespace tint::core::intrinsic {

/// Dialect holds the intrinsic table data and types for the core dialect
struct Dialect {
    /// The dialect's intrinsic table data
    static const TableData kData;

    /// The dialect's builtin function enumerator
    using BuiltinFn = core::BuiltinFn;

    /// The dialect's type constructor / convertor enumerator
    using CtorConv = core::intrinsic::CtorConv;

    /// @returns the name of the builtin function @p fn
    /// @param fn the builtin function
    static std::string_view ToString(BuiltinFn fn) { return str(fn); }

    /// @returns the name of the type constructor / convertor @p ty
    /// @param ty the type constructor / convertor
    static std::string_view ToString(CtorConv ty) { return str(ty); }
};

}  // namespace tint::core::intrinsic

#endif  // SRC_TINT_LANG_CORE_INTRINSIC_DIALECT_H_
