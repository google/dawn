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

#ifndef SRC_TINT_UTILS_ICE_ICE_H_
#define SRC_TINT_UTILS_ICE_ICE_H_

#include <sstream>
#include <string>
#include <utility>

#include "src/tint/utils/macros/compiler.h"
#include "src/tint/utils/text/string_stream.h"

namespace tint {

/// InternalCompilerError is a helper for reporting internal compiler errors.
/// Construct the InternalCompilerError with the source location of the ICE fault and append any
/// error details with the `<<` operator. When the InternalCompilerError is destructed, the
/// concatenated error message is passed to the InternalCompilerErrorReporter.
class InternalCompilerError {
  public:
    /// Constructor
    /// @param file the file containing the ICE
    /// @param line the line containing the ICE
    InternalCompilerError(const char* file, size_t line);

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

    /// @returns the file that triggered the ICE
    const char* File() const { return file_; }

    /// @returns the line that triggered the ICE
    size_t Line() const { return line_; }

    /// @returns the ICE message
    std::string Message() const { return msg_.str(); }

    /// @returns the ICE file, line and message
    std::string Error() const;

  private:
    char const* const file_;
    const size_t line_;
    StringStream msg_;
};

/// Function type used for registering an internal compiler error reporter
using InternalCompilerErrorReporter = void(const InternalCompilerError&);

/// Sets the global error reporter to be called in case of internal compiler
/// errors.
/// @param reporter the error reporter
void SetInternalCompilerErrorReporter(InternalCompilerErrorReporter* reporter);

}  // namespace tint

/// TINT_ICE() is a macro to invoke the InternalCompilerErrorReporter for an Internal Compiler
/// Error. The ICE message contains the callsite's file and line. Use the `<<` operator to append an
/// error message to the ICE.
#define TINT_ICE() tint::InternalCompilerError(__FILE__, __LINE__)

/// TINT_UNREACHABLE() is a macro invoke the InternalCompilerErrorReporter when an expectedly
/// unreachable statement is reached. The ICE message contains the callsite's file and line. Use the
/// `<<` operator to append an error message to the ICE.
#define TINT_UNREACHABLE() TINT_ICE() << "TINT_UNREACHABLE "

/// TINT_UNIMPLEMENTED() is a macro to invoke the InternalCompilerErrorReporter when unimplemented
/// code is executed. The ICE message contains the callsite's file and line. Use the `<<` operator
/// to append an error message to the ICE.
#define TINT_UNIMPLEMENTED() TINT_ICE() << "TINT_UNIMPLEMENTED "

/// TINT_ASSERT() is a macro for checking the expression is true, triggering a
/// TINT_ICE if it is not.
/// The ICE message contains the callsite's file and line.
#define TINT_ASSERT(condition)                           \
    do {                                                 \
        if (TINT_UNLIKELY(!(condition))) {               \
            TINT_ICE() << "TINT_ASSERT(" #condition ")"; \
        }                                                \
    } while (false)

/// TINT_ASSERT_OR_RETURN() is a macro for checking the expression is true, triggering a
/// TINT_ICE if it is not and returning from the calling function.
/// The ICE message contains the callsite's file and line.
#define TINT_ASSERT_OR_RETURN(condition)                 \
    do {                                                 \
        if (TINT_UNLIKELY(!(condition))) {               \
            TINT_ICE() << "TINT_ASSERT(" #condition ")"; \
            return;                                      \
        }                                                \
    } while (false)

/// TINT_ASSERT_OR_RETURN_VALUE() is a macro for checking the expression is true, triggering a
/// TINT_ICE if it is not and returning a value from the calling function.
/// The ICE message contains the callsite's file and line.
#define TINT_ASSERT_OR_RETURN_VALUE(condition, value)    \
    do {                                                 \
        if (TINT_UNLIKELY(!(condition))) {               \
            TINT_ICE() << "TINT_ASSERT(" #condition ")"; \
            return value;                                \
        }                                                \
    } while (false)

#endif  // SRC_TINT_UTILS_ICE_ICE_H_
