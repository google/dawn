// Copyright 2017 The Dawn Authors
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

#ifndef DAWNNATIVE_REFCOUNTED_H_
#define DAWNNATIVE_REFCOUNTED_H_

#include <atomic>
#include <cstdint>

namespace dawn_native {

    class RefCounted {
      public:
        RefCounted(uint64_t payload = 0);
        virtual ~RefCounted();

        uint64_t GetRefCountForTesting() const;
        uint64_t GetRefCountPayload() const;

        // Dawn API
        void Reference();
        void Release();

      protected:
        std::atomic_uint64_t mRefCount;
    };

    template <typename T>
    class Ref {
      public:
        Ref() {
        }

        Ref(T* p) : mPointee(p) {
            Reference();
        }

        Ref(const Ref<T>& other) : mPointee(other.mPointee) {
            Reference();
        }
        Ref<T>& operator=(const Ref<T>& other) {
            if (&other == this)
                return *this;

            other.Reference();
            Release();
            mPointee = other.mPointee;

            return *this;
        }

        Ref(Ref<T>&& other) {
            mPointee = other.mPointee;
            other.mPointee = nullptr;
        }
        Ref<T>& operator=(Ref<T>&& other) {
            if (&other == this)
                return *this;

            Release();
            mPointee = other.mPointee;
            other.mPointee = nullptr;

            return *this;
        }

        ~Ref() {
            Release();
            mPointee = nullptr;
        }

        operator bool() {
            return mPointee != nullptr;
        }

        const T& operator*() const {
            return *mPointee;
        }
        T& operator*() {
            return *mPointee;
        }

        const T* operator->() const {
            return mPointee;
        }
        T* operator->() {
            return mPointee;
        }

        const T* Get() const {
            return mPointee;
        }
        T* Get() {
            return mPointee;
        }

        T* Detach() {
            T* pointee = mPointee;
            mPointee = nullptr;
            return pointee;
        }

      private:
        void Reference() const {
            if (mPointee != nullptr) {
                mPointee->Reference();
            }
        }
        void Release() const {
            if (mPointee != nullptr) {
                mPointee->Release();
            }
        }

        // static_assert(std::is_base_of<RefCounted, T>::value, "");
        T* mPointee = nullptr;
    };

    template <typename T>
    Ref<T> AcquireRef(T* pointee) {
        Ref<T> ref(pointee);
        ref->Release();
        return ref;
    }

}  // namespace dawn_native

#endif  // DAWNNATIVE_REFCOUNTED_H_
