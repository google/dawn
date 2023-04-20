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

#ifndef SRC_TINT_AST_DIAGNOSTIC_CONTROL_H_
#define SRC_TINT_AST_DIAGNOSTIC_CONTROL_H_

#include <string>
#include <unordered_map>

#include "src/tint/builtin/diagnostic_severity.h"
#include "src/tint/diagnostic/diagnostic.h"

// Forward declarations
namespace tint::ast {
class DiagnosticRuleName;
}  // namespace tint::ast

namespace tint::ast {

/// A diagnostic control used for diagnostic directives and attributes.
struct DiagnosticControl {
  public:
    /// Default constructor.
    DiagnosticControl();

    /// Constructor
    /// @param sev the diagnostic severity
    /// @param rule the diagnostic rule name
    DiagnosticControl(builtin::DiagnosticSeverity sev, const DiagnosticRuleName* rule);

    /// Move constructor
    DiagnosticControl(DiagnosticControl&&);

    /// The diagnostic severity control.
    builtin::DiagnosticSeverity severity = builtin::DiagnosticSeverity::kUndefined;

    /// The diagnostic rule name.
    const DiagnosticRuleName* rule_name = nullptr;
};

}  // namespace tint::ast

#endif  // SRC_TINT_AST_DIAGNOSTIC_CONTROL_H_
