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

#ifndef SRC_DAWN_NATIVE_VISITABLE_H_
#define SRC_DAWN_NATIVE_VISITABLE_H_

#include <utility>

#include "dawn/native/stream/BlobSource.h"
#include "dawn/native/stream/ByteVectorSink.h"
#include "dawn/native/stream/Stream.h"

// Helper for X macro to declare a visitable member.
#define DAWN_INTERNAL_VISITABLE_MEMBER_DECL(type, name) type name{};

// Helper for X macro for visiting a visitable member.
#define DAWN_INTERNAL_VISITABLE_MEMBER_ARG(type, name) , name

namespace dawn::native::detail {
constexpr int kInternalVisitableUnusedForComma = 0;
}  // namespace dawn::native::detail

// Helper X macro to declare members of a class or struct, along with VisitAll
// methods to call a functor on all members.
// Example usage:
//   #define MEMBERS(X) \
//       X(int, a)              \
//       X(float, b)            \
//       X(Foo, foo)            \
//       X(Bar, bar)
//   struct MyStruct {
//    DAWN_VISITABLE_MEMBERS(MEMBERS)
//   };
//   #undef MEMBERS
#define DAWN_VISITABLE_MEMBERS(MEMBERS)                                     \
    MEMBERS(DAWN_INTERNAL_VISITABLE_MEMBER_DECL)                            \
                                                                            \
    template <typename V>                                                   \
    constexpr auto VisitAll(V&& visit) const {                              \
        return [&](int, const auto&... ms) {                                \
            return visit(ms...);                                            \
        }(::dawn::native::detail::kInternalVisitableUnusedForComma MEMBERS( \
                   DAWN_INTERNAL_VISITABLE_MEMBER_ARG));                    \
    }                                                                       \
                                                                            \
    template <typename V>                                                   \
    constexpr auto VisitAll(V&& visit) {                                    \
        return [&](int, auto&... ms) {                                      \
            return visit(ms...);                                            \
        }(::dawn::native::detail::kInternalVisitableUnusedForComma MEMBERS( \
                   DAWN_INTERNAL_VISITABLE_MEMBER_ARG));                    \
    }

#endif  // SRC_DAWN_NATIVE_VISITABLE_H_
