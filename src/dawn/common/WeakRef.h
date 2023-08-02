// Copyright 2023 The Dawn Authors
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

#ifndef SRC_DAWN_COMMON_WEAKREF_H_
#define SRC_DAWN_COMMON_WEAKREF_H_

#include <utility>

#include "dawn/common/Ref.h"
#include "dawn/common/WeakRefSupport.h"

namespace dawn {

template <typename T>
class WeakRef;

template <
    typename T,
    typename = typename std::enable_if<std::is_base_of_v<detail::WeakRefSupportBase, T>>::type>
WeakRef<T> GetWeakRef(T* obj) {
    return WeakRef<T>(obj);
}

template <
    typename T,
    typename = typename std::enable_if<std::is_base_of_v<detail::WeakRefSupportBase, T>>::type>
WeakRef<T> GetWeakRef(const Ref<T>& obj) {
    return GetWeakRef(obj.Get());
}

template <typename T>
class WeakRef {
  public:
    WeakRef() {}

    // Constructors from nullptr.
    // NOLINTNEXTLINE(runtime/explicit)
    constexpr WeakRef(std::nullptr_t) : WeakRef() {}

    // Constructors from a WeakRef<U>, where U can also equal T.
    template <typename U, typename = typename std::enable_if<std::is_base_of_v<T, U>>::type>
    WeakRef(const WeakRef<U>& other) : mData(other.mData) {}
    template <typename U, typename = typename std::enable_if<std::is_base_of_v<T, U>>::type>
    WeakRef<T>& operator=(const WeakRef<U>& other) {
        mData = other.mData;
        return *this;
    }
    template <typename U, typename = typename std::enable_if<std::is_base_of_v<T, U>>::type>
    WeakRef(WeakRef<U>&& other) : mData(std::move(other.mData)) {}
    template <typename U, typename = typename std::enable_if<std::is_base_of_v<T, U>>::type>
    WeakRef<T>& operator=(WeakRef<U>&& other) {
        if (&other != this) {
            mData = std::move(other.mData);
        }
        return *this;
    }

    // Constructor from explicit WeakRefSupport<T>* is allowed.
    // NOLINTNEXTLINE(runtime/explicit)
    WeakRef(WeakRefSupport<T>* support) : mData(support->mData) {}

    // Promotes a WeakRef to a Ref. Access to the raw pointer is not allowed because a raw pointer
    // could become invalid after being retrieved.
    Ref<T> Promote() const {
        if (mData != nullptr) {
            return AcquireRef(static_cast<T*>(mData->TryGetRef().Detach()));
        }
        return nullptr;
    }

    // Returns the raw pointer to the RefCountedT if it has not been invalidated. Note that this
    // function is not thread-safe since the returned pointer can become invalid after being
    // retrieved.
    T* UnsafeGet() const {
        if (mData != nullptr) {
            return static_cast<T*>(mData->UnsafeGet());
        }
        return nullptr;
    }

    friend WeakRef GetWeakRef<>(T* obj);
    friend WeakRef GetWeakRef<>(const Ref<T>& obj);

  private:
    // Friend is needed so that we can access the data ref in conversions.
    template <typename U>
    friend class WeakRef;
    // Friend is needed to access the private constructor.
    template <typename U>
    friend class Ref;

    // Constructor from data should only be allowed via the GetWeakRef function.
    explicit WeakRef(detail::WeakRefSupportBase* data) : mData(data->mData) {}

    Ref<detail::WeakRefData> mData = nullptr;
};

}  // namespace dawn

#endif  // SRC_DAWN_COMMON_WEAKREFCOUNTED_H_
