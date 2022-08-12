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

#ifndef SRC_DAWN_NATIVE_SERIALIZABLE_H_
#define SRC_DAWN_NATIVE_SERIALIZABLE_H_

#include <utility>

#include "dawn/native/VisitableMembers.h"
#include "dawn/native/stream/BlobSource.h"
#include "dawn/native/stream/ByteVectorSink.h"
#include "dawn/native/stream/Stream.h"

namespace dawn::native {

// Base CRTP for implementing StreamIn/StreamOut/FromBlob/ToBlob for Derived,
// assuming Derived has VisitAll methods provided by DAWN_VISITABLE_MEMBERS.
template <typename Derived>
class Serializable {
  public:
    friend void StreamIn(stream::Sink* s, const Derived& in) {
        in.VisitAll([&](const auto&... members) { StreamIn(s, members...); });
    }

    friend MaybeError StreamOut(stream::Source* s, Derived* out) {
        return out->VisitAll([&](auto&... members) { return StreamOut(s, &members...); });
    }

    static ResultOrError<Derived> FromBlob(Blob blob) {
        stream::BlobSource source(std::move(blob));
        Derived out;
        DAWN_TRY(StreamOut(&source, &out));
        return out;
    }

    Blob ToBlob() const {
        stream::ByteVectorSink sink;
        StreamIn(&sink, static_cast<const Derived&>(*this));
        return CreateBlob(std::move(sink));
    }
};
}  // namespace dawn::native

// Helper macro to define a struct or class along with VisitAll methods to call
// a functor on all members. Derives from Visitable which provides
// implementations of StreamIn/StreamOut/FromBlob/ToBlob.
// Example usage:
//   #define MEMBERS(X) \
//       X(int, a)              \
//       X(float, b)            \
//       X(Foo, foo)            \
//       X(Bar, bar)
//   DAWN_SERIALIZABLE(struct, MyStruct, MEMBERS) {
//      void SomeAdditionalMethod();
//   };
//   #undef MEMBERS
#define DAWN_SERIALIZABLE(qualifier, Name, MEMBERS) \
    struct Name##__Contents {                       \
        DAWN_VISITABLE_MEMBERS(MEMBERS)             \
    };                                              \
    qualifier Name : Name##__Contents, public ::dawn::native::Serializable<Name>

#endif  // SRC_DAWN_NATIVE_SERIALIZABLE_H_
