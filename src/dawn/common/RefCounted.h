// Copyright 2017 The Dawn Authors
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

#ifndef SRC_DAWN_COMMON_REFCOUNTED_H_
#define SRC_DAWN_COMMON_REFCOUNTED_H_

#include <atomic>
#include <cstdint>
#include <type_traits>

namespace dawn {

namespace detail {
class WeakRefData;
}  // namespace detail

class RefCount {
  public:
    // Create a refcount with a payload. The refcount starts initially at one.
    explicit RefCount(uint64_t payload = 0);

    uint64_t GetValueForTesting() const;
    uint64_t GetPayload() const;

    // Add a reference.
    void Increment();
    // Tries to add a reference. Returns false if the ref count is already at 0. This is used when
    // operating on a raw pointer to a RefCounted instead of a valid Ref that may be soon deleted.
    bool TryIncrement();

    // Remove a reference. Returns true if this was the last reference.
    bool Decrement();

  private:
    std::atomic<uint64_t> mRefCount;
};

class RefCounted {
  public:
    explicit RefCounted(uint64_t payload = 0);

    uint64_t GetRefCountForTesting() const;
    uint64_t GetRefCountPayload() const;

    void Reference();
    // Release() is called by internal code, so it's assumed that there is already a thread
    // synchronization in place for destruction.
    void Release();

    void APIReference() { Reference(); }
    void APIRelease() { Release(); }

  protected:
    // Friend class is needed to access the RefCount to TryIncrement.
    friend class detail::WeakRefData;

    virtual ~RefCounted();

    void ReleaseAndLockBeforeDestroy();

    // A Derived class may override these if they require a custom deleter.
    virtual void DeleteThis();
    // This calls DeleteThis() by default.
    virtual void LockAndDeleteThis();

    RefCount mRefCount;
};

}  // namespace dawn

#endif  // SRC_DAWN_COMMON_REFCOUNTED_H_
