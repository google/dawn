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

#ifndef SRC_DAWN_NATIVE_REFCOUNTEDWITHEXTERNALCOUNT_H_
#define SRC_DAWN_NATIVE_REFCOUNTEDWITHEXTERNALCOUNT_H_

#include "dawn/common/RefCounted.h"

namespace dawn::native {

// RecCountedWithExternalCount is a version of RefCounted which tracks a separate
// refcount for calls to APIReference/APIRelease (refs added/removed by the application).
// The external refcount starts at 1, and the total refcount starts at 1 - i.e. the first
// ref is the external ref.
// Then, when the external refcount drops to zero, WillDropLastExternalRef is called.
// The derived class should override the behavior of WillDropLastExternalRef.
class RefCountedWithExternalCount : private RefCounted {
  public:
    using RefCounted::RefCounted;
    using RefCounted::Reference;
    using RefCounted::Release;

    void APIReference();
    void APIRelease();

  private:
    virtual void WillDropLastExternalRef() = 0;

    RefCount mExternalRefCount;
};

}  // namespace dawn::native

#endif  // SRC_DAWN_NATIVE_REFCOUNTEDWITHEXTERNALCOUNT_H_
