// Copyright 2025 The Dawn & Tint Authors
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS-IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#ifndef SRC_DAWN_NATIVE_WEBGPU_SERIALIZATION_H_
#define SRC_DAWN_NATIVE_WEBGPU_SERIALIZATION_H_

#include <webgpu/webgpu_cpp.h>

#include <cstdint>
#include <string>
#include <type_traits>

#include "dawn/native/Error.h"

namespace dawn::native::webgpu {

class CaptureContext;

void WriteBytes(CaptureContext& context, const void* data, size_t size);

void Serialize(CaptureContext& context, int32_t v);
void Serialize(CaptureContext& context, uint32_t v);
void Serialize(CaptureContext& context, uint64_t v);
void Serialize(CaptureContext& context, const std::string& v);

template <typename T>
void Serialize(CaptureContext& context, T v)
    requires(std::is_same_v<T, size_t> && !std::is_same_v<T, uint64_t> &&
             !std::is_same_v<T, uint32_t>)
{
    WriteBytes(context, reinterpret_cast<const char*>(&v), sizeof(v));
}

// Serialize for enum types with uint32_t or uint64_t underlying type.
template <typename T>
void Serialize(CaptureContext& s,
               T value,
               typename std::enable_if<
                   std::is_enum<T>::value &&
                       (std::is_same<typename std::underlying_type<T>::type, uint32_t>::value ||
                        std::is_same<typename std::underlying_type<T>::type, uint64_t>::value),
                   void>::type* = nullptr) {
    return WriteBytes(s, &value, sizeof(T));
}

// Helper to call Serialize on a parameter pack.
template <typename T, typename... Ts>
constexpr auto Serialize(CaptureContext& s, const T& v, const Ts&... vs)
    -> std::enable_if_t<(sizeof...(Ts) > 0) || !std::is_enum_v<T>, void> {
    Serialize(s, v);
    Serialize(s, vs...);
}

// Helper to call Serialize on an empty parameter pack.
// Do nothing.
inline void Serialize(CaptureContext& context) {}

template <typename Derived>
class Serializable {
  public:
    friend void Serialize(CaptureContext& context, const Derived& in) {
        in.VisitAll([&](const auto&... members) { Serialize(context, members...); });
    }
};

// Helper for X macro to declare a visitable member.
#define DAWN_REPLAY_INTERNAL_VISITABLE_MEMBER_DECL(type, name, ...) \
    type name{__VA_OPT__(__VA_ARGS__)};

// Helper for X macro for visiting a visitable member.
#define DAWN_REPLAY_INTERNAL_VISITABLE_MEMBER_ARG(type, name, ...) , name

constexpr int kInternalVisitableUnusedForComma = 0;

// Helper X macro to declare members of a class or struct, along with VisitAll
// methods to call a functor on all members.
// Example usage:
//   #define MEMBERS(X)    \
//       X(int, a)         \
//       X(float, b, 42.0) \
//       X(Foo, foo, kFoo) \
//       X(Bar, bar)
//   struct MyStruct {
//    DAWN_REPLAY_VISITABLE_MEMBERS(MEMBERS)
//   };
//   #undef MEMBERS
#define DAWN_REPLAY_VISITABLE_MEMBERS(MEMBERS)                                                  \
    MEMBERS(DAWN_REPLAY_INTERNAL_VISITABLE_MEMBER_DECL)                                         \
                                                                                                \
    template <typename V>                                                                       \
    constexpr auto VisitAll(V&& visit) const {                                                  \
        return [&](int, const auto&... ms) {                                                    \
            return visit(ms...);                                                                \
        }(kInternalVisitableUnusedForComma MEMBERS(DAWN_REPLAY_INTERNAL_VISITABLE_MEMBER_ARG)); \
    }

// Helper macro to define a struct or class along with VisitAll methods to call
// a functor on all members.
// Example usage:
//   #define MEMBERS(X) \
//       X(int, a)              \
//       X(float, b)            \
//       X(Foo, foo)            \
//       X(Bar, bar)
//   DAWN_REPLAY_SERIALIZABLE(struct, MyStruct, MEMBERS) {
//      void SomeAdditionalMethod();
//   };
//   #undef MEMBERS
#define DAWN_REPLAY_SERIALIZABLE(qualifier, Name, MEMBERS) \
    struct Name##__Contents {                              \
        DAWN_REPLAY_VISITABLE_MEMBERS(MEMBERS)             \
    };                                                     \
    qualifier Name : Name##__Contents, public Serializable<Name>

// Makes both a CmdData and a Cmd struct for a given root command name.
#define DAWN_REPLAY_MAKE_ROOT_CMD_AND_CMD_DATA(CmdName, CMD_MEMBERS)                           \
    DAWN_REPLAY_SERIALIZABLE(struct, CmdName##CmdData, CMD_MEMBERS){};                         \
                                                                                               \
    struct CmdName##Cmd##__Contents {                                                          \
        DAWN_REPLAY_INTERNAL_VISITABLE_MEMBER_DECL(RootCommand, command, RootCommand::CmdName) \
        DAWN_REPLAY_INTERNAL_VISITABLE_MEMBER_DECL(CmdName##CmdData, data)                     \
                                                                                               \
        template <typename V>                                                                  \
        constexpr auto VisitAll(V&& visit) const {                                             \
            return [&](int, const auto&... ms) { return visit(ms...); }(                       \
                       kInternalVisitableUnusedForComma, command, data);                       \
        }                                                                                      \
    };                                                                                         \
    struct CmdName##Cmd : CmdName##Cmd##__Contents, public Serializable<CmdName##Cmd>

#include "dawn/serialization/Schema.h"

}  // namespace dawn::native::webgpu

#endif  // SRC_DAWN_NATIVE_WEBGPU_SERIALIZATION_H_
