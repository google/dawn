// Copyright 2022 The Tint Authors.
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

#ifndef SRC_TINT_WRITER_CHECK_SUPPORTED_EXTENSIONS_H_
#define SRC_TINT_WRITER_CHECK_SUPPORTED_EXTENSIONS_H_

#include "src/tint/builtin/extension.h"
#include "src/tint/utils/vector.h"

namespace tint::ast {
class Module;
}  // namespace tint::ast
namespace tint::diag {
class List;
}  // namespace tint::diag

namespace tint::writer {

/// Checks that all the extensions enabled in @p module are found in @p supported, raising an error
/// diagnostic if an enabled extension is not supported.
/// @param writer_name the name of the writer making this call
/// @param module the AST module
/// @param diags the diagnostics to append an error to, if needed.
/// @returns true if all extensions in use are supported, otherwise returns false.
bool CheckSupportedExtensions(std::string_view writer_name,
                              const ast::Module& module,
                              diag::List& diags,
                              utils::VectorRef<builtin::Extension> supported);

}  // namespace tint::writer

#endif  // SRC_TINT_WRITER_CHECK_SUPPORTED_EXTENSIONS_H_
