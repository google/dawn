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

#ifndef SRC_DAWN_COMMON_RESULT_H_
#define SRC_DAWN_COMMON_RESULT_H_

#include <concepts>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <utility>
#include <variant>

#include "src/dawn/common/Compiler.h"
#include "src/utils/assert.h"

namespace dawn {

// Result<T, E> is the following sum type (Haskell notation):
//
//      data Result T E = Success T | Error E | Empty
//
// It is meant to be used as the return type of functions that might fail. The reason for the Empty
// case is that a Result should never be discarded, only destructured (its error or success moved
// out) or moved into a different Result. The Empty case tags Results that have been moved out and
// Result's destructor should DAWN_ASSERT on it being Empty.
//
// Since C++ doesn't have efficient sum types for the special cases we care about, we provide
// template specializations for them.

template <typename T, typename E>
class Result;

template <typename TChild, typename T>
concept ValidResultConversions = std::same_as<TChild, T> || std::derived_from<TChild, T>;

template <typename E2, typename E>
concept ValidErrorConversions = std::same_as<E2, E> || std::convertible_to<std::unique_ptr<E2>, E>;

namespace detail {
template <typename E, typename E2>
    requires ValidErrorConversions<E2, E>
std::unique_ptr<E> MoveError(std::unique_ptr<E2> error) {
    if constexpr (std::same_as<E, E2>) {
        return error;
    } else {
        if (error == nullptr) {
            return nullptr;
        }
        return std::make_unique<E>(std::move(error));
    }
}
}  // namespace detail

// The interface of Result<T, E> should look like the following.
//  public:
//    Result(T&& success);
//    Result(std::unique_ptr<E> error);
//
//    Result(Result<T, E>&& other);
//    Result<T, E>& operator=(Result<T, E>&& other);
//
//    ~Result();
//
//    bool IsError() const;
//    bool IsSuccess() const;
//
//    T&& AcquireSuccess();
//    std::unique_ptr<E> AcquireError();

// Specialization of Result for returning errors only via pointers. It is basically a pointer
// where nullptr is both Success and Empty.
template <typename E>
class [[nodiscard]] Result<void, E> {
  public:
    Result();
    template <typename E2>
        requires ValidErrorConversions<E2, E>
    explicit(false) Result(std::unique_ptr<E2> error);
    template <typename E2>
        requires ValidErrorConversions<E2, E>
    Result<void, E>& operator=(std::unique_ptr<E2> error);

    template <typename E2>
        requires ValidErrorConversions<E2, E>
    explicit(false) Result(Result<void, E2>&& other);
    template <typename E2>
        requires ValidErrorConversions<E2, E>
    Result<void, E>& operator=(Result<void, E2>&& other);

    ~Result();

    bool IsError() const;
    bool IsSuccess() const;

    void AcquireSuccess();
    std::unique_ptr<E> AcquireError();

  private:
    std::unique_ptr<E> mError;
};

// Uses SFINAE to try to get alignof(T) but fallback to Default if T isn't defined.
template <typename T, size_t Default, typename = size_t>
constexpr size_t alignof_if_defined_else_default = Default;

template <typename T, size_t Default>
constexpr size_t alignof_if_defined_else_default<T, Default, decltype(alignof(T))> = alignof(T);

// Specialization of Result when both the error an success are pointers. It is implemented as a
// tagged pointer. The tag for Success is 0 so that returning the value is fastest.

namespace detail {
// Utility functions to manipulate the tagged pointer. Some of them don't need to be templated
// but we really want them inlined so we keep them in the headers
enum PayloadType {
    Success = 0,
    Error = 1,
    Empty = 2,
};

intptr_t MakePayload(const void* pointer, PayloadType type);
PayloadType GetPayloadType(intptr_t payload);

template <typename T>
static T* GetSuccessFromPayload(intptr_t payload);
template <typename E>
static E* GetErrorFromPayload(intptr_t payload);

constexpr static intptr_t kEmptyPayload = Empty;
}  // namespace detail

template <typename T, typename E>
class [[nodiscard]] Result<T*, E> {
  public:
    static_assert(alignof_if_defined_else_default<T, 4> >= 4,
                  "Result<T*, E*> reserves two bits for tagging pointers");
    static_assert(alignof_if_defined_else_default<E, 4> >= 4,
                  "Result<T*, E*> reserves two bits for tagging pointers");

    explicit(false) Result(T* success);
    template <typename E2>
        requires ValidErrorConversions<E2, E>
    explicit(false) Result(std::unique_ptr<E2> error);

    // Support returning a Result<T*, E*> from a Result<TChild*, E2*>
    template <typename TChild, typename E2>
        requires(ValidResultConversions<TChild, T> && ValidErrorConversions<E2, E>)
    explicit(false) Result(Result<TChild*, E2>&& other);
    template <typename TChild, typename E2>
        requires(ValidResultConversions<TChild, T> && ValidErrorConversions<E2, E>)
    Result<T*, E>& operator=(Result<TChild*, E2>&& other);

    ~Result();

    bool IsError() const;
    bool IsSuccess() const;

    T* AcquireSuccess();
    std::unique_ptr<E> AcquireError();

  private:
    template <typename T2, typename E2>
    friend class Result;

    intptr_t mPayload = detail::kEmptyPayload;
};

template <typename T, typename E>
class [[nodiscard]] Result<const T*, E> {
  public:
    static_assert(alignof_if_defined_else_default<T, 4> >= 4,
                  "Result<T*, E*> reserves two bits for tagging pointers");
    static_assert(alignof_if_defined_else_default<E, 4> >= 4,
                  "Result<T*, E*> reserves two bits for tagging pointers");

    explicit(false) Result(const T* success);
    template <typename E2>
        requires ValidErrorConversions<E2, E>
    explicit(false) Result(std::unique_ptr<E2> error);

    template <typename TChild, typename E2>
        requires(ValidResultConversions<TChild, T> && ValidErrorConversions<E2, E>)
    explicit(false) Result(Result<const TChild*, E2>&& other);
    template <typename TChild, typename E2>
        requires(ValidResultConversions<TChild, T> && ValidErrorConversions<E2, E>)
    Result<const T*, E>& operator=(Result<const TChild*, E2>&& other);

    ~Result();

    bool IsError() const;
    bool IsSuccess() const;

    const T* AcquireSuccess();
    std::unique_ptr<E> AcquireError();

  private:
    template <typename T2, typename E2>
    friend class Result;

    intptr_t mPayload = detail::kEmptyPayload;
};

template <typename T>
class Ref;

template <typename T, typename E>
class [[nodiscard]] Result<Ref<T>, E> {
  public:
    static_assert(alignof_if_defined_else_default<T, 4> >= 4,
                  "Result<Ref<T>, E> reserves two bits for tagging pointers");
    static_assert(alignof_if_defined_else_default<E, 4> >= 4,
                  "Result<Ref<T>, E> reserves two bits for tagging pointers");

    template <typename TChild>
        requires ValidResultConversions<TChild, T>
    explicit(false) Result(Ref<TChild>&& success);
    template <typename TChild>
        requires ValidResultConversions<TChild, T>
    explicit(false) Result(const Ref<TChild>& success);
    template <typename E2>
        requires ValidErrorConversions<E2, E>
    explicit(false) Result(std::unique_ptr<E2> error);
    explicit(false) constexpr Result(std::nullptr_t);

    template <typename TChild, typename E2>
        requires(ValidResultConversions<TChild, T> && ValidErrorConversions<E2, E>)
    explicit(false) Result(Result<Ref<TChild>, E2>&& other);

    template <typename TChild, typename E2>
        requires(ValidResultConversions<TChild, T> && ValidErrorConversions<E2, E>)
    Result<Ref<T>, E>& operator=(Result<Ref<TChild>, E2>&& other);

    ~Result();

    bool IsError() const;
    bool IsSuccess() const;

    Ref<T> AcquireSuccess();
    std::unique_ptr<E> AcquireError();

  private:
    template <typename T2, typename E2>
    friend class Result;

    intptr_t mPayload = detail::kEmptyPayload;
};

// Catchall definition of Result<T, E> implemented as a tagged struct. It could be improved to use
// a tagged union instead if it turns out to be a hotspot. T and E must be movable and default
// constructible.
template <typename T, typename E>
class [[nodiscard]] Result {
  public:
    explicit(false) Result(T success);
    template <typename E2>
        requires ValidErrorConversions<E2, E>
    explicit(false) Result(std::unique_ptr<E2> error);

    template <typename E2>
        requires ValidErrorConversions<E2, E>
    explicit(false) Result(Result<T, E2>&& other);
    template <typename E2>
        requires ValidErrorConversions<E2, E>
    Result<T, E>& operator=(Result<T, E2>&& other);

    ~Result();

    bool IsError() const;
    bool IsSuccess() const;

    T AcquireSuccess();
    std::unique_ptr<E> AcquireError();

  private:
    std::variant<std::monostate, T, std::unique_ptr<E>> mPayload;
};

// Implementation of Result<void, E>
template <typename E>
Result<void, E>::Result() {}

template <typename E>
template <typename E2>
    requires ValidErrorConversions<E2, E>
Result<void, E>::Result(std::unique_ptr<E2> error)
    : mError(detail::MoveError<E>(std::move(error))) {}

template <typename E>
template <typename E2>
    requires ValidErrorConversions<E2, E>
Result<void, E>& Result<void, E>::operator=(std::unique_ptr<E2> error) {
    DAWN_ASSERT(mError == nullptr);
    mError = detail::MoveError<E>(std::move(error));
    return *this;
}

template <typename E>
template <typename E2>
    requires ValidErrorConversions<E2, E>
Result<void, E>::Result(Result<void, E2>&& other) : Result(other.AcquireError()) {}

template <typename E>
template <typename E2>
    requires ValidErrorConversions<E2, E>
Result<void, E>& Result<void, E>::operator=(Result<void, E2>&& other) {
    *this = other.AcquireError();
    return *this;
}

template <typename E>
Result<void, E>::~Result() {
    DAWN_ASSERT(mError == nullptr);
}

template <typename E>
bool Result<void, E>::IsError() const {
    return mError != nullptr;
}

template <typename E>
bool Result<void, E>::IsSuccess() const {
    return mError == nullptr;
}

template <typename E>
void Result<void, E>::AcquireSuccess() {}

template <typename E>
std::unique_ptr<E> Result<void, E>::AcquireError() {
    return std::move(mError);
}

// Implementation details of the tagged pointer Results
namespace detail {

template <typename T>
T* GetSuccessFromPayload(intptr_t payload) {
    DAWN_ASSERT(GetPayloadType(payload) == Success);
    return reinterpret_cast<T*>(payload);
}

template <typename E>
E* GetErrorFromPayload(intptr_t payload) {
    DAWN_ASSERT(GetPayloadType(payload) == Error);
    return reinterpret_cast<E*>(payload ^ 1);
}

}  // namespace detail

// Implementation of Result<T*, E>
template <typename T, typename E>
Result<T*, E>::Result(T* success) : mPayload(detail::MakePayload(success, detail::Success)) {}

template <typename T, typename E>
template <typename E2>
    requires ValidErrorConversions<E2, E>
Result<T*, E>::Result(std::unique_ptr<E2> error)
    : mPayload(
          detail::MakePayload(detail::MoveError<E>(std::move(error)).release(), detail::Error)) {}

template <typename T, typename E>
template <typename TChild, typename E2>
    requires(ValidResultConversions<TChild, T> && ValidErrorConversions<E2, E>)
Result<T*, E>::Result(Result<TChild*, E2>&& other) {
    *this = std::move(other);
}

template <typename T, typename E>
template <typename TChild, typename E2>
    requires(ValidResultConversions<TChild, T> && ValidErrorConversions<E2, E>)
Result<T*, E>& Result<T*, E>::operator=(Result<TChild*, E2>&& other) {
    DAWN_ASSERT(mPayload == detail::kEmptyPayload);
    if (other.IsError()) {
        mPayload = detail::MakePayload(detail::MoveError<E>(other.AcquireError()).release(),
                                       detail::Error);
    } else {
        mPayload = other.mPayload;
        other.mPayload = detail::kEmptyPayload;
    }
    return *this;
}

template <typename T, typename E>
Result<T*, E>::~Result() {
    DAWN_ASSERT(mPayload == detail::kEmptyPayload);
}

template <typename T, typename E>
bool Result<T*, E>::IsError() const {
    return detail::GetPayloadType(mPayload) == detail::Error;
}

template <typename T, typename E>
bool Result<T*, E>::IsSuccess() const {
    return detail::GetPayloadType(mPayload) == detail::Success;
}

template <typename T, typename E>
T* Result<T*, E>::AcquireSuccess() {
    T* success = detail::GetSuccessFromPayload<T>(mPayload);
    mPayload = detail::kEmptyPayload;
    return success;
}

template <typename T, typename E>
std::unique_ptr<E> Result<T*, E>::AcquireError() {
    std::unique_ptr<E> error(detail::GetErrorFromPayload<E>(mPayload));
    mPayload = detail::kEmptyPayload;
    return std::move(error);
}

// Implementation of Result<const T*, E*>
template <typename T, typename E>
Result<const T*, E>::Result(const T* success)
    : mPayload(detail::MakePayload(success, detail::Success)) {}

template <typename T, typename E>
template <typename E2>
    requires ValidErrorConversions<E2, E>
Result<const T*, E>::Result(std::unique_ptr<E2> error)
    : mPayload(
          detail::MakePayload(detail::MoveError<E>(std::move(error)).release(), detail::Error)) {}

template <typename T, typename E>
template <typename TChild, typename E2>
    requires(ValidResultConversions<TChild, T> && ValidErrorConversions<E2, E>)
Result<const T*, E>::Result(Result<const TChild*, E2>&& other) {
    *this = std::move(other);
}

template <typename T, typename E>
template <typename TChild, typename E2>
    requires(ValidResultConversions<TChild, T> && ValidErrorConversions<E2, E>)
Result<const T*, E>& Result<const T*, E>::operator=(Result<const TChild*, E2>&& other) {
    DAWN_ASSERT(mPayload == detail::kEmptyPayload);
    if (other.IsError()) {
        mPayload = detail::MakePayload(detail::MoveError<E>(other.AcquireError()).release(),
                                       detail::Error);
    } else {
        mPayload = other.mPayload;
        other.mPayload = detail::kEmptyPayload;
    }
    return *this;
}

template <typename T, typename E>
Result<const T*, E>::~Result() {
    DAWN_ASSERT(mPayload == detail::kEmptyPayload);
}

template <typename T, typename E>
bool Result<const T*, E>::IsError() const {
    return detail::GetPayloadType(mPayload) == detail::Error;
}

template <typename T, typename E>
bool Result<const T*, E>::IsSuccess() const {
    return detail::GetPayloadType(mPayload) == detail::Success;
}

template <typename T, typename E>
const T* Result<const T*, E>::AcquireSuccess() {
    T* success = detail::GetSuccessFromPayload<T>(mPayload);
    mPayload = detail::kEmptyPayload;
    return success;
}

template <typename T, typename E>
std::unique_ptr<E> Result<const T*, E>::AcquireError() {
    std::unique_ptr<E> error(detail::GetErrorFromPayload<E>(mPayload));
    mPayload = detail::kEmptyPayload;
    return std::move(error);
}

// Implementation of Result<Ref<T>, E>
template <typename T, typename E>
constexpr Result<Ref<T>, E>::Result(std::nullptr_t) : Result(Ref<T>(nullptr)) {}

template <typename T, typename E>
template <typename TChild>
    requires ValidResultConversions<TChild, T>
Result<Ref<T>, E>::Result(Ref<TChild>&& success)
    : mPayload(detail::MakePayload(success.Detach(), detail::Success)) {}

template <typename T, typename E>
template <typename TChild>
    requires ValidResultConversions<TChild, T>
Result<Ref<T>, E>::Result(const Ref<TChild>& success) : Result(Ref<TChild>(success)) {}

template <typename T, typename E>
template <typename E2>
    requires ValidErrorConversions<E2, E>
Result<Ref<T>, E>::Result(std::unique_ptr<E2> error)
    : mPayload(
          detail::MakePayload(detail::MoveError<E>(std::move(error)).release(), detail::Error)) {}

template <typename T, typename E>
template <typename TChild, typename E2>
    requires(ValidResultConversions<TChild, T> && ValidErrorConversions<E2, E>)
Result<Ref<T>, E>::Result(Result<Ref<TChild>, E2>&& other) {
    *this = std::move(other);
}

template <typename T, typename E>
template <typename TChild, typename E2>
    requires(ValidResultConversions<TChild, T> && ValidErrorConversions<E2, E>)
Result<Ref<T>, E>& Result<Ref<T>, E>::operator=(Result<Ref<TChild>, E2>&& other) {
    DAWN_ASSERT(mPayload == detail::kEmptyPayload);
    if (other.IsError()) {
        mPayload = detail::MakePayload(detail::MoveError<E>(other.AcquireError()).release(),
                                       detail::Error);
    } else {
        mPayload = other.mPayload;
        other.mPayload = detail::kEmptyPayload;
    }
    return *this;
}

template <typename T, typename E>
Result<Ref<T>, E>::~Result() {
    DAWN_ASSERT(mPayload == detail::kEmptyPayload);
}

template <typename T, typename E>
bool Result<Ref<T>, E>::IsError() const {
    return detail::GetPayloadType(mPayload) == detail::Error;
}

template <typename T, typename E>
bool Result<Ref<T>, E>::IsSuccess() const {
    return detail::GetPayloadType(mPayload) == detail::Success;
}

template <typename T, typename E>
Ref<T> Result<Ref<T>, E>::AcquireSuccess() {
    DAWN_ASSERT(IsSuccess());
    Ref<T> success = AcquireRef(detail::GetSuccessFromPayload<T>(mPayload));
    mPayload = detail::kEmptyPayload;
    return success;
}

template <typename T, typename E>
std::unique_ptr<E> Result<Ref<T>, E>::AcquireError() {
    DAWN_ASSERT(IsError());
    std::unique_ptr<E> error(detail::GetErrorFromPayload<E>(mPayload));
    mPayload = detail::kEmptyPayload;
    return std::move(error);
}

// Implementation of Result<T, E>
template <typename T, typename E>
Result<T, E>::Result(T success) : mPayload(std::move(success)) {}

template <typename T, typename E>
template <typename E2>
    requires ValidErrorConversions<E2, E>
Result<T, E>::Result(std::unique_ptr<E2> error)
    : mPayload(detail::MoveError<E>(std::move(error))) {}

template <typename T, typename E>
Result<T, E>::~Result() {
    // Happens if you ignore a result.
    DAWN_ASSERT(std::holds_alternative<std::monostate>(mPayload));
}

template <typename T, typename E>
template <typename E2>
    requires ValidErrorConversions<E2, E>
Result<T, E>::Result(Result<T, E2>&& other) {
    *this = std::move(other);
}

template <typename T, typename E>
template <typename E2>
    requires ValidErrorConversions<E2, E>
Result<T, E>& Result<T, E>::operator=(Result<T, E2>&& other) {
    DAWN_ASSERT(std::holds_alternative<std::monostate>(mPayload));
    if (other.IsError()) {
        mPayload = detail::MoveError<E>(other.AcquireError());
    } else if (other.IsSuccess()) {
        mPayload = other.AcquireSuccess();
    }
    return *this;
}

template <typename T, typename E>
bool Result<T, E>::IsError() const {
    return std::holds_alternative<std::unique_ptr<E>>(mPayload);
}

template <typename T, typename E>
bool Result<T, E>::IsSuccess() const {
    return std::holds_alternative<T>(mPayload);
}

template <typename T, typename E>
T Result<T, E>::AcquireSuccess() {
    DAWN_ASSERT(IsSuccess());
    auto payload = std::move(mPayload);
    mPayload = {};
    return std::move(std::get<T>(payload));
}

template <typename T, typename E>
std::unique_ptr<E> Result<T, E>::AcquireError() {
    DAWN_ASSERT(IsError());
    auto payload = std::move(mPayload);
    mPayload = {};
    return std::move(std::get<std::unique_ptr<E>>(payload));
}

}  // namespace dawn

#endif  // SRC_DAWN_COMMON_RESULT_H_
