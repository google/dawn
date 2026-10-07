// Copyright 2018 The Dawn & Tint Authors
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

#ifndef SRC_DAWN_NATIVE_ERROR_H_
#define SRC_DAWN_NATIVE_ERROR_H_

#include <concepts>  // IWYU pragma: export
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "src/dawn/common/Result.h"
#include "src/dawn/native/ErrorData.h"
#include "src/dawn/native/webgpu_absl_format.h"  // IWYU pragma: export

namespace dawn::native {

enum class InternalErrorType : uint32_t {
    None = 0,
    Validation = 1,
    BackendDeviceLost = 2,
    Unrecoverable = 4,
    PipelineUncategorized = 8,
    OutOfMemory = 16
};

class UnknownError;
class UnrecoverableError;
class ValidationError;

// MaybeError and ResultOrError are meant to be used as return value for function that are not
// expected to, but might fail. The handling of error is potentially much slower than successes.
using MaybeError = Result<void, UnrecoverableError>;
using MaybeValError = Result<void, ValidationError>;
using MaybeUnknownError = Result<void, UnknownError>;

template <typename T>
using ResultOrError = Result<T, UnrecoverableError>;
template <typename T>
using ResultOrValError = Result<T, ValidationError>;
template <typename T>
using ResultOrUnknownError = Result<T, UnknownError>;

template <typename T>
concept IsMaybeConcreteError = std::is_same_v<T, MaybeError> || std::is_same_v<T, MaybeValError>;

template <typename E, typename T>
concept IsResultOrConcreteError =
    std::is_same_v<E, ResultOrError<T>> || std::is_same_v<E, ResultOrValError<T>>;

template <typename T>
concept IsConcreteError =
    std::is_same_v<T, UnrecoverableError> || std::is_same_v<T, ValidationError>;

class UnknownError {
  public:
    static std::unique_ptr<UnknownError> Create(std::unique_ptr<ErrorData> e) {
        return std::make_unique<UnknownError>(std::move(e));
    }

    explicit UnknownError(std::unique_ptr<ErrorData> d);
    explicit(false) UnknownError(std::unique_ptr<UnrecoverableError> d);
    explicit(false) UnknownError(std::unique_ptr<ValidationError> d);
    UnknownError(UnknownError&&) = default;

    virtual ~UnknownError() = default;

    UnknownError& operator=(UnknownError&&) = default;

    bool IsVal() const { return GetType() == InternalErrorType::Validation; }

    std::unique_ptr<ValidationError> TakeAsVal();
    std::unique_ptr<UnrecoverableError> TakeAsUnrecoverable();

    InternalErrorType GetType() const { return mData->GetType(); }

    const std::string& GetMessage() const { return mData->GetMessage(); }
    const std::vector<ErrorData::BacktraceRecord>& GetBacktrace() const {
        return mData->GetBacktrace();
    }
    const std::vector<std::string>& GetContexts() const { return mData->GetContexts(); }
    std::string GetFormattedMessage() const { return mData->GetFormattedMessage(); }

    void AppendContext(std::string context) { mData->AppendContext(std::move(context)); }
    template <typename... Args>
    void AppendContext(const char* formatStr, const Args&... args) {
        mData->AppendContext(formatStr, args...);
    }
    void AppendBacktrace(const char* file, const char* function, int line) {
        mData->AppendBacktrace(file, function, line);
    }
    void AppendDebugGroup(std::string_view label) { mData->AppendDebugGroup(label); }

    std::unique_ptr<ErrorData> ReleaseData() { return std::move(mData); }
    ErrorData* GetData() { return mData.get(); }

    UnknownError(const UnknownError&) = delete;
    UnknownError& operator=(const UnknownError&) = delete;

  private:
    std::unique_ptr<ErrorData> mData;
};

class ValidationError : public UnknownError {
  public:
    static std::unique_ptr<ValidationError> Create(std::unique_ptr<ErrorData> data) {
        return std::make_unique<ValidationError>(std::move(data));
    }

    explicit ValidationError(std::unique_ptr<ErrorData> d) : UnknownError(std::move(d)) {
        DAWN_CHECK(GetData()->GetType() == InternalErrorType::Validation);
    }
};

class UnrecoverableError : public UnknownError {
  public:
    static std::unique_ptr<UnrecoverableError> Create(std::unique_ptr<ErrorData> data) {
        return std::make_unique<UnrecoverableError>(std::move(data));
    }

    explicit UnrecoverableError(std::unique_ptr<ErrorData> d) : UnknownError(std::move(d)) {}
    explicit(false) UnrecoverableError(std::unique_ptr<UnknownError> d);

    void SetType(InternalErrorType type) { GetData()->SetType(type); }
};

template <typename T>
class ErrorAdapter {
  public:
    static constexpr bool value = false;
    explicit ErrorAdapter(std::unique_ptr<T> d) : mData(std::move(d)) {}

    ~ErrorAdapter() = default;

    // Conversions to `UnknownError`
    //   * from `UnrecoverableError`, return the error
    //   * from `ValidationError`, return the error
    // NOLINTNEXTLINE(google-explicit-constructor)
    explicit(false) operator std::unique_ptr<UnknownError>()
        requires(IsConcreteError<T>)
    {
        return UnknownError::Create(mData->ReleaseData());
    }
    // NOLINTNEXTLINE(google-explicit-constructor)
    explicit(false) operator std::unique_ptr<UnknownError>()
        requires(std::is_same_v<T, UnknownError>)
    {
        return std::move(mData);
    }

    // Conversions to `UnrecoverableError`
    //   * from `UnrecoverableError`, just return the error
    //   * from `ValidationError`, convert to Unrecoverable
    //   * from `UnknownError`, if type is `Validation`, convert to `Unrecoverable`, else return
    // NOLINTNEXTLINE(google-explicit-constructor)
    explicit(false) operator std::unique_ptr<UnrecoverableError>()
        requires(std::is_same_v<T, UnrecoverableError>)
    {
        return std::move(mData);
    }
    // NOLINTNEXTLINE(google-explicit-constructor)
    explicit(false) operator std::unique_ptr<UnrecoverableError>()
        requires(std::is_same_v<T, ValidationError> || std::is_same_v<T, UnknownError>)
    {
        std::unique_ptr<ErrorData> ed = mData->ReleaseData();
        if (ed->GetType() == InternalErrorType::Validation) {
            ed->SetType(InternalErrorType::Unrecoverable);
        }
        return UnrecoverableError::Create(std::move(ed));
    }

    // Conversion to `ValidationError`
    //   * from `ValidationError`, just return the error
    // NOLINTNEXTLINE(google-explicit-constructor)
    explicit(false) operator std::unique_ptr<ValidationError>()
        requires(std::is_same_v<T, ValidationError>)
    {
        return std::move(mData);
    }

    // Can only get a MaybeValError with a ValidationError
    // NOLINTNEXTLINE(google-explicit-constructor)
    explicit(false) operator MaybeValError()
        requires(std::is_same_v<T, ValidationError>)
    {
        std::unique_ptr<ValidationError> err = *this;
        return std::move(err);
    }

    // Can get to MaybeError with any source Error type
    explicit(false) operator MaybeError() {  // NOLINT(google-explicit-constructor)
        std::unique_ptr<UnrecoverableError> err = *this;
        return std::move(err);
    }

    // Can get to MaybeError with any source Error type
    explicit(false) operator MaybeUnknownError() {  // NOLINT(google-explicit-constructor)
        std::unique_ptr<UnknownError> err = *this;
        return std::move(err);
    }

    // ResultOrValError requires a validation source
    template <typename K>
    // NOLINTNEXTLINE(google-explicit-constructor)
    explicit(false) operator ResultOrValError<K>()
        requires(std::is_same_v<T, ValidationError>)
    {
        std::unique_ptr<ValidationError> err = *this;
        return std::move(err);
    }

    // ResultOrError with any kind of source error
    template <typename K>
    explicit(false) operator ResultOrError<K>() {  // NOLINT(google-explicit-constructor)
        std::unique_ptr<UnrecoverableError> err = *this;
        return std::move(err);
    }

    // ResultOrUnknownError with any kind of source error
    template <typename K>
    explicit(false) operator ResultOrUnknownError<K>() {  // NOLINT(google-explicit-constructor)
        std::unique_ptr<UnknownError> err = *this;
        return std::move(err);
    }

    [[nodiscard]] std::unique_ptr<ErrorData> ReleaseData() { return mData->ReleaseData(); }
    [[nodiscard]] ErrorData* GetData() { return mData->GetData(); }

  private:
    std::unique_ptr<T> mData;
};

template <typename T>
ErrorAdapter(std::unique_ptr<T>) -> ErrorAdapter<T>;

namespace detail {

template <typename T>
struct UnwrapResultOrError {
    using type = T;
};

template <typename T>
struct UnwrapResultOrError<ResultOrError<T>> {
    using type = T;
};
template <typename T>
struct UnwrapResultOrError<ResultOrValError<T>> {
    using type = T;
};
template <typename T>
struct UnwrapResultOrError<ResultOrUnknownError<T>> {
    using type = T;
};

template <typename T>
struct IsResultOrError {
    static constexpr bool value = false;
};

template <typename T>
struct IsResultOrError<ResultOrError<T>> {
    static constexpr bool value = true;
};
template <typename T>
struct IsResultOrError<ResultOrValError<T>> {
    static constexpr bool value = true;
};
template <typename T>
struct IsResultOrError<ResultOrUnknownError<T>> {
    static constexpr bool value = true;
};

template <typename T>
struct IsResultOrUnknownError {
    static constexpr bool value = false;
};
template <typename T>
struct IsResultOrUnknownError<ResultOrUnknownError<T>> {
    static constexpr bool value = true;
};

}  // namespace detail

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

#define DAWN_MAKE_ERROR_DATA(TYPE, MESSAGE) \
    ::dawn::native::ErrorData::Create(TYPE, MESSAGE, __FILE__, __func__, __LINE__)

#define DAWN_MAKE_ERROR(TYPE, MESSAGE)                                                  \
    ::dawn::native::ErrorAdapter {                                                      \
        ::dawn::native::UnrecoverableError::Create(DAWN_MAKE_ERROR_DATA(TYPE, MESSAGE)) \
    }

#define DAWN_MAKE_VALIDATION_ERROR(MESSAGE)                               \
    ::dawn::native::ErrorAdapter<ValidationError> {                       \
        ::dawn::native::ValidationError::Create(                          \
            DAWN_MAKE_ERROR_DATA(InternalErrorType::Validation, MESSAGE)) \
    }

#define DAWN_VALIDATION_ERROR(...) DAWN_MAKE_VALIDATION_ERROR(absl::StrFormat(__VA_ARGS__))

#define DAWN_INVALID_IF(EXPR, ...)                                       \
    if (EXPR) [[unlikely]] {                                             \
        return DAWN_MAKE_VALIDATION_ERROR(absl::StrFormat(__VA_ARGS__)); \
    }                                                                    \
    for (;;)                                                             \
    break

#define DAWN_PIPELINE_UNCATEGORIZED_ERROR(...) \
    DAWN_MAKE_ERROR(InternalErrorType::PipelineUncategorized, absl::StrFormat(__VA_ARGS__))

#define DAWN_PIPELINE_UNCATEGORIZED_IF(EXPR, ...)              \
    if (EXPR) [[unlikely]] {                                   \
        return DAWN_PIPELINE_UNCATEGORIZED_ERROR(__VA_ARGS__); \
    }                                                          \
    for (;;)                                                   \
    break

// DAWN_BACKEND_DEVICE_LOST_ERROR means that there was a real unrecoverable native device lost
// error. We can't even do a graceful shutdown because the Device is gone.
#define DAWN_BACKEND_DEVICE_LOST_ERROR(MESSAGE) \
    DAWN_MAKE_ERROR(InternalErrorType::BackendDeviceLost, MESSAGE)

#define DAWN_MAKE_UNRECOVERABLE_ERROR(MESSAGE) \
    DAWN_MAKE_ERROR(InternalErrorType::Unrecoverable, MESSAGE)

// DAWN_UNRECOVERABLE_ERROR means Dawn hit an unexpected error in the backend and should try to
// gracefully shut down.
#define DAWN_UNRECOVERABLE_ERROR(...) DAWN_MAKE_UNRECOVERABLE_ERROR(absl::StrFormat(__VA_ARGS__))

#define DAWN_UNRECOVERABLE_ERROR_IF(EXPR, ...)                              \
    if (EXPR) [[unlikely]] {                                                \
        return DAWN_MAKE_UNRECOVERABLE_ERROR(absl::StrFormat(__VA_ARGS__)); \
    }                                                                       \
    for (;;)                                                                \
    break

#define DAWN_UNIMPLEMENTED_ERROR(MESSAGE) \
    DAWN_MAKE_UNRECOVERABLE_ERROR(std::string("Unimplemented: ") + MESSAGE)

// DAWN_OUT_OF_MEMORY_ERROR means we ran out of memory. It may be used as a signal internally in
// Dawn to free up unused resources. Or, it may bubble up to the application to signal an allocation
// was too large or they should free some existing resources.
#define DAWN_OUT_OF_MEMORY_ERROR(MESSAGE) DAWN_MAKE_ERROR(InternalErrorType::OutOfMemory, MESSAGE)

template <typename T>
std::string MakeIncreaseLimitMessage(std::string_view limitName, T adapterLimitValue) {
    return absl::StrFormat(
        " This adapter supports a higher %s of %u, which can be specified in requiredLimits when "
        "calling requestDevice(). Limits differ by hardware, so always check the adapter limits "
        "prior to requesting a higher limit.",
        limitName, adapterLimitValue);
}

#define DAWN_INCREASE_LIMIT_MESSAGE(adapterLimits, limitName, limitValue)                         \
    [&]() -> std::string {                                                                        \
        return (limitValue > adapterLimits.limitName) ? ""                                        \
                                                      : ::dawn::native::MakeIncreaseLimitMessage( \
                                                            #limitName, adapterLimits.limitName); \
    }()

#define DAWN_CONCAT1(x, y) x##y
#define DAWN_CONCAT2(x, y) DAWN_CONCAT1(x, y)
#define DAWN_LOCAL_VAR(name) DAWN_CONCAT2(DAWN_CONCAT2(_localVar, __LINE__), name)

// Backtrace information adds a lot of binary size with the name of all the files and functions,
// plus additional calls to AppendBacktrace. Only add the backtrace in Debug so as to save binary
// size in release. Most backtrace information useful to developers is already added via
// DAWN_TRY_CONTEXT anyway.
#if defined(DAWN_ENABLE_ASSERTS)
#define DAWN_APPEND_ERROR_BACKTRACE(error) error->AppendBacktrace(__FILE__, __func__, __LINE__)
#else  // defined(DAWN_ENABLE_ASSERTS)
#define DAWN_APPEND_ERROR_BACKTRACE(error) \
    for (;;)                               \
    break
#endif  // defined(DAWN_ENABLE_ASSERTS)

// When Errors aren't handled explicitly, calls to functions returning errors should be
// wrapped in an DAWN_TRY. It will return the error if any, otherwise keep executing
// the current function.
#define DAWN_TRY(EXPR) DAWN_TRY_WITH_CLEANUP(EXPR, {})

#define DAWN_TRY_CONTEXT(EXPR, ...) \
    DAWN_TRY_WITH_CLEANUP(EXPR,     \
                          { DAWN_LOCAL_VAR(Error)->AppendContext(absl::StrFormat(__VA_ARGS__)); })

#define DAWN_TRY_WITH_CLEANUP(EXPR, BODY)                                          \
    {                                                                              \
        auto DAWN_LOCAL_VAR(Result) = EXPR;                                        \
        if (DAWN_LOCAL_VAR(Result).IsError()) [[unlikely]] {                       \
            auto DAWN_LOCAL_VAR(Error) = DAWN_LOCAL_VAR(Result).AcquireError();    \
            {BODY} /* comment to force the formatter to insert a newline */        \
            DAWN_APPEND_ERROR_BACKTRACE(DAWN_LOCAL_VAR(Error));                    \
            return ::dawn::native::ErrorAdapter{std::move(DAWN_LOCAL_VAR(Error))}; \
        }                                                                          \
    }                                                                              \
    for (;;)                                                                       \
    break

// DAWN_TRY_ASSIGN is the same as DAWN_TRY for ResultOrError and assigns the success value, if
// any, to VAR.
#define DAWN_TRY_ASSIGN(VAR, EXPR) DAWN_TRY_ASSIGN_WITH_CLEANUP(VAR, EXPR, {})
#define DAWN_TRY_ASSIGN_CONTEXT(VAR, EXPR, ...) \
    DAWN_TRY_ASSIGN_WITH_CLEANUP(               \
        VAR, EXPR, { DAWN_LOCAL_VAR(Error)->AppendContext(absl::StrFormat(__VA_ARGS__)); })

// Argument helpers are used to determine which macro implementations should be called when
// overloading with different number of variables.
#define DAWN_ERROR_UNIMPLEMENTED_MACRO_(...) DAWN_UNREACHABLE()

// Example usage:
//  Result res;
//  DAWN_TRY_ASSIGN_WITH_CLEANUP(
//      res, GetResultOrErrorFunction(), {
//          AddAdditionalErrorInformation(DAWN_LOCAL_VAR(Error).get());
//      });
//
#define DAWN_TRY_ASSIGN_WITH_CLEANUP(VAR, EXPR, BODY)                              \
    {                                                                              \
        auto DAWN_LOCAL_VAR(Result) = EXPR;                                        \
        if (DAWN_LOCAL_VAR(Result).IsError()) [[unlikely]] {                       \
            auto DAWN_LOCAL_VAR(Error) = DAWN_LOCAL_VAR(Result).AcquireError();    \
            {BODY} /* comment to force the formatter to insert a newline */        \
            DAWN_APPEND_ERROR_BACKTRACE(DAWN_LOCAL_VAR(Error));                    \
            return ::dawn::native::ErrorAdapter{std::move(DAWN_LOCAL_VAR(Error))}; \
        }                                                                          \
        VAR = DAWN_LOCAL_VAR(Result).AcquireSuccess();                             \
    }                                                                              \
    for (;;)                                                                       \
    break

// Assert that errors are device loss so that we can continue with destruction
void IgnoreErrors(MaybeUnknownError maybeError);
void IgnoreErrors(MaybeError maybeError);

wgpu::ErrorType ToWGPUErrorType(InternalErrorType type);
InternalErrorType FromWGPUErrorType(wgpu::ErrorType type);

absl::FormatConvertResult<absl::FormatConversionCharSet::kString |
                          absl::FormatConversionCharSet::kIntegral>
AbslFormatConvert(InternalErrorType value,
                  const absl::FormatConversionSpec& spec,
                  absl::FormatSink* s);

}  // namespace dawn::native

// Enable dawn enum bitmask for error types.
template <>
struct wgpu::IsWGPUBitmask<dawn::native::InternalErrorType> {
    static constexpr bool enable = true;
};

#endif  // SRC_DAWN_NATIVE_ERROR_H_
