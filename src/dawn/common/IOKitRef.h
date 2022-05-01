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

#ifndef SRC_DAWN_COMMON_IOKITREF_H_
#define SRC_DAWN_COMMON_IOKITREF_H_

#include <IOKit/IOKitLib.h>

#include "dawn/common/RefBase.h"

template <typename T>
struct IOKitRefTraits {
    static constexpr T kNullValue = IO_OBJECT_NULL;
    static void Reference(T value) { IOObjectRetain(value); }
    static void Release(T value) { IOObjectRelease(value); }
};

template <typename T>
class IORef : public RefBase<T, IOKitRefTraits<T>> {
  public:
    using RefBase<T, IOKitRefTraits<T>>::RefBase;
};

template <typename T>
IORef<T> AcquireIORef(T pointee) {
    IORef<T> ref;
    ref.Acquire(pointee);
    return ref;
}

#endif  // SRC_DAWN_COMMON_IOKITREF_H_
