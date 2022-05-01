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

#ifndef SRC_DAWN_COMMON_NSREF_H_
#define SRC_DAWN_COMMON_NSREF_H_

#include "dawn/common/RefBase.h"

#import <Foundation/NSObject.h>

#if !defined(__OBJC__)
#error "NSRef can only be used in Objective C/C++ code."
#endif

// This file contains smart pointers that automatically reference and release Objective C objects
// and prototocals in a manner very similar to Ref<>. Note that NSRef<> and NSPRef's constructor add
// a reference to the object by default, so the pattern to get a reference for a newly created
// NSObject is the following:
//
//    NSRef<NSFoo> foo = AcquireNSRef([NSFoo alloc]);
//
// NSRef overloads -> and * but these operators don't work extremely well with Objective C's
// features. For example automatic dereferencing when doing the following doesn't work:
//
//    NSFoo* foo;
//    foo.member = 1;
//    someVar = foo.member;
//
// Instead use the message passing syntax:
//
//    NSRef<NSFoo> foo;
//    [*foo setMember: 1];
//    someVar = [*foo member];
//
// Also did you notive the extra '*' in the example above? That's because Objective C's message
// passing doesn't automatically call a C++ operator to dereference smart pointers (like -> does) so
// we have to dereference manually using '*'. In some cases the extra * or message passing syntax
// can get a bit annoying so instead a local "naked" pointer can be borrowed from the NSRef. This
// would change the syntax overload in the following:
//
//    NSRef<NSFoo> foo;
//    [*foo setA:1];
//    [*foo setB:2];
//    [*foo setC:3];
//
// Into (note access to members of ObjC classes referenced via pointer is done with . and not ->):
//
//    NSRef<NSFoo> fooRef;
//    NSFoo* foo = fooRef.Get();
//    foo.a = 1;
//    foo.b = 2;
//    boo.c = 3;
//
// Which can be subjectively easier to read.

template <typename T>
struct NSRefTraits {
    static constexpr T kNullValue = nullptr;
    static void Reference(T value) { [value retain]; }
    static void Release(T value) { [value release]; }
};

template <typename T>
class NSRef : public RefBase<T*, NSRefTraits<T*>> {
  public:
    using RefBase<T*, NSRefTraits<T*>>::RefBase;

    const T* operator*() const { return this->Get(); }

    T* operator*() { return this->Get(); }
};

template <typename T>
NSRef<T> AcquireNSRef(T* pointee) {
    NSRef<T> ref;
    ref.Acquire(pointee);
    return ref;
}

// This is a RefBase<> for an Objective C protocol (hence the P). Objective C protocols must always
// be referenced with id<ProtocolName> and not just ProtocolName* so they cannot use NSRef<>
// itself. That's what the P in NSPRef stands for: Protocol.
template <typename T>
class NSPRef : public RefBase<T, NSRefTraits<T>> {
  public:
    using RefBase<T, NSRefTraits<T>>::RefBase;

    const T operator*() const { return this->Get(); }

    T operator*() { return this->Get(); }
};

template <typename T>
NSPRef<T> AcquireNSPRef(T pointee) {
    NSPRef<T> ref;
    ref.Acquire(pointee);
    return ref;
}

#endif  // SRC_DAWN_COMMON_NSREF_H_
