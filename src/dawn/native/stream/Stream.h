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

#ifndef SRC_DAWN_NATIVE_STREAM_STREAM_H_
#define SRC_DAWN_NATIVE_STREAM_STREAM_H_

#include <algorithm>
#include <bitset>
#include <functional>
#include <limits>
#include <unordered_map>
#include <utility>
#include <vector>

#include <optional>

#include "dawn/common/Platform.h"
#include "dawn/common/TypedInteger.h"
#include "dawn/native/Error.h"
#include "dawn/native/stream/Sink.h"
#include "dawn/native/stream/Source.h"

namespace dawn::native::stream {

// Base Stream template for specialization. Specializations may define static methods:
//   static void Write(Sink* s, const T& v);
//   static MaybeError Read(Source* s, T* v);
template <typename T, typename SFINAE = void>
class Stream {
  public:
    static void Write(Sink* s, const T& v);
    static MaybeError Read(Source* s, T* v);
};

// Helper to call Stream<T>::Write.
// By default, calling StreamIn will call this overload inside the stream namespace.
// Other definitons of StreamIn found by ADL may override this one.
template <typename T>
constexpr void StreamIn(Sink* s, const T& v) {
    Stream<T>::Write(s, v);
}

// Helper to call Stream<T>::Read
// By default, calling StreamOut will call this overload inside the stream namespace.
// Other definitons of StreamOut found by ADL may override this one.
template <typename T>
MaybeError StreamOut(Source* s, T* v) {
    return Stream<T>::Read(s, v);
}

// Helper to take an rvalue passed to StreamOut and forward it as a pointer.
// This makes it possible to pass output wrappers like stream::StructMembers inline.
// For example: `DAWN_TRY(StreamOut(&source, stream::StructMembers(...)));`
template <typename T>
MaybeError StreamOut(Source* s, T&& v) {
    return StreamOut(s, &v);
}

// Helper to call StreamIn on a parameter pack.
template <typename T, typename... Ts>
constexpr void StreamIn(Sink* s, const T& v, const Ts&... vs) {
    StreamIn(s, v);
    (StreamIn(s, vs), ...);
}

// Helper to call StreamOut on a parameter pack.
template <typename T, typename... Ts>
MaybeError StreamOut(Source* s, T* v, Ts*... vs) {
    DAWN_TRY(StreamOut(s, v));
    return StreamOut(s, vs...);
}

// Stream specialization for fundamental types.
template <typename T>
class Stream<T, std::enable_if_t<std::is_fundamental_v<T>>> {
  public:
    static void Write(Sink* s, const T& v) { memcpy(s->GetSpace(sizeof(T)), &v, sizeof(T)); }
    static MaybeError Read(Source* s, T* v) {
        const void* ptr;
        DAWN_TRY(s->Read(&ptr, sizeof(T)));
        memcpy(v, ptr, sizeof(T));
        return {};
    }
};

namespace detail {
// NOLINTNEXTLINE(runtime/int) Alias "unsigned long long" type to match std::bitset to_ullong
using BitsetUllong = unsigned long long;
constexpr size_t kBitsPerUllong = 8 * sizeof(BitsetUllong);
constexpr bool BitsetSupportsToUllong(size_t N) {
    return N <= kBitsPerUllong;
}
}  // namespace detail

// Stream specialization for bitsets that are smaller than BitsetUllong.
template <size_t N>
class Stream<std::bitset<N>, std::enable_if_t<detail::BitsetSupportsToUllong(N)>> {
  public:
    static void Write(Sink* s, const std::bitset<N>& t) { StreamIn(s, t.to_ullong()); }
    static MaybeError Read(Source* s, std::bitset<N>* v) {
        detail::BitsetUllong value;
        DAWN_TRY(StreamOut(s, &value));
        *v = std::bitset<N>(value);
        return {};
    }
};

// Stream specialization for bitsets since using the built-in to_ullong has a size limit.
template <size_t N>
class Stream<std::bitset<N>, std::enable_if_t<!detail::BitsetSupportsToUllong(N)>> {
  public:
    static void Write(Sink* s, const std::bitset<N>& t) {
        // Iterate in chunks of detail::BitsetUllong.
        static std::bitset<N> mask(std::numeric_limits<detail::BitsetUllong>::max());

        std::bitset<N> bits = t;
        for (size_t offset = 0; offset < N;
             offset += detail::kBitsPerUllong, bits >>= detail::kBitsPerUllong) {
            StreamIn(s, (mask & bits).to_ullong());
        }
    }

    static MaybeError Read(Source* s, std::bitset<N>* v) {
        static_assert(N > 0);
        *v = {};

        // Iterate in chunks of detail::BitsetUllong.
        for (size_t offset = 0; offset < N;
             offset += detail::kBitsPerUllong, (*v) <<= detail::kBitsPerUllong) {
            detail::BitsetUllong ullong;
            DAWN_TRY(StreamOut(s, &ullong));
            *v |= std::bitset<N>(ullong);
        }
        return {};
    }
};

// Stream specialization for enums.
template <typename T>
class Stream<T, std::enable_if_t<std::is_enum_v<T>>> {
    using U = std::underlying_type_t<T>;

  public:
    static void Write(Sink* s, const T& v) { StreamIn(s, static_cast<U>(v)); }

    static MaybeError Read(Source* s, T* v) {
        U out;
        DAWN_TRY(StreamOut(s, &out));
        *v = static_cast<T>(out);
        return {};
    }
};

// Stream specialization for TypedInteger.
template <typename Tag, typename Integer>
class Stream<::detail::TypedIntegerImpl<Tag, Integer>> {
    using T = ::detail::TypedIntegerImpl<Tag, Integer>;

  public:
    static void Write(Sink* s, const T& t) { StreamIn(s, static_cast<Integer>(t)); }

    static MaybeError Read(Source* s, T* v) {
        Integer out;
        DAWN_TRY(StreamOut(s, &out));
        *v = T(out);
        return {};
    }
};

// Stream specialization for pointers. We always serialize via value, not by pointer.
// To handle nullptr scenarios, we always serialize whether the pointer was not nullptr,
// followed by the contents if applicable.
template <typename T>
class Stream<T, std::enable_if_t<std::is_pointer_v<T>>> {
  public:
    static void Write(stream::Sink* sink, const T& t) {
        using Pointee = std::decay_t<std::remove_pointer_t<T>>;
        static_assert(!std::is_same_v<char, Pointee> && !std::is_same_v<wchar_t, Pointee> &&
                          !std::is_same_v<char16_t, Pointee> && !std::is_same_v<char32_t, Pointee>,
                      "C-str like type likely has ambiguous serialization. For a string, wrap with "
                      "std::string_view instead.");
        StreamIn(sink, t != nullptr);
        if (t != nullptr) {
            StreamIn(sink, *t);
        }
    }
};

// Stream specialization for std::optional
template <typename T>
class Stream<std::optional<T>> {
  public:
    static void Write(stream::Sink* sink, const std::optional<T>& t) {
        bool hasValue = t.has_value();
        StreamIn(sink, hasValue);
        if (hasValue) {
            StreamIn(sink, *t);
        }
    }
};

// Stream specialization for fixed arrays of fundamental types.
template <typename T, size_t N>
class Stream<T[N], std::enable_if_t<std::is_fundamental_v<T>>> {
  public:
    static void Write(Sink* s, const T (&t)[N]) {
        static_assert(N > 0);
        memcpy(s->GetSpace(sizeof(t)), &t, sizeof(t));
    }

    static MaybeError Read(Source* s, T (*t)[N]) {
        static_assert(N > 0);
        const void* ptr;
        DAWN_TRY(s->Read(&ptr, sizeof(*t)));
        memcpy(*t, ptr, sizeof(*t));
        return {};
    }
};

// Specialization for fixed arrays of non-fundamental types.
template <typename T, size_t N>
class Stream<T[N], std::enable_if_t<!std::is_fundamental_v<T>>> {
  public:
    static void Write(Sink* s, const T (&t)[N]) {
        static_assert(N > 0);
        for (size_t i = 0; i < N; i++) {
            StreamIn(s, t[i]);
        }
    }

    static MaybeError Read(Source* s, T (*t)[N]) {
        static_assert(N > 0);
        for (size_t i = 0; i < N; i++) {
            DAWN_TRY(StreamOut(s, &(*t)[i]));
        }
        return {};
    }
};

// Stream specialization for std::vector.
template <typename T>
class Stream<std::vector<T>> {
  public:
    static void Write(Sink* s, const std::vector<T>& v) {
        StreamIn(s, v.size());
        for (const T& it : v) {
            StreamIn(s, it);
        }
    }

    static MaybeError Read(Source* s, std::vector<T>* v) {
        using SizeT = decltype(std::declval<std::vector<T>>().size());
        SizeT size;
        DAWN_TRY(StreamOut(s, &size));
        *v = {};
        v->reserve(size);
        for (SizeT i = 0; i < size; ++i) {
            T el;
            DAWN_TRY(StreamOut(s, &el));
            v->push_back(std::move(el));
        }
        return {};
    }
};

// Stream specialization for std::pair.
template <typename A, typename B>
class Stream<std::pair<A, B>> {
  public:
    static void Write(Sink* s, const std::pair<A, B>& v) {
        StreamIn(s, v.first);
        StreamIn(s, v.second);
    }

    static MaybeError Read(Source* s, std::pair<A, B>* v) {
        DAWN_TRY(StreamOut(s, &v->first));
        DAWN_TRY(StreamOut(s, &v->second));
        return {};
    }
};

// Stream specialization for std::unordered_map<K, V> which sorts the entries
// to provide a stable ordering.
template <typename K, typename V>
class Stream<std::unordered_map<K, V>> {
  public:
    static void Write(stream::Sink* sink, const std::unordered_map<K, V>& m) {
        std::vector<std::pair<K, V>> ordered(m.begin(), m.end());
        std::sort(
            ordered.begin(), ordered.end(),
            [](const std::pair<K, V>& a, const std::pair<K, V>& b) { return a.first < b.first; });
        StreamIn(sink, ordered);
    }
};

// Helper class to contain the begin/end iterators of an iterable.
namespace detail {
template <typename Iterator>
struct Iterable {
    Iterator begin;
    Iterator end;
};
}  // namespace detail

// Helper for making detail::Iterable from a pointer and count.
template <typename T>
auto Iterable(const T* ptr, size_t count) {
    using Iterator = const T*;
    return detail::Iterable<Iterator>{ptr, ptr + count};
}

// Stream specialization for detail::Iterable which writes the number of elements,
// followed by the elements.
template <typename Iterator>
class Stream<detail::Iterable<Iterator>> {
  public:
    static void Write(stream::Sink* sink, const detail::Iterable<Iterator>& iter) {
        StreamIn(sink, std::distance(iter.begin, iter.end));
        for (auto it = iter.begin; it != iter.end; ++it) {
            StreamIn(sink, *it);
        }
    }
};

}  // namespace dawn::native::stream

#endif  // SRC_DAWN_NATIVE_STREAM_STREAM_H_
