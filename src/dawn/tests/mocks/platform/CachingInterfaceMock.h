// Copyright 2022 The Dawn Authors
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

#ifndef SRC_DAWN_TESTS_MOCKS_PLATFORM_CACHINGINTERFACEMOCK_H_
#define SRC_DAWN_TESTS_MOCKS_PLATFORM_CACHINGINTERFACEMOCK_H_

#include <dawn/platform/DawnPlatform.h>
#include <gmock/gmock.h>

#include <string>
#include <unordered_map>
#include <vector>

#include "dawn/common/TypedInteger.h"

#define EXPECT_CACHE_HIT(cache, N, statement) \
    do {                                      \
        FlushWire();                          \
        size_t before = cache.GetHitCount();  \
        statement;                            \
        FlushWire();                          \
        size_t after = cache.GetHitCount();   \
        EXPECT_EQ(N, after - before);         \
    } while (0)

// Check that |HitN| cache hits occurred, and |AddN| entries were added.
// Usage: EXPECT_CACHE_STATS(myMockCache, Hit(42), Add(3), ...)
// Hit / Add help readability, and enforce the args are passed correctly in the expected order.
#define EXPECT_CACHE_STATS(cache, HitN, AddN, statement)                    \
    do {                                                                    \
        using Hit = dawn::TypedInteger<struct HitT, size_t>;                \
        using Add = dawn::TypedInteger<struct AddT, size_t>;                \
        static_assert(std::is_same_v<decltype(HitN), Hit>);                 \
        static_assert(std::is_same_v<decltype(AddN), Add>);                 \
        FlushWire();                                                        \
        size_t hitBefore = cache.GetHitCount();                             \
        size_t entriesBefore = cache.GetNumEntries();                       \
        statement;                                                          \
        FlushWire();                                                        \
        size_t hitAfter = cache.GetHitCount();                              \
        size_t entriesAfter = cache.GetNumEntries();                        \
        EXPECT_EQ(static_cast<size_t>(HitN), hitAfter - hitBefore);         \
        EXPECT_EQ(static_cast<size_t>(AddN), entriesAfter - entriesBefore); \
    } while (0)

// A mock caching interface class that also supplies an in memory cache for testing.
class CachingInterfaceMock : public dawn::platform::CachingInterface {
  public:
    CachingInterfaceMock();

    // Toggles to disable/enable caching.
    void Disable();
    void Enable();

    // Returns the number of cache hits up to this point.
    size_t GetHitCount() const;

    // Returns the number of entries in the cache.
    size_t GetNumEntries() const;

    MOCK_METHOD(size_t, LoadData, (const void*, size_t, void*, size_t), (override));
    MOCK_METHOD(void, StoreData, (const void*, size_t, const void*, size_t), (override));

  private:
    size_t LoadDataDefault(const void* key, size_t keySize, void* value, size_t valueSize);
    void StoreDataDefault(const void* key, size_t keySize, const void* value, size_t valueSize);

    bool mEnabled = true;
    size_t mHitCount = 0;
    std::unordered_map<std::string, std::vector<uint8_t>> mCache;
};

// Dawn platform used for testing with a mock caching interface.
class DawnCachingMockPlatform : public dawn::platform::Platform {
  public:
    explicit DawnCachingMockPlatform(dawn::platform::CachingInterface* cachingInterface);

    dawn::platform::CachingInterface* GetCachingInterface() override;

  private:
    dawn::platform::CachingInterface* mCachingInterface = nullptr;
};

#endif  // SRC_DAWN_TESTS_MOCKS_PLATFORM_CACHINGINTERFACEMOCK_H_
