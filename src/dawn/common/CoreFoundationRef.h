// Copyright 2020 The Dawn Authors
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

#ifndef SRC_DAWN_COMMON_COREFOUNDATIONREF_H_
#define SRC_DAWN_COMMON_COREFOUNDATIONREF_H_

#include <CoreFoundation/CoreFoundation.h>

#include "dawn/common/RefBase.h"

template <typename T>
struct CoreFoundationRefTraits {
    static constexpr T kNullValue = nullptr;
    static void Reference(T value) { CFRetain(value); }
    static void Release(T value) { CFRelease(value); }
};

template <typename T>
class CFRef : public RefBase<T, CoreFoundationRefTraits<T>> {
  public:
    using RefBase<T, CoreFoundationRefTraits<T>>::RefBase;
};

template <typename T>
CFRef<T> AcquireCFRef(T pointee) {
    CFRef<T> ref;
    ref.Acquire(pointee);
    return ref;
}

#endif  // SRC_DAWN_COMMON_COREFOUNDATIONREF_H_
