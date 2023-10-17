// Copyright 2021 The Dawn & Tint Authors
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
#include <utility>

#include "dawn/common/ConcurrentCache.h"
#include "dawn/native/AsyncTask.h"
#include "dawn/platform/DawnPlatform.h"
#include "dawn/utils/SystemUtils.h"
#include "gtest/gtest.h"

namespace dawn {
namespace {

class SimpleCachedObject {
  public:
    explicit SimpleCachedObject(size_t value) : mValue(value) {}

    size_t GetValue() const { return mValue; }

    struct EqualityFunc {
        bool operator()(const SimpleCachedObject* a, const SimpleCachedObject* b) const {
            return a->mValue == b->mValue;
        }
    };

    struct HashFunc {
        size_t operator()(const SimpleCachedObject* obj) const { return obj->mValue; }
    };

  private:
    size_t mValue;
};

class ConcurrentCacheTest : public testing::Test {
  public:
    ConcurrentCacheTest() : mPool(mPlatform.CreateWorkerTaskPool()), mTaskManager(mPool.get()) {}

  protected:
    platform::Platform mPlatform;
    std::unique_ptr<platform::WorkerTaskPool> mPool;
    native::AsyncTaskManager mTaskManager;
    ConcurrentCache<SimpleCachedObject> mCache;
};

// Test inserting two objects that are equal to each other into the concurrent cache works as
// expected.
TEST_F(ConcurrentCacheTest, InsertAtSameTime) {
    SimpleCachedObject cachedObject(1);
    SimpleCachedObject anotherCachedObject(1);

    std::pair<SimpleCachedObject*, bool> insertOutput = {};
    std::pair<SimpleCachedObject*, bool> anotherInsertOutput = {};

    ConcurrentCache<SimpleCachedObject>* cachePtr = &mCache;
    native::AsyncTask asyncTask1([&insertOutput, cachePtr, &cachedObject] {
        insertOutput = cachePtr->Insert(&cachedObject);
    });
    native::AsyncTask asyncTask2([&anotherInsertOutput, cachePtr, &anotherCachedObject] {
        anotherInsertOutput = cachePtr->Insert(&anotherCachedObject);
    });
    mTaskManager.PostTask(std::move(asyncTask1));
    mTaskManager.PostTask(std::move(asyncTask2));

    mTaskManager.WaitAllPendingTasks();

    ASSERT_TRUE(insertOutput.first == &cachedObject || insertOutput.first == &anotherCachedObject);
    ASSERT_EQ(insertOutput.first, anotherInsertOutput.first);
    ASSERT_EQ(insertOutput.second, !anotherInsertOutput.second);
}

// Testing erasing an object after inserting into the cache works as expected.
TEST_F(ConcurrentCacheTest, EraseAfterInsertion) {
    SimpleCachedObject cachedObject(1);

    std::pair<SimpleCachedObject*, bool> insertOutput = {};
    ConcurrentCache<SimpleCachedObject>* cachePtr = &mCache;
    native::AsyncTask insertTask([&insertOutput, cachePtr, &cachedObject] {
        insertOutput = cachePtr->Insert(&cachedObject);
    });

    size_t erasedObjectCount = 0;
    native::AsyncTask eraseTask([&erasedObjectCount, cachePtr, &cachedObject] {
        while (cachePtr->Find(&cachedObject) == nullptr) {
            utils::USleep(100);
        }
        erasedObjectCount = cachePtr->Erase(&cachedObject);
    });

    mTaskManager.PostTask(std::move(insertTask));
    mTaskManager.PostTask(std::move(eraseTask));

    mTaskManager.WaitAllPendingTasks();

    ASSERT_EQ(&cachedObject, insertOutput.first);
    ASSERT_TRUE(insertOutput.second);
    ASSERT_EQ(1u, erasedObjectCount);
}

}  // anonymous namespace
}  // namespace dawn
