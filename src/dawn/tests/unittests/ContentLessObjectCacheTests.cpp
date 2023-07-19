// Copyright 2023 The Dawn Authors
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

#include <condition_variable>
#include <functional>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

#include "dawn/common/ContentLessObjectCache.h"
#include "dawn/utils/BinarySemaphore.h"
#include "gtest/gtest.h"

namespace dawn {
namespace {

using utils::BinarySemaphore;

class RefCountedT : public RefCounted {
  public:
    explicit RefCountedT(size_t value) : mValue(value) {}
    RefCountedT(size_t value, std::function<void(RefCountedT*)> deleteFn)
        : mValue(value), mDeleteFn(deleteFn) {}

    ~RefCountedT() override { mDeleteFn(this); }

    struct HashFunc {
        size_t operator()(const RefCountedT* x) const { return x->mValue; }
    };

    struct EqualityFunc {
        bool operator()(const RefCountedT* l, const RefCountedT* r) const {
            return l->mValue == r->mValue;
        }
    };

  private:
    size_t mValue;
    std::function<void(RefCountedT*)> mDeleteFn = [](RefCountedT*) -> void {};
};

// Empty cache returns true on Empty().
TEST(ContentLessObjectCacheTest, Empty) {
    ContentLessObjectCache<RefCountedT> cache;
    EXPECT_TRUE(cache.Empty());
}

// Non-empty cache returns false on Empty().
TEST(ContentLessObjectCacheTest, NonEmpty) {
    ContentLessObjectCache<RefCountedT> cache;
    Ref<RefCountedT> object =
        AcquireRef(new RefCountedT(1, [&](RefCountedT* x) { cache.Erase(x); }));
    EXPECT_TRUE(cache.Insert(object.Get()).second);
    EXPECT_FALSE(cache.Empty());
}

// Object inserted into the cache are findable.
TEST(ContentLessObjectCacheTest, Insert) {
    ContentLessObjectCache<RefCountedT> cache;
    Ref<RefCountedT> object =
        AcquireRef(new RefCountedT(1, [&](RefCountedT* x) { cache.Erase(x); }));
    EXPECT_TRUE(cache.Insert(object.Get()).second);

    RefCountedT blueprint(1);
    Ref<RefCountedT> cached = cache.Find(&blueprint);
    EXPECT_TRUE(object.Get() == cached.Get());
}

// Duplicate insert calls on different objects with the same hash only inserts the first.
TEST(ContentLessObjectCacheTest, InsertDuplicate) {
    ContentLessObjectCache<RefCountedT> cache;
    Ref<RefCountedT> object1 =
        AcquireRef(new RefCountedT(1, [&](RefCountedT* x) { cache.Erase(x); }));
    EXPECT_TRUE(cache.Insert(object1.Get()).second);

    Ref<RefCountedT> object2 = AcquireRef(new RefCountedT(1));
    EXPECT_FALSE(cache.Insert(object2.Get()).second);

    RefCountedT blueprint(1);
    Ref<RefCountedT> cached = cache.Find(&blueprint);
    EXPECT_TRUE(object1.Get() == cached.Get());
}

// Erasing the only entry leaves the cache empty.
TEST(ContentLessObjectCacheTest, Erase) {
    ContentLessObjectCache<RefCountedT> cache;
    Ref<RefCountedT> object = AcquireRef(new RefCountedT(1));
    EXPECT_TRUE(cache.Insert(object.Get()).second);
    EXPECT_FALSE(cache.Empty());

    cache.Erase(object.Get());
    EXPECT_TRUE(cache.Empty());
}

// Erasing a hash equivalent but not pointer equivalent entry is a no-op.
TEST(ContentLessObjectCacheTest, EraseDuplicate) {
    ContentLessObjectCache<RefCountedT> cache;
    Ref<RefCountedT> object1 =
        AcquireRef(new RefCountedT(1, [&](RefCountedT* x) { cache.Erase(x); }));
    EXPECT_TRUE(cache.Insert(object1.Get()).second);
    EXPECT_FALSE(cache.Empty());

    Ref<RefCountedT> object2 = AcquireRef(new RefCountedT(1));
    cache.Erase(object2.Get());
    EXPECT_FALSE(cache.Empty());
}

// Inserting and finding elements should respect the results from the insert call.
TEST(ContentLessObjectCacheTest, InsertingAndFinding) {
    constexpr size_t kNumObjects = 100;
    constexpr size_t kNumThreads = 8;
    ContentLessObjectCache<RefCountedT> cache;
    std::vector<Ref<RefCountedT>> objects(kNumObjects);

    auto f = [&] {
        for (size_t i = 0; i < kNumObjects; i++) {
            Ref<RefCountedT> object =
                AcquireRef(new RefCountedT(i, [&](RefCountedT* x) { cache.Erase(x); }));
            if (cache.Insert(object.Get()).second) {
                // This shouldn't race because exactly 1 thread should successfully insert.
                objects[i] = object;
            }
        }
        for (size_t i = 0; i < kNumObjects; i++) {
            RefCountedT blueprint(i);
            Ref<RefCountedT> cached = cache.Find(&blueprint);
            EXPECT_NE(cached.Get(), nullptr);
            EXPECT_EQ(cached.Get(), objects[i].Get());
        }
    };

    std::vector<std::thread> threads;
    for (size_t t = 0; t < kNumThreads; t++) {
        threads.emplace_back(f);
    }
    for (size_t t = 0; t < kNumThreads; t++) {
        threads[t].join();
    }
}

// Finding an element that is in the process of deletion should return nullptr.
TEST(ContentLessObjectCacheTest, FindDeleting) {
    BinarySemaphore semA, semB;

    ContentLessObjectCache<RefCountedT> cache;
    Ref<RefCountedT> object = AcquireRef(new RefCountedT(1, [&](RefCountedT* x) {
        semA.Release();
        semB.Acquire();
        cache.Erase(x);
    }));
    EXPECT_TRUE(cache.Insert(object.Get()).second);

    // Thread A will release the last reference of the original object.
    auto threadA = [&] { object = nullptr; };
    // Thread B will try to Find the entry before it is completely destroyed.
    auto threadB = [&] {
        semA.Acquire();
        RefCountedT blueprint(1);
        EXPECT_TRUE(cache.Find(&blueprint) == nullptr);
        semB.Release();
    };

    std::thread tA(threadA);
    std::thread tB(threadB);
    tA.join();
    tB.join();
}

// Inserting an element that has an entry which is in process of deletion should insert the new
// object.
TEST(ContentLessObjectCacheTest, InsertDeleting) {
    BinarySemaphore semA, semB;

    ContentLessObjectCache<RefCountedT> cache;
    Ref<RefCountedT> object1 = AcquireRef(new RefCountedT(1, [&](RefCountedT* x) {
        semA.Release();
        semB.Acquire();
        cache.Erase(x);
    }));
    EXPECT_TRUE(cache.Insert(object1.Get()).second);

    Ref<RefCountedT> object2 =
        AcquireRef(new RefCountedT(1, [&](RefCountedT* x) { cache.Erase(x); }));

    // Thread A will release the last reference of the original object.
    auto threadA = [&] { object1 = nullptr; };
    // Thread B will try to Insert a hash equivalent entry before the original is completely
    // destroyed.
    auto threadB = [&] {
        semA.Acquire();
        EXPECT_TRUE(cache.Insert(object2.Get()).second);
        semB.Release();
    };

    std::thread tA(threadA);
    std::thread tB(threadB);
    tA.join();
    tB.join();

    RefCountedT blueprint(1);
    Ref<RefCountedT> cached = cache.Find(&blueprint);
    EXPECT_TRUE(object2.Get() == cached.Get());
}

}  // anonymous namespace
}  // namespace dawn
