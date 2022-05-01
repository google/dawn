// Copyright 2021 The Dawn Authors
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

#include <memory>
#include <utility>

#include "dawn/common/ConcurrentCache.h"
#include "dawn/native/AsyncTask.h"
#include "dawn/platform/DawnPlatform.h"
#include "dawn/utils/SystemUtils.h"
#include "gtest/gtest.h"

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

}  // anonymous namespace

class ConcurrentCacheTest : public testing::Test {
  public:
    ConcurrentCacheTest() : mPool(mPlatform.CreateWorkerTaskPool()), mTaskManager(mPool.get()) {}

  protected:
    dawn::platform::Platform mPlatform;
    std::unique_ptr<dawn::platform::WorkerTaskPool> mPool;
    dawn::native::AsyncTaskManager mTaskManager;
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
    dawn::native::AsyncTask asyncTask1([&insertOutput, cachePtr, &cachedObject] {
        insertOutput = cachePtr->Insert(&cachedObject);
    });
    dawn::native::AsyncTask asyncTask2([&anotherInsertOutput, cachePtr, &anotherCachedObject] {
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
    dawn::native::AsyncTask insertTask([&insertOutput, cachePtr, &cachedObject] {
        insertOutput = cachePtr->Insert(&cachedObject);
    });

    size_t erasedObjectCount = 0;
    dawn::native::AsyncTask eraseTask([&erasedObjectCount, cachePtr, &cachedObject] {
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
