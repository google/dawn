// Copyright 2026 The Dawn & Tint Authors
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

#include <memory>
#include <tuple>

#include "src/dawn/common/Ref.h"
#include "src/dawn/common/RefCounted.h"
#include "src/dawn/native/Error.h"

namespace dawn::native {

constexpr const char* kErrorMessage = "Error message";

class RefT : public RefCounted {};

std::unique_ptr<UnknownError> MakeUnknownError() {
    return std::make_unique<UnknownError>(DAWN_UNRECOVERABLE_ERROR(kErrorMessage));
}

MaybeValError FuncReturningMaybeValError() {
    return DAWN_VALIDATION_ERROR(kErrorMessage);
}

MaybeError FuncReturningMaybeUnrecoverableError() {
    return DAWN_UNRECOVERABLE_ERROR(kErrorMessage);
}

MaybeUnknownError FuncReturningMaybeUnknownError() {
    return DAWN_UNRECOVERABLE_ERROR(kErrorMessage);
}

template <typename T>
ResultOrValError<T> FuncReturningResultOrValError() {
    return DAWN_VALIDATION_ERROR(kErrorMessage);
}

template <typename T>
ResultOrError<T> FuncReturningResultOrUnrecoverableError() {
    return DAWN_UNRECOVERABLE_ERROR(kErrorMessage);
}

template <typename T>
ResultOrUnknownError<T> FuncReturningResultOrUnknownError() {
    return DAWN_UNRECOVERABLE_ERROR(kErrorMessage);
}

// Verify that UnrecoverableError and UnknownError cannot be returned when ValidationError is
// expected.
void TestReturnDirectValidationError() {
    // Control cases.
    std::ignore = []() -> UnknownError { return DAWN_VALIDATION_ERROR(kErrorMessage); };
    std::ignore = []() -> UnrecoverableError { return DAWN_VALIDATION_ERROR(kErrorMessage); };
    std::ignore = []() -> std::unique_ptr<ValidationError> {
        return DAWN_VALIDATION_ERROR(kErrorMessage);
    };

    std::ignore = []() -> ValidationError {
        return DAWN_UNRECOVERABLE_ERROR(kErrorMessage);  // expected-error {{no viable conversion}}
    };
    std::ignore = []() -> ValidationError {
        return MakeUnknownError();  // expected-error {{no viable conversion}}
    };
    std::ignore = []() -> std::unique_ptr<ValidationError> {
        return DAWN_UNRECOVERABLE_ERROR(kErrorMessage);  // expected-error {{no viable conversion}}
    };
    std::ignore = []() -> std::unique_ptr<ValidationError> {
        return MakeUnknownError();  // expected-error {{no viable conversion}}
    };
}

// Verify that Unrecoverable and Unknown errors cannot be returned or propagated via DAWN_TRY /
// DAWN_TRY_ASSIGN in a function returning MaybeValError.
void TestMaybeValError() {
    // Direct error returns.
    std::ignore = []() -> MaybeValError {
        return DAWN_VALIDATION_ERROR(kErrorMessage);  // Control case.
    };
    std::ignore = []() -> MaybeValError {
        return DAWN_UNRECOVERABLE_ERROR(kErrorMessage);  // expected-error {{no viable conversion}}
    };
    std::ignore = []() -> MaybeValError {
        return MakeUnknownError();  // expected-error {{no viable conversion}}
    };

    // Direct Maybe*Error returns.
    std::ignore = []() -> MaybeValError {
        return FuncReturningMaybeValError();  // Control case.
    };
    std::ignore = []() -> MaybeValError {
        return FuncReturningMaybeUnrecoverableError();  // expected-error {{no viable conversion}}
    };
    std::ignore = []() -> MaybeValError {
        return FuncReturningMaybeUnknownError();  // expected-error {{no viable conversion}}
    };

    // DAWN_TRY with functions returning Maybe*Error.
    std::ignore = []() -> MaybeValError {
        DAWN_TRY(FuncReturningMaybeValError());  // Control case.
        return {};
    };
    std::ignore = []() -> MaybeValError {
        DAWN_TRY(FuncReturningMaybeUnrecoverableError());  // expected-error {{no viable conversion}}
        return {};
    };
    std::ignore = []() -> MaybeValError {
        DAWN_TRY(FuncReturningMaybeUnknownError());  // expected-error {{no viable conversion}}
        return {};
    };

    // DAWN_TRY with functions returning ResultOr*Error.
    std::ignore = []() -> MaybeValError {
        DAWN_TRY(FuncReturningResultOrValError<int>());  // Control case.
        return {};
    };
    std::ignore = []() -> MaybeValError {
        DAWN_TRY(FuncReturningResultOrUnrecoverableError<int>());  // expected-error {{no viable conversion}}
        return {};
    };
    std::ignore = []() -> MaybeValError {
        DAWN_TRY(FuncReturningResultOrUnknownError<int>());  // expected-error {{no viable conversion}}
        return {};
    };

    // DAWN_TRY_ASSIGN with functions returning ResultOr*Error.
    std::ignore = []() -> MaybeValError {
        [[maybe_unused]] int v = 0;
        DAWN_TRY_ASSIGN(v, FuncReturningResultOrValError<int>());  // Control case.
        return {};
    };
    std::ignore = []() -> MaybeValError {
        [[maybe_unused]] int v = 0;
        DAWN_TRY_ASSIGN(v, FuncReturningResultOrUnrecoverableError<int>());  // expected-error {{no viable conversion}}
        return {};
    };
    std::ignore = []() -> MaybeValError {
        [[maybe_unused]] int v = 0;
        DAWN_TRY_ASSIGN(v, FuncReturningResultOrUnknownError<int>());  // expected-error {{no viable conversion}}
        return {};
    };
}

// Verify that Unrecoverable and Unknown errors cannot be returned or propagated via DAWN_TRY /
// DAWN_TRY_ASSIGN in a function returning ResultOrValError<T>.
void TestResultOrValError() {
    // Direct error returns.
    std::ignore = []() -> ResultOrValError<int> {
        return DAWN_VALIDATION_ERROR(kErrorMessage);  // Control case.
    };
    std::ignore = []() -> ResultOrValError<int> {
        return DAWN_UNRECOVERABLE_ERROR(kErrorMessage);  // expected-error {{no viable conversion}}
    };
    std::ignore = []() -> ResultOrValError<int> {
        return MakeUnknownError();  // expected-error {{no viable conversion}}
    };

    // Direct ResultOr*Error returns across Result specializations.
    std::ignore = []() -> ResultOrValError<int> {
        return FuncReturningResultOrValError<int>();  // Control case.
    };
    std::ignore = []() -> ResultOrValError<int> {
        return FuncReturningResultOrUnrecoverableError<int>();  // expected-error {{no viable conversion}}
    };
    std::ignore = []() -> ResultOrValError<int> {
        return FuncReturningResultOrUnknownError<int>();  // expected-error {{no viable conversion}}
    };
    std::ignore = []() -> ResultOrValError<RefT*> {
        return FuncReturningResultOrValError<RefT*>();  // Control case.
    };
    std::ignore = []() -> ResultOrValError<RefT*> {
        return FuncReturningResultOrUnrecoverableError<RefT*>();  // expected-error {{no viable conversion}}
    };
    std::ignore = []() -> ResultOrValError<RefT*> {
        return FuncReturningResultOrUnknownError<RefT*>();  // expected-error {{no viable conversion}}
    };
    std::ignore = []() -> ResultOrValError<const RefT*> {
        return FuncReturningResultOrValError<const RefT*>();  // Control case.
    };
    std::ignore = []() -> ResultOrValError<const RefT*> {
        return FuncReturningResultOrUnrecoverableError<const RefT*>();  // expected-error {{no viable conversion}}
    };
    std::ignore = []() -> ResultOrValError<const RefT*> {
        return FuncReturningResultOrUnknownError<const RefT*>();  // expected-error {{no viable conversion}}
    };
    std::ignore = []() -> ResultOrValError<Ref<RefT>> {
        return FuncReturningResultOrValError<Ref<RefT>>();  // Control case.
    };
    std::ignore = []() -> ResultOrValError<Ref<RefT>> {
        return FuncReturningResultOrUnrecoverableError<Ref<RefT>>();  // expected-error {{no viable conversion}}
    };
    std::ignore = []() -> ResultOrValError<Ref<RefT>> {
        return FuncReturningResultOrUnknownError<Ref<RefT>>();  // expected-error {{no viable conversion}}
    };

    // DAWN_TRY with functions returning Maybe*Error.
    std::ignore = []() -> ResultOrValError<int> {
        DAWN_TRY(FuncReturningMaybeValError());  // Control case.
        return 0;
    };
    std::ignore = []() -> ResultOrValError<int> {
        DAWN_TRY(FuncReturningMaybeUnrecoverableError());  // expected-error {{no viable conversion}}
        return 0;
    };
    std::ignore = []() -> ResultOrValError<int> {
        DAWN_TRY(FuncReturningMaybeUnknownError());  // expected-error {{no viable conversion}}
        return 0;
    };

    // DAWN_TRY with functions returning ResultOr*Error.
    std::ignore = []() -> ResultOrValError<int> {
        DAWN_TRY(FuncReturningResultOrValError<int>());  // Control case.
        return 0;
    };
    std::ignore = []() -> ResultOrValError<int> {
        DAWN_TRY(FuncReturningResultOrUnrecoverableError<int>());  // expected-error {{no viable conversion}}
        return 0;
    };
    std::ignore = []() -> ResultOrValError<int> {
        DAWN_TRY(FuncReturningResultOrUnknownError<int>());  // expected-error {{no viable conversion}}
        return 0;
    };

    // DAWN_TRY_ASSIGN with functions returning ResultOr*Error.
    std::ignore = []() -> ResultOrValError<int> {
        int v = 0;
        DAWN_TRY_ASSIGN(v, FuncReturningResultOrValError<int>());  // Control case.
        return v;
    };
    std::ignore = []() -> ResultOrValError<int> {
        int v = 0;
        DAWN_TRY_ASSIGN(v, FuncReturningResultOrUnrecoverableError<int>());  // expected-error {{no viable conversion}}
        return v;
    };
    std::ignore = []() -> ResultOrValError<int> {
        int v = 0;
        DAWN_TRY_ASSIGN(v, FuncReturningResultOrUnknownError<int>());  // expected-error {{no viable conversion}}
        return v;
    };
}

}  // namespace dawn::native
