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

#ifndef SRC_DAWN_REPLAY_DESERIALIZATION_H_
#define SRC_DAWN_REPLAY_DESERIALIZATION_H_

#include <webgpu/webgpu_cpp.h>

#include <cstdint>
#include <string>
#include <type_traits>
#include <vector>

#include "dawn/replay/Error.h"

namespace dawn::replay {

class ReadHead;

// ----- Deserialization -----

MaybeError ReadBytes(ReadHead& s, void* data, size_t size);
MaybeError Deserialize(ReadHead& s, int32_t* v);
MaybeError Deserialize(ReadHead& s, uint16_t* v);
MaybeError Deserialize(ReadHead& s, uint32_t* v);
MaybeError Deserialize(ReadHead& s, uint64_t* v);
MaybeError Deserialize(ReadHead& s, float* v);
MaybeError Deserialize(ReadHead& s, double* v);
MaybeError Deserialize(ReadHead& s, bool* v);
MaybeError Deserialize(ReadHead& s, std::string* v);

template <typename T>
MaybeError Deserialize(ReadHead& s, T* v)
    requires(std::is_same_v<T, size_t> && !std::is_same_v<T, uint64_t> &&
             !std::is_same_v<T, uint32_t>)
{
    return ReadBytes(s, reinterpret_cast<char*>(v), sizeof(*v));
}

template <typename T>
MaybeError Deserialize(ReadHead& s, std::vector<T>* v) {
    size_t size = 0;
    DAWN_TRY(Deserialize(s, &size));
    v->resize(size);
    for (size_t i = 0; i < size; ++i) {
        DAWN_TRY(Deserialize(s, &(*v)[i]));
    }
    return {};
}

// Deserialize for enum types with uint32_t or uint64_t underlying type.
template <typename T>
MaybeError Deserialize(
    ReadHead& s,
    T* value,
    typename std::enable_if<
        std::is_enum<T>::value &&
            (std::is_same<typename std::underlying_type<T>::type, uint32_t>::value ||
             std::is_same<typename std::underlying_type<T>::type, uint64_t>::value),
        void>::type* = nullptr) {
    return ReadBytes(s, reinterpret_cast<char*>(value), sizeof(T));
}

// Helper to call Deserialize on a parameter pack.
template <typename T, typename... Ts>
auto Deserialize(ReadHead& s, T* v, Ts*... vs)
    -> std::enable_if_t<(sizeof...(Ts) > 0) || !std::is_enum_v<T>, MaybeError> {
    DAWN_TRY(Deserialize(s, v));
    return Deserialize(s, vs...);
}

// Helper to call Deserialize on an empty parameter pack.
// Do nothing and return success.
inline MaybeError Deserialize(ReadHead& s) {
    return {};
}

// ----- Serializable -----

template <typename Derived>
class Deserializable {
  public:
    friend MaybeError Deserialize(ReadHead& readHead, Derived* out) {
        return out->VisitAll([&](auto&... members) { return Deserialize(readHead, &members...); });
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
    constexpr auto VisitAll(V&& visit) {                                                        \
        return [&](int, auto&... ms) {                                                          \
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
    qualifier Name : Name##__Contents, public ::dawn::replay::Deserializable<Name>

// Makes a struct for a given bindgrouplayout entry type.
#define DAWN_REPLAY_MAKE_BINDGROUP_LAYOUT_VARIANT(VariantName, VARIANT_MEMBERS)                 \
    DAWN_REPLAY_SERIALIZABLE(struct, BindGroupLayoutEntryType##VariantName##Data,               \
                             VARIANT_MEMBERS){};                                                \
                                                                                                \
    struct BindGroupLayoutEntryType##VariantName##__Contents {                                  \
        DAWN_REPLAY_INTERNAL_VISITABLE_MEMBER_DECL(BindGroupLayoutEntryType,                    \
                                                   variantType,                                 \
                                                   BindGroupLayoutEntryType::VariantName)       \
        DAWN_REPLAY_INTERNAL_VISITABLE_MEMBER_DECL(BindGroupLayoutBinding, binding)             \
        DAWN_REPLAY_INTERNAL_VISITABLE_MEMBER_DECL(BindGroupLayoutEntryType##VariantName##Data, \
                                                   data)                                        \
                                                                                                \
        template <typename V>                                                                   \
        constexpr auto VisitAll(V&& visit) {                                                    \
            return [&](int, auto&... ms) { return visit(ms...); }(                              \
                       kInternalVisitableUnusedForComma, variantType, binding, data);           \
        }                                                                                       \
    };                                                                                          \
    struct BindGroupLayoutEntryType##VariantName                                                \
        : BindGroupLayoutEntryType##VariantName##__Contents,                                    \
          public ::dawn::replay::Deserializable<BindGroupLayoutEntryType##VariantName>

// Makes a struct for a given bindgroup entry type.
#define DAWN_REPLAY_MAKE_BINDGROUP_VARIANT(VariantName, VARIANT_MEMBERS)                        \
    DAWN_REPLAY_SERIALIZABLE(struct, BindGroupEntryType##VariantName##Data, VARIANT_MEMBERS){}; \
                                                                                                \
    struct BindGroupEntryType##VariantName##__Contents {                                        \
        DAWN_REPLAY_INTERNAL_VISITABLE_MEMBER_DECL(BindGroupLayoutEntryType,                    \
                                                   variantType,                                 \
                                                   BindGroupLayoutEntryType::VariantName)       \
        DAWN_REPLAY_INTERNAL_VISITABLE_MEMBER_DECL(uint32_t, binding)                           \
        DAWN_REPLAY_INTERNAL_VISITABLE_MEMBER_DECL(BindGroupEntryType##VariantName##Data, data) \
                                                                                                \
        template <typename V>                                                                   \
        constexpr auto VisitAll(V&& visit) {                                                    \
            return [&](int, auto&... ms) { return visit(ms...); }(                              \
                       kInternalVisitableUnusedForComma, variantType, binding, data);           \
        }                                                                                       \
    };                                                                                          \
    struct BindGroupEntryType##VariantName                                                      \
        : BindGroupEntryType##VariantName##__Contents,                                          \
          public ::dawn::replay::Deserializable<BindGroupEntryType##VariantName>

// Makes both a CmdData and a Cmd struct for a given command name.
#define DAWN_REPLAY_MAKE_CMD_AND_CMD_DATA(CmdType, CmdName, CMD_MEMBERS)               \
    DAWN_REPLAY_SERIALIZABLE(struct, CmdType##CmdName##CmdData, CMD_MEMBERS){};        \
                                                                                       \
    struct CmdType##CmdName##Cmd##__Contents {                                         \
        DAWN_REPLAY_INTERNAL_VISITABLE_MEMBER_DECL(CmdType, command, CmdType::CmdName) \
        DAWN_REPLAY_INTERNAL_VISITABLE_MEMBER_DECL(CmdType##CmdName##CmdData, data)    \
                                                                                       \
        template <typename V>                                                          \
        constexpr auto VisitAll(V&& visit) {                                           \
            return [&](int, auto&... ms) { return visit(ms...); }(                     \
                       kInternalVisitableUnusedForComma, command, data);               \
        }                                                                              \
    };                                                                                 \
    struct CmdType##CmdName##Cmd : CmdType##CmdName##Cmd##__Contents,                  \
                                   public ::dawn::replay::Deserializable<CmdType##CmdName##Cmd>

// Makes both a CmdData and a Cmd struct for a given render pass command name.
#define DAWN_REPLAY_MAKE_RENDER_PASS_CMD_AND_CMD_DATA(CmdName, CMD_MEMBERS) \
    DAWN_REPLAY_MAKE_CMD_AND_CMD_DATA(RenderPassCommand, CmdName, CMD_MEMBERS)

// Makes both a CmdData and a Cmd struct for a given compute pass command name.
#define DAWN_REPLAY_MAKE_COMPUTE_PASS_CMD_AND_CMD_DATA(CmdName, CMD_MEMBERS) \
    DAWN_REPLAY_MAKE_CMD_AND_CMD_DATA(ComputePassCommand, CmdName, CMD_MEMBERS)

// Makes both a CmdData and a Cmd struct for a given encoder command name.
#define DAWN_REPLAY_MAKE_ENCODER_CMD_AND_CMD_DATA(CmdName, CMD_MEMBERS) \
    DAWN_REPLAY_MAKE_CMD_AND_CMD_DATA(EncoderCommand, CmdName, CMD_MEMBERS)

// Makes both a CmdData and a Cmd struct for a given root command name.
#define DAWN_REPLAY_MAKE_ROOT_CMD_AND_CMD_DATA(CmdName, CMD_MEMBERS) \
    DAWN_REPLAY_MAKE_CMD_AND_CMD_DATA(RootCommand, CmdName, CMD_MEMBERS)

#include "dawn/serialization/Schema.h"

}  // namespace dawn::replay

#endif  // SRC_DAWN_REPLAY_DESERIALIZATION_H_
