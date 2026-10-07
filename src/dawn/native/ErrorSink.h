// Copyright 2024 The Dawn & Tint Authors
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

#ifndef SRC_DAWN_NATIVE_ERRORSINK_H_
#define SRC_DAWN_NATIVE_ERRORSINK_H_

#include <concepts>
#include <memory>
#include <utility>

#include "src/dawn/native/Error.h"
#include "src/dawn/native/ErrorData.h"

namespace dawn::native {

class ErrorSink {
  public:
    virtual ~ErrorSink() = default;

    // Variants of ConsumedError must use the returned boolean to handle failure cases since an
    // error may cause a fatal error and further execution may be undefined. This is especially
    // true for the ResultOrError variants.
    template <typename E>
        requires(IsMaybeError<E>)
    [[nodiscard]] bool ConsumedError(
        E maybeError,
        InternalErrorType additionalAllowedErrors = InternalErrorType::None) {
        if (maybeError.IsError()) [[unlikely]] {
            ConsumeError(maybeError.AcquireError(), additionalAllowedErrors);
            return true;
        }
        return false;
    }

    template <typename E, typename... Args>
        requires(IsMaybeError<E>)
    [[nodiscard]] bool ConsumedError(E maybeError,
                                     InternalErrorType additionalAllowedErrors,
                                     const char* formatStr,
                                     const Args&... args) {
        if (maybeError.IsError()) [[unlikely]] {
            ConsumeError(maybeError.AcquireError(), additionalAllowedErrors, formatStr, args...);
            return true;
        }
        return false;
    }

    template <typename E, typename... Args>
        requires(IsMaybeError<E>)
    [[nodiscard]] bool ConsumedError(E maybeError, const char* formatStr, const Args&... args) {
        return ConsumedError(std::move(maybeError), InternalErrorType::None, formatStr, args...);
    }

    template <typename E, typename T>
        requires(IsResultOrError<E, T>)
    [[nodiscard]] bool ConsumedError(
        E resultOrError,
        T* result,
        InternalErrorType additionalAllowedErrors = InternalErrorType::None) {
        if (resultOrError.IsError()) [[unlikely]] {
            ConsumeError(resultOrError.AcquireError(), additionalAllowedErrors);
            return true;
        }
        *result = resultOrError.AcquireSuccess();
        return false;
    }

    template <typename E, typename T, typename... Args>
        requires(IsResultOrError<E, T>)
    [[nodiscard]] bool ConsumedError(E resultOrError,
                                     T* result,
                                     const char* formatStr,
                                     const Args&... args) {
        return ConsumedError(std::move(resultOrError), result, InternalErrorType::None, formatStr,
                             args...);
    }

    template <typename E, typename T, typename... Args>
        requires(IsResultOrError<E, T>)
    [[nodiscard]] bool ConsumedError(E resultOrError,
                                     T* result,
                                     InternalErrorType additionalAllowedErrors,
                                     const char* formatStr,
                                     const Args&... args) {
        if (resultOrError.IsError()) [[unlikely]] {
            ConsumeError(resultOrError.AcquireError(), additionalAllowedErrors, formatStr, args...);
            return true;
        }
        *result = resultOrError.AcquireSuccess();
        return false;
    }

    template <typename T>
    void ConsumeError(ErrorAdapter<T> error) {
        ConsumeErrorImpl(error);
    }

    template <typename T, typename... Args>
        requires(IsConcreteError<T> || std::is_same_v<T, UnknownError>)
    void ConsumeError(std::unique_ptr<T> error) {
        ConsumeError(std::move(error), InternalErrorType::None);
    }

    template <typename T, typename... Args>
        requires(IsConcreteError<T> || std::is_same_v<T, UnknownError>)
    void ConsumeError(std::unique_ptr<T> error, InternalErrorType additionalAllowedErrors) {
        return ConsumeErrorImpl(ErrorAdapter{std::move(error)}, additionalAllowedErrors);
    }

    template <typename T, typename... Args>
        requires(IsConcreteError<T> || std::is_same_v<T, UnknownError>)
    void ConsumeError(std::unique_ptr<T> error, const char* formatStr, const Args&... args) {
        ConsumeError(std::move(error), InternalErrorType::None, formatStr, args...);
    }

    template <typename T, typename... Args>
        requires(IsConcreteError<T> || std::is_same_v<T, UnknownError>)
    void ConsumeError(std::unique_ptr<T> error,
                      InternalErrorType additionalAllowedErrors,
                      const char* formatStr,
                      const Args&... args) {
        ConsumeError(ErrorAdapter{std::move(error)}, additionalAllowedErrors, formatStr, args...);
    }

    template <typename T, typename... Args>
    void ConsumeError(ErrorAdapter<T> error,
                      InternalErrorType additionalAllowedErrors,
                      const char* formatStr,
                      const Args&... args) {
        if (static_cast<uint32_t>(error.GetData()->GetType()) &
            (static_cast<uint32_t>(additionalAllowedErrors) |
             static_cast<uint32_t>(InternalErrorType::Validation))) {
            error.GetData()->AppendContext(formatStr, args...);
        }
        ConsumeErrorImpl(std::move(error), additionalAllowedErrors);
    }

    virtual void ConsumeErrorImpl(
        std::unique_ptr<UnknownError> error,
        InternalErrorType additionalAllowedErrors = InternalErrorType::None) = 0;
};

}  // namespace dawn::native

#endif  // SRC_DAWN_NATIVE_ERRORSINK_H_
