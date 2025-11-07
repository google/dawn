// Copyright 2025 The Dawn & Tint Authors
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

#ifndef SRC_DAWN_NATIVE_FUTURERESULT_H_
#define SRC_DAWN_NATIVE_FUTURERESULT_H_

#include <atomic>
#include <memory>
#include <mutex>
#include <utility>

#include "dawn/common/RefCounted.h"
#include "dawn/native/Error.h"

namespace dawn::native {

// FutureResult is a thread-safe class that holds an asynchronous ResultOrError<T>.
// Unlike ResultOrError, this class allows multiple calls to TryGet() without moving the result
// (unless it's an error, in which case the error is moved on the first call and subsequent calls
// return a placeholder error). Threads calling TryGet() will block until the result is available
// via a call to Set().
//
// Example usage:
//
// Ref<FutureResult<int>> future = AcquireRef(new FutureResult<int>());
//
// // Producer thread
// std::thread producer_thread([&]() {
//     // Do some asynchronous work
//     future->Set(42);
// });
//
// // Consumer thread
// int result;
// DAWN_TRY_ASSIGN(result, future->TryGet());
template <typename T>
class FutureResult : public RefCounted {
  public:
    FutureResult() = default;

    ResultOrError<T> TryGet() {
        mIsSet.wait(false, std::memory_order_acquire);

        if (DAWN_LIKELY(!mIsError)) {
            return mValue;
        }

        std::lock_guard<std::mutex> lock(mMutex);
        if (mError) {
            return std::move(mError);
        }
        // ErrorData cannot be copied, so after the first call to TryGet() that acquires the error,
        // mError will be null. To ensure subsequent calls to TryGet() still return a valid
        // ResultOrError, we return a placeholder error.
        return DAWN_INTERNAL_ERROR("Error was already moved.");
    }

    void Set(ResultOrError<T> result) {
        if (result.IsError()) {
            mError = result.AcquireError();
            mIsError = true;
        } else {
            mValue = result.AcquireSuccess();
        }
        [[maybe_unused]] bool wasSet = mIsSet.exchange(true, std::memory_order_acq_rel);
        // We allow Set only once.
        DAWN_ASSERT(!wasSet);
        mIsSet.notify_all();
    }

  private:
    std::atomic_bool mIsSet{false};
    std::mutex mMutex;
    std::unique_ptr<ErrorData> mError;
    T mValue;
    bool mIsError = false;
};

}  // namespace dawn::native

#endif  // SRC_DAWN_NATIVE_FUTURERESULT_H_
