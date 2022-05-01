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

#ifndef SRC_DAWN_UTILS_SCOPEDAUTORELEASEPOOL_H_
#define SRC_DAWN_UTILS_SCOPEDAUTORELEASEPOOL_H_

#include <cstddef>

#include "dawn/common/Compiler.h"

namespace utils {

/**
 * ScopedAutoreleasePool is a scoped class which initializes an NSAutoreleasePool on
 * creation, and drains it on destruction. On non-Apple platforms, ScopedAutoreleasePool
 * is a no-op.
 *
 * An autoreleasepool is needed when using protocol objects in Objective-C because Cocoa
 * expects a pool to always be available in each thread. If a pool is not available, then
 * autoreleased objects will never be released and will leak.
 *
 * In long-running blocks of code or loops, it is important to periodically create and drain
 * autorelease pools so that memory is recycled. In Dawn's tests, we have an autoreleasepool
 * per-test. In graphics applications it's advised to create an autoreleasepool around the
 * frame loop. Ex.)
 *   void frame() {
 *     // Any protocol objects will be reclaimed when this object falls out of scope.
 *     utils::ScopedAutoreleasePool pool;
 *
 *     // do rendering ...
 *   }
 */
class [[nodiscard]] ScopedAutoreleasePool {
  public:
    ScopedAutoreleasePool();
    ~ScopedAutoreleasePool();

    ScopedAutoreleasePool(const ScopedAutoreleasePool&) = delete;
    ScopedAutoreleasePool& operator=(const ScopedAutoreleasePool&) = delete;

    ScopedAutoreleasePool(ScopedAutoreleasePool&&);
    ScopedAutoreleasePool& operator=(ScopedAutoreleasePool&&);

  private:
    void* mPool = nullptr;
};

}  // namespace utils

#endif  // SRC_DAWN_UTILS_SCOPEDAUTORELEASEPOOL_H_
