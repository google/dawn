// Copyright 2018 The Dawn Authors
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

#ifndef SRC_DAWN_NATIVE_ERROR_H_
#define SRC_DAWN_NATIVE_ERROR_H_

#include <memory>
#include <string>
#include <utility>

#include "absl/strings/str_format.h"
#include "dawn/common/Result.h"
#include "dawn/native/ErrorData.h"
#include "dawn/native/Toggles.h"
#include "dawn/native/webgpu_absl_format.h"

namespace dawn::native {

enum class InternalErrorType : uint32_t { Validation, DeviceLost, Internal, OutOfMemory };

// MaybeError and ResultOrError are meant to be used as return value for function that are not
// expected to, but might fail. The handling of error is potentially much slower than successes.
using MaybeError = Result<void, ErrorData>;

template <typename T>
using ResultOrError = Result<T, ErrorData>;

// Returning a success is done like so:
//   return {}; // for Error
//   return SomethingOfTypeT; // for ResultOrError<T>
//
// Returning an error is done via:
//   return DAWN_MAKE_ERROR(errorType, "My error message");
//
// but shorthand version for specific error types are preferred:
//   return DAWN_VALIDATION_ERROR("My error message with details %s", details);
//
// There are different types of errors that should be used for different purpose:
//
//   - Validation: these are errors that show the user did something bad, which causes the
//     whole call to be a no-op. It's most commonly found in the frontend but there can be some
//     backend specific validation in non-conformant backends too.
//
//   - Out of memory: creation of a Buffer or Texture failed because there isn't enough memory.
//     This is similar to validation errors in that the call becomes a no-op and returns an
//     error object, but is reported separated from validation to the user.
//
//   - Device loss: the backend driver reported that the GPU has been lost, which means all
//     previous commands magically disappeared and the only thing left to do is clean up.
//     Note: Device loss should be used rarely and in most case you want to use Internal
//     instead.
//
//   - Internal: something happened that the backend didn't expect, and it doesn't know
//     how to recover from that situation. This causes the device to be lost, but is separate
//     from device loss, because the GPU execution is still happening so we need to clean up
//     more gracefully.
//
//   - Unimplemented: same as Internal except it puts "unimplemented" in the error message for
//     more clarity.

#define DAWN_MAKE_ERROR(TYPE, MESSAGE) \
    ::dawn::native::ErrorData::Create(TYPE, MESSAGE, __FILE__, __func__, __LINE__)

#define DAWN_VALIDATION_ERROR(...) \
    DAWN_MAKE_ERROR(InternalErrorType::Validation, absl::StrFormat(__VA_ARGS__))

#define DAWN_INVALID_IF(EXPR, ...)                                                           \
    if (DAWN_UNLIKELY(EXPR)) {                                                               \
        return DAWN_MAKE_ERROR(InternalErrorType::Validation, absl::StrFormat(__VA_ARGS__)); \
    }                                                                                        \
    for (;;)                                                                                 \
    break

// DAWN_MAKE_DEPRECATION_ERROR is used at deprecation paths. It returns a MaybeError.
// When the disallow_deprecated_path toggle is on, it creates an internal validation error.
// Otherwise it returns {} and emits a deprecation warning, and moves on.
#define DAWN_MAKE_DEPRECATION_ERROR(device, ...)            \
    device->IsToggleEnabled(Toggle::DisallowDeprecatedAPIs) \
        ? MaybeError(DAWN_VALIDATION_ERROR(__VA_ARGS__))    \
        : (device->EmitDeprecationWarning(absl::StrFormat(__VA_ARGS__)), MaybeError{})

// DAWN_DEPRECATED_IF is used analogous to DAWN_INVALID_IF at deprecation paths.
#define DAWN_DEPRECATED_IF(device, EXPR, ...)                    \
    if (DAWN_UNLIKELY(EXPR)) {                                   \
        return DAWN_MAKE_DEPRECATION_ERROR(device, __VA_ARGS__); \
    }                                                            \
    for (;;)                                                     \
    break

// DAWN_DEVICE_LOST_ERROR means that there was a real unrecoverable native device lost error.
// We can't even do a graceful shutdown because the Device is gone.
#define DAWN_DEVICE_LOST_ERROR(MESSAGE) DAWN_MAKE_ERROR(InternalErrorType::DeviceLost, MESSAGE)

// DAWN_INTERNAL_ERROR means Dawn hit an unexpected error in the backend and should try to
// gracefully shut down.
#define DAWN_INTERNAL_ERROR(MESSAGE) DAWN_MAKE_ERROR(InternalErrorType::Internal, MESSAGE)

#define DAWN_FORMAT_INTERNAL_ERROR(...) \
    DAWN_MAKE_ERROR(InternalErrorType::Internal, absl::StrFormat(__VA_ARGS__))

#define DAWN_UNIMPLEMENTED_ERROR(MESSAGE) \
    DAWN_MAKE_ERROR(InternalErrorType::Internal, std::string("Unimplemented: ") + MESSAGE)

// DAWN_OUT_OF_MEMORY_ERROR means we ran out of memory. It may be used as a signal internally in
// Dawn to free up unused resources. Or, it may bubble up to the application to signal an allocation
// was too large or they should free some existing resources.
#define DAWN_OUT_OF_MEMORY_ERROR(MESSAGE) DAWN_MAKE_ERROR(InternalErrorType::OutOfMemory, MESSAGE)

#define DAWN_CONCAT1(x, y) x##y
#define DAWN_CONCAT2(x, y) DAWN_CONCAT1(x, y)
#define DAWN_LOCAL_VAR DAWN_CONCAT2(_localVar, __LINE__)

// When Errors aren't handled explicitly, calls to functions returning errors should be
// wrapped in an DAWN_TRY. It will return the error if any, otherwise keep executing
// the current function.
#define DAWN_TRY(EXPR) DAWN_TRY_WITH_CLEANUP(EXPR, {})

#define DAWN_TRY_CONTEXT(EXPR, ...) \
    DAWN_TRY_WITH_CLEANUP(EXPR, { error->AppendContext(absl::StrFormat(__VA_ARGS__)); })

#define DAWN_TRY_WITH_CLEANUP(EXPR, BODY)                                                     \
    {                                                                                         \
        auto DAWN_LOCAL_VAR = EXPR;                                                           \
        if (DAWN_UNLIKELY(DAWN_LOCAL_VAR.IsError())) {                                        \
            std::unique_ptr<::dawn::native::ErrorData> error = DAWN_LOCAL_VAR.AcquireError(); \
            {BODY} /* comment to force the formatter to insert a newline */                   \
            error->AppendBacktrace(__FILE__, __func__, __LINE__);                             \
            return {std::move(error)};                                                        \
        }                                                                                     \
    }                                                                                         \
    for (;;)                                                                                  \
    break

// DAWN_TRY_ASSIGN is the same as DAWN_TRY for ResultOrError and assigns the success value, if
// any, to VAR.
#define DAWN_TRY_ASSIGN(VAR, EXPR) DAWN_TRY_ASSIGN_WITH_CLEANUP(VAR, EXPR, {})
#define DAWN_TRY_ASSIGN_CONTEXT(VAR, EXPR, ...) \
    DAWN_TRY_ASSIGN_WITH_CLEANUP(VAR, EXPR, { error->AppendContext(absl::StrFormat(__VA_ARGS__)); })

// Argument helpers are used to determine which macro implementations should be called when
// overloading with different number of variables.
#define DAWN_ERROR_UNIMPLEMENTED_MACRO_(...) UNREACHABLE()
#define DAWN_ERROR_GET_5TH_ARG_HELPER_(_1, _2, _3, _4, NAME, ...) NAME
#define DAWN_ERROR_GET_5TH_ARG_(args) DAWN_ERROR_GET_5TH_ARG_HELPER_ args

// DAWN_TRY_ASSIGN_WITH_CLEANUP is overloaded with 2 version so that users can override the
// return value of the macro when necessary. This is particularly useful if the function
// calling the macro may want to return void instead of the error, i.e. in a test where we may
// just want to assert and fail if the assign cannot go through. In both the cleanup and return
// clauses, users can use the `error` variable to access the pointer to the acquired error.
//
// Example usages:
//     3 Argument Case:
//          Result res;
//          DAWN_TRY_ASSIGN_WITH_CLEANUP(
//              res, GetResultOrErrorFunction(), { AddAdditionalErrorInformation(error.get()); }
//          );
//
//     4 Argument Case:
//          bool FunctionThatReturnsBool() {
//              DAWN_TRY_ASSIGN_WITH_CLEANUP(
//                  res, GetResultOrErrorFunction(),
//                  { AddAdditionalErrorInformation(error.get()); },
//                  false
//              );
//          }
#define DAWN_TRY_ASSIGN_WITH_CLEANUP(...)                                       \
    DAWN_ERROR_GET_5TH_ARG_((__VA_ARGS__, DAWN_TRY_ASSIGN_WITH_CLEANUP_IMPL_4_, \
                             DAWN_TRY_ASSIGN_WITH_CLEANUP_IMPL_3_,              \
                             DAWN_ERROR_UNIMPLEMENTED_MACRO_))                  \
    (__VA_ARGS__)

#define DAWN_TRY_ASSIGN_WITH_CLEANUP_IMPL_3_(VAR, EXPR, BODY) \
    DAWN_TRY_ASSIGN_WITH_CLEANUP_IMPL_4_(VAR, EXPR, BODY, std::move(error))

#define DAWN_TRY_ASSIGN_WITH_CLEANUP_IMPL_4_(VAR, EXPR, BODY, RET)            \
    {                                                                         \
        auto DAWN_LOCAL_VAR = EXPR;                                           \
        if (DAWN_UNLIKELY(DAWN_LOCAL_VAR.IsError())) {                        \
            std::unique_ptr<ErrorData> error = DAWN_LOCAL_VAR.AcquireError(); \
            {BODY} /* comment to force the formatter to insert a newline */   \
            error->AppendBacktrace(__FILE__, __func__, __LINE__);             \
            return (RET);                                                     \
        }                                                                     \
        VAR = DAWN_LOCAL_VAR.AcquireSuccess();                                \
    }                                                                         \
    for (;;)                                                                  \
    break

// Assert that errors are device loss so that we can continue with destruction
void IgnoreErrors(MaybeError maybeError);

wgpu::ErrorType ToWGPUErrorType(InternalErrorType type);
InternalErrorType FromWGPUErrorType(wgpu::ErrorType type);

}  // namespace dawn::native

#endif  // SRC_DAWN_NATIVE_ERROR_H_
