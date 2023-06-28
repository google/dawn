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

#include <type_traits>

#include "gtest/gtest.h"

#include "dawn/common/Ref.h"
#include "dawn/common/RefCounted.h"
#include "dawn/native/ToBackend.h"

// Make our own Base - Backend object pair, reusing the MyObjectBase name
namespace dawn::native {

class MyObjectBase : public RefCounted {};

class MyObject : public MyObjectBase {};

struct MyBackendTraits {
    using MyObjectType = MyObject;
};

template <typename BackendTraits>
struct ToBackendTraits<MyObjectBase, BackendTraits> {
    using BackendType = typename BackendTraits::MyObjectType;
};

// Instantiate ToBackend for our "backend"
template <typename T>
auto ToBackend(T&& common) -> decltype(ToBackendBase<MyBackendTraits>(common)) {
    return ToBackendBase<MyBackendTraits>(common);
}

// Test that ToBackend correctly converts pointers to base classes.
TEST(ToBackend, Pointers) {
    {
        MyObject* myObject = new MyObject;
        const MyObjectBase* base = myObject;

        auto* backendAdapter = ToBackend(base);
        static_assert(std::is_same<decltype(backendAdapter), const MyObject*>::value);
        ASSERT_EQ(myObject, backendAdapter);

        myObject->Release();
    }
    {
        MyObject* myObject = new MyObject;
        MyObjectBase* base = myObject;

        auto* backendAdapter = ToBackend(base);
        static_assert(std::is_same<decltype(backendAdapter), MyObject*>::value);
        ASSERT_EQ(myObject, backendAdapter);

        myObject->Release();
    }
}

// Test that ToBackend correctly converts Refs to base classes.
TEST(ToBackend, Ref) {
    {
        MyObject* myObject = new MyObject;
        const Ref<MyObjectBase> base(myObject);

        const auto& backendAdapter = ToBackend(base);
        static_assert(std::is_same<decltype(ToBackend(base)), const Ref<MyObject>&>::value);
        ASSERT_EQ(myObject, backendAdapter.Get());

        myObject->Release();
    }
    {
        MyObject* myObject = new MyObject;
        Ref<MyObjectBase> base(myObject);

        auto backendAdapter = ToBackend(base);
        static_assert(std::is_same<decltype(ToBackend(base)), Ref<MyObject>&>::value);
        ASSERT_EQ(myObject, backendAdapter.Get());

        myObject->Release();
    }
}
}  // namespace dawn::native
