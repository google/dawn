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

#include <gtest/gtest.h>

#include "backend/Error.h"
#include "backend/ErrorData.h"

using namespace backend;

namespace {

int dummySuccess = 0xbeef;
const char* dummyErrorMessage = "I am an error message :3";

// Check returning a success MaybeError with {};
TEST(ErrorTests, Error_Success) {
    auto ReturnSuccess = []() -> MaybeError {
        return {};
    };

    MaybeError result = ReturnSuccess();
    ASSERT_TRUE(result.IsSuccess());
}

// Check returning an error MaybeError with NXT_RETURN_ERROR
TEST(ErrorTests, Error_Error) {
    auto ReturnError = []() -> MaybeError {
        NXT_RETURN_ERROR(dummyErrorMessage);
    };

    MaybeError result = ReturnError();
    ASSERT_TRUE(result.IsError());

    ErrorData* errorData = result.AcquireError();
    ASSERT_EQ(errorData->GetMessage(), dummyErrorMessage);
    delete errorData;
}

// Check returning a success ResultOrError with an implicit conversion
TEST(ErrorTests, ResultOrError_Success) {
    auto ReturnSuccess = []() -> ResultOrError<int*> {
        return &dummySuccess;
    };

    ResultOrError<int*> result = ReturnSuccess();
    ASSERT_TRUE(result.IsSuccess());
    ASSERT_EQ(result.AcquireSuccess(), &dummySuccess);
}

// Check returning an error ResultOrError with NXT_RETURN_ERROR
TEST(ErrorTests, ResultOrError_Error) {
    auto ReturnError = []() -> ResultOrError<int*> {
        NXT_RETURN_ERROR(dummyErrorMessage);
    };

    ResultOrError<int*> result = ReturnError();
    ASSERT_TRUE(result.IsError());

    ErrorData* errorData = result.AcquireError();
    ASSERT_EQ(errorData->GetMessage(), dummyErrorMessage);
    delete errorData;
}

// Check NXT_TRY handles successes correctly.
TEST(ErrorTests, TRY_Success) {
    auto ReturnSuccess = []() -> MaybeError {
        return {};
    };

    // We need to check that NXT_TRY doesn't return on successes
    bool tryReturned = true;

    auto Try = [ReturnSuccess, &tryReturned]() -> MaybeError {
        NXT_TRY(ReturnSuccess());
        tryReturned = false;
        return {};
    };

    MaybeError result = Try();
    ASSERT_TRUE(result.IsSuccess());
    ASSERT_FALSE(tryReturned);
}

// Check NXT_TRY handles errors correctly.
TEST(ErrorTests, TRY_Error) {
    auto ReturnError = []() -> MaybeError {
        NXT_RETURN_ERROR(dummyErrorMessage);
    };

    auto Try = [ReturnError]() -> MaybeError {
        NXT_TRY(ReturnError());
        // NXT_TRY should return before this point
        EXPECT_FALSE(true);
        return {};
    };

    MaybeError result = Try();
    ASSERT_TRUE(result.IsError());

    ErrorData* errorData = result.AcquireError();
    ASSERT_EQ(errorData->GetMessage(), dummyErrorMessage);
    delete errorData;
}

// Check NXT_TRY adds to the backtrace.
TEST(ErrorTests, TRY_AddsToBacktrace) {
    auto ReturnError = []() -> MaybeError {
        NXT_RETURN_ERROR(dummyErrorMessage);
    };

    auto SingleTry = [ReturnError]() -> MaybeError {
        NXT_TRY(ReturnError());
        return {};
    };

    auto DoubleTry = [SingleTry]() -> MaybeError {
        NXT_TRY(SingleTry());
        return {};
    };

    MaybeError singleResult = SingleTry();
    ASSERT_TRUE(singleResult.IsError());

    MaybeError doubleResult = DoubleTry();
    ASSERT_TRUE(doubleResult.IsError());

    ErrorData* singleData = singleResult.AcquireError();
    ErrorData* doubleData = doubleResult.AcquireError();

    ASSERT_EQ(singleData->GetBacktrace().size() + 1, doubleData->GetBacktrace().size());

    delete singleData;
    delete doubleData;
}

// Check NXT_TRY_ASSIGN handles successes correctly.
TEST(ErrorTests, TRY_RESULT_Success) {
    auto ReturnSuccess = []() -> ResultOrError<int*> {
        return &dummySuccess;
    };

    // We need to check that NXT_TRY doesn't return on successes
    bool tryReturned = true;

    auto Try = [ReturnSuccess, &tryReturned]() -> ResultOrError<int*> {
        int* result = nullptr;
        NXT_TRY_ASSIGN(result, ReturnSuccess());
        tryReturned = false;

        EXPECT_EQ(result, &dummySuccess);
        return result;
    };

    ResultOrError<int*> result = Try();
    ASSERT_TRUE(result.IsSuccess());
    ASSERT_FALSE(tryReturned);
    ASSERT_EQ(result.AcquireSuccess(), &dummySuccess);
}

// Check NXT_TRY_ASSIGN handles errors correctly.
TEST(ErrorTests, TRY_RESULT_Error) {
    auto ReturnError = []() -> ResultOrError<int*> {
        NXT_RETURN_ERROR(dummyErrorMessage);
    };

    auto Try = [ReturnError]() -> ResultOrError<int*> {
        int* result = nullptr;
        NXT_TRY_ASSIGN(result, ReturnError());
        (void) result;

        // NXT_TRY should return before this point
        EXPECT_FALSE(true);
        return &dummySuccess;
    };

    ResultOrError<int*> result = Try();
    ASSERT_TRUE(result.IsError());

    ErrorData* errorData = result.AcquireError();
    ASSERT_EQ(errorData->GetMessage(), dummyErrorMessage);
    delete errorData;
}

// Check NXT_TRY_ASSIGN adds to the backtrace.
TEST(ErrorTests, TRY_RESULT_AddsToBacktrace) {
    auto ReturnError = []() -> ResultOrError<int*> {
        NXT_RETURN_ERROR(dummyErrorMessage);
    };

    auto SingleTry = [ReturnError]() -> ResultOrError<int*> {
        NXT_TRY(ReturnError());
        return &dummySuccess;
    };

    auto DoubleTry = [SingleTry]() -> ResultOrError<int*> {
        NXT_TRY(SingleTry());
        return &dummySuccess;
    };

    ResultOrError<int*> singleResult = SingleTry();
    ASSERT_TRUE(singleResult.IsError());

    ResultOrError<int*> doubleResult = DoubleTry();
    ASSERT_TRUE(doubleResult.IsError());

    ErrorData* singleData = singleResult.AcquireError();
    ErrorData* doubleData = doubleResult.AcquireError();

    ASSERT_EQ(singleData->GetBacktrace().size() + 1, doubleData->GetBacktrace().size());

    delete singleData;
    delete doubleData;
}

// Check a ResultOrError can be NXT_TRY_ASSIGNED in a function that returns an Error
TEST(ErrorTests, TRY_RESULT_ConversionToError) {
    auto ReturnError = []() -> ResultOrError<int*> {
        NXT_RETURN_ERROR(dummyErrorMessage);
    };

    auto Try = [ReturnError]() -> MaybeError {
        int* result = nullptr;
        NXT_TRY_ASSIGN(result, ReturnError());
        (void) result;

        return {};
    };

    MaybeError result = Try();
    ASSERT_TRUE(result.IsError());

    ErrorData* errorData = result.AcquireError();
    ASSERT_EQ(errorData->GetMessage(), dummyErrorMessage);
    delete errorData;
}

// Check a MaybeError can be NXT_TRIED in a function that returns an ResultOrError
// Check NXT_TRY handles errors correctly.
TEST(ErrorTests, TRY_ConversionToErrorOrResult) {
    auto ReturnError = []() -> MaybeError {
        NXT_RETURN_ERROR(dummyErrorMessage);
    };

    auto Try = [ReturnError]() -> ResultOrError<int*>{
        NXT_TRY(ReturnError());
        return &dummySuccess;
    };

    ResultOrError<int*> result = Try();
    ASSERT_TRUE(result.IsError());

    ErrorData* errorData = result.AcquireError();
    ASSERT_EQ(errorData->GetMessage(), dummyErrorMessage);
    delete errorData;
}

}  // anonymous namespace
