// Copyright 2017 The NXT Authors
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

#ifndef BACKEND_REFCOUNTED_H_
#define BACKEND_REFCOUNTED_H_

#include <cstdint>

namespace backend {

    class RefCounted {
      public:
        RefCounted();
        virtual ~RefCounted();

        void ReferenceInternal();
        void ReleaseInternal();

        uint32_t GetExternalRefs() const;
        uint32_t GetInternalRefs() const;

        // NXT API
        void Reference();
        void Release();

      protected:
        uint32_t mExternalRefs = 1;
        uint32_t mInternalRefs = 1;
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

      private:
        void Reference() const {
            if (mPointee != nullptr) {
                mPointee->ReferenceInternal();
            }
        }
        void Release() const {
            if (mPointee != nullptr) {
                mPointee->ReleaseInternal();
            }
        }

        // static_assert(std::is_base_of<RefCounted, T>::value, "");
        T* mPointee = nullptr;
    };

}  // namespace backend

#endif  // BACKEND_REFCOUNTED_H_
