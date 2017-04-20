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

#ifndef BACKEND_COMMON_REFCOUNTED_H_
#define BACKEND_COMMON_REFCOUNTED_H_

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
            uint32_t externalRefs = 1;
            uint32_t internalRefs = 1;
    };

    template<typename T>
    class Ref {
        public:
            Ref() {}

            Ref(T* p): pointee(p) {
                Reference();
            }

            Ref(Ref<T>& other): pointee(other.pointee) {
                Reference();
            }
            Ref<T>& operator=(const Ref<T>& other) {
                if (&other == this) return *this;

                other.Reference();
                Release();
                pointee = other.pointee;

                return *this;
            }

            Ref(Ref<T>&& other) {
                pointee = other.pointee;
                other.pointee = nullptr;
            }
            Ref<T>& operator=(Ref<T>&& other) {
                if (&other == this) return *this;

                Release();
                pointee = other.pointee;
                other.pointee = nullptr;

                return *this;
            }

            ~Ref() {
                Release();
                pointee = nullptr;
            }

            operator bool() {
                return pointee != nullptr;
            }

            const T& operator*() const {
                return *pointee;
            }
            T& operator*() {
                return *pointee;
            }

            const T* operator->() const {
                return pointee;
            }
            T* operator->() {
                return pointee;
            }

            const T* Get() const {
                return pointee;
            }
            T* Get() {
                return pointee;
            }

        private:
            void Reference() const {
                if (pointee != nullptr) {
                    pointee->ReferenceInternal();
                }
            }
            void Release() const {
                if (pointee != nullptr) {
                    pointee->ReleaseInternal();
                }
            }

            //static_assert(std::is_base_of<RefCounted, T>::value, "");
            T* pointee = nullptr;
    };

}

#endif // BACKEND_COMMON_REFCOUNTED_H_
