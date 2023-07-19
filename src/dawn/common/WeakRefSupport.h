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

#ifndef SRC_DAWN_COMMON_WEAKREFSUPPORT_H_
#define SRC_DAWN_COMMON_WEAKREFSUPPORT_H_

#include "dawn/common/Ref.h"

namespace dawn {

template <typename T>
class WeakRef;

namespace detail {

// Indirection layer to provide external ref-counting for WeakRefs.
class WeakRefData : public RefCounted {
  public:
    explicit WeakRefData(RefCounted* value);
    void Invalidate();

    // Tries to return a valid Ref to `mValue` if it's internal refcount is not already 0. If the
    // internal refcount has already reached 0, returns nullptr instead.
    Ref<RefCounted> TryGetRef();

  private:
    std::mutex mMutex;
    RefCounted* mValue = nullptr;
};

// Interface base class used to enable compile-time verification of WeakRefSupport functions.
class WeakRefSupportBase {
  protected:
    explicit WeakRefSupportBase(Ref<detail::WeakRefData> data);
    ~WeakRefSupportBase();

  private:
    template <typename T>
    friend class ::dawn::WeakRef;

    Ref<detail::WeakRefData> mData;
};

}  // namespace detail

// Class that should be extended to enable WeakRefs for the type.
template <typename T>
class WeakRefSupport : public detail::WeakRefSupportBase {
  public:
    WeakRefSupport()
        : WeakRefSupportBase(AcquireRef(new detail::WeakRefData(static_cast<T*>(this)))) {}
};

}  // namespace dawn

#endif  // SRC_DAWN_COMMON_WEAKREFSUPPORT_H_
