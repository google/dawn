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

#ifndef SRC_TINT_DEBUG_H_
#define SRC_TINT_DEBUG_H_

#include <utility>

#include "src/tint/diagnostic/diagnostic.h"
#include "src/tint/diagnostic/formatter.h"
#include "src/tint/diagnostic/printer.h"
#include "src/tint/utils/compiler_macros.h"
#include "src/tint/utils/string_stream.h"

namespace tint {

/// Function type used for registering an internal compiler error reporter
using InternalCompilerErrorReporter = void(const diag::List&);

/// Sets the global error reporter to be called in case of internal compiler
/// errors.
/// @param reporter the error reporter
void SetInternalCompilerErrorReporter(InternalCompilerErrorReporter* reporter);

/// InternalCompilerError is a helper for reporting internal compiler errors.
/// Construct the InternalCompilerError with the source location of the ICE
/// fault and append any error details with the `<<` operator.
/// When the InternalCompilerError is destructed, the concatenated error message
/// is appended to the diagnostics list with the severity of
/// tint::diag::Severity::InternalCompilerError, and if a
/// InternalCompilerErrorReporter is set, then it is called with the diagnostic
/// list.
class InternalCompilerError {
  public:
    /// Constructor
    /// @param file the file containing the ICE
    /// @param line the line containing the ICE
    /// @param system the Tint system that has raised the ICE
    /// @param diagnostics the list of diagnostics to append the ICE message to
    InternalCompilerError(const char* file,
                          size_t line,
                          diag::System system,
                          diag::List& diagnostics);

    /// Destructor.
    /// Adds the internal compiler error message to the diagnostics list, and then
    /// calls the InternalCompilerErrorReporter if one is set.
    ~InternalCompilerError();

    /// Appends `arg` to the ICE message.
    /// @param arg the argument to append to the ICE message
    /// @returns this object so calls can be chained
    template <typename T>
    InternalCompilerError& operator<<(T&& arg) {
        msg_ << std::forward<T>(arg);
        return *this;
    }

  private:
    char const* const file_;
    const size_t line_;
    diag::System system_;
    diag::List& diagnostics_;
    utils::StringStream msg_;
};

}  // namespace tint

/// TINT_ICE() is a macro for appending an internal compiler error message
/// to the diagnostics list `diagnostics`, and calling the
/// InternalCompilerErrorReporter with the full diagnostic list if a reporter is
/// set.
/// The ICE message contains the callsite's file and line.
/// Use the `<<` operator to append an error message to the ICE.
#define TINT_ICE(system, diagnostics) \
    tint::InternalCompilerError(__FILE__, __LINE__, ::tint::diag::System::system, diagnostics)

/// TINT_UNREACHABLE() is a macro for appending a "TINT_UNREACHABLE"
/// internal compiler error message to the diagnostics list `diagnostics`, and
/// calling the InternalCompilerErrorReporter with the full diagnostic list if a
/// reporter is set.
/// The ICE message contains the callsite's file and line.
/// Use the `<<` operator to append an error message to the ICE.
#define TINT_UNREACHABLE(system, diagnostics) TINT_ICE(system, diagnostics) << "TINT_UNREACHABLE "

/// TINT_UNIMPLEMENTED() is a macro for appending a "TINT_UNIMPLEMENTED"
/// internal compiler error message to the diagnostics list `diagnostics`, and
/// calling the InternalCompilerErrorReporter with the full diagnostic list if a
/// reporter is set.
/// The ICE message contains the callsite's file and line.
/// Use the `<<` operator to append an error message to the ICE.
#define TINT_UNIMPLEMENTED(system, diagnostics) \
    TINT_ICE(system, diagnostics) << "TINT_UNIMPLEMENTED "

/// TINT_ASSERT() is a macro for checking the expression is true, triggering a
/// TINT_ICE if it is not.
/// The ICE message contains the callsite's file and line.
/// @warning: Unlike TINT_ICE() and TINT_UNREACHABLE(), TINT_ASSERT() does not
/// append a message to an existing tint::diag::List. As such, TINT_ASSERT()
/// may silently fail in builds where SetInternalCompilerErrorReporter() is not
/// called. Only use in places where there's no sensible place to put proper
/// error handling.
#define TINT_ASSERT(system, condition)                                                   \
    do {                                                                                 \
        if (TINT_UNLIKELY(!(condition))) {                                               \
            tint::diag::List diagnostics;                                                \
            TINT_ICE(system, diagnostics) << "TINT_ASSERT(" #system ", " #condition ")"; \
        }                                                                                \
    } while (false)

#endif  // SRC_TINT_DEBUG_H_
