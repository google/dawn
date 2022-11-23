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

#ifndef SRC_DAWN_NATIVE_CACHEREQUEST_H_
#define SRC_DAWN_NATIVE_CACHEREQUEST_H_

#include <memory>
#include <utility>

#include "dawn/common/Assert.h"
#include "dawn/common/Compiler.h"
#include "dawn/native/Blob.h"
#include "dawn/native/BlobCache.h"
#include "dawn/native/CacheKey.h"
#include "dawn/native/CacheResult.h"
#include "dawn/native/Device.h"
#include "dawn/native/Error.h"
#include "dawn/native/VisitableMembers.h"

namespace dawn::native {

namespace detail {

template <typename T>
struct UnwrapResultOrError {
    using type = T;
};

template <typename T>
struct UnwrapResultOrError<ResultOrError<T>> {
    using type = T;
};

template <typename T>
struct IsResultOrError {
    static constexpr bool value = false;
};

template <typename T>
struct IsResultOrError<ResultOrError<T>> {
    static constexpr bool value = true;
};

void LogCacheHitError(std::unique_ptr<ErrorData> error);

}  // namespace detail

// Implementation of a CacheRequest which provides a LoadOrRun friend function which can be found
// via argument-dependent lookup. So, it doesn't need to be called with a fully qualified function
// name.
//
// Example usage:
//   Request r = { ... };
//   ResultOrError<CacheResult<T>> cacheResult =
//       LoadOrRun(device, std::move(r),
//                 [](Blob blob) -> T { /* handle cache hit */ },
//                 [](Request r) -> ResultOrError<T> { /* handle cache miss */ }
//       );
// Or with free functions:
///  T OnCacheHit(Blob blob) { ... }
//   ResultOrError<T> OnCacheMiss(Request r) { ... }
//   // ...
//   Request r = { ... };
//   auto result = LoadOrRun(device, std::move(r), OnCacheHit, OnCacheMiss);
//
// LoadOrRun generates a CacheKey from the request and loads from the device's BlobCache. On cache
// hit, calls CacheHitFn and returns a CacheResult<T>. On cache miss or if CacheHitFn returned an
// Error, calls CacheMissFn -> ResultOrError<T> with the request data and returns a
// ResultOrError<CacheResult<T>>. CacheHitFn must return the same unwrapped type as CacheMissFn.
// i.e. it doesn't need to be wrapped in ResultOrError.
//
// CacheMissFn may not have any additional data bound to it. It may not be a lambda or std::function
// which captures additional information, so it can only operate on the request data. This is
// enforced with a compile-time static_assert, and ensures that the result created from the
// computation is exactly the data included in the CacheKey.
template <typename Request>
class CacheRequestImpl {
  public:
    CacheRequestImpl() = default;

    // Require CacheRequests to be move-only to avoid unnecessary copies.
    CacheRequestImpl(CacheRequestImpl&&) = default;
    CacheRequestImpl& operator=(CacheRequestImpl&&) = default;
    CacheRequestImpl(const CacheRequestImpl&) = delete;
    CacheRequestImpl& operator=(const CacheRequestImpl&) = delete;

    // Create a CacheKey from the request type and all members
    CacheKey CreateCacheKey(const DeviceBase* device) const {
        CacheKey key = device->GetCacheKey();
        StreamIn(&key, Request::kName);
        static_cast<const Request*>(this)->VisitAll(
            [&](const auto&... members) { StreamIn(&key, members...); });
        return key;
    }

    template <typename CacheHitFn, typename CacheMissFn>
    friend auto LoadOrRun(DeviceBase* device,
                          Request&& r,
                          CacheHitFn cacheHitFn,
                          CacheMissFn cacheMissFn) {
        // Get return types and check that CacheMissReturnType can be cast to a raw function
        // pointer. This means it's not a std::function or lambda that captures additional data.
        using CacheHitReturnType = decltype(cacheHitFn(std::declval<Blob>()));
        using CacheMissReturnType = decltype(cacheMissFn(std::declval<Request>()));
        static_assert(
            std::is_convertible_v<CacheMissFn, CacheMissReturnType (*)(Request)>,
            "CacheMissFn function signature does not match, or it is not a free function.");

        static_assert(detail::IsResultOrError<CacheMissReturnType>::value,
                      "CacheMissFn should return a ResultOrError.");
        using UnwrappedReturnType = typename detail::UnwrapResultOrError<CacheMissReturnType>::type;

        static_assert(std::is_same_v<typename detail::UnwrapResultOrError<CacheHitReturnType>::type,
                                     UnwrappedReturnType>,
                      "If CacheMissFn returns T, CacheHitFn must return T or ResultOrError<T>.");

        using CacheResultType = CacheResult<UnwrappedReturnType>;
        using ReturnType = ResultOrError<CacheResultType>;

        CacheKey key = r.CreateCacheKey(device);
        Blob blob = device->GetBlobCache()->Load(key);

        if (!blob.Empty()) {
            // Cache hit. Handle the cached blob.
            auto result = cacheHitFn(std::move(blob));

            if constexpr (!detail::IsResultOrError<CacheHitReturnType>::value) {
                // If the result type is not a ResultOrError, return it.
                return ReturnType(CacheResultType::CacheHit(std::move(key), std::move(result)));
            } else {
                // Otherwise, if the value is a success, also return it.
                if (DAWN_LIKELY(result.IsSuccess())) {
                    return ReturnType(
                        CacheResultType::CacheHit(std::move(key), result.AcquireSuccess()));
                }
                // On error, continue to the cache miss path and log the error.
                detail::LogCacheHitError(result.AcquireError());
            }
        }
        // Cache miss, or the CacheHitFn failed.
        auto result = cacheMissFn(std::move(r));
        if (DAWN_LIKELY(result.IsSuccess())) {
            return ReturnType(CacheResultType::CacheMiss(std::move(key), result.AcquireSuccess()));
        }
        return ReturnType(result.AcquireError());
    }
};

}  // namespace dawn::native

// Helper for X macro to declare a struct member.
#define DAWN_INTERNAL_CACHE_REQUEST_DECL_STRUCT_MEMBER(type, name) type name{};

// Helper for X macro for recording cache request fields into a CacheKey.
#define DAWN_INTERNAL_CACHE_REQUEST_RECORD_KEY(type, name) StreamIn(&key, name);

// Helper X macro to define a CacheRequest struct.
// Example usage:
//   #define REQUEST_MEMBERS(X) \
//       X(int, a)              \
//       X(float, b)            \
//       X(Foo, foo)            \
//       X(Bar, bar)
//   DAWN_MAKE_CACHE_REQUEST(MyCacheRequest, REQUEST_MEMBERS)
//   #undef REQUEST_MEMBERS
#define DAWN_MAKE_CACHE_REQUEST(Request, MEMBERS)                      \
    class Request : public ::dawn::native::CacheRequestImpl<Request> { \
      public:                                                          \
        static constexpr char kName[] = #Request;                      \
        Request() = default;                                           \
        DAWN_VISITABLE_MEMBERS(MEMBERS)                                \
    }

// Helper macro for the common pattern of DAWN_TRY_ASSIGN around LoadOrRun.
// Requires an #include of dawn/native/Error.h
#define DAWN_TRY_LOAD_OR_RUN(var, ...) DAWN_TRY_ASSIGN(var, LoadOrRun(__VA_ARGS__))

#endif  // SRC_DAWN_NATIVE_CACHEREQUEST_H_
