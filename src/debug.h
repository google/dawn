// Copyright 2021 The Tint Authors.
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

#ifndef SRC_DEBUG_H_
#define SRC_DEBUG_H_

#include <string>

#include "src/diagnostic/diagnostic.h"
#include "src/diagnostic/formatter.h"
#include "src/diagnostic/printer.h"

namespace tint {

/// Function type used for registering an internal compiler error reporter
using InternalCompilerErrorReporter = void(const diag::List&);

/// Frees any memory allocated for reporting internal compiler errors.
/// Must only be called once on application termination.
/// If an internal compiler error is raised and this function is not called,
/// then memory will leak.
void FreeInternalCompilerErrors();

/// Sets the global error reporter to be called in case of internal compiler
/// errors.
/// @param reporter the error reporter
void SetInternalCompilerErrorReporter(InternalCompilerErrorReporter* reporter);

/// InternalCompilerError adds the internal compiler error message to the
/// diagnostics list, and then calls the InternalCompilerErrorReporter if one is
/// set.
/// @param file the file containing the ICE
/// @param line the line containing the ICE
/// @param msg the ICE message
/// @param diagnostics the list of diagnostics to append the ICE message to
void InternalCompilerError(const char* file,
                           size_t line,
                           const std::string& msg,
                           diag::List& diagnostics);

}  // namespace tint

/// TINT_ICE() is a macro for appending an internal compiler error message
/// to the diagnostics list `diagnostics`, and calling the
/// InternalCompilerErrorReporter with the full diagnostic list if a reporter is
/// set.
/// The ICE message contains the callsite's file and line.
#define TINT_ICE(diagnostics, msg) \
  tint::InternalCompilerError(__FILE__, __LINE__, msg, diagnostics)

/// TINT_UNREACHABLE() is a macro for appending a "TINT_UNREACHABLE"
/// internal compiler error message to the diagnostics list `diagnostics`, and
/// calling the InternalCompilerErrorReporter with the full diagnostic list if a
/// reporter is set.
/// The ICE message contains the callsite's file and line.
#define TINT_UNREACHABLE(diagnostics) TINT_ICE(diagnostics, "TINT_UNREACHABLE")

#endif  // SRC_DEBUG_H_
