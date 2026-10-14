// Copyright 2026 The Dawn & Tint Authors
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

#ifndef SRC_UTILS_HEAP_ARRAY_H_
#define SRC_UTILS_HEAP_ARRAY_H_

#include <algorithm>
#include <cstddef>
#include <limits>
#include <ranges>
#include <type_traits>
#include <utility>

#include "src/utils/non_copyable.h"
#include "src/utils/numeric.h"
#include "src/utils/platform.h"
#include "src/utils/span.h"
#include "src/utils/underlying_type.h"

namespace dawn {

namespace ityp {

// "Lite", index-typed version of base::HeapArray from Chromium. It's like a safer, sized
// std::unique_ptr<T[]>, and it has a nicer (span-like / std::ranges-compatible) interface.
// To convert to an actual span, use `Span{heapArray}`, which constructs it via std::ranges.
//
// Note that just like new[], it's invalid to reinterpret to another type UNLESS the element type is
// char or std::byte AND the target type's alignment is <= alignof(std::max_align_t).
template <HasUnsignedUnderlyingType Index, typename Value>
class HeapArray :
    // Could be copyable, if needed, but we haven't needed it.
    public NonCopyable {
  private:
    using I = UnderlyingType<Index>;
    // TODO(https://crbug.com/526537224): This should probably be a RawSpan.
    using TSpan = ityp::span<Index, Value>;

    // HeapArrays never start with data in them, so a constant HeapArray wouldn't be usable.
    // If we really want that, we can implement a move-constructor from non-const to const.
    static_assert(!std::is_const_v<Value>,
                  "Array contents cannot be constant. Use `const HeapArray<I, V>` instead of "
                  "`HeapArray<I, const V>`.");

  public:
    // Constructs an empty HeapArray. (Note it cannot be resized.)
    constexpr HeapArray() = default;

    constexpr HeapArray(HeapArray<Index, Value>&& other) { *this = std::move(other); }
    constexpr HeapArray<Index, Value>& operator=(HeapArray<Index, Value>&& other) {
        std::swap(mOwnedData, other.mOwnedData);
        return *this;
    }

    constexpr ~HeapArray() {
        if (Value* ptr = mOwnedData.data()) {
            mOwnedData = {};
            DeleteAllocation(ptr);
        }
    }

    // Constructs a zero-initialized HeapArray with count `count`.
    constexpr explicit HeapArray(Index count)
        // SAFETY: Allocation size matches container size.
        : DAWN_UNSAFE_BUFFERS(HeapArray{Alloc<InitType::Init, ThrowType::Throw>(count), count}) {
        // Even if count is 0, the new[] shouldn't have returned nullptr.
        DAWN_ASSERT(mOwnedData.data() != nullptr);
    }
    // Constructs a zero-initialized HeapArray with count `count`.
    // If allocation fails, returns a falsy object, with .data() == nullptr and .size() == 0.
    constexpr HeapArray(Index count, std::nothrow_t)
        // SAFETY: Allocation size matches container size; private constructor handles if it fails.
        : DAWN_UNSAFE_BUFFERS(HeapArray{Alloc<InitType::Init, ThrowType::NoThrow>(count), count}) {
        if (mOwnedData.data() == nullptr) {
            DAWN_ASSERT(mOwnedData.size() == Index{});
        }
    }

    // Constructs an uninitialized HeapArray with count `count`.
    // This can only be used with POD types, as other types are always initialized.
    [[nodiscard]] DAWN_UNSAFE_BUFFER_USAGE static constexpr HeapArray<Index, Value> Uninit(
        Index count)
        requires std::is_trivially_default_constructible_v<Value>
    {
        return HeapArray<Index, Value>{Alloc<InitType::Uninit, ThrowType::Throw>(count), count};
    }
    // Constructs an uninitialized HeapArray with count `count`, or count 0 if allocation fails.
    // This can only be used with POD types, as other types are always initialized.
    [[nodiscard]] DAWN_UNSAFE_BUFFER_USAGE static constexpr HeapArray<Index, Value> Uninit(
        Index count,
        std::nothrow_t)
        requires std::is_trivially_default_constructible_v<Value>
    {
        return HeapArray<Index, Value>{Alloc<InitType::Uninit, ThrowType::NoThrow>(count), count};
    }

    // Acquire the contents as a size, it's valid to delete using `delete[] span.data()`).
    // Useful when returning an array as a span (e.g. via the webgpu.h C++ API).
    // The resulting data must be deallocated using DeleteAllocationFromHeapArray so that the
    // standard library delete function is used.
    constexpr TSpan MoveToSpan() && {
        TSpan result = mOwnedData;
        mOwnedData = {};
        return result;
    }

    static void DeleteAllocation(const Value* alloc) { Delete(alloc); }

    // Returns true if the allocation succeeded. This can be used like `if (myHeapArray) {}` to
    // check if nothrow allocation succeeded. Note, even if the size is 0, this may return true.
    constexpr explicit operator bool() { return mOwnedData.data() != nullptr; }

    // Span-like interface
    constexpr auto empty() const { return mOwnedData.empty(); }
    constexpr auto data() { return mOwnedData.data(); }
    constexpr auto data() const { return mOwnedData.data(); }
    constexpr auto size() const { return mOwnedData.size(); }
    constexpr auto begin() { return mOwnedData.begin(); }
    constexpr auto begin() const { return mOwnedData.begin(); }
    constexpr auto end() { return mOwnedData.end(); }
    constexpr auto end() const { return mOwnedData.end(); }
    constexpr auto subspan(Index offset) { return mOwnedData.subspan(offset); }
    constexpr auto subspan(Index offset) const { return mOwnedData.subspan(offset); }
    constexpr auto subspan(Index offset, Index count) { return mOwnedData.subspan(offset, count); }
    constexpr auto subspan(Index offset, Index count) const {
        return mOwnedData.subspan(offset, count);
    }
    constexpr auto& front() { return mOwnedData.front(); }
    constexpr const auto& front() const { return mOwnedData.front(); }
    constexpr auto& back() { return mOwnedData.back(); }
    constexpr const auto& back() const { return mOwnedData.back(); }
    constexpr auto& operator[](Index i) { return mOwnedData[i]; }
    constexpr const auto& operator[](Index i) const { return mOwnedData[i]; }

  private:
    // Constructs a HeapArray by taking ownership of an existing allocation that was allocated with
    // new[] (we will free it with delete[]). If the allocation is nullptr, we treat it as a failed
    // allocation (e.g. from AllocNoThrow), and replace the count with 0.
    [[nodiscard]] DAWN_UNSAFE_BUFFER_USAGE constexpr HeapArray(Value* data, Index count)
        : mOwnedData{data ? TSpan{data, count} : TSpan{}} {}

    enum class InitType {
        // Value-initialized
        Init,
        // For trivially constructible default types this uses their default-initialization which
        // leaves the value as uninitialized memory.
        Uninit,
    };

    enum class ThrowType {
        Throw,
        NoThrow,
    };

    // HeapArrays of byte types are used for allocations that can be reinterpreted as other
    // kinds of data. Make sure that they are aligned to max_align_t such that the allocation is
    // aligned enough for any reasonable type. Also ensure allocations are aligned to 16 as the
    // WebGPU API guarantees that alignment for mapped buffers.
    template <typename T>
    static constexpr bool IsByteType =
        std::is_same_v<std::remove_cv_t<T>, std::byte> ||
        std::is_same_v<std::remove_cv_t<T>, char> || std::is_same_v<std::remove_cv_t<T>, uint8_t>;
    template <typename T>
    static constexpr size_t AllocAlignment =
        IsByteType<T> ? std::max(alignof(std::max_align_t), size_t{16u}) : alignof(T);

    template <size_t Alignment>
    struct AlignedArrayOfBytes {
        alignas(Alignment) std::byte data[Alignment];
    };

    template <InitType Init,
              ThrowType Throws,
              typename AllocType = Value,
              typename IndexType = Index>
    static constexpr AllocType* Alloc(IndexType count) {
#if DAWN_ASAN_ENABLED() || DAWN_MSAN_ENABLED() || DAWN_TSAN_ENABLED()
        // std::nothrow isn't implemented in sanitizers and they often have a 2GB allocation
        // limit. Catch large allocations and error out so fuzzers make progress.
        [[maybe_unused]] constexpr size_t kLargestAllowedAllocationAttemptBytes = 0x70000000;
#else
        [[maybe_unused]] constexpr size_t kLargestAllowedAllocationAttemptBytes =
            std::numeric_limits<size_t>::max() - 4095;
#endif

        if constexpr (Throws == ThrowType::NoThrow) {
            // Early-fail to cover two cases:
            // - The checked_cast is going to fail.
            // - The total allocation size is going to be too close to the whole address space.
            //   PartitionAlloc in particular crashes instead of failing if (size >= SIZE_MAX - 23).
            if (I{count} > kLargestAllowedAllocationAttemptBytes / sizeof(AllocType)) {
                return nullptr;
            }
        }

        // Workaround an MSVC bug where new (std::align_val_t) produces an error C2956 because it
        // treats it the same as a placement new, and requires a matching placement delete, even if
        // it is a different construct. Even when using Clang, the Windows CRT also has mysterious
        // issues with new (std::align_val_t). So on Windows use new (std::align_t) only with clang
        // and PartitionAlloc and the workaround otherwise.
        // TODO(https://crbug.com/571664297): Revisit if/when MSVC and CRT no longer have issues.
        // TODO(b/571962072): Reenable on others OSes even when partition alloc is not present once
        // GWP-Asan is fixed.
#if DAWN_COMPILER_IS(MSVC) || !defined(DAWN_ENABLE_PARTITION_ALLOC)
        if constexpr (IsByteType<AllocType>) {
            // Replace allocations for byte types with an allocation of an aligned equivalent type
            // to force an alignment.
            using AlignedAllocType = AlignedArrayOfBytes<AllocAlignment<AllocType>>;
            static_assert(sizeof(AlignedAllocType) == AllocAlignment<AllocType>);
            static_assert(alignof(AlignedAllocType) == AllocAlignment<AllocType>);

            // Check against kLargestAllowedAllocationAttemptBytes because it ensures no overflow
            // happens in the addition below.
            static_assert(sizeof(AlignedAllocType) < 4095);
            DAWN_CHECK(checked_cast<size_t>(count) <=
                       kLargestAllowedAllocationAttemptBytes / sizeof(AllocType));
            size_t countForAligned = (checked_cast<size_t>(count) + sizeof(AlignedAllocType) - 1) /
                                     sizeof(AlignedAllocType);

            AlignedAllocType* aligned =
                Alloc<Init, Throws, AlignedAllocType, size_t>(countForAligned);
            return reinterpret_cast<AllocType*>(aligned);
        }

        if constexpr (Throws == ThrowType::NoThrow) {
            if constexpr (Init == InitType::Init) {
                return new (std::nothrow) AllocType[checked_cast<size_t>(count)]{};
            } else {
                return new (std::nothrow) AllocType[checked_cast<size_t>(count)];
            }
        } else {
            if constexpr (Init == InitType::Init) {
                return new AllocType[checked_cast<size_t>(count)]{};
            } else {
                return new AllocType[checked_cast<size_t>(count)];
            }
        }
#else   // MSVC || (WINDOWS && !PARTITION_ALLOC)

        constexpr std::align_val_t kAlignment{AllocAlignment<AllocType>};

        if constexpr (Throws == ThrowType::NoThrow) {
            if constexpr (Init == InitType::Init) {
                return new (kAlignment, std::nothrow) AllocType[checked_cast<size_t>(count)]{};
            } else {
                return new (kAlignment, std::nothrow) AllocType[checked_cast<size_t>(count)];
            }
        } else {
            if constexpr (Init == InitType::Init) {
                return new (kAlignment) AllocType[checked_cast<size_t>(count)]{};
            } else {
                return new (kAlignment) AllocType[checked_cast<size_t>(count)];
            }
        }
#endif  // MSVC || (WINDOWS && !PARTITION_ALLOC)

        DAWN_UNREACHABLE();
        return nullptr;
    }

    template <typename AllocType = Value>
    static constexpr void Delete(const AllocType* alloc) {
        // Match the logic in Alloc that allocates aligned array of bytes instead of using the
        // operator new that takes an std::align_val_t.
#if DAWN_COMPILER_IS(MSVC) || !defined(DAWN_ENABLE_PARTITION_ALLOC)
        if constexpr (IsByteType<AllocType>) {
            using AlignedAllocType = AlignedArrayOfBytes<AllocAlignment<AllocType>>;
            Delete<AlignedAllocType>(reinterpret_cast<const AlignedAllocType*>(alloc));
            return;
        }
#endif

        delete[] alloc;
    }
    // We store this as a span, but we own its allocation.
    // {nullptr, 0} = failed to allocate. (operator bool() returns false)
    // {non-null, size} = succeeded in allocating, even if size==0. (operator bool() returns true)
    TSpan mOwnedData;
};

// Construct a HeapArray by copying its data from a range.
template <typename Index>
[[nodiscard]] static constexpr auto HeapArrayFrom(const std::ranges::sized_range auto& src) {
    // Infer the value type. (HeapArrayFrom is defined outside HeapArray so that we can do this.)
    using Value = std::ranges::range_value_t<decltype(src)>;

    Index size = checked_cast<Index>(std::ranges::size(src));

    auto result = HeapArray<Index, Value>(size);
    std::ranges::copy(src, result.begin());
    return result;
}

}  // namespace ityp

// Aliases in the dawn:: namespace with size_t for the index type.
template <typename Value>
using HeapArray = ityp::HeapArray<size_t, Value>;

[[nodiscard]] static constexpr auto HeapArrayFrom(const std::ranges::sized_range auto& src) {
    return ityp::HeapArrayFrom<size_t>(src);
}

// Must be called to free allocations originating from a HeapArray and acquired with MoveToSpan.
template <typename T>
static constexpr void DeleteAllocationFromHeapArray(T* alloc) {
    HeapArray<std::remove_const_t<T>>::DeleteAllocation(alloc);
}

}  // namespace dawn

#endif  // SRC_UTILS_HEAP_ARRAY_H_
