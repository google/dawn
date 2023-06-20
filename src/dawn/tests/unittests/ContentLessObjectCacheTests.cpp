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
#include "gtest/gtest.h"

namespace dawn {
namespace {

using ::testing::Test;
using ::testing::Types;

class BlueprintT {
  public:
    explicit BlueprintT(size_t value) : mValue(value) {}

    size_t GetValue() const { return mValue; }

    struct HashFunc {
        size_t operator()(const BlueprintT* x) const { return x->mValue; }
    };

    struct EqualityFunc {
        bool operator()(const BlueprintT* l, const BlueprintT* r) const {
            return l->mValue == r->mValue;
        }
    };

  protected:
    size_t mValue;
};

class RefCountedT : public BlueprintT, public RefCounted {
  public:
    explicit RefCountedT(size_t value) : BlueprintT(value) {}
    RefCountedT(size_t value, std::function<void(RefCountedT*)> deleteFn)
        : BlueprintT(value), mDeleteFn(deleteFn) {}

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
    std::function<void(RefCountedT*)> mDeleteFn = [](RefCountedT*) -> void {};
};

template <typename Blueprint>
class ContentLessObjectCacheTest : public Test {};

class BlueprintTypeNames {
  public:
    template <typename T>
    static std::string GetName(int) {
        if (std::is_same<T, RefCountedT>()) {
            return "RefCountedT";
        }
        if (std::is_same<T, BlueprintT>()) {
            return "BlueprintT";
        }
    }
};
using BlueprintTypes = Types<RefCountedT, BlueprintT>;
TYPED_TEST_SUITE(ContentLessObjectCacheTest, BlueprintTypes, BlueprintTypeNames);

// Empty cache returns true on Empty().
TYPED_TEST(ContentLessObjectCacheTest, Empty) {
    ContentLessObjectCache<RefCountedT, TypeParam> cache;
    EXPECT_TRUE(cache.Empty());
}

// Non-empty cache returns false on Empty().
TYPED_TEST(ContentLessObjectCacheTest, NonEmpty) {
    ContentLessObjectCache<RefCountedT, TypeParam> cache;
    Ref<RefCountedT> object =
        AcquireRef(new RefCountedT(1, [&](RefCountedT* x) { cache.Erase(x); }));
    EXPECT_TRUE(cache.Insert(object.Get()).second);
    EXPECT_FALSE(cache.Empty());
}

// Object inserted into the cache are findable.
TYPED_TEST(ContentLessObjectCacheTest, Insert) {
    ContentLessObjectCache<RefCountedT, TypeParam> cache;
    Ref<RefCountedT> object =
        AcquireRef(new RefCountedT(1, [&](RefCountedT* x) { cache.Erase(x); }));
    EXPECT_TRUE(cache.Insert(object.Get()).second);

    TypeParam blueprint(1);
    Ref<RefCountedT> cached = cache.Find(&blueprint);
    EXPECT_TRUE(object.Get() == cached.Get());
}

// Duplicate insert calls on different objects with the same hash only inserts the first.
TYPED_TEST(ContentLessObjectCacheTest, InsertDuplicate) {
    ContentLessObjectCache<RefCountedT, TypeParam> cache;
    Ref<RefCountedT> object1 =
        AcquireRef(new RefCountedT(1, [&](RefCountedT* x) { cache.Erase(x); }));
    EXPECT_TRUE(cache.Insert(object1.Get()).second);

    Ref<RefCountedT> object2 = AcquireRef(new RefCountedT(1));
    EXPECT_FALSE(cache.Insert(object2.Get()).second);

    TypeParam blueprint(1);
    Ref<RefCountedT> cached = cache.Find(&blueprint);
    EXPECT_TRUE(object1.Get() == cached.Get());
}

// Erasing the only entry leaves the cache empty.
TYPED_TEST(ContentLessObjectCacheTest, Erase) {
    ContentLessObjectCache<RefCountedT, TypeParam> cache;
    Ref<RefCountedT> object = AcquireRef(new RefCountedT(1));
    EXPECT_TRUE(cache.Insert(object.Get()).second);
    EXPECT_FALSE(cache.Empty());

    cache.Erase(object.Get());
    EXPECT_TRUE(cache.Empty());
}

// Erasing a hash equivalent but not pointer equivalent entry is a no-op.
TYPED_TEST(ContentLessObjectCacheTest, EraseDuplicate) {
    ContentLessObjectCache<RefCountedT, TypeParam> cache;
    Ref<RefCountedT> object1 =
        AcquireRef(new RefCountedT(1, [&](RefCountedT* x) { cache.Erase(x); }));
    EXPECT_TRUE(cache.Insert(object1.Get()).second);
    EXPECT_FALSE(cache.Empty());

    Ref<RefCountedT> object2 = AcquireRef(new RefCountedT(1));
    cache.Erase(object2.Get());
    EXPECT_FALSE(cache.Empty());
}

// Helper struct that basically acts as a semaphore to allow for flow control in multiple threads.
struct Signal {
    std::mutex mutex;
    std::condition_variable cv;
    bool signaled = false;

    void Fire() {
        std::lock_guard<std::mutex> lock(mutex);
        signaled = true;
        cv.notify_one();
    }
    void Wait() {
        std::unique_lock<std::mutex> lock(mutex);
        while (!signaled) {
            cv.wait(lock);
        }
        signaled = false;
    }
};

// Inserting and finding elements should respect the results from the insert call.
TYPED_TEST(ContentLessObjectCacheTest, InsertingAndFinding) {
    constexpr size_t kNumObjects = 100;
    constexpr size_t kNumThreads = 8;
    ContentLessObjectCache<RefCountedT, TypeParam> cache;
    std::vector<Ref<RefCountedT>> objects(kNumObjects);

    auto f = [&]() {
        for (size_t i = 0; i < kNumObjects; i++) {
            Ref<RefCountedT> object =
                AcquireRef(new RefCountedT(i, [&](RefCountedT* x) { cache.Erase(x); }));
            if (cache.Insert(object.Get()).second) {
                // This shouldn't race because exactly 1 thread should successfully insert.
                objects[i] = object;
            }
        }
        for (size_t i = 0; i < kNumObjects; i++) {
            TypeParam blueprint(i);
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
TYPED_TEST(ContentLessObjectCacheTest, FindDeleting) {
    Signal signalA, signalB;

    ContentLessObjectCache<RefCountedT, TypeParam> cache;
    Ref<RefCountedT> object = AcquireRef(new RefCountedT(1, [&](RefCountedT* x) {
        signalA.Fire();
        signalB.Wait();
        cache.Erase(x);
    }));
    EXPECT_TRUE(cache.Insert(object.Get()).second);

    // Thread A will release the last reference of the original object.
    auto threadA = [&]() { object = nullptr; };
    // Thread B will try to Find the entry before it is completely destroyed.
    auto threadB = [&]() {
        signalA.Wait();
        TypeParam blueprint(1);
        EXPECT_TRUE(cache.Find(&blueprint) == nullptr);
        signalB.Fire();
    };

    std::thread tA(threadA);
    std::thread tB(threadB);
    tA.join();
    tB.join();
}

// Inserting an element that has an entry which is in process of deletion should insert the new
// object.
TYPED_TEST(ContentLessObjectCacheTest, InsertDeleting) {
    Signal signalA, signalB;

    ContentLessObjectCache<RefCountedT, TypeParam> cache;
    Ref<RefCountedT> object1 = AcquireRef(new RefCountedT(1, [&](RefCountedT* x) {
        signalA.Fire();
        signalB.Wait();
        cache.Erase(x);
    }));
    EXPECT_TRUE(cache.Insert(object1.Get()).second);

    Ref<RefCountedT> object2 =
        AcquireRef(new RefCountedT(1, [&](RefCountedT* x) { cache.Erase(x); }));

    // Thread A will release the last reference of the original object.
    auto threadA = [&]() { object1 = nullptr; };
    // Thread B will try to Insert a hash equivalent entry before the original is completely
    // destroyed.
    auto threadB = [&]() {
        signalA.Wait();
        EXPECT_TRUE(cache.Insert(object2.Get()).second);
        signalB.Fire();
    };

    std::thread tA(threadA);
    std::thread tB(threadB);
    tA.join();
    tB.join();

    TypeParam blueprint(1);
    Ref<RefCountedT> cached = cache.Find(&blueprint);
    EXPECT_TRUE(object2.Get() == cached.Get());
}

}  // anonymous namespace
}  // namespace dawn
