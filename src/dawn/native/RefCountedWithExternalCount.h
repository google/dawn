// Copyright 2022 The Dawn & Tint Authors
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
