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

#include <memory>
#include <string>

#include "gtest/gtest.h"
#include "src/dawn/common/Ref.h"
#include "src/dawn/common/RefCounted.h"
#include "src/dawn/native/Error.h"
#include "src/dawn/native/ErrorData.h"

namespace dawn::native {
namespace {

using ::testing::Test;
using ::testing::Types;

constexpr int kPlaceholderSuccess = 0xbeef;
constexpr const char* kPlaceholderErrorMessage = "I am an error message :3";

class BaseRefCounted : public RefCounted {};
class DerivedRefCounted : public BaseRefCounted {};

// Compile-time checks for allowed and disallowed error conversions.
static_assert(ValidErrorConversions<UnrecoverableError, UnknownError>);
static_assert(ValidErrorConversions<UnknownError, UnrecoverableError>);
static_assert(ValidErrorConversions<ValidationError, UnrecoverableError>);
static_assert(ValidErrorConversions<ValidationError, UnknownError>);

static_assert(!ValidErrorConversions<UnrecoverableError, ValidationError>);
static_assert(!ValidErrorConversions<UnknownError, ValidationError>);

// Check returning a success MaybeError with {};
TEST(ErrorTests, Error_Success) {
    auto ReturnSuccess = []() -> MaybeError { return {}; };

    MaybeError result = ReturnSuccess();
    ASSERT_TRUE(result.IsSuccess());
}

// Check returning an error MaybeError with "return DAWN_VALIDATION_ERROR"
TEST(ErrorTests, Error_Error) {
    auto ReturnError = []() -> MaybeValError {
        return DAWN_VALIDATION_ERROR(kPlaceholderErrorMessage);
    };

    MaybeValError result = ReturnError();
    ASSERT_TRUE(result.IsError());

    std::unique_ptr<ValidationError> errorData = result.AcquireError();
    ASSERT_EQ(errorData->GetMessage(), kPlaceholderErrorMessage);
}

// Check returning a success ResultOrError with an implicit conversion
TEST(ErrorTests, ResultOrError_Success) {
    auto ReturnSuccess = []() -> ResultOrError<const int*> { return &kPlaceholderSuccess; };

    ResultOrError<const int*> result = ReturnSuccess();
    ASSERT_TRUE(result.IsSuccess());
    ASSERT_EQ(result.AcquireSuccess(), &kPlaceholderSuccess);
}

// Check returning an error ResultOrError with "return DAWN_VALIDATION_ERROR"
TEST(ErrorTests, ResultOrError_Error) {
    auto ReturnError = []() -> ResultOrValError<const int*> {
        return DAWN_VALIDATION_ERROR(kPlaceholderErrorMessage);
    };

    ResultOrValError<const int*> result = ReturnError();
    ASSERT_TRUE(result.IsError());

    std::unique_ptr<ValidationError> errorData = result.AcquireError();
    ASSERT_EQ(errorData->GetMessage(), kPlaceholderErrorMessage);
}

// Check DAWN_TRY handles successes correctly.
TEST(ErrorTests, TRY_Success) {
    auto ReturnSuccess = []() -> MaybeError { return {}; };

    // We need to check that DAWN_TRY doesn't return on successes
    bool tryReturned = true;

    auto Try = [ReturnSuccess, &tryReturned]() -> MaybeError {
        DAWN_TRY(ReturnSuccess());
        tryReturned = false;
        return {};
    };

    MaybeError result = Try();
    ASSERT_TRUE(result.IsSuccess());
    ASSERT_FALSE(tryReturned);
}

// Check DAWN_TRY handles errors correctly.
TEST(ErrorTests, TRY_Error) {
    auto ReturnError = []() -> MaybeValError {
        return DAWN_VALIDATION_ERROR(kPlaceholderErrorMessage);
    };

    auto Try = [ReturnError]() -> MaybeValError {
        DAWN_TRY(ReturnError());
        // DAWN_TRY should return before this point
        EXPECT_FALSE(true);
        return {};
    };

    MaybeValError result = Try();
    ASSERT_TRUE(result.IsError());

    std::unique_ptr<ValidationError> errorData = result.AcquireError();
    ASSERT_EQ(errorData->GetMessage(), kPlaceholderErrorMessage);
}

// Check DAWN_TRY adds to the backtrace.
TEST(ErrorTests, TRY_AddsToBacktrace) {
    auto ReturnError = []() -> MaybeValError {
        return DAWN_VALIDATION_ERROR(kPlaceholderErrorMessage);
    };

    auto SingleTry = [ReturnError]() -> MaybeValError {
        DAWN_TRY(ReturnError());
        return {};
    };

    auto DoubleTry = [SingleTry]() -> MaybeValError {
        DAWN_TRY(SingleTry());
        return {};
    };

    MaybeValError singleResult = SingleTry();
    ASSERT_TRUE(singleResult.IsError());

    MaybeValError doubleResult = DoubleTry();
    ASSERT_TRUE(doubleResult.IsError());

    std::unique_ptr<ValidationError> singleData = singleResult.AcquireError();
    std::unique_ptr<ValidationError> doubleData = doubleResult.AcquireError();

    // Backtraces are only added in debug mode.
#if defined(DAWN_ENABLE_ASSERTS)
    ASSERT_EQ(singleData->GetBacktrace().size() + 1, doubleData->GetBacktrace().size());
#else
    ASSERT_EQ(singleData->GetBacktrace().size(), doubleData->GetBacktrace().size());
#endif
}

// Check DAWN_TRY_ASSIGN handles successes correctly.
TEST(ErrorTests, TRY_RESULT_Success) {
    auto ReturnSuccess = []() -> ResultOrError<const int*> { return &kPlaceholderSuccess; };

    // We need to check that DAWN_TRY doesn't return on successes
    bool tryReturned = true;

    auto Try = [ReturnSuccess, &tryReturned]() -> ResultOrError<const int*> {
        const int* result = nullptr;
        DAWN_TRY_ASSIGN(result, ReturnSuccess());
        tryReturned = false;

        EXPECT_EQ(result, &kPlaceholderSuccess);
        return result;
    };

    ResultOrError<const int*> result = Try();
    ASSERT_TRUE(result.IsSuccess());
    ASSERT_FALSE(tryReturned);
    ASSERT_EQ(result.AcquireSuccess(), &kPlaceholderSuccess);
}

// Check DAWN_TRY_ASSIGN handles errors correctly.
TEST(ErrorTests, TRY_RESULT_Error) {
    auto ReturnError = []() -> ResultOrValError<const int*> {
        return DAWN_VALIDATION_ERROR(kPlaceholderErrorMessage);
    };

    auto Try = [ReturnError]() -> ResultOrValError<const int*> {
        DAWN_TRY_ASSIGN(std::ignore, ReturnError());

        // DAWN_TRY should return before this point
        EXPECT_FALSE(true);
        return &kPlaceholderSuccess;
    };

    ResultOrValError<const int*> result = Try();
    ASSERT_TRUE(result.IsError());

    std::unique_ptr<ValidationError> errorData = result.AcquireError();
    ASSERT_EQ(errorData->GetMessage(), kPlaceholderErrorMessage);
}

// Check DAWN_TRY_ASSIGN adds to the backtrace.
TEST(ErrorTests, TRY_RESULT_AddsToBacktrace) {
    auto ReturnError = []() -> ResultOrValError<const int*> {
        return DAWN_VALIDATION_ERROR(kPlaceholderErrorMessage);
    };

    auto SingleTry = [ReturnError]() -> ResultOrValError<const int*> {
        DAWN_TRY(ReturnError());
        return &kPlaceholderSuccess;
    };

    auto DoubleTry = [SingleTry]() -> ResultOrValError<const int*> {
        DAWN_TRY(SingleTry());
        return &kPlaceholderSuccess;
    };

    ResultOrValError<const int*> singleResult = SingleTry();
    ASSERT_TRUE(singleResult.IsError());

    ResultOrValError<const int*> doubleResult = DoubleTry();
    ASSERT_TRUE(doubleResult.IsError());

    std::unique_ptr<ValidationError> singleData = singleResult.AcquireError();
    std::unique_ptr<ValidationError> doubleData = doubleResult.AcquireError();

    // Backtraces are only added in debug mode.
#if defined(DAWN_ENABLE_ASSERTS)
    ASSERT_EQ(singleData->GetBacktrace().size() + 1, doubleData->GetBacktrace().size());
#else
    ASSERT_EQ(singleData->GetBacktrace().size(), doubleData->GetBacktrace().size());
#endif
}

// Check a ResultOrError can be DAWN_TRY_ASSIGNED in a function that returns an Error
TEST(ErrorTests, TRY_RESULT_ConversionToError) {
    auto ReturnError = []() -> ResultOrValError<const int*> {
        return DAWN_VALIDATION_ERROR(kPlaceholderErrorMessage);
    };

    auto Try = [ReturnError]() -> MaybeValError {
        DAWN_TRY_ASSIGN(std::ignore, ReturnError());

        return {};
    };

    MaybeValError result = Try();
    ASSERT_TRUE(result.IsError());

    std::unique_ptr<ValidationError> errorData = result.AcquireError();
    ASSERT_EQ(errorData->GetMessage(), kPlaceholderErrorMessage);
}

// Check a ResultOrError can be DAWN_TRY_ASSIGNED in a function that returns an Error
// Version without Result<E*, T*>
TEST(ErrorTests, TRY_RESULT_ConversionToErrorNonPointer) {
    auto ReturnError = []() -> ResultOrValError<int> {
        return DAWN_VALIDATION_ERROR(kPlaceholderErrorMessage);
    };

    auto Try = [ReturnError]() -> MaybeValError {
        DAWN_TRY_ASSIGN(std::ignore, ReturnError());

        return {};
    };

    MaybeValError result = Try();
    ASSERT_TRUE(result.IsError());

    std::unique_ptr<ValidationError> errorData = result.AcquireError();
    ASSERT_EQ(errorData->GetMessage(), kPlaceholderErrorMessage);
}

// Check DAWN_TRY_ASSIGN handles successes correctly.
TEST(ErrorTests, TRY_RESULT_CLEANUP_Success) {
    auto ReturnSuccess = []() -> ResultOrError<const int*> { return &kPlaceholderSuccess; };

    // We need to check that DAWN_TRY_ASSIGN_WITH_CLEANUP doesn't return on successes and the
    // cleanup is not called.
    bool tryReturned = true;
    bool tryCleanup = false;

    auto Try = [ReturnSuccess, &tryReturned, &tryCleanup]() -> ResultOrError<const int*> {
        const int* result = nullptr;
        DAWN_TRY_ASSIGN_WITH_CLEANUP(result, ReturnSuccess(), { tryCleanup = true; });
        tryReturned = false;

        EXPECT_EQ(result, &kPlaceholderSuccess);
        return result;
    };

    ResultOrError<const int*> result = Try();
    ASSERT_TRUE(result.IsSuccess());
    ASSERT_FALSE(tryReturned);
    ASSERT_FALSE(tryCleanup);
    ASSERT_EQ(result.AcquireSuccess(), &kPlaceholderSuccess);
}

// Check DAWN_TRY_ASSIGN handles cleanups.
TEST(ErrorTests, TRY_RESULT_CLEANUP_Cleanup) {
    auto ReturnError = []() -> ResultOrValError<const int*> {
        return DAWN_VALIDATION_ERROR(kPlaceholderErrorMessage);
    };

    // We need to check that DAWN_TRY_ASSIGN_WITH_CLEANUP calls cleanup when error.
    bool tryCleanup = false;

    auto Try = [ReturnError, &tryCleanup]() -> ResultOrValError<const int*> {
        DAWN_TRY_ASSIGN_WITH_CLEANUP(std::ignore, ReturnError(), { tryCleanup = true; });

        // DAWN_TRY_ASSIGN_WITH_CLEANUP should return before this point
        EXPECT_FALSE(true);
        return &kPlaceholderSuccess;
    };

    ResultOrValError<const int*> result = Try();
    ASSERT_TRUE(result.IsError());

    std::unique_ptr<ValidationError> errorData = result.AcquireError();
    ASSERT_EQ(errorData->GetMessage(), kPlaceholderErrorMessage);
    ASSERT_TRUE(tryCleanup);
}

// Test returning the different errors directly as converted error types:
// - Unrecoverable -> Unknown
// - Unknown -> Unrecoverable
// - Validation -> Unrecoverable
// - Validation -> Unknown
// The other cases are not allowed and checked in nocompile tests.
TEST(ErrorTests, ReturnConvertedErrorTypes) {
    // Unrecoverable -> Unknown
    {
        auto ReturnUnrecoverableAsUnknown = []() -> UnknownError {
            return DAWN_UNRECOVERABLE_ERROR(kPlaceholderErrorMessage);
        };
        UnknownError err = ReturnUnrecoverableAsUnknown();
        EXPECT_EQ(err.GetType(), InternalErrorType::Unrecoverable);
        EXPECT_EQ(err.GetMessage(), kPlaceholderErrorMessage);
    }

    // Unknown -> Unrecoverable (when Unknown holds an unrecoverable error type)
    {
        auto ReturnUnknownUnrecoverableAsUnrecoverable = []() -> UnrecoverableError {
            std::unique_ptr<UnknownError> unknown =
                std::make_unique<UnknownError>(DAWN_UNRECOVERABLE_ERROR(kPlaceholderErrorMessage));
            return unknown;
        };
        UnrecoverableError err = ReturnUnknownUnrecoverableAsUnrecoverable();
        EXPECT_EQ(err.GetType(), InternalErrorType::Unrecoverable);
        EXPECT_EQ(err.GetMessage(), kPlaceholderErrorMessage);
    }

    // Unknown -> Unrecoverable (when Unknown holds a Validation error, converts type to
    // Unrecoverable)
    {
        auto ReturnUnknownValAsUnrecoverable = []() -> UnrecoverableError {
            std::unique_ptr<UnknownError> unknown =
                std::make_unique<UnknownError>(DAWN_VALIDATION_ERROR(kPlaceholderErrorMessage));
            return unknown;
        };
        UnrecoverableError err = ReturnUnknownValAsUnrecoverable();
        EXPECT_EQ(err.GetType(), InternalErrorType::Unrecoverable);
        EXPECT_EQ(err.GetMessage(), kPlaceholderErrorMessage);
    }

    // Validation -> Unrecoverable (converts type to Unrecoverable)
    {
        auto ReturnValAsUnrecoverable = []() -> UnrecoverableError {
            return DAWN_VALIDATION_ERROR(kPlaceholderErrorMessage);
        };
        UnrecoverableError err = ReturnValAsUnrecoverable();
        EXPECT_EQ(err.GetType(), InternalErrorType::Unrecoverable);
        EXPECT_EQ(err.GetMessage(), kPlaceholderErrorMessage);
    }

    // Validation -> Unknown (preserves Validation type)
    {
        auto ReturnValAsUnknown = []() -> UnknownError {
            return DAWN_VALIDATION_ERROR(kPlaceholderErrorMessage);
        };
        UnknownError err = ReturnValAsUnknown();
        EXPECT_EQ(err.GetType(), InternalErrorType::Validation);
        EXPECT_TRUE(err.IsVal());
        EXPECT_EQ(err.GetMessage(), kPlaceholderErrorMessage);
    }
}

template <typename SourceT, typename TargetT>
struct ResultFromTo {
    using Source = SourceT;
    using Target = TargetT;
};

template <typename T>
class ErrorParameterizedTests : public Test {
  public:
    using Source = typename T::Source;
    using Target = typename T::Target;

    Source GetPlaceholderSuccess() {
        if constexpr (std::is_void_v<Source>) {
            return;
        } else if constexpr (std::is_same_v<Source, int>) {
            return kPlaceholderSuccess;
        } else {
            return &mDerived;
        }
    }

    template <typename ResultType>
    ResultType ReturnSuccess() {
        if constexpr (std::is_void_v<Source>) {
            return {};
        } else {
            return GetPlaceholderSuccess();
        }
    }

    template <typename Error>
    void ExpectSuccess(Result<Target, Error> result) {
        ASSERT_TRUE(result.IsSuccess());
        if constexpr (std::is_void_v<Target>) {
            result.AcquireSuccess();
        } else {
            EXPECT_EQ(result.AcquireSuccess(), Target(GetPlaceholderSuccess()));
        }
    }

  private:
    DerivedRefCounted mDerived;
};

class ResultTestTypeNames {
  public:
    template <typename T>
    static std::string GetName(int) {
        if constexpr (std::is_same_v<T, ResultFromTo<void, void>>) {
            return "MaybeErrors";
        } else if constexpr (std::is_same_v<T, ResultFromTo<int, int>>) {
            return "FallthroughResults";
        } else if constexpr (std::is_same_v<T, ResultFromTo<BaseRefCounted*, BaseRefCounted*>>) {
            return "Base*->Base*";
        } else if constexpr (std::is_same_v<T, ResultFromTo<DerivedRefCounted*, BaseRefCounted*>>) {
            return "Derived*->Base*";
        } else if constexpr (std::is_same_v<
                                 T, ResultFromTo<const BaseRefCounted*, const BaseRefCounted*>>) {
            return "const Base*->const Base*";
        } else if constexpr (std::is_same_v<T, ResultFromTo<const DerivedRefCounted*,
                                                            const BaseRefCounted*>>) {
            return "const Derived*->const Base*";
        } else if constexpr (std::is_same_v<
                                 T, ResultFromTo<Ref<BaseRefCounted>, Ref<BaseRefCounted>>>) {
            return "Ref<Base>->Ref<Base>";
        } else if constexpr (std::is_same_v<
                                 T, ResultFromTo<Ref<DerivedRefCounted>, Ref<BaseRefCounted>>>) {
            return "Ref<Derived>->Ref<Base>";
        }
    }
};

using ResultTestTypes = Types<
    // ResultFromTo<void, void> is equivalent to Maybe*Error.
    ResultFromTo<void, void>,
    ResultFromTo<int, int>,
    ResultFromTo<BaseRefCounted*, BaseRefCounted*>,
    ResultFromTo<DerivedRefCounted*, BaseRefCounted*>,
    ResultFromTo<const BaseRefCounted*, const BaseRefCounted*>,
    ResultFromTo<const DerivedRefCounted*, const BaseRefCounted*>,
    ResultFromTo<Ref<BaseRefCounted>, Ref<BaseRefCounted>>,
    ResultFromTo<Ref<DerivedRefCounted>, Ref<BaseRefCounted>>>;
TYPED_TEST_SUITE(ErrorParameterizedTests, ResultTestTypes, ResultTestTypeNames);

// 1. Validation -> Unrecoverable (ResultOrValError<Source> -> ResultOrError<Target>)
TYPED_TEST(ErrorParameterizedTests, ValidationToUnrecoverable) {
    using Source = typename TypeParam::Source;
    using Target = typename TypeParam::Target;

    // Direct error conversion.
    {
        auto DirectError = []() -> ResultOrError<Target> {
            return DAWN_VALIDATION_ERROR(kPlaceholderErrorMessage);
        };
        ResultOrError<Target> res = DirectError();
        ASSERT_TRUE(res.IsError());
        std::unique_ptr<UnrecoverableError> err = res.AcquireError();
        EXPECT_EQ(err->GetType(), InternalErrorType::Unrecoverable);
        EXPECT_EQ(err->GetMessage(), kPlaceholderErrorMessage);
    }

    // Direct ResultOrError conversion.
    {
        auto DirectResultError = []() -> ResultOrError<Target> {
            return ResultOrValError<Source>(DAWN_VALIDATION_ERROR(kPlaceholderErrorMessage));
        };
        ResultOrError<Target> res = DirectResultError();
        ASSERT_TRUE(res.IsError());
        std::unique_ptr<UnrecoverableError> err = res.AcquireError();
        EXPECT_EQ(err->GetType(), InternalErrorType::Unrecoverable);
        EXPECT_EQ(err->GetMessage(), kPlaceholderErrorMessage);
    }

    // Direct success conversion.
    {
        auto DirectSuccess = [this]() -> ResultOrError<Target> {
            return this->template ReturnSuccess<ResultOrValError<Source>>();
        };
        this->ExpectSuccess(DirectSuccess());
    }
}

// 2. Validation -> Unknown (ResultOrValError<Source> -> ResultOrUnknownError<Target>)
TYPED_TEST(ErrorParameterizedTests, ValidationToUnknown) {
    using Source = typename TypeParam::Source;
    using Target = typename TypeParam::Target;

    // Direct error conversion.
    {
        auto DirectError = []() -> ResultOrUnknownError<Target> {
            return DAWN_VALIDATION_ERROR(kPlaceholderErrorMessage);
        };
        ResultOrUnknownError<Target> res = DirectError();
        ASSERT_TRUE(res.IsError());
        std::unique_ptr<UnknownError> err = res.AcquireError();
        EXPECT_EQ(err->GetType(), InternalErrorType::Validation);
        EXPECT_EQ(err->GetMessage(), kPlaceholderErrorMessage);
    }

    // Direct ResultOrError conversion.
    {
        auto DirectResultError = []() -> ResultOrUnknownError<Target> {
            return ResultOrValError<Source>(DAWN_VALIDATION_ERROR(kPlaceholderErrorMessage));
        };
        ResultOrUnknownError<Target> res = DirectResultError();
        ASSERT_TRUE(res.IsError());
        std::unique_ptr<UnknownError> err = res.AcquireError();
        EXPECT_EQ(err->GetType(), InternalErrorType::Validation);
        EXPECT_EQ(err->GetMessage(), kPlaceholderErrorMessage);
    }

    // Direct success conversion.
    {
        auto DirectSuccess = [this]() -> ResultOrUnknownError<Target> {
            return this->template ReturnSuccess<ResultOrValError<Source>>();
        };
        this->ExpectSuccess(DirectSuccess());
    }
}

// 3. Unrecoverable -> Unknown (ResultOrError<Source> -> ResultOrUnknownError<Target>)
TYPED_TEST(ErrorParameterizedTests, UnrecoverableToUnknown) {
    using Source = typename TypeParam::Source;
    using Target = typename TypeParam::Target;

    // Direct error conversion.
    {
        auto DirectError = []() -> ResultOrUnknownError<Target> {
            return DAWN_UNRECOVERABLE_ERROR(kPlaceholderErrorMessage);
        };
        ResultOrUnknownError<Target> res = DirectError();
        ASSERT_TRUE(res.IsError());
        std::unique_ptr<UnknownError> err = res.AcquireError();
        EXPECT_EQ(err->GetType(), InternalErrorType::Unrecoverable);
        EXPECT_EQ(err->GetMessage(), kPlaceholderErrorMessage);
    }

    // Direct ResultOrError conversion.
    {
        auto DirectResultError = []() -> ResultOrUnknownError<Target> {
            return ResultOrError<Source>(DAWN_UNRECOVERABLE_ERROR(kPlaceholderErrorMessage));
        };
        ResultOrUnknownError<Target> res = DirectResultError();
        ASSERT_TRUE(res.IsError());
        std::unique_ptr<UnknownError> err = res.AcquireError();
        EXPECT_EQ(err->GetType(), InternalErrorType::Unrecoverable);
        EXPECT_EQ(err->GetMessage(), kPlaceholderErrorMessage);
    }

    // Direct success conversion.
    {
        auto DirectSuccess = [this]() -> ResultOrUnknownError<Target> {
            return this->template ReturnSuccess<ResultOrError<Source>>();
        };
        this->ExpectSuccess(DirectSuccess());
    }
}

// 4. Unknown -> Unrecoverable (ResultOrUnknownError<Source> -> ResultOrError<Target>)
TYPED_TEST(ErrorParameterizedTests, UnknownToUnrecoverable) {
    using Source = typename TypeParam::Source;
    using Target = typename TypeParam::Target;

    // Direct error conversion.
    {
        auto DirectError = []() -> ResultOrError<Target> {
            return std::make_unique<UnknownError>(
                DAWN_UNRECOVERABLE_ERROR(kPlaceholderErrorMessage));
        };
        ResultOrError<Target> res = DirectError();
        ASSERT_TRUE(res.IsError());
        std::unique_ptr<UnrecoverableError> err = res.AcquireError();
        EXPECT_EQ(err->GetType(), InternalErrorType::Unrecoverable);
        EXPECT_EQ(err->GetMessage(), kPlaceholderErrorMessage);
    }

    // Direct ResultOrError conversion.
    {
        auto DirectResultError = []() -> ResultOrError<Target> {
            return ResultOrUnknownError<Source>(DAWN_UNRECOVERABLE_ERROR(kPlaceholderErrorMessage));
        };
        ResultOrError<Target> res = DirectResultError();
        ASSERT_TRUE(res.IsError());
        std::unique_ptr<UnrecoverableError> err = res.AcquireError();
        EXPECT_EQ(err->GetType(), InternalErrorType::Unrecoverable);
        EXPECT_EQ(err->GetMessage(), kPlaceholderErrorMessage);
    }

    // Direct success conversion.
    {
        auto DirectSuccess = [this]() -> ResultOrError<Target> {
            return this->template ReturnSuccess<ResultOrUnknownError<Source>>();
        };
        this->ExpectSuccess(DirectSuccess());
    }
}

}  // namespace
}  // namespace dawn::native
