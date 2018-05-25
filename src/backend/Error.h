// Copyright 2018 The NXT Authors
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

#ifndef BACKEND_ERROR_H_
#define BACKEND_ERROR_H_

#include "common/Result.h"

namespace backend {

    // This is the content of an error value for MaybeError or ResultOrError, split off to its own
    // file to avoid having all files including headers like <string> and <vector>
    class ErrorData;

    // MaybeError and ResultOrError are meant to be used as return value for function that are not
    // expected to, but might fail. The handling of error is potentially much slower than successes.
    using MaybeError = Result<void, ErrorData*>;

    template <typename T>
    using ResultOrError = Result<T, ErrorData*>;

    // Returning a success is done like so:
    //   return {}; // for Error
    //   return SomethingOfTypeT; // for ResultOrError<T>
    //
    // Returning an error is done via:
    //   NXT_RETURN_ERROR("My error message");
#define NXT_RETURN_ERROR(EXPR) return MakeError(EXPR, __FILE__, __func__, __LINE__)

#define NXT_CONCAT1(x, y) x##y
#define NXT_CONCAT2(x, y) NXT_CONCAT1(x, y)
#define NXT_LOCAL_VAR NXT_CONCAT2(_localVar, __LINE__)

    // When Errors aren't handled explicitly, calls to functions returning errors should be
    // wrapped in an NXT_TRY. It will return the error if any, otherwise keep executing
    // the current function.
#define NXT_TRY(EXPR)                                             \
    {                                                             \
        auto NXT_LOCAL_VAR = EXPR;                                \
        if (NXT_UNLIKELY(NXT_LOCAL_VAR.IsError())) {              \
            ErrorData* error = NXT_LOCAL_VAR.AcquireError();      \
            AppendBacktrace(error, __FILE__, __func__, __LINE__); \
            return {error};                                       \
        }                                                         \
    }                                                             \
    for (;;)                                                      \
    break

    // NXT_TRY_ASSIGN is the same as NXT_TRY for ResultOrError and assigns the success value, if
    // any, to VAR.
#define NXT_TRY_ASSIGN(VAR, EXPR)                                 \
    {                                                             \
        auto NXT_LOCAL_VAR = EXPR;                                \
        if (NXT_UNLIKELY(NXT_LOCAL_VAR.IsError())) {              \
            ErrorData* error = NXT_LOCAL_VAR.AcquireError();      \
            AppendBacktrace(error, __FILE__, __func__, __LINE__); \
            return {error};                                       \
        }                                                         \
        VAR = NXT_LOCAL_VAR.AcquireSuccess();                     \
    }                                                             \
    for (;;)                                                      \
    break

    // Implementation detail of NXT_TRY and NXT_TRY_ASSIGN's adding to the Error's backtrace.
    void AppendBacktrace(ErrorData* error, const char* file, const char* function, int line);

    // Implementation detail of NXT_RETURN_ERROR
    ErrorData* MakeError(const char* message, const char* file, const char* function, int line);

}  // namespace backend

#endif  // BACKEND_ERROR_H_
